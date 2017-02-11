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

#ifndef KEND_KENDSERVICE_P_H
#define KEND_KENDSERVICE_P_H

#include <utopia2/auth/authagent.h>
#include <utopia2/auth/service.h>
#include <utopia2/networkaccessmanager.h>

#include <QDateTime>
#include <QMap>
#include <QNetworkRequest>
#include <QObject>
#include <QPointer>
#include <QStringList>
#include <QTimer>
#include <QUrl>

class QNetworkReply;
class QNetworkRequest;
class QUuid;

namespace Kend
{

    typedef struct
    {
        QString name;
        QString schema;
        QStringList capabilities;
        QString description;
        QVariantMap terminology;
    } AuthBackend;

    class ServicePrivate : public QObject, public Utopia::NetworkAccessManagerMixin
    {
        Q_OBJECT

    public:
        ServicePrivate(Service * service);
        ServicePrivate(Service * service, const QUuid & uuid);

        QNetworkRequest authenticatedRequest(const QNetworkRequest & req, const QString & mime_type = QString());
        bool changeState(Service::ServiceState desiredState);
        void clear();
        bool clearCache();
        QNetworkReply * get(const QNetworkRequest & request);
        QNetworkReply * options(const QNetworkRequest & request);
        void init();
        bool populateFromUrl(bool block);
        bool populateFromCache();
        bool saveToCache();
        bool setError(Service::ServiceError errorCode, const QString & errorString);
        void setServiceName(const QString & serviceName);

        QUuid uuid;
        Service * service;
        Service::ServiceState serviceState;
        QUrl serviceUrl;
        QMap< QString, Service::ResourceType > resourceNames;
        QMap< Service::ResourceType, QUrl > resourceUrls;
        QMap< Service::ResourceType, QStringList > resourceCapabilities;
        QMap< QString, AuthBackend > availableAuthenticationMethods;
        Service::ServiceError errorCode;
        QString errorString;
        QString description;
        QString serviceName;
        QDateTime lastReload;

        bool populated;
        bool anonymous;
        bool enabled;
        QString authenticationToken;
        QString authenticationMethod;
        QVariantMap credentials;
        QString userURI;

        int redirects;

        boost::shared_ptr< AuthAgent > authAgent;

    public slots:
        void finished();

        void onLogInComplete(QString userURI, QString authenticationToken);
        void onLogOutComplete();

    signals:
        void logInComplete(QString userURI, QString authenticationToken);
        void replyFinished();
        void serviceError();
        void serviceLoggingIn();
        void serviceLoggingOut();
        void serviceNameChanged(QString name);
        void servicePopulating();
        void serviceStarted();
        void serviceStateChanged(Kend::Service::ServiceState newState);
        void serviceStopped();

    };

}

#endif // KEND_KENDSERVICE_P_H
