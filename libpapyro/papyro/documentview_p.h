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

#ifndef DOCUMENTVIEW_P_H
#define DOCUMENTVIEW_P_H

#include <papyro/config.h>
#include <papyro/overlayrenderer.h>
#include <papyro/overlayrenderermapper.h>
#include <papyro/documentproxy.h>
#include <papyro/documentview.h>
#include <papyro/pageview.h>

#if !defined(Q_MOC_RUN) || QT_VERSION >= 0x050000
#  include <spine/Document.h>
#  include <boost/multi_array.hpp>
#endif

#include <QMap>
#include <QObject>
#include <QPicture>

class QBoxLayout;
class QSignalMapper;
class QActionGroup;

namespace Papyro
{

    class DocumentView;
    class PageView;



    class PageViewOverlay
    {
    public:
        PageViewOverlay();

        // Overlay widget
        QWidget * widget;

        // Selection paths
        QPainterPath savedSelectionHighlight; // applied selection
        QPainterPath activeSelectionHighlight; // current interactive selection
        QPainterPath selectionHighlight; // the above two paths added together
    };




    class PageViewMouseEvent
    {
    public:
        PageViewMouseEvent();
        PageViewMouseEvent(PageView * pageView,
                           QMouseEvent * event,
                           const QPoint & pos,
                           int cardinality);
        PageViewMouseEvent(PageView * pageView,
                           const QPoint & pos,
                           Qt::MouseButton button,
                           Qt::MouseButtons buttons,
                           Qt::KeyboardModifiers modifiers,
                           int cardinality);

        PageView * pageView;
        QPoint pos;
        QPointF pagePos;
        Qt::MouseButton button;
        Qt::MouseButtons buttons;
        Qt::KeyboardModifiers modifiers;
        int cardinality;

        QPoint globalPos() const;
        operator bool() const;
    };




    enum {
        GridChange      = 0x0001,
        SizeChange      = 0x0002
    };




    class DocumentViewPrivate : public QObject
    {
        Q_OBJECT

    public:
        // Current interaction state
        typedef enum
        {
            IdleState                   = 0x00000000,

            SelectingFlag               = 0x00001000,
            ActivatingFlag              = 0x00002000,
            DraggingFlag                = 0x00004000,

            SelectionFlag               = 0x00000001,
            TextFlag                    = 0x00000002,
            AreaTextFlag                = 0x00000004,
            AreaFlag                    = 0x00000008,
            StaticTextFlag              = 0x00000010,
            AnnotationFlag              = 0x00000020,
            ImageFlag                   = 0x00000040,

            SelectingTextState          = SelectingFlag + TextFlag,
            SelectingAreaTextState      = SelectingFlag + AreaTextFlag,
            SelectingAreaState          = SelectingFlag + AreaFlag,
            SelectingStaticTextState    = SelectingFlag + StaticTextFlag,
            SelectingImageState         = SelectingFlag + ImageFlag,
            ActivatingAnnotationState   = ActivatingFlag + AnnotationFlag,
            ActivatingSelectionState    = ActivatingFlag + SelectionFlag,
            DraggingSelectionState      = DraggingFlag + SelectionFlag,
            DraggingImageState          = DraggingFlag + ImageFlag,

            AmbiguousState              = 0xffffffff
        } InteractionState;

        DocumentViewPrivate(DocumentView * documentView);

        // Document View
        DocumentView * documentView;
        Spine::DocumentHandle document;
        boost::scoped_ptr< DocumentProxy > documentProxy;

        // Which annotation / part there-of is currently being focused on by the user
        struct {
            Spine::AnnotationSet annotations;
            Spine::AnnotationHandle annotation;
            //Spine::TextExtent textExtent;
            //Spine::Area area;
        } current;

        // Interaction modes / state
        struct {
            // What mode is expected / currently being used
            DocumentView::InteractionMode mode;
            QList< InteractionState > states;

            // Mouse interaction
            bool isPressed;
            bool isDragging;
            int numberOfClicks;
            int numberOfClickTimeouts;
            int numberOfHolds;
            int numberOfHoldTimeouts;
            PageViewMouseEvent previousPressEvent;
            QPoint previousMovePos;
            QPoint previousReleasePos;
            PageViewMouseEvent pendingClickEvent;
            int holdInterval;
            Spine::CursorHandle mouseTextCursor;

            // Scrollbar interaction
            bool updateOrigin;

            // Keyboard interaction
            bool isExposing;
        } interaction;

        // Selection state
        struct {
            // Selecting an area
            QPointF activeAreaFromPoint;
            QPointF activeAreaToPoint;
            PageView * activeAreaPageView;

            // Selecting text
            Spine::CursorHandle activeTextFromCursor;
            Spine::CursorHandle activeTextToCursor;
            Spine::TextExtentHandle activeTextExtent;

            // Selecting image
            const Spine::Image * image;

            // Colour?
            QColor color;
        } selection;

        // Annotation rendering / interaction
        struct {
            QMap< Spine::AnnotationHandle, QPair< OverlayRenderer *, QMap< int, QPainterPath > > > bounds;
            QMap< OverlayRenderer *, QMap< OverlayRenderer::State, QPair< Spine::AnnotationSet, QMap< int, QPicture > > > > pictures;
            QMap< Spine::AnnotationHandle, QMap< int, QPicture > > hoverPictures;

            DefaultOverlayRenderer defaultOverlayRenderer;
        } rendering;
        QMap< QString, OverlayRenderer * > overlayRenderers;
        QList< OverlayRendererMapper * > overlayRendererMappers;
        void setAnnotationState(const Spine::AnnotationSet & annotations, OverlayRenderer::State state);

        // Page Views
        QVector< PageView * > pageViews;
        QMap< PageView *, PageViewOverlay > pageViewOverlays;

        // Current state
        int pageNumber;
        DocumentView::BindingMode bindingMode;
        DocumentView::PageFlow pageFlow;
        DocumentView::PageFlowDirection pageFlowDirection;
        DocumentView::PageMode pageMode;
        DocumentView::ZoomMode zoomMode;
        double zoom;

        // Search highlights
        std::vector< Spine::TextExtentHandle > spotlights;
        size_t activeSpotlight;

        // Zooming members
        QMap< int, QAction * > zoomPercentages;
        QAction * actionBespokeZoom;
        QSignalMapper * zoomSignalMapper;
        QActionGroup * zoomActionGroup;

        // Menus and actions
        QMenu * zoomMenu;
        QAction * actionZoomIn;
        QAction * actionZoomOut;
        QAction * actionFitToHeight;
        QAction * actionFitToWidth;
        QAction * actionFitToWindow;
        QMenu * layoutMenu;
        QAction * actionOnePage;
        QAction * actionOnePageContinuous;
        QAction * actionTwoPages;
        QAction * actionTwoPagesContinuous;
        QActionGroup * groupDirection;
        QAction * actionLeftToRightFlow;
        QAction * actionTopDownFlow;
        QAction * actionFirstPageOdd;

        bool autoScrollBars;

        QPoint panStartPos;
        QPoint panStartOffset;

        // Layout calculation cache
        class Layout {
        public:
            class CellCoord {
            public:
                CellCoord() : index(0), largestPageView(0) {}
                int index;
                PageView * largestPageView;
            };

            class Cell {
            public:
                Cell() : pageView(0) {}
                PageView * pageView;
                QPoint pos;
            };

            typedef std::map< int, CellCoord > SpacingMap;
            typedef boost::multi_array< Cell, 2 > PageViewMatrix;

            SpacingMap columnSpacing;
            SpacingMap rowSpacing;
            PageViewMatrix matrix;

            QSize size;

            int horizontalWhitespace;
            int verticalWhitespace;

            qreal horizontalOrigin;
            QPoint horizontalOriginPageViewGridCoords;
            qreal verticalOrigin;
            QPoint verticalOriginPageViewGridCoords;
        } layout;

        // Outlines between pages
        QPicture pageOutlines;

        void applyActiveAreaSelection();
        void applyActiveTextSelection();
        void clearPageViews();
        Spine::AnnotationHandle createHighlight(const Spine::Area * area, Spine::TextExtentHandle extent, bool store = true, bool persist = false);
        void createPageViews();
        void initialise();
        bool isMouseOverImage();
        bool isMouseOverText();
        QColor selectionColor() const;

        InteractionState interactionState() const;
        InteractionState primaryInteractionState() const;
        void addInteractionState(InteractionState state);
        void clearInteractionStates();
        void setInteractionState(InteractionState state);
        void setInteractionStates(const QList< InteractionState > & states);

        void setZoom(double newZoom);
        void updateActions();
        void updateActiveAreaSelection();
        void updateActiveTextSelection();
        void updateAnnotationsUnderMouse(PageView * pageView, const QPointF & pagePos);
        void updateSavedSelection(const QSet< int > & changedPages);
        void updateSelection(PageView * pageView);
        void updateZoomMenu();

        // Layout engine
        void layout_calculateGrid();
        void layout_calculateWhitespace();
        void layout_calculatePageViewPositions();
        void layout_calculateHorizontalOrigin();
        void layout_calculateVerticalOrigin();
        void layout_updatePageViewSizes();
        void layout_updatePageViewPositions();

        void updatePageOutlines();
        void updateScrollBars();
        void updateScrollBarPolicies();

        // Deal with mouse interaction and overlay of annotations
        bool eventFilter(QObject * obj, QEvent * e);

    public slots:
        void update_layout(int changed = GridChange);

        // Context menu action slots
        void onFitToHeight();
        void onFitToWidth();
        void onFitToWindow();
        void onLeftToRightFlow();
        void onOnePage();
        void onOnePageContinuous();
        void onTopDownFlow();
        void onTwoPages();
        void onTwoPagesContinuous();
        void toggleBespokeZoom(bool);
        void toggleFirstPageOdd(bool odd);

        // Deal with page view signals
        void onTextSelectionChanged();
        void onSelectionStarted();
        void onSelectionFinished();

        // Scrollbar interaction
        void onHorizontalScrollBarValueChanged(int value);
        void onVerticalScrollBarValueChanged(int value);

        // Mouse interaction
        void onWaitingForDblClickTimeout();
        void onWaitingForHoldTimeout();

        // Deal with document changes
        void onDocumentAnnotationsChanged(const std::string & name, const Spine::AnnotationSet & annotations, bool added);
        void onDocumentAreaSelectionChanged(const std::string & name, const Spine::AreaSet & areas, bool added);
        void onDocumentTextSelectionChanged(const std::string & name, const Spine::TextExtentSet & extents, bool added);

    signals:
        void annotationsActivated(Spine::AnnotationSet annotations, const QVariantMap & context);
        void focusChanged(PageView * pageView, const QPointF & pagePos);
        void pageFocusChanged(size_t pageNumber);
        void selectionChanged(Spine::TextSelection selection);
        void zoomChanged();

    protected:
        void mouseClick(PageViewMouseEvent * event);
        void mouseDrag(PageViewMouseEvent * event);
        void mouseHold(PageViewMouseEvent * event);
        void mouseMove(PageViewMouseEvent * event);
        void mousePress(PageViewMouseEvent * event);
        void mouseRelease(PageViewMouseEvent * event);

        Spine::CursorHandle textCursorAt(PageView * pageView, const QPointF & point, Spine::DocumentElement element = Spine::ElementCharacter) const;
    };

}

#endif // DOCUMENTVIEW_P_H
