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

#ifndef ALIGNMENTVIEW_H
#define ALIGNMENTVIEW_H

#include <cinema6/config.h>
#include <cinema6/selection.h>
#include <utopia2/qt/abstractwidget.h>

#include <QAbstractScrollArea>
#include <QPair>
#include <boost/scoped_ptr.hpp>

class QAction;
class QActionGroup;
class QChildEvent;
class QDragEnterEvent;
class QDragMoveEvent;
class QDropEvent;
class QSlider;

namespace CINEMA6
{

    class AbstractComponent;
    class Aspect;
    class Component;
    class DataComponent;

    class AlignmentViewPrivate;
    class LIBCINEMA_API AlignmentView : public QAbstractScrollArea, public Utopia::AbstractWidgetInterface
    {
        Q_OBJECT

    public:
        // Aspect positioning
        typedef enum
        {
            Left = 0,
            Right
        } AspectPosition;

        // Component positioning
        typedef enum
        {
            Top = 0,
            Center,
            Bottom
        } ComponentPosition;

        // Alignment Edit Mode
        typedef enum
        {
            SelectMode = 0,
            SlideMode,
            GapMode,
            AnnotateMode,
            Locked
        } InteractionMode;

        // Construction
        AlignmentView(QWidget * parent = 0);
        // Destruction
        virtual ~AlignmentView();

        // Properties
        QPair< int, AspectPosition > actualToLogicalAspect(int index) const;
        QPair< int, ComponentPosition > actualToLogicalComponent(int index) const;
        void appendAspect(AspectPosition position, Aspect * aspect);
        void appendComponent(ComponentPosition position, Component * component);
        Aspect * aspectAt(int index, AspectPosition position) const;
        int aspectCount() const;
        int aspectCount(AspectPosition position) const;
        int aspectMargin() const;
        int aspectSpacing() const;
        QPair< int, AspectPosition > aspectPosition(Aspect * aspect) const;
        int aspectRangeWidth(int from, int to, AspectPosition position) const;
        Aspect * aspectUnder(int x) const;
        Component * componentAt(int index, ComponentPosition position) const;
        int componentCount() const;
        int componentCount(ComponentPosition position) const;
        QPair< int, ComponentPosition > componentPosition(Component * component) const;
        int componentRangeHeight(int from, int to, ComponentPosition position) const;
        Component * componentUnder(int y) const;
        QRect documentGeometry() const;
        int indexOfAspect(Aspect * aspect, AspectPosition position) const;
        int indexOfComponent(Component * component, ComponentPosition position) const;
        void insertAspect(int index, AspectPosition position, Aspect * aspect);
        void insertComponent(int index, ComponentPosition position, Component * component);
        InteractionMode interactionMode() const;
        bool isDiscreteScrolling() const;
        int logicalToActualAspect(int index, AspectPosition position) const;
        int logicalToActualComponent(int index, ComponentPosition position) const;
        QPoint mapFrom(AbstractComponent * component, const QPoint & pos) const;
        QRect mapFrom(AbstractComponent * component, const QRect & rect) const;
        QPoint mapTo(AbstractComponent * component, const QPoint & pos) const;
        QRect mapTo(AbstractComponent * component, const QRect & rect) const;
        void moveAspect(int from, AspectPosition fromPosition, int to, AspectPosition toPosition);
        void moveComponent(int from, ComponentPosition fromPosition, int to, ComponentPosition toPosition);
        void removeAspectAt(int index, AspectPosition position);
        void removeComponentAt(int index, ComponentPosition position);
        Selection selection() const;
        void setAspectMargin(int padding);
        void setAspectSpacing(int spacing);
        void setDiscreteScrolling(bool discrete);
        void setHorizontalScrollArea(const QRect & rect);
        void setSelection(const Selection & selection);
        void setVerticalScrollArea(const QRect & rect);
        void setZoom(double zoom);
        Aspect * takeAspectAt(int index, AspectPosition position);
        Component * takeComponentAt(int index, ComponentPosition position);
        int unitSize() const;
        double unitSizeF() const;
        double zoom() const;

    public slots:
        // AbstractWidget Interface
        virtual bool load(Utopia::Node * model);
        virtual bool supports(Utopia::Node * model) const;

    public slots:
        // Scrolling around
        void setInteractionMode(InteractionMode mode);
        void setUnitSize(int size);
        void zoomIn(int factor = 1);
        void zoomOut(int factor = 1);

    signals:
        // Scrolling around
        void unitSizeChanged(int size);
        void zoomChanged(double zoom);

    protected:
        // Events
        void childEvent(QChildEvent * event);
        void dragEnterEvent(QDragEnterEvent * event);
        void dragMoveEvent(QDragMoveEvent * event);
        void dropEvent(QDropEvent * event);
//            bool event(QEvent * event);
        void leaveEvent(QEvent * event);
        void mouseDoubleClickEvent(QMouseEvent * event);
        void mouseMoveEvent(QMouseEvent * event);
        void mousePressEvent(QMouseEvent * event);
        void mouseReleaseEvent(QMouseEvent * event);
        void paintEvent(QPaintEvent * event);
        void resizeEvent(QResizeEvent * event);
        void scrollContentsBy(int dx, int dy);
        void wheelEvent(QWheelEvent * event);

    private slots:
        // Actions
        void activateSelectMode();
        void activateSlideMode();
        void activateGapMode();
        void activateAnnotateMode();

        // Recalculate component heights / widths
        void heightsChanged();
        void widthsChanged();

    private:
        boost::scoped_ptr< AlignmentViewPrivate > d;

    }; // class AlignmentView

}

#endif // ALIGNMENTVIEW_H
