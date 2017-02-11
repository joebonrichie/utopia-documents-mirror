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

#include <boost/foreach.hpp>
#include <boost/system/error_code.hpp>
#include <utopia2/encryption.h>
#include <utopia2/extension.h>
#include <utopia2/extensionlibrary.h>
#include <utopia2/initializer.h>
#include <utopia2/library.h>
#include <utopia2/networkaccessmanager.h>
#include <utopia2/pacproxyfactory.h>
#include <utopia2/pacscript.h>
#include <string>

#include "version_p.h"

#include <QCoreApplication>
#include <QDir>
#include <QPointer>
#include <QRegExp>
#include <QString>
#include <QSet>
#include <QSettings>
#include <QWebSettings>
#include <QtDebug>

#ifdef _WIN32
#include <windows.h>
#endif

namespace Utopia
{
    static QPointer< Utopia::PACProxyFactory > _globalProxyFactory;

    void init(QObject* progressIndicator_)
    {
        static bool initialised = false;
        if (initialised) return;

        QCoreApplication * coreApp = 0;
        if (QCoreApplication::instance() == 0)
        {
            int argc = 0;
            coreApp = new QCoreApplication(argc, (char**) 0);
        }

        QCoreApplication::setOrganizationName("Lost Island Labs");
        QCoreApplication::setApplicationName("Utopia");

        // Copy over old settings if need be (AIG->LIL)
        QSettings aigSettings("Advanced Interfaces Group", "Utopia");
        aigSettings.setFallbacksEnabled(false);
        QSettings lilSettings;
        lilSettings.setFallbacksEnabled(false);
        if (lilSettings.allKeys().isEmpty()) {
            foreach (QString key, aigSettings.allKeys()) {
                lilSettings.setValue(key, aigSettings.value(key));
            }
        }
        aigSettings.setFallbacksEnabled(true);

#ifdef Q_OS_LINUX
        QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, config_path());
#endif

        // Start by ensuring appropriate encryption blocks
        generateSecurityBlocks();

        QWebSettings::globalSettings()->setAttribute(QWebSettings::LocalContentCanAccessRemoteUrls, true);

        globalProxyFactory();

        // Load libraries
        ExtensionLibrary::loadDirectory(plugin_path());

        // Load system Extension
        Initializer* system = instantiateExtension< Initializer >("Utopia::SystemInitializer");
        if (system)
        {
            if (progressIndicator_)
            {
                QObject::connect(system, SIGNAL(messageChanged(QString)), progressIndicator_, SLOT(changeMessage(QString)));
            }
            system->init();
            if (system->errorCode() != Initializer::None)
            {
                qDebug() << "FATAL ERROR:" << system->message() << ". Cannot initialise Utopia.";
                exit(0);
            }
            else if (progressIndicator_)
            {
                QObject::disconnect(system, SIGNAL(messageChanged(QString)), progressIndicator_, SLOT(changeMessage(QString)));
            }
        }
        else
        {
            qDebug() << "FATAL ERROR: No system Extension found. Cannot initialise Utopia.";
            exit(0);
        }

        // Load other Initializer extensions
        std::set< std::string > names = registeredExtensionNames< Initializer >();
        std::list< Initializer* > initializers;
        names.erase("Utopia::SystemInitializer");
        BOOST_FOREACH (std::string name, names)
        {
            Initializer* custom = instantiateExtension< Initializer >(name);
            initializers.push_back(custom);
            if (progressIndicator_)
            {
                QObject::connect(custom, SIGNAL(messageChanged(QString)), progressIndicator_, SLOT(changeMessage(QString)));
            }
            custom->init();
            if (custom->errorCode() != Initializer::None)
            {
                qDebug() << "FATAL ERROR:" << custom->message() << ". Cannot initialise Utopia.";
                exit(0);
            }
            else if (progressIndicator_)
            {
                QObject::disconnect(custom, SIGNAL(messageChanged(QString)), progressIndicator_, SLOT(changeMessage(QString)));
            }
        }

        // Post initialisation routines
        BOOST_FOREACH (Initializer* custom, initializers)
        {
            if (progressIndicator_)
            {
                QObject::connect(custom, SIGNAL(messageChanged(QString)), progressIndicator_, SLOT(changeMessage(QString)));
            }
            custom->postInit();
            if (custom->errorCode() != Initializer::None)
            {
                qDebug() << "FATAL ERROR:" << custom->message() << ". Cannot initialise Utopia.";
                exit(0);
            }
            else if (progressIndicator_)
            {
                QObject::disconnect(custom, SIGNAL(messageChanged(QString)), progressIndicator_, SLOT(changeMessage(QString)));
            }
        }

        if (progressIndicator_)
        {
            SplashHelper helper(progressIndicator_);
            helper.changeMessage("Launching application...");
        }

        initialised = true;

        if (coreApp) { delete coreApp; }
    }

    const QVariantMap & defaults()
    {
        static QVariantMap defaults;
        if (defaults.isEmpty()) {
            {
                QString defaultsPath(resource_path() + "/defaults.conf");
                QSettings defaultSettings(defaultsPath, QSettings::IniFormat);
                defaultSettings.setIniCodec("UTF-8");
                foreach (QString key, defaultSettings.allKeys()) {
                    defaults[key] = defaultSettings.value(key);
                }
            }

#ifdef _WIN32
            char env[1024] = { 0 };
            int status = GetEnvironmentVariable("UTOPIA_DEFAULTS_CONF", env, sizeof(env));
            if (status == 0) { env[0] = 0; }
            if (*env) {
#else
            char * env = ::getenv("UTOPIA_DEFAULTS_CONF");
            if (env && *env) {
#endif
                QFileInfo fileInfo(env);
                if (fileInfo.exists() && fileInfo.isFile()) {
                    QString defaultsPath(env);
                    QSettings defaultSettings(defaultsPath, QSettings::IniFormat);
                    defaultSettings.setIniCodec("UTF-8");
                    foreach (QString key, defaultSettings.allKeys()) {
                        defaults[key] = defaultSettings.value(key);
                    }
                }
            }
        }
        return defaults;
    }

    int versionMajor()
    {
        return UTOPIA_VERSION_MAJOR;
    }

    int versionMinor()
    {
        return UTOPIA_VERSION_MINOR;
    }

    int versionPatch()
    {
        return UTOPIA_VERSION_PATCH;
    }

    QString versionString(bool forceDebug)
    {
        return forceDebug ? UTOPIA_VERSION_DEBUG_STRING : UTOPIA_VERSION_STRING;
    }

    QString revisionHash()
    {
        return UTOPIA_REVISION_HASH;
    }

    QString revisionBranch()
    {
        return UTOPIA_REVISION_BRANCH;
    }

    PACProxyFactory * globalProxyFactory()
    {
        // Ensure the PAC aware global proxy factory is installed
        if (!_globalProxyFactory)
        {
            Utopia::PACScript * script = new Utopia::PACScript;
            _globalProxyFactory = new Utopia::PACProxyFactory(script);
            QNetworkProxyFactory::setApplicationProxyFactory(_globalProxyFactory);
        }
        return _globalProxyFactory;
    }

    QString current_path()
    {
        return QDir::currentPath();
    }

    QString executable_path()
    {
        return QCoreApplication::applicationDirPath();
    }

    static bool cd(QDir & dir, QString step)
    {
        return dir.cd(step) || (dir.mkdir(step) && dir.cd(step));
    }

    QString plugin_path()
    {
#if defined(Q_OS_LINUX)
        QDir path(private_library_path());
#else
        QDir path(executable_path());
#endif

        if (
#if defined(Q_OS_WIN)
            path.cdUp() && path.cd("plugins")
#elif defined(Q_OS_MACX)
            path.cdUp() && path.cd("PlugIns")
#elif defined(Q_OS_LINUX)
            path.cd("plugins")
#else
#  error Unsupported platform!
#endif
            ) {
            return QDir::cleanPath(path.canonicalPath());
        } else {
            return QString();
        }
    }

    QString private_library_path()
    {
#if defined(Q_OS_LINUX)
        QDir path(executable_path());
        path.cdUp() && path.cd("lib") && path.cd("utopia-documents");
        return QDir::cleanPath(path.canonicalPath());
#else
        return resource_path();
#endif
    }

    QString resource_path()
    {
        QDir path(executable_path());
        if (
#if defined(Q_OS_WIN)
            path.cdUp() && path.cd("resources")
#elif defined(Q_OS_MACX)
            path.cdUp() && path.cd("Resources")
#elif defined(Q_OS_LINUX)
            path.cdUp() && path.cd("share") && path.cd("utopia-documents")
#else
#  error Unsupported platform!
#endif
            ) {
            return QDir::cleanPath(path.canonicalPath());
        } else {
            return QString();
        }
    }

    QString config_path()
    {
#if defined(Q_OS_WIN) || defined(Q_OS_MACX)
        return profile_path(ProfileRoot);
#elif defined(Q_OS_LINUX)
        QDir path(QDir::home());
        return cd(path, ".config") && cd(path, "utopia") ? QDir::cleanPath(path.canonicalPath()) : QString();
#else
#  error Unsupported platform!
#endif
    }

    QString profile_path(ProfilePathPart part)
    {
#if defined(Q_OS_WIN)
        QDir path(QString(getenv("APPDATA")));
#else
        QDir path(QDir::home());
#endif
        if (
#if defined(Q_OS_WIN)
            cd(path, "Utopia")
#elif defined(Q_OS_MACX)
            cd(path, "Library") && cd(path, "Utopia")
#elif defined(Q_OS_LINUX)
            cd(path, ".local") && cd(path, "share") && cd(path, "utopia")
#else
#  error Unsupported platform!
#endif
            ) {
            if ((part == ProfilePlugins && cd(path, "plugins")) ||
                (part == ProfileLogs && cd(path, "logs")) ||
                (part == ProfileData && cd(path, "data")) ||
                 part == ProfileRoot) {
                return QDir::cleanPath(path.canonicalPath());
            }
        }

        return QString();
    }

}
