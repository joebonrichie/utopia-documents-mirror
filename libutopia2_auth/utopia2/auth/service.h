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

#ifndef KEND_SERVICE_H
#define KEND_SERVICE_H

#include <utopia2/auth/config.h>

#include <QByteArray>
#include <QMap>
#include <QObject>
#include <QString>
#include <QUrl>
#include <QUuid>
#include <QVariant>

class QSettings;

class QIODevice;
class QNetworkReply;
class QNetworkRequest;

namespace Kend
{

    class AuthAgentPrivate;
    class User;

    class ServicePrivate;
    class LIBUTOPIA_AUTH_API Service : public QObject
    {
        Q_OBJECT
        Q_PROPERTY(QString description
                   READ description
                   WRITE setDescription
                   NOTIFY descriptionChanged)
        Q_PROPERTY(QString serviceName
                   READ serviceName
                   NOTIFY serviceNameChanged
                   STORED false)
        Q_PROPERTY(Service::ServiceState serviceState
                   READ serviceState
                   NOTIFY serviceStateChanged
                   STORED false);
        Q_PROPERTY(QUrl serviceUrl
                   READ url
                   WRITE setUrl)
        Q_PROPERTY(QUuid serviceUuid
                   READ uuid
                   STORED false)
        Q_PROPERTY(bool anonymous
                   READ isAnonymous
                   WRITE setAnonymous)
        Q_PROPERTY(QString userURI
                   READ userURI
                   WRITE setUserURI)
        Q_PROPERTY(QString authenticationToken
                   READ authenticationToken
                   WRITE setAuthenticationToken
                   STORED false)
        Q_PROPERTY(QString authenticationMethod
                   READ authenticationMethod
                   WRITE setAuthenticationMethod)
        Q_PROPERTY(bool enabled
                   READ isEnabled
                   WRITE setEnabled)
        Q_PROPERTY(QVariantMap credentials
                   READ credentials
                   WRITE setCredentials
                   NOTIFY credentialsChanged
                   STORED false)
        Q_PROPERTY(QByteArray cachedCredentials
                   READ cacheCredentials
                   WRITE uncacheCredentials)

    public:
        typedef enum
        {
            InvalidResourceType,
            AnnotationsResource,
            AuthenticationResource,
            DefinitionsResource,
            DocumentsResource
        } ResourceType;
        Q_ENUMS(ResourceType);

        typedef enum
        {
            EmptyState      = 0x0000,
            PopulatingState = 0x0001,
            StoppedState    = 0x0002,
            StartedState    = 0x0008,
            LoggingInState  = 0x0020,
            LoggingOutState = 0x0040,
            ErrorState      = 0x1000
        } ServiceState;
        Q_ENUMS(ServiceState);

        typedef enum
        {
            Success,
            ServiceServerError,
            ServiceServerInaccessible,
            InvalidCredentials,
            AuthenticationServerError,
            AuthenticationServerInaccessible,
            UnknownError
        } ServiceError;
        Q_ENUMS(ServiceError);

        Service(QObject * parent = 0);
        Service(const QUuid & uuid, QObject * parent = 0);
        ~Service();

        // Set up
        bool setUrl(const QUrl & serviceUrl, bool reload = false);
        QUrl url() const;
        QUuid uuid() const;

        // Query service
        QString description() const;
        ServiceError errorCode() const;
        QString errorString() const;
        bool isAnonymous() const;
        bool isEnabled() const;
        QStringList resourceCapabilities(ResourceType type) const;
        QUrl resourceUrl(ResourceType type) const;
        QString serviceName() const;
        ServiceState serviceState() const;
        void setAnonymous(bool anonymous);
        void setDescription(const QString & description);
        void setEnabled(bool enabled);
        void setError(ServiceError errorCode, const QString & errorString = QString());

        // Account information
        QString authenticationToken() const;
        QString authenticationMethod() const;
        QVariantMap credentials() const;
        bool isLoggedIn() const;
        void setAuthenticationMethod(const QString & authenticationMethod);
        void setAuthenticationToken(const QString & authenticationToken);
        void setCredentials(const QVariantMap & credentials);
        void setUserURI(const QString & userURI);
        QVariantMap supportedAuthenticationMethods() const;
        QString userURI() const;
        User * user(const QString & id = QString());

        // Storage
        bool saveTo(QSettings & conf);
        bool loadFrom(QSettings & conf);

        // Authenticated requests
        QNetworkReply * deleteResource(const QNetworkRequest & req);
        QNetworkReply * get(const QNetworkRequest & req);
        QNetworkReply * post(const QNetworkRequest & req, QIODevice * data, const QString & mime_type = QString());
        QNetworkReply * post(const QNetworkRequest & req, const QByteArray & data = QByteArray(), const QString & mime_type = QString());
        QNetworkReply * put(const QNetworkRequest & req, QIODevice * data, const QString & mime_type = QString());
        QNetworkReply * put(const QNetworkRequest & req, const QByteArray & data = QByteArray(), const QString & mime_type = QString());

    public slots:
        // State machine
        bool logIn(bool wait = false);
        bool logOut(bool wait = false);
        bool reset();
        bool start(bool wait = false);
        bool stop(bool wait = false);

    protected slots:
        void logInComplete(const QString & userURI, const QString & authenticationToken);
        void logOutComplete();
        friend class AuthAgentPrivate;

    signals:
        void anonymityChanged(bool anon);
        void credentialsChanged(QVariantMap credentials);
        void descriptionChanged(QString description);
        void newAuthenticationToken(QString userURI, QString authenticationToken);
        void serviceStateChanged(Kend::Service::ServiceState state);
        void serviceNameChanged(QString serviceName);

        void serviceError();
        void serviceLoggingIn();
        void serviceLoggingOut();
        void servicePopulating();
        void serviceStarted();
        void serviceStopped();

    protected:
        ServicePrivate * d;

        QByteArray cacheCredentials() const;
        void uncacheCredentials(const QByteArray & encoded);
    };

}

#endif // KEND_SERVICE_H
