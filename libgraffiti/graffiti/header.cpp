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

#include <graffiti/header.h>
#include <graffiti/header_p.h>

#include <QApplication>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>

#include <QDebug>
#include <iostream>

namespace Graffiti
{

    namespace {
        static const int aura = 3;
    }


    HeaderPrivate::HeaderPrivate(Qt::Orientation orientation, Sections * sections, Header * header)
        : QObject(header),
          header(header),
          sections(0),
          orientation(orientation),
          draggingBoundary(-1),
          draggingOffset(0.0),
          context(false),
          interactionState(Idle),
          reversed(false),
          guidePen(QColor(0, 0, 0, 150))
    {
        mouse.move.value = -1;
        mouse.move.boundary = -1;
        mouse.press.value = -1;
        mouse.press.boundary = -1;
        snap.enabled = false;
        snap.nearest = 0.0;
        snap.available = false;
        snapped.move.value = -1;
        snapped.press.value = -1;
        cursor.isVisible = false;
        cursor.value = 0.0;

        QVector<qreal> dashes;
        dashes << 1 << 3;
        guidePen.setDashPattern(dashes);

        setSections(sections ? sections : new Sections(0.0, 1.0, this));

        visible.from = this->sections->firstBoundary();
        visible.to = this->sections->lastBoundary();
    }

    HeaderPrivate::~HeaderPrivate()
    {}

    int HeaderPrivate::boundaryForValue(int mousePos, double value)
    {
        int idx = sections->find(value);
        int count = sections->count();
        if (idx >= 0 && idx < count) {
            Section section = sections->sectionAt(idx);
            int boundaryLeft = translateToWidget(section.offset);
            int boundaryRight = translateToWidget(section.offset + section.size);
            if (abs(mousePos - boundaryLeft) <= aura && idx > 0) {
                return idx;
            } else if (abs(mousePos - boundaryRight) <= aura && idx < (count - 1)) {
                return idx + 1;
            }
        }
        return -1;
    }

    int HeaderPrivate::breadth() const
    {
        return orientation == Qt::Horizontal ? header->height() : header->width();
    }

    bool HeaderPrivate::isSnappable() const
    {
        return (snap.enabled && !snap.values.empty());
    }

    int HeaderPrivate::length() const
    {
        return orientation == Qt::Horizontal ? header->width() : header->height();
    }

    void HeaderPrivate::mouseMoved()
    {
        // If the mouse is outside the widget, don't do anything
        if (header->isVisible() && header->rect().contains(header->mapFromGlobal(QCursor::pos()))) {
            // Otherwise...
            if (interactionState == HeaderPrivate::Idle) {
                if (mouse.move.boundary >= 0) {
                    header->setCursor(orientation == Qt::Horizontal ? Qt::SizeHorCursor : Qt::SizeVerCursor);
                    header->setCursorVisible(false);
                } else {
                    if (snap.enabled && !snap.available) {
                        header->setCursor(Qt::ArrowCursor);
                        header->setCursorVisible(false);
                    } else {
                        header->setCursor(orientation == Qt::Horizontal ? Qt::SplitHCursor : Qt::SplitVCursor);
                        header->setCursorValue(snap.enabled ? snapped.move.value : mouse.move.value);
                        header->setCursorVisible(true);
                    }
                }
            } else if (interactionState == HeaderPrivate::Dragging) {
                double value = snap.enabled ? snap.nearest : (mouse.move.value - draggingOffset);
                // Make sure we stay within the confines of the neighbouring sections
                double before = sections->boundaryAt(draggingBoundary - 1);
                double after = sections->boundaryAt(draggingBoundary + 1);
                if (reversed) {
                    qSwap(before, after);
                }
                // Apply an aura
                before = translateFromWidget(translateToWidget(before) + aura);
                after = translateFromWidget(translateToWidget(after) - aura);
                if (reversed) {
                    qSwap(before, after);
                }
                value = qBound(before, value, after);
                sections->moveBoundary(draggingBoundary, value);
                mouse.move.value = value + (snap.enabled ? 0 : draggingOffset);
            }
        }

        header->update();
    }

    void HeaderPrivate::mousePositionChanged(const QPoint & point)
    {
        // Helpful defines
        typedef std::set< double > SnapSet;

        // If a mouse position is provided, remember for later use
        if (!point.isNull()) {
            mouse.move.raw = point;
        }
        // Normalise the position (convert it to horizontal coordinates)
        mouse.move.pos = normalisedPos(mouse.move.raw);
        // Find the value of that position along this header
        mouse.move.value = translateFromWidget(mouse.move.pos.x());
        mouse.move.value = qBound(sections->firstBoundary(), mouse.move.value, sections->lastBoundary());
        // Set some intial variables
        mouse.move.boundary = -1;
        snap.available = false;
        snapped.move.value = mouse.move.value;

        // Find the closest guides to the mouse position
        SnapSet::const_iterator next_guide_iter = snap.values.upper_bound(mouse.move.value);
        double next_guide = (next_guide_iter == snap.values.end()) ? sections->lastBoundary() : *next_guide_iter;
        double previous_guide = (next_guide_iter == snap.values.begin()) ? sections->firstBoundary() : *--next_guide_iter;

        // Find the closest boundaries to the mouse position
        int next_boundary_idx = sections->next(mouse.move.value);
        int previous_boundary_idx = sections->previous(mouse.move.value);
        // Ignore the boundary currently being dragged (if there is one)
        if (interactionState == HeaderPrivate::Dragging) {
            if (next_boundary_idx == draggingBoundary) {
                ++next_boundary_idx;
            }
            if (previous_boundary_idx == draggingBoundary) {
                --previous_boundary_idx;
            }
        }

        double next_boundary = next_boundary_idx >= 0 ? sections->boundaryAt(next_boundary_idx) : sections->lastBoundary();
        double previous_boundary = previous_boundary_idx >= 0 ? sections->boundaryAt(previous_boundary_idx) : sections->firstBoundary();

        // Distance from guides to the mouse position
        double before = mouse.move.value - qMax(previous_guide, previous_boundary);
        double after = qMin(next_guide, next_boundary) - mouse.move.value;
        snap.nearest = (before > after) ? qMin(next_guide, next_boundary) : qMax(previous_guide, previous_boundary);

        // Are the guides available?
        bool next_guide_available = (next_guide < sections->lastBoundary() && next_boundary > next_guide);
        bool previous_guide_available = (previous_guide > sections->firstBoundary() && previous_boundary < previous_guide);

        // Snap the mouse position to an appropriate guide
        if (next_guide_available || previous_guide_available) {
            snap.available = true;
            if (!next_guide_available) {
                snapped.move.value = previous_guide;
            } else if (!previous_guide_available) {
                snapped.move.value = next_guide;
            } else {
                snapped.move.value = (before > after) ? next_guide : previous_guide;
            }
        }

        mouse.move.boundary = boundaryForValue(mouse.move.pos.x(), mouse.move.value);
    }

    QPoint HeaderPrivate::normalisedPos(const QPoint & point) const
    {
        if (orientation == Qt::Horizontal) {
            return point;
        } else {
            return QPoint(point.y(), point.x());
        }
    }

    void HeaderPrivate::onBoundariesChanged()
    {
        // Make sure the header lands within the confines
        //header->setVisibleRange(qMax(visible.from, sections->firstBoundary()),
        //                        qMin(visible.to, sections->lastBoundary()));
        header->update();
    }

    void HeaderPrivate::onBoundaryAdded(int i)
    {
        header->update();
    }

    void HeaderPrivate::onBoundaryRemoved(int i)
    {
        header->update();
    }

    void HeaderPrivate::onSplit()
    {
        sections->addBoundary((sender() || !snap.enabled) ? mouse.press.value : snapped.press.value);
        header->update();
    }

    void HeaderPrivate::onMerge()
    {
        if (mouse.press.boundary > 0) {
            sections->removeBoundary(mouse.press.boundary);
        }
        header->update();
    }

    void HeaderPrivate::setCursor(bool visible, double value)
    {
        bool signal = false;
        if (cursor.isVisible != visible || cursor.value != value) {
            signal = true;
        }
        cursor.isVisible = visible;
        cursor.value = value;
        if (signal) {
            emit cursorChanged(visible, value);
        }
        header->update();
    }

    void HeaderPrivate::setSections(Sections * sections)
    {
        if (this->sections) {
            this->sections->disconnect(this);
        }

        this->sections = sections;

        if (sections) {
            connect(sections, SIGNAL(boundariesChanged()), this, SLOT(onBoundariesChanged()));
            connect(sections, SIGNAL(boundaryAdded(int)), this, SLOT(onBoundaryAdded(int)));
            connect(sections, SIGNAL(boundaryRemoved(int)), this, SLOT(onBoundaryRemoved(int)));
        }
    }

    int HeaderPrivate::translateToWidget(double position) const
    {
        double size = visibleSize();
        if (size != 0.0) {
            int pos = length() * (position - visible.from) / size;
            if (reversed) {
                pos = length() - 1 - pos;
            }
            return pos;
        } else {
            return 0;
        }
    }

    double HeaderPrivate::translateFromWidget(int position) const
    {
        if (length() != 0) {
            if (reversed) {
                position = length() - 1 - position;
            }
            return (position * visibleSize() / (double) length()) + visible.from;
        } else {
            return 0.0;
        }
    }

    double HeaderPrivate::visibleSize() const
    {
        return visible.to - visible.from;
    }




    Header::Header(Qt::Orientation orientation, QWidget * parent)
        : QFrame(parent), d(new HeaderPrivate(orientation, 0, this))
    {
        connect(d, SIGNAL(cursorChanged(bool, double)),
                this, SIGNAL(cursorChanged(bool, double)));

        setMouseTracking(true);
        setFrameShape(QFrame::NoFrame);
    }

    Header::Header(Sections * sections, Qt::Orientation orientation, QWidget * parent)
        : QFrame(parent), d(new HeaderPrivate(orientation, sections, this))
    {
        connect(d, SIGNAL(cursorChanged(bool, double)),
                this, SIGNAL(cursorChanged(bool, double)));

        setMouseTracking(true);
        setFrameShape(QFrame::NoFrame);
    }

    Header::~Header()
    {}

    void Header::contextMenuEvent(QContextMenuEvent * event)
    {
        QMenu menu;
        d->context = true;
        if (d->mouse.move.boundary >= 0) {
            menu.addAction("Remove " + QString(d->orientation == Qt::Horizontal ? "column" : "row") + " line", d, SLOT(onMerge()));
        } else {
            menu.addAction("Add " + QString(d->orientation == Qt::Horizontal ? "column" : "row") + " line", d, SLOT(onSplit()));
        }
        menu.exec(event->globalPos());
        d->context = false;

        update();
    }

    QColor Header::cursorColor() const
    {
        return d->cursorColor;
    }

    double Header::cursorValue() const
    {
        return d->cursor.value;
    }

    bool Header::cursorVisible() const
    {
        return d->cursor.isVisible;
    }

    QColor Header::gridColor() const
    {
        return d->gridColor;
    }

    void Header::enterEvent(QEvent * /*event*/)
    {
        d->mousePositionChanged(mapFromGlobal(QCursor::pos()));
        d->mouseMoved();
    }

    void Header::leaveEvent(QEvent * /*event*/)
    {
        if (!d->context) {
            setCursorVisible(false);
        }

        update();
    }

    void Header::mouseMoveEvent(QMouseEvent * event)
    {
        d->mousePositionChanged(event->pos());
        d->mouseMoved();

        if (d->interactionState == HeaderPrivate::Dragging) {
            emit dragUpdated();
        }
    }

    void Header::mousePressEvent(QMouseEvent * event)
    {
        d->mousePositionChanged(event->pos());
        d->mouse.press.boundary = d->mouse.move.boundary;
        d->mouse.press.pos = d->mouse.move.pos;
        d->mouse.press.value = d->mouse.move.value;
        d->snapped.press.value = d->snapped.move.value;

        if (event->button() == Qt::LeftButton) {
            // If there's no boundary under the mouse, split the section
            if (cursorVisible()) {
                d->onSplit();
                setCursorVisible(false);
                d->mousePositionChanged(event->pos());
                d->draggingOffset = 0.0;
            }

            // If there's a boundary under the mouse, then start dragging it
            if (d->mouse.move.boundary >= 0) {
                d->interactionState = HeaderPrivate::Dragging;
                d->draggingBoundary = d->mouse.move.boundary;
                double boundary = d->sections->boundaryAt(d->mouse.move.boundary);
                d->draggingOffset = d->mouse.move.value - boundary;
                setCursor(d->orientation == Qt::Horizontal ? Qt::SizeHorCursor : Qt::SizeVerCursor);
                emit dragInitiated(d->draggingBoundary);
            }

            update();
        }
    }

    void Header::mouseReleaseEvent(QMouseEvent * event)
    {
        d->mousePositionChanged(event->pos());

        if (d->interactionState == HeaderPrivate::Dragging) {
            d->interactionState = HeaderPrivate::Idle;
            emit dragCompleted();
        }

        d->mouseMoved();
        update();
    }

    QString Header::orientation() const
    {
        return d->orientation == Qt::Horizontal ? "horizontal" : "vertical";
    }

    void Header::setOrientation(Qt::Orientation orientation)
    {
        d->orientation = orientation;
        update();
    }

    void Header::paintEvent(QPaintEvent * /*event*/)
    {
        QPainter painter(this);

        // Paint guides if needed
        if (d->snap.enabled) {
            painter.setPen(d->guidePen);
            foreach (double value, d->snap.values) {
                int position = d->translateToWidget(value);
                painter.drawLine(d->normalisedPos(QPoint(position, 0)),
                                 d->normalisedPos(QPoint(position, d->breadth())));
            }
        }

        int idx = 0;
        int count = d->sections->count();
        painter.setPen(QPen(d->gridColor, 0.0));
        foreach (double boundary, d->sections->boundaries()) {
            if (idx > 0 && idx < count) {
                int position = d->translateToWidget(boundary);
                painter.drawLine(d->normalisedPos(QPoint(position, 0)),
                                 d->normalisedPos(QPoint(position, d->breadth())));
            }
            ++idx;
        }

        if (cursorVisible()) {
            painter.setPen(QPen(d->cursorColor, 0.0));
            int position = d->translateToWidget(cursorValue());
            painter.drawLine(d->normalisedPos(QPoint(position, 0)),
                             d->normalisedPos(QPoint(position, d->breadth())));
        }
    }

    bool Header::reversed() const
    {
        return d->reversed;
    }

    Sections * Header::sections()
    {
        return d->sections;
    }

    void Header::setCursorColor(const QColor & color)
    {
        d->cursorColor = color;
        update();
    }

    void Header::setCursorValue(double value)
    {
        d->setCursor(cursorVisible(), value);
    }

    void Header::setCursorVisible(bool visible)
    {
        d->setCursor(visible, cursorValue());
    }

    void Header::setGridColor(const QColor & color)
    {
        d->gridColor = color;
        update();
    }

    void Header::setReversed(bool reversed)
    {
        if (d->reversed != reversed) {
            d->reversed = reversed;
            emit reversal();
        }
    }

    void Header::setSections(Sections * sections)
    {
        d->setSections(sections);
        d->mouseMoved();
    }

    void Header::setSnapToGuides(bool snap)
    {
        d->snap.enabled = snap;
        d->mouseMoved();
    }

    void Header::setSnapValues(const QVector< double > & values)
    {
        d->snap.values.clear();
        foreach (double value, values) {
            d->snap.values.insert(value);
        }
        d->mouseMoved();
    }

    void Header::setVisibleRange(double from, double to)
    {
        d->visible.from = qMin(from, to);
        d->visible.to = qMax(from, to);
        // Deal with mouse stuff? FIXME
        update();
    }

    int Header::translateToWidget(double position) const
    {
        return d->translateToWidget(position);
    }

    double Header::translateFromWidget(int position) const
    {
        return d->translateFromWidget(position);
    }

    QVector< double > Header::snapValues() const
    {
        QVector< double > values;
        foreach (double value, d->snap.values) {
            values.push_back(value);
        }
        return values;
    }

    double Header::visibleFrom() const
    {
        return d->visible.from;
    }

    double Header::visibleTo() const
    {
        return d->visible.to;
    }

} // namespace Graffiti
