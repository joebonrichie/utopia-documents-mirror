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
#include <cinema6/controlaspect.h>

#include <QMouseEvent>
#include <QPainter>

namespace CINEMA6
{

    /**
     *  \brief Default component construction.
     */
    ControlAspect::ControlAspect(const QString & title)
        : Aspect(title)
    {}

    /**
     *  \brief Destructor.
     */
    ControlAspect::~ControlAspect()
    {}

    bool inCloseButton(Aspect * aspect, Component * component, const QPoint & pos)
    {
        int height = component->height();

        if (height > 8)
        {
            // Close button
            int buttonRadius = height < 13 ? (height - 3) / 2 : 5;
            int width = aspect->width();
            int middle = height / 2;

            bool overClose = (pos - QPoint(width - 11, middle)).manhattanLength() <= buttonRadius + 3;

            if (overClose && isComponent< DataComponent >(component))
            {
                return true;
            }
        }

        return false;
    }

    void ControlAspect::leaveEvent(Component * component)
    {
        HoverFocus oldHoverFocus = this->_hoverFocus[component];
        if (oldHoverFocus != Nothing)
        {
            this->_hoverFocus[component] = Nothing;
            this->update(component);
        }
    }

    void ControlAspect::mouseDoubleClickEvent(Component * component, QMouseEvent * event)
    {
    }

    void ControlAspect::mouseMoveEvent(Component * component, QMouseEvent * event)
    {
        int height = component->height();
        int width = this->width();
        int middle = height / 2;

        if (height > 8 && event->buttons() == Qt::NoButton)
        {
            // Close button
            int buttonRadius = height < 13 ? (height - 3) / 2 : 5;

            bool overClose = (event->pos() - QPoint(width - 11, middle)).manhattanLength() <= buttonRadius + 3;

            HoverFocus oldHoverFocus = this->_hoverFocus[component];
            if (overClose && oldHoverFocus != Close)
            {
                this->_hoverFocus[component] = Close;
                this->update(component);
            }
            else if (!overClose && oldHoverFocus == Close)
            {
                this->_hoverFocus[component] = Nothing;
                this->update(component);
            }
        }
    }

    void ControlAspect::mousePressEvent(Component * component, QMouseEvent * event)
    {
        int height = component->height();

        if (height > 8)
        {
            // Close button
            int buttonRadius = height < 13 ? (height - 3) / 2 : 5;
            int width = this->width();
            int middle = height / 2;

            bool overClose = (event->pos() - QPoint(width - 11, middle)).manhattanLength() <= buttonRadius + 3;

            if (overClose && isComponent< DataComponent >(component))
            {
                event->accept();
            }
        }
    }

    void ControlAspect::mouseReleaseEvent(Component * component, QMouseEvent * event)
    {
        int height = component->height();

        if (height > 8)
        {
            // Close button
            int buttonRadius = height < 13 ? (height - 3) / 2 : 5;
            int width = this->width();
            int middle = height / 2;

            bool overClose = (event->pos() - QPoint(width - 11, middle)).manhattanLength() <= buttonRadius + 3;

            if (overClose && isComponent< DataComponent >(component))
            {
                component->deleteLater();
                event->accept();
            }
        }
    }

    /**
     *  \brief Paint control to screen.
     */
    void ControlAspect::paint(QPainter * painter, const QRect & rect, const QRect & /*clip*/, Component * component)
    {
        // Bail if not data
        if (!isComponent< DataComponent >(component)) { return; }

        int width = rect.width();
        int height = rect.height() - 1;
        HoverFocus hoverFocus = this->_hoverFocus.value(component, Nothing);

        if (height > 8)
        {
            // Close button
            int buttonRadius = height < 13 ? (height - 3) / 2 : 5;
            int buttonDiameter = buttonRadius * 2 + 1;
            int crossRadius = buttonRadius == 5 ? 3 : buttonRadius - 1;
            painter->setRenderHint(QPainter::Antialiasing, true);
            int middle = height / 2;
            painter->setPen(Qt::NoPen);
            painter->setBrush(QColor(0, 0, 0, hoverFocus == Close ? 170 : 70));
            painter->drawEllipse(width - 11 - buttonRadius, middle - buttonRadius, buttonDiameter, buttonDiameter);
            painter->setPen(QColor(255, 255, 255, hoverFocus == Close ? 255 : 200));
            painter->drawLine(width - 11 - crossRadius + 1, middle - crossRadius + 1, width - 11 + crossRadius, middle + crossRadius);
            painter->drawLine(width - 11 - crossRadius + 1, middle + crossRadius, width - 11 + crossRadius, middle - crossRadius + 1);
            painter->setRenderHint(QPainter::Antialiasing, false);
        }
    }

    /**
     *  \brief Width of control aspect.
     */
    int ControlAspect::width() const
    {
        return 21;
    }

}
