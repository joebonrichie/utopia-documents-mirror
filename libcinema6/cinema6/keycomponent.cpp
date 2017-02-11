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
#include <cinema6/keycomponent.h>

#include <QPainter>
#include <math.h>

namespace CINEMA6
{

    /**
     *  \brief Named component construction.
     */
    KeyComponent::KeyComponent()
        : Component()
    {
        // KeyComponent objects are not resizable
        this->setResizable(false);
    }

    /**
     *  \brief Destructor.
     */
    KeyComponent::~KeyComponent()
    {}

    /**
     *  \brief Query this component's height.
     *  \return this component's height.
     */
    int KeyComponent::height() const
    {
        return 24;
    }

    /**
     *  \brief Paint sequence to screen.
     */
    void KeyComponent::paint(QPainter * painter, const QRect & rect)
    {
        // Only if parented
        if (this->alignmentView())
        {
            // Set font to be a little smaller than usual
            QFont font = painter->font();
            font.setPointSizeF(8);
            painter->setFont(font);

            QPair< int, AlignmentView::ComponentPosition > logicalKeyPosition = this->alignmentView()->componentPosition(this);
            int actualKeyPosition = this->alignmentView()->logicalToActualComponent(logicalKeyPosition.first, logicalKeyPosition.second);
            bool top = actualKeyPosition != 0;
            bool bottom = actualKeyPosition != (this->alignmentView()->componentCount() - 1);

            // Draw control
            painter->setPen(Qt::NoPen);
            painter->setBrush(QColor(245, 245, 255));
            painter->drawRect(rect);

            // Set style
            painter->setPen(this->alignmentView()->palette().color(QPalette::Dark));
            QPen pen = painter->pen();
            pen.setWidth(1);
            painter->setPen(pen);

            // Find cell sizes
            double unitSize = this->alignmentView()->unitSizeF();
            int firstUnit = this->alignmentIndexAt(rect.topLeft());
            int lastUnit = this->alignmentIndexAt(rect.topRight());

            // Calculate legend frequency
            int interval = 1;
            if (unitSize < 100)
            {
                double unitsIn100 = 100 / unitSize;
                double log = log10(unitsIn100);
                interval = pow(10, ceil(log));
//            if (log > 1.3) { interval = 20; }
//            else if (log > 0.8) { interval = 10; }
//            else { interval = 5; }
            }

            for (int actual = firstUnit; actual <= lastUnit; ++actual)
            {
                int index = actual + 1;
                QRectF unitRect = this->rectAt(actual);
                int unitMiddle = unitRect.center().x();
                int ext = 0;
                if (index % interval == 0)
                {
                    ext = 3;
                }

                // Ticks
                if (bottom)
                {
                    painter->drawLine(unitMiddle, this->height() - 1, unitMiddle, this->height() - 3 - ext);
                }
                if (top)
                {
                    painter->drawLine(unitMiddle, 0, unitMiddle, 2 + ext);
                }

                // Legend
                if (index % interval == 0)
                {
                    QRect textRect(unitRect.left() - 30, unitRect.top() + 1, unitRect.width() + 60, unitRect.bottom());
                    painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignHCenter, QString("%1").arg(index));
                }
            }

            pen = painter->pen();
            pen.setWidth(1);
            painter->setPen(pen);
            if (bottom)
            {
                painter->drawLine(rect.left(), this->height() - 1, rect.right(), this->height() - 1);
            }
            if (top)
            {
                painter->drawLine(rect.left(), 0, rect.right(), 0);
            }
        }
    }

}
