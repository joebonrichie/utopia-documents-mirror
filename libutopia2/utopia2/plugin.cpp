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

#include <utopia2/plugin_p.h>
#include <utopia2/plugin.h>

#include <QDir>
#include <QFile>

#include <QDebug>

namespace Utopia
{

    PluginPrivate::PluginPrivate(Plugin * plugin, const QFileInfo & fileInfo)
        : QObject(plugin), plugin(plugin), enabled(true), fileInfo(fileInfo), removed(false), uuid(QUuid::createUuid())
    {
#ifdef Q_OS_MAC
        appDir = Utopia::plugin_path();
#endif
    }

    PluginPrivate::PluginPrivate(Plugin * plugin, const QUuid & uuid)
        : QObject(plugin), plugin(plugin), enabled(true), removed(false), uuid(uuid)
    {
#ifdef Q_OS_MAC
        appDir = Utopia::plugin_path();
#endif
    }

    // The following four methods are only to make Q_PROPERTY work

    QString PluginPrivate::getFilePath() const
    {
        QString path = fileInfo.absoluteFilePath();
#ifdef Q_OS_MAC
        if (path.startsWith(appDir.absolutePath())) {
            path = appDir.relativeFilePath(path);
        }
#endif
        return path;
    }

    bool PluginPrivate::isEnabled() const
    {
        return enabled;
    }

    void PluginPrivate::setFilePath(const QString & filePath)
    {
        fileInfo = QFileInfo(filePath);
#ifdef Q_OS_MAC
        if (fileInfo.isRelative()) {
            fileInfo = QFileInfo(appDir.filePath(fileInfo.filePath()));
        }
#endif
    }

    void PluginPrivate::setEnabled(bool enabled)
    {
        this->enabled = enabled;
    }




    Plugin::Plugin(const QFileInfo & fileInfo, QObject * parent)
        : QObject(parent), d(new PluginPrivate(this, fileInfo))
    {}

    Plugin::Plugin(const QUuid & uuid, QObject * parent)
        : QObject(parent), d(new PluginPrivate(this, uuid))
    {}

    bool Plugin::isEnabled() const
    {
        return d->enabled;
    }

    bool Plugin::isRemoved() const
    {
        return d->removed;
    }

    QFileInfo Plugin::fileInfo() const
    {
        return d->fileInfo;
    }

    QString Plugin::path() const
    {
        return d->fileInfo.canonicalFilePath();
    }

    void Plugin::remove()
    {
        if (!d->removed) {
            d->removed = !QFile::exists(path()) || QFile::remove(path());
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
