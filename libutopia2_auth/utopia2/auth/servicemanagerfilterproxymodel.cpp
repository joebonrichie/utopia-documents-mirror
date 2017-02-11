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

#include <utopia2/auth/servicemanagerfilterproxymodel.h>

#include <utopia2/auth/servicemanagermodel.h>

#include <QPointer>

Q_DECLARE_METATYPE(QPointer< Kend::Service >);

namespace Kend
{

    class ServiceManagerFilterProxyModelPrivate
    {
    public:
        boost::shared_ptr< ServiceManager > manager;
        ServiceManager::Filters filters;
    };




    ServiceManagerFilterProxyModel::ServiceManagerFilterProxyModel(ServiceManager::Filters filters, QObject * parent)
        : QSortFilterProxyModel(parent), d(new ServiceManagerFilterProxyModelPrivate)
    {
        d->manager = ServiceManager::instance();
        setFilters(filters);
        setDynamicSortFilter(true);
    }

    ServiceManagerFilterProxyModel::~ServiceManagerFilterProxyModel()
    {}

    ServiceManager::Filters ServiceManagerFilterProxyModel::filters() const
    {
        return d->filters;
    }

    void ServiceManagerFilterProxyModel::setFilters(ServiceManager::Filters filters)
    {
        d->filters = filters;
    }

    bool ServiceManagerFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex & sourceParent) const
    {
        QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
        QVariant variant = sourceModel()->data(index, ServiceManagerModel::ServiceRole);
        QPointer< Service > service = variant.value< QPointer< Service > >();
        return service && d->manager->matches(service, d->filters);
    }

}

