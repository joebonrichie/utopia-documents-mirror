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

#include "tabling.h"

#include <papyro/documentview.h>
#include <papyro/embeddedframe.h>
#include <papyro/utils.h>
#include <spine/Area.h>

#include <QDebug>




/////////////////////////////////////////////////////////////////////////////////////////
/// TableFactory

void TableFactory::activate(Spine::DocumentHandle document, Spine::AnnotationSet annotations, const QPoint & globalPos)
{
    foreach (Spine::AnnotationHandle annotation, annotations) {
        if (QWidget * widget = Papyro::EmbeddedFrame::probe(annotation)) {
            widget->resize(600, 400);
            widget->show();
        }
    }
}

bool TableFactory::canActivate(Spine::DocumentHandle document, Spine::AnnotationHandle annotation) const
{
    return (annotation->getFirstProperty("concept") == "Table" &&
            !annotation->hasProperty("property:horizontalBoundaries")) ||
           annotation->getFirstProperty("concept") == "ElsevierTable";
}

int TableFactory::category() const
{
    return 10;
}

QIcon TableFactory::icon() const
{
    return generateFromMonoPixmap(QPixmap(":/processors/tabling/icon.png"));
}

void TableFactory::processSelection(Spine::DocumentHandle document, Spine::CursorHandle cursor, const QPoint & globalPos)
{
    Spine::AreaSet areas = document->areaSelection();
    // Do stuff to the area(s)
}

QList< boost::shared_ptr< Papyro::SelectionProcessor > > TableFactory::selectionProcessors(Spine::DocumentHandle document, Spine::CursorHandle cursor)
{
    QList< boost::shared_ptr< Papyro::SelectionProcessor > > list;
    if (hasAreaSelection(document, cursor)) {
        list << boost::shared_ptr< Papyro::SelectionProcessor >(new TableFactory);
    }
    return list;
}

QString TableFactory::title() const
{
    return "Make Table...";
}

QString TableFactory::title(Spine::DocumentHandle document, Spine::AnnotationHandle annotation) const
{
    return "Open Table...";
}

int TableFactory::weight() const
{
    return 10;
}




/////////////////////////////////////////////////////////////////////////////////////////
/// TablingRenderer

TablingRenderer::TablingRenderer()
    : Papyro::SvgMarginaliaOverlayRenderer(":/processors/tabling/margin-icon.svg")
{}

QString TablingRenderer::id()
{
    return "table";
}

int TablingRenderer::weight()
{
    return 10;
}
