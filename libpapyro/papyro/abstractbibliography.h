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

#ifndef ATHENAEUM_ABSTRACTBIBLIOGRAPHY_H
#define ATHENAEUM_ABSTRACTBIBLIOGRAPHY_H

#include <papyro/citation.h>

#include <QMetaType>
#include <QString>
#include <QVector>

namespace Athenaeum
{

    class PersistenceModel;

    /////////////////////////////////////////////////////////////////////////////////////
    // AbstractBibliography provides the main API for interacting with a
    // collection of bibliographic items, but not with the items themselves.

    class AbstractBibliography
    {
    public:
        // State of this bibliographic model
        typedef enum State {
            IdleState = 0,
            CorruptState,
            BusyState,
            PurgedState
        } State; // enum State
        Q_ENUMS(State)

        // Each item in this bibliography can have a state associated with it,
        // used by other components to keep track of any processing they may
        // be doing on the item
        typedef enum ItemState {
            IdleItemState = 0,
            BusyItemState,
            ErrorItemState
        } ItemState; // enum State
        Q_ENUMS(ItemState)

        // Each item in a bibliography can have certain flags associated with it
        enum ItemFlag {
            NoItemFlags         = 0x00,
            UnreadItemFlag      = 0x01,
            StarredItemFlag     = 0x02,
            AllItemFlags        = 0xff
        }; // enum ItemFlag
        Q_DECLARE_FLAGS(ItemFlags, ItemFlag);

        // Roles for the various data of the model
        enum Roles {
            // Standard bibliographic item roles
            // These are persisted and can be written to while in memory
            KeyRole = Qt::UserRole,
            TitleRole,
            SubtitleRole,
            AuthorsRole,
            UrlRole,
            VolumeRole,
            IssueRole,
            YearRole,
            PageFromRole,
            PageToRole,
            AbstractRole,
            PublicationTitleRole,
            PublisherRole,
            DateImportedRole,
            DateModifiedRole,
            DateResolvedRole,
            DatePublishedRole,
            KeywordsRole,
            TypeRole,
            IdentifiersRole,
            LinksRole,
            DocumentUriRole,
            OriginatingUriRole,
            ObjectFileRole, // Actual place on disk this article can be found
            ItemFlagsRole,
            UnstructuredRole,

            PersistentRoleCount,
            // These can be written to, but are never persisted
            ItemStateRole = PersistentRoleCount,
            KnownRole,
            UserDefRole,

            MutableRoleCount,
            // These are read-only and are never persisted
            FullTextSearchRole = MutableRoleCount,
            ItemRole,

            RollCount
        }; // enum Roles
        Q_ENUMS(Roles)

        // Destructor
        virtual ~AbstractBibliography() {}

        /////////////////////////////////////////////////////////////////////////////////
		// Bibliographic collections need a user-friendly title

        virtual QString title() const = 0;

        /////////////////////////////////////////////////////////////////////////////////
		// Models can be made read only

        virtual bool isReadOnly() const { return false; };

        /////////////////////////////////////////////////////////////////////////////////
		// Models can be in a small number of states: see State enum above. Also, while
		// fetching, it can have a progress (<0 means no progress known).

        virtual qreal progress() const = 0;
		virtual State state() const = 0;
		virtual void setState(State state) = 0;

        /////////////////////////////////////////////////////////////////////////////////
		// Bibliographies have persistence models associated with them

		virtual PersistenceModel * persistenceModel() const = 0;

        /////////////////////////////////////////////////////////////////////////////////
		// Should be able to access the underlying (though opaque) item object, and be
		// able to add such an object to another model

        virtual void appendItem(CitationHandle item) { appendItems(QVector< CitationHandle >() << item); }
        virtual void appendItems(const QVector< CitationHandle > & items) {}
		virtual void clear() = 0;
        virtual void insertItem(CitationHandle before, CitationHandle item) { insertItems(before, QVector< CitationHandle >() << item); }
        virtual void insertItems(CitationHandle before, const QVector< CitationHandle > & items) {}
        virtual CitationHandle itemAt(int idx) const = 0;
        virtual int itemCount(ItemFlags flags = AllItemFlags) const = 0;
        virtual CitationHandle itemForId(const QString & id) const = 0;
        virtual CitationHandle itemForKey(const QString & key) const = 0;
        virtual QVector< CitationHandle > items() const { return QVector< CitationHandle >(); }
        virtual void prependItem(CitationHandle item) { prependItems(QVector< CitationHandle >() << item); }
        virtual void prependItems(const QVector< CitationHandle > & items) {}
        virtual bool removeItem(CitationHandle item) { return false; }
        virtual CitationHandle takeItemAt(int idx) = 0;

    protected:
        virtual void progressChanged(qreal progress) = 0;
        virtual void stateChanged(Athenaeum::AbstractBibliography::State state) = 0;
        virtual void titleChanged(QString title) = 0;

    }; // class AbstractBibliography

} // namespace Athenaeum

// Register various things with Qt's metatype system
Q_DECLARE_METATYPE(Athenaeum::AbstractBibliography::State);
Q_DECLARE_METATYPE(Athenaeum::AbstractBibliography::ItemState);
Q_DECLARE_METATYPE(Athenaeum::AbstractBibliography::ItemFlags);
Q_DECLARE_OPERATORS_FOR_FLAGS(Athenaeum::AbstractBibliography::ItemFlags);
Q_DECLARE_INTERFACE(Athenaeum::AbstractBibliography, "com.utopiadocs.Athenaeum.AbstractBibliography/1.1");

#endif // ATHENAEUM_ABSTRACTBIBLIOGRAPHY_H
