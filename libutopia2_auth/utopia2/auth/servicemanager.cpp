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

#include <utopia2/global.h>
#include <utopia2/auth/servicemanager_p.h>
#include <utopia2/auth/servicemanager.h>
#include <utopia2/auth/service.h>

#include <boost/weak_ptr.hpp>

#include <QList>
#include <QNetworkAccessManager>
#include <QSettings>
#include <QStringList>
#include <QUuid>

#include <QDebug>

#define K_PROPERTY_INTENTION "_intention"

namespace Kend
{

    namespace
    {

        QString uuid_to_string(const QUuid & uuid)
        {
            QString string(uuid.toString());
            return string.mid(1, string.length() - 2);
        }

        QUuid string_to_uuid(const QString & string)
        {
            return QUuid("{" + string + "}");
        }

    }




    ServiceManagerPrivate::ServiceManagerPrivate(ServiceManager * manager)
    : QObject(manager), manager(manager)
    {
        checker.setInterval(1000 * 60);
        connect(&checker, SIGNAL(timeout()), this, SLOT(onCheckerTimeout()));
        checker.start();
    }

    int ServiceManagerPrivate::inProgress() const
    {
        int inProgress = 0;
        foreach (Service * service, services) {
            switch (service->serviceState()) {
            case Service::StartedState:
                if (!service->property(K_PROPERTY_INTENTION).toString().isEmpty()) {
                    ++inProgress;
                }
                break;
            default:
                break;
            }
        }
        return inProgress;
    }

    void ServiceManagerPrivate::onCheckerTimeout()
    {
        // Find any active account that is in an intermittent error state, and try again
        foreach (Service * service, services) {
            switch (service->errorCode()) {
            case Service::ServiceServerError:
            case Service::ServiceServerInaccessible:
            case Service::AuthenticationServerError:
            case Service::AuthenticationServerInaccessible:
            case Service::UnknownError:
                if (service->isEnabled()) {
                    // Stop and start
                    service->reset();
                    manager->start(service);
                }
                break;
            default:
                break;
            }
        }
    }

    void ServiceManagerPrivate::onServiceLoggingIn()
    {
        emit manager->serviceLoggingIn((Service *) sender());
    }

    void ServiceManagerPrivate::onServiceLoggingOut()
    {
        emit manager->serviceLoggingOut((Service *) sender());
    }

    void ServiceManagerPrivate::onServiceStarted()
    {
        Service * service = (Service *) sender();

        emit manager->serviceStarted(service);

        if (service->property(K_PROPERTY_INTENTION).toString() == "logIn")
        {
            service->setProperty(K_PROPERTY_INTENTION, QVariant());
            service->logIn();
        }
        else if (service->property(K_PROPERTY_INTENTION).toString() == "stop")
        {
            service->setProperty(K_PROPERTY_INTENTION, QVariant());
            service->stop();
        }
    }

    void ServiceManagerPrivate::onServicePopulating()
    {
        emit manager->servicePopulating((Service *) sender());
    }

    void ServiceManagerPrivate::onServiceStopped()
    {
        emit manager->serviceStopped((Service *) sender());
    }

    void ServiceManagerPrivate::onServiceError()
    {
        emit manager->serviceError((Service *) sender());
    }

    void ServiceManagerPrivate::onServiceStateChanged(Service::ServiceState state)
    {
        emit manager->serviceStateChanged((Service *) sender(), state);
    }



    ServiceManager::ServiceManager(QObject * parent)
    : QObject(parent), d(new ServiceManagerPrivate(this))
    {}

    ServiceManager::~ServiceManager()
    {}

    Service * ServiceManager::addService(const QUrl & url)
    {
        Service * service = new Service();
        service->setUrl(url, true);
        addService(service);
        return service;
    }

    void ServiceManager::addService(Service * service)
    {
        if (!d->services.contains(service))
        {
            d->services.append(service);

            connect(service, SIGNAL(serviceLoggingIn()), d, SLOT(onServiceLoggingIn()));
            connect(service, SIGNAL(serviceLoggingOut()), d, SLOT(onServiceLoggingOut()));
            connect(service, SIGNAL(servicePopulating()), d, SLOT(onServicePopulating()));
            connect(service, SIGNAL(serviceStarted()), d, SLOT(onServiceStarted()));
            connect(service, SIGNAL(serviceStopped()), d, SLOT(onServiceStopped()));
            connect(service, SIGNAL(serviceError()), d, SLOT(onServiceError()));
            connect(service, SIGNAL(serviceStateChanged(Kend::Service::ServiceState)),
                    d, SLOT(onServiceStateChanged(Kend::Service::ServiceState)));

            // Take control
            service->setParent(this);

            emit serviceAdded(service);
        }
    }

    int ServiceManager::count() const
    {
        return d->services.size();
    }

    void ServiceManager::getStatistics(int * online, int * offline, int * busy, int * error) const
    {
        struct { int online, offline, busy, error; } tmp = { 0, 0, 0, 0 };
        foreach (Service * service, d->services) {
            if (service->isEnabled()) {
                switch (service->serviceState()) {
                case Service::ErrorState:
                    ++tmp.error;
                    break;
                case Service::StoppedState:
                    ++tmp.offline;
                    break;
                case Service::StartedState:
                    if (service->property(K_PROPERTY_INTENTION).toString().isEmpty()) {
                        ++tmp.online;
                        break;
                    }
                default:
                    ++tmp.busy;
                    break;
                }
            }
        }
        if (online) *online = tmp.online;
        if (offline) *offline = tmp.offline;
        if (busy) *busy = tmp.busy;
        if (error) *error = tmp.error;
    }

    boost::shared_ptr< ServiceManager > ServiceManager::instance()
    {
        static boost::weak_ptr< ServiceManager > singleton;
        boost::shared_ptr< ServiceManager > shared(singleton.lock());
        if (singleton.expired())
        {
            shared = boost::shared_ptr< ServiceManager >(new ServiceManager());
            singleton = shared;
        }
        return shared;
    }

    bool ServiceManager::loadFromSettings()
    {
        QSettings conf;
        conf.beginGroup("Services");
        conf.beginGroup("Store");
        QStringListIterator uuids(conf.childGroups());
        while (uuids.hasNext())
        {
            QString uuid = uuids.next();
            conf.beginGroup(uuid);
            Service * service = new Service(string_to_uuid(uuid), (QObject *) this);
            if (service->loadFrom(conf))
            {
                addService(service);
            }
            else
            {
                delete service;
            }
            conf.endGroup();
        }
        conf.endGroup();
        conf.endGroup();

        return true;
    }

    bool ServiceManager::manages(Service * service) const
    {
        return d->services.contains(service);
    }

    bool ServiceManager::removeService(Service * service)
    {
        QSettings conf;
        conf.beginGroup("Services");
        conf.beginGroup("Store");
        conf.remove(uuid_to_string(service->uuid()));

        disconnect(service, SIGNAL(serviceLoggingIn()), d, SLOT(onServiceLoggingIn()));
        disconnect(service, SIGNAL(serviceLoggingOut()), d, SLOT(onServiceLoggingOut()));
        disconnect(service, SIGNAL(serviceStarted()), d, SLOT(onServiceStarted()));
        disconnect(service, SIGNAL(servicePopulating()), d, SLOT(onServicePopulating()));
        disconnect(service, SIGNAL(serviceStopped()), d, SLOT(onServiceStopped()));
        disconnect(service, SIGNAL(serviceError()), d, SLOT(onServiceError()));
        disconnect(service, SIGNAL(serviceStateChanged(Kend::Service::ServiceState)),
                   d, SLOT(onServiceStateChanged(Kend::Service::ServiceState)));

        if (d->services.removeAll(service) > 0) {
            emit serviceRemoved(service);
            service->deleteLater(); // Delete service
            return true;
        } else {
            return false;
        }
    }

    bool ServiceManager::saveToSettings()
    {
        bool success = true;

        QSettings conf;
        conf.beginGroup("Services");
        conf.beginGroup("Store");
        QListIterator< Service * > services(d->services);
        while (services.hasNext())
        {
            Service * service = services.next();
            conf.beginGroup(uuid_to_string(service->uuid()));
            success = success && service->saveTo(conf);
            conf.endGroup();
        }
        conf.endGroup();
        conf.endGroup();

        return success;
    }

    Service * ServiceManager::serviceAt(int index) const
    {
        return d->services.at(index);
    }

    bool ServiceManager::matches(Service * service, Filters filter) const
    {
        return (filter == NoFilter) ||
               (filter & FilterLoggedIn && service->isLoggedIn()) ||
               (filter & FilterAnonymous && service->serviceState() == Service::StartedState && service->isAnonymous()) ||
               (filter & FilterOffline && service->serviceState() == Service::StoppedState) ||
               (filter & FilterBusy && service->serviceState() & (Service::PopulatingState | Service::LoggingInState | Service::LoggingOutState)) ||
               (filter & FilterError && service->serviceState() == Service::ErrorState);
    }

    QList< Service * > ServiceManager::services(Filters filter) const
    {
        if (filter == NoFilter) {
            return d->services;
        }

        QList< Service * > matchingServices;
        foreach (Service * service, d->services) {
            if (matches(service, filter)) {
                matchingServices.append(service);
            }
        }
        return matchingServices;
    }

    void ServiceManager::start(Service * candidate)
    {
        QListIterator< Service * > services(d->services);
        while (services.hasNext()) {
            Service * service = services.next();
            if (candidate == 0 || service == candidate) {
                if (service->isEnabled()) {
                    if (!service->isAnonymous()) {
                        service->setProperty(K_PROPERTY_INTENTION, "logIn");
                    }
                    service->start(true);
                }
            }
        }
    }

    void ServiceManager::stop(Service * candidate)
    {
        QListIterator< Service * > services(d->services);
        while (services.hasNext())
        {
            Service * service = services.next();
            if (candidate == 0 || service == candidate) {
                if (service->isLoggedIn())
                {
                    service->setProperty(K_PROPERTY_INTENTION, "stop");
                    service->logOut();
                }
                else
                {
                    service->stop(true);
                }
            }
        }
    }

}
