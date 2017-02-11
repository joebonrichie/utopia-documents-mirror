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

#ifndef ATHENAEUM_AGGREGATINGPROXYMODEL_H
#define ATHENAEUM_AGGREGATINGPROXYMODEL_H

#include <QAbstractItemModel>

class QItemSelection;

namespace Athenaeum
{

    class AggregatingProxyModelPrivate;
    class AggregatingProxyModel : public QAbstractItemModel
    {
        Q_OBJECT

    public:
        AggregatingProxyModel(Qt::Orientation orientation, QObject * parent = 0);
        virtual ~AggregatingProxyModel();

        // Re-implemented public methods of QAbstractItemModel
        virtual int columnCount(const QModelIndex & index = QModelIndex()) const;
        virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
        virtual Qt::ItemFlags flags(const QModelIndex & index) const;
        virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
        virtual QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
        virtual bool insertRows(int row, int count, const QModelIndex & parent = QModelIndex());
        virtual QMimeData * mimeData(const QModelIndexList & indexes) const;
        virtual QStringList mimeTypes() const;
        virtual QModelIndex parent(const QModelIndex & index) const;
        virtual bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex());
        virtual int rowCount(const QModelIndex & index = QModelIndex()) const;
        virtual bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);

        // Methods of this aggregating model
        void appendSourceModel(QAbstractItemModel * sourceModel);
        void clear();
        QModelIndex mapFromSource(const QModelIndex & sourceIndex) const;
        QItemSelection mapSelectionFromSource(const QItemSelection & sourceSelection) const;
        QItemSelection mapSelectionToSource(const QItemSelection & proxySelection) const;
        QModelIndex mapToSource(const QModelIndex & proxyIndex) const;
        void removeSourceModel(QAbstractItemModel * sourceModel);

    protected:
        AggregatingProxyModelPrivate * d;
    }; // class AggregatingProxyModel

} // namespace Athenaeum

#endif // ATHENAEUM_AGGREGATINGPROXYMODEL_H
