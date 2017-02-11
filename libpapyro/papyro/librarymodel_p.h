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

#ifndef ATHENAEUM_LIBRARYMODEL_P_H
#define ATHENAEUM_LIBRARYMODEL_P_H

#include <papyro/abstractbibliography.h>

#include <QList>
#include <QMap>
#include <QObject>
#include <QRunnable>
#include <QStringList>
#include <QThreadPool>
#include <QWeakPointer>

#include <QDebug>

class QAbstractItemModel;
class QMimeData;
class QModelIndex;

namespace Athenaeum
{

    // Header items
    enum {
        _EVERYTHING = 1,
        _HEADER_COLLECTIONS,
        _NO_COLLECTIONS,
        _HEADER_SEARCHES,
        _NO_SEARCHES,

        _PLACEHOLDER_COUNT
    };

    class Bibliography;
    class RemoteQueryBibliography;
    class ResolverQueue;
    class SortFilterProxyModel;

    class LibraryModel;
    class LibraryModelPrivate : public QObject
    {
        Q_OBJECT

    public:
        LibraryModelPrivate(LibraryModel * model);
        ~LibraryModelPrivate();

        LibraryModel * m;

        Athenaeum::Bibliography * master;
        SortFilterProxyModel * starred;
        SortFilterProxyModel * recent;
        QList< QAbstractItemModel * > models;
        QMap< QAbstractItemModel *, QList< SortFilterProxyModel * > > filters;
        QList< RemoteQueryBibliography * > searches;
        QStringList mimeTypes;
        ResolverQueue * resolverQueue;

        bool noCollectionPlaceholder;
        bool noWatchPlaceholder;

        void connectModel(QAbstractItemModel * model);
        void disconnectModel(QAbstractItemModel * model);
        void relayDataChanged();
        QModelIndex masterIndex() const;
        QModelIndex starredIndex() const;
        QModelIndex recentIndex() const;
        QModelIndex collectionParentIndex() const;
        QModelIndex searchParentIndex() const;

        void updateMimeTypes();

        void save();

    public slots:
        // Update of underlying models
        void onDataChanged(const QModelIndex &, const QModelIndex &, const QVector< int > & roles = QVector< int >());
        void onRowsAboutToBeRemoved(const QModelIndex &, int, int);
        void onRowsInserted(const QModelIndex &, int, int);
        void onRowsRemoved(const QModelIndex &, int, int);
        void onStateChanged(Athenaeum::AbstractBibliography::State state);
        void onTitleChanged(const QString & title);

    signals:
        void dataChanged(const QModelIndex &, const QModelIndex &);
    }; // class LibraryModelPrivate

} // namespace Athenaeum

#endif // ATHENAEUM_LIBRARYMODEL_P_H
