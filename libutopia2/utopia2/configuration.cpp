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

#include <utopia2/configuration.h>
#include <utopia2/configuration_p.h>
#include <utopia2/encryption.h>

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include <QMutexLocker>
#include <QSettings>
#include <QStringList>

#include <QDebug>

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



    ConfigurationPrivate::ConfigurationPrivate(Configuration * configuration, const QUuid & id)
        : configuration(configuration), mutex(QMutex::Recursive), id(id)
    {
        load();
    }

    ConfigurationPrivate::~ConfigurationPrivate()
    {
        save();
    }

    void ConfigurationPrivate::load()
    {
        QMutexLocker guard(&mutex);
        QString key(uuid_to_string(id));

        // Read configuration
        QSettings conf;
        conf.beginGroup("Configurations");
        conf.beginGroup(key);
        QByteArray ciphertext = conf.value("data").toByteArray();
        data = decryptMap(ciphertext, key);
        title = conf.value("title").toString();
    }

    void ConfigurationPrivate::save()
    {
        QMutexLocker guard(&mutex);
        QString key(uuid_to_string(id));

        // Read configuration
        QSettings conf;
        conf.beginGroup("Configurations");
        conf.beginGroup(key);
        QByteArray ciphertext = encryptMap(data, key);
        conf.setValue("data", ciphertext);
        conf.setValue("title", title);
    }




    Configuration::Configuration(const QUuid & id)
        : d(new ConfigurationPrivate(this, id))
    {}

    Configuration::~Configuration()
    {
        delete d;
    }

    bool Configuration::contains(const QString & key) const
    {
        return d->data.contains(key);
    }

    void Configuration::del(const QString & key)
    {
        d->data.remove(key);
    }

    QVariant Configuration::get(const QString & key, const QVariant & defaultValue) const
    {
        QMutexLocker guard(&d->mutex);
        return d->data.value(key);
    }

    QUuid Configuration::id() const
    {
        QMutexLocker guard(&d->mutex);
        return d->id;
    }

    Configuration * Configuration::instance(const QUuid & id)
    {
        static QMap< QUuid, boost::shared_ptr< Configuration > > instances;
        Configuration * configuration = instances.value(id).get();
        if (!configuration) {
            configuration = new Configuration(id);
            instances[id] = boost::shared_ptr< Configuration >(configuration);
        }
        return configuration;
    }

    bool Configuration::isValid() const
    {
        QMutexLocker guard(&d->mutex);
        return true;
    }

    QStringList Configuration::keys() const
    {
        QMutexLocker guard(&d->mutex);
        return d->data.keys();
    }

    bool Configuration::set(const QString & key, const QVariant & value)
    {
        bool success = false;
        {
            QMutexLocker guard(&d->mutex);
            if (d->data.value(key) != value) {
                d->data[key] = value;
                success = true;
            }
        }
        if (success) {
            emit configurationChanged(key);
            emit configurationChanged();
        }
        return success;
    }

    void Configuration::setTitle(const QString title)
    {
        QMutexLocker guard(&d->mutex);
        if (d->title != title) {
            d->title = title;
            emit titleChanged(title);
        }
    }

    QString Configuration::title() const
    {
        QMutexLocker guard(&d->mutex);
        return d->title;
    }

}
