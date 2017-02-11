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
#include <cinema6/aspect.h>
#include <cinema6/component.h>
#include <cinema6/datacomponent.h>
#include <cinema6/sequence.h>
#include <cinema6/renderevent.h>

#include <cinema6/controlaspect.h>
#include <cinema6/keycomponent.h>
#include <cinema6/sequencecomponent.h>
#include <cinema6/titleaspect.h>

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QChildEvent>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QList>
#include <QMap>
#include <QMimeData>
#include <QPainter>
#include <QResizeEvent>
#include <QScrollBar>
#include <QSlider>

#include <math.h>

namespace CINEMA6
{

    namespace {

        // Drag source
        typedef enum
        {
            NoDrag,
            StartDrag,
            DragComponent,
            DragAspect
        } DragState;

        // Interaction state
        typedef enum
        {
            Idle = 0,
            Gapping,
            Annotating,
            Sliding,
            Selecting,
            Customizing
        } InteractionState;

    }

    class AlignmentViewPrivate
    {
    public:
        AlignmentViewPrivate(AlignmentView * alignmentView)
            : av(alignmentView),
              unitSize(20),
              discreteScrolling(false),
              zoomSlider(0),
              zoom(20),
              interactionModes(0),
              interactionCursor(Qt::ArrowCursor),
              aspectPartition(0),
              aspectUnderMouse(0),
              componentUnderMouse(0),
              dragState(NoDrag),
              interaction(Idle),
              interactionComponent(0),
              aspectMargin(0),
              aspectSpacing(1)
            {
                componentPartitions[0] = 0;
                componentPartitions[1] = 0;
            }

        AlignmentView * av;

        // Unit size information
        int unitSize;
        bool discreteScrolling;
        QSlider * zoomSlider;
        double zoom;

        // Interaction
        QActionGroup * interactionModes;
        QCursor interactionCursor;

        // Cached document geometry
        QRect horizontalScrollArea;
        QRect verticalScrollArea;

        // Actual subsidiary components / aspects
        QList< Aspect * > aspectsAll;
        int aspectPartition;
        QList< Component * > componentsAll;
        int componentPartitions[2];

        // Quick geometric access to components / aspects
        QMap< int, Aspect * > aspectPositions;
        QMap< int, Component * > dynamicComponentPositions;
        QMap< int, Component * > staticComponentPositions;

        // Under mouse
        QPoint mousePos;
        Aspect * aspectUnderMouse;
        Component * componentUnderMouse;

        // Dragging components
        DragState dragState;
        QPoint dragStartPosition;
        AlignmentView::AspectPosition dragAspectPosition;
        int dragAspectIndex;
        AlignmentView::AspectPosition dropAspectPosition;
        int dropAspectIndex;
        AlignmentView::ComponentPosition dragComponentPosition;
        int dragComponentIndex;
        AlignmentView::ComponentPosition dropComponentPosition;
        int dropComponentIndex;

        // Component interaction
        InteractionState interaction;
        QPoint mousePress;
        int sequenceIndexPress;
        int alignmentIndexPress;
        int offsetPress;
        int gapPress;
        int sequenceIndexMove;
        int alignmentIndexMove;
        QPoint mouseRelease;
        int sequenceIndexRelease;
        int alignmentIndexRelease;
        DataComponent * interactionComponent;

        // Styling
        int aspectMargin;
        int aspectSpacing;

        // Selection
        Selection selection;


        void updateMaxVerticalOffset()
            {
                int topHeight = av->componentRangeHeight(0, av->componentCount(AlignmentView::Top) - 1, AlignmentView::Top);
                int centerHeight = av->componentRangeHeight(0, av->componentCount(AlignmentView::Center) - 1, AlignmentView::Center);
                int bottomHeight = av->componentRangeHeight(0, av->componentCount(AlignmentView::Bottom) - 1, AlignmentView::Bottom);
                int newMaxVerticalOffset = topHeight + centerHeight + bottomHeight - av->viewport()->height();

                // Constrain
                if (newMaxVerticalOffset < 0) { newMaxVerticalOffset = 0; }

                av->verticalScrollBar()->setMaximum(newMaxVerticalOffset);
            }

        void updateMousePos(const QPoint & pos)
            {
                // Aspect under mouse...
                Aspect * aspect = av->aspectUnder(pos.x());
                // Component under mouse...
                Component * component = av->componentUnder(pos.y());

                QEvent event(QEvent::Leave);
                // If we were in the document...
                if (aspectUnderMouse == 0)
                {
                    // If we are still in the document, but in a different component
                    // or have left the document entirely
                    if (componentUnderMouse != 0 && (
                            (aspect == 0 && componentUnderMouse != component)
                            || aspect != 0)
                        )
                    {
                        QApplication::sendEvent(componentUnderMouse, &event);
                    }
                }
                // Else if we have moved from one aspect to another
                else if (aspectUnderMouse != aspect)
                {
                    QApplication::sendEvent(aspectUnderMouse, &event);
                }

                // Cache information
                mousePos = pos;
                aspectUnderMouse = aspect;
                componentUnderMouse = component;
            }
    };


    /**
     *  \brief Default widget construction.
     */
    AlignmentView::AlignmentView(QWidget * parent)
        : QAbstractScrollArea(parent), d(new AlignmentViewPrivate(this))
    {
        // Defaults

        horizontalScrollBar()->setRange(0, 6000);
        verticalScrollBar()->setRange(0, 100);

        // Widget defaults
        setAcceptDrops(true);
        setMouseTracking(true);
        setFrameShape(QFrame::NoFrame);
//        viewport()->setAttribute(Qt::WA_StaticContents, true);
//        setAttribute(Qt::WA_StaticContents, true);

        d->zoomSlider = new QSlider(Qt::Vertical);
        d->zoomSlider->setMaximumHeight(50);
        d->zoomSlider->setMinimum(2);
        d->zoomSlider->setMaximum(40);
        d->zoomSlider->setValue(20);
        d->zoomSlider->setSingleStep(1);
        d->zoomSlider->setPageStep(1);
        d->zoomSlider->setInvertedAppearance(true);
        d->zoomSlider->setInvertedControls(true);
        d->zoomSlider->setFixedWidth(160);
        QObject::connect(d->zoomSlider, SIGNAL(valueChanged(int)),
                         this, SLOT(setUnitSize(int)));
        QObject::connect(this, SIGNAL(unitSizeChanged(int)),
                         d->zoomSlider, SLOT(setValue(int)));
        d->zoomSlider->hide();
//        addScrollBarWidget(d->zoomSlider, Qt::AlignRight);

        // Document geometry
        setHorizontalScrollArea(viewport()->rect());
        setVerticalScrollArea(viewport()->rect());

        // Actions
        {
            d->interactionModes = new QActionGroup(this);

            QAction * action = new QAction("Select Mode", this);
            action->setCheckable(true);
            d->interactionModes->addAction(action);
            action->setChecked(true);
            addAction(action);
            QObject::connect(action, SIGNAL(triggered()), this, SLOT(activateSelectMode()));

            action = new QAction("Slide Mode", this);
            action->setCheckable(true);
            d->interactionModes->addAction(action);
            addAction(action);
            QObject::connect(action, SIGNAL(triggered()), this, SLOT(activateSlideMode()));

            action = new QAction("Gap Mode", this);
            action->setCheckable(true);
            d->interactionModes->addAction(action);
            action->setCheckable(true);
            addAction(action);
            QObject::connect(action, SIGNAL(triggered()), this, SLOT(activateGapMode()));

            action = new QAction("Annotate Mode", this);
            action->setCheckable(true);
            d->interactionModes->addAction(action);
            addAction(action);
            QObject::connect(action, SIGNAL(triggered()), this, SLOT(activateAnnotateMode()));

            action = new QAction("Zoom In", this);
        	d->interactionModes->addAction(action);
        	addAction(action);
        	QObject::connect(action, SIGNAL(triggered()), this, SLOT(zoomIn()));

        	action = new QAction("Zoom Out", this);
        	d->interactionModes->addAction(action);
        	addAction(action);
        	QObject::connect(action, SIGNAL(triggered()), this, SLOT(zoomOut()));
        }
    }

    /**
     *  \brief Destructor.
     */
    AlignmentView::~AlignmentView()
    {}

    void AlignmentView::activateSelectMode()
    {
        d->interactionCursor = QCursor(Qt::ArrowCursor);
        setCursor(d->interactionCursor);
    }

    void AlignmentView::activateSlideMode()
    {
        d->interactionCursor = QCursor(Qt::OpenHandCursor);
        setCursor(d->interactionCursor);
    }

    void AlignmentView::activateGapMode()
    {
        d->interactionCursor = QCursor(Qt::OpenHandCursor);
        setCursor(d->interactionCursor);
    }

    void AlignmentView::activateAnnotateMode()
    {
        d->interactionCursor = QCursor(Qt::ArrowCursor);
        setCursor(d->interactionCursor);
    }

    QPair< int, AlignmentView::AspectPosition > AlignmentView::actualToLogicalAspect(int index) const
    {
        AspectPosition position = Left;
        if (d->aspectPartition <= index)
        {
            index -= d->aspectPartition;
            position = Right;
        }
        return qMakePair(index, position);
    }

    QPair< int, AlignmentView::ComponentPosition > AlignmentView::actualToLogicalComponent(int index) const
    {
        int i = 0;
        for (i = 0; i < 2; ++i)
        {
            int partition = d->componentPartitions[i];
            if (index >= partition)
            {
                index -= partition;
            }
            else
            {
                break;
            }
        }
        return qMakePair(index, (ComponentPosition) i);
    }

    /**
     *  \brief Append an aspect to this view.
     */
    void AlignmentView::appendAspect(AspectPosition position, Aspect * aspect)
    {
        if (aspect->alignmentView() == this) { return; }

        // Reparent
        aspect->setParent(this);

        // Add aspect
        int insertionIndex = logicalToActualAspect(aspectCount(position), position);
        d->aspectsAll.insert(insertionIndex, aspect);
        if (position == Left) { ++d->aspectPartition; }

        // Update
        widthsChanged();
    }

    /**
     *  \brief Append a component to this view.
     */
    void AlignmentView::appendComponent(ComponentPosition position, Component * component)
    {
        if (component->alignmentView() == this) { return; }

        // Reparent
        component->setParent(this);

        // Add component
        int insertionIndex = logicalToActualComponent(componentCount(position), position);
        d->componentsAll.insert(insertionIndex, component);
        if (position != Bottom) { ++d->componentPartitions[(int) position]; }

        // Update
        heightsChanged();
    }

    /**
     *  \brief Retrieve an aspect from this view.
     */
    Aspect * AlignmentView::aspectAt(int index, AspectPosition position) const
    {
        return d->aspectsAll.at(logicalToActualAspect(index, position));
    }

    /**
     *  \brief Count aspects in this view.
     */
    int AlignmentView::aspectCount() const
    {
        return d->aspectsAll.count();
    }

    /**
     *  \brief Count aspects in this view.
     */
    int AlignmentView::aspectCount(AspectPosition position) const
    {
        switch (position)
        {
        case Left:
            return d->aspectPartition;
        case Right:
            return aspectCount() - d->aspectPartition;
        }

        return 0;
    }

    /**
     *  \brief Query the aspect margin.
     */
    int AlignmentView::aspectMargin() const
    {
        return d->aspectMargin;
    }

    /**
     *  \brief Query the inter-aspect spacing.
     */
    int AlignmentView::aspectSpacing() const
    {
        return d->aspectSpacing;
    }

    QPair< int, AlignmentView::AspectPosition > AlignmentView::aspectPosition(Aspect * aspect) const
    {
        return actualToLogicalAspect(d->aspectsAll.indexOf(aspect));
    }

    /**
     *  \brief Query the width of a range of aspects.
     */
    int AlignmentView::aspectRangeWidth(int from, int to, AspectPosition position) const
    {
        int width = 0;
        for (int index = from; index <= to; ++index)
        {
            width += aspectAt(index, position)->width();
        }
        return width;
    }

    Aspect * AlignmentView::aspectUnder(int x) const
    {
        QMap< int, Aspect * >::const_iterator found = d->aspectPositions.lowerBound(x);
        QMap< int, Aspect * >::const_iterator end = d->aspectPositions.end();
        if (found == end)
        {
            return *--found;
        }
        else
        {
            return *found;
        }
    }

    /**
     *  \brief Query the width of an aspect.
     */
/*    int AlignmentView::aspectWidthAt(int index, AspectPosition position) const
      {
      int width = 0;
      Aspect * aspect = aspectAt(index, position);
      if (aspect->displayFlags().testFlag(AbstractComponent::Minimised))
      {
      width = 12;
      }
      else
      {
      width = aspect->width();
      }
      return width;
      } */

    void AlignmentView::childEvent(QChildEvent * event)
    {
        if (event->removed())
        {
            // Disconnect all signals from child
            disconnect(event->child());

            Aspect * aspect = d->aspectsAll.contains(static_cast< Aspect * >(event->child())) ? static_cast< Aspect * >(event->child()) : 0;
            Component * component = d->componentsAll.contains(static_cast< Component * >(event->child())) ? static_cast< Component * >(event->child()) : 0;

            if (aspect)
            {
                QPair< int, AspectPosition > logical = aspectPosition(aspect);
                int index = logical.first;
                AspectPosition position = logical.second;
                int removalIndex = logicalToActualAspect(index, position);
                Aspect * aspect = d->aspectsAll.at(removalIndex);
                if (position == Left) { --d->aspectPartition; }
                d->aspectsAll.removeAt(removalIndex);

                // Propogate changes
                widthsChanged();
                if (d->aspectUnderMouse == aspect)
                {
                    // Update
                    d->aspectUnderMouse = 0;
                }
            }
            else if (component)
            {
                QPair< int, ComponentPosition > logical = componentPosition(component);
                int index = logical.first;
                ComponentPosition position = logical.second;
                int removalIndex = logicalToActualComponent(index, position);
                Component * component = d->componentsAll.at(removalIndex);
                if (position != Bottom) { --d->componentPartitions[(int) position]; }
                d->componentsAll.removeAt(removalIndex);

                // Propogate changes
                heightsChanged();
                if (d->componentUnderMouse == component)
                {
                    // Update
                    d->componentUnderMouse = 0;
                }
            }
        }
        else if (event->added())
        {
            AbstractComponent * abstractComponent = dynamic_cast< AbstractComponent * >(event->child());

            // Connect signals
            if (abstractComponent)
            {
                QObject::connect(this, SIGNAL(unitSizeChanged(int)), abstractComponent, SLOT(unitSizeUpdate(int)));
                QObject::connect(this, SIGNAL(zoomChanged(double)), abstractComponent, SLOT(zoomUpdate(double)));
                //QObject::connect(parent, SIGNAL(styleUpdated(?)), this, SLOT(styleUpdate(?)));
                // FIXME and the rest
            }
        }
    }

    /**
     *  \brief Retrieve a component from this view.
     */
    Component * AlignmentView::componentAt(int index, ComponentPosition position) const
    {
        return d->componentsAll.at(logicalToActualComponent(index, position));
    }

    /**
     *  \brief Count components in this view.
     */
    int AlignmentView::componentCount() const
    {
        return d->componentsAll.count();
    }

    /**
     *  \brief Count components in this view.
     */
    int AlignmentView::componentCount(ComponentPosition position) const
    {
        switch (position)
        {
        case Top:
            return d->componentPartitions[0];
        case Center:
            return d->componentPartitions[1];
        case Bottom:
            return componentCount() - d->componentPartitions[1] - d->componentPartitions[0];
        }

        return 0;
    }

    /**
     *  \brief Query the height of a component.
     */
/*    int AlignmentView::componentHeightAt(int index, ComponentPosition position) const
      {
      int height = 0;
      Component * component = componentAt(index, position);
      if (component->displayFlags().testFlag(AbstractComponent::Minimised))
      {
      height = 12;
      }
      else
      {
      height = component->height();
      }
      return height;
      } */

    QPair< int, AlignmentView::ComponentPosition > AlignmentView::componentPosition(Component * component) const
    {
        return actualToLogicalComponent(d->componentsAll.indexOf(component));
    }

    /**
     *  \brief Query the height of a range of components.
     */
    int AlignmentView::componentRangeHeight(int from, int to, ComponentPosition position) const
    {
        int height = 0;
        for (int index = from; index <= to; ++index)
        {
            height += componentAt(index, position)->height();
        }
        return height;
    }

    Component * AlignmentView::componentUnder(int y) const
    {
        QMap< int, Component * >::const_iterator found;
        QMap< int, Component * >::const_iterator end;

        QRect vScrollArea = d->verticalScrollArea;
        if (y >= vScrollArea.top() && y <= vScrollArea.bottom())
        {
            y += verticalScrollBar()->value() - vScrollArea.top();
            found = d->dynamicComponentPositions.lowerBound(y);
            end = d->dynamicComponentPositions.end();
        }
        else
        {
            found = d->staticComponentPositions.lowerBound(y);
            end = d->staticComponentPositions.end();
        }

        if (found == end)
        {
            return 0;
        }
        else
        {
            return *found;
        }
    }

    QRect AlignmentView::documentGeometry() const
    {
        return d->horizontalScrollArea.intersected(d->verticalScrollArea);
    }

    void AlignmentView::dragEnterEvent(QDragEnterEvent * event)
    {
        if (d->dragState != NoDrag)
        {
            event->acceptProposedAction();
        }
    }

    void AlignmentView::dragMoveEvent(QDragMoveEvent * event)
    {
        // So long as we are actually dragging (this should always be the case)!
        if (d->dragState != NoDrag)
        {
            QPoint pos = event->pos();
            if (pos.x() < 0 || pos.y() < 0) { return; }

            switch (d->dragState)
            {
            case DragAspect:
            {
                Aspect * aspect = aspectUnder(pos.x());

                // If we're over the document area...
                if (aspect == 0)
                {
                    if (pos.x() < d->horizontalScrollArea.left() + d->horizontalScrollArea.width() / 2)
                    {
                        d->dropAspectPosition = Left;
                        d->dropAspectIndex = aspectCount(Left);

                        // If the left hand side is where we've come from, then compensate for the
                        // dragged index being skipped
                        if (d->dropAspectPosition == d->dragAspectPosition)
                        {
                            --d->dropAspectIndex;
                        }
                    }
                    else
                    {
                        d->dropAspectPosition = Right;
                        d->dropAspectIndex = 0;
                    }
                }
                // If we're over the right or left aspect areas...
                else
                {
                    int left = aspect->left();
                    int width = aspect->width();
                    QPair< int, AspectPosition > logical = aspectPosition(aspect);
                    // Record position of aspect under mouse
                    d->dropAspectPosition = logical.second;
                    d->dropAspectIndex = logical.first;
                    // If we're moving within an area, and moving to the right, compensate for the
                    // dragged index being skipped
                    if (d->dropAspectPosition == d->dragAspectPosition &&
                        d->dropAspectIndex >= d->dragAspectIndex)
                    {
                        if (d->dropAspectIndex > d->dragAspectIndex &&
                            pos.x() < left + width / 2)
                        {
                            --d->dropAspectIndex;
                        }
                    }
                    else if (pos.x() > left + width / 2)
                    {
                        ++d->dropAspectIndex;
                    }
                }

                break;
            }
            case DragComponent:
            {
                Component * component = componentUnder(pos.y());

                if (component != 0)
                {
                    int top = component->top();
                    int height = component->height();
                    QPair< int, ComponentPosition > logical = componentPosition(component);
                    // Record position of aspect under mouse
                    d->dropComponentPosition = logical.second;
                    d->dropComponentIndex = logical.first;
                    // If we're moving within an area, and moving down, compensate for the
                    // dragged index being skipped
                    if (d->dropComponentPosition == d->dragComponentPosition &&
                        d->dropComponentIndex >= d->dragComponentIndex)
                    {
                        if (d->dropComponentIndex > d->dragComponentIndex &&
                            pos.y() < top + height / 2)
                        {
                            --d->dropComponentIndex;
                        }
                    }
                    else if (pos.y() > top + height / 2)
                    {
                        ++d->dropComponentIndex;
                    }
                }
                else
                {
                    // In space after the center FIXME
                }

                break;
            }
            default:
                break;
            }

            viewport()->update();
        }

        event->acceptProposedAction();
    }

    void AlignmentView::dropEvent(QDropEvent * event)
    {
        switch (d->dragState)
        {
        case DragAspect:
        {
            moveAspect(d->dragAspectIndex, d->dragAspectPosition, d->dropAspectIndex, d->dropAspectPosition);
            event->acceptProposedAction();
            break;
        }
        case DragComponent:
        {
            moveComponent(d->dragComponentIndex, d->dragComponentPosition, d->dropComponentIndex, d->dropComponentPosition);
            event->acceptProposedAction();
            break;
        }
        default:
            break;
        }
        d->dragState = NoDrag;
        viewport()->update();
    }

/*      bool AlignmentView::event(QEvent * event)
        {
        if (event->type() == QEvent::Paint)
        {
        qDebug() << "AlignmentView::event" << static_cast< QPaintEvent * >(event)->rect();
        }
        return QAbstractScrollArea::event(event);
        } */

    /**
     *  \brief Recalculate component heights.
     */
    void AlignmentView::heightsChanged()
    {
        // Recompute maximum vertical offset
        d->updateMaxVerticalOffset();

        // Set widget Maximums
        int bottomHeight = componentRangeHeight(0, componentCount(Bottom) - 1, Bottom);
        int topHeight = componentRangeHeight(0, componentCount(Top) - 1, Top);
        setMinimumHeight(bottomHeight + topHeight + horizontalScrollBar()->height());

        // Reset position cache
        d->dynamicComponentPositions.clear();
        d->staticComponentPositions.clear();

        // Component sizing
        int top = 0;
        int relativeTop = 0;
        int maxHeight = 0;
        int minHeight = 0;

        // Collect each top aligned component
        for (int index = 0; index < componentCount(Top); ++index)
        {
            Component * component = componentAt(index, Top);
            int height = component->height();
            component->setTop(top);
            d->staticComponentPositions[top + height - 1] = component;

            // Accumulate height
            top += height;
            maxHeight += height;
            minHeight += height;
        }

        // Set document geometry
        QRect vScrollArea = d->verticalScrollArea;
        vScrollArea.setTop(top);

        // Decide where the bottom set of components will be drawn
        int centerHeight = 0;

        // Collect each center aligned component
        for (int index = 0; index < componentCount(Center); ++index)
        {
            Component * component = componentAt(index, Center);
            int height = component->height();
            maxHeight += height;
            centerHeight += height;

            // Accumulate height
            d->dynamicComponentPositions[relativeTop + height - 1] = component;
            component->setRelativeTop(relativeTop);
            relativeTop += height;
        }

        // Attach...
        top = minHeight + centerHeight;
        if (top + bottomHeight > viewport()->height())
        {
            top = viewport()->height() - bottomHeight;
        }

        // Set document geometry
        vScrollArea.setBottom(top - 1);
        setVerticalScrollArea(vScrollArea);

        // Collect each bottom aligned component
        for (int index = 0; index < componentCount(Bottom); ++index)
        {
            Component * component = componentAt(index, Bottom);
            int height = component->height();
            component->setTop(top);
            d->staticComponentPositions[top + height - 1] = component;

            // Accumulate height
            top += height;
            relativeTop += height;
            maxHeight += height;
            minHeight += height;
        }

        // Set document geometry
        QRect hScrollArea = d->horizontalScrollArea;
        hScrollArea.setHeight(maxHeight);
        setHorizontalScrollArea(hScrollArea);

        // Deal with aspect heights // FIXME
/*
  QListIterator< Aspect * > iter(d->aspectsAll);
  while (iter.hasNext())
  {
  Aspect * aspect = iter.next();
  }
*/
    }

    /**
     *  \brief Find the index of an aspect in this view.
     */
    int AlignmentView::indexOfAspect(Aspect * aspect, AspectPosition position) const
    {
        QPair< int, AspectPosition > logical = actualToLogicalAspect(d->aspectsAll.indexOf(aspect));
        if (logical.second == position)
        {
            return logical.first;
        }
        else
        {
            return -1;
        }
    }

    /**
     *  \brief Find the index of a component in this view.
     */
    int AlignmentView::indexOfComponent(Component * component, ComponentPosition position) const
    {
        QPair< int, ComponentPosition > logical = actualToLogicalComponent(d->componentsAll.indexOf(component));
        if (logical.second == position)
        {
            return logical.first;
        }
        else
        {
            return -1;
        }
    }

    /**
     *  \brief Insert an aspect in this view.
     */
    void AlignmentView::insertAspect(int index, AspectPosition position, Aspect * aspect)
    {
        if (aspect->alignmentView() == this) { return; }

        // Reparent
        aspect->setParent(this);

        // Insert aspect
        int insertionIndex = logicalToActualAspect(index, position);
        d->aspectsAll.insert(insertionIndex, aspect);
        if (position == Left) { ++d->aspectPartition; }

        // Update
        widthsChanged();
    }

    /**
     *  \brief Insert a component in this view.
     */
    void AlignmentView::insertComponent(int index, ComponentPosition position, Component * component)
    {
        if (component->alignmentView() == this) { return; }

        // Reparent
        component->setParent(this);

        // Insert component
        int insertionIndex = logicalToActualComponent(index, position);
        d->componentsAll.insert(insertionIndex, component);
        if (position != Bottom) { ++d->componentPartitions[(int) position]; }

        // Update
        heightsChanged();
    }

    AlignmentView::InteractionMode AlignmentView::interactionMode() const
    {
        return (InteractionMode) d->interactionModes->actions().indexOf(d->interactionModes->checkedAction());
    }

    bool AlignmentView::isDiscreteScrolling() const
    {
        return d->discreteScrolling;
    }

    void AlignmentView::leaveEvent(QEvent *)
    {
        Aspect * aspect = d->aspectUnderMouse;
        if (aspect == 0)
        {
            Component * component = d->componentUnderMouse;
            if (component != 0)
            {
                QEvent event(QEvent::Leave);
                QApplication::sendEvent(component, &event);
                d->componentUnderMouse = 0;
            }
        }
        else
        {
            QEvent event(QEvent::Leave);
            QApplication::sendEvent(aspect, &event);
            d->aspectUnderMouse = 0;
        }
    }

    bool AlignmentView::load(Utopia::Node * model)
    {
        appendComponent(Top, new KeyComponent());
        appendComponent(Bottom, new KeyComponent());
        appendAspect(Left, new TitleAspect());
        appendAspect(Right, new ControlAspect());

        // FIXME load sequences from model
        return false;
    }

    int AlignmentView::logicalToActualAspect(int index, AspectPosition position) const
    {
        return (position == Left) ? (index) : (index + d->aspectPartition);
    }

    int AlignmentView::logicalToActualComponent(int index, ComponentPosition position) const
    {
        switch (position)
        {
        case Bottom:
            index += d->componentPartitions[1];
        case Center:
            index += d->componentPartitions[0];
        default:
            break;
        }
        return index;
    }

    QPoint AlignmentView::mapFrom(AbstractComponent * component, const QPoint & pos) const
    {
        return pos + component->pos();
    }

    QRect AlignmentView::mapFrom(AbstractComponent * component, const QRect & rect) const
    {
        return rect.translated(component->pos());
    }

    QPoint AlignmentView::mapTo(AbstractComponent * component, const QPoint & pos) const
    {
        return pos - component->pos();
    }

    QRect AlignmentView::mapTo(AbstractComponent * component, const QRect & rect) const
    {
        return rect.translated(-component->pos());
    }

    /**
     *  \brief Capture mouse events for this view.
     */
    void AlignmentView::mouseDoubleClickEvent(QMouseEvent * event)
    {
        QPoint pos = event->pos();
        if (pos.x() < 0 || pos.y() < 0) { return; }
        d->updateMousePos(pos);

        AbstractComponent * abstractComponent = d->aspectUnderMouse ?
            (AbstractComponent *) d->aspectUnderMouse :
            (AbstractComponent *) d->componentUnderMouse;
        if (abstractComponent)
        {
            QApplication::sendEvent(abstractComponent, event);
        }
    }

    /**
     *  \brief Capture mouse events for this view.
     */
    void AlignmentView::mouseMoveEvent(QMouseEvent * event)
    {
        if (d->dragState == NoDrag)
        {
            QPoint pos = event->pos();
            if (pos.x() < 0 || pos.y() < 0) { return; }
            d->updateMousePos(pos);

            AbstractComponent * abstractComponent = d->aspectUnderMouse ?
                (AbstractComponent *) d->aspectUnderMouse :
                (AbstractComponent *) d->componentUnderMouse;

            if (event->buttons() & Qt::LeftButton && d->interactionComponent != 0 && (d->interaction == Gapping || d->interaction == Sliding))
            {
                QPoint componentPos = mapTo(d->interactionComponent, pos);
                int alignmentIndex = componentPos.x() / unitSize();
                if (d->interaction == Gapping)
                {
                    if (d->sequenceIndexPress >= 0)
                    {
                        d->interactionComponent->sequence()->setGap(d->sequenceIndexPress, d->gapPress + (alignmentIndex - d->alignmentIndexPress));
                    }
                }
                else
                {
                    d->interactionComponent->sequence()->setOffset(d->offsetPress + (alignmentIndex - d->alignmentIndexPress));
                }
            }
            else if (abstractComponent)
            {
                QApplication::sendEvent(abstractComponent, event);
            }
        }
        else
        {
            Aspect * aspect = 0;
            Component * component = 0;

            if (d->dragState == StartDrag && event->buttons() & Qt::LeftButton &&
                (event->pos() - d->dragStartPosition).manhattanLength() > QApplication::startDragDistance() * 2)
            {
#if defined(Q_WS_MAC) || defined(Q_WS_WIN)
                // Image to display when dragging
                QImage dragImage;
                QPoint hotSpot;
#endif

                // Find what is being dragged
                QPoint dir = event->pos() - d->dragStartPosition;
                if (abs(dir.x()) > abs(dir.y()) && d->aspectUnderMouse != 0)
                {
                    // Move aspect
                    aspect = d->aspectUnderMouse;
                    QPair< int, AspectPosition > logical = aspectPosition(aspect);
                    d->dragAspectIndex = logical.first;
                    d->dragAspectPosition = logical.second;
#if defined(Q_WS_MAC) || defined(Q_WS_WIN)
                    QRect geometry = aspect->geometry();
                    dragImage = QImage(geometry.width(), viewport()->height(), QImage::Format_ARGB32);
                    render(&dragImage, QPoint(0, 0), geometry.translated(1, 0));
                    hotSpot = QPoint(d->dragStartPosition.x() - geometry.left(), d->dragStartPosition.y());
#endif
                    d->dragState = DragAspect;
                }
                else if (d->componentUnderMouse != 0)
                {
                    // Move component
                    component = d->componentUnderMouse;
                    QPair< int, ComponentPosition > logical = componentPosition(component);
                    d->dragComponentIndex = logical.first;
                    d->dragComponentPosition = logical.second;
                    QRect geometry = component->geometry();
                    geometry.setLeft(0);
                    geometry.setWidth(viewport()->width());
#if defined(Q_WS_MAC) || defined(Q_WS_WIN)
                    dragImage = QImage(geometry.width(), geometry.height(), QImage::Format_ARGB32);
                    render(&dragImage, QPoint(0, 0), geometry.translated(0, 1));
                    hotSpot = QPoint(d->dragStartPosition.x(), d->dragStartPosition.y() - geometry.top());
#endif
                    d->dragState = DragComponent;
                }
                else
                {
                    return;
                }

                QDrag * drag = new QDrag(this);
                QMimeData * mimeData = new QMimeData;
                mimeData->setText("hello there");
                drag->setMimeData(mimeData);
#if defined(Q_WS_MAC) || defined(Q_WS_WIN)
                QPainter painter(&dragImage);
                painter.setBrush(Qt::NoBrush);
                painter.setPen(QColor(80, 80, 80));
                painter.drawRect(0, 0, dragImage.width() - 1, dragImage.height() - 1);
                painter.end();
                drag->setPixmap(QPixmap::fromImage(dragImage));
                drag->setHotSpot(hotSpot);
#endif
                /*Qt::DropAction dropAction =*/ drag->exec(Qt::MoveAction);
                d->dragState = NoDrag;
            }
        }
    }

    /**
     *  \brief Capture press events for this view.
     */
    void AlignmentView::mousePressEvent(QMouseEvent * event)
    {
        QPoint pos = event->pos();
        if (pos.x() < 0 || pos.y() < 0) { return; }
        d->updateMousePos(pos);

        AbstractComponent * abstractComponent = d->aspectUnderMouse ?
            (AbstractComponent *) d->aspectUnderMouse :
            (AbstractComponent *) d->componentUnderMouse;
        if (abstractComponent)
        {
            DataComponent * dataComponent = dynamic_cast< DataComponent * >(abstractComponent);
            QPoint componentPos = mapTo(abstractComponent, pos);

            int iMode = interactionMode();
            if (dataComponent && (iMode == GapMode || iMode == SlideMode))
            {
                int alignmentIndex = componentPos.x() / unitSize();
                d->interaction = iMode == GapMode ? Gapping : Sliding;
                setCursor(Qt::ClosedHandCursor);
                d->interactionComponent = dataComponent;
                d->alignmentIndexPress = d->alignmentIndexMove = alignmentIndex;
                d->sequenceIndexPress = d->sequenceIndexMove = dataComponent->sequence()->mapToSequence(alignmentIndex);
                d->offsetPress = dataComponent->sequence()->offset();
                d->gapPress = dataComponent->sequence()->gap(d->sequenceIndexPress);
            }
            else
            {
                QApplication::sendEvent(abstractComponent, event);
            }
        }

        // If already accepted
        if (event->isAccepted())
        {
            QMouseEvent transformedEvent(QEvent::MouseMove,
                                         event->pos(),
                                         event->globalPos(),
                                         Qt::NoButton,
                                         event->buttons() | event->button(),
                                         event->modifiers());
            mouseMoveEvent(&transformedEvent);
        }
        else
        {
            if (event->button() == Qt::LeftButton && d->dragState == NoDrag)
            {
                d->dragStartPosition = event->pos();
                d->dragState = StartDrag;
            }
        }
    }

    /**
     *  \brief Capture mouse events for this view.
     */
    void AlignmentView::mouseReleaseEvent(QMouseEvent * event)
    {
        if (d->dragState == NoDrag || d->dragState == StartDrag)
        {
            QPoint pos = event->pos();
            if (pos.x() < 0 || pos.y() < 0) { return; }
            d->updateMousePos(pos);
            d->interaction = Idle;
            setCursor(d->interactionCursor);

            AbstractComponent * abstractComponent = d->aspectUnderMouse ?
                (AbstractComponent *) d->aspectUnderMouse :
                (AbstractComponent *) d->componentUnderMouse;
            if (abstractComponent)
            {
                QApplication::sendEvent(abstractComponent, event);
            }
        }

        d->dragState = NoDrag;
    }

    /**
     *  \brief Move an aspect within this view.
     */
    void AlignmentView::moveAspect(int from, AspectPosition fromPosition, int to, AspectPosition toPosition)
    {
        // Move aspect
        from = logicalToActualAspect(from, fromPosition);
        to = logicalToActualAspect(to, toPosition);
        if (fromPosition != toPosition)
        {
            if (fromPosition == Left) { --d->aspectPartition; }
            if (toPosition == Left) { ++d->aspectPartition; }
            if (to > from) { --to; }
        }
        if (from != to)
        {
            d->aspectsAll.move(from, to);
        }

        // Update
        widthsChanged();
    }

    /**
     *  \brief Move a component within this view.
     */
    void AlignmentView::moveComponent(int from, ComponentPosition fromPosition, int to, ComponentPosition toPosition)
    {
        // Move component
        from = logicalToActualComponent(from, fromPosition);
        to = logicalToActualComponent(to, toPosition);
        if (fromPosition != toPosition)
        {
            if (fromPosition != Bottom) { --d->componentPartitions[(int) fromPosition]; }
            if (toPosition != Bottom) { ++d->componentPartitions[(int) toPosition]; }
            if (to > from) { --to; }
        }
        if (from != to)
        {
            d->componentsAll.move(from, to);
        }

        // Update
        heightsChanged();
    }

    /**
     *  \brief Paint this view.
     */
    void AlignmentView::paintEvent(QPaintEvent * event)
    {
//              qDebug() << "AlignmentView::paintEvent" << event->rect();
//              qDebug() << "viewport()->testAttribute" << viewport()->testAttribute(Qt::WA_StaticContents);
//              qDebug() << "testAttribute" << testAttribute(Qt::WA_StaticContents);

        // Painter
        QPainter painter(viewport());

        // Set font according to current zoom factor
        QFont font = painter.font();
#ifdef Q_WS_MAC
        font.setPointSizeF(unitSizeF() / 2.2);
#else
        font.setPointSizeF(unitSizeF() / 2.8);
#endif
        painter.setFont(font);

        // Background
        painter.setPen(Qt::NoPen);
        painter.setBrush(palette().color(QPalette::AlternateBase));
        painter.drawRect(viewport()->rect());

        // Render selections
        Selection::const_iterator iter = d->selection.begin();
        while (iter != d->selection.end())
        {
            SelectionRange range = *iter;
            SelectionRange rangeParts[3];
            if (range.contains(d->componentPartitions[0]) && range.from() < d->componentPartitions[0])
            {
                rangeParts[0].setRange(range.from(), d->componentPartitions[0] - 1);
                if (range.contains(d->componentPartitions[1] + d->componentPartitions[0]))
                {
                    rangeParts[1].setRange(d->componentPartitions[0], d->componentPartitions[1] + d->componentPartitions[0] - 1);
                    rangeParts[2].setRange(d->componentPartitions[1] + d->componentPartitions[0], range.to());
                }
                else
                {
                    rangeParts[1].setRange(d->componentPartitions[0], range.to());
                }
            }
            else if (range.contains(d->componentPartitions[1] + d->componentPartitions[0]) && range.from() < d->componentPartitions[1] + d->componentPartitions[0])
            {
                rangeParts[1].setRange(range.from(), d->componentPartitions[1] + d->componentPartitions[0] - 1);
                rangeParts[2].setRange(d->componentPartitions[1] + d->componentPartitions[0], range.to());
            }
            else if (range.to() < d->componentPartitions[0])
            {
                rangeParts[0] = range;
            }
            else if (range.to() < d->componentPartitions[1] + d->componentPartitions[0])
            {
                rangeParts[1] = range;
            }
            else
            {
                rangeParts[2] = range;
            }

            int offset = 0;
            for (int i = 0; i < 3; ++i)
            {
                painter.save();

                if (i > 0)
                {
                    offset += d->componentPartitions[i - 1];
                }
                if (i == 1)
                {
                    painter.setClipRect(d->verticalScrollArea);
                }

                if (rangeParts[i].isValid())
                {
                    static const ComponentPosition pos[3] = { Top, Center, Bottom };
                    int top = d->componentsAll[rangeParts[i].from()]->top();
                    int height = componentRangeHeight(rangeParts[i].from() - offset, rangeParts[i].to() - offset, pos[i]);
                    QRect rect = QRect(0, top, width(), height);
                    painter.setPen(Qt::NoPen);
                    QColor highlight = palette().color(QPalette::Highlight);
                    QLinearGradient grad(rect.topLeft(), rect.bottomLeft());
                    grad.setColorAt(0, highlight.lighter(180));
                    grad.setColorAt(1, highlight);
                    painter.setBrush(grad);
                    painter.drawRect(rect);
                    painter.setPen(highlight.lighter(140));
                    painter.drawLine(rect.topLeft(), rect.topRight());
                }

                painter.restore();
            }

            ++iter;
        }

        painter.setPen(palette().color(QPalette::Dark));
        painter.setBrush(palette().color(QPalette::Base));
        painter.drawRect(d->horizontalScrollArea.adjusted(-1, 0, 0, 0));

        // Dropping outline
        QPainterPath dropIndicator;

        // Render components
        for (int actual = 0; actual < componentCount(); ++actual)
        {
            QPair< int, ComponentPosition > logical = actualToLogicalComponent(actual);
            int index = logical.first;
            ComponentPosition position = logical.second;
            Component * component = componentAt(index, position);
            QRect geometry = component->geometry();
            QRect clipped = geometry.intersected(event->rect()).intersected(d->horizontalScrollArea);
            if (position == Center) { clipped &= d->verticalScrollArea; }

            // Ignore if outside region
            if (clipped.isEmpty()) { continue; }

            // Transform and clip
            painter.save();
            painter.setClipRect(clipped);
            painter.setWindow(-geometry.left(), -geometry.top(), viewport()->width(), viewport()->height());

            // Delegate painting
            RenderEvent renderEvent(&painter, mapTo(component, clipped));
            QApplication::sendEvent(component, &renderEvent);
            painter.restore();

            // Drop indicator
            if (d->dragState == DragComponent)
            {
                geometry.adjust(0, 1, 0, -2);
                geometry.setLeft(1);
                geometry.setRight(viewport()->width() - 3);
                if (position == d->dropComponentPosition &&
                    index == d->dropComponentIndex)
                {
                    if (d->dragComponentPosition != d->dropComponentPosition ||
                        d->dragComponentIndex != d->dropComponentIndex)
                    {
                        if (d->dropComponentPosition == d->dragComponentPosition &&
                            d->dropComponentIndex > d->dragComponentIndex)
                        {
                            dropIndicator.moveTo(geometry.bottomLeft() + QPoint(0, 1));
                            dropIndicator.lineTo(geometry.bottomRight() + QPoint(1, 1));
                        }
                        else
                        {
                            dropIndicator.moveTo(geometry.topLeft());
                            dropIndicator.lineTo(geometry.topRight() + QPoint(1, 0));
                        }
                    }
                    else
                    {
                        dropIndicator.addRect(geometry);
                    }
                }
            }
        }

        // Render aspects
        for (int actual = 0; actual < aspectCount(); ++actual)
        {
            QPair< int, AspectPosition > logical = actualToLogicalAspect(actual);
            int index = logical.first;
            AspectPosition position = logical.second;
            Aspect * aspect = aspectAt(index, position);
            QRect geometry = aspect->geometry();
            QRect clipped = geometry.intersected(event->rect());

            // Ignore if outside region
            if (clipped.isEmpty()) { continue; }

            // Transform and clip
            painter.save();
            painter.translate(geometry.topLeft());
            painter.setClipRect(aspect->rect());

            // Delegate painting
            RenderEvent renderEvent(&painter, mapTo(aspect, clipped));
            QApplication::sendEvent(aspect, &renderEvent);
            painter.restore();

            // Drop indicator
            if (d->dragState == DragAspect)
            {
                geometry.adjust(1, 1, -2, -2);
                if (position == d->dropAspectPosition &&
                    index == d->dropAspectIndex)
                {
                    if (d->dragAspectPosition != d->dropAspectPosition ||
                        d->dragAspectIndex != d->dropAspectIndex)
                    {
                        if (d->dropAspectPosition == d->dragAspectPosition &&
                            d->dropAspectIndex > d->dragAspectIndex)
                        {
                            dropIndicator.moveTo(geometry.topRight() + QPoint(1, 0));
                            dropIndicator.lineTo(geometry.bottomRight() + QPoint(1, 1));
                        }
                        else
                        {
                            dropIndicator.moveTo(geometry.topLeft());
                            dropIndicator.lineTo(geometry.bottomLeft());
                        }
                    }
                    else
                    {
                        dropIndicator.addRect(geometry);
                    }
                }
            }
        }

        // Drop indicator
        if (d->dragState == DragAspect)
        {
            if (d->dropAspectIndex == aspectCount(d->dropAspectPosition))
            {
                switch (d->dropAspectPosition)
                {
                case Left:
                    dropIndicator.moveTo(d->horizontalScrollArea.left() + 1, 1);
                    dropIndicator.lineTo(d->horizontalScrollArea.left() + 1, viewport()->height() - 2);
                    break;
                case Right:
                    dropIndicator.moveTo(viewport()->width() - 2 - aspectMargin(), 1);
                    dropIndicator.lineTo(viewport()->width() - 2 - aspectMargin(), viewport()->height() - 2);
                    break;
                default:
                    break;
                }
            }
        }
        else if (d->dragState == DragComponent)
        {
            if (d->dropComponentIndex == componentCount(d->dropComponentPosition))
            {
                switch (d->dropComponentPosition)
                {
                case Top:
                    dropIndicator.moveTo(0, d->verticalScrollArea.top() - 2);
                    dropIndicator.lineTo(viewport()->width() - 2, d->verticalScrollArea.top() - 2);
                    break;
                case Center:
                    dropIndicator.moveTo(0, d->verticalScrollArea.bottom() - 1);
                    dropIndicator.lineTo(viewport()->width() - 2, d->verticalScrollArea.bottom() - 1);
                    break;
                case Bottom:
                {
                    int visibleBottom = componentAt(componentCount(Bottom) - 1, Bottom)->geometry().bottom();
                    dropIndicator.moveTo(0, visibleBottom - 1);
                    dropIndicator.lineTo(viewport()->width() - 2, visibleBottom - 1);
                    break;
                }
                default:
                    break;
                }
            }
        }

        // Drop indicator
        if (d->dragState == DragAspect || d->dragState == DragComponent)
        {
            painter.save();
            painter.setPen(QColor(205, 0, 0));
            painter.setBrush(Qt::NoBrush);
            QPen pen = painter.pen();
            pen.setWidth(3);
            painter.setPen(pen);
            painter.drawPath(dropIndicator);
            painter.restore();
        }

#if 0
        QVectorIterator< QRect > rects = event->region().rects();
        while (rects.hasNext())
        {
            painter.setPen(QColor(205, 0, 0));
            painter.setBrush(Qt::NoBrush);
            painter.drawRect(rects.next().adjusted(0, 0, -1, -1));
        }
#endif
    }

    /**
     *  \brief Remove an aspect from this view.
     */
    void AlignmentView::removeAspectAt(int index, AspectPosition position)
    {
        delete aspectAt(index, position);
    }

    /**
     *  \brief Remove a component from this view.
     */
    void AlignmentView::removeComponentAt(int index, ComponentPosition position)
    {
        delete componentAt(index, position);
    }

    /**
     *  \brief Resize this view.
     */
    void AlignmentView::resizeEvent(QResizeEvent * event)
    {
        // Update widths if necessary
        if (event->oldSize().width() != event->size().width())
        {
            d->verticalScrollArea.setWidth(event->size().width());
            widthsChanged();
        }

        // Update heights if necessary
        if (event->oldSize().height() != event->size().height())
        {
            d->horizontalScrollArea.setHeight(event->size().height());
            heightsChanged();
        }

        horizontalScrollBar()->setPageStep(unitSize() * (documentGeometry().width() / unitSizeF()));
        verticalScrollBar()->setPageStep(unitSize() * (documentGeometry().height() / unitSizeF()));
    }

    Selection AlignmentView::selection() const
    {
        return d->selection;
    }

    void AlignmentView::scrollContentsBy(int dx, int dy)
    {
//              qDebug() << "AlignmentView::scrollContentsBy" << dx << dy;
        if (dx != 0)
        {
            viewport()->update(d->horizontalScrollArea);
        }
        if (dy != 0)
        {
            viewport()->update(d->verticalScrollArea);
        }
    }

    /**
     *  \brief Set inter-aspect margin.
     */
    void AlignmentView::setAspectMargin(int margin)
    {
        // Constrain
        if (margin < 0) { margin = 0; }

        // Change is necessary
        if (margin != aspectMargin())
        {
            d->aspectMargin = margin;

            // FIXME Propagate
        }
    }

    /**
     *  \brief Set inter-aspect spacing.
     */
    void AlignmentView::setAspectSpacing(int spacing)
    {
        // Constrain
        if (spacing < 0) { spacing = 0; }

        // Change is necessary
        if (spacing != aspectSpacing())
        {
            d->aspectSpacing = spacing;

            // FIXME Propagate
        }
    }

    void AlignmentView::setDiscreteScrolling(bool discrete)
    {
        d->discreteScrolling = discrete;
    }

    void AlignmentView::setHorizontalScrollArea(const QRect & rect)
    {
        QRect oldRect = d->horizontalScrollArea;
        d->horizontalScrollArea = rect;

        if (rect.left() != oldRect.left())
        {
            viewport()->update(rect);
        }
        if (rect.width() > oldRect.width())
        {
            viewport()->update(QRect(oldRect.right(), rect.top(), rect.width() - oldRect.width(), rect.height()));
        }
    }

    void AlignmentView::setInteractionMode(InteractionMode mode)
    {
        d->interactionModes->actions()[mode]->setChecked(true);
    }

    void AlignmentView::setSelection(const Selection & selection)
    {
        Selection oldSelection(d->selection);
        d->selection = selection;
        oldSelection.merge(selection, Selection::Select);
        foreach(SelectionRange range, oldSelection)
        {
            for (int index = range.from(); index <= range.to(); ++index)
            {
                QPair< int, AlignmentView::ComponentPosition > changedPosition(actualToLogicalComponent(index));
                componentAt(changedPosition.first, changedPosition.second)->updateGeometry();
            }
        }
    }

    /**
     *  \brief Set an alignment view's unit size.
     *  \param size new unit size.
     */
    void AlignmentView::setUnitSize(int size)
    {
        // Constrain
        if (size < 2) { size = 2; }
        else if (size > 80) { size = 80; }

        // Make change if necessary
        if (d->unitSize != size)
        {
            d->unitSize = size;

            // Propagate change
            heightsChanged();
            widthsChanged();
            viewport()->update();

            // Emit signal
            emit unitSizeChanged(d->unitSize);
        }
    }

    void AlignmentView::setVerticalScrollArea(const QRect & rect)
    {
        QRect oldRect = d->verticalScrollArea;
        d->verticalScrollArea = rect;

        if (rect.top() != oldRect.top())
        {
            viewport()->update(rect);
        }
        if (rect.height() > oldRect.height())
        {
            viewport()->update(QRect(rect.left(), oldRect.bottom(), rect.width(), rect.height() - oldRect.height()));
        }
    }

    void AlignmentView::setZoom(double zoom)
    {
        if (d->zoom != zoom)
        {
            d->zoom = zoom;
            d->unitSize = (int) zoom;

            // Propagate change
            heightsChanged();
            widthsChanged();
            viewport()->update();

            emit zoomChanged(zoom);
        }
    }

    bool AlignmentView::supports(Utopia::Node * model) const
    {
        // is it just a sequence?
        bool supported = (model->type() == Utopia::UtopiaDomain.term("Sequence"));

        // if not, is it a collection of sequences?
        if (!supported)
        {
            Utopia::Node::relation::iterator seq = model->relations(Utopia::UtopiaSystem.hasPart).begin();
            Utopia::Node::relation::iterator end = model->relations(Utopia::UtopiaSystem.hasPart).end();

            while (seq != end)
            {
//                              qDebug() << "testing" << (*seq)->type()->attributes.get(Utopia::UtopiaSystem.uri).toString();
                if ((supported = supports(*seq)))
                {
                    break;
                }
                ++seq;
            }
        }

        return supported;
    }

    /**
     *  \brief Take an aspect from this view.
     */
    Aspect * AlignmentView::takeAspectAt(int index, AspectPosition position)
    {
        Aspect * aspect = aspectAt(index, position);
        aspect->setParent(0);
        return aspect;
    }

    /**
     *  \brief Take a component from this view.
     */
    Component * AlignmentView::takeComponentAt(int index, ComponentPosition position)
    {
        Component * component = componentAt(index, position);
        component->setParent(0);
        return component;
    }

    /**
     *  \brief Query this view's unit size.
     *  \return this view's unit size.
     */
    int AlignmentView::unitSize() const
    {
        return d->unitSize;
    }

    double AlignmentView::unitSizeF() const
    {
        return d->zoom;
    }

    /**
     *  \brief Recalculate aspect widths.
     */
    void AlignmentView::widthsChanged()
    {
        // Reset position cache
        d->aspectPositions.clear();

        // Component sizing
        int margin = aspectMargin();
        int spacing = aspectSpacing();
        int left = margin;
        int right = viewport()->width() - margin;
        int minWidth = 2 * margin;

        // Collect each left aligned aspect
        for (int index = 0; index < aspectCount(Left); ++index)
        {
            Aspect * aspect = aspectAt(index, Left);
            int width = aspect->width();
            aspect->setLeft(left);
            aspect->setWidth(width);
            d->aspectPositions[left + width - 1] = aspect;

            // Accumulate width (+ spacing)
            left += width + spacing;
            minWidth += width + spacing;
        }

        // Collect each right aligned aspect
        for (int index = aspectCount(Right) - 1; index >= 0; --index)
        {
            Aspect * aspect = aspectAt(index, Right);
            int width = aspect->width();

            // Accumulate width (+ spacing)
            right -= width + spacing;
            minWidth += width + spacing;

            aspect->setLeft(right + spacing);
            aspect->setWidth(width);
            d->aspectPositions[right + spacing + width - 1] = aspect;
        }

        // Deal with component widths
/*
  QListIterator< Component * > iter(d->componentsAll);
  while (iter.hasNext())
  {
  Component * component = iter.next();
  }
*/

        // Set document geometry
        d->horizontalScrollArea.setLeft(left);
        d->horizontalScrollArea.setWidth(right - left);
        d->aspectPositions[right] = 0;

        // Set widget Minimums
        setMinimumWidth(minWidth + verticalScrollBar()->width());
    }

    void AlignmentView::wheelEvent(QWheelEvent * event)
    {
        // Catch Control modified wheel events
        if (event->modifiers() == Qt::ControlModifier &&
            event->buttons() == Qt::NoButton)
        {
            zoomIn((int) (event->delta() / 120.0));
        }
        else
        {
            // Standard event processing
            QAbstractScrollArea::wheelEvent(event);
        }
    }

    double AlignmentView::zoom() const
    {
        return d->zoom;
    }

    void AlignmentView::zoomIn(int factor)
    {
        if (factor < 0)
        {
            zoomOut(-factor);
        }
        else if (factor > 0)
        {
            for (; factor > 0; --factor)
            {
                if (d->zoom < 1)
                {
                    setZoom(d->zoom * 2);
                }
                else
                {
                    setZoom(d->zoom + 1);
                }
            }
        }
        qDebug() << "Zoom factor: " << zoom();
    }

    void AlignmentView::zoomOut(int factor)
    {
        if (factor < 0)
        {
            zoomIn(-factor);
        }
        else if (factor > 0)
        {
            for (; factor > 0; --factor)
            {
                if (d->zoom > 1)
                {
                    setZoom(d->zoom - 1);
                }
                else
                {
                    setZoom(d->zoom / 2.0);
                }
            }
        }
        qDebug() << "Zoom factor: " << zoom();

    }

}
