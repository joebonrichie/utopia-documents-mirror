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

#ifndef GRAFFITI_GRIDVIEW_H
#define GRAFFITI_GRIDVIEW_H

#include <QColor>
#include <QFrame>

namespace Graffiti
{

    class Grid;
    class Header;

    class GridViewPrivate;
    class GridView : public QFrame
    {
        Q_OBJECT

        Q_PROPERTY(QColor cursorColor
                   READ cursorColor
                   WRITE setCursorColor)
        Q_PROPERTY(QColor gridColor
                   READ gridColor
                   WRITE setGridColor)

    public:
        GridView(QWidget * parent = 0);
        ~GridView();

        Grid * grid();

        QColor cursorColor() const;
        QColor gridColor() const;
        const QVector< QRectF > & obstacles() const;
        void setCursorColor(const QColor & color);
        void setGridColor(const QColor & color);
        void setHorizontalHeader(Header * header);
        void setObstacles(const QVector< QRectF > & obstacles);
        void setRotation(int rotation);
        void setVerticalHeader(Header * header);
        void setViewportRect(const QRectF & rect);

    protected:
        void contextMenuEvent(QContextMenuEvent * event);
        void leaveEvent(QEvent * event);
        void mouseMoveEvent(QMouseEvent * event);
        void mousePressEvent(QMouseEvent * event);
        void mouseReleaseEvent(QMouseEvent * event);
        void paintEvent(QPaintEvent * event);

    protected:
        GridViewPrivate * d;

    }; // class GridView

} // namespace Graffiti

#endif // GRAFFITI_GRIDVIEW_H
