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

#ifndef UTOPIA_CONFIGURATION_H
#define UTOPIA_CONFIGURATION_H

#include <QObject>
#include <QUuid>
#include <QVariant>

namespace Utopia
{

    class ConfigurationPrivate;
    class Configuration : public QObject
    {
        Q_OBJECT

    public:
        ~Configuration();

        bool contains(const QString & key) const;
        void del(const QString & key);
        QVariant get(const QString & key, const QVariant & defaultValue = QVariant()) const;
        bool isValid() const;
        QStringList keys() const;
        bool set(const QString & key, const QVariant & value);
        void setTitle(const QString title);
        QString title() const;
        QUuid id() const;

        static Configuration * instance(const QUuid & configurationId);

    signals:
        void titleChanged(const QString & title);
        void configurationChanged();
        void configurationChanged(const QString & key);

    protected:
        ConfigurationPrivate * d;

        Configuration(const QUuid & configurationId);
    };

}

#endif // UTOPIA_CONFIGURATION_H
