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

#include <utopia2/qt/speechbubble.h>

#include <QPainter>

namespace Utopia
{

    SpeechBubble::SpeechBubble(QWidget * parent, bool reversed) : QWidget(parent), _reversed(reversed)
    {
        this->setColor(QColor(120,120,120));
    }

    void SpeechBubble::paintEvent(QPaintEvent * event)
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
        QPoint start;

        if (!_reversed)
        {
            start = rect.topLeft()+QPoint(0,pipOffset);
            bubblePip.moveTo(start);
            bubblePip.lineTo(start+QPoint(pipWidth+1,-pipWidth));
            bubblePip.lineTo(start+QPoint(pipWidth+1,pipWidth));
            bubblePip.lineTo(start);
        }
        else
        {
            start = rect.topRight()+QPoint(0,pipOffset);
            bubblePip.moveTo(start);
            bubblePip.lineTo(start-QPoint(pipWidth+1,-pipWidth));
            bubblePip.lineTo(start-QPoint(pipWidth+1,pipWidth));
            bubblePip.lineTo(start);
        }

        QPainterPath bubble = bubblePip.united(bubbleRect);

        painter.setBrush(this->color);
        painter.setPen(Qt::black);
        painter.drawPath(bubble);
    }

} // namespace Utopia
