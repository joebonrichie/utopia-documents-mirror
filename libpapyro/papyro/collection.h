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

#ifndef ATHENAEUM_COLLECTION_H
#define ATHENAEUM_COLLECTION_H

#include <papyro/abstractbibliography.h>

#include <QAbstractItemModel>

namespace Athenaeum
{

    class CollectionPrivate;
    class Collection : public QAbstractItemModel, public AbstractBibliography
    {
        Q_OBJECT
        Q_PROPERTY(bool readOnly
                   READ isReadOnly
                   WRITE setReadOnly
                   STORED false)
        Q_PROPERTY(QString title
                   READ title
                   WRITE setTitle
                   NOTIFY titleChanged)
        Q_INTERFACES(Athenaeum::AbstractBibliography)

    public:
        // General constructor for a new library
        Collection(AbstractBibliography * source, QObject * parent = 0);
        virtual ~Collection();

        /////////////////////////////////////////////////////////////////////////////////
		// Collections work over another bibliography

        AbstractBibliography * sourceBibliography() const;

        /////////////////////////////////////////////////////////////////////////////////
		// Bibliographic models need a user-friendly title

        virtual void setTitle(const QString & title);
        virtual QString title() const;

        /////////////////////////////////////////////////////////////////////////////////
		// Some models should be read-only from the outside, and some should be
		// persistent

		virtual bool isReadOnly() const;
		virtual void setReadOnly(bool readOnly);

        /////////////////////////////////////////////////////////////////////////////////
		// Models can be in a small number of states: see State enum above. Also, while
		// fetching, it can have a progress (<0 means no progress known).

        virtual qreal progress() const;
		virtual void setProgress(qreal progress);
		virtual void setState(State state);
		virtual State state() const;

        /////////////////////////////////////////////////////////////////////////////////
		// Bibliographies have persistence models associated with them

		virtual PersistenceModel * persistenceModel() const;
		void setPersistenceModel(PersistenceModel * persistenceModel);

	public:
        /////////////////////////////////////////////////////////////////////////////////
		// Should be able to access the underlying (though opaque) item object, and be
		// able to add such an object to another model

        virtual void appendItems(const QVector< CitationHandle > & items);
		virtual void clear();
        virtual void insertItems(CitationHandle before, const QVector< CitationHandle > & items);
        virtual CitationHandle itemAt(int idx) const;
        virtual int itemCount(Citation::Flags flags = Citation::AllFlags) const;
        virtual CitationHandle itemForId(const QString & id) const;
        virtual CitationHandle itemForKey(const QString & key) const;
        virtual QVector< CitationHandle > items() const;
        virtual void prependItems(const QVector< CitationHandle > & items);
        virtual bool removeItem(CitationHandle item);
        virtual CitationHandle takeItemAt(int idx);

        /////////////////////////////////////////////////////////////////////////////////
        // AbstractItemModel methods

        virtual int columnCount(const QModelIndex & index = QModelIndex()) const;
        virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
        virtual Qt::ItemFlags flags(const QModelIndex & index) const;
        virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
        virtual QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
        virtual bool insertRows(int row, int count, const QModelIndex & parent = QModelIndex());
        virtual QModelIndex parent(const QModelIndex & index) const;
        virtual bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex());
        virtual int rowCount(const QModelIndex & index = QModelIndex()) const;
        virtual bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);

        /////////////////////////////////////////////////////////////////////////////////
		// Drag and drop functionality

        virtual bool dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent);
        virtual QMimeData * mimeData(const QModelIndexList & indexes) const;
        virtual QStringList mimeTypes() const;

    signals:
        void progressChanged(qreal progress);
        void stateChanged(Athenaeum::AbstractBibliography::State state);
        void titleChanged(QString title);

    private:
        CollectionPrivate * d;

    }; // class AbstractBibliography

} // namespace Athenaeum

#endif // ATHENAEUM_PERSISTENTBIBLIOGRAPHICMODEL_H
