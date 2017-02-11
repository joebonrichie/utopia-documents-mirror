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

    signals:
        void changed();
        void changed(int role, QVariant oldValue);

    private:
        boost::shared_ptr< CitationPrivate > d;
    };

    typedef boost::shared_ptr< Citation > CitationHandle;

} // namespace Athenaeum

Q_DECLARE_SMART_POINTER_METATYPE(boost::shared_ptr);

#endif // ATHENAEUM_CITATION_H
