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

#include <papyro/aggregatingproxymodel.h>
#include <papyro/aggregatingproxymodel_p.h>
#include <papyro/bibliographicmimedata_p.h>

#include <QItemSelection>
#include <QStringList>

#include <QDebug>

namespace Athenaeum
{

    AggregatingProxyModelPrivate::AggregatingProxyModelPrivate(AggregatingProxyModel * proxyModel, Qt::Orientation orientation)
        : QObject(proxyModel), proxyModel(proxyModel), orientation(orientation), aggregatedLength(0), maximumWidth(0)
    {}

    void AggregatingProxyModelPrivate::appendSourceModel(QAbstractItemModel * sourceModel)
    {
        if (!sourceModels.contains(sourceModel)) {
            // Add source model to list
            sourceModels.append(sourceModel);

            // Recalculate necessary values
            sourceModelIndexMap[aggregatedLength] = sourceModel;
            calculateIndexMap();
            calculateMaximumWidth();

            // connect up signals & slots
            connect(sourceModel, SIGNAL(columnsAboutToBeInserted(const QModelIndex &, int, int)),
                    this, SLOT(on_columnsAboutToBeInserted(const QModelIndex &, int, int)));
            connect(sourceModel, SIGNAL(columnsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
                    this, SLOT(on_columnsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
            connect(sourceModel, SIGNAL(columnsAboutToBeRemoved(const QModelIndex &, int, int)),
                    this, SLOT(on_columnsAboutToBeRemoved(const QModelIndex &, int, int)));
            connect(sourceModel, SIGNAL(columnsInserted(const QModelIndex &, int, int)),
                    this, SLOT(on_columnsInserted(const QModelIndex &, int, int)));
            connect(sourceModel, SIGNAL(columnsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
                    this, SLOT(on_columnsMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
            connect(sourceModel, SIGNAL(columnsRemoved(const QModelIndex &, int, int)),
                    this, SLOT(on_columnsRemoved(const QModelIndex &, int, int)));
            connect(sourceModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &, const QVector< int >)),
                    this, SLOT(on_dataChanged(const QModelIndex &, const QModelIndex &, const QVector< int >)));
            connect(sourceModel, SIGNAL(headerDataChanged(Qt::Orientation, int, int)),
                    this, SLOT(on_headerDataChanged(Qt::Orientation, int, int)));
            connect(sourceModel, SIGNAL(layoutAboutToBeChanged()),
                    this, SLOT(on_layoutAboutToBeChanged()));
            connect(sourceModel, SIGNAL(layoutChanged()),
                    this, SLOT(on_layoutChanged()));
            connect(sourceModel, SIGNAL(modelAboutToBeReset()),
                    this, SLOT(on_modelAboutToBeReset()));
            connect(sourceModel, SIGNAL(modelReset()),
                    this, SLOT(on_modelReset()));
            connect(sourceModel, SIGNAL(rowsAboutToBeInserted(const QModelIndex &, int, int)),
                    this, SLOT(on_rowsAboutToBeInserted(const QModelIndex &, int, int)));
            connect(sourceModel, SIGNAL(rowsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
                    this, SLOT(on_rowsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
            connect(sourceModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)),
                    this, SLOT(on_rowsAboutToBeRemoved(const QModelIndex &, int, int)));
            connect(sourceModel, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
                    this, SLOT(on_rowsInserted(const QModelIndex &, int, int)));
            connect(sourceModel, SIGNAL(rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
                    this, SLOT(on_rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
            connect(sourceModel, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
                    this, SLOT(on_rowsRemoved(const QModelIndex &, int, int)));
        }
    }

    void AggregatingProxyModelPrivate::calculateIndexMap()
    {
        int oldLength = aggregatedLength;
        aggregatedLength = 0;
        sourceModelIndexMap.clear();
        foreach (QAbstractItemModel * sourceModel, sourceModels) {
            sourceModelIndexMap[aggregatedLength] = sourceModel;
            aggregatedLength += length(sourceModel);
        }
        if (oldLength != aggregatedLength) {
            emit layoutChanged();
        }
    }

    void AggregatingProxyModelPrivate::calculateMaximumWidth()
    {
        int max = 0;
        foreach (QAbstractItemModel * sourceModel, sourceModels) {
            int modelWidth = width(sourceModel);
            max = qMax(max, modelWidth);
        }
        if (max != maximumWidth) {
            maximumWidth = max;
            emit layoutChanged();
        }
    }

    int AggregatingProxyModelPrivate::length(QAbstractItemModel * sourceModel)
    {
        if (sourceModel) {
            return (orientation == Qt::Vertical) ? sourceModel->rowCount() : sourceModel->columnCount();
        } else {
            return aggregatedLength;
        }
    }

    int AggregatingProxyModelPrivate::lengthwiseIndex(const QModelIndex & index) const
    {
        return (orientation == Qt::Vertical) ? index.row() : index.column();
    }

    QModelIndex AggregatingProxyModelPrivate::mapFromSource(const QModelIndex & sourceIndex) const
    {
        if (!sourceIndex.isValid()) { return QModelIndex(); }
        if (sourceIndex.parent().isValid()) { return sourceIndex; }
        int mapped = sourceModelIndexMap.key(sourceIndex.model()) + lengthwiseIndex(sourceIndex);
        return proxyModel->index(mapped, widthwiseIndex(sourceIndex));
    }

    int AggregatingProxyModelPrivate::mapFromSourceColumn(QAbstractItemModel * sourceModel, int column) const
    {
        return (orientation == Qt::Vertical) ? column : (sourceModelIndexMap.key(sourceModel) + column);
    }

    int AggregatingProxyModelPrivate::mapFromSourceRow(QAbstractItemModel * sourceModel, int row) const
    {
        return (orientation == Qt::Vertical) ? (sourceModelIndexMap.key(sourceModel) + row) : row;
    }

    QItemSelection AggregatingProxyModelPrivate::mapSelectionFromSource(const QItemSelection & sourceSelection) const
    {
        return QItemSelection();
    }

    QItemSelection AggregatingProxyModelPrivate::mapSelectionToSource(const QItemSelection & proxySelection) const
    {
        return QItemSelection();
    }

    QModelIndex AggregatingProxyModelPrivate::mapToSource(const QModelIndex & proxyIndex) const
    {
        if (!proxyIndex.isValid()) { return QModelIndex(); }
        if (proxyIndex.parent().isValid()) { return proxyIndex; }
        int proxyLengthwiseIndex = lengthwiseIndex(proxyIndex);
        QMap< int, const QAbstractItemModel * >::const_iterator upperBound = sourceModelIndexMap.upperBound(proxyLengthwiseIndex);
        if (upperBound != sourceModelIndexMap.begin()) {
            --upperBound;
            int offset = upperBound.key();
            const QAbstractItemModel * sourceModel = upperBound.value();
            return sourceModel->index(proxyLengthwiseIndex - offset, widthwiseIndex(proxyIndex));
        } else {
            return QModelIndex();
        }
    }

    void AggregatingProxyModelPrivate::on_columnsAboutToBeInserted(const QModelIndex & parent, int start, int end)
    {}

    void AggregatingProxyModelPrivate::on_columnsAboutToBeMoved(const QModelIndex & sourceParent, int sourceStart, int sourceEnd, const QModelIndex & destinationParent, int destinationColumn)
    {}

    void AggregatingProxyModelPrivate::on_columnsAboutToBeRemoved(const QModelIndex & parent, int start, int end)
    {}

    void AggregatingProxyModelPrivate::on_columnsInserted(const QModelIndex & parent, int start, int end)
    {}

    void AggregatingProxyModelPrivate::on_columnsMoved(const QModelIndex & sourceParent, int sourceStart, int sourceEnd, const QModelIndex & destinationParent, int destinationColumn)
    {}

    void AggregatingProxyModelPrivate::on_columnsRemoved(const QModelIndex & parent, int start, int end)
    {}

    void AggregatingProxyModelPrivate::on_dataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight, const QVector< int > & roles)
    {
        if ( /*QAbstractItemModel * sourceModel = */ qobject_cast< QAbstractItemModel * >(sender())) {
            emit dataChanged(mapFromSource(topLeft),
                             mapFromSource(bottomRight),
                             roles);
        }
    }

    void AggregatingProxyModelPrivate::on_headerDataChanged(Qt::Orientation orientation, int first, int last)
    {}

    void AggregatingProxyModelPrivate::on_layoutAboutToBeChanged()
    {
        emit layoutAboutToBeChanged();
    }

    void AggregatingProxyModelPrivate::on_layoutChanged()
    {
        emit layoutChanged();
    }

    void AggregatingProxyModelPrivate::on_modelAboutToBeReset()
    {
        emit modelAboutToBeReset();
    }

    void AggregatingProxyModelPrivate::on_modelReset()
    {
        emit modelReset();
    }

    void AggregatingProxyModelPrivate::on_rowsAboutToBeInserted(const QModelIndex & parent, int start, int end)
    {
        if (QAbstractItemModel * sourceModel = qobject_cast< QAbstractItemModel * >(sender())) {
            emit rowsAboutToBeInserted(mapFromSource(parent),
                                       mapFromSourceRow(sourceModel, start),
                                       mapFromSourceRow(sourceModel, end));
        }
    }

    void AggregatingProxyModelPrivate::on_rowsAboutToBeMoved(const QModelIndex & sourceParent, int sourceStart, int sourceEnd, const QModelIndex & destinationParent, int destinationRow)
    {}

    void AggregatingProxyModelPrivate::on_rowsAboutToBeRemoved(const QModelIndex & parent, int start, int end)
    {
        if (QAbstractItemModel * sourceModel = qobject_cast< QAbstractItemModel * >(sender())) {
            emit rowsAboutToBeRemoved(mapFromSource(parent),
                                      mapFromSourceRow(sourceModel, start),
                                      mapFromSourceRow(sourceModel, end));
        }
    }

    void AggregatingProxyModelPrivate::on_rowsInserted(const QModelIndex & parent, int start, int end)
    {
        if (QAbstractItemModel * sourceModel = qobject_cast< QAbstractItemModel * >(sender())) {
            calculateIndexMap();
            emit rowsInserted(mapFromSource(parent),
                              mapFromSourceRow(sourceModel, start),
                              mapFromSourceRow(sourceModel, end));
        }
    }

    void AggregatingProxyModelPrivate::on_rowsMoved(const QModelIndex & sourceParent, int sourceStart, int sourceEnd, const QModelIndex & destinationParent, int destinationRow)
    {}

    void AggregatingProxyModelPrivate::on_rowsRemoved(const QModelIndex & parent, int start, int end)
    {
        if (QAbstractItemModel * sourceModel = qobject_cast< QAbstractItemModel * >(sender())) {
            calculateIndexMap();
            emit rowsRemoved(mapFromSource(parent),
                             mapFromSourceRow(sourceModel, start),
                             mapFromSourceRow(sourceModel, end));
        }
    }

    void AggregatingProxyModelPrivate::removeSourceModel(QAbstractItemModel * sourceModel)
    {
        if (sourceModels.removeOne(sourceModel)) {
            sourceModel->disconnect(this);
        }
    }

    int AggregatingProxyModelPrivate::width(QAbstractItemModel * sourceModel)
    {
        if (sourceModel) {
            return (orientation == Qt::Vertical) ? sourceModel->columnCount() : sourceModel->rowCount();
        } else {
            return maximumWidth ;
        }
    }

    int AggregatingProxyModelPrivate::widthwiseIndex(const QModelIndex & index) const
    {
        return (orientation == Qt::Vertical) ? index.column() : index.row();
    }




    AggregatingProxyModel::AggregatingProxyModel(Qt::Orientation orientation, QObject * parent)
        : QAbstractItemModel(parent), d(new AggregatingProxyModelPrivate(this, orientation))
    {

        // connect up signals
        connect(d, SIGNAL(columnsAboutToBeInserted(const QModelIndex &, int, int)),
                this, SIGNAL(columnsAboutToBeInserted(const QModelIndex &, int, int)));
        connect(d, SIGNAL(columnsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
                this, SIGNAL(columnsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
        connect(d, SIGNAL(columnsAboutToBeRemoved(const QModelIndex &, int, int)),
                this, SIGNAL(columnsAboutToBeRemoved(const QModelIndex &, int, int)));
        connect(d, SIGNAL(columnsInserted(const QModelIndex &, int, int)),
                this, SIGNAL(columnsInserted(const QModelIndex &, int, int)));
        connect(d, SIGNAL(columnsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
                this, SIGNAL(columnsMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
        connect(d, SIGNAL(columnsRemoved(const QModelIndex &, int, int)),
                this, SIGNAL(columnsRemoved(const QModelIndex &, int, int)));
        connect(d, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &, const QVector< int >)),
                this, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &, const QVector< int >)));
        connect(d, SIGNAL(headerDataChanged(Qt::Orientation, int, int)),
                this, SIGNAL(headerDataChanged(Qt::Orientation, int, int)));
        connect(d, SIGNAL(layoutAboutToBeChanged()),
                this, SIGNAL(layoutAboutToBeChanged()));
        connect(d, SIGNAL(layoutChanged()),
                this, SIGNAL(layoutChanged()));
        connect(d, SIGNAL(modelAboutToBeReset()),
                this, SIGNAL(modelAboutToBeReset()));
        connect(d, SIGNAL(modelReset()),
                this, SIGNAL(modelReset()));
        connect(d, SIGNAL(rowsAboutToBeInserted(const QModelIndex &, int, int)),
                this, SIGNAL(rowsAboutToBeInserted(const QModelIndex &, int, int)));
        connect(d, SIGNAL(rowsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
                this, SIGNAL(rowsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
        connect(d, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)),
                this, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)));
        connect(d, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
                this, SIGNAL(rowsInserted(const QModelIndex &, int, int)));
        connect(d, SIGNAL(rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
                this, SIGNAL(rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
        connect(d, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
                this, SIGNAL(rowsRemoved(const QModelIndex &, int, int)));
    }

    AggregatingProxyModel::~AggregatingProxyModel()
    {}

    void AggregatingProxyModel::appendSourceModel(QAbstractItemModel * sourceModel)
    {
        d->appendSourceModel(sourceModel);
    }

    void AggregatingProxyModel::clear()
    {
        foreach (QAbstractItemModel * sourceModel, d->sourceModels) {
            removeSourceModel(sourceModel);
        }
        d->aggregatedLength = 0;
        d->maximumWidth = 0;
        d->sourceModels.clear();
        d->sourceModelIndexMap.clear();
    }

    int AggregatingProxyModel::columnCount(const QModelIndex & index) const
    {
        return (d->orientation == Qt::Vertical) ? d->width() : d->length();
    }

    QVariant AggregatingProxyModel::data(const QModelIndex & index, int role) const
    {
        return d->mapToSource(index).data(role);
    }

    Qt::ItemFlags AggregatingProxyModel::flags(const QModelIndex & index) const
    {
        return d->mapToSource(index).flags();
    }

    QVariant AggregatingProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        return QVariant();
    }

    QModelIndex AggregatingProxyModel::index(int row, int column, const QModelIndex & parent) const
    {
        if (parent.isValid()) { return QModelIndex(); }
        return createIndex(row, column);
    }

    bool AggregatingProxyModel::insertRows(int row, int count, const QModelIndex & parent)
    {
        return false;
    }

    QModelIndex AggregatingProxyModel::mapFromSource(const QModelIndex & sourceIndex) const
    {
        return d->mapFromSource(sourceIndex);
    }

    QItemSelection AggregatingProxyModel::mapSelectionFromSource(const QItemSelection & sourceSelection) const
    {
        return d->mapSelectionFromSource(sourceSelection);
    }

    QItemSelection AggregatingProxyModel::mapSelectionToSource(const QItemSelection & proxySelection) const
    {
        return d->mapSelectionToSource(proxySelection);
    }

    QModelIndex AggregatingProxyModel::mapToSource(const QModelIndex & proxyIndex) const
    {
        return d->mapToSource(proxyIndex);
    }

    QMimeData * AggregatingProxyModel::mimeData(const QModelIndexList & indexes) const
    {
        if (!indexes.isEmpty()) {
            QMimeData * mimeData = new BibliographicMimeData(indexes);
            mimeData->setData(_INTERNAL_MIMETYPE_CITATIONS, QByteArray());
            return mimeData;
        } else {
            return 0;
        }
    }

    QStringList AggregatingProxyModel::mimeTypes() const
    {
        QStringList types;
        foreach (QAbstractItemModel * sourceModel, d->sourceModels) {
            types += sourceModel->mimeTypes();
        }
        types.removeDuplicates();
        return types;
    }

    QModelIndex AggregatingProxyModel::parent(const QModelIndex & index) const
    {
        return QModelIndex();
    }

    bool AggregatingProxyModel::removeRows(int row, int count, const QModelIndex & parent)
    {
        return false;
    }

    void AggregatingProxyModel::removeSourceModel(QAbstractItemModel * sourceModel)
    {
        d->removeSourceModel(sourceModel);
    }

    int AggregatingProxyModel::rowCount(const QModelIndex & index) const
    {
        return (d->orientation == Qt::Vertical) ? d->length() : d->width();
    }

    bool AggregatingProxyModel::setData(const QModelIndex & index, const QVariant & value, int role)
    {
        QModelIndex sourceIndex(d->mapToSource(index));
        return const_cast< QAbstractItemModel * >(sourceIndex.model())->setData(sourceIndex, value, role);
    }

} // namespace Athenaeum
