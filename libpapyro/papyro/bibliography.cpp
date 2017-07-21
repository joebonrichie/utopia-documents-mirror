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

#include <papyro/bibliography.h>
#include <papyro/bibliography_p.h>
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

#include <QDateTime>
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

    BibliographyPrivate::BibliographyPrivate(Bibliography * bibliography)
        : QObject(bibliography),
          bibliography(bibliography),
          mutex(QMutex::Recursive),
          progress(-1),
          state(AbstractBibliography::IdleState),
          readOnly(false),
          persistenceModel(new NoPersistenceModel)
    {}

    void BibliographyPrivate::addItemIds(const CitationHandle & item)
    {
        QVariantMap ids(item->field(Citation::IdentifiersRole).toMap());
        QMapIterator< QString, QVariant > iter(ids);
        while (iter.hasNext()) {
            iter.next();
            if (!iter.key().isEmpty() && !iter.value().toString().isEmpty()) {
                itemsById[iter.key() + ":" + iter.value().toString()] = item;
            }
        }
        connect(item.get(), SIGNAL(changed(int, QVariant)),
                this, SLOT(onCitationChanged(int, QVariant)));
    }

    void BibliographyPrivate::onCitationChanged(int role, QVariant oldValue)
    {
        if (Citation * citation = dynamic_cast< Citation * >(sender())) {
            int row = 0;
            foreach (CitationHandle candidate, items) {
                if (candidate.get() == citation) {
                    break;
                }
                ++row;
            }
            if (row < items.count()) {
                QModelIndex index(bibliography->index(row, 0));
                QVector< int > roles; roles << Qt::DisplayRole << role;
                emit dataChanged(index, index, roles);
            }
        }
    }

    void BibliographyPrivate::removeItemIds(const CitationHandle & item)
    {
        QVariantMap ids(item->field(Citation::IdentifiersRole).toMap());
        QMapIterator< QString, QVariant > iter(ids);
        while (iter.hasNext()) {
            iter.next();
            if (!iter.key().isEmpty() && !iter.value().toString().isEmpty()) {
                itemsById.remove(iter.key() + ":" + iter.value().toString());
            }
        }
        disconnect(item.get(), SIGNAL(changed(int, QVariant)),
                   this, SLOT(onCitationChanged(int, QVariant)));
    }





    Bibliography::Bibliography(QObject * parent)
        : QAbstractItemModel(parent), d(new BibliographyPrivate(this))
    {
        connect(d, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &, const QVector< int > &)),
                this, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &, const QVector< int > &)));
    }

    Bibliography::~Bibliography()
    {}

    void Bibliography::appendItems(const QVector< CitationHandle > & items)
    {
        if (!items.isEmpty()) {
            // Only append items that are not already present. This ensures no
            // two items exist with the same key in the same bibliography.
            QVector< CitationHandle > newItems;
            foreach (CitationHandle item, items) {
                QString key = item->field(Citation::KeyRole).toString();
                if (!d->itemsByKey.contains(key)) {
                    newItems.append(item);
                    d->itemsByKey[key] = item;
                    item->setDirty();
                }
            }
            if (newItems.size() > 0) {
                beginInsertRows(QModelIndex(), rowCount(), rowCount() + newItems.size() - 1);
                d->items += newItems;
                foreach (const CitationHandle & item, newItems) {
                    d->addItemIds(item);
                }
                endInsertRows();
            }
        }
    }

    void Bibliography::clear()
    {
        beginRemoveRows(QModelIndex(), 0, d->items.size() - 1);
        d->items.clear();
        d->itemsByKey.clear();
        d->itemsById.clear();
        endRemoveRows();
    }

    int Bibliography::columnCount(const QModelIndex & index) const
    {
        return index.isValid() ? 0 : COLUMN_COUNT;
    }

    QVariant Bibliography::data(const QModelIndex & index, int role) const
    {
        if (index.row() >= 0 && index.row() < d->items.size()) {
            CitationHandle item = d->items.at(index.row());

            switch (role) {
            case Citation::ItemRole:
                return QVariant::fromValue(item);
            case Citation::FullTextSearchRole:
                // Ignore some fields for searching purposes
                switch (index.column() + Qt::UserRole) {
                case Citation::KeyRole:
                case Citation::TypeRole:
                case Citation::DocumentUriRole:
                case Citation::OriginatingUriRole:
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
                case Citation::OriginatingUriRole:
                case Citation::ObjectFileRole:
                case Citation::UnstructuredRole:
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

        return QVariant();
    }

    bool Bibliography::dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent)
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
                    CitationHandle citation = index.data(Citation::ItemRole).value< CitationHandle >();
                    if (!citation->field(Citation::DateImportedRole).toDateTime().isValid()) {
                        citation->setField(Citation::DateImportedRole, QDateTime::currentDateTime());
                    }
                    items << citation;
                }
                if (!items.isEmpty()) {
                    appendItems(items);
                }
            }
        }

        return true;
    }

    Qt::ItemFlags Bibliography::flags(const QModelIndex & index) const
    {
        Qt::ItemFlags flags(QAbstractItemModel::flags(index));
        if (index.isValid()) {
            flags |= Qt::ItemIsDragEnabled;
        } else if (!isReadOnly()) {
            flags |= Qt::ItemIsDropEnabled;
        }
        return flags;
    }

    QVariant Bibliography::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (role == Qt::DisplayRole && section >= 0) {
            if (orientation == Qt::Horizontal && section < COLUMN_COUNT) {
                return Citation::roleTitle((Citation::Role)(section + Qt::UserRole));
            } else if (orientation == Qt::Vertical && section < d->items.size()) {
                return QString::number(section + 1);
            }
        }
        return QVariant();
    }

    QModelIndex Bibliography::index(int row, int column, const QModelIndex & parent) const
    {
        // Only top-level indices can be created
        if (parent.isValid() || !hasIndex(row, column, parent)) {
            return QModelIndex();
        } else {
            return createIndex(row, column);
        }
    }

    void Bibliography::insertItems(CitationHandle before, const QVector< CitationHandle > & items)
    {
        if (!items.isEmpty()) {
            int idx = 0;
            QVector< CitationHandle >::iterator where(d->items.begin());
            while (where != d->items.end() && *where != before) { ++where; ++idx; }
            QVector< CitationHandle > newItems;
            foreach (CitationHandle item, items) {
                QString key = item->field(Citation::KeyRole).toString();
                if (!d->itemsByKey.contains(key)) {
                    newItems.append(item);
                    d->itemsByKey[key] = item;
                    item->setDirty();
                }
            }
            if (newItems.count() > 0) {
                beginInsertRows(QModelIndex(), idx, idx + newItems.count() - 1);
                d->items.insert(where, newItems.count(), CitationHandle());
                foreach (CitationHandle item, newItems) {
                    d->items[idx++] = item;
                    d->addItemIds(item);
                }
                endInsertRows();
            }
        }
    }

    bool Bibliography::insertRows(int row, int count, const QModelIndex & parent)
    {
        if (parent.isValid() || row < 0 || row > d->items.size()) {
            return false;
        }

        if (count > 0) {
            beginInsertRows(parent, row, row + count - 1);
            d->items.insert(row, count, CitationHandle());
            endInsertRows();
        }

        return true;
    }

    CitationHandle Bibliography::itemAt(int idx) const
    {
        return d->items.at(idx);
    }

    int Bibliography::itemCount(Citation::Flags flags) const
    {
        // FIXME
        if (flags == Citation::AllFlags) {
            return rowCount();
        } else {
            return 0;
        }
    }

    CitationHandle Bibliography::itemForId(const QString & id) const
    {
        return d->itemsById.value(id);
    }

    CitationHandle Bibliography::itemForKey(const QString & key) const
    {
        return d->itemsByKey.value(key);
    }

    QVector< CitationHandle > Bibliography::items() const
    {
        return d->items;
    }

    bool Bibliography::isReadOnly() const
    {
        return d->readOnly;
    }

    QMimeData * Bibliography::mimeData(const QModelIndexList & indexes) const
    {
        if (!indexes.isEmpty()) {
            QMimeData * mimeData = new BibliographicMimeData(indexes);
            mimeData->setData(_INTERNAL_MIMETYPE_CITATIONS, QByteArray());
            return mimeData;
        } else {
            return 0;
        }
    }

    QStringList Bibliography::mimeTypes() const
    {
        return QStringList() << QLatin1String(_INTERNAL_MIMETYPE_CITATIONS)
                             << "text/plain"
                             << "text/uri-list";
    }

    QModelIndex Bibliography::parent(const QModelIndex & /*index*/) const
    {
        // No parent, as there is no hierarchy
        return QModelIndex();
    }

    PersistenceModel * Bibliography::persistenceModel() const
    {
        return d->persistenceModel;
    }

    void Bibliography::prependItems(const QVector< CitationHandle > & items)
    {
        if (!items.isEmpty()) {
            int idx = 0;
            QVector< CitationHandle > newItems;
            foreach (CitationHandle item, items) {
                QString key = item->field(Citation::KeyRole).toString();
                if (!d->itemsByKey.contains(key)) {
                    newItems.append(item);
                    d->itemsByKey[key] = item;
                    item->setDirty();
                }
            }
            if (newItems.count() > 0) {
                beginInsertRows(QModelIndex(), idx, idx + newItems.count() - 1);
                d->items.insert(d->items.begin(), newItems.count(), CitationHandle());
                foreach (CitationHandle item, newItems) {
                    d->items[idx++] = item;
                    d->addItemIds(item);
                }
                endInsertRows();
            }
        }
    }

    qreal Bibliography::progress() const
    {
        return d->progress;
    }

    bool Bibliography::removeItem(CitationHandle item)
    {
        int row = d->items.indexOf(item);
        if (row >= 0) {
            // Remove item
            return removeRow(row);
        } else {
            // Not found
            return false;
        }
    }

    bool Bibliography::removeRows(int row, int count, const QModelIndex & parent)
    {
        QMutexLocker guard(&d->mutex);

        if (parent.isValid() || row < 0 || (count - row) > d->items.size()) {
            return false;
        } else {
            beginRemoveRows(parent, row, row + count - 1);
            for (int i = row; i < row + count; ++i) {
                CitationHandle item = d->items[i];
                d->itemsByKey.remove(item->field(Citation::KeyRole).toString());
                d->removeItemIds(item);
            }
            d->items.remove(row, count);
            endRemoveRows();
            return true;
        }
    }

    int Bibliography::rowCount(const QModelIndex & index) const
    {
        // Only the root item has children
        return index.isValid() ? 0 : d->items.size();
    }

    bool Bibliography::setData(const QModelIndex & index, const QVariant & value, int role)
    {
        QMutexLocker guard(&d->mutex);

        if (index.model() != this)
            return false;

        // Only top level items can be set
        QVariant oldValue(index.data(role));
        bool changed = (value != oldValue);
        if (index.row() >= 0 && index.row() < d->items.size()) {
            CitationHandle item = d->items.at(index.row());
            if (role == Qt::DisplayRole) {
                item->setField(index.column() + Qt::UserRole, value);
            } else if (role >= Qt::UserRole && role < Citation::MutableRoleCount) {
                item->setField(role, value);
            }
            return changed;
        }

        return false;
    }

    void Bibliography::setPersistenceModel(PersistenceModel * persistenceModel)
    {
        if (d->persistenceModel) {
            delete d->persistenceModel;
        }
        d->persistenceModel = persistenceModel;
    }

	void Bibliography::setProgress(qreal /*progress*/)
	{
	    /* no-op */
	}

    void Bibliography::setReadOnly(bool readOnly)
    {
        d->readOnly = readOnly;
    }

    void Bibliography::setState(AbstractBibliography::State state)
    {
	    if (d->state != state) {
	        d->state = state;
	        emit stateChanged(state);
	    }
    }

    void Bibliography::setTitle(const QString & title)
    {
        if (d->title != title) {
            d->title = title;
            emit titleChanged(title);
        }
    }

    AbstractBibliography::State Bibliography::state() const
    {
        // By default persistent models are idle, though they can be corrupt
        return d->state;
    }

    CitationHandle Bibliography::takeItemAt(int idx)
    {
        QMutexLocker guard(&d->mutex);
        CitationHandle taken;

        if (idx >= 0 && idx < d->items.size()) {
            beginRemoveRows(QModelIndex(), idx, idx);
            taken = d->items.at(idx);
            d->items.remove(idx);
            d->itemsByKey.remove(taken->field(Citation::KeyRole).toString());
            endRemoveRows();
        }

        return taken;
    }

    QString Bibliography::title() const
    {
        return d->title;
    }

} // namespace Athenaeum
