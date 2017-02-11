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

#include <utopia2/auth/servicemanagernotifier_p.h>
#include <utopia2/auth/servicemanagernotifier.h>
#include <utopia2/auth/servicemanagermodel.h>
#include <utopia2/auth/servicemanagerfilterproxymodel.h>

namespace Kend
{

    ServiceManagerNotifierPrivate::ServiceManagerNotifierPrivate(ServiceManagerNotifier * notifier, ServiceManager::Filters filters)
        : QObject(notifier), manager(ServiceManager::instance()), filters(filters), valid(true)
    {
        check();
        connect(this, SIGNAL(validityChanged(bool)), notifier, SIGNAL(validityChanged(bool)));
        connect(manager.get(), SIGNAL(serviceStateChanged(Kend::Service *, Kend::Service::ServiceState)),
                this, SLOT(onServiceStateChanged(Kend::Service *, Kend::Service::ServiceState)));
    }

    void ServiceManagerNotifierPrivate::check()
    {
        bool newValid = !manager->services(filters).isEmpty();
        if (valid != newValid) {
            valid = newValid;
            emit validityChanged(valid);
        }
    }

    void ServiceManagerNotifierPrivate::onServiceStateChanged(Service *, Service::ServiceState)
    {
        check();
    }




    ServiceManagerNotifier::ServiceManagerNotifier(ServiceManager::Filters filters, QObject * parent)
        : QObject(parent), d(new ServiceManagerNotifierPrivate(this, filters))
    {}

    ServiceManager::Filters ServiceManagerNotifier::filters() const
    {
        return d->filters;
    }

    bool ServiceManagerNotifier::isValid() const
    {
        return d->valid;
    }

    void ServiceManagerNotifier::setFilters(ServiceManager::Filters filters)
    {
        d->filters = filters;
        d->check();
    }

}
