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

#include <utopia2/qt/closebutton.h>

#include <QPainter>

namespace Utopia
{
    CloseButton::CloseButton(QWidget * parent) : QToolButton(parent)
    {
        setFixedSize(11, 11);
        setContentsMargins(0, 0, 0, 0);
    }

    void CloseButton::paintEvent(QPaintEvent * event)
    {
        QPainter paint(this);
        paint.setRenderHint(QPainter::Antialiasing, true);
        paint.setRenderHint(QPainter::TextAntialiasing, true);

        paint.setBrush(QColor(200, 200, 200));
        paint.setPen(Qt::NoPen);

        paint.drawEllipse(rect());

        paint.setPen(QColor(50, 50, 50));
        paint.setBrush(Qt::NoBrush);

        QRect adjustedRect(rect());
        adjustedRect.adjust(0, 0, 1, 1);

        paint.drawLine(adjustedRect.topLeft() + QPoint(3, 3), adjustedRect.bottomRight() + QPoint(-3, -3));
        paint.drawLine(adjustedRect.topRight() + QPoint(-3, 3), adjustedRect.bottomLeft() + QPoint(3, -3));
    }

    EmptyButton::EmptyButton(QWidget * parent) : QToolButton(parent)
    {}

    void EmptyButton::paintEvent(QPaintEvent *)
    {}


} // namespace Utopia
