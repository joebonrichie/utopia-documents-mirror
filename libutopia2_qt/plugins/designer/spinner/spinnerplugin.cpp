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

#include <utopia2/qt/spinner.h>
#include "spinnerplugin.h"

#include <QtDebug>
#include <QtPlugin>

SpinnerPlugin::SpinnerPlugin(QObject * parent)
    : QObject(parent)
{
    initialized = false;
}

void SpinnerPlugin::initialize(QDesignerFormEditorInterface * /* core */)
{
    if (initialized)
        return;

    initialized = true;
}

bool SpinnerPlugin::isInitialized() const
{
    return initialized;
}

QWidget * SpinnerPlugin::createWidget(QWidget * parent)
{
    return new Utopia::Spinner(parent);
}

QString SpinnerPlugin::name() const
{
    return "Utopia::Spinner";
}

QString SpinnerPlugin::group() const
{
    return "Utopia Widgets";
}

QIcon SpinnerPlugin::icon() const
{
    return QIcon();
}

QString SpinnerPlugin::toolTip() const
{
    return "";
}

QString SpinnerPlugin::whatsThis() const
{
    return "";
}

bool SpinnerPlugin::isContainer() const
{
    return false;
}

QString SpinnerPlugin::domXml() const
{
    return "<widget class=\"Utopia::Spinner\" name=\"flowBrowser\">\n"
        " <property name=\"geometry\">\n"
        "  <rect>\n"
        "   <x>0</x>\n"
        "   <y>0</y>\n"
        "   <width>30</width>\n"
        "   <height>30</height>\n"
        "  </rect>\n"
        " </property>\n"
        " <property name=\"whatsThis\" >\n"
        "  <string>The Spinner is a small label signifying indefinite progress.</string>\n"
        " </property>\n"
        "</widget>\n";
}

QString SpinnerPlugin::includeFile() const
{
    return "utopia2/qt/spinner.h";
}

Q_EXPORT_PLUGIN2(spinnerplugin, SpinnerPlugin)
