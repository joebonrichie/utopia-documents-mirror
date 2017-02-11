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
#include <cinema6/sequencecomponent.h>

#include <QLinearGradient>
#include <QPainter>

namespace CINEMA6
{

    QPixmap DoubleHelixPixmapFactory::foreground(QSizeF size)
    {
        QPixmap pixmap = this->_foregrounds.value(size, QPixmap());

        // If not found...
        if (pixmap.isNull())
        {
            // Calculate important points
            int unitSize = size.width();
            float ribbonWidth = qMax(unitSize * 0.9, 5.0);
            QPointF p0(unitSize / 2.0, 1);
            QPointF c0(11 * unitSize / 6.0, 1);
            QPointF c1(19 * unitSize / 6.0, size.height() - 1);
            QPointF p1(9 * unitSize / 2.0, size.height() - 1);
            QPointF ribbonOffset(ribbonWidth / 2.0, 0);

            // Generate transparent pixmap
            pixmap = QPixmap(QSize(unitSize * 5, size.height()));
            pixmap.fill(QColor(0, 0, 0, 0));

            // Render foreground ribbon
            QPainter painter(&pixmap);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setPen(Qt::NoPen);
            QLinearGradient grad(0, 0, 0, size.height());
            grad.setColorAt(0, QColor(130, 130, 210));
            grad.setColorAt(0.2, QColor(200, 200, 255));
            grad.setColorAt(0.3, QColor(200, 200, 255));
            grad.setColorAt(0.5, QColor(130, 130, 210));
            grad.setColorAt(0.8, QColor(130, 130, 210));
            grad.setColorAt(1, QColor(100, 100, 170));
            painter.setBrush(grad);
            QPainterPath ribbon;
            ribbon.moveTo(p0 + ribbonOffset);
            ribbon.cubicTo(c0 + ribbonOffset, c1 + ribbonOffset, p1 + ribbonOffset);
            ribbon.lineTo(p1 - ribbonOffset);
            ribbon.cubicTo(c1 - ribbonOffset, c0 - ribbonOffset, p0 - ribbonOffset);
            ribbon.lineTo(p0 + ribbonOffset);
            ribbon.closeSubpath();
            painter.drawPath(ribbon);
            painter.end();

            // Cache
            this->_foregrounds[size] = pixmap;
        }

        // Return
        return pixmap;
    }

    QPixmap DoubleHelixPixmapFactory::background(QSizeF size)
    {
        QPixmap pixmap = this->_backgrounds.value(size, QPixmap());

        // If not found...
        if (pixmap.isNull())
        {
            // Calculate important points
            int unitSize = size.width();
            float ribbonWidth = qMax(unitSize * 0.9, 5.0);
            QPointF p0(unitSize / 2.0, size.height() - 1);
            QPointF c0(11 * unitSize / 6.0, size.height() - 1);
            QPointF c1(19 * unitSize / 6.0, 1);
            QPointF p1(9 * unitSize / 2.0, 1);
            QPointF ribbonOffset(ribbonWidth / 2.0, 0);

            // Generate transparent pixmap
            pixmap = QPixmap(QSize(unitSize * 5, size.height()));
            pixmap.fill(QColor(0, 0, 0, 0));

            // Render foreground ribbon
            QPainter painter(&pixmap);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(60, 60, 80));
            QPainterPath ribbon;
            ribbon.moveTo(p0 + ribbonOffset);
            ribbon.cubicTo(c0 + ribbonOffset, c1 + ribbonOffset, p1 + ribbonOffset);
            ribbon.lineTo(p1 - ribbonOffset);
            ribbon.cubicTo(c1 - ribbonOffset, c0 - ribbonOffset, p0 - ribbonOffset);
            ribbon.lineTo(p0 + ribbonOffset);
            ribbon.closeSubpath();
            painter.drawPath(ribbon);
            painter.end();

            // Cache
            this->_backgrounds[size] = pixmap;
        }

        // Return
        return pixmap;
    }

    QPixmap DoubleHelixPixmapFactory::base(QSizeF size, int index)
    {
        QPixmap pixmap = this->_bases[size].value(index % 8, QPixmap());

        // If not found...
        if (pixmap.isNull())
        {
            // Calculate important points
            int unitSize = size.width();
            float rungWidth = qMax(unitSize * 0.4, 2.0);
            static float radii[8] = { 0.74, 1, 0.74, 0.74, 1, 0.74, 0.10, 0.10 };
            static int yRnds[8] = { 10, 10, 30, 25, 0, 0, 30, 25 };
            float upRadius = qMin(radii[index % 8] * size.height() / 2.0, size.height() / 2.0 - 1);
            float downRadius = qMin(radii[(index + 4) % 8] * size.height() / 2.0, size.height() / 2.0 - 1);
            int yRnd = yRnds[index % 8];
            QRectF rung((unitSize - rungWidth) / 2.0, size.height() / 2.0 - upRadius, rungWidth, upRadius + downRadius);

            // Generate transparent pixmap
            pixmap = QPixmap(QSize(unitSize, size.height()));
            pixmap.fill(QColor(0, 0, 0, 0));

            // Render foreground ribbon
            QPainter painter(&pixmap);
            painter.setRenderHint(QPainter::Antialiasing);
            QLinearGradient grad(rung.left(), rung.top(), rung.right(), rung.top());
            grad.setColorAt(0, QColor(140, 60, 60).darker(((index + 1) % 4 - 1.5) * 10 + 100));
            grad.setColorAt(1, QColor(200, 120, 120).darker(((index + 1) % 4 - 1.5) * 10 + 100));
            painter.setBrush(grad);
            painter.setPen(Qt::NoPen);
            painter.drawRoundRect(rung, 99, yRnd);
            painter.end();

            // Cache
            this->_bases[size][index % 8] = pixmap;
        }

        // Return
        return pixmap;
    }



    /**
     *  \brief Default component construction.
     */
    AnnotationComponent::AnnotationComponent(const QString & title)
        : DataComponent(title), _sequenceComponent(0)
    {}

    /**
     *  \brief Destructor.
     */
    AnnotationComponent::~AnnotationComponent()
    {}

    int AnnotationComponent::height() const
    {
        int height = this->Component::height();
        return qMax(height, 20);
    }

    /**
     *  \brief Paint sequence to screen.
     */
    void AnnotationComponent::paint(QPainter * painter, const QRect & rect)
    {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(255, 255, 255));
        painter->drawRect(rect);
        painter->setRenderHint(QPainter::Antialiasing);

        // Find cell sizes
        double unitSize = this->alignmentView()->unitSizeF();
        if (unitSize < 1)
        {
            painter->setPen(Qt::NoPen);
            QLinearGradient grad(0, 0, 0, this->height());
            grad.setColorAt(0, QColor(130, 130, 210));
            grad.setColorAt(0.2, QColor(200, 200, 255));
            grad.setColorAt(0.3, QColor(200, 200, 255));
            grad.setColorAt(0.5, QColor(130, 130, 210));
            grad.setColorAt(0.8, QColor(130, 130, 210));
            grad.setColorAt(1, QColor(100, 100, 170));
            painter->setBrush(grad);
            painter->drawRect(rect.adjusted(0, 1, 0, -1));
        }
        else
        {
            int firstUnit = this->alignmentIndexAt(rect.topLeft());
            int lastUnit = this->alignmentIndexAt(rect.topRight());

            if (true || this->sequenceComponent())
            {
                firstUnit -= 8;
                firstUnit -= firstUnit % 8;

                painter->setPen(Qt::NoPen);
                painter->setBrush(QColor(60, 60, 80));
                bool odd = true;
                for (int res = firstUnit; res <= lastUnit + 8; res += 3)
                {
                    double unitLeft = this->rectAt(res).left();
                    QPixmap ribbon = this->pixmapFactory().background(QSizeF(unitSize, this->height()));
                    painter->drawPixmap(unitLeft, 0, ribbon);

                    // Skip to next position
                    if (odd)
                    {
                        res += 2;
                        odd = false;
                    }
                    else
                    {
                        odd = true;
                    }
                }

                for (int res = this->alignmentIndexAt(rect.topLeft()); res <= lastUnit; ++res)
                {
                    double unitLeft = this->rectAt(res).left();
                    QPixmap base = this->pixmapFactory().base(QSizeF(unitSize, this->height()), res);
                    painter->drawPixmap(unitLeft, 0, base);
                }

                odd = true;
                for (int res = firstUnit; res <= lastUnit + 8; res += 3)
                {
                    double unitLeft = this->rectAt(res + 1).left();
                    QPixmap ribbon = this->pixmapFactory().foreground(QSizeF(unitSize, this->height()));
                    painter->drawPixmap(unitLeft, 0, ribbon);

                    // Skip to next position
                    if (odd)
                    {
                        odd = false;
                    }
                    else
                    {
                        res += 2;
                        odd = true;
                    }
                }
            }
            else
            {
                painter->setBrush(QColor(255, 235, 235));
                painter->drawRect(rect);
                painter->setBrush(QColor(255, 205, 205));

                int unitLeft = firstUnit * unitSize;

                for (int res = firstUnit; res <= lastUnit; ++res)
                {
                    QRectF resRect(unitLeft, 0, unitSize, this->height());
                    float oneQuarter = unitLeft + unitSize / 4.0;
                    float threeQuarters = unitLeft + 3 * unitSize / 4.0;
                    QPolygonF triangle;
                    triangle << QPoint(oneQuarter, 0) << QPoint(resRect.left(), 0) << QPoint(resRect.left(), resRect.height() / 2);
                    painter->drawPolygon(triangle);
                    QPolygonF pentagon;
                    pentagon << QPoint(threeQuarters, 0) << QPoint(oneQuarter, resRect.height()) << QPoint(threeQuarters, resRect.height()) << QPoint(resRect.right(), resRect.height() / 2) << QPoint(resRect.right(), 0);
                    painter->drawPolygon(pentagon);

                    // Skip to next position
                    unitLeft += unitSize;
                }
            }
        }
    }

    Sequence * AnnotationComponent::sequence() const
    {
        return this->sequenceComponent()->sequence();
    }

    SequenceComponent * AnnotationComponent::sequenceComponent() const
    {
        return this->_sequenceComponent;
    }

    void AnnotationComponent::setSequenceComponent(SequenceComponent * sequenceComponent)
    {
        this->_sequenceComponent = sequenceComponent;
    }

}
