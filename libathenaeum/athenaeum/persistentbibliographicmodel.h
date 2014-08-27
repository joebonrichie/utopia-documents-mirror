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

#ifndef ATHENAEUM_PERSISTENTBIBLIOGRAPHICMODEL_H
#define ATHENAEUM_PERSISTENTBIBLIOGRAPHICMODEL_H

#include <athenaeum/abstractbibliographicmodel.h>

#include <QDir>
#include <QUuid>

namespace Athenaeum
{

    /////////////////////////////////////////////////////////////////////////////////////
    // PersistentBibliographicModel is the specialisation of an AbstractBibliographyModel
    // that persistently stores its contents on disk
    //
    // The model is stored within a directory hierarchy, the root of which is given to
    // the model on construction. This given directory then stores a metadata file and
    // two sub-directories: jsondb/ stores the json for each item of the model, and
    // objects/ stores their PDF files. These two directories are organised by that
    // item's key (a UUID), with items grouped according to the first two characters of
    // that key. This equates to a maximum of 256 subdirectories (00, 01, ... fe, ff).


    class PersistentBibliographicModelPrivate;
    class PersistentBibliographicModel : public AbstractBibliographicModel
    {
        Q_OBJECT

    public:
        // General constructor for a new library
        PersistentBibliographicModel(const QDir & path, QObject * parent = 0);
        virtual ~PersistentBibliographicModel();

        /////////////////////////////////////////////////////////////////////////////////
		// Bibliographic models need a user-friendly title, and a path on disk (a
		// directory into which files are written).

        virtual QDir path() const;
        virtual void setTitle(const QString & title);
        virtual QString title() const;

        /////////////////////////////////////////////////////////////////////////////////
		// Some models should be read-only from the outside, and some should be
		// persistent

		virtual bool isReadOnly() const;
		virtual bool isPersistent() const;

        /////////////////////////////////////////////////////////////////////////////////
		// Should be able to clear and purge a model. Clearing a model simply removes its
		// contents, while purging a model completely removes its persistence, preventing
		// it from being re-created in the future.

		virtual void clear();
		virtual void purge();

        /////////////////////////////////////////////////////////////////////////////////
		// Models can be in a small number of states: see State enum above. Also, while
		// fetching, it can have a progress (<0 means no progress known).

        virtual qreal progress() const;
		virtual State state() const;
		virtual int count(ItemFlags flags) const;

	public:
        /////////////////////////////////////////////////////////////////////////////////
		// Should be able to access the underlying (though opaque) item object, and be
		// able to add such an object to another model

        virtual void appendItem(BibliographicItem * model);
        virtual void insertItem(BibliographicItem * before, BibliographicItem * model);
        virtual BibliographicItem * itemAt(int idx) const;
        virtual inline int count() const { return rowCount(); }
        virtual QList< BibliographicItem * > items() const;
        virtual bool removeItem(BibliographicItem * model);
        virtual BibliographicItem * takeItemAt(int idx);

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

    protected:
        PersistentBibliographicModelPrivate * d;
        friend class PersistentBibliographicModelPrivate;

		void setReadOnly(bool readOnly);
		virtual void setProgress(qreal progress);
		virtual void setState(AbstractBibliographicModel::State state);

    }; // class AbstractBibliographicModel

} // namespace Athenaeum

Q_DECLARE_METATYPE(Athenaeum::BibliographicItem *)

#endif // ATHENAEUM_PERSISTENTBIBLIOGRAPHICMODEL_H
