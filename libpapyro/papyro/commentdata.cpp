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

#include <papyro/commentdata.h>

#include <papyro/utils.h>
#include <string>

namespace Papyro
{

    class CommentDataPrivate
    {
    public:
        Spine::AnnotationHandle annotation;
    };



    CommentData::CommentData(Spine::AnnotationHandle annotation)
        : d(new CommentDataPrivate)
    {
        d->annotation = annotation;
    }

    CommentData::~CommentData() {}

    QString CommentData::anchor() const
    {
        QString anchor;
        if (d->annotation)
        {
            std::string text(d->annotation->text());
            if (!text.empty())
            {
                anchor = qStringFromUnicode(text);
            }
        }
        return anchor;
    }

    QString CommentData::author() const
    {
        QString author;
        if (d->annotation)
        {
            std::string property(d->annotation->getFirstProperty("author"));
            if (!property.empty())
            {
                author = qStringFromUnicode(property);
            }
        }
        return author;
    }

    QDateTime CommentData::date() const
    {
        QDateTime dateTime;
        if (d->annotation)
        {
            std::string property(d->annotation->getFirstProperty("created"));
            if (!property.empty())
            {
                dateTime = QDateTime::fromString(qStringFromUnicode(property), Qt::ISODate);
            }
        }
        return dateTime;
    }

    QString CommentData::id() const
    {
        QString id;
        if (d->annotation)
        {
            std::string property(d->annotation->getFirstProperty("id"));
            if (!property.empty())
            {
                id = qStringFromUnicode(property);
            }
        }
        return id;
    }

    bool CommentData::isPublic() const
    {
        return d->annotation && d->annotation->isPublic();
    }

    QString CommentData::parent() const
    {
        QString parent;
        if (d->annotation)
        {
            std::string property(d->annotation->getFirstProperty("parent"));
            if (!property.empty())
            {
                parent = qStringFromUnicode(property);
            }
        }
        return parent;
    }

    QString CommentData::text() const
    {
        QString text;
        if (d->annotation)
        {
            std::string property(d->annotation->getFirstProperty("property:comment"));
            if (!property.empty())
            {
                text = qStringFromUnicode(property);
            }
        }
        return text;
    }

}
