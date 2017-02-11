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

#ifndef ATHENAEUM_REMOTEQUERY_H
#define ATHENAEUM_REMOTEQUERY_H

#include <utopia2/extension.h>

#include <QString>
#include <QStringList>
#include <QThread>
#include <QVariantList>
#include <QVariantMap>

namespace Athenaeum
{

    class RemoteQueryResultSet
    {
    public:
        RemoteQueryResultSet();

        int offset;
        int limit;
        int count;
        QVariantList results;

        QString error;
    };



    class RemoteQueryPrivate;
    class RemoteQuery : public QThread
    {
        Q_OBJECT

    public:
        RemoteQuery(QObject * parent = 0);
        virtual ~RemoteQuery();
        typedef RemoteQuery API;

        virtual QString description() = 0;
        virtual bool fetch(const QVariantMap & query, int offset = 0, int limit = -1) = 0;
        QVariant persistentProperty(const QString & key) const;
        QStringList persistentPropertyNames() const;
        void setPersistentProperty(const QString & key, const QVariant & value);
        virtual QString title() = 0;

    signals:
        void fetched(Athenaeum::RemoteQueryResultSet results);

    protected:
        RemoteQueryPrivate * d;
    }; // class RemoteQuery

} // namespace Athenaeum

Q_DECLARE_METATYPE(Athenaeum::RemoteQueryResultSet);

UTOPIA_DECLARE_EXTENSION_CLASS(LIBATHENAEUM, Athenaeum::RemoteQuery)

#endif // ATHENAEUM_REMOTEQUERY_H
