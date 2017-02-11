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

#ifndef KEND_SERVICEMANAGER_H
#define KEND_SERVICEMANAGER_H

#include <utopia2/auth/config.h>
#include <utopia2/auth/service.h>

#include <boost/shared_ptr.hpp>

#include <QObject>
#include <QString>

class QUrl;

namespace Kend
{

    class ServiceManagerPrivate;
    class LIBUTOPIA_AUTH_API ServiceManager : public QObject
    {
        Q_OBJECT

    public:
        typedef enum
        {
            NoFilter          = 0x00,
            FilterLoggedIn    = 0x01,
            FilterAnonymous   = 0x02,
            FilterOnline      = FilterLoggedIn | FilterAnonymous,
            FilterOffline     = 0x04,
            FilterIdle        = FilterOnline | FilterOffline,
            FilterBusy        = 0x08,
            FilterError       = 0x10
        } Filter;
        Q_DECLARE_FLAGS(Filters, Filter);

        ServiceManager(QObject * parent = 0);
        ~ServiceManager();

        // Manage services
        Service * addService(const QUrl & url);
        void addService(Service * service);
        bool manages(Service * service) const;
        bool removeService(Service * service);
        Service * serviceAt(int index) const;
        int count() const;
        bool matches(Service * service, Filters filter = NoFilter) const;
        QList< Service * > services(Filters filter = NoFilter) const;

        // Storage
        bool loadFromSettings();
        bool saveToSettings();

        // Overview statistics
        void getStatistics(int * online, int * offline, int * busy, int * error) const;

        // Shared instance pointer
        static boost::shared_ptr< ServiceManager > instance();

    public slots:
        // State Machine
        void start(Kend::Service * service = 0);
        void stop(Kend::Service * service = 0);

    signals:
        // Management signals
        void serviceAdded(Kend::Service * service);
        void serviceRemoved(Kend::Service * service);

        // State machine signals
        void serviceError(Kend::Service * service);
        void serviceLoggingIn(Kend::Service * service);
        void serviceLoggingOut(Kend::Service * service);
        void serviceStarted(Kend::Service * service);
        void servicePopulating(Kend::Service * service);
        void serviceStopped(Kend::Service * service);
        void serviceStateChanged(Kend::Service * service, Kend::Service::ServiceState state);

    protected:
        ServiceManagerPrivate * d;
        friend class ServiceManagerPrivate;
    };

}

Q_DECLARE_OPERATORS_FOR_FLAGS(Kend::ServiceManager::Filters);

#endif // KEND_SERVICEMANAGER_H
