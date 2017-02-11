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

#include <utopia2/qt/flowbrowser.h>
#include "flowbrowserplugin.h"

#include <QtDebug>
#include <QtPlugin>

FlowBrowserPlugin::FlowBrowserPlugin(QObject * parent)
    : QObject(parent)
{
    initialized = false;
}

void FlowBrowserPlugin::initialize(QDesignerFormEditorInterface * /* core */)
{
    if (initialized)
        return;

    initialized = true;
}

bool FlowBrowserPlugin::isInitialized() const
{
    return initialized;
}

QWidget * FlowBrowserPlugin::createWidget(QWidget * parent)
{
    return new Utopia::FlowBrowser(parent);
}

QString FlowBrowserPlugin::name() const
{
    return "Utopia::FlowBrowser";
}

QString FlowBrowserPlugin::group() const
{
    return "Utopia Widgets";
}

QIcon FlowBrowserPlugin::icon() const
{
    return QIcon();
}

QString FlowBrowserPlugin::toolTip() const
{
    return "";
}

QString FlowBrowserPlugin::whatsThis() const
{
    return "";
}

bool FlowBrowserPlugin::isContainer() const
{
    return false;
}

QString FlowBrowserPlugin::domXml() const
{
    return "<widget class=\"Utopia::FlowBrowser\" name=\"flowBrowser\">\n"
        " <property name=\"geometry\">\n"
        "  <rect>\n"
        "   <x>0</x>\n"
        "   <y>0</y>\n"
        "   <width>300</width>\n"
        "   <height>100</height>\n"
        "  </rect>\n"
        " </property>\n"
        " <property name=\"whatsThis\" >\n"
        "  <string>The Flow Browser provides an integrable cover flow widget.</string>\n"
        " </property>\n"
        "</widget>\n";
}

QString FlowBrowserPlugin::includeFile() const
{
    return "utopia2/qt/flowbrowser.h";
}

Q_EXPORT_PLUGIN2(flowbrowserplugin, FlowBrowserPlugin)
