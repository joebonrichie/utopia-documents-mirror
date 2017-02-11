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

#ifndef GRAFFITI_GRIDVIEW_P_H
#define GRAFFITI_GRIDVIEW_P_H

#include <QColor>
#include <QObject>
#include <QPoint>
#include <QRect>
#include <QTransform>
#include <QVector>

namespace Graffiti
{

    class Grid;
    class GridView;
    class Header;

    class GridViewPrivate : public QObject
    {
        Q_OBJECT

    public:
        typedef enum {
            Idle,
            Dragging
        } State;

        GridViewPrivate(GridView * gridView);
        ~GridViewPrivate();

        GridView * gridView;
        Grid * grid;

        Header * horizontalHeader;
        Header * verticalHeader;

        struct {
            struct {
                QPoint pos;
                QPointF values;
                QPoint boundaries;
                QPoint cellCoords;
            } move;
            struct {
                QPoint pos;
                QPointF values;
                QPoint boundaries;
                QPoint cellCoords;
            } press;
        } mouse;
        QPoint draggingBoundaries;
        QPointF draggingOffsets;
        bool context;
        State interactionState;
        int rotation;
        QTransform transform;

        QRectF viewportRect;

        QColor gridColor;
        QColor cursorColor;

        QVector< QRectF > obstacles;
        QVector< QRectF > obstaclesUnderCursor;

        void mousePositionChanged(const QPoint & point);

        double translateFromWidgetHorizontal(int position) const;
        double translateFromWidgetVertical(int position) const;
        int translateToWidgetHorizontal(double position) const;
        int translateToWidgetVertical(double position) const;

        QRect visibleRectForCell(int row, int column) const;

        void setHorizontalObstacleBoundary(double boundary);
        void setObstacleBoundaries(double horizontal, double vertical);
        void setVerticalObstacleBoundary(double boundary);

    public slots:
        void onAddColumnLine();
        void onAddRowLine();
        void onAddRowAndColumnLines();
        void onHorizontalHeaderCursorChanged(bool visible, double value);
        void onHorizontalHeaderDestroyed(QObject * obj);
        void onHorizontalHeaderDragCompleted();
        void onHorizontalHeaderDragInitiated(int boundary);
        void onHorizontalHeaderDragUpdated();
        void onRemoveColumnLine();
        void onRemoveRowLine();
        void onRemoveRowAndColumnLines();
        void onVerticalHeaderCursorChanged(bool visible, double value);
        void onVerticalHeaderDestroyed(QObject * obj);
        void onVerticalHeaderDragCompleted();
        void onVerticalHeaderDragInitiated(int boundary);
        void onVerticalHeaderDragUpdated();

    }; // class GridViewPrivate

} // namespace Graffiti

#endif // GRAFFITI_GRIDVIEW_P_H
