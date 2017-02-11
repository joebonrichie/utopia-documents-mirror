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

#include <utopia2/pluginmanager.h>
#include <utopia2/pluginmanager_p.h>
#include <utopia2/plugin.h>
#include <utopia2/plugin_p.h>

#include <boost/weak_ptr.hpp>

#include <QMetaObject>
#include <QMetaProperty>
#include <QSettings>
#include <QStringList>
#include <QStringListIterator>
#include <QUuid>

namespace Utopia
{

    namespace
    {

        QString uuid_to_string(const QUuid & uuid)
        {
            QString string(uuid.toString());
            return string.mid(1, string.length() - 2);
        }

    }




    PluginManagerPrivate::PluginManagerPrivate(PluginManager * manager)
        : QObject(manager), manager(manager)
    {
        // Load saved plugin descriptions FIXME
        load();
    }

    PluginManagerPrivate::~PluginManagerPrivate()
    {
        save();
    }

    void PluginManagerPrivate::load()
    {
        QSettings conf;
        conf.beginGroup("Plugins");
        conf.beginGroup("Store");
        foreach (QString uuidStr, conf.childGroups()) {
            QUuid uuid(uuidStr);
            if (!uuid.isNull()) {
                Plugin * plugin = new Plugin(uuid, this);

                conf.beginGroup(uuidStr);
                conf.beginGroup("properties");
                QStringListIterator propertyNames(conf.childKeys());
                while (propertyNames.hasNext()) {
                    QString key = propertyNames.next();
                    QVariant value = conf.value(key);
                    if (value.isValid()) {
                        plugin->setProperty(key.toUtf8().constData(), value);
                    }
                }
                conf.endGroup();
                propertyNames = conf.childKeys();
                while (propertyNames.hasNext()) {
                    QString key = propertyNames.next();
                    QVariant value = conf.value(key);
                    if (value.isValid()) {
                        plugin->d->setProperty(key.toUtf8().constData(), value);
                    }
                }
                conf.endGroup();

                pluginsByUuid[plugin->uuid()] = plugin;
                pluginsByPath[plugin->fileInfo().absoluteFilePath()] = plugin;
            }
        }
    }

    void PluginManagerPrivate::save()
    {
        QSettings conf;
        conf.beginGroup("Plugins");
        conf.beginGroup("Store");
        foreach (Plugin * plugin, pluginsByUuid.values()) {
            QUuid uuid(plugin->uuid());
            if (!uuid.isNull()) {
                if (plugin->isRemoved()) {
                    // Remove conf values
                    conf.remove(uuid_to_string(uuid));
                } else {
                    conf.beginGroup(uuid_to_string(uuid));
                    conf.beginGroup("properties");
                    QListIterator< QByteArray > propertyNames(plugin->dynamicPropertyNames());
                    while (propertyNames.hasNext()) {
                        QByteArray key = propertyNames.next();
                        if (!key.startsWith("_")) {
                            conf.setValue(key, plugin->property(key.constData()));
                        }
                    }
                    conf.endGroup();
                    const QMetaObject * metaObject(plugin->d->metaObject());
                    for (int index = 0; index < metaObject->propertyCount(); ++index) {
                        QMetaProperty metaProperty(metaObject->property(index));
                        // Only save properties I can/should write back later
                        if (metaProperty.isWritable() && metaProperty.isStored() && metaProperty.name()[0] != '_') {
                            QVariant value(metaProperty.read(plugin->d));
                            if (!value.isNull()) {
                                conf.setValue(metaProperty.name(), value);
                            }
                        }
                    }
                    conf.endGroup();

                    if (!pluginsByUuid.contains(plugin->uuid()) && !pluginsByPath.contains(plugin->fileInfo().absoluteFilePath())) {
                        pluginsByUuid[plugin->uuid()] = plugin;
                        pluginsByPath[plugin->fileInfo().absoluteFilePath()] = plugin;
                    } else {
                        // For security reasons, delete this plugin
                        // FIXME and the conflicting one?
                        delete plugin;
                    }
                }
            }
        }
    }




    PluginManager::PluginManager()
        : QObject(), d(new PluginManagerPrivate(this))
    {}

    boost::shared_ptr< PluginManager > PluginManager::instance()
    {
        static boost::weak_ptr< PluginManager > singleton;
        boost::shared_ptr< PluginManager > shared(singleton.lock());
        if (singleton.expired())
        {
            shared = boost::shared_ptr< PluginManager >(new PluginManager());
            singleton = shared;
        }
        return shared;
    }

    QStringList PluginManager::paths() const
    {
        QStringList paths;
        foreach (Plugin * plugin, d->pluginsByPath.values()) {
            paths.append(plugin->path());
        }
        return paths;
    }

    QList< Plugin * > PluginManager::plugins() const
    {
        return d->pluginsByPath.values();
    }

    bool PluginManager::remove(Plugin * plugin)
    {
        return true;
    }

    Plugin * PluginManager::resolve(const QFileInfo & fileInfo)
    {
        // Return matching plugin
        Plugin * plugin = d->pluginsByPath.value(fileInfo.absoluteFilePath(), 0);

        if (!plugin) {
            // Or create a new plugin, if none match
            plugin = new Plugin(fileInfo, d);
            d->pluginsByUuid[plugin->uuid()] = plugin;
            d->pluginsByPath[plugin->fileInfo().absoluteFilePath()] = plugin;
        }

        return plugin;
    }

    Plugin * PluginManager::resolve(const QUuid & uuid)
    {
        return d->pluginsByUuid.value(uuid, 0);
    }

}
