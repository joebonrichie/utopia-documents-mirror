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
#include <cinema6/aspect.h>
#include <cinema6/component.h>
#include <cinema6/renderevent.h>

#include <QMouseEvent>
#include <QPainter>
#include <QPointer>

namespace CINEMA6
{

    class AspectPrivate
    {
    public:
        AspectPrivate(Aspect * aspect)
            : aspect(aspect),
              widthFactor(1),
              left(0),
              previousHoverComponent(0)
            {}

        Aspect * aspect;

        // Height information
        double widthFactor;
        int left;
        QPointer< Component > previousHoverComponent;

        QRect geometryFor(Component * component) const
            {
                return QRect(0, component->top(), aspect->width(), component->height());
            }
    };


    /**
     *  \brief Named component construction.
     */
    Aspect::Aspect(const QString & title)
        : AbstractComponent(title), d(new AspectPrivate(this))
    {}

    /**
     *  \brief Destructor.
     */
    Aspect::~Aspect()
    {}

    bool Aspect::event(QEvent * event)
    {
        if (AlignmentView * av = alignmentView())
        {
            if (event->type() == QEvent::MouseButtonDblClick ||
                event->type() == QEvent::MouseMove ||
                event->type() == QEvent::MouseButtonPress ||
                event->type() == QEvent::MouseButtonRelease)
            {
                QMouseEvent * mouseEvent = (QMouseEvent *) event;
                if (Component * component = av->componentUnder(mouseEvent->y()))
                {
                    QMouseEvent aspectEvent(mouseEvent->type(),
                                            av->mapTo(this, mouseEvent->pos()) - QPoint(0, component->top()),
                                            mouseEvent->globalPos(),
                                            mouseEvent->button(),
                                            mouseEvent->buttons(),
                                            mouseEvent->modifiers());
                    switch (event->type())
                    {
                    case QEvent::MouseButtonDblClick:
                        mouseDoubleClickEvent(component, &aspectEvent);

                    case QEvent::MouseMove:
                        mouseMoveEvent(component, &aspectEvent);

                        if (d->previousHoverComponent != component)
                        {
                            if (d->previousHoverComponent)
                            {
                                leaveEvent(d->previousHoverComponent);
                            }
                            d->previousHoverComponent = component;
                        }
                        break;

                    case QEvent::MouseButtonPress:
                        mousePressEvent(component, &aspectEvent);
                        break;

                    case QEvent::MouseButtonRelease:
                        mouseReleaseEvent(component, &aspectEvent);
                        break;

                    default:
                        break;
                    }
                }
            }
            else if (event->type() == QEvent::Leave)
            {
                if (d->previousHoverComponent)
                {
                    leaveEvent(d->previousHoverComponent);
                    d->previousHoverComponent = 0;
                }
            }
        }

        return AbstractComponent::event(event);
    }

    /**
     *  \brief Query this component's height.
     *  \return this component's height.
     */
    int Aspect::height() const
    {
        return alignmentView() ? alignmentView()->viewport()->height() : 0;
    }

    int Aspect::left() const
    {
        return d->left;
    }

    /**
     *  \brief Set this component's width.
     *  \param width width to set.
     */
    void Aspect::setWidth(int width)
    {
        if (isResizable())
        {
            setWidthFactor(width / alignmentView()->unitSizeF());
        }
    }

    /**
     *  \brief Set this component's width factor.
     *  \param widthFactor width factor to set.
     */
    void Aspect::setWidthFactor(double widthFactor)
    {
        if (widthFactor < 1) { widthFactor = 1; }

        if (isResizable() && d->widthFactor != widthFactor)
        {
            d->widthFactor = widthFactor;
            update();
        }
    }

    void Aspect::setLeft(int left)
    {
        if (d->left != left)
        {
            d->left = left;
            update();
        }
    }

    /**
     *  \brief Query this component's width.
     *  \return this component's width.
     */
    int Aspect::width() const
    {
        if (displayFlags().testFlag(Minimised))
        {
            return 12;
        }
        else
        {
            return (int) (alignmentView()->unitSizeF() * widthFactor());
        }
    }

    /**
     *  \brief Query this component's height factor.
     *  \return this component's height factor.
     */
    double Aspect::widthFactor() const
    {
        return d->widthFactor;
    }

    void Aspect::leaveEvent(Component *) {}

    void Aspect::mouseDoubleClickEvent(Component *, QMouseEvent *) {}
    void Aspect::mouseMoveEvent(Component *, QMouseEvent *) {}
    void Aspect::mousePressEvent(Component *, QMouseEvent *) {}
    void Aspect::mouseReleaseEvent(Component *, QMouseEvent *) {}

    void Aspect::paint(QPainter * painter, const QRect & rect) {}
    void Aspect::paint(QPainter *, const QRect &, const QRect &, Component *) {}

    /**
     *  \brief Render aspect to screen.
     */
    void Aspect::renderEvent(RenderEvent * event)
    {
        // Start out by rendering whole aspect
        event->painter()->save();
        paint(event->painter(), event->rect());
        event->painter()->restore();

        // For each component, parcel out painting
        for (int index = 0; index < alignmentView()->componentCount(AlignmentView::Top); ++index)
        {
            Component * component = alignmentView()->componentAt(index, AlignmentView::Top);
            QRect geometry = d->geometryFor(component);
            QRect clipped = event->rect().intersected(geometry);

            // ignore if not within rect
            if (clipped.isEmpty()) { continue; }

            event->painter()->save();
            event->painter()->setClipRect(geometry);
            event->painter()->translate(geometry.topLeft());

            // Delegate painting
            clipped.translate(-geometry.topLeft());
            paint(event->painter(), QRect(0, 0, geometry.width(), geometry.height()), clipped, component);
            event->painter()->restore();
        }
        for (int index = 0; index < alignmentView()->componentCount(AlignmentView::Center); ++index)
        {
            Component * component = alignmentView()->componentAt(index, AlignmentView::Center);
            QRect geometry = d->geometryFor(component);
            QRect documentGeometry = alignmentView()->documentGeometry();
            QRect visibleGeometry = geometry.intersected(QRect(0, documentGeometry.top(), width(), documentGeometry.height()));
            QRect clipped = visibleGeometry.intersected(event->rect());

            // ignore if not within rect
            if (clipped.isEmpty()) { continue; }

            event->painter()->save();
            event->painter()->setClipRect(clipped);
            event->painter()->translate(geometry.topLeft());

            // Delegate painting
            clipped.translate(-geometry.topLeft());
            paint(event->painter(), QRect(0, 0, geometry.width(), geometry.height()), clipped, component);
            event->painter()->restore();
        }
        for (int index = 0; index < alignmentView()->componentCount(AlignmentView::Bottom); ++index)
        {
            Component * component = alignmentView()->componentAt(index, AlignmentView::Bottom);
            QRect geometry = d->geometryFor(component);
            QRect clipped = event->rect().intersected(geometry);

            // ignore if not within rect
            if (clipped.isEmpty()) { continue; }

            event->painter()->save();
            event->painter()->setClipRect(geometry);
            event->painter()->translate(geometry.topLeft());

            // Delegate painting
            clipped.translate(-geometry.topLeft());
            paint(event->painter(), QRect(0, 0, geometry.width(), geometry.height()), clipped, component);
            event->painter()->restore();
        }
    }

    int Aspect::top() const
    {
        return 0;
    }

    /**
     *  \brief Update this aspect's rendering.
     */
    void Aspect::update()
    {
        if (alignmentView()) { alignmentView()->viewport()->update(geometry()); }
    }

    void Aspect::update(Component * component)
    {
        if (alignmentView()) { alignmentView()->viewport()->update(d->geometryFor(component).translated(left(), 0)); }
    }

    /**
     *  \brief Is this an aspect?
     */
    bool isAspect(const AbstractComponent * abstractComponent)
    {
        return (abstractComponent != 0 && abstractComponent == dynamic_cast< const Aspect* >(abstractComponent));
    }

}
