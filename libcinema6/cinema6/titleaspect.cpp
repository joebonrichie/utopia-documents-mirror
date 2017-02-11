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
#include <cinema6/annotationcomponent.h>
#include <cinema6/component.h>
#include <cinema6/titleaspect.h>

#include <QMouseEvent>
#include <QPainter>

namespace CINEMA6
{

    /**
     *  \brief Default component construction.
     */
    TitleAspect::TitleAspect(const QString & title)
        : Aspect(title)
    {}

    /**
     *  \brief Destructor.
     */
    TitleAspect::~TitleAspect()
    {}

    void TitleAspect::mousePressEvent(Component * component, QMouseEvent * event)
    {
        // Bail if not data
        if (isComponent< DataComponent >(component))
        {
            if (event->button() == Qt::LeftButton)
            {
                Selection selection;
                QPair< int, AlignmentView::ComponentPosition > logicalKeyPosition = this->alignmentView()->componentPosition(component);
                int componentIndex = this->alignmentView()->logicalToActualComponent(logicalKeyPosition.first, logicalKeyPosition.second);

                if (event->modifiers() == Qt::NoModifier)
                {
                    selection.select(componentIndex, componentIndex);
                    this->alignmentView()->setSelection(selection);
                    event->accept();
                }
                else if (event->modifiers() & Qt::ShiftModifier & Qt::ControlModifier)
                {
                    selection = this->alignmentView()->selection();
                    selection.merge(Selection(componentIndex, componentIndex), Selection::Toggle);
                    this->alignmentView()->setSelection(selection);
                    component->updateGeometry();
                    event->accept();
                }
                else if (event->modifiers() & Qt::ShiftModifier)
                {
                    selection = this->alignmentView()->selection();
                    selection.merge(Selection(componentIndex, componentIndex), Selection::Toggle);
                    this->alignmentView()->setSelection(selection);
                    component->updateGeometry();
                    event->accept();
                }
                else if (event->modifiers() & Qt::ControlModifier)
                {
                    selection = this->alignmentView()->selection();
                    selection.merge(Selection(componentIndex, componentIndex), Selection::Toggle);
                    this->alignmentView()->setSelection(selection);
                    component->updateGeometry();
                    event->accept();
                }
            }
        }
        else
        {
            this->alignmentView()->setSelection(Selection());
            this->update();
        }
    }

    /**
     *  \brief Paint control to screen.
     */
    void TitleAspect::paint(QPainter * painter, const QRect & rect, const QRect & /*clip*/, Component * component)
    {
        // Bail if not data
        if (!isComponent< DataComponent >(component)) { return; }

        QPair< int, AlignmentView::ComponentPosition > logicalKeyPosition = this->alignmentView()->componentPosition(component);
        int componentIndex = this->alignmentView()->logicalToActualComponent(logicalKeyPosition.first, logicalKeyPosition.second);
        Selection selection = this->alignmentView()->selection();

        int width = rect.width();
        int height = rect.height();
        int textHeight = height < 8 ? 8 : height;

        QFont font = painter->font();
        float size = font.pointSizeF() * 1.2;
        font.setPointSizeF(size < 4 ? 4 : size);
        QFont annFont(font);
        annFont.setItalic(true);
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setRenderHint(QPainter::TextAntialiasing);

        QColor col;
        if (selection.contains(componentIndex))
        {
            col = this->alignmentView()->palette().color(QPalette::HighlightedText);
        }
        else
        {
            col = this->alignmentView()->palette().color(QPalette::Text);
        }

        // Title
        col.setAlphaF(height <= 10 ? (textHeight - 7) / 3.0 : 1.0);
        painter->setBrush(Qt::NoBrush);
        painter->setPen(col);
        int textWidth = width - 12;
        painter->setFont(isComponent< AnnotationComponent >(component) ? annFont : font);
        QString text = painter->fontMetrics().elidedText(component->title(), Qt::ElideRight, textWidth);
        painter->drawText(6, (height - textHeight) / 2, textWidth, textHeight, Qt::AlignVCenter | Qt::AlignRight, text);
    }

    /**
     *  \brief Width of control aspect.
     */
    int TitleAspect::width() const
    {
        return 160;
    }

}
