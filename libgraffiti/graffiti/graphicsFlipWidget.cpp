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

#include <graffiti/graphicsFlipWidget.h>
#include <QDrag>
#include <QGraphicsProxyWidget>
#include <QPainter>
#include <QPen>
#include <QWidget>
#include <math.h>
#include <QMouseEvent>
#include <QDebug>

namespace Graffiti
{

    GraphicsFlipWidget::GraphicsFlipWidget(QWidget * frontWidget, QWidget * backWidget, QGraphicsItem * parent)
        : QGraphicsItem(parent), flipTimeLine(500)
    {

        // General widget setup
        setFlags(QGraphicsItem::ItemIsFocusable);

        frontProxyWidget = new QGraphicsProxyWidget(this);
        frontProxyWidget->setFocusPolicy(Qt::StrongFocus);
        frontProxyWidget->setWidget(frontWidget);
        //frontProxyWidget->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
        frontProxyWidget->show();

        backProxyWidget = new QGraphicsProxyWidget(this);
        backProxyWidget->setFocusPolicy(Qt::StrongFocus);
        backProxyWidget->setWidget(backWidget);
        //backProxyWidget->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
        backProxyWidget->hide();

        // Set up timeline for flip
        flipTimeLine.setUpdateInterval(20);
        connect(&flipTimeLine, SIGNAL(valueChanged(qreal)), this, SLOT(animateFlip(qreal)));
        connect(&flipTimeLine, SIGNAL(finished()), this, SLOT(regainFocus()));

        this->setFocus(Qt::MouseFocusReason);

    }

    GraphicsFlipWidget::~GraphicsFlipWidget()
    {
    }


    void GraphicsFlipWidget::animateFlip(qreal val)
    {
        struct
        {
            QSizeF size;
            QPointF center;
        } from, to, step;

        QGraphicsProxyWidget * visible = visibleProxyWidget();
        QGraphicsProxyWidget * hidden = hiddenProxyWidget();

        // Ensure visibility of widgets
        if (!visible->isVisible())
        {
            visible->show();
            visible->setGeometry(visible->geometry());
            //visible->setFocus();
        }
        if (hidden->isVisible())
        {
            hidden->hide();
        }

        // Get geometry information
        QSizeF size = visible->size();
        from.size = frontProxyWidget->size();
        from.center = frontProxyWidget->geometry().center();
        to.size = backProxyWidget->size();
        to.center = backProxyWidget->geometry().center();

        // Interpolate
        QTransform transform;
        transform.translate(size.width() / 2.0, size.height() / 2.0);
        if (isFlipped())
        {
            // Interpolate position
            step.center = (to.center - from.center) * (1 - val);
            transform.translate(-step.center.x(), -step.center.y());

            // Interpolate size
            qreal mid = 1 + (1 - val) * (from.size.height() / to.size.height() - 1);
            transform.scale(mid, mid);

            transform.rotate(-90.0 * ((1 - val) / (1 - transitionPoint())), Qt::YAxis);
        }
        else
        {
            // Interpolate position
            step.center = (to.center - from.center) * val;
            transform.translate(step.center.x(), step.center.y());

            // Interpolate size
            qreal mid = 1 + val * (to.size.height() / from.size.height() - 1);
            transform.scale(mid, mid);

            // Interpolate position
            transform.rotate(90.0 * (val / transitionPoint()), Qt::YAxis);
        }
        transform.translate(-size.width() / 2.0, -size.height() / 2.0);
        visible->setTransform(transform);
        visible->setFocus(Qt::MouseFocusReason);

    }

    QWidget * GraphicsFlipWidget::backWidget() const
    {
        return backProxyWidget->widget();
    }

    QRectF GraphicsFlipWidget::boundingRect () const
    {
        return visibleProxyWidget()->geometry();
    }

    void GraphicsFlipWidget::flip()
    {
        if (flipTimeLine.state() != QTimeLine::Running)
        {
            flipTimeLine.stop();
            flipTimeLine.setDirection(isFlipped() ? QTimeLine::Backward : QTimeLine::Forward);
            flipTimeLine.start();
        }
    }

    QWidget * GraphicsFlipWidget::frontWidget() const
    {
        return frontProxyWidget->widget();
    }

    QGraphicsProxyWidget * GraphicsFlipWidget::hiddenProxyWidget() const
    {
        return isFlipped() ? frontProxyWidget : backProxyWidget;
    }

    QWidget * GraphicsFlipWidget::hiddenWidget() const
    {
        return hiddenProxyWidget()->widget();
    }

    bool GraphicsFlipWidget::isFlipped() const
    {
        return flipTimeLine.currentValue() > transitionPoint();
    }


    void GraphicsFlipWidget::keyPressEvent(QKeyEvent * event)
    {

        if (event->isAutoRepeat() || event->key() != Qt::Key_Return
            || flipTimeLine.state() == QTimeLine::Running)
        {
            QGraphicsItem::keyPressEvent(event);
            return;
        }

        // For debug purposes, and to allow a flashy slow-down
        if (event->modifiers().testFlag(Qt::ShiftModifier)) {
            flipTimeLine.setDuration(3000);
            if (flipTimeLine.currentTime() != 0) flipTimeLine.setCurrentTime(3000);
        } else {
            flipTimeLine.setDuration(600);
            if (flipTimeLine.currentTime() != 0) flipTimeLine.setCurrentTime(600);
        }

        this->setFocus(Qt::MouseFocusReason);
        this->backWidget()->setFocus(Qt::MouseFocusReason);
        this->backWidget()->setFocusPolicy(Qt::StrongFocus);
        flip();

    }

    void GraphicsFlipWidget::paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *)
    {
    }

    void GraphicsFlipWidget::regainFocus()
    {
        this->backProxyWidget->setFocus(Qt::MouseFocusReason);
    }

    qreal GraphicsFlipWidget::transitionPoint() const
    {
        return 0.5;
        // FIXME there follows a version that uses a different transition point
        return backProxyWidget->geometry().width() / (frontProxyWidget->geometry().width() + backProxyWidget->geometry().width());
    }

    QGraphicsProxyWidget * GraphicsFlipWidget::visibleProxyWidget() const
    {
        return isFlipped() ? backProxyWidget : frontProxyWidget;
    }

    QWidget * GraphicsFlipWidget::visibleWidget() const
    {
        return visibleProxyWidget()->widget();
    }

}
