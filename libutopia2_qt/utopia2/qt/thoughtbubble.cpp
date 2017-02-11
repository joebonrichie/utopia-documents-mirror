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

#include <utopia2/qt/thoughtbubble.h>

#include <QPainter>

namespace Utopia
{

    ThoughtBubble::ThoughtBubble(QWidget * parent, bool reversed) : QWidget(parent), _reversed(reversed)
    {
        this->setColor(QColor(80,80,80));
    }
    
    void ThoughtBubble::paintEvent(QPaintEvent * event)
    {
        QPainter painter(this);

        painter.setRenderHint(QPainter::TextAntialiasing, true);
        painter.setRenderHint(QPainter::Antialiasing, true);

        QRect rect =  this->rect().adjusted(1,1,-1,-1);

        int pipWidth = 10;
        int pipOffset = 24;
        QPainterPath bubbleRect;

        if (!_reversed)
        {
            bubbleRect.addRoundedRect(QRect(rect.topLeft()+QPoint(pipWidth,0), rect.bottomRight()),5,5);
        }
        else
        {
            bubbleRect.addRoundedRect(QRect(rect.topLeft(), rect.bottomRight()-QPoint(pipWidth,0)),5,5);
        }

        QPainterPath bubblePip;

        float alpha = 1.5; // ratio between the diameters of the two circles
        float spacer = 1; // padding around the circles
        float d1 = (pipWidth - 3 * spacer) / (1 + alpha);
        float d2 = alpha * d1;

        float dx1 = spacer;
        float dx2 = dx1 + d1 + spacer;

        float y1 = rect.top()  + pipOffset - d1/2;
        float y2 = rect.top()  + pipOffset - d2/2;

        if (!_reversed)
        {
            bubblePip.addEllipse(rect.left() + dx1, y1, d1, d1);
            bubblePip.addEllipse(rect.left() + dx2, y2, d2, d2);
        }
        else
        {
            bubblePip.addEllipse(rect.right() - dx1 - d1, y1, d1, d1);
            bubblePip.addEllipse(rect.right() - dx2 - d2, y2, d2, d2);
        }

        QPainterPath bubble = bubblePip.united(bubbleRect);

        painter.setBrush(this->color);
        painter.setPen(Qt::black);
        painter.drawPath(bubble);
    }

} // namespace Utopia
