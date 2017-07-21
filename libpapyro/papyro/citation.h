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

#ifndef ATHENAEUM_CITATION_H
#define ATHENAEUM_CITATION_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QVector>

#include <boost/shared_ptr.hpp>

struct cJSON;

namespace Athenaeum
{

    /////////////////////////////////////////////////////////////////////////////////////
    // citation ////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////

    class CitationPrivate;
    class Citation : public QObject
    {
        Q_OBJECT

    public:
        // Every citation can have a state associated with it, used by other
        // components to keep track of any processing they may be doing on it
        typedef enum State {
            IdleState = 0,
            BusyState,
            ErrorState
        } State; // enum State
        Q_ENUMS(State)

        // Each citation can have certain flags associated with it
        enum Flag {
            NoFlags         = 0x00,
            UnreadFlag      = 0x01,
            StarredFlag     = 0x02,
            AllFlags        = 0xff
        }; // enum Flag
        Q_DECLARE_FLAGS(Flags, Flag);

        // Roles for the various data of the model
        enum Role {
            ///////////////////////////////////////////////////////////////////
            // The first set of roles are persisted and can be written to while
            // in memory.

            // Unique key (UUID) of a citation. If two citations have the same
            // key, they are expected to be identical, but not necessarily
            // vice versa.
            KeyRole = Qt::UserRole,

            // Title and subtitle of a citation. Expected to be strings.
            TitleRole,
            SubTitleRole,

            // Authors list, where each author is a string with a format of:
            // "SURNAME, FORENAME(S)"
            AuthorsRole,

            // Volume, issue, and year of citation (numerical).
            VolumeRole,
            IssueRole,
            YearRole,

            PageFromRole,
            PageToRole,

            // Full abstract of the article being cited.
            AbstractRole,

            // Title of the publication
            PublicationTitleRole,

            // Publisher's name
            PublisherRole,

            // List of strings for each keyword / keyphrase
            KeywordsRole,

            // What kind of entity is being cited? (book, article, etc.)
            TypeRole,

            // This is a mapping of names (doi, pubmed, utopia...) to IDs.
            IdentifiersRole,

            // This is a list of structured hyperlinks (each a mapping)
            LinksRole,

            DocumentUriRole,

            // How was this citation originally
            OriginatingUriRole,

            // Actual place on disk this article can be found
            ObjectFileRole,

            // Unstructured text of this citation, in the case where we don't
            // have any of the above roles to begin with.
            UnstructuredRole,

            ProvenanceRole,

            // Organisational roles for use in the Library.
            FlagsRole,
            DateImportedRole,
            DateResolvedRole,

            // Deprecated roles, or roles of questionable virtue
            UrlRole,
            DatePublishedRole,
            DateModifiedRole,

            PersistentRoleCount,

            ///////////////////////////////////////////////////////////////////
            // The next set of roles are never persisted, but can be written to
            // while in memory.

            StateRole = PersistentRoleCount,
            KnownRole,
            UserDefRole,

            MutableRoleCount,

            ///////////////////////////////////////////////////////////////////
            // The next set of roles are never persisted, and are read-only.

            FullTextSearchRole = MutableRoleCount,
            ItemRole,

            RoleCount
        }; // enum Role
        Q_ENUMS(Role)

        // Construct a new item
        Citation(bool dirty = false);

        const QVariant & field(int role) const;
        bool isBusy() const;
        bool isDirty() const;
        bool isKnown() const;
        bool isStarred() const;
        void setClean();
        void setDirty();
        void setField(int role, const QVariant & data);
        cJSON * toJson() const;
        QVariantMap toMap() const;
        void updateFromMap(const QVariantMap & variant);

        static boost::shared_ptr< Citation > fromJson(cJSON * json);
        static boost::shared_ptr< Citation > fromMap(const QVariantMap & variant);

        bool operator == (const Citation & other) const;
        bool operator != (const Citation & other) const;

        static QString roleTitle(Role role);

    signals:
        void changed();
        void changed(int role, QVariant oldValue);

    private:
        boost::shared_ptr< CitationPrivate > d;
    };

    typedef boost::shared_ptr< Citation > CitationHandle;

} // namespace Athenaeum

Q_DECLARE_SMART_POINTER_METATYPE(boost::shared_ptr);
Q_DECLARE_METATYPE(Athenaeum::Citation::State);
Q_DECLARE_METATYPE(Athenaeum::Citation::Flags);
Q_DECLARE_OPERATORS_FOR_FLAGS(Athenaeum::Citation::Flags);

#endif // ATHENAEUM_CITATION_H
