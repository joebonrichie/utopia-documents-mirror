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

#ifndef ATHENAEUM_LIBRARYMODEL_H
#define ATHENAEUM_LIBRARYMODEL_H

#include <athenaeum/abstractbibliographicmodel.h>

#include <QAbstractItemModel>

namespace Athenaeum
{

    class RemoteQueryBibliographicModel;

    /////////////////////////////////////////////////////////////////////////////////////
    // LibraryModel is the Qt model that represents a library of
    // bibliographic items

    class LibraryModelPrivate;
    class LibraryModel : public QAbstractItemModel
    {
        Q_OBJECT

    public:
        // Roles for the various data of the model
        enum Roles {
            // Roles
            ModelRole = Qt::UserRole,
            TitleRole,
            StateRole,
            ItemCountRole,
            UnreadItemCountRole,
            ImportantItemCountRole,
            CanFetchMoreRole
        }; // enum Roles

        // General constructor for a new library
        LibraryModel(QObject * parent = 0);
        // Destructor
        virtual ~LibraryModel();

        /////////////////////////////////////////////////////////////////////////////////
		// Should be able to access the underlying bibliographic model objects, and be
		// able to add such an object (whether they are full libraries or filters etc.).

        void appendModel(AbstractBibliographicModel * model);
        void insertModel(AbstractBibliographicModel * before, AbstractBibliographicModel * model);
        AbstractBibliographicModel * modelAt(int idx) const;
        int modelCount() const;
        QList< AbstractBibliographicModel * > models() const;
        bool removeModel(AbstractBibliographicModel * model);

        void appendSearch(RemoteQueryBibliographicModel * model);
        void insertSearch(RemoteQueryBibliographicModel * before, RemoteQueryBibliographicModel * model);
        RemoteQueryBibliographicModel * searchAt(int idx) const;
        int searchCount() const;
        QList< RemoteQueryBibliographicModel * > searches() const;
        bool removeSearch(RemoteQueryBibliographicModel * model);

        /////////////////////////////////////////////////////////////////////////////////
        // AbstractItemModel methods

        virtual int columnCount(const QModelIndex & index = QModelIndex()) const;
        virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
        virtual bool dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent);
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

        /////////////////////////////////////////////////////////////////////////////////
        // DnD methods

        void setImpendingMimeData(const QMimeData * data);
        Qt::DropActions supportedDropActions() const { return Qt::CopyAction; }
        Q_INVOKABLE bool acceptsDrop(const QModelIndex & index, bool child, const QMimeData * mimeData);

    protected:
        LibraryModelPrivate * d;

    }; // class AbstractBibliographicModel

} // namespace Athenaeum

Q_DECLARE_METATYPE(QAbstractItemModel *)

#endif // ATHENAEUM_LIBRARYMODEL_H
