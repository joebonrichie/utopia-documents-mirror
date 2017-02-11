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
#include <utopia2/initializer.h>
#include <utopia2/auth/service.h>
#include <utopia2/auth/servicemanager.h>

#include <utopia2/extension.h>
#include <utopia2/extensionlibrary.h>

#include <QDesktopServices>
#include <QMap>
#include <QObject>
#include <QSettings>
#include <QVariant>

#include <QDebug>

#ifdef _WIN32
#include <windows.h>
#endif

class ServiceManagerInitializer : public Utopia::Initializer
{
public:
    ServiceManagerInitializer()
        : Utopia::Initializer(), manager(Kend::ServiceManager::instance())
    {}

    void init()
    {
        this->setMessage("Loading services...");

        manager->loadFromSettings();

        // Make sure unencrypted passwords are now encrypted!
        QSettings conf;
        conf.beginGroup("Auth");
        QString oldUsername = conf.value("Previous Username").toString();
        QString oldPassword = conf.value("Previous Password").toString();
        if (!oldUsername.isEmpty() && !oldPassword.isEmpty()) {
            conf.setValue("Username", oldUsername);
            conf.remove("Previous Username");
            conf.setValue("Password", Utopia::encryptPassword(oldUsername, oldPassword));
            conf.remove("Previous Password");
        }

        // Get defaults
        QVariantMap defaults(Utopia::defaults());
        QUrl url = defaults.value("service_uri").toUrl();
        QString method = defaults.value("service_method").toString();

        // If accounts are prevented
        if (method == "prevent") {
            // Remove all existing accounts
            foreach (Kend::Service * service, manager->services()) {
                manager->removeService(service);
            }
        } else if (manager->count() == 0 && method != "none") {
            // Create new empty public service if none exists
            // Create new service and keep track of its setup
            Kend::Service * service = new Kend::Service;
            bool success = false;

            // Set the Url (causes the service document to be downloaded and installed)
            service->setUrl(url);
            if (service->errorCode() == Kend::Service::Success) {
                // Try and set the authentication method
                QVariantMap methods(service->supportedAuthenticationMethods());
                if (method.isEmpty() || !methods.contains(method)) {
                    if (methods.size() > 0) {
                        method = (--methods.end()).key();
                    }
                }
                service->setAuthenticationMethod(method);
                service->setEnabled(true);

                bool anonymousService = !service->resourceUrl(Kend::Service::AuthenticationResource).isValid();
                bool anonymousCapableAccount = service->resourceCapabilities(Kend::Service::AuthenticationResource).contains("anonymous");

                // If existing credentials are present, use these to set up this service
                QString username = conf.value("Username").toString();
                QString password = Utopia::decryptPassword(username, conf.value("Password").toString());
                if (!anonymousService && !username.isEmpty() && !password.isEmpty()) {
                    QVariantMap credentials;
                    credentials["user"] = username;
                    credentials["password"] = password;
                    service->setCredentials(credentials);
                    service->setProperty("previousAuthenticationMethod", method);
                } else if (anonymousCapableAccount) {
                    service->setAnonymous(true);
                }

                success = true;

                conf.remove("Username");
                conf.remove("Password");
            }

            // If unsuccessful, delete the service object, otherwise add it to the manager
            if (success) {
                manager->addService(service);
            } else {
                delete service;
            }
        }
        conf.endGroup();
        conf.remove("Auth");

        // Show (only once) a welcome screen explaining how to use accounts
        conf.beginGroup("Help");
        QUrl welcome = defaults.value("welcome").toUrl();
        if (welcome.isValid() && conf.value("Welcome Seen", 0.0).toReal() < 2.1) {
            QDesktopServices::openUrl(welcome);
            conf.setValue("Welcome Seen", 2.1);
        }
        conf.endGroup();
    }

    void postInit()
    {
        this->setMessage("Logging in...");
        manager->start();
    }

    void final()
    {
        manager->stop();
        manager->saveToSettings();
    }

    QString description()
    {
        return "Service manager";
    }

protected:
    boost::shared_ptr< Kend::ServiceManager > manager;
};




// dlsym handles

extern "C" const char * utopia_apiVersion()
{
    return UTOPIA_EXTENSION_LIBRARY_VERSION;
}

extern "C" const char * utopia_description()
{
    return "Kend service manager";
}

extern "C" void utopia_registerExtensions()
{
    UTOPIA_REGISTER_EXTENSION(ServiceManagerInitializer);
}
