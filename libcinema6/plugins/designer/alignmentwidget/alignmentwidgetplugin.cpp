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

#include <cinema6/alignmentview.h>
#include "alignmentwidgetplugin.h"

#include <QtDebug>
#include <QtPlugin>

AlignmentWidgetPlugin::AlignmentWidgetPlugin(QObject * parent)
    : QObject(parent)
{
    initialized = false;
}

void AlignmentWidgetPlugin::initialize(QDesignerFormEditorInterface * /* core */)
{
    if (initialized)
        return;

    initialized = true;
}

bool AlignmentWidgetPlugin::isInitialized() const
{
    return initialized;
}

QWidget * AlignmentWidgetPlugin::createWidget(QWidget * parent)
{
    return new CINEMA6::AlignmentView(parent);
}

QString AlignmentWidgetPlugin::name() const
{
    return "CINEMA6::AlignmentView";
}

QString AlignmentWidgetPlugin::group() const
{
    return "CINEMA6 Widgets";
}

QIcon AlignmentWidgetPlugin::icon() const
{
    return QIcon();
}

QString AlignmentWidgetPlugin::toolTip() const
{
    return "";
}

QString AlignmentWidgetPlugin::whatsThis() const
{
    return "";
}

bool AlignmentWidgetPlugin::isContainer() const
{
    return false;
}

QString AlignmentWidgetPlugin::domXml() const
{
    return "<widget class=\"CINEMA6::AlignmentView\" name=\"alignmentView\">\n"
        " <property name=\"geometry\">\n"
        "  <rect>\n"
        "   <x>0</x>\n"
        "   <y>0</y>\n"
        "   <width>300</width>\n"
        "   <height>100</height>\n"
        "  </rect>\n"
        " </property>\n"
        " <property name=\"toolTip\" >\n"
        "  <string>Multiple sequence alignment</string>\n"
        " </property>\n"
        " <property name=\"whatsThis\" >\n"
        "  <string>The Alignment Widget displays a multiple sequence alignment.</string>\n"
        " </property>\n"
        "</widget>\n";
}

QString AlignmentWidgetPlugin::includeFile() const
{
    return "cinema6/alignmentview.h";
}

Q_EXPORT_PLUGIN2(alignmentwidgetplugin, AlignmentWidgetPlugin)
