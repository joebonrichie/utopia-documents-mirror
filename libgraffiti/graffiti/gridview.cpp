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

#include <graffiti/gridview.h>
#include <graffiti/gridview_p.h>
#include <graffiti/grid.h>
#include <graffiti/header.h>
#include <graffiti/sections.h>

#include <QMenu>
#include <QMouseEvent>
#include <QPainter>

#include <QDebug>

namespace Graffiti
{

    namespace {
        static const int aura = 3;
    }


    GridViewPrivate::GridViewPrivate(GridView * gridView)
        : QObject(gridView),
          gridView(gridView),
          grid(new Grid(this)),
          horizontalHeader(0),
          verticalHeader(0),
          context(false),
          interactionState(Idle),
          rotation(0),
          viewportRect(0, 0, 1, 1)
    {}

    GridViewPrivate::~GridViewPrivate()
    {}

    void GridViewPrivate::mousePositionChanged(const QPoint & point)
    {
        static const int aura = 3;

        mouse.move.pos = point;
        mouse.move.values = QPointF(translateFromWidgetHorizontal(mouse.move.pos.x()),
                                    translateFromWidgetVertical(mouse.move.pos.y()));
        mouse.move.boundaries = QPoint(-1, -1);
        mouse.move.cellCoords = QPoint(-1, -1);

        { // Horizontal boundary
            int idx = horizontalHeader->sections()->find(mouse.move.values.x());
            int count = horizontalHeader->sections()->count();
            if (idx >= 0 && idx < count) {
                Section section = horizontalHeader->sections()->sectionAt(idx);
                int boundaryLeft = translateToWidgetHorizontal(section.offset);
                int boundaryRight = translateToWidgetHorizontal(section.offset + section.size);
                if (abs(mouse.move.pos.x() - boundaryLeft) <= aura && idx > 0) {
                    mouse.move.boundaries.setX(idx);
                } else if (abs(mouse.move.pos.x() - boundaryRight) <= aura) {
                    mouse.move.boundaries.setX(idx + 1);
                }
                mouse.move.cellCoords.setX(idx);
            }
        }

        { // Vertical boundary
            int idx = verticalHeader->sections()->find(mouse.move.values.y());
            int count = verticalHeader->sections()->count();
            if (idx >= 0 && idx < count) {
                Section section = verticalHeader->sections()->sectionAt(idx);
                int boundaryTop = translateToWidgetVertical(section.offset);
                int boundaryBottom = translateToWidgetVertical(section.offset + section.size);
                if (abs(mouse.move.pos.y() - boundaryTop) <= aura && idx > 0) {
                    mouse.move.boundaries.setY(idx);
                } else if (abs(mouse.move.pos.y() - boundaryBottom) <= aura) {
                    mouse.move.boundaries.setY(idx + 1);
                }
                mouse.move.cellCoords.setY(idx);
            }
        }
    }

    void GridViewPrivate::onAddColumnLine()
    {
        horizontalHeader->sections()->addBoundary(mouse.press.values.x());
    }

    void GridViewPrivate::onAddRowLine()
    {
        verticalHeader->sections()->addBoundary(mouse.press.values.y());
    }

    void GridViewPrivate::onAddRowAndColumnLines()
    {
        onAddRowLine();
        onAddColumnLine();
    }

    void GridViewPrivate::onHorizontalHeaderCursorChanged(bool visible, double value)
    {
        // Update obstacles under cursor
        if (visible) {
            setHorizontalObstacleBoundary(value);
        } else {
            obstaclesUnderCursor.clear();
        }

        gridView->update();
    }

    void GridViewPrivate::onHorizontalHeaderDragCompleted()
    {
        obstaclesUnderCursor.clear();
        gridView->update();
    }

    void GridViewPrivate::onHorizontalHeaderDragInitiated(int boundary)
    {
        draggingBoundaries = QPoint(boundary, 0);
        setHorizontalObstacleBoundary(horizontalHeader->sections()->boundaryAt(draggingBoundaries.x()));
        gridView->update();
    }

    void GridViewPrivate::onHorizontalHeaderDragUpdated()
    {
        setHorizontalObstacleBoundary(horizontalHeader->sections()->boundaryAt(draggingBoundaries.x()));
        gridView->update();
    }

    void GridViewPrivate::onHorizontalHeaderDestroyed(QObject * /*obj*/)
    {
        gridView->setHorizontalHeader(0);
    }

    void GridViewPrivate::onRemoveColumnLine()
    {
        horizontalHeader->sections()->removeBoundary(mouse.press.boundaries.x());
    }

    void GridViewPrivate::onRemoveRowLine()
    {
        verticalHeader->sections()->removeBoundary(mouse.press.boundaries.y());
    }

    void GridViewPrivate::onRemoveRowAndColumnLines()
    {
        onRemoveRowLine();
        onRemoveColumnLine();
    }

    void GridViewPrivate::onVerticalHeaderCursorChanged(bool visible, double value)
    {
        // Update obstacles under cursor
        if (visible) {
            setVerticalObstacleBoundary(value);
        } else {
            obstaclesUnderCursor.clear();
        }

        gridView->update();
    }

    void GridViewPrivate::onVerticalHeaderDestroyed(QObject * /*obj*/)
    {
        gridView->setVerticalHeader(0);
    }

    void GridViewPrivate::onVerticalHeaderDragCompleted()
    {
        obstaclesUnderCursor.clear();
        gridView->update();
    }

    void GridViewPrivate::onVerticalHeaderDragInitiated(int boundary)
    {
        draggingBoundaries = QPoint(-1, boundary);
        setVerticalObstacleBoundary(verticalHeader->sections()->boundaryAt(draggingBoundaries.y()));
        gridView->update();
    }

    void GridViewPrivate::onVerticalHeaderDragUpdated()
    {
        setVerticalObstacleBoundary(verticalHeader->sections()->boundaryAt(draggingBoundaries.y()));
        gridView->update();
    }

    void GridViewPrivate::setObstacleBoundaries(double horizontal, double vertical)
    {
        obstaclesUnderCursor.clear();
        foreach (const QRectF & obstacle, obstacles) {
            if ((obstacle.left() < horizontal && horizontal < obstacle.right()) ||
                (obstacle.top() < vertical && vertical < obstacle.bottom())) {
                obstaclesUnderCursor.append(obstacle);
            }
        }
    }

    void GridViewPrivate::setHorizontalObstacleBoundary(double boundary)
    {
        obstaclesUnderCursor.clear();
        foreach (const QRectF & obstacle, obstacles) {
            if (obstacle.left() < boundary && boundary < obstacle.right()) {
                obstaclesUnderCursor.append(obstacle);
            }
        }
    }

    void GridViewPrivate::setVerticalObstacleBoundary(double boundary)
    {
        obstaclesUnderCursor.clear();
        foreach (const QRectF & obstacle, obstacles) {
            if (obstacle.top() < boundary && boundary < obstacle.bottom()) {
                obstaclesUnderCursor.append(obstacle);
            }
        }
    }

    double GridViewPrivate::translateFromWidgetHorizontal(int position) const
    {
        return horizontalHeader->translateFromWidget(position);
    }

    double GridViewPrivate::translateFromWidgetVertical(int position) const
    {
        return verticalHeader->translateFromWidget(position);
    }

    int GridViewPrivate::translateToWidgetHorizontal(double position) const
    {
        return horizontalHeader->translateToWidget(position);
    }

    int GridViewPrivate::translateToWidgetVertical(double position) const
    {
        return verticalHeader->translateToWidget(position);
    }

    QRect GridViewPrivate::visibleRectForCell(int r, int c) const
    {
        Section vSec(verticalHeader->sections()->sectionAt(r));
        Section hSec(horizontalHeader->sections()->sectionAt(c));
        Cell cell(grid->at(r, c));
        return QRect(QPoint(translateToWidgetHorizontal(hSec.offset),
                            translateToWidgetVertical(vSec.offset)),
                     QPoint(translateToWidgetHorizontal(hSec.offset + hSec.size),
                            translateToWidgetVertical(vSec.offset + vSec.size))).adjusted((c == 0 ? 0 : 1), (r == 0 ? 0 : 1), -1, -1);
    }




    GridView::GridView(QWidget * parent)
        : QFrame(parent), d(new GridViewPrivate(this))
    {
        setMouseTracking(true);
        setFrameShape(QFrame::NoFrame);
    }

    GridView::~GridView()
    {}

    QColor GridView::cursorColor() const
    {
        return d->cursorColor;
    }

    Grid * GridView::grid()
    {
        return d->grid;
    }

    QColor GridView::gridColor() const
    {
        return d->gridColor;
    }

    void GridView::contextMenuEvent(QContextMenuEvent * event)
    {
        QMenu menu;
        d->context = true;
        if (d->mouse.move.boundaries.x() >= 0) {
            menu.addAction("Remove column line", d, SLOT(onRemoveColumnLine()));
        } else {
            menu.addAction("Add column line", d, SLOT(onAddColumnLine()));
        }
        if (d->mouse.move.boundaries.y() >= 0) {
            menu.addAction("Remove row line", d, SLOT(onRemoveRowLine()));
        } else {
            menu.addAction("Add row line", d, SLOT(onAddRowLine()));
        }
        if (d->mouse.move.boundaries == QPoint(-1, -1)) {
            menu.addAction("Add both", d, SLOT(onAddRowAndColumnLines()));
        }
        if (d->mouse.move.boundaries.y() >= 0 &&
            d->mouse.move.boundaries.x() >= 0) {
            menu.addAction("Remove both", d, SLOT(onRemoveRowAndColumnLines()));
        }
        menu.exec(event->globalPos());
        d->context = false;

        update();
    }

    void GridView::leaveEvent(QEvent * event)
    {
    }

    void GridView::mouseMoveEvent(QMouseEvent * event)
    {
        d->mousePositionChanged(event->pos());

        if (d->interactionState == GridViewPrivate::Idle) {
            if (d->mouse.move.boundaries != QPoint(-1, -1)) {
                if (d->mouse.move.boundaries.y() == -1) {
                    setCursor(Qt::SizeHorCursor);
                } else if (d->mouse.move.boundaries.x() == -1) {
                    setCursor(Qt::SizeVerCursor);
                } else {
                    setCursor(Qt::SizeFDiagCursor);
                }
                //d->setCursor(false, 0.0);
            } else {
                //setCursor(Qt::CrossCursor);
                setCursor(Qt::ArrowCursor);
                //d->setCursor(true, d->mouse.move.value);
            }

            d->obstaclesUnderCursor.clear();
        } else if (d->interactionState == GridViewPrivate::Dragging) {
            // Make sure we stay within the confines of the neighbouring sections
            if (d->draggingBoundaries.x() > 0) {
                double value = d->mouse.move.values.x() - d->draggingOffsets.x();
                double before = d->horizontalHeader->sections()->boundaryAt(d->draggingBoundaries.x() - 1);
                double after = d->horizontalHeader->sections()->boundaryAt(d->draggingBoundaries.x() + 1);
                if (d->horizontalHeader->reversed()) {
                    qSwap(before, after);
                }
                // Apply an aura
                before = d->translateFromWidgetHorizontal(d->translateToWidgetHorizontal(before) + aura);
                after = d->translateFromWidgetHorizontal(d->translateToWidgetHorizontal(after) - aura);
                if (d->horizontalHeader->reversed()) {
                    qSwap(before, after);
                }
                value = qBound(before, value, after);
                d->horizontalHeader->sections()->moveBoundary(d->draggingBoundaries.x(), value);
            //    d->mouse.move.values.setX(value + d->draggingOffsets.x());
            }
            if (d->draggingBoundaries.y() > 0) {
                double value = d->mouse.move.values.y() - d->draggingOffsets.y();
                double before = d->verticalHeader->sections()->boundaryAt(d->draggingBoundaries.y() - 1);
                double after = d->verticalHeader->sections()->boundaryAt(d->draggingBoundaries.y() + 1);
                if (d->verticalHeader->reversed()) {
                    qSwap(before, after);
                }
                // Apply an aura
                before = d->translateFromWidgetVertical(d->translateToWidgetVertical(before) + aura);
                after = d->translateFromWidgetVertical(d->translateToWidgetVertical(after) - aura);
                if (d->verticalHeader->reversed()) {
                    qSwap(before, after);
                }
                value = qBound(before, value, after);
                d->verticalHeader->sections()->moveBoundary(d->draggingBoundaries.y(), value);
            //    d->mouse.move.values.setY(value + d->draggingOffsets.y());
            }

            if (d->draggingBoundaries.x() > 0 && d->draggingBoundaries.y() > 0) {
                d->setObstacleBoundaries(d->horizontalHeader->sections()->boundaryAt(d->draggingBoundaries.x()),
                                         d->verticalHeader->sections()->boundaryAt(d->draggingBoundaries.y()));
            } else if (d->draggingBoundaries.x() > 0) {
                d->setHorizontalObstacleBoundary(d->horizontalHeader->sections()->boundaryAt(d->draggingBoundaries.x()));
            } else if (d->draggingBoundaries.y() > 0) {
                d->setVerticalObstacleBoundary(d->verticalHeader->sections()->boundaryAt(d->draggingBoundaries.y()));
            }
        }

        update();
    }

    void GridView::mousePressEvent(QMouseEvent * event)
    {
        d->mousePositionChanged(event->pos());
        d->mouse.press.boundaries = d->mouse.move.boundaries;
        d->mouse.press.pos = d->mouse.move.pos;
        d->mouse.press.values = d->mouse.move.values;
        d->mouse.press.cellCoords = d->mouse.move.cellCoords;
        d->draggingOffsets = QPointF(0, 0);

        if (event->button() == Qt::LeftButton) {
            // If there's no boundary under the mouse, split the section
            // FIXME

            // If there's a boundary under the mouse, and the modifier is pressed,
            // then split here
            if (event->modifiers() & Qt::ShiftModifier) {
                // FIXME
            }

            // If there's a boundary under the mouse, then start dragging it
            if (d->mouse.move.boundaries != QPoint(-1, -1)) {
                d->interactionState = GridViewPrivate::Dragging;
                d->draggingBoundaries = d->mouse.move.boundaries;
                if (d->mouse.move.boundaries.x() >= 0) { // horizontal dragging
                    double boundary = d->horizontalHeader->sections()->boundaryAt(d->mouse.move.boundaries.x());
                    d->draggingOffsets.setX(d->mouse.move.values.x() - boundary);
                }
                if (d->mouse.move.boundaries.y() >= 0) { // vertical dragging
                    double boundary = d->verticalHeader->sections()->boundaryAt(d->mouse.move.boundaries.y());
                    d->draggingOffsets.setY(d->mouse.move.values.y() - boundary);
                }

                if (d->mouse.move.boundaries.y() == -1) { // horizontal dragging
                    setCursor(Qt::SizeHorCursor);
                } else if (d->mouse.move.boundaries.x() == -1) { // vertical dragging
                    setCursor(Qt::SizeVerCursor);
                } else {
                    setCursor(Qt::SizeFDiagCursor);
                }
            }

            update();
        }
    }

    void GridView::mouseReleaseEvent(QMouseEvent * event)
    {
        d->mousePositionChanged(event->pos());

        if (d->interactionState == GridViewPrivate::Dragging) {
            d->interactionState = GridViewPrivate::Idle;
        }

        update();
    }

    const QVector< QRectF > & GridView::obstacles() const
    {
        return d->obstacles;
    }

    void GridView::paintEvent(QPaintEvent * event)
    {
        QPainter painter(this);

        // Mask out visible image
        QRect maskRect(QPoint(d->translateToWidgetHorizontal(0),
                              d->translateToWidgetVertical(0)),
                       QPoint(d->translateToWidgetHorizontal(1),
                              d->translateToWidgetVertical(1)));

        // Draw obstacles
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 0, 0, 30));
        foreach (QRectF obstacle, d->obstaclesUnderCursor) {
            QRect visibleRect(QPoint(d->translateToWidgetHorizontal(obstacle.left()),
                                     d->translateToWidgetVertical(obstacle.top())),
                              QPoint(d->translateToWidgetHorizontal(obstacle.right()),
                                     d->translateToWidgetVertical(obstacle.bottom())));
            painter.drawRect(visibleRect & maskRect);
        }

        // Clip cells
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 0, 255, 50));
        QPainterPath clip;
        for (int r = 0; r < d->grid->rowCount(); ++r) {
            for (int c = 0; c < d->grid->columnCount(); ++c) {
                QRect rect = d->visibleRectForCell(r, c);
                //painter.drawRect(rect);
                clip.addRect(rect);
            }
        }
        //painter.setClipPath(clip);

        // Draw grid
        painter.setPen(QPen(d->gridColor, 0.0));
        painter.setBrush(Qt::NoBrush);
        QList< double > boundaries = d->horizontalHeader->sections()->boundaries();
        boundaries.pop_front();
        boundaries.pop_back();
        foreach (double offset, boundaries) {
            int x = d->translateToWidgetHorizontal(offset);
            painter.drawLine(x, 0, x, height());
        }
        boundaries = d->verticalHeader->sections()->boundaries();
        boundaries.pop_front();
        boundaries.pop_back();
        foreach (double offset, boundaries) {
            int y = d->translateToWidgetVertical(offset);
            painter.drawLine(0, y, width(), y);
        }

        // Draw cursor
        painter.setPen(QPen(d->cursorColor, 0.0));
        if (d->horizontalHeader->cursorVisible()) {
            int x = d->translateToWidgetHorizontal(d->horizontalHeader->cursorValue());
            painter.drawLine(x, 0, x, height());
        }
        if (d->verticalHeader->cursorVisible()) {
            int y = d->translateToWidgetVertical(d->verticalHeader->cursorValue());
            painter.drawLine(0, y, width(), y);
        }
    }

    void GridView::setCursorColor(const QColor & color)
    {
        d->cursorColor = color;
        update();
    }

    void GridView::setGridColor(const QColor & color)
    {
        d->gridColor = color;
        update();
    }

    void GridView::setHorizontalHeader(Header * sections)
    {
        if (d->horizontalHeader) {
            d->horizontalHeader->disconnect(this);
        }
        d->horizontalHeader = sections;
        if (d->horizontalHeader) {
            connect(d->horizontalHeader->sections(), SIGNAL(boundaryAdded(int)),
                    d->grid, SLOT(insertColumn(int)));
            connect(d->horizontalHeader->sections(), SIGNAL(boundaryRemoved(int)),
                    d->grid, SLOT(removeColumn(int)));
            connect(d->horizontalHeader->sections(), SIGNAL(boundariesChanged()),
                    this, SLOT(update()));

            connect(d->horizontalHeader, SIGNAL(cursorChanged(bool, double)),
                    d, SLOT(onHorizontalHeaderCursorChanged(bool, double)));
            connect(d->horizontalHeader, SIGNAL(dragCompleted()),
                    d, SLOT(onHorizontalHeaderDragCompleted()));
            connect(d->horizontalHeader, SIGNAL(dragInitiated(int)),
                    d, SLOT(onHorizontalHeaderDragInitiated(int)));
            connect(d->horizontalHeader, SIGNAL(dragUpdated()),
                    d, SLOT(onHorizontalHeaderDragUpdated()));

            d->grid->resize(d->grid->rowCount(), d->horizontalHeader->sections()->count());
        }
    }

    void GridView::setObstacles(const QVector< QRectF > & obstacles)
    {
        d->obstacles = obstacles;
        update();
    }

    void GridView::setRotation(int rotation)
    {
        d->rotation = rotation;
        d->transform.reset();
        d->transform.rotate(rotation * 90);
        d->transform = QImage::trueMatrix(d->transform, 1, 1);
    }

    void GridView::setVerticalHeader(Header * sections)
    {
        if (d->verticalHeader) {
            d->verticalHeader->disconnect(this);
        }
        d->verticalHeader = sections;
        if (d->verticalHeader) {
            connect(d->verticalHeader->sections(), SIGNAL(boundaryAdded(int)),
                    d->grid, SLOT(insertRow(int)));
            connect(d->verticalHeader->sections(), SIGNAL(boundaryRemoved(int)),
                    d->grid, SLOT(removeRow(int)));
            connect(d->verticalHeader->sections(), SIGNAL(boundariesChanged()),
                    this, SLOT(update()));

            connect(d->verticalHeader, SIGNAL(cursorChanged(bool, double)),
                    d, SLOT(onVerticalHeaderCursorChanged(bool, double)));
            connect(d->verticalHeader, SIGNAL(dragCompleted()),
                    d, SLOT(onVerticalHeaderDragCompleted()));
            connect(d->verticalHeader, SIGNAL(dragInitiated(int)),
                    d, SLOT(onVerticalHeaderDragInitiated(int)));
            connect(d->verticalHeader, SIGNAL(dragUpdated()),
                    d, SLOT(onVerticalHeaderDragUpdated()));

            d->grid->resize(d->verticalHeader->sections()->count(), d->grid->columnCount());
        }
    }

    void GridView::setViewportRect(const QRectF & rect)
    {
        d->viewportRect = rect;
        update();
    }

} // namespace Graffiti
