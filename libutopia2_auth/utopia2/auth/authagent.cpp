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

#include <utopia2/encryption.h>
#include <utopia2/auth/authagent.h>
#include <utopia2/auth/authagent_p.h>
#include <utopia2/auth/cJSON.h>
#include <utopia2/auth/service.h>
#include <utopia2/auth/credentialmanager.h>

#include <boost/weak_ptr.hpp>

#include <QDataStream>
#include <QDomDocument>
#include <QMetaType>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSignalMapper>
#include <QStringList>
#include <QTimer>

#include <QtDebug>

Q_DECLARE_METATYPE(Kend::Service *);

namespace Kend
{

    AuthAgentPrivate::AuthAgentPrivate(AuthAgent * agent)
    : QObject(agent), agent(agent)
    {
        // Instantiate a map of credential managers
        std::set< CredentialManager * > loadedCredentialManagers(Utopia::instantiateAllExtensions< CredentialManager >());
        foreach (CredentialManager * credentialManager, loadedCredentialManagers)
        {
            credentialManagers[credentialManager->schema()] = credentialManager;
        }
    }

    AuthAgentPrivate::~AuthAgentPrivate()
    {}

    QNetworkReply * AuthAgentPrivate::get(const QNetworkRequest & request)
    {
        QNetworkReply * reply = networkAccessManager()->get(request);
        registerNetworkReply(reply);
        return reply;
    }

    void AuthAgentPrivate::finished()
    {
        QNetworkReply * reply = static_cast< QNetworkReply * >(sender());
        Service * service = reply->property("__k_service").value< Service * >();
        reply->deleteLater();

        // This is to compensate for what looks like a bug in QNetworkAccessManager, that returns
        // an UnknownNetworkError caused by what looks to be a premature end of file. This HACK
        // just retries (once) if this occurs
        int attempt = service->property("__k_attempt").toInt();
        if (reply->error() == QNetworkReply::UnknownNetworkError && attempt == 0) {
            service->setProperty("__k_attempt", 1);
            agent->logIn(service);
            return;
        } else {
            service->setProperty("__k_attempt", 0);
        }

        // Success? Failure?
        switch (reply->error())
        {
        case QNetworkReply::AuthenticationRequiredError:
            service->setProperty("previousCredentials", QVariant());
            service->setError(Service::InvalidCredentials, "Credentials failed to authenticate");
            agent->logIn(service);
            break;
        case QNetworkReply::NoError:
            if (service->serviceState() == Service::LoggingInState)
            {
                // Parse user URI and token from stream
                QDomDocument output;
                output.setContent(reply);
                QString userURI, authenticationToken;
                QDomNodeList children(output.documentElement().childNodes());
                for (int i = 0; i < children.size(); ++i)
                {
                    if (children.at(i).isElement())
                    {
                        QDomElement element(children.at(i).toElement());
                        if (element.tagName() == "token")
                        {
                            authenticationToken = element.text();
                        }
                        else if (element.tagName() == "uid")
                        {
                            userURI = element.text();
                        }
                    }
                }

                if (!userURI.isEmpty() && !authenticationToken.isEmpty())
                {
                    service->logInComplete(userURI, authenticationToken);
                    break;
                }
            }
            else if (service->serviceState() == Service::LoggingOutState)
            {
                service->logOutComplete();
                break;
            }
            service->setProperty("previousCredentials", QVariant());
            service->setError(Service::InvalidCredentials, "Credentials failed to authenticate");
            //service->setError(Service::AuthenticationServerError, "Authentication server error");
            break;
        case QNetworkReply::TimeoutError:
        case QNetworkReply::OperationCanceledError:
            service->setError(Service::AuthenticationServerInaccessible, "Connection timed out while attempting to authenticate");
            break;
        case QNetworkReply::HostNotFoundError:
            service->setError(Service::AuthenticationServerInaccessible, "Authentication server not found");
            break;
        case QNetworkReply::SslHandshakeFailedError:
            service->setError(Service::AuthenticationServerInaccessible, "Authentication server security error");
            break;
        case QNetworkReply::ContentNotFoundError:
        default:
            service->setError(Service::AuthenticationServerError, QString("Authentication server error (%1)").arg(reply->error()));
            break;
        }
    }

    QNetworkReply * AuthAgentPrivate::post(const QNetworkRequest & request, const QByteArray & data)
    {
        QNetworkReply * reply = networkAccessManager()->post(request, data);
        registerNetworkReply(reply);
        return reply;
    }

    void AuthAgentPrivate::registerNetworkReply(QNetworkReply * reply)
    {
        connect(reply, SIGNAL(finished()), this, SLOT(finished()));
        connect(reply, SIGNAL(sslErrors(const QList< QSslError > &)), this, SLOT(sslErrors(const QList< QSslError > &)));
        //reply->ignoreSslErrors();
    }

    void AuthAgentPrivate::sslErrors(const QList< QSslError > & errors)
    {
        return;

        /* The following can be used to pass back useful error reports
           QNetworkReply * reply = static_cast< QNetworkReply * >(sender());
           foreach(const QSslError & error, errors)
           {
           switch (error.error())
           {
           case QSslError::SelfSignedCertificate:
           case QSslError::SelfSignedCertificateInChain:
           case QSslError::CertificateUntrusted:
           break;
           default:
           break;
           }
           }
        //*/
    }





    AuthAgent::AuthAgent()
    : QObject(0), d(new AuthAgentPrivate(this))
    {}

    void AuthAgent::captureCompleted(Service * service, const QVariantMap & credentials)
    {
        // FIXME conflicts? empty packet?
        if (credentials.isEmpty()) {
            service->setError(Service::InvalidCredentials, "Credentials failed to authenticate");
        } else {
            service->setProperty("previousAuthenticationMethod", service->authenticationMethod());
            service->setCredentials(credentials);
            logIn(service);
        }
    }

    boost::shared_ptr< AuthAgent > AuthAgent::instance()
    {
        static boost::weak_ptr< AuthAgent > singleton;
        boost::shared_ptr< AuthAgent > shared(singleton.lock());
        if (singleton.expired())
        {
            shared = boost::shared_ptr< AuthAgent >(new AuthAgent());
            singleton = shared;
        }
        return shared;
    }

    void AuthAgent::logIn(Service * service)
    {
        // Ensure it is actually sane to log into this service
        QUrl authUrl = service->resourceUrl(Service::AuthenticationResource);
        if (authUrl.isValid()) {
            // Complete signin URL
            authUrl.setPath(authUrl.path().replace(QRegExp("$"), "/signin"));

            // What do we know?
            QVariantMap credentials(service->credentials());
            QString authenticationMethodName(service->authenticationMethod());
            QString previousAuthenticationMethodName(service->property("previousAuthenticationMethod").toString());
            bool isEmpty = credentials.isEmpty();
            bool isError = service->serviceState() == Service::ErrorState;
            bool isInvalidCredentials = isError && service->errorCode() == Service::InvalidCredentials;
            bool isNewAuthenticationMethod = previousAuthenticationMethodName != authenticationMethodName;

            // If required, capture credentials from the correct credential manager
            if (isEmpty || isInvalidCredentials || isNewAuthenticationMethod) {
                // Find appropriate credential manager
                qDebug() << "authenticationMethodName" << authenticationMethodName;
                qDebug() << "authenticationMethods" << d->credentialManagers.keys();
                if (CredentialManager * credentialManager = d->credentialManagers.value(service->supportedAuthenticationMethods().value(authenticationMethodName).toString(), 0)) {
                    credentialManager->captureCredentials(service, this);
                } else {
                    service->setError(Service::UnknownError, "No known credential manager");
                }
            } else if (isError) {
                // Do nothing - the service is in an error state not caused by invalid credentials
            } else {
                QString xml("<?xml version='1.0' encoding='utf-8'?>"
                            "<logininput xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
                            "    xmlns=\"http://utopia.cs.manchester.ac.uk/authd\""
                            "    version=\"0.3\""
                            "    xsi:schemaLocation=\"http://utopia.cs.manchester.ac.uk/authd https://utopia.cs.manchester.ac.uk/authd/0.3/xsd/logininput\">"
                            "  <service>%1</service>"
                            "  %2"
                            "</logininput>");
                QString keyValue("  <credentials key=\"%1\">%2</credentials>");
                QString credentialStr;
                QMapIterator< QString, QVariant > iter(credentials);
                while (iter.hasNext())
                {
                    iter.next();
                    credentialStr += keyValue.arg(iter.key()).arg(iter.value().toString());
                }
                xml = xml.arg(authenticationMethodName, credentialStr);
                QNetworkRequest request(authUrl);
                request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-authd+xml;version=0.3;type=logininput");
                //qDebug() << xml;
                QNetworkReply * reply = d->post(request, xml.toUtf8());
                reply->setProperty("__k_service", QVariant::fromValue(service));
            }
        } else {
            service->setError(Service::UnknownError, "No authentication necessary");
        }
    }

    void AuthAgent::logOut(Service * service)
    {
        // Ensure it is actually sane to log out of this service
        QUrl authUrl = service->resourceUrl(Service::AuthenticationResource);
        if (authUrl.isValid()) {
            // Firstly, log out using the credential manager
            QString currentAuthenticationMethod = service->authenticationMethod();

            // Find appropriate credential manager
            if (CredentialManager * credentialManager = d->credentialManagers.value(service->supportedAuthenticationMethods().value(currentAuthenticationMethod).toString(), 0)) {
                credentialManager->logOut(service, this);
            } else {
                service->setError(Service::UnknownError, "No known credential manager");
            }
        } else {
            service->setError(Service::UnknownError, "No authentication necessary");
        }
    }

    void AuthAgent::logOutCompleted(Service * service)
    {
        QUrl authUrl = service->resourceUrl(Service::AuthenticationResource);
        if (authUrl.isValid())
        {
            // Complete URL
            authUrl.setPath(authUrl.path().replace(QRegExp("$"), "/signout"));
            // [!] API: signout URL

            QNetworkReply * reply = service->post(QNetworkRequest(authUrl), QByteArray(), "application/xml");
            d->registerNetworkReply(reply);
            reply->setProperty("__k_service", QVariant::fromValue(service));
        }
    }

    QStringList AuthAgent::supportedAuthenticationSchemas() const
    {
        QStringList schemas;
        foreach (CredentialManager * credentialManager, d->credentialManagers) {
            schemas << credentialManager->schema();
        }
        return schemas;
    }

}
