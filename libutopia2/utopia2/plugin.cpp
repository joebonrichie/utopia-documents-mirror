/*****************************************************************************
 *  
 *   This file is part of the Utopia Documents application.
 *       Copyright (c) 2008-2014 Lost Island Labs
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

#include <utopia2/plugin_p.h>
#include <utopia2/plugin.h>

#include <QFile>


namespace Utopia
{

    PluginPrivate::PluginPrivate(Plugin * plugin, Plugin::PluginBase base, const QString & relativePath)
        : QObject(plugin), plugin(plugin), enabled(true), base(base), relativePath(relativePath), removed(false), uuid(QUuid::createUuid())
    {}

    PluginPrivate::PluginPrivate(Plugin * plugin, const QUuid & uuid)
        : QObject(plugin), plugin(plugin), enabled(true), removed(false), uuid(uuid)
    {}

    QString PluginPrivate::constructAbsolutePath(Plugin::PluginBase base)
    {
        switch (base) {
        case Plugin::InstallBase:
            return Utopia::plugin_path();
            break;
        case Plugin::ProfileBase:
            return Utopia::profile_path(Utopia::ProfilePlugins);
            break;
        }

        return QString();
    }

    Plugin::PluginBase PluginPrivate::getBase() const
    {
        return base;
    }

    QString PluginPrivate::getRelativePath() const
    {
        return relativePath;
    }

    bool PluginPrivate::isEnabled() const
    {
        return enabled;
    }

    void PluginPrivate::setEnabled(bool enabled)
    {
        this->enabled = enabled;
    }

    void PluginPrivate::setBase(Plugin::PluginBase base)
    {
        this->base = base;
    }

    void PluginPrivate::setRelativePath(const QString & relativePath)
    {
        this->relativePath = relativePath;
    }




    Plugin::Plugin(Plugin::PluginBase base, const QString & relativePath, QObject * parent)
        : QObject(parent), d(new PluginPrivate(this, base, relativePath))
    {}

    Plugin::Plugin(const QUuid & uuid, QObject * parent)
        : QObject(parent), d(new PluginPrivate(this, uuid))
    {}

    QString Plugin::absolutePath() const
    {
        return basePath() + "/" + relativePath();
    }

    Plugin::PluginBase Plugin::base() const
    {
        return d->getBase();
    }

    QString Plugin::basePath() const
    {
        return PluginPrivate::constructAbsolutePath(base());
    }

    QString Plugin::constructAbsolutePath(Plugin::PluginBase base, const QString & relativePath)
    {
        return PluginPrivate::constructAbsolutePath(base) + "/" + relativePath;
    }

    bool Plugin::isEnabled() const
    {
        return d->isEnabled();
    }

    bool Plugin::isRemoved() const
    {
        return d->removed;
    }

    QString Plugin::relativePath() const
    {
        return d->getRelativePath();
    }

    void Plugin::remove()
    {
        if (!d->removed) {
            d->removed = !QFile::exists(absolutePath()) || QFile::remove(absolutePath());
            if (d->removed) {
                emit removed();
            }
        }
    }

    void Plugin::setEnabled(bool enabled)
    {
        if (enabled != d->isEnabled()) {
            d->setEnabled(enabled);
            emit enabledChanged(enabled);
        }
    }

    QUuid Plugin::uuid() const
    {
        return d->uuid;
    }

} // namespace Utopia
