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
#include <utopia2/encryption.h>
#include <utopia2/auth/service_p.h>
#include <utopia2/auth/service.h>
#include <utopia2/auth/authagent.h>
#include <utopia2/auth/user.h>

#include <QDateTime>
#include <QDomDocument>
#include <QDomElement>
#include <QEventLoop>
#include <QIODevice>
#include <QMap>
#include <QMetaEnum>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QRegExp>
#include <QSettings>
#include <QSignalMapper>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QUuid>

#include <QtDebug>

namespace Kend
{

    namespace
    {
        static QByteArray encode(const QVariantMap & map)
        {
            QByteArray blob;
            QDataStream stream(&blob, QIODevice::WriteOnly);
            stream.setVersion(QDataStream::Qt_4_6);
            stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
            stream << map;
            return blob;
        }

        static QVariantMap decode(const QByteArray & blob)
        {
            QVariantMap map;
            QDataStream stream(blob);
            stream.setVersion(QDataStream::Qt_4_6);
            stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
            stream >> map;
            return map;
        }
    }




    ServicePrivate::ServicePrivate(Service * service)
    : QObject(service),
      uuid(QUuid::createUuid()),
      service(service),
      serviceState(Service::EmptyState),
      errorCode(Service::Success),
      populated(false),
      anonymous(false),
      enabled(true),
      redirects(0),
      authAgent(AuthAgent::instance())
    {
        init();
    }

    ServicePrivate::ServicePrivate(Service * service, const QUuid & uuid)
    : QObject(service),
      uuid(uuid),
      service(service),
      serviceState(Service::EmptyState),
      errorCode(Service::Success),
      populated(false),
      anonymous(false),
      enabled(true),
      redirects(0),
      authAgent(AuthAgent::instance())
    {
        init();
    }

    QNetworkRequest ServicePrivate::authenticatedRequest(const QNetworkRequest & req_, const QString & mime_type)
    {
        QNetworkRequest req(req_);
        if (service->isLoggedIn() || service->serviceState() == Service::LoggingOutState) {
            req.setRawHeader("Authorization", QString("Kend %1").arg(service->authenticationToken()).toUtf8());
        }
        if (!mime_type.isEmpty()) {
            req.setRawHeader("Content-Type", mime_type.toUtf8());
        }
        return req;
    }

    bool ServicePrivate::changeState(Service::ServiceState desiredState)
    {
        //qDebug() << "SERVICE" << serviceState << desiredState << "--" << service->serviceName();
        if (serviceState == desiredState) {
            return true;
        }

        bool valid = false;

        switch (desiredState) {
        case Service::PopulatingState:
            valid = (serviceState == Service::EmptyState);
            break;
        case Service::StoppedState:
            valid = (serviceState & (Service::PopulatingState | Service::StartedState | Service::ErrorState));
            break;
        case Service::StartedState:
            valid = (serviceState & (Service::StoppedState | Service::LoggingInState | Service::LoggingOutState));
            break;
        case Service::LoggingInState:
            valid = (serviceState & Service::StartedState);
            break;
        case Service::LoggingOutState:
            valid = (serviceState & Service::StartedState);
            break;
        case Service::ErrorState:
            valid = true;
            break;
        default:
            break;
        }

        if (valid) {
            serviceState = desiredState;
            emit serviceStateChanged(serviceState);

            switch (serviceState) {
            case Service::PopulatingState:
                emit servicePopulating();
                break;
            case Service::StoppedState:
                emit serviceStopped();
                break;
            case Service::StartedState:
                emit serviceStarted();
                break;
            case Service::LoggingInState:
                emit serviceLoggingIn();
                break;
            case Service::LoggingOutState:
                emit serviceLoggingOut();
                break;
            case Service::ErrorState:
                emit serviceError();
                break;
            default:
                break;
            }
        }

        return valid;
    }

    void ServicePrivate::clear()
    {
        setServiceName(QString());
        resourceUrls.clear();
        resourceCapabilities.clear();
    }

    bool ServicePrivate::clearCache()
    {
        QSettings conf;
        conf.remove("Services/Cache/" + QUrl::toPercentEncoding(serviceUrl.toString()));
        return true;
    }

    QNetworkReply * ServicePrivate::get(const QNetworkRequest & request)
    {
        QNetworkReply * reply = networkAccessManager()->get(request);
        connect(reply, SIGNAL(finished()), this, SLOT(finished()));
        return reply;
    }

    QNetworkReply * ServicePrivate::options(const QNetworkRequest & request)
    {
        QNetworkReply * reply = networkAccessManager()->sendCustomRequest(request, "OPTIONS");
        connect(reply, SIGNAL(finished()), this, SLOT(finished()));
        return reply;
    }

    void ServicePrivate::finished()
    {
        // Get reply and schedule for deletion
        QNetworkReply * reply = static_cast< QNetworkReply * >(sender());
        reply->deleteLater();

        // If this is a redirect, follow it transparently, but only to a maximum of (arbitrarily)
        // four redirections
        QUrl redirectedUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        if (redirectedUrl.isValid()) {
            if (redirectedUrl.isRelative()) {
                QString redirectedAuthority = redirectedUrl.authority();
                redirectedUrl = reply->url().resolved(redirectedUrl);
                if (!redirectedAuthority.isEmpty()) {
                    redirectedUrl.setAuthority(redirectedAuthority);
                }
            }
            if (redirects++ < 4) {
                QNetworkRequest request = reply->request();
                request.setUrl(redirectedUrl);
                if (reply->operation() == QNetworkAccessManager::CustomOperation) {
                    options(request);
                } else {
                    get(request);
                }
                return;
            } else {
                setError(Service::ServiceServerInaccessible, "Cannot resolve service URL (too many redirects)");
            }
        }

        // Reset redirect count and make sure those who care deal with this reply finishing
        redirects = 0;
        emit replyFinished();

        // Bail if the service is now in an error state
        if (serviceState == Service::ErrorState) {
            return;
        }

        // Depending on the reply's error state
        switch (reply->error()) {
        case QNetworkReply::NoError:
        {
            QDomDocument doc;
            doc.setContent(reply);
            QDomElement docElem = doc.documentElement();

            bool done = false;

            if (resourceUrls.size() == 0 && reply->operation() == QNetworkAccessManager::GetOperation) {
                if (docElem.tagName() == "services" && docElem.attribute("version") == "0.7") {
                    QDomElement serviceElem = docElem.firstChildElement("service");
                    if (!serviceElem.isNull()) {
                        setServiceName(serviceElem.attribute("name"));
                        QDomElement resourceElem = serviceElem.firstChildElement("resource");
                        while (!resourceElem.isNull()) {
                            Service::ResourceType type = resourceNames.value(resourceElem.attribute("type"), Service::InvalidResourceType);
                            if (type != Service::InvalidResourceType) {
                                QUrl url(resourceElem.attribute("href"));
                                url.setPath(url.path().replace(QRegExp("/+$"), ""));
                                resourceUrls[type] = url;
                                resourceCapabilities[type] = resourceElem.attribute("capabilities").split(QRegExp("\\s+"), QString::SkipEmptyParts);
                            }
                            resourceElem = resourceElem.nextSiblingElement("resource");
                        }
                    }
                }
                if (!resourceUrls.isEmpty()) {
                    if (resourceUrls.contains(Service::AuthenticationResource)) {
                        options(QNetworkRequest(resourceUrls[Service::AuthenticationResource]));
                    } else {
                        done = true;
                    }
                } else {
                    setError(Service::ServiceServerError, "Cannot parse service document");
                }
            } else if (reply->operation() == QNetworkAccessManager::CustomOperation) {
                if (docElem.tagName() == "options" && docElem.attribute("version") == "0.3") {
                    QDomElement backendElem = docElem.firstChildElement("backend");
                    while (!backendElem.isNull()) {
                        QString name(backendElem.attribute("name"));
                        QString schema(backendElem.attribute("schema"));
                        QStringList capabilities(backendElem.attribute("capabilities").split(QRegExp("\\s+"), QString::SkipEmptyParts));
                        QString description;
                        QDomElement descriptionElem = docElem.firstChildElement("description");
                        if (!descriptionElem.isNull()) {
                            description = descriptionElem.text();
                        }
                        QVariantMap terminology;
                        QDomElement terminologyElem = docElem.firstChildElement("terminology");
                        if (!terminologyElem.isNull()) {
                            QDomElement termElem = docElem.firstChildElement("term");
                            while (!termElem.isNull()) {
                                QString key(termElem.attribute("key"));
                                QString val(termElem.text());
                                if (!key.isEmpty()) {
                                    terminology[key] = val;
                                    termElem = termElem.nextSiblingElement("term");
                                }
                            }
                        }
                        AuthBackend backend = {name, schema, capabilities, description, terminology};
                        availableAuthenticationMethods.insert(name, backend);
                        backendElem = backendElem.nextSiblingElement("backend");
                    }
                }
                if (!availableAuthenticationMethods.isEmpty()) {
                    done = true;
                } else {
                    setError(Service::ServiceServerError, "Cannot parse authentication options");
                }
            }

            if (done) {
                populated = true;
                saveToCache();
                changeState(Service::StoppedState);
            }
            break;
        }
        case QNetworkReply::TimeoutError:
        case QNetworkReply::OperationCanceledError:
            setError(Service::ServiceServerInaccessible, "Connection timed out");
            break;
        default:
            setError(Service::ServiceServerInaccessible, "Cannot resolve service URL");
            break;
        }
    }

    void ServicePrivate::init()
    {
        // Pass on state change signals
        connect(this, SIGNAL(logInComplete(QString, QString)),
                service, SIGNAL(newAuthenticationToken(QString, QString)));
        connect(this, SIGNAL(servicePopulating()),
                service, SIGNAL(servicePopulating()));
        connect(this, SIGNAL(serviceStarted()),
                service, SIGNAL(serviceStarted()));
        connect(this, SIGNAL(serviceLoggingIn()),
                service, SIGNAL(serviceLoggingIn()));
        connect(this, SIGNAL(serviceLoggingOut()),
                service, SIGNAL(serviceLoggingOut()));
        connect(this, SIGNAL(serviceStopped()),
                service, SIGNAL(serviceStopped()));
        connect(this, SIGNAL(serviceError()),
                service, SIGNAL(serviceError()));
        connect(this, SIGNAL(serviceNameChanged(QString)),
                service, SIGNAL(serviceNameChanged(QString)));
        connect(this, SIGNAL(serviceStateChanged(Kend::Service::ServiceState)),
                service, SIGNAL(serviceStateChanged(Kend::Service::ServiceState)));

        // Populate resource name map
        resourceNames["annotations"] = Service::AnnotationsResource;
        resourceNames["documents"] = Service::DocumentsResource;
        resourceNames["lookup"] = Service::DefinitionsResource;
        resourceNames["auth"] = Service::AuthenticationResource;
    }

    void ServicePrivate::onLogInComplete(QString userURI, QString authenticationToken)
    {
        emit logInComplete(userURI, authenticationToken);
        changeState(Service::StartedState);
    }

    void ServicePrivate::onLogOutComplete()
    {
        changeState(Service::StartedState);
    }

    bool ServicePrivate::populateFromUrl(bool wait)
    {
        get(QNetworkRequest(serviceUrl));
        if (wait) {
            QEventLoop loop;
            QObject::connect(this, SIGNAL(serviceError()), &loop, SLOT(quit()));
            QObject::connect(this, SIGNAL(serviceStopped()), &loop, SLOT(quit()));
            loop.exec();
        }
        return errorCode == Service::Success;
    }

    bool ServicePrivate::populateFromCache()
    {
        if (!populated) {
            QSettings conf;
            conf.beginGroup("Services");
            conf.beginGroup("Cache");
            QString encodedUrl(QUrl::toPercentEncoding(serviceUrl.toString()));
            if (conf.childGroups().contains(encodedUrl)) {
                conf.beginGroup(encodedUrl);
                QDateTime lastReloaded(conf.value("lastReloaded").toDateTime());
                if (service->serviceName().isEmpty()) {
                    setServiceName(conf.value("serviceName").toString());
                }
                // The following code could provide auto-reload on timeout
                // if (lastReloaded.isValid() && lastReloaded.daysTo(QDateTime::currentDateTime()) < 4)
                {
                    const QMetaObject * metaObject(service->metaObject());
                    QMetaEnum metaEnum(metaObject->enumerator(metaObject->indexOfEnumerator("ResourceType")));
                    for (int i = 0; i < metaEnum.keyCount(); ++i) {
                        if ((Service::ResourceType) metaEnum.value(i) != Service::InvalidResourceType) {
                            conf.beginGroup(metaEnum.key(i));
                            //qDebug() << "group" << metaEnum.key(i) << conf.value("url").toUrl();
                            QUrl url(conf.value("url").toUrl());
                            url.setPath(url.path().replace(QRegExp("/+$"), ""));
                            resourceUrls[(Service::ResourceType) metaEnum.value(i)] = url;
                            resourceCapabilities[(Service::ResourceType) metaEnum.value(i)] = conf.value("capabilities").toStringList();
                            if ((Service::ResourceType) metaEnum.value(i) == Service::AuthenticationResource) {
                                int size = conf.beginReadArray("backends");
                                for (int i = 0; i < size; ++i) {
                                    conf.setArrayIndex(i);
                                    AuthBackend backend = {
                                        conf.value("name").toString(),
                                        conf.value("schema").toString(),
                                        conf.value("capabilities").toStringList(),
                                        conf.value("description").toString(),
                                        conf.value("terminology").toMap()
                                    };
                                    availableAuthenticationMethods[backend.name] = backend;
                                }
                                conf.endArray();
                            }
                            conf.endGroup();
                        }
                    }

                    populated = true;
                }
            }
        }

        return populated;
    }

    bool ServicePrivate::saveToCache()
    {
        clearCache();

        // Resolve cache timings / timeouts

        QSettings conf;
        conf.beginGroup("Services");
        conf.beginGroup("Cache");

        conf.beginGroup(QUrl::toPercentEncoding(serviceUrl.toString()));
        const QMetaObject * metaObject(service->metaObject());
        QMetaEnum metaEnum(metaObject->enumerator(metaObject->indexOfEnumerator("ResourceType")));
        QMapIterator< Service::ResourceType, QUrl > resources(resourceUrls);
        while (resources.hasNext()) {
            resources.next();
            const char* key = metaEnum.valueToKey((int) resources.key());
            conf.beginGroup(key);
            // The encoding here is to fix a bug on OS X that incorrectly stores a QVariant URL
            conf.setValue("url", QString(resources.value().toEncoded()));
            conf.setValue("capabilities", resourceCapabilities.value(resources.key()));
            if (resources.key() == Service::AuthenticationResource) {
                conf.beginWriteArray("backends");
                int i = 0;
                foreach (const AuthBackend & backend, availableAuthenticationMethods.values()) {
                    conf.setArrayIndex(i++);
                    conf.setValue("name", backend.name);
                    conf.setValue("schema", backend.schema);
                    conf.setValue("capabilities", backend.capabilities);
                    conf.setValue("description", backend.description);
                    conf.setValue("terminology", backend.terminology);
                }
                conf.endArray();
            }
            conf.endGroup();
        }
        conf.setValue("serviceName", serviceName);
        conf.setValue("lastReloaded", QDateTime::currentDateTime());

        return true;
    }

    bool ServicePrivate::setError(Service::ServiceError newErrorCode, const QString & newErrorString)
    {
        errorCode = newErrorCode;
        errorString = newErrorString;
        qDebug() << "~~~ ERROR" << errorCode << errorString;
        changeState(Service::ErrorState);
        return true;
    }

    void ServicePrivate::setServiceName(const QString & newServiceName)
    {
        if (serviceName != newServiceName) {
            serviceName = newServiceName;
            emit serviceNameChanged(serviceName);
        }

        if (service->description().isNull()) {
            service->setDescription(serviceName);
        }
    }






    Service::Service(QObject * parent)
    : QObject(parent), d(new ServicePrivate(this))
    {}

    Service::Service(const QUuid & uuid, QObject * parent)
    : QObject(parent), d(new ServicePrivate(this, uuid))
    {}

    Service::~Service()
    {}

    QString Service::authenticationToken() const
    {
        return d->authenticationToken;
    }

    QByteArray Service::cacheCredentials() const
    {
        return Utopia::encrypt(encode(credentials()), uuid().toString());
    }

    QString Service::authenticationMethod() const
    {
        if (supportedAuthenticationMethods().contains(d->authenticationMethod)) {
            return d->authenticationMethod;
        } else {
            return QString();
        }
    }

    QVariantMap Service::credentials() const
    {
        return d->credentials;
    }

    QNetworkReply * Service::deleteResource(const QNetworkRequest & req)
    {
        return d->networkAccessManager()->deleteResource(d->authenticatedRequest(req));
    }

    QString Service::description() const
    {
        return d->description;
    }

    Service::ServiceError Service::errorCode() const
    {
        return d->errorCode;
    }

    QString Service::errorString() const
    {
        return d->errorString;
    }

    QNetworkReply * Service::get(const QNetworkRequest & req)
    {
        return d->networkAccessManager()->get(d->authenticatedRequest(req));
    }

    bool Service::isAnonymous() const
    {
        return d->anonymous;
    }

    bool Service::isEnabled() const
    {
        return d->enabled;
    }

    bool Service::isLoggedIn() const
    {
        return d->serviceState == StartedState && !d->authenticationToken.isEmpty();
    }

    bool Service::loadFrom(QSettings & conf)
    {
        conf.beginGroup("properties");
        QStringListIterator propertyNames(conf.childKeys());
        while (propertyNames.hasNext()) {
            QString encodedKey = propertyNames.next();
            QString key = QUrl::fromPercentEncoding(encodedKey.toUtf8());
            QVariant value = conf.value(encodedKey);
            if (value.isValid()) {
                //qDebug() << "---> loadFrom" << key.toUtf8().constData() << "=" << value;
                setProperty(key.toUtf8().constData(), value);
            }
        }
        conf.endGroup();

        return true;
    }

    bool Service::logIn(bool wait)
    {
        // FIXME wait
        if (resourceUrl(AuthenticationResource).isValid() && d->changeState(LoggingInState)) {
            d->authAgent->logIn(this);
            return true;
        } else {
            return false;
        }
    }

    void Service::logInComplete(const QString & userURI, const QString & authenticationToken)
    {
        setUserURI(userURI);
        setAuthenticationToken(authenticationToken);
        //qDebug() << "newAuthenticationToken" << userURI << authenticationToken;
        emit newAuthenticationToken(userURI, authenticationToken);

        if (!d->changeState(StartedState)) {
            d->setError(Service::UnknownError, "Error while logging in");
        }
    }

    bool Service::logOut(bool wait)
    {
        // FIXME wait
        if (resourceUrl(AuthenticationResource).isValid() && d->changeState(LoggingOutState)) {
            d->authAgent->logOut(this);
            return true;
        } else {
            return false;
        }
    }

    void Service::logOutComplete()
    {
        setAuthenticationToken(QString());
        setUserURI(QString());
        //qDebug() << "logged out!";
        if (!d->changeState(StartedState)) {
            d->setError(Service::UnknownError, "Error while logging out");
        }
    }

    QNetworkReply * Service::post(const QNetworkRequest & req, QIODevice * data, const QString & mime_type)
    {
        return d->networkAccessManager()->post(d->authenticatedRequest(req, mime_type), data);
    }

    QNetworkReply * Service::post(const QNetworkRequest & req, const QByteArray & data, const QString & mime_type)
    {
        return d->networkAccessManager()->post(d->authenticatedRequest(req, mime_type), data);
    }

    QNetworkReply * Service::put(const QNetworkRequest & req, QIODevice * data, const QString & mime_type)
    {
        return d->networkAccessManager()->put(d->authenticatedRequest(req, mime_type), data);
    }

    QNetworkReply * Service::put(const QNetworkRequest & req, const QByteArray & data, const QString & mime_type)
    {
        return d->networkAccessManager()->put(d->authenticatedRequest(req, mime_type), data);
    }

    QStringList Service::resourceCapabilities(ResourceType type) const
    {
        if (type == AuthenticationResource) {
            return d->availableAuthenticationMethods.value(authenticationMethod()).capabilities;
        } else {
            return d->resourceCapabilities.value(type);
        }
    }

    QUrl Service::resourceUrl(ResourceType type) const
    {
        return d->resourceUrls.value(type);
    }

    bool Service::reset()
    {
        bool success = d->changeState(StoppedState);

        // Reset service
        d->errorCode = Success;
        d->errorString = QString();

        return success;
    }

    bool Service::saveTo(QSettings & conf)
    {
        conf.beginGroup("properties");
        QListIterator< QByteArray > propertyNames(dynamicPropertyNames());
        while (propertyNames.hasNext()) {
            QByteArray key = propertyNames.next();
            if (!key.startsWith("_")) {
                QVariant value = property(key.constData());
                switch (value.type()) {
                // Special hack for QUrls that aren't properly serialised on OS X
                case QVariant::Url:
                    conf.setValue(QUrl::toPercentEncoding(key), QString(value.toUrl().toEncoded()));
                    break;
                default:
                    conf.setValue(QUrl::toPercentEncoding(key), value);
                    break;
                }
                //qDebug() << "---> saveTo" << key << "=" << value;
            }
        }
        const QMetaObject * metaObject(this->metaObject());
        for (int index = 0; index < metaObject->propertyCount(); ++index) {
            QMetaProperty metaProperty(metaObject->property(index));
            // Only save properties I can/should write back later
            if (metaProperty.isWritable() && metaProperty.isStored() && metaProperty.name()[0] != '_') {
                QVariant value(metaProperty.read(this));
                if (!value.isNull()) {
                    switch (value.type()) {
                    // Special hack for QUrls that aren't properly serialised on OS X
                    case QVariant::Url:
                        conf.setValue(QUrl::toPercentEncoding(metaProperty.name()), QString(value.toUrl().toEncoded()));
                        break;
                    default:
                        conf.setValue(QUrl::toPercentEncoding(metaProperty.name()), value);
                        break;
                    }
                    //qDebug() << "---> saveTo" << metaProperty.name() << "=" << value;
                }
            }
        }
        conf.endGroup();

        return true;
    }

    QString Service::serviceName() const
    {
        return d->serviceName;
    }

    Service::ServiceState Service::serviceState() const
    {
        return d->serviceState;
    }

    void Service::setAnonymous(bool anonymous)
    {
        if (d->anonymous != anonymous) {
            if (isEnabled()) {
                if (anonymous) {
                    if (isLoggedIn()) {
                        logOut();
                    }
                } else if (!isLoggedIn()) {
                    logIn();
                }
            }
            d->anonymous = anonymous;
            emit anonymityChanged(anonymous);
        }
    }

    void Service::setAuthenticationMethod(const QString & authenticationMethod)
    {
        d->authenticationMethod = authenticationMethod;
    }

    void Service::setAuthenticationToken(const QString & authenticationToken)
    {
        d->authenticationToken = authenticationToken;
    }

    void Service::setCredentials(const QVariantMap & credentials)
    {
        if (d->credentials != credentials) {
            d->credentials = credentials;
            emit credentialsChanged(credentials);
        }
    }

    void Service::setDescription(const QString & description)
    {
        if (d->description != description) {
            d->description = description;
            emit descriptionChanged(description);
        }
    }

    void Service::setEnabled(bool enabled)
    {
        d->enabled = enabled;
    }

    void Service::setError(ServiceError errorCode, const QString & errorString)
    {
        d->setError(errorCode, errorString);
    }

    bool Service::setUrl(const QUrl & serviceUrl, bool reload)
    {
        if (d->serviceUrl != serviceUrl) {
            if (d->changeState(PopulatingState)) {
                d->serviceUrl = serviceUrl;
                if (reload || !d->populateFromCache()) {
                    d->populateFromUrl(true);
                }
                return d->changeState(StoppedState);
            } else {
                d->setError(Service::UnknownError, "Service URL cannot be changed");
            }
        }
        return false;
    }

    void Service::setUserURI(const QString & userURI)
    {
        d->userURI = userURI;
    }

    bool Service::start(bool /* wait */)
    {
        return d->changeState(StartedState);
    }

    bool Service::stop(bool /* wait */)
    {
        // FIXME clear out data?
        return d->changeState(StoppedState);
    }

    QVariantMap Service::supportedAuthenticationMethods() const
    {
        QVariantMap supportedAuthenticationMethods;
        QStringList supportedAuthenticationSchemas(d->authAgent->supportedAuthenticationSchemas());
        foreach (const AuthBackend & backend, d->availableAuthenticationMethods.values()) {
            if (supportedAuthenticationSchemas.contains(backend.schema)) {
                supportedAuthenticationMethods.insert(backend.name, backend.schema);
            }
        }
        return supportedAuthenticationMethods;
    }

    void Service::uncacheCredentials(const QByteArray & encoded)
    {
        setCredentials(decode(Utopia::decrypt(encoded, uuid().toString())));
    }

    QUrl Service::url() const
    {
        return d->serviceUrl;
    }

    User * Service::user(const QString & id)
    {
        return new User(this, id.isEmpty() ? d->userURI : id);
    }

    QString Service::userURI() const
    {
        return d->userURI;
    }

    QUuid Service::uuid() const
    {
        return d->uuid;
    }

}
