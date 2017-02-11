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

#include <utopia2/auth/servicemanagermodel_p.h>
#include <utopia2/auth/servicemanagermodel.h>

#include <utopia2/auth/service.h>
#include <utopia2/auth/servicemanager.h>

#include <QColor>
#include <QDebug>

Q_DECLARE_METATYPE(QPointer< Kend::Service >);

namespace Kend
{


    ServiceManagerModelPrivate::ServiceManagerModelPrivate(ServiceManagerModel * model)
        : QObject(model), model(model), manager(ServiceManager::instance())
    {
        // Populate model from service manager
        for (int i = 0; i < manager->count(); ++i) {
            Service * service = manager->serviceAt(i);
            services.append(service);
            connectService(service);
        }

        // Connect all signals from manager
        connect(manager.get(), SIGNAL(serviceAdded(Kend::Service*)), this, SLOT(onServiceAdded(Kend::Service*)));
        connect(manager.get(), SIGNAL(serviceRemoved(Kend::Service*)), this, SLOT(onServiceRemoved(Kend::Service*)));
    }

    ServiceManagerModelPrivate::~ServiceManagerModelPrivate()
    {}

    void ServiceManagerModelPrivate::connectService(Service * service)
    {
        connect(service, SIGNAL(descriptionChanged(QString)), this, SLOT(onServiceDescriptionChanged(QString)));
        connect(service, SIGNAL(serviceStateChanged(Kend::Service::ServiceState)), this, SLOT(onServiceStateChanged(Kend::Service::ServiceState)));
        connect(service, SIGNAL(credentialsChanged(QVariantMap)), this, SLOT(onServiceCredentialsChanged(QVariantMap)));
    }

    void ServiceManagerModelPrivate::disconnectService(Service * service)
    {
    }

    void ServiceManagerModelPrivate::onServiceDescriptionChanged(QString name)
    {
        if (Kend::Service * service = qobject_cast< Kend::Service * >(sender())) {
            int row = services.indexOf(service);
            emit model->dataChanged(model->index(row, 1), model->index(row, 1));
        }
    }

    void ServiceManagerModelPrivate::onServiceAdded(Service * service)
    {
        model->beginInsertRows(QModelIndex(), services.size(), services.size());
        services.append(service);
        connectService(service);
        model->endInsertRows();
    }

    void ServiceManagerModelPrivate::onServiceRemoved(Service * service)
    {
        int index = services.indexOf(service);
        if (index >= 0) {
            model->beginRemoveRows(QModelIndex(), index, index);
            services.removeAll(service);
            disconnectService(service);
            model->endRemoveRows();
        }
    }

    void ServiceManagerModelPrivate::onServiceStateChanged(Service::ServiceState state)
    {
        if (Kend::Service * service = qobject_cast< Kend::Service * >(sender())) {
            int row = services.indexOf(service);
            emit model->dataChanged(model->index(row, 0), model->index(row, 2));
        }
    }

    void ServiceManagerModelPrivate::onServiceCredentialsChanged(QVariantMap credentials)
    {
        if (Kend::Service * service = qobject_cast< Kend::Service * >(sender())) {
            int row = services.indexOf(service);
            emit model->dataChanged(model->index(row, 0), model->index(row, 2));
        }
    }

    QVariant ServiceManagerModelPrivate::serviceStateDecoration(Service::ServiceState serviceState)
    {
        switch (serviceState) {
        case Service::StoppedState:
            return QColor(Qt::gray);
        case Service::PopulatingState:
            return QColor(Qt::gray);
        case Service::StartedState:
            return QColor(Qt::green);
        case Service::LoggingInState:
            return QColor(Qt::yellow);
        case Service::LoggingOutState:
            return QColor(Qt::yellow);
        case Service::ErrorState:
            return QColor(Qt::red);
        default:
            return QVariant();
        }
    }

    QVariant ServiceManagerModelPrivate::serviceStateDisplayName(Service * service)
    {
        switch (service->serviceState()) {
        case Service::StoppedState:
            return "Disabled";
        case Service::PopulatingState:
            return "Loading...";
        case Service::StartedState:
            return "Online";
        case Service::LoggingInState:
            return "Logging In...";
        case Service::LoggingOutState:
            return "Logging Out...";
        case Service::ErrorState:
            switch (service->errorCode()) {
            case Service::InvalidCredentials:
                return "Login Failed";
            case Service::AuthenticationServerError:
                return "Server Error";
            case Service::AuthenticationServerInaccessible:
                return "Connection Error";
            default:
                return "Unknown Error";
            }
        default:
            return QVariant();
        }
    }





    ServiceManagerModel::ServiceManagerModel(QObject * parent)
        : QAbstractItemModel(parent), d(new ServiceManagerModelPrivate(this))
    {}

    ServiceManagerModel::~ServiceManagerModel()
    {}

    int ServiceManagerModel::columnCount(const QModelIndex & parent) const
    {
        return parent.isValid() ? 0 : 5;
    }

    QVariant ServiceManagerModel::data(const QModelIndex & index, int role) const
    {
        QVariant v;
        int col = index.column();
        int row = index.row();

        if (index.isValid() && col < 5 && row < d->services.size()) {
            if (QPointer< Service > s = d->services.at(row)) {
                switch (role) {
                case Qt::DisplayRole:
                    switch (col) {
                    case 1: v = s->description(); break;
                    case 2: v = s->isAnonymous() ? QString("Anonymous") : s->credentials().value("user").toString(); break;
                    case 3: v = d->serviceStateDisplayName(s); break;
                    }
                    break;
                case Qt::DecorationRole:
                    if (col == 4) {
                        v = d->serviceStateDecoration(s->serviceState());
                    }
                    break;
                case Qt::CheckStateRole:
                    if (col == 0) {
                        v = s->isEnabled() ? Qt::Checked : Qt::Unchecked;
                    }
                    break;
                case Qt::TextAlignmentRole:
                    v = (int) (col == 3 ? Qt::AlignRight : Qt::AlignLeft) | Qt::AlignVCenter;
                    break;
                case Qt::ForegroundRole:
                    v = QColor((s->serviceState() == Service::StoppedState ||
                         s->serviceState() == Service::ErrorState) ? Qt::gray : Qt::black);
                    break;
                case ServiceRole:
                    v = QVariant::fromValue< QPointer< Service > >(s);
                    break;
                case UuidRole:
                    v = s->uuid().toString();
                    break;
                default:
                    break;
                }
            }
        }

        return v;
    }

    Qt::ItemFlags ServiceManagerModel::flags(const QModelIndex & index) const
    {
        Qt::ItemFlags f = QAbstractItemModel::flags(index);
        if (index.isValid() && index.column() == 0) {
            f |= Qt::ItemIsUserCheckable;
        }
        return f;
    }

    QVariant ServiceManagerModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        return QVariant();
    }

    QModelIndex ServiceManagerModel::index(int row, int column, const QModelIndex & parent) const
    {
        return createIndex(row, column);
    }

    QModelIndex ServiceManagerModel::parent(const QModelIndex & index) const
    {
        return QModelIndex();
    }

    int ServiceManagerModel::rowCount(const QModelIndex & parent) const
    {
        return parent.isValid() ? 0 : d->services.size();
    }

    bool ServiceManagerModel::setData(const QModelIndex & index, const QVariant & value, int role)
    {
        int col = index.column();
        int row = index.row();

        if (role == Qt::CheckStateRole && index.isValid() && col == 0 && row < d->services.size()) {
            if (QPointer< Service > s = d->services.at(row)) {
                switch (static_cast< Qt::CheckState >(value.toInt())) {
                case Qt::Checked:
                    s->setEnabled(true);
                    d->manager->start(s);
                    return true;
                case Qt::Unchecked:
                    if (s->serviceState() == Service::ErrorState) {
                        s->reset();
                    } else {
                        d->manager->stop(s);
                    }
                    s->setEnabled(false);
                    return true;
                default:
                    break;
                }
            }
        }

        return false;
    }

}
