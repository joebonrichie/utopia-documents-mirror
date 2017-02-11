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

#ifndef KEND_SERVICEMANAGERMODEL_H
#define KEND_SERVICEMANAGERMODEL_H

#include <utopia2/auth/config.h>

#include <QAbstractItemModel>
#include <QObject>
#include <QVariant>

namespace Kend
{

    // Service manager has a number of columns:
    //  0. Enabled (checkable)
    //  1. Service Description
    //  2. User name
    //  3. Status string
    //  4. Status icon

    class ServiceManager;

    class ServiceManagerModelPrivate;
    class LIBUTOPIA_AUTH_API ServiceManagerModel : public QAbstractItemModel
    {
        Q_OBJECT

    public:
        enum Roles {
            ServiceRole = Qt::UserRole,
            UuidRole
        };

        ServiceManagerModel(QObject * parent = 0);
        ~ServiceManagerModel();

        int columnCount(const QModelIndex & parent = QModelIndex()) const;
        QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
        Qt::ItemFlags flags(const QModelIndex & index) const;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
        QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
        QModelIndex parent(const QModelIndex & index) const;
        int rowCount(const QModelIndex & parent = QModelIndex()) const;
        bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);

    protected:
        ServiceManagerModelPrivate * d;
        friend class ServiceManagerModelPrivate;
    };

}

#endif // KEND_SERVICEMANAGERMODEL_H
