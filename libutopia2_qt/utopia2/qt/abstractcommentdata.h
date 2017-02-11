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

#ifndef Utopia_QT_ABSTRACTCOMMENTDATA_H
#define Utopia_QT_ABSTRACTCOMMENTDATA_H

#include <utopia2/qt/config.h>

#include <QString>
#include <QDateTime>

namespace Utopia
{

    class AbstractCommentData
    {
    public:
        virtual QString   author() const = 0;
        virtual QDateTime   date() const = 0;
        virtual QString       id() const = 0;
        virtual QString   parent() const = 0;
        virtual QString     text() const = 0;
        virtual bool    isPublic() const = 0;
        virtual QString   anchor() const = 0;

        QString dateSince(QDateTime recent, QDateTime past)
            {
                int days = past.daysTo(recent);
                int seconds = past.secsTo(recent);

                if (days > 0)
                {
                    if (days == 1)
                    {
                        return "yesterday";
                    }
                    else if ((days / 7) > 1)
                    {
                        return QString::number(days / 7) + " weeks ago";
                    }
                    else
                    {
                        return QString::number(days) + " days ago";
                    }
                }
                else
                {
                    if (seconds < 60)
                    {
                        return "A few seconds ago";
                    }
                    else
                    {
                        int minutes = seconds / 60;
                        int hours = seconds / (60 * 60);
                        if (hours == 1)
                        {
                            return "An hour ago";
                        }
                        else if (hours > 1)
                        {
                            return QString::number(hours) + " hours ago";
                        }
                        else if (minutes == 1)
                        {
                            return "About a minute ago";
                        }
                        else
                        {
                            return QString::number(minutes) + " minutes ago";
                        }
                    }
                }

                return "";
            }
    };

}

#endif // Utopia_QT_ABSTRACTCOMMENTDATA_H
