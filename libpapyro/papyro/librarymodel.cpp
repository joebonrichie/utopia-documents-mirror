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

#include <papyro/librarymodel.h>
#include <papyro/librarymodel_p.h>
#include <papyro/collection.h>
#include <papyro/bibliographicmimedata_p.h>
#include <papyro/filters.h>
#include <papyro/resolverqueue.h>
#include <papyro/persistencemodel.h>
#include <papyro/remotequerybibliography.h>
#include <papyro/sortfilterproxymodel.h>
#include <utopia2/global.h>

#include <boost/weak_ptr.hpp>

#include <QDateTime>
#include <QDir>
#include <QMimeData>
#include <QPixmap>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QThreadPool>
#include <QUrl>
#include <QUuid>

#include <QDebug>

#define _INTERNAL_MIMETYPE_LIBRARYMODELS "application/x-utopia-internal-librarymodels"
#define _INTERNAL_MIMETYPE_SEARCHMODELS "application/x-utopia-internal-searchmodels"

namespace Athenaeum
{

    namespace
    {

        // Anonymous column index names
        enum {
            COLUMN_TITLE = 0,
            COLUMN_STATE,
            COLUMN_ITEM_COUNT,
            COLUMN_UNREAD_ITEM_COUNT,
            COLUMN_IMPORTANT_ITEM_COUNT,
            COLUMN_CAN_FETCH_MORE,

            COLUMN_COUNT
        };

        static const QAbstractItemModel * origin(const QAbstractItemModel * model)
        {
            if (const QSortFilterProxyModel * proxy = qobject_cast< const QSortFilterProxyModel * >(model)) {
                return origin(proxy->sourceModel());
            } else {
                return model;
            }
        }

        static bool modelAcceptsDrop(QAbstractItemModel * model, const QMimeData * mime)
        {
            AbstractBibliography * bibliography = qobject_cast< AbstractBibliography * >(model);
            if (model && (!bibliography || !bibliography->isReadOnly())) {
                // Reject internal moves from/to the same model
                if (mime->hasFormat(_INTERNAL_MIMETYPE_CITATIONS)) {
                    const BibliographicMimeData * bibData = qobject_cast< const BibliographicMimeData * >(mime);
                    if (bibData && !bibData->indexes().isEmpty() && origin(bibData->indexes().first().model()) == origin(model)) {
                        return false;
                    }
                }

                // Check for a matching mime type
                foreach (const QString & type, model->mimeTypes()) {
                    if (mime->hasFormat(type)) {
                        return true;
                    }
                }
            }

            return false;
        }


    } // Anonymous namespace




    LibraryModelPrivate::LibraryModelPrivate(LibraryModel * model)
        : QObject(model),
          m(model),
          master(0),
          starred(0),
          recent(0),
          noCollectionPlaceholder(true),
          noWatchPlaceholder(true)
    {}

    LibraryModelPrivate::~LibraryModelPrivate()
    {}

    void LibraryModelPrivate::connectModel(QAbstractItemModel * model)
    {
        connect(model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
                this, SLOT(onDataChanged(const QModelIndex &, const QModelIndex &)));
        connect(model, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
                this, SLOT(onRowsInserted(const QModelIndex &, int, int)));
        connect(model, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
                this, SLOT(onRowsRemoved(const QModelIndex &, int, int)));
        if (qobject_cast< Bibliography * >(model)) {
            connect(model, SIGNAL(stateChanged(Athenaeum::AbstractBibliography::State)),
                    this, SLOT(onStateChanged(Athenaeum::AbstractBibliography::State)));
            connect(model, SIGNAL(titleChanged(const QString &)),
                    this, SLOT(onTitleChanged(const QString &)));
        }
    }

    void LibraryModelPrivate::disconnectModel(QAbstractItemModel * model)
    {
        disconnect(model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
                   this, SLOT(onDataChanged(const QModelIndex &, const QModelIndex &)));
        disconnect(model, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
                   this, SLOT(onRowsInserted(const QModelIndex &, int, int)));
        disconnect(model, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
                   this, SLOT(onRowsRemoved(const QModelIndex &, int, int)));
        if (qobject_cast< Bibliography * >(model)) {
            disconnect(model, SIGNAL(stateChanged(Athenaeum::AbstractBibliography::State)),
                       this, SLOT(onStateChanged(Athenaeum::AbstractBibliography::State)));
            disconnect(model, SIGNAL(titleChanged(const QString &)),
                       this, SLOT(onTitleChanged(const QString &)));
        }
    }

    QModelIndex LibraryModelPrivate::collectionParentIndex() const
    {
        return m->index(4, 0);
    }

    QModelIndex LibraryModelPrivate::masterIndex() const
    {
        return m->index(1, 0);
    }

    QModelIndex LibraryModelPrivate::starredIndex() const
    {
        return m->index(2, 0);
    }

    QModelIndex LibraryModelPrivate::recentIndex() const
    {
        return m->index(3, 0);
    }

    void LibraryModelPrivate::onDataChanged(const QModelIndex & from, const QModelIndex & to, const QVector< int > & roles)
    {
        relayDataChanged();

        // If resolution has just happened, rename the object file
        if (sender() == master) {
            // Deal with object file in case its name has changed
            if (roles.isEmpty() || roles.contains(Citation::DateResolvedRole)) {
                for (int row = from.row(); row <= to.row(); ++row) {
                    QModelIndex index = master->index(row, 0);
                    CitationHandle citation = index.data(Citation::ItemRole).value< CitationHandle >();
                    QUrl oldObjectPath(citation->field(Citation::ObjectFileRole).toUrl());
                    QString newObjectPath(m->getObjectFilePath(citation));
                    if (oldObjectPath.isValid() && oldObjectPath.toLocalFile() != newObjectPath) {
                        // Move the file and update the object path
                        if (QFile::rename(oldObjectPath.toLocalFile(), newObjectPath)) {
                            citation->setField(Citation::ObjectFileRole, QUrl::fromLocalFile(newObjectPath));
                        }
                    }
                }
            }
        }
    }

    void LibraryModelPrivate::onRowsAboutToBeRemoved(const QModelIndex & parent, int start, int end)
    {
        // If the model is the master model, make sure we change the citations
        // to be 'unknown'
        if (Bibliography * model = qobject_cast< Bibliography * >(sender())) {
            if (model == master) {
                for (int row = start; row <= end; ++row) {
                    QModelIndex index = master->index(start, 0, parent);
                    master->setData(index, false, Citation::KnownRole);
                    master->setData(index, QVariant::fromValue< Citation::Flags >(Citation::Flags()), Citation::FlagsRole);
                }
            }
        }
    }

    void LibraryModelPrivate::onRowsInserted(const QModelIndex & parent, int start, int end)
    {
        // If the model is the master model, make sure we change the citations
        // to be 'known'
        if (Bibliography * model = qobject_cast< Bibliography * >(sender())) {
            if (model == master) {
                for (int row = start; row <= end; ++row) {
                    QModelIndex index = master->index(row, 0, parent);
                    master->setData(index, true, Citation::KnownRole);
                }
            }
        }

        relayDataChanged();
    }

    void LibraryModelPrivate::onRowsRemoved(const QModelIndex &, int, int)
    {
        relayDataChanged();
    }

    void LibraryModelPrivate::onStateChanged(Athenaeum::AbstractBibliography::State state)
    {
        relayDataChanged();
    }

    void LibraryModelPrivate::onTitleChanged(const QString & title)
    {
        relayDataChanged();
    }

    void LibraryModelPrivate::relayDataChanged()
    {
        emit dataChanged(collectionParentIndex(), searchParentIndex());
    }

    void LibraryModelPrivate::save()
    {
        if (master && master->persistenceModel() && master->persistenceModel()->isSaveable()) {
            master->persistenceModel()->save(master);
        }
        foreach (QAbstractItemModel * model, models) {
            AbstractBibliography * bibliography = qobject_cast< AbstractBibliography * >(model);
            if (bibliography && bibliography->persistenceModel() && bibliography->persistenceModel()->isSaveable()) {
                bibliography->persistenceModel()->save(model);
            }
        }
    }

    QModelIndex LibraryModelPrivate::searchParentIndex() const
    {
        return m->index(5, 0);
    }

    void LibraryModelPrivate::updateMimeTypes()
    {
        // Iterate over the managed models and accumulate a list of accepted mime types
        QSet< QString > types;
        foreach (QAbstractItemModel * model, models) {
            types.unite(QSet< QString >::fromList(model->mimeTypes()));
        }
        if (master) {
            types.unite(QSet< QString >::fromList(master->mimeTypes()));
        }
        mimeTypes = types.toList();
    }




    LibraryModel::LibraryModel(QObject * parent)
        : QAbstractItemModel(parent), d(new LibraryModelPrivate(this))
    {
        connect(d, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
                this, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)));

        // Now load library bibliographies from disk

        // Begin by loading the master bibliography
        QDir dataRoot(Utopia::profile_path());
        if (dataRoot.cd("library") || (dataRoot.mkdir("library") && dataRoot.cd("library"))) {
            QDir masterDir(dataRoot);
            if (masterDir.cd("master") || (masterDir.mkdir("master") && masterDir.cd("master"))) {
                d->master = new Athenaeum::Bibliography(this);
                d->connectModel(d->master);
                connect(d->master, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)),
                        d, SLOT(onRowsAboutToBeRemoved(const QModelIndex &, int, int)));
                if (d->master->property("title").toString().isEmpty()) {
                    d->master->setProperty("title", "My Library");
                }

                // Ready to kick off any resolution that needs doing
                d->resolverQueue = new ResolverQueue(d->master, this);

                Athenaeum::LocalPersistenceModel * persistenceModel = new Athenaeum::LocalPersistenceModel(masterDir.absolutePath(), d->master);
                d->master->setPersistenceModel(persistenceModel);
                persistenceModel->load(d->master);
                d->starred = new SortFilterProxyModel(this);
                d->starred->setFilter(new StarredFilter(d->starred));
                d->starred->setSourceModel(d->master);
                d->connectModel(d->starred);
                d->recent = new SortFilterProxyModel(this);
                DateTimeFilter * recentFilter = new DateTimeFilter(d->recent);
                recentFilter->setDateTimeFrom(QDateTime::currentDateTime().addMonths(-1)); // Arbitrarily see the last 1 month
                recentFilter->setRole(Citation::DateImportedRole);
                d->recent->setFilter(recentFilter); // FIXME
                d->recent->setSourceModel(d->master);
                d->connectModel(d->recent);
            } else {
                qDebug() << "=== Could not open master library directory";
            }
            if (d->master) {
                QDir collectionsDir(dataRoot);
                if (collectionsDir.cd("collections") || (collectionsDir.mkdir("collections") && collectionsDir.cd("collections"))) {
                    foreach (const QFileInfo & dir, collectionsDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
                        Collection * collection = new Collection(d->master, this);
                        Athenaeum::CollectionPersistenceModel * persistenceModel = new Athenaeum::CollectionPersistenceModel(dir.absoluteFilePath(), collection);
                        collection->setPersistenceModel(persistenceModel);
                        persistenceModel->load(collection);
                        appendModel(collection);
                    }
                } else {
                    qDebug() << "=== Could not open collections directory";
                }
            }
        } else {
            qDebug() << "=== Could not open data directory";
        }

        d->updateMimeTypes();
    }

    LibraryModel::~LibraryModel()
    {
        d->save();
    }

//     static bool canDecode(QAbstractItemModel * model, const QMimeData * mimeData)
//     {
//         QStringList modelTypes = model->mimeTypes();
//         for (int i = 0; i < modelTypes.count(); ++i) {
//             if (mimeData->hasFormat(modelTypes.at(i))) {
//                 return true;
//             }
//         }
//         return false;
//     }

    bool LibraryModel::acceptsDrop(const QModelIndex & index, bool child, const QMimeData * mimeData)
    {
        // Check for internal drop operations
        if (mimeData->hasFormat(_INTERNAL_MIMETYPE_LIBRARYMODELS)) {
            return (child && index == d->collectionParentIndex());
        } else if (mimeData->hasFormat(_INTERNAL_MIMETYPE_SEARCHMODELS)) {
            return (child && index == d->searchParentIndex());
        } else if (!child) {
            // Check if the underlying model accepts the mimeData
            quint32 id = index.internalId();
            if (id > _PLACEHOLDER_COUNT) {
                QAbstractItemModel * qaim = (QAbstractItemModel *) index.internalPointer();
                RemoteQueryBibliography * search = qobject_cast< RemoteQueryBibliography * >(qaim);
                return qaim && !search && modelAcceptsDrop(qaim, mimeData);
            }
        }

        return false;
    }

    void LibraryModel::appendModel(QAbstractItemModel * model)
    {
        QModelIndex parent(d->collectionParentIndex());

        if (d->models.isEmpty()) {
            beginRemoveRows(parent, 0, 0);
            d->noCollectionPlaceholder = false;
            endRemoveRows();
        }

        int row = d->models.size();
        beginInsertRows(parent, row, row);
        d->models.append(model);
        d->connectModel(model);
        d->updateMimeTypes();
        endInsertRows();
    }

    void LibraryModel::appendSearch(QAbstractItemModel * model)
    {
        if (RemoteQueryBibliography * remotemodel = qobject_cast< RemoteQueryBibliography * >(model)) {
            QModelIndex parent(d->searchParentIndex());
            bool replaced = d->searches.isEmpty();
            if (!replaced) {
                int row(rowCount(parent));
                beginInsertRows(parent, row, row);
            }
            d->searches.append(remotemodel);
            d->connectModel(model);
            if (!replaced) {
                endInsertRows();
            } else {
                QModelIndex newIndex(index(0, 0, parent));
                emit dataChanged(newIndex, newIndex);
            }
        }
    }

    QModelIndex LibraryModel::collectionParentIndex() const
    {
        return d->collectionParentIndex();
    }

    int LibraryModel::columnCount(const QModelIndex & index) const
    {
        return COLUMN_COUNT;
    }

    QVariant LibraryModel::data(const QModelIndex & index, int role) const
    {
        static const QPixmap iconLibrary(":/icons/library-icon.png");
        static const QPixmap iconSearch(":/icons/search-icon.png");
        static const QPixmap iconFilter(":/icons/search-icon.png");

        quint32 id = index.internalId();
        if (id < _PLACEHOLDER_COUNT) {
            if (role == Qt::DisplayRole) {
                switch (id) {
                case _EVERYTHING:
                    return "Search Online";
                case _HEADER_COLLECTIONS:
                    return "COLLECTIONS";
                case _HEADER_SEARCHES:
                    return "WATCH LIST";
                case _NO_COLLECTIONS:
                    return "No collections";
                case _NO_SEARCHES:
                    return "No watches";
                }
            }
        } else {
            QAbstractItemModel * qaim = (QAbstractItemModel *) index.internalPointer();
            AbstractBibliography * collection = qobject_cast< AbstractBibliography * >(qaim);
            //AbstractBibliography * model = qobject_cast< AbstractBibliography * >(qaim);
            //RemoteQueryBibliography * search = qobject_cast< RemoteQueryBibliography * >(qaim);
            SortFilterProxyModel * starred = (index == d->starredIndex() ? qobject_cast< SortFilterProxyModel * >(qaim) : 0);
            if (starred) {
                collection = qobject_cast< AbstractBibliography * >(starred->sourceModel());
            }
            SortFilterProxyModel * recent = (index == d->recentIndex() ? qobject_cast< SortFilterProxyModel * >(qaim) : 0);
            if (recent) {
                collection = qobject_cast< AbstractBibliography * >(recent->sourceModel());
            }

            // Without a model, there's nothing to return
            if (collection) {
                switch (role) {
                case Qt::DecorationRole:
                    if (index.column() == 0) {
/*
                        if (search) {
                            return iconSearch;
                        } else if (model) {
                            return iconLibrary;
                        } else {
                            return iconFilter;
                        }
*/
                    }
                    break;
                case Qt::EditRole:
                case Qt::DisplayRole:
                    // Post processing of values
                    switch (index.column()) {
                    case COLUMN_TITLE:
                        return starred ? QString("Starred Articles") : recent ? QString("Recently Imported") : collection->title();
                    case COLUMN_ITEM_COUNT:
                        return starred ? starred->rowCount() : recent ? recent->rowCount() : collection->itemCount(Citation::AllFlags);
                    case COLUMN_UNREAD_ITEM_COUNT:
                        return starred || recent ? 0 : collection->itemCount(Citation::UnreadFlag);
                    case COLUMN_IMPORTANT_ITEM_COUNT:
                        return starred ? starred->rowCount() : recent ? recent->rowCount() : collection->itemCount(Citation::StarredFlag);
                    case COLUMN_CAN_FETCH_MORE:
                        return qaim->canFetchMore(QModelIndex());
                    case COLUMN_STATE:
                        // No textual representation
                        break;
                    default:
                        // Should never happen
                        qWarning("data: invalid display value column %d", index.column());
                        break;
                    }
                    break;
                case ModelRole:
                    return QVariant::fromValue(qaim);
                case TitleRole:
                    return starred ? QString("Starred articles") : recent ? QString("Recently Imported") : collection->title();
                case StateRole:
                    return QVariant::fromValue(collection->state());
                case ItemCountRole:
                    return starred ? starred->rowCount() : recent ? recent->rowCount() : collection->itemCount(Citation::AllFlags);
                case UnreadItemCountRole:
                    return starred || recent ? 0 : collection->itemCount(Citation::UnreadFlag);
                case ImportantItemCountRole:
                    return starred ? starred->rowCount() : recent ? recent->rowCount() : collection->itemCount(Citation::StarredFlag);
                case CanFetchMoreRole:
                    return qaim->canFetchMore(QModelIndex());
                default:
                    break;
                }
            }
        }

        return QVariant();
    }

    bool LibraryModel::dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent)
    {
        //qDebug() << "LibraryModel::dropMimeData" << data << action << row << parent;
        // Model dragging / dropping
        if (parent == d->collectionParentIndex() && data->hasFormat(_INTERNAL_MIMETYPE_LIBRARYMODELS)) {
            QVector< int > sourceRowsBeforeDrop;
            QVector< int > sourceRowsAfterDrop;
            foreach (const QString & sourceIndex, QString::fromUtf8(data->data(_INTERNAL_MIMETYPE_LIBRARYMODELS)).split(" ")) {
                int sourceRow = sourceIndex.toInt();
                if (sourceRow < row) {
                    sourceRowsBeforeDrop << sourceRow;
                } else {
                    sourceRowsAfterDrop << sourceRow;
                }
            }
            qSort(sourceRowsAfterDrop.begin(), sourceRowsAfterDrop.end());
            foreach (int sourceRow, sourceRowsAfterDrop) {
                // Move the source row to the current row
                int destinationRow = row >= 0 ? row : rowCount(parent) - 1;
                //qDebug() << "++<" << row << sourceRow << destinationRow;
                if (sourceRow != destinationRow) {
                    beginMoveRows(parent, sourceRow, sourceRow, parent, row);
                    d->models.move(sourceRow, destinationRow);
                    endMoveRows();
                }
            }
            qSort(sourceRowsBeforeDrop.begin(), sourceRowsBeforeDrop.end(), qGreater< int >());
            foreach (int sourceRow, sourceRowsBeforeDrop) {
                // Move the source row to the current row
                int destinationRow = (row >= 0 ? row : rowCount(parent)) - 1;
                //qDebug() << "++>" << row << sourceRow << destinationRow;
                if (sourceRow != destinationRow) {
                    beginMoveRows(parent, sourceRow, sourceRow, parent, row);
                    d->models.move(sourceRow, destinationRow);
                    endMoveRows();
                }
            }
        } else if (parent == d->searchParentIndex() && data->hasFormat(_INTERNAL_MIMETYPE_SEARCHMODELS)) { // Search dragging / dropping
            QVector< int > sourceRowsBeforeDrop;
            QVector< int > sourceRowsAfterDrop;
            foreach (const QString & sourceIndex, QString::fromUtf8(data->data(_INTERNAL_MIMETYPE_SEARCHMODELS)).split(" ")) {
                int sourceRow = sourceIndex.toInt();
                if (sourceRow < row) {
                    sourceRowsBeforeDrop << sourceRow;
                } else {
                    sourceRowsAfterDrop << sourceRow;
                }
            }
            qSort(sourceRowsAfterDrop.begin(), sourceRowsAfterDrop.end());
            foreach (int sourceRow, sourceRowsAfterDrop) {
                // Move the source row to the current row
                int destinationRow = row >= 0 ? row : rowCount(parent) - 1;
                //qDebug() << "++<" << row << sourceRow << destinationRow;
                if (sourceRow != destinationRow) {
                    beginMoveRows(parent, sourceRow, sourceRow, parent, row);
                    d->searches.move(sourceRow, destinationRow);
                    endMoveRows();
                }
            }
            qSort(sourceRowsBeforeDrop.begin(), sourceRowsBeforeDrop.end(), qGreater< int >());
            foreach (int sourceRow, sourceRowsBeforeDrop) {
                // Move the source row to the current row
                int destinationRow = (row >= 0 ? row : rowCount(parent)) - 1;
                //qDebug() << "++>" << row << sourceRow << destinationRow;
                if (sourceRow != destinationRow) {
                    beginMoveRows(parent, sourceRow, sourceRow, parent, row);
                    d->searches.move(sourceRow, destinationRow);
                    endMoveRows();
                }
            }
        } else if (!parent.parent().isValid() && row >= 0) { // Drop on item itself
            quint32 id = index(row, column, parent).internalId();
            if (id > _PLACEHOLDER_COUNT) {
                QAbstractItemModel * qaim = (QAbstractItemModel *) index(row, column, parent).internalPointer();
                AbstractBibliography * model = qobject_cast< AbstractBibliography * >(qaim);
                RemoteQueryBibliography * search = qobject_cast< RemoteQueryBibliography * >(qaim);
                if (model && !search) {
                    qaim->dropMimeData(data, action, -1, -1, QModelIndex());
                }
            }
        }
        return true;
    }

    QModelIndex LibraryModel::everything() const
    {
        return index(0, 0);
    }

    Qt::ItemFlags LibraryModel::flags(const QModelIndex & index) const
    {
        //qDebug() << "== flags" << index;
        quint32 id = index.internalId();
        if (id == _EVERYTHING) {
            return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
        } else if (id > _PLACEHOLDER_COUNT) {
            Qt::ItemFlags flags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

            flags |= Qt::ItemIsDropEnabled;

            if (index.isValid()) {
                if (index.column() == COLUMN_TITLE && index.parent() == collectionParentIndex()) {
                    flags |= Qt::ItemIsEditable;
                }
                flags |= Qt::ItemIsDragEnabled;
            }

            return flags;
        } else if (index == d->recentIndex()) {
            return Qt::ItemFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        } else {
            return Qt::ItemIsDropEnabled; // FIXME do the parents need to be droppable?
        }
    }

    static QString sanitise(QString input)
    {
        static QRegularExpression special("[^\\w\\p{Pd}\\p{Ps}\\p{Pe}\\p{Pi}\\p{Pf}\\p{Pc}]+", QRegularExpression::UseUnicodePropertiesOption);
        return input.replace(special, " ").replace(QRegExp("\\s+"), " ").trimmed();
    }

    QString LibraryModel::getObjectFilePath(CitationHandle citation, const QString & ext) const
    {
        // Generate a filename in the library path
        QString tpl;
        int tpl_idx = 1;
        QStringList tpl_params;
        bool found = false;

        // For that we need the first author's surname...
        QString surname;
        foreach (const QString & author, citation->field(Citation::AuthorsRole).toStringList()) {
            surname = author.section(',', 0, 0);
            if (!surname.isEmpty()) {
                break;
            }
        }
        if (surname.isEmpty()) {
            surname = "Unknown Author";
        } else {
            found = true;
        }
        tpl += QString("%%1 ").arg(tpl_idx++);
        tpl_params << surname;

        // ...the year in which it was published...
        QString year = citation->field(Citation::YearRole).toString();
        if (!year.isEmpty()) {
            tpl += QString("(%%1) ").arg(tpl_idx++);
            tpl_params << year;
            found = true;
        }

        // ...and the title.
        QString title = citation->field(Citation::TitleRole).toString();
        if (title.isEmpty()) {
            title = "Unknown Article";
        } else {
            found = true;
        }
        tpl += QString((tpl.isEmpty() ? "" : "- ") + QString("%%1")).arg(tpl_idx++);
        tpl_params << title;

        // If information has been found, use this new filename
        QString baseName;
        if (found) {
            baseName = tpl;
            foreach (const QString & param, tpl_params) {
                baseName = baseName.arg(sanitise(param));
            }
        } else {
            QString key = citation->field(Citation::KeyRole).toString();
            baseName = key;
        }

        // Concoct the actual filename
        QDir libraryDir(getObjectFileDir());
        return libraryDir.absoluteFilePath(baseName+ext);
    }

    static bool cd(QDir & dir, QString step)
    {
        return dir.cd(step) || (dir.mkdir(step) && dir.cd(step));
    }

    QDir LibraryModel::getObjectFileDir() const
    {
        QDir dir(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
        if (!cd(dir, "Articles")) {
            dir = QDir::home();
        }
        return QDir::cleanPath(dir.canonicalPath());
    }

    bool LibraryModel::hasObjectFile(CitationHandle citation, const QString & ext)
    {
        // Return the stored file path if present and exists
        QUrl path(citation->field(Citation::ObjectFileRole).toUrl());
        if (path.isLocalFile()) {
            return QFileInfo(path.toLocalFile()).exists();
        }
        return false;
    }

    QVariant LibraryModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        return QVariant();
    }

    QModelIndex LibraryModel::index(int row, int column, const QModelIndex & parent) const
    {
        if (!parent.isValid()) {
            switch (row) {
            case 0:
                return createIndex(row, column, _EVERYTHING);
            case 1:
                return createIndex(row, column, (void *) d->master);
            case 2:
                return createIndex(row, column, (void *) d->starred);
            case 3:
                return createIndex(row, column, (void *) d->recent);
            case 4:
                return createIndex(row, column, _HEADER_COLLECTIONS);
            case 5:
                return createIndex(row, column, _HEADER_SEARCHES);
            default:
                break;
            }
        } else if (parent == d->collectionParentIndex()) { // Collections
            if (row >= 0 && row < d->models.size()) {
                return createIndex(row, column, (void *) d->models.at(row));
            } else if (row == 0) {
                return createIndex(row, column, _NO_COLLECTIONS);
            }
        } else if (parent == d->searchParentIndex()) { // Searches
            if (row >= 0 && row < d->searches.size()) {
                return createIndex(row, column, (void *) d->searches.at(row));
            } else if (row == 0) {
                return createIndex(row, column, _NO_SEARCHES);
            }
        }

        return QModelIndex();
    }

    boost::shared_ptr< LibraryModel > LibraryModel::instance()
    {
        static boost::weak_ptr< LibraryModel > singleton;
        boost::shared_ptr< LibraryModel > shared(singleton.lock());
        if (singleton.expired())
        {
            shared = boost::shared_ptr< LibraryModel >(new LibraryModel());
            singleton = shared;
        }
        //static boost::shared_ptr< LibraryModel > shared(new LibraryModel());
        return shared;
    }

    void LibraryModel::insertModel(QAbstractItemModel * before, QAbstractItemModel * model)
    {
        QModelIndex parent(d->collectionParentIndex());

        if (d->noCollectionPlaceholder) {
            beginRemoveRows(parent, 0, 0);
            d->noCollectionPlaceholder = false;
            endRemoveRows();
        }

        int idx = 0;
        QList< QAbstractItemModel * >::iterator where(d->models.begin());
        while (where != d->models.end() && *where == before) { ++where; ++idx; }
        beginInsertRows(parent, idx, idx);
        d->models.insert(where, model);
        d->connectModel(model);
        d->updateMimeTypes();
        endInsertRows();
    }

    void LibraryModel::insertSearch(QAbstractItemModel * before, QAbstractItemModel * model)
    {
        RemoteQueryBibliography * remotemodel = qobject_cast< RemoteQueryBibliography * >(model);
        RemoteQueryBibliography * remotebefore = qobject_cast< RemoteQueryBibliography * >(before);
        if (remotemodel && remotebefore) {
            QList< RemoteQueryBibliography * >::iterator where(d->searches.begin());
            if (d->searches.isEmpty()) {
                beginInsertRows(QModelIndex(), 1, 1);
            } else {
                int idx = 0;
                while (where != d->searches.end() && *where == remotebefore) { ++where; ++idx; }
                beginInsertRows(d->searchParentIndex(), idx, idx);
            }
            d->searches.insert(where, remotemodel);
            d->connectModel(model);
            endInsertRows();
        }
    }

    bool LibraryModel::insertRows(int row, int count, const QModelIndex & parent)
    {
        return true;
    }

    Athenaeum::Bibliography * LibraryModel::master() const
    {
        return d->master;
    }

    QModelIndex LibraryModel::masterIndex() const
    {
        return d->masterIndex();
    }

    QModelIndex LibraryModel::recentIndex() const
    {
        return d->recentIndex();
    }

    QModelIndex LibraryModel::starredIndex() const
    {
        return d->starredIndex();
    }

    QMimeData * LibraryModel::mimeData(const QModelIndexList & indexes) const
    {
        QStringList modelIndexes;
        QString mimeType;

        foreach (const QModelIndex & index, indexes) {
            if (index.parent() == d->collectionParentIndex()) {
                mimeType = _INTERNAL_MIMETYPE_LIBRARYMODELS;
            } else if (index.parent() == d->searchParentIndex()) {
                mimeType = _INTERNAL_MIMETYPE_SEARCHMODELS;
            } else {
                continue;
            }
            quint32 id = index.internalId();
            if (id > _PLACEHOLDER_COUNT) {
                QAbstractItemModel * qaim = (QAbstractItemModel *) index.internalPointer();
                if (/* AbstractBibliography * model = */ qobject_cast< AbstractBibliography * >(qaim)) {
                    modelIndexes << QString::number(index.row());
                }
            }
        }

        if (!modelIndexes.isEmpty()) {
            QMimeData * mimeData = new QMimeData;
            mimeData->setData(mimeType, modelIndexes.join(" ").toUtf8());
            return mimeData;
        } else {
            return 0;
        }
    }

    QStringList LibraryModel::mimeTypes() const
    {
        QStringList types(d->mimeTypes);
        types << QLatin1String(_INTERNAL_MIMETYPE_LIBRARYMODELS);
        types << QLatin1String(_INTERNAL_MIMETYPE_SEARCHMODELS);
        return types;
    }

    QAbstractItemModel * LibraryModel::modelAt(int idx) const
    {
        return d->models.at(idx);
    }

    int LibraryModel::modelCount() const
    {
        return d->models.size();
    }

    QList< QAbstractItemModel * > LibraryModel::models() const
    {
        return d->models;
    }

    QModelIndex LibraryModel::newCollection(const QString & title)
    {
        if (d->master) {
            QDir dataRoot(Utopia::profile_path());
            if (dataRoot.cd("library") || (dataRoot.mkdir("library") && dataRoot.cd("library"))) {
                QDir collectionsDir(dataRoot);
                if (collectionsDir.cd("collections") || (collectionsDir.mkdir("collections") && collectionsDir.cd("collections"))) {
                    QString uuid = QUuid::createUuid().toString();
                    uuid = uuid.mid(1, uuid.size() - 2);
                    QDir collectionDir(collectionsDir);
                    if (collectionDir.cd(uuid) || (collectionDir.mkdir(uuid) && collectionDir.cd(uuid))) {
                        Collection * collection = new Collection(d->master, this);
                        Athenaeum::CollectionPersistenceModel * persistenceModel = new Athenaeum::CollectionPersistenceModel(collectionsDir.absoluteFilePath(uuid), collection);
                        collection->setPersistenceModel(persistenceModel);
                        if (!title.isEmpty()) {
                            collection->setTitle(title);
                        }
                        persistenceModel->save(collection);
                        appendModel(collection);
                        return index(d->models.size() - 1, 0, d->collectionParentIndex());
                    } else {
                        qDebug() << "=== Could not open collection directory";
                    }
                } else {
                    qDebug() << "=== Could not open collections directory";
                }
            } else {
                qDebug() << "=== Could not open data directory";
            }
        }

        return QModelIndex();
    }

    QModelIndex LibraryModel::parent(const QModelIndex & index) const
    {
        // Get this index's model
        switch (index.internalId()) {
        case _NO_COLLECTIONS:
            return d->collectionParentIndex();
        case _NO_SEARCHES:
            return d->searchParentIndex();
        case _EVERYTHING:
        case _HEADER_COLLECTIONS:
        case _HEADER_SEARCHES:
            return QModelIndex();
        default:
            QAbstractItemModel * qaim = (QAbstractItemModel *) index.internalPointer();
            if (qaim != d->master && qaim != d->starred && qaim != d->recent) {
                // What object do we have in the index?
                RemoteQueryBibliography * search = qobject_cast< RemoteQueryBibliography * >(qaim);
                //SortFilterProxyModel * filter = qobject_cast< SortFilterProxyModel * >(qaim);
                AbstractBibliography * model = qobject_cast< AbstractBibliography * >(qaim);

                if (search) {
                    return d->searchParentIndex();
                } else if (model) {
                    return d->collectionParentIndex();
                }
            }
            break;
        }

        // Otherwise no parent
        return QModelIndex();
    }

    QSortFilterProxyModel * LibraryModel::recent() const
    {
        return d->recent;
    }

    bool LibraryModel::removeModel(QAbstractItemModel * model)
    {
        int idx = d->models.indexOf(model);
        if (idx >= 0) {
            QModelIndex parent(d->collectionParentIndex());

            //int row = d->models.size();
            beginRemoveRows(parent, idx, idx);
            d->models.removeAt(idx);
            d->disconnectModel(model);
            AbstractBibliography * bibliography = dynamic_cast< AbstractBibliography * >(model);
            if (bibliography && bibliography->persistenceModel()) {
                bibliography->persistenceModel()->purge();
            }
            d->updateMimeTypes();
            endRemoveRows();

            if (d->models.isEmpty()) {
                beginInsertRows(parent, 0, 0);
                d->noCollectionPlaceholder = true;
                endInsertRows();
            }
        }

        return false;
    }

    bool LibraryModel::removeSearch(QAbstractItemModel * model)
    {
        // FIXME delete rows
        return false;
    }

    bool LibraryModel::removeRows(int row, int count, const QModelIndex & parent)
    {
        bool success = false;

        if (parent.isValid()) { // Not Headings
            if (parent == d->collectionParentIndex()) { // Models
                beginRemoveRows(parent, row, row + count - 1);
                for (int i = (row + count - 1); i >= row && i < d->models.size(); --i) {
                    //QAbstractItemModel * model = d->models.at(i);
                    d->models.removeAt(i); // FIXME deal with filters
                }
                d->updateMimeTypes();
                endRemoveRows();
                success = true;
            } else if (parent == d->searchParentIndex()) { // Searches
                beginRemoveRows(parent, row, row + count - 1);
                for (int i = (row + count - 1); i >= row && i < d->searches.size(); --i) {
                    //RemoteQueryBibliography * search = d->searches.at(i);
                    d->searches.removeAt(i); // FIXME deal with filters ?
                }
                endRemoveRows();
                success = true;
            }
        }

        return success;
    }

    ResolverQueue * LibraryModel::resolverQueue() const
    {
        return d->resolverQueue;
    }

    int LibraryModel::rowCount(const QModelIndex & index) const
    {
        if (!index.isValid()) { // Top level items in tree = everything + master + headings
            return 5;
        } else if (index == d->collectionParentIndex()) { // Models
            return d->noCollectionPlaceholder ? 1 : d->models.size();
        } else if (index == d->searchParentIndex()) { // Searches
            return d->noWatchPlaceholder ? 1 : d->searches.size();
        }

        return 0;
    }

    bool LibraryModel::saveObjectFile(CitationHandle citation, const QByteArray & data, const QString & ext) const
    {
        if (citation && !data.isEmpty()) {
            QString filePath = getObjectFilePath(citation, ext);
            QFile file(filePath);
            if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                file.write(data);
                file.close();
                citation->setField(Athenaeum::Citation::ObjectFileRole, QUrl::fromLocalFile(filePath));
                return true;
            }
        }
        return false;
    }

    QAbstractItemModel * LibraryModel::searchAt(int idx) const
    {
        return dynamic_cast< QAbstractItemModel * >(d->searches.at(idx));
    }

    int LibraryModel::searchCount() const
    {
        return d->searches.size();
    }

    QList< QAbstractItemModel * > LibraryModel::searches() const
    {
        QList< QAbstractItemModel * > searches;
        foreach (RemoteQueryBibliography * remote, d->searches) {
            searches << dynamic_cast< QAbstractItemModel * >(remote);
        }
        return searches;
    }

    QModelIndex LibraryModel::searchIndex() const
    {
        return everything();
    }

    QModelIndex LibraryModel::searchParentIndex() const
    {
        return d->searchParentIndex();
    }

    bool LibraryModel::setData(const QModelIndex & index, const QVariant & value, int role)
    {
        // Without a model, there's nothing to return
        quint32 id = index.internalId();
        if (role == Qt::EditRole && !value.toString().isEmpty() && id > _PLACEHOLDER_COUNT) {
            QAbstractItemModel * qaim = (QAbstractItemModel *) index.internalPointer();
            if (qaim) {
                return qaim->setProperty("title", value);
            }
        }

        return false;
    }

    QSortFilterProxyModel * LibraryModel::starred() const
    {
        return d->starred;
    }

} // namespace Athenaeum
