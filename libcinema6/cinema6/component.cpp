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

#include <cinema6/component.h>
#include <cinema6/alignmentview.h>
#include <cinema6/renderevent.h>

#include <QPainter>
#include <QScrollBar>

namespace CINEMA6
{

    class ComponentPrivate
    {
    public:
        ComponentPrivate()
            : heightFactor(1),
              top(0),
              relative(false)
            {}

        // Height information
        double heightFactor;
        int top;
        bool relative;
        // Painting cache
        QPixmap pixmap;
        QRect pixmapRect;
    };


    /**
     *  \brief Default component construction.
     */
    Component::Component(const QString & title)
        : AbstractComponent(title), d(new ComponentPrivate)
    {}

    /**
     *  \brief Destructor.
     */
    Component::~Component()
    {}

    /**
     *  \brief Find the horizonal alignment index for a particular mouse position.
     */
    int Component::alignmentIndexAt(const QPoint & pos)
    {
        return (int) (pos.x() / alignmentView()->unitSizeF());
    }

    /**
     *  \brief Query this component's height.
     *  \return this component's height.
     */
    int Component::height() const
    {
        if (displayFlags().testFlag(Minimised))
        {
            return 12;
        }
        else
        {
            int height = (int) (alignmentView()->unitSizeF() * heightFactor());
            return qMax(height, 2);
        }
    }

    /**
     *  \brief Query this component's height factor.
     *  \return this component's height factor.
     */
    double Component::heightFactor() const
    {
        return d->heightFactor;
    }

    int Component::left() const
    {
        if (alignmentView())
        {
            if (alignmentView()->isDiscreteScrolling())
            {
                return alignmentView()->documentGeometry().left() - (int) (alignmentView()->unitSizeF() * alignmentView()->horizontalScrollBar()->value());
            }
            else
            {
                return alignmentView()->documentGeometry().left() - alignmentView()->horizontalScrollBar()->value();
            }
        }
        else
        {
            return 0;
        }
    }

    void Component::paint(QPainter * painter, const QRect & rect) {}

    QRectF Component::rectAt(int index) const
    {
        if (alignmentView())
        {
            double unitSize = alignmentView()->unitSizeF();
            return QRectF(unitSize * index, 0, unitSize, height());
        }
        else
        {
            return QRectF();
        }
    }

    /**
     *  \brief Paint sequence to screen.
     */
    void Component::render(QPaintDevice * target, const QPoint & targetOffset, const QRect & sourceRect) {}

    void Component::renderEvent(RenderEvent * event)
    {
        QRect fullRect(event->rect().left(), 0, event->rect().width(), height());
        QPixmap pixmap(fullRect.size());
        pixmap.fill(QColor(255, 255, 255, 255));

        if (d->pixmapRect.isValid() && d->pixmapRect.intersects(fullRect))
        {
            {
                QPainter painter(&pixmap);
                painter.drawPixmap(d->pixmapRect.topLeft() - fullRect.topLeft(), d->pixmap);
            }
            QRect leftRect = QRect(fullRect.topLeft(), d->pixmapRect.bottomLeft() + QPoint(-1, 0));
            if (leftRect.isValid())
            {
                render(&pixmap, leftRect.topLeft() - fullRect.topLeft(), leftRect);
            }
            QRect rightRect = QRect(d->pixmapRect.topRight() + QPoint(1, 0), fullRect.bottomRight());
            if (rightRect.isValid())
            {
                render(&pixmap, rightRect.topLeft() - fullRect.topLeft(), rightRect);
            }
        }
        else
        {
            render(&pixmap, QPoint(), event->rect());
        }

        event->painter()->drawPixmap(fullRect.topLeft(), pixmap);
        d->pixmap = pixmap;
        d->pixmapRect = fullRect;

        event->painter()->save();
        paint(event->painter(), event->rect());
        event->painter()->restore();
    }

    /**
     *  \brief Set this component's height.
     *  \param height height to set.
     */
    void Component::setHeight(int height)
    {
        if (isResizable())
        {
            setHeightFactor(height / alignmentView()->unitSizeF());
        }
    }

    /**
     *  \brief Set this component's height factor.
     *  \param heightFactor height factor to set.
     */
    void Component::setHeightFactor(double heightFactor)
    {
        if (heightFactor < 1) { heightFactor = 1; }

        if (isResizable() && d->heightFactor != heightFactor)
        {
            d->heightFactor = heightFactor;
            updateGeometry();
        }
    }

    void Component::setRelativeTop(int top)
    {
        if (!d->relative || d->top != top)
        {
            d->relative = true;
            d->top = top;
            updateGeometry();
        }
    }

    void Component::setTop(int top)
    {
        if (d->relative || d->top != top)
        {
            d->relative = false;
            d->top = top;
            updateGeometry();
        }
    }

    int Component::top() const
    {
        if (d->relative && alignmentView())
        {
            return d->top + alignmentView()->documentGeometry().top() - alignmentView()->verticalScrollBar()->value();
        }
        else
        {
            return d->top;
        }
    }

    void Component::unitSizeUpdate(int)
    {
        d->pixmapRect = QRect();
    }

    void Component::zoomUpdate(double)
    {
        d->pixmapRect = QRect();
    }

    /**
     *  \brief Update this component's rendering.
     */
    void Component::update()
    {
        if (alignmentView()) { alignmentView()->viewport()->update(geometry().intersected(alignmentView()->documentGeometry())); }
        d->pixmapRect = QRect();
    }

    void Component::updateGeometry()
    {
        if (alignmentView())
        {
            QRect updateRect(0, top(), alignmentView()->width(), height());
            if (alignmentView()->componentPosition(this).second == AlignmentView::Center)
            {
                QRect documentGeometry(alignmentView()->documentGeometry());
                updateRect &= QRect(0, documentGeometry.top(), width(), documentGeometry.height());
            }
            alignmentView()->viewport()->update(updateRect);
        }
        d->pixmapRect = QRect();
    }

    /**
     *  \brief Query this component's width.
     *  \return this component's width.
     */
    int Component::width() const
    {
        return 100000; // FIXME
    }

    /**
     *  \brief Is this a component?
     */
    bool isComponent(const AbstractComponent * abstractComponent)
    {
        return (abstractComponent != 0 && abstractComponent == dynamic_cast< const Component* >(abstractComponent));
    }

}
