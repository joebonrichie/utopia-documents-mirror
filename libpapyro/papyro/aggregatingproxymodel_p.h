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

#ifndef ATHENAEUM_AGGREGATINGPROXYMODEL_P_H
#define ATHENAEUM_AGGREGATINGPROXYMODEL_P_H

#include <QList>
#include <QMap>
#include <QObject>

class QAbstractItemModel;
class QItemSelection;
class QModelIndex;

namespace Athenaeum
{

    class AggregatingProxyModel;
    class AggregatingProxyModelPrivate : public QObject
    {
        Q_OBJECT

    public:
        AggregatingProxyModelPrivate(AggregatingProxyModel * parent, Qt::Orientation orientation);

        void appendSourceModel(QAbstractItemModel * sourceModel);
        void calculateIndexMap();
        void calculateMaximumWidth();
        int length(QAbstractItemModel * sourceModel = 0);
        int lengthwiseIndex(const QModelIndex & index) const;
        QModelIndex mapFromSource(const QModelIndex & sourceIndex) const;
        int mapFromSourceColumn(QAbstractItemModel * sourceModel, int column) const;
        int mapFromSourceRow(QAbstractItemModel * sourceModel, int row) const;
        QItemSelection mapSelectionFromSource(const QItemSelection & sourceSelection) const;
        QItemSelection mapSelectionToSource(const QItemSelection & proxySelection) const;
        QModelIndex mapToSource(const QModelIndex & proxyIndex) const;
        int mapToSourceRow(int index, QAbstractItemModel * sourceModel) const;
        void removeSourceModel(QAbstractItemModel * sourceModel);
        int width(QAbstractItemModel * sourceModel = 0);
        int widthwiseIndex(const QModelIndex & index) const;

        AggregatingProxyModel * proxyModel;
        Qt::Orientation orientation;
        QList< QAbstractItemModel * > sourceModels;
        QMap< int, const QAbstractItemModel * > sourceModelIndexMap;
        int aggregatedLength;
        int maximumWidth;

    public slots:
        void on_columnsAboutToBeInserted(const QModelIndex & parent, int start, int end);
        void on_columnsAboutToBeMoved(const QModelIndex & sourceParent, int sourceStart, int sourceEnd, const QModelIndex & destinationParent, int destinationColumn);
        void on_columnsAboutToBeRemoved(const QModelIndex & parent, int start, int end);
        void on_columnsInserted(const QModelIndex & parent, int start, int end);
        void on_columnsMoved(const QModelIndex & sourceParent, int sourceStart, int sourceEnd, const QModelIndex & destinationParent, int destinationColumn);
        void on_columnsRemoved(const QModelIndex & parent, int start, int end);
        void on_dataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight, const QVector< int > & roles = QVector< int >());
        void on_headerDataChanged(Qt::Orientation orientation, int first, int last);
        void on_layoutAboutToBeChanged();
        void on_layoutChanged();
        void on_modelAboutToBeReset();
        void on_modelReset();
        void on_rowsAboutToBeInserted(const QModelIndex & parent, int start, int end);
        void on_rowsAboutToBeMoved(const QModelIndex & sourceParent, int sourceStart, int sourceEnd, const QModelIndex & destinationParent, int destinationRow);
        void on_rowsAboutToBeRemoved(const QModelIndex & parent, int start, int end);
        void on_rowsInserted(const QModelIndex & parent, int start, int end);
        void on_rowsMoved(const QModelIndex & sourceParent, int sourceStart, int sourceEnd, const QModelIndex & destinationParent, int destinationRow);
        void on_rowsRemoved(const QModelIndex & parent, int start, int end);


    signals:
        void columnsAboutToBeInserted(const QModelIndex & parent, int start, int end);
        void columnsAboutToBeMoved(const QModelIndex & sourceParent, int sourceStart, int sourceEnd, const QModelIndex & destinationParent, int destinationColumn);
        void columnsAboutToBeRemoved(const QModelIndex & parent, int start, int end);
        void columnsInserted(const QModelIndex & parent, int start, int end);
        void columnsMoved(const QModelIndex & sourceParent, int sourceStart, int sourceEnd, const QModelIndex & destinationParent, int destinationColumn);
        void columnsRemoved(const QModelIndex & parent, int start, int end);
        void dataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight, const QVector< int > & roles);
        void headerDataChanged(Qt::Orientation orientation, int first, int last);
        void layoutAboutToBeChanged();
        void layoutChanged();
        void modelAboutToBeReset();
        void modelReset();
        void rowsAboutToBeInserted(const QModelIndex & parent, int start, int end);
        void rowsAboutToBeMoved(const QModelIndex & sourceParent, int sourceStart, int sourceEnd, const QModelIndex & destinationParent, int destinationRow);
        void rowsAboutToBeRemoved(const QModelIndex & parent, int start, int end);
        void rowsInserted(const QModelIndex & parent, int start, int end);
        void rowsMoved(const QModelIndex & sourceParent, int sourceStart, int sourceEnd, const QModelIndex & destinationParent, int destinationRow);
        void rowsRemoved(const QModelIndex & parent, int start, int end);

    }; // class AggregatingProxyModelPrivate
} // namespace Athenaeum

#endif // ATHENAEUM_AGGREGATINGPROXYMODEL_P_H
