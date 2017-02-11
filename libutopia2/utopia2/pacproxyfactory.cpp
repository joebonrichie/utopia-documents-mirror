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
#include <utopia2/pacproxyfactory.h>
#include <utopia2/pacproxyfactory_p.h>
#include <utopia2/pacscript.h>

#include <boost/scoped_array.hpp>

#include <QAuthenticator>
#include <QByteArray>
#include <QEventLoop>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMutex>
#include <QMutexLocker>
#include <QNetworkAccessManager>
#include <QNetworkProxyQuery>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPushButton>
#include <QRegExp>
#include <QSettings>
#include <QStringList>
#include <QUrl>
#include <QVBoxLayout>
#include <QWaitCondition>

#include <QtDebug>

#if defined(Q_OS_MACX)
#include <SystemConfiguration/SystemConfiguration.h>
#endif

namespace Utopia
{

    namespace
    {
        QByteArray fetchURL(const QUrl & url)
        {
            bool first = true;
            static QNetworkAccessManager manager;
            if (first)
            {
                // Ensure the PAC subsystem doesn't use a proxy
                manager.setProxy(QNetworkProxy::NoProxy);
                first = false;
            }
            QNetworkReply * reply = manager.get(QNetworkRequest(url));
            QEventLoop loop;
            QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
            loop.exec();
            reply->deleteLater();
            return reply->readAll();
        }

#if defined(Q_OS_MACX)
        static int32_t cfnum_to_int32(CFNumberRef num)
        {
            int32_t result;
            CFNumberGetValue(num, kCFNumberSInt32Type, &result);
            return result;
        }
#endif

        QUrl systemPAC()
        {
#if defined(Q_OS_MACX)
            CFDictionaryRef proxyDict = SCDynamicStoreCopyProxies(NULL);
            if (proxyDict)
            {
                CFNumberRef pacEnabled = (CFNumberRef) CFDictionaryGetValue(proxyDict, kSCPropNetProxiesProxyAutoConfigEnable);
                if (pacEnabled && cfnum_to_int32(pacEnabled))
                {
                    CFStringRef pacUrlRef = (CFStringRef) CFDictionaryGetValue(proxyDict, kSCPropNetProxiesProxyAutoConfigURLString);
                    if (pacUrlRef)
                    {
                        const char * pacUrlStr = CFStringGetCStringPtr(pacUrlRef, kCFStringEncodingUTF8);
                        if (pacUrlStr)
                        {
                            QUrl pacUrl = QString::fromUtf8(pacUrlStr);
                            if (pacUrl.isValid())
                            {
                                CFRelease(proxyDict);
                                return pacUrl;
                            }
                        }
                        else
                        {
                            CFIndex len = CFStringGetLength(pacUrlRef);
                            boost::scoped_array< char > pacUrlStr(new char[len * 4 + 1]);
                            if (CFStringGetCString(pacUrlRef, pacUrlStr.get(), len * 4, kCFStringEncodingUTF8))
                            {
                                QUrl pacUrl = QString::fromUtf8(pacUrlStr.get());
                                if (pacUrl.isValid())
                                {
                                    CFRelease(proxyDict);
                                    return pacUrl;
                                }
                            }
                        }
                    }
                }
                CFRelease(proxyDict);
            }
#endif
            return QUrl();
        }
    }

    static QString env(const QString & name)
    {
        return QString(::getenv(name.toUtf8().data()));
    }





    CredentialDialog::CredentialDialog()
        : QDialog(0)
    {
        setWindowTitle("Proxy authentication required...");

        QVBoxLayout * dialogLayout = new QVBoxLayout(this);
        QFormLayout * formLayout = new QFormLayout;
        dialogLayout->addLayout(formLayout);
        QLabel * instructionLabel = new QLabel("<span>Your proxy server requires authentication before it will allow you to access the network. Please enter your credentials:</span>");
        instructionLabel->setWordWrap(true);
        formLayout->addRow(instructionLabel);
        userNameLineEdit = new QLineEdit;
        formLayout->addRow("Username", userNameLineEdit);
        passwordLineEdit = new QLineEdit;
        formLayout->addRow("Password", passwordLineEdit);
        passwordLineEdit->setEchoMode(QLineEdit::Password);
        QHBoxLayout * buttonLayout = new QHBoxLayout;
        dialogLayout->addLayout(buttonLayout);
        buttonLayout->addStretch(1);
        QPushButton * okButton = new QPushButton("OK");
        buttonLayout->addWidget(okButton);
        QPushButton * cancelButton = new QPushButton("Cancel");
        buttonLayout->addWidget(cancelButton);

        connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
        connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

        adjustSize();
    }

    void CredentialDialog::showEvent(QShowEvent * event)
    {
        passwordLineEdit->setText(QString());
    }






    PACProxyFactoryPrivate::PACProxyFactoryPrivate(PACProxyFactory * factory)
        : QObject(factory),
          factory(factory),
          script(0),
          mutex(QMutex::Recursive),
          no_proxy(env("no_proxy").split(QRegExp("[\\s,]+"), QString::SkipEmptyParts))
    {
        QSettings conf;
        conf.beginGroup("Networking");
        conf.beginGroup("Proxies");

        conf.beginGroup("Realms");
        // For each realm, access any saved credentials
        foreach (QString realm, conf.childGroups()) {
            conf.beginGroup(realm);
            QString username(conf.value("username").toString());
            QString password(decryptPassword(username, conf.value("password").toString()));
            bool valid(!username.isEmpty() && !password.isEmpty());
            if (valid) {
                Attempt entry = { username, password, 2 };
                credentialCacheByRealm[QUrl::fromPercentEncoding(realm.toUtf8())] = entry;
            }
            conf.endGroup();
            if (!valid) {
                // If the saved credentials cannot be decrypted properly, remove them entirely
                conf.remove(realm);
            }
        }
        conf.endGroup();

        conf.beginGroup("Hosts");
        // For each realm, access any saved credentials
        foreach (QString host, conf.childGroups()) {
            conf.beginGroup(host);
            QString username(conf.value("username").toString());
            QString password(decryptPassword(username, conf.value("password").toString()));
            bool valid(!username.isEmpty() && !password.isEmpty());
            if (valid) {
                Attempt entry = { username, password, 2 };
                credentialCacheByHost[host] = entry;
            }
            conf.endGroup();
            if (!valid) {
                // If the saved credentials cannot be decrypted properly, remove them entirely
                conf.remove(host);
            }
        }
        conf.endGroup();

        connect(this, SIGNAL(requestNewCredentials(QString, QString)), this, SLOT(doRequestNewCredentials(QString, QString)), Qt::QueuedConnection);

        // Force proxy method
        QString method(env("UTOPIA_PROXY_METHOD"));
        if (method != "")
        {
            conf.setValue("Method", method);
        }
        if (conf.value("Method").toString().isEmpty())
        {
            conf.setValue("Method", QString("SYSTEM"));
        }
    }

    PACProxyFactoryPrivate::~PACProxyFactoryPrivate()
    {}

    void PACProxyFactoryPrivate::doRequestNewCredentials(QString realm, QString host)
    {
#ifdef UTOPIA_BUILD_DEBUG
        qDebug() << "PACProxyFactoryPrivate::doRequestNewCredentials" << realm << host;
#endif
        QMutexLocker guard(&authMutex);

        // FIXME pop up dialog box
        CredentialDialog dialog;
        dialog.exec();

        QString username;
        QString password;

        if (!realm.isEmpty()) {
            credentialCacheByRealm.remove(realm);
        } else {
            credentialCacheByHost.remove(host);
        }

        if (dialog.result() == QDialog::Accepted)
        {
            username = dialog.userNameLineEdit->text();
            password = dialog.passwordLineEdit->text();

            if (!username.isEmpty() && !password.isEmpty()) {
                Attempt entry = { username, password, 0 };
                QSettings conf;
                conf.beginGroup("Networking");
                conf.beginGroup("Proxies");
                if (!realm.isEmpty()) {
                    conf.beginGroup("Realms");
                    conf.beginGroup(QUrl::toPercentEncoding(realm));
                    credentialCacheByRealm[realm] = entry;
#ifdef UTOPIA_BUILD_DEBUG
                    qDebug() << "Adding realm" << realm << "to cache";
#endif
                } else {
                    conf.beginGroup("Hosts");
                    conf.beginGroup(host);
                    credentialCacheByHost[host] = entry;
#ifdef UTOPIA_BUILD_DEBUG
                    qDebug() << "Adding host" << host << "to cache";
#endif
                }
                conf.setValue("username", username);
                conf.setValue("password", encryptPassword(username, password));
            }
        }



        authCondition.wakeAll();
#ifdef UTOPIA_BUILD_DEBUG
        qDebug() << "~PACProxyFactoryPrivate::doRequestNewCredentials";
#endif
    }

    bool PACProxyFactoryPrivate::usingPAC()
    {
        QMutexLocker guard(&mutex);
        // First check for a PAC URL in the conf file, and reload if necessary
        QSettings conf;
        conf.sync();
        conf.beginGroup("Networking");
        conf.beginGroup("Proxies");
        QString method(conf.value("Method").toString());
        QUrl pacURL;

        if (method == "AUTO")
        {
            pacURL = conf.value("PAC").toUrl();
            if (!pacURL.isEmpty())
            {
                // FIXME - enforce periodic reloading perhaps?
                if (pacURL != url)
                {
                    QString scriptContent(fetchURL(pacURL));
                    if (!scriptContent.isEmpty())
                    {
                        //qDebug() << scriptContent;
                        script->setScript(scriptContent);
                        url = script->isValid() ? pacURL : QUrl();
                    }
                }
                return url.isValid();
            }
        }

        if (method == "SYSTEM")
        {
            pacURL = systemPAC();
            if (!pacURL.isEmpty())
            {
                // FIXME - enforce periodic reloading perhaps?
                if (pacURL != url)
                {
                    QString scriptContent(fetchURL(pacURL));
                    if (!scriptContent.isEmpty())
                    {
                        //qDebug() << scriptContent;
                        script->setScript(scriptContent);
                        url = script->isValid() ? pacURL : QUrl();
                    }
                }
                return url.isValid();
            }
        }

        return false;
    }






    PACProxyFactory::PACProxyFactory(PACScript * script)
        : QObject(), QNetworkProxyFactory(), d(new PACProxyFactoryPrivate(this))
    {
        setScript(script);
    }

    PACProxyFactory::~PACProxyFactory()
    {
        if (d->script)
        {
            delete d->script;
        }
    }

    static QUrl envProxy(const QNetworkProxyQuery & query)
    {
        return QUrl(env(query.url().scheme().toLower() + "_proxy"));
    }

    static QUrl confProxy(const QNetworkProxyQuery & query)
    {
        QSettings conf;
        conf.sync();
        conf.beginGroup("Networking");
        conf.beginGroup("Proxies");
        QString protocol = conf.value("Use HTTP Proxy For All Protocols", false).toBool() ? "HTTP" : query.url().scheme().toUpper();
        return QUrl("http://" + conf.value(protocol + " Proxy").toString() + "/");
    }

    void PACProxyFactory::getCredentials(const QString & realm,
                                         const QString & host,
                                         bool refresh,
                                         QString * newUserName,
                                         QString * newPassword)
    {
#ifdef UTOPIA_BUILD_DEBUG
        qDebug() << "getCredentials:" << host << realm << "( refresh =" << refresh << ")";
#endif
        // Bail if return values are not writable
        if (newUserName && newPassword) {
            QMutexLocker guard(&d->authMutex);
            Attempt attempt;

            if (!refresh) {
                if (!realm.isEmpty()) {
                    if (d->credentialCacheByRealm.contains(realm)) {
                        attempt = d->credentialCacheByRealm[realm];
                    }
                } else {
                    if (d->credentialCacheByHost.contains(host)) {
                        attempt = d->credentialCacheByHost[host];
                    }
                }
            } else {
                emit d->requestNewCredentials(realm, host);
                d->authCondition.wait(&d->authMutex);

                // If credentials filled in, then set them
                if (!realm.isEmpty()) {
                    if (d->credentialCacheByRealm.contains(realm)) {
                        attempt = d->credentialCacheByRealm[realm];
                    }
                } else {
                    if (d->credentialCacheByHost.contains(host)) {
                        attempt = d->credentialCacheByHost[host];
                    }
                }
            }

            *newUserName = attempt.user;
            *newPassword = attempt.password;
        }
    }

    void PACProxyFactory::proxyAuthenticationRequired(const QNetworkProxy & proxy, QAuthenticator * authenticator)
    {
        QNetworkAccessManager * manager = static_cast< QNetworkAccessManager * >(sender());
        manager->setProperty("__u_doNotTimeout", true);

        // Find identifying information
        QString realm(authenticator->realm());
        QString host = QString("%1:%2").arg(proxy.hostName()).arg(proxy.port());
#ifdef UTOPIA_BUILD_DEBUG
        qDebug() << "PACProxyFactory::proxyAuthenticationRequired" << realm << host;
#endif
        // Storage for this attempt
        Attempt attempt;

        // If the realm is set, and working credentials are available for this realm
        if (!realm.isEmpty()) {
            if (d->credentialCacheByRealm.contains(realm)) {
#ifdef UTOPIA_BUILD_DEBUG
                qDebug() << "Found realm cache";
#endif
                Attempt & cached = d->credentialCacheByRealm[realm];
                // If proxy never seen before, then just try the credentials
                if (cached.first > 0) {
                    --cached.first;
                    attempt = cached;
#ifdef UTOPIA_BUILD_DEBUG
                    qDebug() << "Using realm cache";
#endif
                }
            }
        } else { // Otherwise key off the proxy's host name and port
            if (d->credentialCacheByHost.contains(host)) {
#ifdef UTOPIA_BUILD_DEBUG
                qDebug() << "Found host cache";
#endif
                Attempt & cached = d->credentialCacheByHost[host];
                // If host name and port not seen before, then try credentials
                if (cached.first > 0) {
                    --cached.first;
                    attempt = cached;
#ifdef UTOPIA_BUILD_DEBUG
                    qDebug() << "Using host cache";
#endif
                }
            }
        }

        // If no valid credentials found in cache, then ask the user for them
        if (!attempt.isValid()) {
            d->doRequestNewCredentials(realm, host);

            // If credentials filled in, then set them
            if (!realm.isEmpty()) {
                if (d->credentialCacheByRealm.contains(realm)) {
                    attempt = d->credentialCacheByRealm[realm];
                }
            } else {
                if (d->credentialCacheByHost.contains(host)) {
                    attempt = d->credentialCacheByHost[host];
                }
            }
        }

        // If credentials set, use them
        if (attempt.isValid()) {
            authenticator->setUser(attempt.user);
            authenticator->setPassword(attempt.password);
        } else { // Otherwise turn off networking
#ifndef Q_OS_LINUX
            manager->setNetworkAccessible(QNetworkAccessManager::NotAccessible);
#endif
        }

        manager->setProperty("__u_doNotTimeout", QVariant());
#ifdef UTOPIA_BUILD_DEBUG
        qDebug() << "~PACProxyFactory::proxyAuthenticationRequired";
#endif
    }

    QList< QNetworkProxy > PACProxyFactory::queryProxy(const QNetworkProxyQuery & query)
    {
#ifdef UTOPIA_BUILD_DEBUG
        qDebug() << "PROXY for" << query.url().toString();
#endif
        QMutexLocker guard(&d->mutex);
        QList< QNetworkProxy > proxies;

        QSettings conf;
        conf.sync();
        conf.beginGroup("Networking");
        conf.beginGroup("Proxies");
        QString method(conf.value("Method").toString());

        if (method != "NONE")
        {
            // Skip proxies entirely?
            QStringList no_proxy;
            if (method == "MANUAL")
            {
                no_proxy = conf.value("Exclude List").toString().split(QRegExp("\\s*[;,]\\s*"), QString::SkipEmptyParts);
            }
            else if (method == "SYSTEM")
            {
                no_proxy = d->no_proxy;
            }
            foreach (QString proxy, no_proxy)
            {
                QString match;
                if (proxy.contains(":"))
                {
                    match = QString("%1:%2").arg(query.peerHostName()).arg(query.peerPort());
                }
                else
                {
                    match = query.peerHostName();
                }
                if (("." + match).endsWith("." + proxy))
                {
#ifdef UTOPIA_BUILD_DEBUG
                    qDebug() << "   -- no_proxy";
#endif
                    proxies.append(QNetworkProxy::NoProxy);
                    break;
                }
            }

            // Protocol specific proxies
            if (proxies.isEmpty())
            {
                QUrl url;
                if (method == "SYSTEM")
                {
                    url = envProxy(query);
                }
                else if (method == "MANUAL")
                {
                    url = confProxy(query);
                }
                if (url.isValid())
                {
#ifdef UTOPIA_BUILD_DEBUG
                    qDebug() << "   -- SYSTEM/MANUAL" << url;
#endif
                    proxies.append(QNetworkProxy(QNetworkProxy::HttpProxy,
                                                 url.host(),
                                                 url.port(0),
                                                 url.userName(),
                                                 url.password()));
                }
            }

            // Proxy Auto Configuration
            if (proxies.isEmpty() && d->usingPAC())
            {
                if (d->script)
                {
                    QString result = script()->findProxyForUrl(query.url().toString(), query.peerHostName());
#ifdef UTOPIA_BUILD_DEBUG
                    qDebug() << "   -- PAC" << result;
#endif
                    QStringList commands = result.simplified().split(";", QString::SkipEmptyParts);
                    QStringListIterator command_iter(commands);
                    while (command_iter.hasNext())
                    {
                        QString command = command_iter.next().simplified();
                        QString method = command.section(" ", 0, 0, QString::SectionSkipEmpty);

                        if (method == "DIRECT")
                        {
                            proxies.append(QNetworkProxy::NoProxy);
                        }
                        else if (!method.isEmpty())
                        {
                            QString address = command.section(" ", 1, 1, QString::SectionSkipEmpty);
                            QString host = address.section(":", 0, 0, QString::SectionSkipEmpty);
                            bool portOk;
                            int port = address.section(":", 1, 1, QString::SectionSkipEmpty).toInt(&portOk);
                            if (!host.isEmpty() && portOk)
                            {
                                QNetworkProxy::ProxyType proxyType = QNetworkProxy::NoProxy;
                                if (method == "PROXY") proxyType = QNetworkProxy::HttpProxy;
                                //else if (method == "SOCKS") proxyType = QNetworkProxy::Socks5Proxy;

                                if (proxyType != QNetworkProxy::NoProxy)
                                {
                                    proxies.append(QNetworkProxy(proxyType, host, port));
                                }
                            }
                        }
                    }
                }
                if (proxies.isEmpty())
                {
                    proxies.append(QNetworkProxy::NoProxy);
                }
            }
            if (proxies.isEmpty() && method == "SYSTEM")
            {
                return QNetworkProxyFactory::systemProxyForQuery(query);
            }
        }

        if (proxies.isEmpty())
        {
            proxies.append(QNetworkProxy::NoProxy);
        }

        return proxies;
    }

    QString PACProxyFactory::queryProxyString(const QNetworkProxyQuery & query)
    {
        QList< QNetworkProxy > proxies = queryProxy(query);
        QString proxyString;

        foreach (const QNetworkProxy & proxy, proxies)
        {
            if (!proxyString.isEmpty()) { proxyString += ";"; }
            switch (proxy.type())
            {
            case QNetworkProxy::NoProxy:
                proxyString += QString("DIRECT");
                break;
            case QNetworkProxy::Socks5Proxy:
                proxyString += QString("SOCKS %1:%2").arg(proxy.hostName()).arg(proxy.port());
                break;
            case QNetworkProxy::HttpProxy:
                proxyString += QString("PROXY %1:%2").arg(proxy.hostName()).arg(proxy.port());
                break;
            case QNetworkProxy::HttpCachingProxy:
            case QNetworkProxy::FtpCachingProxy:
            case QNetworkProxy::DefaultProxy:
                // Ignore
                break;
            }
        }

#ifdef UTOPIA_BUILD_DEBUG
        qDebug() << "    -- to Python..." << proxyString;
#endif
        return proxyString;
    }

    void PACProxyFactory::setScript(PACScript * script)
    {
        if (d->script)
        {
            delete d->script;
        }
        d->script = script;
    }

    PACScript * PACProxyFactory::script() const
    {
        return d->script;
    }

}
