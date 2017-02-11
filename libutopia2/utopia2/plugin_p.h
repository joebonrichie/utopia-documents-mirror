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

#ifndef UTOPIA_PLUGIN_P_H
#define UTOPIA_PLUGIN_P_H

#include <utopia2/config.h>
#include <utopia2/plugin.h>

#include <QDir>
#include <QFileInfo>
#include <QObject>
#include <QUuid>

namespace Utopia
{

    class PluginPrivate : public QObject
    {
        Q_OBJECT
        Q_PROPERTY(bool enabled
                   READ isEnabled
                   WRITE setEnabled)
        Q_PROPERTY(QString filePath
                   READ getFilePath
                   WRITE setFilePath)

    public:
        PluginPrivate(Plugin * plugin, const QFileInfo & fileInfo);
        PluginPrivate(Plugin * plugin, const QUuid & uuid);

        Plugin * plugin;

        bool enabled;
        QFileInfo fileInfo;
        bool removed;
        QUuid uuid;
#ifdef Q_OS_MAC
        QDir appDir;
#endif

        QString getFilePath() const;
        bool isEnabled() const;
        void setFilePath(const QString & filePath);
        void setEnabled(bool enabled);

    }; // class Plugin

} // namespace Utopia

#endif // UTOPIA_PLUGIN_P_H
