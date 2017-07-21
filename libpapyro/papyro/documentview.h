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

#ifndef DOCUMENTVIEW_H
#define DOCUMENTVIEW_H

#include <papyro/config.h>

#if !defined(Q_MOC_RUN) || QT_VERSION >= 0x050000
#  include <papyro/pageview.h>
#  include <spine/Document.h>
#endif

#include <QScrollArea>
//#include <QtDesigner/QDesignerExportWidget>

class QAbstractSlider;

namespace Papyro
{

    class DocumentViewPrivate;
    class PageView;

    class LIBPAPYRO_API DocumentView : public QAbstractScrollArea
    {
        Q_OBJECT

    public:
        typedef enum
        {
            Separate,
            Continuous
        } PageFlow;
        typedef enum
        {
            LeftToRight,
            TopDown
        } PageFlowDirection;
        typedef enum
        {
            OneUp,
            TwoUp
        } PageMode;
        typedef enum
        {
            Odd                 = 0x1,
            Even                = 0x2,
            OddAndEven          = 0x0
        } Parity;
        typedef Parity BindingMode;
        typedef enum
        {
            FitToHeight,
            FitToWidth,
            FitToWindow,
            FitToGingPo = FitToWindow,
            CustomZoom
        } ZoomMode;
        enum PageDecoration
        {
            Shadows             = 0x000001,
            CreasedSpines       = 0x000002,
            Reflections         = 0x000004,
            Legend              = 0x000008
        };
        Q_DECLARE_FLAGS(PageDecorations, PageDecoration);
        enum PageMargin
        {
            NoMargin            = 0x000000,
            TopMargin           = 0x000001,
            BottomMargin        = 0x000002,
            InsideMargin        = 0x000004,
            OutsideMargin       = 0x000008
        };
        Q_DECLARE_FLAGS(PageMargins, PageMargin);
        enum ActionFlag
        {
            PageLayoutFlag      = 0x000001,
            PageZoomFlag        = 0x000002,
            AnnotateFlag        = 0x000004
        };
        Q_DECLARE_FLAGS(ActionMask, ActionFlag);
        typedef struct
        {
            BindingMode bindingMode;
            PageFlow pageFlow;
            PageFlowDirection pageFlowDirection;
            PageMode pageMode;
            ZoomMode zoomMode;
            double zoom;
        } OptionState;
        typedef enum
        {
            SelectingMode,
            HighlightingMode,
            DoodlingMode
        } InteractionMode;

        DocumentView(QWidget * parent = 0);
        DocumentView(Spine::DocumentHandle document, QWidget * parent = 0);
        ~DocumentView();

        Spine::AnnotationSet activeAnnotations() const;
        Spine::AreaSet areaSelection() const;
        bool autoScrollBars() const;
        BindingMode bindingMode() const;
        void clear();
        Spine::DocumentHandle document() const;
        void hideSpotlights();
        QColor highlightColor() const;
        InteractionMode interactionMode() const;
        bool isEmpty() const;
        bool isExposing() const;
        void loadState(const OptionState & state);
        QMenu * layoutMenu() const;
        PageFlow pageFlow() const;
        PageFlowDirection pageFlowDirection() const;
        PageMode pageMode() const;
        PageView * pageView(int page) const;
        OptionState saveState() const;
        Spine::TextExtentSet search(const QString & term, int options=0);
        void setPageSlider(QAbstractSlider * slider);
        Spine::TextSelection textSelection() const;
        QList< QAction * > toolBarActions() const;
        double zoom() const;
        QMenu * zoomMenu() const;
        ZoomMode zoomMode() const;

    public slots:
        void clearSearch();
        void copySelectedText();
        void focusNextSpotlight();
        void focusPreviousSpotlight();
        void highlightSelection();
        void selectNone();
        void setAutoScrollBars(bool value);
        void setBindingMode(BindingMode mode);
        void setDocument(Spine::DocumentHandle document, int pageNumber, const QRectF & pageRect);
        void setDocument(Spine::DocumentHandle document, int pageNumber = 0, const QPointF & pagePos = QPointF()) { setDocument(document, pageNumber, QRectF(pagePos, QSizeF())); }
        void setExposing(bool exposing);
        void setHighlightColor(const QColor & color);
        void setInteractionMode(InteractionMode interactionMode);
        void setPageFlow(PageFlow flow);
        void setPageFlowDirection(PageFlowDirection direction);
        void setPageMode(PageMode mode);
        void setZoom(double zoom);
        void setZoom(int percentage);
        void setZoomMode(ZoomMode zoom);
        void showAnnotation(Spine::AnnotationHandle annotation);
        void showFirstPage();
        void showLastPage();
        void showNextPage();
        void showPage(Spine::TextExtentHandle extent);
        void showPage(int pageNumber, const QRectF & pageRect);
        void showPage(int pageNumber, const QPointF & pagePos = QPointF()) { showPage(pageNumber, QRectF(pagePos, QSize())); }
        void showPage(const QVariantMap & params = QVariantMap());
        void showPreviousPage();
        void updateAnnotations();
        void zoomIn(double delta = 0.1);
        void zoomOut(double delta = 0.1);

    signals:
        // State changes
        void bindingModeChanged();
        void pageFlowChanged();
        void pageFlowDirectionChanged();
        void pageModeChanged();
        void spotlightsHidden();
        void zoomChanged();
        void zoomModeChanged();

        // Interaction
        void annotationsActivated(Spine::AnnotationSet annotations, const QVariantMap & context);
        void contextMenuAboutToShow(QMenu * menu, Spine::DocumentHandle document, Spine::CursorHandle cursor);
        void focusChanged(PageView * pageView, const QPointF & pagePos);
        void pageFocusChanged(size_t pageNumber);
        void selectionChanged(Spine::TextSelection selection);
        void exploreSelection();
        void publishChanges();
        void urlRequested(const QUrl & url, const QString & target);

    protected:
        void contextMenuEvent(QContextMenuEvent * event);
        void focusOutEvent(QFocusEvent * event);
        void paintEvent(QPaintEvent * event);
        void resizeEvent(QResizeEvent * event);
        void wheelEvent(QWheelEvent * event);

    private:
        DocumentViewPrivate * d;
    };

}

Q_DECLARE_OPERATORS_FOR_FLAGS(Papyro::DocumentView::PageDecorations);
Q_DECLARE_OPERATORS_FOR_FLAGS(Papyro::DocumentView::ActionMask);

#endif // DOCUMENTVIEW_H
