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

#include <papyro/remotequery.h>
#include <papyro/remotequery_p.h>

#include <QMutexLocker>

namespace Athenaeum
{

    RemoteQueryResultSet::RemoteQueryResultSet()
        : offset(0), limit(0), count(0)
    {}




    RemoteQueryPrivate::RemoteQueryPrivate()
        : mutex(QMutex::Recursive)
    {}




    RemoteQuery::RemoteQuery(QObject * parent)
        : QThread(parent), d(new RemoteQueryPrivate)
    {}

    RemoteQuery::~RemoteQuery()
    {
        delete d;
    }

    QVariant RemoteQuery::persistentProperty(const QString & key) const
    {
        QMutexLocker guard(&d->mutex);
        return d->persistentProperties.value(key);
    }

    QStringList RemoteQuery::persistentPropertyNames() const
    {
        QMutexLocker guard(&d->mutex);
        return d->persistentProperties.keys();
    }

    void RemoteQuery::setPersistentProperty(const QString & key, const QVariant & value)
    {
        QMutexLocker guard(&d->mutex);
        d->persistentProperties[key] = value;
    }

} // namespace Athenaeum

UTOPIA_DEFINE_EXTENSION_CLASS(Athenaeum::RemoteQuery)
