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

#ifndef ATHENAEUM_ABSTRACTBIBLIOGRAPHICCOLLECTION_H
#define ATHENAEUM_ABSTRACTBIBLIOGRAPHICCOLLECTION_H

#include <QMetaType>
#include <QString>
#include <QStringList>

namespace Athenaeum
{

    /////////////////////////////////////////////////////////////////////////////////////
    // AbstractBibliographicCollection provides the main API for interacting with a
    // collection of bibliographic items, but not with the items themselves.

    class BibliographicItem;
    class AbstractBibliographicCollection
    {
    public:
        // State of this bibliographic model
        typedef enum {
            IdleState = 0,
            CorruptState,
            BusyState,
            PurgedState
        } State; // enum State

        // State of this model's item
        typedef enum {
            IdleItemState = 0,
            BusyItemState,
            ErrorItemState
        } ItemState; // enum State

        // User flags for this model's items
        enum ItemFlag {
            NoFlags         = 0x00,
            UnreadFlag      = 0x01,
            ImportantFlag   = 0x02,

            AllFlags        = 0xff
        }; // enum ItemFlag
        Q_DECLARE_FLAGS(ItemFlags, ItemFlag);

        // Roles for the various data of the model
        enum Roles {
            // Roles
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
            DateModifiedRole,
            KeywordsRole,
            TypeRole,
            IdentifiersRole,
            DocumentUriRole,
            PdfRole,
            ItemFlagsRole,

            PersistentRoleCount,
            ItemStateRole = PersistentRoleCount,

            RoleCount,
            FullTextSearchRole,
            ItemRole
        }; // enum Roles

        // Destructor
        virtual ~AbstractBibliographicCollection() {}

        /////////////////////////////////////////////////////////////////////////////////
		// Bibliographic collections need a user-friendly title

        virtual void setTitle(const QString & title) = 0;
        virtual QString title() const = 0;

        /////////////////////////////////////////////////////////////////////////////////
		// Some collections should be read-only from the outside, and some should be
		// persistent

		virtual bool isReadOnly() const = 0;
		virtual bool isPersistent() const = 0;

        /////////////////////////////////////////////////////////////////////////////////
		// Models can be in a small number of states: see State enum above. Also, while
		// fetching, it can have a progress (<0 means no progress known).

        virtual qreal progress() const = 0;
		virtual State state() const = 0;
		virtual int count(ItemFlags flags) const = 0;

        /////////////////////////////////////////////////////////////////////////////////
		// Should be able to clear and purge a model. Clearing a model simply removes its
		// contents, while purging a model completely removes its persistence, preventing
		// it from being re-created in the future.

		virtual void clear() = 0;
		virtual void purge() {};

        /////////////////////////////////////////////////////////////////////////////////
		// Should be able to access the underlying (though opaque) item object, and be
		// able to add such an object to another model

        virtual void appendItem(BibliographicItem * model) {}
        virtual void insertItem(BibliographicItem * before, BibliographicItem * model) {}
        virtual BibliographicItem * itemAt(int idx) const { return 0; }
        virtual int count() const { return 0; }
        virtual QList< BibliographicItem * > items() const { return QList< BibliographicItem * >(); }
        virtual bool removeItem(BibliographicItem * model) { return false; }
        virtual BibliographicItem * takeItemAt(int idx) { return 0; }

    protected:
        virtual void stateChanged(Athenaeum::AbstractBibliographicCollection::State state) = 0;

    }; // class AbstractBibliographicCollection

} // namespace Athenaeum

// Register various things with Qt's metatype system
Q_DECLARE_METATYPE(Athenaeum::AbstractBibliographicCollection::State)
Q_DECLARE_METATYPE(Athenaeum::AbstractBibliographicCollection::ItemState)
Q_DECLARE_METATYPE(Athenaeum::AbstractBibliographicCollection::ItemFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(Athenaeum::AbstractBibliographicCollection::ItemFlags)
Q_DECLARE_INTERFACE(Athenaeum::AbstractBibliographicCollection, "com.utopiadocs.Athenaeum.AbstractBibliographicCollection/1.0")

#endif // ATHENAEUM_ABSTRACTBIBLIOGRAPHICCOLLECTION_H
