/*****************************************************************************
 *  
 *   This file is part of the Utopia Documents application.
 *       Copyright (c) 2008-2017 Lost Island Labs
 *           <info@utopiadocs.com>
 *   
 *   Utopia Documents is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU GENERAL PUBLIC LICENSE VERSION 3 as
 *   published by the Free Software Foundation.
 *   
 *   Utopia Documents is distributed in the hope that it will be useful, but
 *   WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 *   Public License for more details.
 *   
 *   In addition, as a special exception, the copyright holders give
 *   permission to link the code of portions of this program with the OpenSSL
 *   library under certain conditions as described in each individual source
 *   file, and distribute linked combinations including the two.
 *   
 *   You must obey the GNU General Public License in all respects for all of
 *   the code used other than OpenSSL. If you modify file(s) with this
 *   exception, you may extend this exception to your version of the file(s),
 *   but you are not obligated to do so. If you do not wish to do so, delete
 *   this exception statement from your version.
 *   
 *   You should have received a copy of the GNU General Public License
 *   along with Utopia Documents. If not, see <http://www.gnu.org/licenses/>
 *  
 *****************************************************************************/

#include <papyro/collection.h>
#include <papyro/collection_p.h>
#include <papyro/citation.h>
#include <papyro/bibliographicmimedata_p.h>
#include <papyro/persistencemodel.h>
#include <papyro/cJSON.h>
#include <spine/Annotation.h>
#include <spine/Document.h>
#include <utopia2/fileformat.h>
#include <utopia2/networkaccessmanager.h>
#include <utopia2/node.h>
#include <utopia2/parser.h>
#include <utopia2/qt/uimanager.h>

#include <QMetaProperty>
#include <QMimeData>
#include <QNetworkReply>
#include <QThreadPool>
#include <QUrl>
#include <QDebug>

#define COLUMN_COUNT (Citation::PersistentRoleCount - Qt::UserRole)
#define _INTERNAL_MIMETYPE_PLAINTEXT "text/plain"
#define _INTERNAL_MIMETYPE_URILIST "text/uri-list"

namespace Athenaeum
{

    CollectionPrivate::CollectionPrivate(Collection * collection, AbstractBibliography * sourceBibliography)
        : QObject(collection),
          collection(collection),
          sourceBibliography(sourceBibliography),
          mutex(QMutex::Recursive),
          progress(-1),
          state(AbstractBibliography::IdleState),
          readOnly(false),
          persistenceModel(0)
    {
        // Hook up signals so that the collection can keep up to date with the
        // source bibliography
        if (QAbstractItemModel * sourceModel = dynamic_cast< QAbstractItemModel * >(sourceBibliography)) {
            connect(sourceModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &,int,int)),
                    this, SLOT(rowsAboutToBeRemoved(const QModelIndex &,int,int)));
        }
    }

    void CollectionPrivate::rowsAboutToBeRemoved(const QModelIndex & parent, int first, int last)
    {
        if (QAbstractItemModel * sourceModel = dynamic_cast< QAbstractItemModel * >(sourceBibliography)) {
            for (int row = first; row <= last; ++row) {
                QModelIndex idx = sourceModel->index(row, 0, parent);
                QString key = sourceModel->data(idx, Citation::KeyRole).toString();
                int doomed = keys.indexOf(key);
                if (doomed >= 0) {
                    collection->removeRow(doomed);
                }
            }
        }
    }




    Collection::Collection(AbstractBibliography * sourceBibliography, QObject * parent)
        : QAbstractItemModel(parent), d(new CollectionPrivate(this, sourceBibliography))
    {}

    Collection::~Collection()
    {}

    void Collection::appendItems(const QVector< CitationHandle > & items)
    {
        if (d->sourceBibliography && !items.isEmpty()) {
            QVector< CitationHandle > newItems;
            QVector< QString > newKeys;
            foreach (CitationHandle item, items) {
                QString key = item->field(Citation::KeyRole).toString();
                if (!d->sourceBibliography->itemForKey(key)) {
                    newItems << item;
                }
                if (!d->keys.contains(key)) {
                    newKeys << key;
                }
            }
            beginInsertRows(QModelIndex(), rowCount(), rowCount() + newKeys.size());
            d->sourceBibliography->appendItems(newItems);
            d->keys += newKeys;
            endInsertRows();
        }
    }

    void Collection::clear()
    {
        beginRemoveRows(QModelIndex(), 0, d->keys.size() - 1);
        d->keys.clear();
        endRemoveRows();
    }

    int Collection::columnCount(const QModelIndex & index) const
    {
        return index.isValid() ? 0 : COLUMN_COUNT;
    }

    QVariant Collection::data(const QModelIndex & index, int role) const
    {
        if (/* QAbstractItemModel * sourceModel = */ dynamic_cast< QAbstractItemModel * >(d->sourceBibliography)) {
            if (index.row() >= 0 && index.row() < d->keys.size()) {
                QString key = d->keys.at(index.row());
                CitationHandle item = d->sourceBibliography->itemForKey(key);

                switch (role) {
                case Citation::ItemRole:
                    return QVariant::fromValue(item);
                case Citation::FullTextSearchRole:
                    // Ignore some fields for searching purposes
                    switch (index.column() + Qt::UserRole) {
                    case Citation::KeyRole:
                    case Citation::TypeRole:
                    case Citation::DocumentUriRole:
                    case Citation::UrlRole:
                    case Citation::ObjectFileRole:
                    case Citation::FlagsRole:
                    case Citation::StateRole:
                        return QVariant();
                    default:
                        break;
                    }
                case Qt::EditRole:
                case Qt::DisplayRole:
                    // Post processing of values
                    switch (index.column() + Qt::UserRole) {
                    case Citation::IdentifiersRole: {
                        QVariantMap identifiers(item->field(Citation::IdentifiersRole).toMap());
                        QMapIterator< QString, QVariant > iter(identifiers);
                        QStringList idText;
                        while (iter.hasNext()) {
                            iter.next();
                            idText << (iter.key() + ":" + iter.value().toString());
                        }
                        return idText.join("\n");
                    }
                    case Citation::AuthorsRole: {
                        QStringList authors(item->field(Citation::AuthorsRole).toStringList());
                        QStringList authorStrings;
                        foreach (const QString & author, authors) {
                            QString authorString;
                            foreach (const QString & forename, author.section(", ", 1, 1).split(" ")) {
                                authorString += forename.left(1).toUpper() + ". ";
                            }
                            authorString += author.section(", ", 0, 0);
                            authorString = authorString.trimmed();
                            if (!authorString.isEmpty()) {
                                authorStrings << authorString;
                            }
                        }
                        if (!authorStrings.isEmpty()) {
                            QString authorString;
                            if (authorStrings.size() == 1) {
                                authorString = authorStrings.at(0) + ".";
                            } else {
                                if (authorStrings.size() > 2) {
                                    authorString = QStringList(authorStrings.mid(0, authorStrings.size() - 2)).join(", ") + ", ";
                                }
                                authorString += authorStrings.at(authorStrings.size() - 2) + " and " + authorStrings.at(authorStrings.size() - 1);
                            }
                            return authorString;
                        }
                        break;
                    }
                    case Citation::KeywordsRole:
                        return item->field(Citation::KeywordsRole).toStringList().join(", ");
                    case Citation::DateImportedRole:
                    case Citation::DateModifiedRole:
                    case Citation::DateResolvedRole:
                    case Citation::DatePublishedRole:
                        return item->field(index.column() + Qt::UserRole).toDateTime().toString(Qt::ISODate);
                    case Citation::KeyRole:
                    case Citation::TitleRole:
                    case Citation::SubTitleRole:
                    case Citation::UrlRole:
                    case Citation::VolumeRole:
                    case Citation::IssueRole:
                    case Citation::YearRole:
                    case Citation::PageFromRole:
                    case Citation::PageToRole:
                    case Citation::AbstractRole:
                    case Citation::PublicationTitleRole:
                    case Citation::PublisherRole:
                    case Citation::TypeRole:
                    case Citation::DocumentUriRole:
                    case Citation::ObjectFileRole:
                        return item->field(index.column() + Qt::UserRole);
                    default:
                        // Should never happen
                        qWarning("data: invalid display value column %d", index.column());
                        break;
                    }
                    break;
                default:
                    // If it's just a writeable role, then return it unfiltered
                    if (role >= Qt::UserRole && role < Citation::MutableRoleCount) {
                        return item->field(role);
                    }
                    break;
                }
            }
        }
        return QVariant();
    }

    bool Collection::dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent)
    {
        if (data->hasUrls() || data->hasText()) { // Dragging PDFs and other files into a library
            // FIXME
        } else if (data->hasFormat(_INTERNAL_MIMETYPE_CITATIONS)) {
            if (const BibliographicMimeData * bibData = qobject_cast< const BibliographicMimeData * >(data)) {
                QMap< int, QModelIndex > ordered;
                foreach (const QModelIndex & index, bibData->indexes()) {
                    ordered[index.row()] = index;
                }
                QVector< CitationHandle > items;
                QMapIterator< int, QModelIndex > iter(ordered);
                iter.toBack();
                while (iter.hasPrevious()) {
                    iter.previous();
                    QModelIndex index = iter.value();
                    items << index.data(Citation::ItemRole).value< CitationHandle >();
                }
                if (!items.isEmpty()) {
                    appendItems(items);
                }
            }
        }

        return true;
    }

    Qt::ItemFlags Collection::flags(const QModelIndex & index) const
    {
        Qt::ItemFlags flags(QAbstractItemModel::flags(index));
        if (index.isValid()) {
            flags |= Qt::ItemIsDragEnabled;
        } else if (!isReadOnly()) {
            flags |= Qt::ItemIsDropEnabled;
        }
        return flags;
    }

    QVariant Collection::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (QAbstractItemModel * sourceModel = dynamic_cast< QAbstractItemModel * >(d->sourceBibliography)) {
            return sourceModel->headerData(section, orientation, role);
        }
        return QVariant();
    }

    QModelIndex Collection::index(int row, int column, const QModelIndex & parent) const
    {
        // Only top-level indices can be created
        if (parent.isValid() || !hasIndex(row, column, parent) || !d->sourceBibliography) {
            return QModelIndex();
        } else {
            return createIndex(row, column);
        }
    }

    void Collection::insertItems(CitationHandle before, const QVector< CitationHandle > & items)
    {
        if (!items.isEmpty() && d->sourceBibliography) {
            QVector< CitationHandle > newItems;
            QVector< QString > newKeys;
            foreach (CitationHandle item, items) {
                QString key = item->field(Citation::KeyRole).toString();
                if (!d->sourceBibliography->itemForKey(key)) {
                    newItems << item;
                }
                if (!d->keys.contains(key)) {
                    newKeys << key;
                }
            }

            int idx = 0;
            QString beforeKey = before->field(Citation::KeyRole).toString();
            QVector< QString >::iterator where(d->keys.begin());
            while (where != d->keys.end() && *where != beforeKey) { ++where; ++idx; }

            beginInsertRows(QModelIndex(), idx, idx + newKeys.count() - 1);
            d->sourceBibliography->appendItems(newItems);
            d->keys.insert(where, newKeys.count(), QString());
            foreach (const QString & key, newKeys) {
                d->keys[idx++] = key;
            }
            endInsertRows();
        }
    }

    bool Collection::insertRows(int row, int count, const QModelIndex & parent)
    {
        if (parent.isValid() || row < 0 || row > d->keys.size()) {
            return false;
        }

        beginInsertRows(parent, row, row + count - 1);
        d->keys.insert(row, count, QString());
        endInsertRows();

        return true;
    }

    CitationHandle Collection::itemAt(int idx) const
    {
        if (d->sourceBibliography) {
            return d->sourceBibliography->itemForKey(d->keys.at(idx));
        } else {
            return CitationHandle();
        }
    }

    int Collection::itemCount(Citation::Flags flags) const
    {
        if (flags == Citation::AllFlags) {
            return rowCount();
        } else {
            return 0;
        }
    }

    CitationHandle Collection::itemForId(const QString & id) const
    {
        if (d->sourceBibliography) {
            if (CitationHandle found = d->sourceBibliography->itemForId(id)) {
                QString key(found->field(Citation::KeyRole).toString());
                if (!key.isEmpty() && d->keys.contains(key)) {
                    return found;
                }
            }
        }
        return CitationHandle();
    }

    CitationHandle Collection::itemForKey(const QString & key) const
    {
        if (d->sourceBibliography && d->keys.contains(key)) {
            return d->sourceBibliography->itemForKey(key);
        } else {
            return CitationHandle();
        }
    }

    QVector< CitationHandle > Collection::items() const
    {
        QVector< CitationHandle > items;
        if (d->sourceBibliography) {
            foreach (const QString & key, d->keys) {
                if (CitationHandle item = d->sourceBibliography->itemForKey(key)) {
                    items.append(item);
                }
            }
        }
        return items;
    }

    bool Collection::isReadOnly() const
    {
        return d->readOnly;
    }

    QMimeData * Collection::mimeData(const QModelIndexList & indexes) const
    {
        if (!indexes.isEmpty()) {
            QMimeData * mimeData = new BibliographicMimeData(indexes);
            mimeData->setData(_INTERNAL_MIMETYPE_CITATIONS, QByteArray());
            return mimeData;
        } else {
            return 0;
        }
    }

    QStringList Collection::mimeTypes() const
    {
        return QStringList() << QLatin1String(_INTERNAL_MIMETYPE_CITATIONS)
                             << "text/plain"
                             << "text/uri-list";
    }

    QModelIndex Collection::parent(const QModelIndex & /*index*/) const
    {
        // No parent, as there is no hierarchy
        return QModelIndex();
    }

    PersistenceModel * Collection::persistenceModel() const
    {
        return d->persistenceModel;
    }

    void Collection::prependItems(const QVector< CitationHandle > & items)
    {
        CitationHandle before;
        if (d->keys.isEmpty()) {
            before = d->sourceBibliography->itemForKey(d->keys.first());
        }
        insertItems(before, items);
    }

    qreal Collection::progress() const
    {
        return d->progress;
    }

    bool Collection::removeItem(CitationHandle item)
    {
        QString key = item->field(Citation::KeyRole).toString();
        int idx = d->keys.indexOf(key);
        if (idx >= 0) {
            d->keys.remove(idx);
            return true;
        }

        // Not found
        return false;
    }

    bool Collection::removeRows(int row, int count, const QModelIndex & parent)
    {
        QMutexLocker guard(&d->mutex);

        if (parent.isValid() || row < 0 || (count - row) > d->keys.size()) {
            return false;
        } else {
            beginRemoveRows(parent, row, row + count - 1);
            d->keys.remove(row, count);
            endRemoveRows();
            return true;
        }
    }

    int Collection::rowCount(const QModelIndex & index) const
    {
        // Only the root item has children
        return index.isValid() ? 0 : d->keys.size();
    }

    bool Collection::setData(const QModelIndex & index, const QVariant & value, int role)
    {
        // FIXME
        return false;
    }

    void Collection::setPersistenceModel(PersistenceModel * persistenceModel)
    {
        if (d->persistenceModel) {
            delete d->persistenceModel;
        }
        d->persistenceModel = persistenceModel;
    }

	void Collection::setProgress(qreal /*progress*/)
	{
	    /* no-op */
	}

    void Collection::setReadOnly(bool readOnly)
    {
        d->readOnly = readOnly;
    }

    void Collection::setState(AbstractBibliography::State state)
    {
	    if (d->state != state) {
	        d->state = state;
	        emit stateChanged(state);
	    }
    }

    void Collection::setTitle(const QString & title)
    {
        if (d->title != title) {
            d->title = title;
            emit titleChanged(title);
        }
    }

    AbstractBibliography * Collection::sourceBibliography() const
    {
        return d->sourceBibliography;
    }

    AbstractBibliography::State Collection::state() const
    {
        // By default persistent models are idle, though they can be corrupt
        return d->state;
    }

    CitationHandle Collection::takeItemAt(int idx)
    {
        CitationHandle taken;

        if (d->sourceBibliography && idx >= 0 && idx < d->keys.size()) {
            beginRemoveRows(QModelIndex(), idx, idx);
            taken = d->sourceBibliography->itemForKey(d->keys.at(idx));
            d->keys.remove(idx);
            endRemoveRows();
        }

        return taken;
    }

    QString Collection::title() const
    {
        return d->title;
    }

} // namespace Athenaeum
