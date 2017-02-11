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

#include "highlighting.h"

#include <papyro/documentview.h>
#include <papyro/papyrotab.h>
#include <papyro/utils.h>

#include <QPainter>

#include <QtDebug>




/////////////////////////////////////////////////////////////////////////////////////////
/// HighlightFactory

int HighlightFactory::category() const
{
    return 10;
}

QIcon HighlightFactory::icon() const
{
    return generateFromMonoPixmap(QPixmap(":/processors/highlighting/icon.png"));
}

void HighlightFactory::processSelection(Spine::DocumentHandle document, Spine::CursorHandle, const QPoint & globalPos)
{
    Spine::AnnotationHandle annotation(new Spine::Annotation);
    annotation->setProperty("concept", "Highlight");
    annotation->setProperty("property:color", "yellow");
    foreach (const Spine::Area & area, document->areaSelection()) {
        annotation->addArea(area);
    }
    foreach (Spine::TextExtentHandle extent, document->textSelection()) {
        annotation->addExtent(extent);
    }
    document->addAnnotation(annotation, "PersistQueue");
    document->clearSelection();
}

QList< boost::shared_ptr< Papyro::SelectionProcessor > > HighlightFactory::selectionProcessors(Spine::DocumentHandle document, Spine::CursorHandle cursor)
{
    QList< boost::shared_ptr< Papyro::SelectionProcessor > > list;
    if (hasTextSelection(document, cursor)) {
        list << boost::shared_ptr< Papyro::SelectionProcessor >(new HighlightFactory);
    }
    return list;
}

QString HighlightFactory::title() const
{
    return "Highlight";
}

int HighlightFactory::weight() const
{
    return 10;
}




/////////////////////////////////////////////////////////////////////////////////////////
/// HighlightRenderer

void HighlightRenderer::configurePainter(QPainter * painter, State state)
{
    painter->setBrush(color);
    painter->setPen(QPen(color, 0));
    painter->setCompositionMode(QPainter::CompositionMode_Multiply);
}

QCursor HighlightRenderer::cursor()
{
    return QCursor();
}

QString HighlightRenderer::id()
{
    return "highlight";
}

QMap< int, QPicture > HighlightRenderer::render(Spine::DocumentHandle document, const Spine::AnnotationSet & annotations, Papyro::OverlayRenderer::State state)
{
    QMap< int, QPicture > pictures;
    QMap< int, QPicture > multiPictures;
    QHash< QString, Spine::AnnotationSet > groups;

    // Group by colour
    foreach (Spine::AnnotationHandle annotation, annotations) {
        QString colorStr(Papyro::qStringFromUnicode(annotation->getFirstProperty("property:color")));
        if (colorStr.isEmpty()) {
            colorStr = Papyro::qStringFromUnicode(annotation->getFirstProperty("session:color"));
        }
        QColor color(colorStr);
        if (!color.isValid()) {
            color = Qt::yellow;
        }
        groups[color.name()].insert(annotation);
    }

    QHashIterator< QString, Spine::AnnotationSet > iter(groups);
    while (iter.hasNext()) {
        iter.next();
        color = QColor(iter.key());
        multiPictures.unite(Papyro::OverlayRenderer::render(document, iter.value(), state));
    }

    foreach (int page, multiPictures.keys()) {
        QPainter painter(&pictures[page]);
        foreach (QPicture picture, multiPictures.values(page)) {
            painter.drawPicture(0, 0, picture);
        }
    }

    return pictures;
}

int HighlightRenderer::weight()
{
    return 10;
}
