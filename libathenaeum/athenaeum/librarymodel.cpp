/*****************************************************************************
 *  
 *   This file is part of the Utopia Documents application.
 *       Copyright (c) 2008-2014 Lost Island Labs
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

#include <athenaeum/librarymodel.h>
#include <athenaeum/librarymodel_p.h>
#include <athenaeum/remotequerybibliographicmodel.h>
#include <athenaeum/sortfilterproxymodel.h>
#include <athenaeum/cJSON.h>

#include <QMimeData>
#include <QPixmap>
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

    } // Anonymous namespace




    LibraryModelPrivate::LibraryModelPrivate(LibraryModel * model)
        : QObject(model), m(model)
    {}

    LibraryModelPrivate::~LibraryModelPrivate()
    {}

    void LibraryModelPrivate::connectModel(AbstractBibliographicModel * model)
    {
        connect(model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
                this, SLOT(onDataChanged(const QModelIndex &, const QModelIndex &)));
        connect(model, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
                this, SLOT(onRowsInserted(const QModelIndex &, int, int)));
        connect(model, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
                this, SLOT(onRowsRemoved(const QModelIndex &, int, int)));
        connect(model, SIGNAL(stateChanged(Athenaeum::AbstractBibliographicCollection::State)),
                this, SLOT(onStateChanged(Athenaeum::AbstractBibliographicCollection::State)));
        connect(model, SIGNAL(titleChanged(const QString &)),
                this, SLOT(onTitleChanged(const QString &)));
    }

    void LibraryModelPrivate::disconnectModel(AbstractBibliographicModel * model)
    {
        disconnect(model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
                   this, SLOT(onDataChanged(const QModelIndex &, const QModelIndex &)));
        disconnect(model, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
                   this, SLOT(onRowsInserted(const QModelIndex &, int, int)));
        disconnect(model, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
                   this, SLOT(onRowsRemoved(const QModelIndex &, int, int)));
        disconnect(model, SIGNAL(stateChanged(Athenaeum::AbstractBibliographicCollection::State)),
                   this, SLOT(onStateChanged(Athenaeum::AbstractBibliographicCollection::State)));
        disconnect(model, SIGNAL(titleChanged(const QString &)),
                   this, SLOT(onTitleChanged(const QString &)));
    }

    QModelIndex LibraryModelPrivate::modelParentIndex() const
    {
        return m->index(0, 0);
    }

    void LibraryModelPrivate::onDataChanged(const QModelIndex &, const QModelIndex &)
    {
        relayDataChanged();
    }

    void LibraryModelPrivate::onRowsInserted(const QModelIndex &, int, int)
    {
        relayDataChanged();
    }

    void LibraryModelPrivate::onRowsRemoved(const QModelIndex &, int, int)
    {
        relayDataChanged();
    }

    void LibraryModelPrivate::onStateChanged(Athenaeum::AbstractBibliographicCollection::State state)
    {
        relayDataChanged();
    }

    void LibraryModelPrivate::onTitleChanged(const QString & title)
    {
        relayDataChanged();
    }

    void LibraryModelPrivate::relayDataChanged()
    {
        emit dataChanged(modelParentIndex(), searchParentIndex());
    }

    QModelIndex LibraryModelPrivate::searchParentIndex() const
    {
        return m->index(1, 0);
    }

    void LibraryModelPrivate::updateMimeTypes()
    {
        // Iterate over the managed models and accumulate a list of accepted mime types
        QSet< QString > types;
        foreach (AbstractBibliographicModel * model, models) {
            types.unite(QSet< QString >::fromList(model->mimeTypes()));
        }
        mimeTypes = types.toList();
    }




    LibraryModel::LibraryModel(QObject * parent)
        : QAbstractItemModel(parent), d(new LibraryModelPrivate(this))
    {
        setSupportedDragActions(Qt::CopyAction | Qt::MoveAction);

        connect(d, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
                this, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)));
    }

    LibraryModel::~LibraryModel()
    {}

    static bool canDecode(QAbstractItemModel * model, const QMimeData * mimeData)
    {
        QStringList modelTypes = model->mimeTypes();
        for (int i = 0; i < modelTypes.count(); ++i) {
            if (mimeData->hasFormat(modelTypes.at(i))) {
                return true;
            }
        }
        return false;
    }

    bool LibraryModel::acceptsDrop(const QModelIndex & index, bool child, const QMimeData * mimeData)
    {
        // Check for internal drop operations
        if (mimeData->hasFormat(_INTERNAL_MIMETYPE_LIBRARYMODELS)) {
            return (child && index == d->modelParentIndex());
        } else if (mimeData->hasFormat(_INTERNAL_MIMETYPE_SEARCHMODELS)) {
            return (child && index == d->searchParentIndex());
        } else if (!child) {
            // Check if the underlying model accepts the mimeData
            QAbstractItemModel * qaim = (QAbstractItemModel *) index.internalPointer();
            AbstractBibliographicModel * model = qobject_cast< AbstractBibliographicModel * >(qaim);
            RemoteQueryBibliographicModel * search = qobject_cast< RemoteQueryBibliographicModel * >(qaim);
            return model && !search && model->acceptsDrop(mimeData);
        }

        return false;
    }

    void LibraryModel::appendModel(AbstractBibliographicModel * model)
    {
        QModelIndex parent(d->modelParentIndex());
        int row(rowCount(parent));
        beginInsertRows(parent, row, row);
        d->models.append(model);
        d->connectModel(model);
        d->updateMimeTypes();
        endInsertRows();
    }

    void LibraryModel::appendSearch(RemoteQueryBibliographicModel * model)
    {
        if (d->searches.isEmpty()) {
            beginInsertRows(QModelIndex(), 1, 1);
        } else {
            QModelIndex parent(d->searchParentIndex());
            int row(rowCount(parent));
            beginInsertRows(parent, row, row);
        }
        d->searches.append(model);
        d->connectModel(model);
        endInsertRows();
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

        if (index == d->modelParentIndex()) {
            if (role == Qt::DisplayRole) {
                return QString("LIBRARIES");
            }
        } else if (index == d->searchParentIndex()) {
            if (role == Qt::DisplayRole) {
                return QString("SEARCHES");
            }
        } else {
            QAbstractItemModel * qaim = (QAbstractItemModel *) index.internalPointer();
            AbstractBibliographicCollection * collection = qobject_cast< AbstractBibliographicCollection * >(qaim);
            AbstractBibliographicModel * model = qobject_cast< AbstractBibliographicModel * >(qaim);
            RemoteQueryBibliographicModel * search = qobject_cast< RemoteQueryBibliographicModel * >(qaim);

            // Without a model, there's nothing to return
            if (collection) {
                switch (role) {
                case Qt::DecorationRole:
                    if (index.column() == 0) {
                        if (search) {
                            return iconSearch;
                        } else if (model) {
                            return iconLibrary;
                        } else {
                            return iconFilter;
                        }
                    }
                    break;
                case Qt::EditRole:
                case Qt::DisplayRole:
                    // Post processing of values
                    switch (index.column()) {
                    case COLUMN_TITLE:
                        return collection->title();
                    case COLUMN_ITEM_COUNT:
                        return collection->count(AbstractBibliographicModel::AllFlags);
                    case COLUMN_UNREAD_ITEM_COUNT:
                        return collection->count(AbstractBibliographicModel::UnreadFlag);
                    case COLUMN_IMPORTANT_ITEM_COUNT:
                        return collection->count(AbstractBibliographicModel::ImportantFlag);
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
                    return collection->title();
                case StateRole:
                    return QVariant::fromValue(collection->state());
                case ItemCountRole:
                    return collection->count(AbstractBibliographicModel::AllFlags);
                case UnreadItemCountRole:
                    return collection->count(AbstractBibliographicModel::UnreadFlag);
                case ImportantItemCountRole:
                    return collection->count(AbstractBibliographicModel::ImportantFlag);
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
        if (parent == d->modelParentIndex() && data->hasFormat(_INTERNAL_MIMETYPE_LIBRARYMODELS)) {
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
            QAbstractItemModel * qaim = (QAbstractItemModel *) index(row, column, parent).internalPointer();
            AbstractBibliographicModel * model = qobject_cast< AbstractBibliographicModel * >(qaim);
            RemoteQueryBibliographicModel * search = qobject_cast< RemoteQueryBibliographicModel * >(qaim);
            if (model && !search) {
                qaim->dropMimeData(data, action, -1, -1, QModelIndex());
            }
        }
        return true;
    }

    Qt::ItemFlags LibraryModel::flags(const QModelIndex & index) const
    {
        //qDebug() << "== flags" << index;
        if (index.parent().isValid()) {
            Qt::ItemFlags flags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

            flags |= Qt::ItemIsDropEnabled;

            if (index.isValid()) {
                if (index.column() == COLUMN_TITLE) {
                    flags |= Qt::ItemIsEditable;
                }
                flags |= Qt::ItemIsDragEnabled;
            }

            //qDebug() << "== flags" << flags;
            return flags;
        } else {
            return Qt::ItemIsDropEnabled; // FIXME do the parents need to be droppable?
        }
    }

    QVariant LibraryModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        return QVariant();
    }

    QModelIndex LibraryModel::index(int row, int column, const QModelIndex & parent) const
    {
        if (!parent.isValid()) { // Headings
            if (row >= 0 && row <= 1) {
                return createIndex(row, column);
            }
        } else if (parent == d->modelParentIndex()) { // Models
            if (row >= 0 && row < d->models.size()) {
                return createIndex(row, column, (void *) d->models.at(row));
            }
        } else if (parent == d->searchParentIndex()) { // Searches
            if (row >= 0 && row < d->searches.size()) {
                return createIndex(row, column, (void *) d->searches.at(row));
            }
        } else if (parent.parent() == d->modelParentIndex()) { // Filters on models
            if (parent.row() >= 0 && parent.row() < d->models.size()) {
                AbstractBibliographicModel * model = d->models.at(parent.row());
                if (d->filters.contains(model) && row >= 0 && row < d->filters[model].size()) {
                    return createIndex(row, column, (void *) d->filters[model].at(row));
                }
            }
        }

        return QModelIndex();
    }

    void LibraryModel::insertModel(AbstractBibliographicModel * before, AbstractBibliographicModel * model)
    {
        int idx = 0;
        QList< AbstractBibliographicModel * >::iterator where(d->models.begin());
        while (where != d->models.end() && *where == before) { ++where; ++idx; }
        beginInsertRows(d->modelParentIndex(), idx, idx);
        d->models.insert(where, model);
        d->connectModel(model);
        d->updateMimeTypes();
        endInsertRows();
    }

    void LibraryModel::insertSearch(RemoteQueryBibliographicModel * before, RemoteQueryBibliographicModel * model)
    {
        QList< RemoteQueryBibliographicModel * >::iterator where(d->searches.begin());
        if (d->searches.isEmpty()) {
            beginInsertRows(QModelIndex(), 1, 1);
        } else {
            int idx = 0;
            while (where != d->searches.end() && *where == before) { ++where; ++idx; }
            beginInsertRows(d->searchParentIndex(), idx, idx);
        }
        d->searches.insert(where, model);
        d->connectModel(model);
        endInsertRows();
    }

    bool LibraryModel::insertRows(int row, int count, const QModelIndex & parent)
    {
        return true;
    }

    QMimeData * LibraryModel::mimeData(const QModelIndexList & indexes) const
    {
        QStringList modelIndexes;
        QString mimeType;

        foreach (const QModelIndex & index, indexes) {
            if (index.parent() == d->modelParentIndex()) {
                mimeType = _INTERNAL_MIMETYPE_LIBRARYMODELS;
            } else if (index.parent() == d->searchParentIndex()) {
                mimeType = _INTERNAL_MIMETYPE_SEARCHMODELS;
            } else {
                continue;
            }
            QAbstractItemModel * qaim = (QAbstractItemModel *) index.internalPointer();
            if (AbstractBibliographicModel * model = qobject_cast< AbstractBibliographicModel * >(qaim)) {
                modelIndexes << QString::number(index.row());
            }
        }

        if (!modelIndexes.isEmpty()) {
            QMimeData * mimeData = new QMimeData;
            mimeData->setData(mimeType, modelIndexes.join(" ").toAscii());
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

    AbstractBibliographicModel * LibraryModel::modelAt(int idx) const
    {
        return d->models.at(idx);
    }

    int LibraryModel::modelCount() const
    {
        return d->models.size();
    }

    QList< AbstractBibliographicModel * > LibraryModel::models() const
    {
        return d->models;
    }

    QModelIndex LibraryModel::parent(const QModelIndex & index) const
    {
        // Get this index's model
        QAbstractItemModel * qaim = (QAbstractItemModel *) index.internalPointer();
        if (!qaim) { // Headings
            return QModelIndex();
        } else {
            // What object do we have in the index?
            RemoteQueryBibliographicModel * search = qobject_cast< RemoteQueryBibliographicModel * >(qaim);
            SortFilterProxyModel * filter = qobject_cast< SortFilterProxyModel * >(qaim);
            AbstractBibliographicModel * model = qobject_cast< AbstractBibliographicModel * >(qaim);

            if (search) {
                return d->searchParentIndex();
            } else if (model) {
                return d->modelParentIndex();
            } else if (filter) {
                QMapIterator< AbstractBibliographicModel *, QList< SortFilterProxyModel * > > iter(d->filters);
                AbstractBibliographicModel * sourceModel = 0;
                while (iter.hasNext()) {
                    iter.next();
                    if (iter.value().contains(filter)) {
                        sourceModel = iter.key();
                        break;
                    }
                }
                if (sourceModel) {
                    int row = d->models.indexOf(sourceModel);
                    if (row >= 0) {
                        return createIndex(row, 0, (void *) sourceModel);
                    }
                }
            }
        }

        // Otherwise no parent
        return QModelIndex();
    }

    bool LibraryModel::removeModel(AbstractBibliographicModel * model)
    {
        // FIXME delete rows
        d->updateMimeTypes();
        return false;
    }

    bool LibraryModel::removeSearch(RemoteQueryBibliographicModel * model)
    {
        // FIXME delete rows
        return false;
    }

    bool LibraryModel::removeRows(int row, int count, const QModelIndex & parent)
    {
        bool success = false;

        if (parent.isValid()) { // Not Headings
            if (parent == d->modelParentIndex()) { // Models
                beginRemoveRows(parent, row, row + count - 1);
                for (int i = (row + count - 1); i >= row && i < d->models.size(); --i) {
                    AbstractBibliographicModel * model = d->models.at(i);
                    d->models.removeAt(i); // FIXME deal with filters
                }
                d->updateMimeTypes();
                endRemoveRows();
                success = true;
            } else if (parent == d->searchParentIndex()) { // Searches
                beginRemoveRows(parent, row, row + count - 1);
                for (int i = (row + count - 1); i >= row && i < d->searches.size(); --i) {
                    RemoteQueryBibliographicModel * search = d->searches.at(i);
                    d->searches.removeAt(i); // FIXME deal with filters ?
                }
                endRemoveRows();
                success = true;
            } else { // Filters
                if (parent.row() >= 0 && parent.row() < d->models.size()) {
                    AbstractBibliographicModel * model = d->models.at(parent.row());
                    if (d->filters.contains(model)) {
                        beginRemoveRows(parent, row, row + count - 1);
                        for (int i = (i + count - 1); i >= row && i < d->filters[model].size(); --i) {
                            SortFilterProxyModel * filter = d->filters[model].at(i);
                            d->filters[model].removeAt(i);
                        }
                        endRemoveRows();
                        success = true;
                    }
                }
            }
        }

        return success;
    }

    int LibraryModel::rowCount(const QModelIndex & index) const
    {
        if (!index.isValid()) { // Headings
            return d->searches.isEmpty() ? 1 : 2; // models + searches
        } else if (index == d->modelParentIndex()) { // Models
            return d->models.size();
        } else if (index == d->searchParentIndex()) { // Searches
            return d->searches.size();
        } else { // Filters
            if (index.row() >= 0 && index.row() < d->models.size()) {
                AbstractBibliographicModel * model = d->models.at(index.row());
                if (d->filters.contains(model)) {
                    return d->filters[model].size();
                }
            }
        }

        return 0;
    }

    RemoteQueryBibliographicModel * LibraryModel::searchAt(int idx) const
    {
        return d->searches.at(idx);
    }

    int LibraryModel::searchCount() const
    {
        return d->searches.size();
    }

    QList< RemoteQueryBibliographicModel * > LibraryModel::searches() const
    {
        return d->searches;
    }

    bool LibraryModel::setData(const QModelIndex & index, const QVariant & value, int role)
    {
        // Without a model, there's nothing to return
        if (role == Qt::EditRole && !value.toString().isEmpty()) {
            QAbstractItemModel * qaim = (QAbstractItemModel *) index.internalPointer();
            AbstractBibliographicCollection * collection = qobject_cast< AbstractBibliographicCollection * >(qaim);
            if (collection) {
                collection->setTitle(value.toString());
                return true;
            }
        }

        return false;
    }

} // namespace Athenaeum
