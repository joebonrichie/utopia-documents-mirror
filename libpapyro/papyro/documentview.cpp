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

#include <papyro/documentfactory.h>
#include <papyro/documentview_p.h>
#include <papyro/documentview.h>
#include <papyro/pageview.h>
#include <papyro/utils.h>
#include <spine/spine.h>
#include <utopia2/qt/actionproxy.h>
#include <utopia2/library.h>
#include <utopia2/node.h>
#include <utopia2/parser.h>

#include <spine/TextIterator.h>
#include <boost/regex.hpp>

#include <set>

#include <QApplication>
#include <QAction>
#include <QBuffer>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QDir>
#include <QDrag>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMimeData>
#include <QPainter>
#include <QScrollBar>
#include <QSignalMapper>
#include <QStyle>
#include <QStyleOption>
#include <QTemporaryFile>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>
#include <QVector2D>

#include <QtCore/qmath.h>

#define PAGEVIEW_SHADOW_SIZE (5)
#define PAGEVIEW_SHADOW_OFFSET ((int) (PAGEVIEW_SHADOW_SIZE * 0.75))

namespace
{

    // Simple rectangular point intersection, with a grace paddings
    static inline bool contains(const Spine::BoundingBox & bb, const QPointF & point, qreal grace = 0.0)
    {
        return Spine::BoundingBox(bb.x1 - grace, bb.y1 - grace, bb.x2 + grace, bb.y2 + grace).contains(point.x(), point.y());
    }

    // Is a point bounded horizontally by bbC, but only by bbY's top coordinate? include a horizontal grace
    static inline bool containsOrBelow(const Spine::BoundingBox & bbC, const Spine::BoundingBox & bbB, const QPointF & point, qreal grace = 0.0)
    {
        return (point.x() >= (bbC.x1 - grace) && point.x() <= (bbC.x2 + grace) && point.y() >= bbB.y1);
    }

    static inline bool containsOrRightOf(const Spine::BoundingBox & bbL, const Spine::BoundingBox & bb, const QPointF & point, qreal grace = 0.0)
    {
        return (point.x() >= bb.x1 && point.y() >= (bbL.y1 - grace) && point.y() <= (bbL.y2 + grace));
    }

    static inline bool overlapHorizontally(const QRectF & rect1, const QRectF & rect2, float grace = 0.0)
    {
        return (rect1.left() <= (rect2.right() + grace) && rect1.right() >= (rect2.left() - grace));
    }

    static inline bool overlapVertically(const QRectF & rect1, const QRectF & rect2, float grace = 0.0)
    {
        return (rect1.top() <= (rect2.bottom() + grace) && rect1.bottom() >= (rect2.top() - grace));
    }

//     static QPointF qRound(const QPointF & p, int places = 0)
//     {
//         const int factor(qPow(10, places));
//         return QPointF(::qRound(p.x() * factor) / (qreal) factor,
//                        ::qRound(p.y() * factor) / (qreal) factor);
//     }

//     static QPolygonF qRound(const QPolygonF & p, int places = 0)
//     {
//         QPolygonF rounded;
//         QPointF previous;
//         foreach (const QPointF & point, p) {
//             QPointF roundedPoint(qRound(point, places));
//             if (previous.isNull() || roundedPoint != previous) {
//                 rounded << roundedPoint;
//             }
//             previous = roundedPoint;
//         }
//         return rounded;
//     }

    static Spine::CursorHandle resolveCursor(Spine::CursorHandle cursor, const QPointF & point, qreal grace = 0.0)
    {
        if (cursor && cursor->word()) {
            if (cursor->character()) { // Mouse over character
                // If more than half-way into the character, adjust to include
                if (contains(cursor->line()->boundingBox(), point, grace) &&
                    point.x() >= (cursor->character()->boundingBox().x1 + cursor->character()->boundingBox().x2) / 2.0) {
                    cursor->nextCharacter();
                }
            } else { // Mouse between characters
                float spaceLeft = cursor->word()->boundingBox().x2;
                float spaceRight = spaceLeft;
                Spine::CursorHandle nextWord(cursor->clone()); nextWord->nextWord();
                if (nextWord->word()) {
                    spaceRight = nextWord->word()->boundingBox().x1;
                }
                float spaceMid = (spaceLeft + spaceRight) / 2.0;
                // If more than half-way into the space, adjust to include
                if (point.x() >= spaceMid) {
                    cursor->nextCharacter(Spine::WithinLine);
                }
            }
        }

        return cursor;
    }

    QMap< int, QPainterPath > asPaths(const Spine::TextExtentHandle & extent)
    {
        QMap< int, QPainterPath > paths;

        // Set sentinels
        Spine::CursorHandle start = extent->first.cursor()->clone();
        Spine::CursorHandle end = extent->second.cursor()->clone();

        // Iterate over lines
        int pageNumber = 0;
        Spine::CursorHandle line = start->clone();
        Spine::CursorHandle prevLine;
        QRectF prevLineRect;
        qreal prevLineBase = 0.0;
        while (line->line() && *line <= *end) {
            // Reset previous line information if now in different block
            if (prevLine && line->block() != prevLine->block()) {
                prevLineRect = QRectF();
                prevLineBase = 0.0;
            }

            // Reset paths for each new page
            if (line->page()->pageNumber() != pageNumber) {
                pageNumber = line->page()->pageNumber();
                prevLine = line->clone();
            }

            // Get bounds of line
            Spine::BoundingBox bb = line->line()->boundingBox();
            QRectF lineRect(QRectF(bb.x1, bb.y1, bb.x2-bb.x1, bb.y2-bb.y1).adjusted(0, -0.5, 0, 0.5));
            if (!prevLineRect.isNull() &&
                overlapHorizontally(lineRect, prevLineRect) &&
                overlapVertically(lineRect, prevLineRect, lineRect.height() / 2.0)) {
                prevLineBase = prevLineRect.bottom() - 0.001;
            }

            // If this is the first or last line (or both)
            if (line->line() == start->line() || line->line() == end->line())
            {
                // Iterate over words
                Spine::CursorHandle word = line->clone();
                QRectF prevWordRect;
                while (word->word() && *word <= *end)
                {
                    // Get bounds of word
                    Spine::BoundingBox bb = word->word()->boundingBox();
                    QRectF wordRect(QRectF(bb.x1, bb.y1, bb.x2-bb.x1, bb.y2-bb.y1).adjusted(0, -0.5, 0, 0.5));

                    // If this is the first or last word (or both)
                    if (word->word() == start->word() || word->word() == end->word())
                    {
                        // Iterate over characters
                        Spine::CursorHandle character = word->clone();
                        while (*character < *end)
                        {
                            if (const Spine::Character * ch = character->character())
                            {
                                // Get bounds of character
                                Spine::BoundingBox bb = ch->boundingBox();
                                QRectF characterRect(QRectF(bb.x1, bb.y1, bb.x2-bb.x1, bb.y2-bb.y1).adjusted(0, -0.5, 0, 0.5));

                                // Pad to previous line
                                if (prevLineBase && characterRect.top() > prevLineBase)
                                {
                                    characterRect.setTop(prevLineBase);
                                }

                                if (!prevWordRect.isNull() &&
                                    *character == *word)
                                {
                                    characterRect.setLeft(prevWordRect.right());
                                }

                                paths[pageNumber].addRect(characterRect);

                                // Move on to the next character in the selection
                                character->nextCharacter();
                            }
                            else
                            {
                                // Deal with extraneous spaces
                                Spine::CursorHandle nextWord(character->clone()); nextWord->nextWord();
                                if (word->word()->spaceAfter() && nextWord->word())
                                {
                                    Spine::BoundingBox pre_bb = word->word()->boundingBox();
                                    Spine::BoundingBox post_bb = nextWord->word()->boundingBox();
                                    QRectF spaceRect(QRectF(pre_bb.x2, pre_bb.y1, post_bb.x1-pre_bb.x2, pre_bb.y2-pre_bb.y1).adjusted(0, -0.5, 0, 0.5));

                                    // Pad to previous line
                                    if (prevLineBase && spaceRect.top() > prevLineBase)
                                    {
                                        spaceRect.setTop(prevLineBase);
                                    }

                                    paths[pageNumber].addRect(spaceRect);
                                }
                                break;
                            }
                        }
                    }
                    else
                    {
                        // Otherwise add the whole word
                        if (prevLineBase && wordRect.top() > prevLineBase)
                        {
                            wordRect.setTop(prevLineBase);
                        }
                        // Deal with extraneous spaces
                        Spine::CursorHandle nextWord(word->clone()); nextWord->nextWord();
                        if (word->word()->spaceAfter() && nextWord->word())
                        {
                            wordRect.setRight(nextWord->word()->boundingBox().x1);
                        }
                        paths[pageNumber].addRect(wordRect);
                    }

                    // Move on to the next word in the selection
                    prevWordRect = wordRect;
                    word->nextWord();
                }
            }
            else
            {
                // Otherwise add the whole line
                if (prevLineBase && lineRect.top() > prevLineBase)
                {
                    lineRect.setTop(prevLineBase);
                }
                paths[pageNumber].addRect(lineRect);
            }

            // Move on to the next line in the selection
            if (prevLineRect.isNull()) {
                prevLineBase = prevLineRect.bottom();
            }
            prevLineRect = lineRect;
            prevLine = line->clone();

            line->nextLine(Spine::WithinDocument);
        }

        // Clean up paths
        QMutableMapIterator< int, QPainterPath > iter(paths);
        while (iter.hasNext()) {
            iter.next();
            iter.value().setFillRule(Qt::WindingFill);
            iter.value() = iter.value().simplified();
        }
        return paths;
    }

    static QMap< int, QPainterPath > asPaths(const Spine::TextSelection & selection)
    {
        QMap< int, QPainterPath > paths;
        foreach (const Spine::TextExtentHandle & extent, selection) {
            QMap< int, QPainterPath > newPaths(asPaths(extent));
            QMapIterator< int, QPainterPath > iter(newPaths);
            while (iter.hasNext()) {
                iter.next();
                paths[iter.key()].addPath(iter.value());
            }
        }

        // Clean up paths
        QMutableMapIterator< int, QPainterPath > iter(paths);
        while (iter.hasNext()) {
            iter.next();
            iter.value().setFillRule(Qt::WindingFill);
            iter.value() = iter.value().simplified();
        }
        return paths;
    }

} // Anonymous namespace




namespace Papyro
{

    static QRectF bound(PageView * pageView, const QRectF & rect)
    {
        return QRectF(QPointF(0, 0), pageView->pageSize()).intersected(rect);
    }







    PageViewOverlay::PageViewOverlay()
    {}




    /// PageViewMouseEvent ///////////////////////////////////////////////////////////////////////

    PageViewMouseEvent::PageViewMouseEvent()
        : pageView(0), cardinality(0)
    {}

    PageViewMouseEvent::PageViewMouseEvent(PageView * pageView, QMouseEvent * event, const QPoint & pos, int cardinality)
        : pageView(pageView), pos(pos), pagePos(pageView->transformToPage(pos)), button(event->button()), buttons(event->buttons()), modifiers(event->modifiers()), cardinality(cardinality)
    {}

    PageViewMouseEvent::PageViewMouseEvent(PageView * pageView, const QPoint & pos, Qt::MouseButton button, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, int cardinality)
        : pageView(pageView), pos(pos), pagePos(pageView->transformToPage(pos)), button(button), buttons(buttons), modifiers(modifiers), cardinality(cardinality)
    {}

    QPoint PageViewMouseEvent::globalPos() const
    {
        return pageView->mapToGlobal(pos);
    }

    PageViewMouseEvent::operator bool() const
    {
        return pageView;
    }

    /// DocumentViewPrivate ///////////////////////////////////////////////////////////////////////

    DocumentViewPrivate::DocumentViewPrivate(DocumentView * documentView)
        : QObject(documentView), documentView(documentView)
    {
        interaction.mode = DocumentView::SelectingMode;
        interaction.isPressed = false;
        interaction.isDragging = false;
        interaction.numberOfClicks = 0;
        interaction.numberOfClickTimeouts = 0;
        interaction.numberOfHolds = 0;
        interaction.numberOfHoldTimeouts = 0;
        interaction.holdInterval = 500;
        interaction.updateOrigin = true;
        interaction.isExposing = false;
        selection.color = Qt::yellow;

        connect(this, SIGNAL(zoomChanged()), documentView, SIGNAL(zoomChanged()));
    }

    void DocumentViewPrivate::addInteractionState(InteractionState state)
    {
        if (state != IdleState) {
            interaction.states << state;
        }
    }

    void DocumentViewPrivate::applyActiveAreaSelection()
    {
        // Add the currently active area to the document's selection
        if (selection.activeAreaFromPoint != selection.activeAreaToPoint) {
            QRectF rect = bound(selection.activeAreaPageView, QRectF(selection.activeAreaFromPoint, selection.activeAreaToPoint).normalized());
            Spine::Area area(selection.activeAreaPageView->pageNumber(), 0, Spine::BoundingBox(rect.left(), rect.top(), rect.right(), rect.bottom()));
            if (interaction.mode == DocumentView::SelectingMode) {
                document->addToAreaSelection(area);
            } else {
                createHighlight(&area, Spine::TextExtentHandle(), true, true);
            }
        }
        selection.activeAreaPageView = 0;
        selection.activeAreaFromPoint = QPointF();
        selection.activeAreaToPoint = QPointF();
        updateActiveAreaSelection();
    }

    void DocumentViewPrivate::applyActiveTextSelection()
    {
        // Add the currently active text extent to the document's selection
        if (selection.activeTextFromCursor && selection.activeTextToCursor && *selection.activeTextFromCursor != *selection.activeTextToCursor) {
            Spine::CursorHandle from(selection.activeTextFromCursor);
            Spine::CursorHandle to(selection.activeTextToCursor);
            Spine::order(from, to);
            Spine::TextExtentHandle extent(new Spine::TextExtent(from, to));
            if (interaction.mode == DocumentView::SelectingMode) {
                document->addToTextSelection(extent);
                QString selectedText(qStringFromUnicode(extent->text()));
                QApplication::clipboard()->setText(selectedText, QClipboard::Selection);
            } else {
                createHighlight(0, extent, true, true);
            }
        }
        selection.activeTextFromCursor = Spine::CursorHandle();
        selection.activeTextToCursor = Spine::CursorHandle();
        updateActiveTextSelection();
    }

    void DocumentViewPrivate::clearInteractionStates()
    {
        setInteractionState(IdleState);
    }

    void DocumentViewPrivate::clearPageViews()
    {
        QVector< PageView * > doomed(pageViews);
        pageViews.clear();
        pageViewOverlays.clear();
        foreach (PageView * pageView, doomed) {
            pageView->clear();
            pageView->hide();
            pageView->deleteLater();
        }
    }

    Spine::AnnotationHandle DocumentViewPrivate::createHighlight(const Spine::Area * area, Spine::TextExtentHandle extent, bool store, bool persist)
    {
        Spine::AnnotationHandle annotation(new Spine::Annotation);
        annotation->setProperty("concept", "Highlight");
        annotation->setProperty("property:color", unicodeFromQString(selection.color.name()));
        if (area) {
            annotation->addArea(*area);
        } else if (extent) {
            annotation->addExtent(extent);
        } else {
            // No anchor, bail
            return Spine::AnnotationHandle();
        }

        if (persist) {
            document->addAnnotation(annotation, "PersistQueue");
        } else if (store) {
            document->addAnnotation(annotation);
        }

        return annotation;
    }

    void DocumentViewPrivate::createPageViews()
    {
        if (!document) { return; }

        // Create one PageView for each page in the document
        for (size_t i = 0; i < document->numberOfPages(); ++i) {
            PageView * pageView = new PageView(document, i + 1, documentView->viewport());
            pageView->setZoom(0.2);
            pageView->setMouseTracking(true);
            pageView->installEventFilter(this);
            QObject::connect(pageView, SIGNAL(exploreSelection()), documentView, SIGNAL(exploreSelection()));
            QObject::connect(pageView, SIGNAL(publishChanges()), documentView, SIGNAL(publishChanges()));
            QObject::connect(pageView, SIGNAL(urlRequested(const QUrl &, const QString &)), documentView, SIGNAL(urlRequested(const QUrl &, const QString &)));
            QObject::connect(pageView, SIGNAL(pageRotated()), this, SLOT(update_layout()));

            // Add overlay widget
            QHBoxLayout * pageViewLayout = new QHBoxLayout(pageView);
            pageViewLayout->setSpacing(0);
            pageViewLayout->setContentsMargins(0, 0, 0, 0);
            QWidget * pageViewOverlayWidget = new QWidget;
            pageViewLayout->addWidget(pageViewOverlayWidget);
            pageViewOverlayWidget->setMouseTracking(true);
            pageViewOverlayWidget->installEventFilter(this);

            pageViews.append(pageView);
            pageViewOverlays[pageView].widget = pageViewOverlayWidget;
        }

        update_layout(GridChange);
        documentView->update();
    }

    bool DocumentViewPrivate::eventFilter(QObject * obj, QEvent * e)
    {
        //qDebug() << obj << e;

        // This next section of code decides exactly what has been clicked on, and
        // modifies the mouse position accordingly
        QWidget * widget = qobject_cast< QWidget * >(obj);
        PageView * pageView = 0;
        if (widget) { pageView = qobject_cast< PageView * >(widget); }
        QMouseEvent * me = dynamic_cast< QMouseEvent * >(e);
        QContextMenuEvent * cme = dynamic_cast< QContextMenuEvent * >(e);
        QPoint mePos;
        if (me || cme) {
            mePos = me ? me->pos() : cme->pos();
            if (pageView) { // Check to see if the mouse is grabbed
                QPoint viewportPos(widget->mapTo(documentView->viewport(), mePos));
                QRect viewportGeometry(widget->mapTo(documentView->viewport(), QPoint(0, 0)), widget->size());
                if (!viewportGeometry.contains(viewportPos)) {
                    // Find the page view in which the mouse falls
                    QWidget * underMouse = documentView->viewport()->childAt(viewportPos);
                    if (underMouse) {
                        if (PageView * hoverPageView = qobject_cast< PageView * >(underMouse->parent())) {
                            pageView = hoverPageView;
                            mePos = pageView->mapFrom(documentView->viewport(), viewportPos);
                        }
                    }
                }
            }
        }
        PageView * overlayedPageView = 0;
        if (widget && !pageView) { overlayedPageView = qobject_cast< PageView * >(widget->parent()); }
        QWidget * overlayWidget = 0;
        if (overlayedPageView) { overlayWidget = widget; }

        if (pageView) {
            // Hide spotlights if need be
            if ((e->type() == QEvent::MouseButtonPress || e->type() == QEvent::ContextMenu || e->type() == QEvent::MouseButtonRelease)) {
                documentView->hideSpotlights();
            }

            // Catch context menu events deal with them locally
            if (e->type() == QEvent::ContextMenu && pageView) {
                // Ensure annotations under the mouse are correct
                updateAnnotationsUnderMouse(pageView, pageView->transformToPage(cme->pos()));

                e->ignore();
                return false;
            }

            // Emit some useful signals
            if (e->type() == QEvent::MouseButtonPress && pageView) {
                emit pageFocusChanged(pageView->pageNumber());
            } else if (e->type() == QEvent::MouseButtonDblClick && pageView) {
                QPointF pagePos = pageView->transformToPage(mePos);
                emit focusChanged(pageView, pagePos);
            }

            // Now, intercept events and deal with them to provide user interaction of
            // text/area/image selections and annotation editing
            switch (e->type()) {
            case QEvent::MouseButtonPress:
                if (interaction.numberOfClicks == 1) break;
            case QEvent::MouseButtonDblClick: {
                // This may signify a multi-click, so let us remember this mouse event
                // and only deal with it once the double click interval is up.
                interaction.isPressed = true;
                interaction.isDragging = false;
                interaction.numberOfClicks += 1;
                interaction.numberOfHolds += 1;
                QTimer::singleShot(QApplication::doubleClickInterval(),
                                   this, SLOT(onWaitingForDblClickTimeout()));
                QTimer::singleShot(interaction.holdInterval,
                                   this, SLOT(onWaitingForHoldTimeout()));
                interaction.previousPressEvent = PageViewMouseEvent(pageView, me, mePos, interaction.numberOfClicks);
                mousePress(&interaction.previousPressEvent);
                break;
            }
            case QEvent::MouseMove: {
                // If we're waiting for a double click, forget it, and send the press
                // event through first ???
                PageViewMouseEvent pvme(pageView, me, mePos, interaction.numberOfClicks);
                mouseMove(&pvme);
                if (interaction.isPressed) {
                    if (interaction.isDragging || (interaction.previousPressEvent.pos - mePos).manhattanLength() > QApplication::startDragDistance()) {
                        interaction.isDragging = true;
                        mouseDrag(&pvme);
                    }
                }
                break;
            }
            case QEvent::MouseButtonRelease: {
                interaction.isPressed = false;
                interaction.previousReleasePos = mePos;
                PageViewMouseEvent pvme(pageView, me, mePos, interaction.previousPressEvent.cardinality);
                mouseRelease(&pvme);
                if (interaction.pendingClickEvent) {
                    if ((interaction.previousPressEvent.pos - interaction.previousReleasePos).manhattanLength() < QApplication::startDragDistance()) {
                        mouseClick(&interaction.pendingClickEvent);
                    }
                    interaction.pendingClickEvent = PageViewMouseEvent();
                }
                break;
            }
            default:
                break;
            }
        } else if (overlayWidget) {
            switch (e->type()) {
            case QEvent::Paint: {
                // Draw this page view's overlay
                QPainter painter(overlayWidget);

                // Scale to current zoom
                QSizeF pSize = overlayedPageView->pageSize(true);
                int page = overlayedPageView->pageNumber();
                //painter.translate(-0.5, -0.5);
                painter.scale(overlayWidget->width() / (double) pSize.width(),
                              overlayWidget->height() / (double) pSize.height());
                painter.setTransform(overlayedPageView->userTransform(), true);

                // Antialias
                painter.setRenderHint(QPainter::Antialiasing, true);
                painter.setRenderHint(QPainter::TextAntialiasing, true);

                // Draw this page's overlay
                PageViewOverlay & overlay = pageViewOverlays[overlayedPageView];

                // === SELECTION ===
                painter.save();
                painter.setCompositionMode(QPainter::CompositionMode_Multiply);
                QColor color(selectionColor());
                painter.setBrush(color);
                painter.setPen(QPen(color, 0));
                painter.drawPath(overlay.selectionHighlight);
                painter.restore();

                // === ANNOTATIONS ===
                if (interaction.isExposing) {
                    QMapIterator< Spine::AnnotationHandle, QMap< int, QPicture > > h_iter(rendering.hoverPictures);
                    while (h_iter.hasNext()) {
                        h_iter.next();
                        painter.save();
                        painter.drawPicture(0, 0, h_iter.value()[page]);
                        painter.restore();
                    }
                } else {
                    QMapIterator< OverlayRenderer *, QMap< OverlayRenderer::State, QPair< Spine::AnnotationSet, QMap< int, QPicture > > > > r_iter(rendering.pictures);
                    while (r_iter.hasNext()) {
                        r_iter.next();
                        QMapIterator< OverlayRenderer::State, QPair< Spine::AnnotationSet, QMap< int, QPicture > > > s_iter(r_iter.value());
                        while (s_iter.hasNext()) {
                            s_iter.next();
                            painter.save();
                            painter.drawPicture(0, 0, s_iter.value().second[page]);
                            painter.restore();
                        }
                    }
                }

                return true;
            }
            default:
                break;
            }
        }

        return QObject::eventFilter(obj, e);
    }

    void DocumentViewPrivate::initialise()
    {
        // Set up scroll area
        documentView->viewport()->setObjectName("scrollwidget");
        documentView->setFrameShape(QFrame::NoFrame);
        documentView->horizontalScrollBar()->setSingleStep(20);
        documentView->verticalScrollBar()->setSingleStep(20);
        connect(documentView->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(onHorizontalScrollBarValueChanged(int)));
        connect(documentView->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(onVerticalScrollBarValueChanged(int)));

        // Pass-through signals
        connect(this, SIGNAL(annotationsActivated(Spine::AnnotationSet, const QVariantMap &)), documentView, SIGNAL(annotationsActivated(Spine::AnnotationSet, const QVariantMap &)));
        connect(this, SIGNAL(focusChanged(PageView *, const QPointF &)), documentView, SIGNAL(focusChanged(PageView *, const QPointF &)));
        connect(this, SIGNAL(pageFocusChanged(size_t)), documentView, SIGNAL(pageFocusChanged(size_t)));

        pageNumber = 1;
        zoom = 1.0;
        actionBespokeZoom = 0;
        autoScrollBars = true;

        // Actions
        {
            // Layout menu
            layoutMenu = new QMenu("Layout", documentView);
            {
                QActionGroup * group = new QActionGroup(this);

                actionOnePage = new QAction(QIcon(":/icons/one-up.png"), "One Page", this);
                actionOnePage->setCheckable(true);
                group->addAction(actionOnePage);
                actionOnePage->setChecked(true);
                layoutMenu->addAction(actionOnePage);
                QObject::connect(actionOnePage, SIGNAL(triggered()), this, SLOT(onOnePage()));

                actionOnePageContinuous = new QAction("One Page Continuous", this);
                actionOnePageContinuous->setCheckable(true);
                group->addAction(actionOnePageContinuous);
                layoutMenu->addAction(actionOnePageContinuous);
                QObject::connect(actionOnePageContinuous, SIGNAL(triggered()), this, SLOT(onOnePageContinuous()));

                actionTwoPages = new QAction(QIcon(":/icons/two-up.png"), "Two Pages", this);
                actionTwoPages->setCheckable(true);
                group->addAction(actionTwoPages);
                layoutMenu->addAction(actionTwoPages);
                QObject::connect(actionTwoPages, SIGNAL(triggered()), this, SLOT(onTwoPages()));

                actionTwoPagesContinuous = new QAction("Two Pages Continuous", this);
                actionTwoPagesContinuous->setCheckable(true);
                group->addAction(actionTwoPagesContinuous);
                layoutMenu->addAction(actionTwoPagesContinuous);
                QObject::connect(actionTwoPagesContinuous, SIGNAL(triggered()), this, SLOT(onTwoPagesContinuous()));
            }
            layoutMenu->addSeparator();
            {
                groupDirection = new QActionGroup(this);

                actionTopDownFlow = new QAction("Top to Bottom", this);
                actionTopDownFlow->setCheckable(true);
                groupDirection->addAction(actionTopDownFlow);
                actionTopDownFlow->setChecked(true);
                layoutMenu->addAction(actionTopDownFlow);
                QObject::connect(actionTopDownFlow, SIGNAL(triggered()), this, SLOT(onTopDownFlow()));

                actionLeftToRightFlow = new QAction("Left to Right", this);
                actionLeftToRightFlow->setCheckable(true);
                groupDirection->addAction(actionLeftToRightFlow);
                layoutMenu->addAction(actionLeftToRightFlow);
                QObject::connect(actionLeftToRightFlow, SIGNAL(triggered()), this, SLOT(onLeftToRightFlow()));

                groupDirection->setEnabled(false);
            }
            layoutMenu->addSeparator();
            {
                actionFirstPageOdd = new QAction("Treat First Page As Odd", this);
                actionFirstPageOdd->setCheckable(true);
                actionFirstPageOdd->setChecked(true);
                actionFirstPageOdd->setEnabled(false);
                layoutMenu->addAction(actionFirstPageOdd);
                QObject::connect(actionFirstPageOdd, SIGNAL(toggled(bool)), this, SLOT(toggleFirstPageOdd(bool)));
            }
        }
        {
            zoomActionGroup = new QActionGroup(this);

            // Zoom menu
            zoomMenu = new QMenu("Zoom", documentView);

            actionFitToWidth = new QAction(QIcon(":/icons/fit-to-width.png"), "Fit to Width", this);
            actionFitToWidth->setCheckable(true);
            actionFitToWidth->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_1));
            zoomActionGroup->addAction(actionFitToWidth);
            actionFitToWidth->setChecked(true);
            zoomMenu->addAction(actionFitToWidth);
            QObject::connect(actionFitToWidth, SIGNAL(triggered()), this, SLOT(onFitToWidth()));

            actionFitToHeight = new QAction("Fit to Height", this);
            actionFitToHeight->setCheckable(true);
            actionFitToHeight->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_2));
            zoomActionGroup->addAction(actionFitToHeight);
            zoomMenu->addAction(actionFitToHeight);
            QObject::connect(actionFitToHeight, SIGNAL(triggered()), this, SLOT(onFitToHeight()));

            actionFitToWindow = new QAction(QIcon(":/icons/fit-to-window.png"), "Fit to Window", this);
            actionFitToWindow->setCheckable(true);
            actionFitToWindow->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_0));
            zoomActionGroup->addAction(actionFitToWindow);
            zoomMenu->addAction(actionFitToWindow);
            QObject::connect(actionFitToWindow, SIGNAL(triggered()), this, SLOT(onFitToWindow()));

            zoomMenu->addSeparator();

            actionZoomIn = new QAction("Zoom In", this);
            QList< QKeySequence > zoomInShortcuts;
            zoomInShortcuts << QKeySequence(QString("Ctrl+="));
            zoomInShortcuts << QKeySequence::ZoomIn;
            actionZoomIn->setShortcuts(zoomInShortcuts);
            zoomMenu->addAction(actionZoomIn);
            QObject::connect(actionZoomIn, SIGNAL(triggered()), documentView, SLOT(zoomIn()));

            actionZoomOut = new QAction("Zoom Out", this);
            actionZoomOut->setShortcut(QKeySequence::ZoomOut);
            zoomMenu->addAction(actionZoomOut);
            QObject::connect(actionZoomOut, SIGNAL(triggered()), documentView, SLOT(zoomOut()));

            zoomMenu->addSeparator();

            zoomSignalMapper = new QSignalMapper(this);
            int zoomPercentageValues[] = { 25, 50, 75, 100, 125, 150, 200, 0 };
            int * zoomPercentage = zoomPercentageValues;
            while (*zoomPercentage) {
                int percentage = *zoomPercentage;
                QString str = QString("%1").arg(percentage);
                QAction * action = zoomMenu->addAction(str + "%");
                action->setCheckable(true);
                zoomActionGroup->addAction(action);
                zoomPercentages[percentage] = action;
                QObject::connect(action, SIGNAL(triggered()), zoomSignalMapper, SLOT(map()));
                zoomSignalMapper->setMapping(action, percentage);
                ++zoomPercentage;
            }
            QObject::connect(zoomSignalMapper, SIGNAL(mapped(int)), documentView, SLOT(setZoom(int)));
        }

        // Set up display mode
        bindingMode = DocumentView::Odd;
        pageFlow = DocumentView::Separate;
        pageFlowDirection = DocumentView::TopDown;
        pageMode = DocumentView::OneUp;
        zoomMode = DocumentView::FitToWidth;
        updateScrollBarPolicies();
        documentView->setBindingMode(DocumentView::Odd);
        documentView->setPageFlow(DocumentView::Separate);
        documentView->setPageFlowDirection(DocumentView::TopDown);
        documentView->setPageMode(DocumentView::OneUp);
        documentView->setZoomMode(DocumentView::FitToWidth);

        // Collect all renderers and renderer mappers
        {
            foreach (OverlayRenderer * renderer, Utopia::instantiateAllExtensions< OverlayRenderer >()) {
                overlayRenderers.insertMulti(renderer->id(), renderer);
            }
            // Add system overlay renderers
            OverlayRenderer * renderer = new NoOverlayRenderer;
            overlayRenderers.insertMulti(renderer->id(), renderer);
        }
        {
            QMap< int, QList< OverlayRendererMapper * > > rendererMapperCache;
            foreach (OverlayRendererMapper * mapper, Utopia::instantiateAllExtensions< OverlayRendererMapper >()) {
                rendererMapperCache[mapper->weight()] << mapper;
            }
            QMapIterator< int, QList< OverlayRendererMapper * > > iter(rendererMapperCache);
            iter.toBack();
            while (iter.hasPrevious()) {
                iter.previous();
                overlayRendererMappers += iter.value();
            }
        }
    }

    DocumentViewPrivate::InteractionState DocumentViewPrivate::interactionState() const
    {
        if (interaction.states.size() == 0) {
            return IdleState;
        } else if (interaction.states.size() == 1) {
            return interaction.states.first();
        } else {
            return AmbiguousState;
        }
    }

    bool DocumentViewPrivate::isMouseOverImage()
    {
        return (interaction.mouseTextCursor && interaction.mouseTextCursor->image());
    }

    bool DocumentViewPrivate::isMouseOverText()
    {
        return (interaction.mouseTextCursor && interaction.mouseTextCursor->line());
    }

    void DocumentViewPrivate::mouseClick(PageViewMouseEvent * event)
    {
        static QStringList ignore;
        if (ignore.isEmpty()) {
            ignore << "Highlight";
        } // FIXME how to deal with this in a generic way

        //qDebug() << "mouseClick" << event << SelectingStaticTextState << interactionState();
        switch (interaction.mode) {
        case DocumentView::SelectingMode:
            if (current.annotation && !ignore.contains(qStringFromUnicode(current.annotation->getFirstProperty("concept")))) {
                setInteractionState(ActivatingAnnotationState);
                QVariantMap context;
                context["pos"] = interaction.previousPressEvent.pagePos;
                context["page"] = event->pageView->pageNumber();
                emit annotationsActivated(current.annotations, context);
                setInteractionState(IdleState);
                break;
            }
        case DocumentView::HighlightingMode: {
            switch (interactionState()) {
            case SelectingStaticTextState:
                applyActiveTextSelection();
                setInteractionState(IdleState);
                break;
            case SelectingImageState:
                setInteractionState(IdleState);
                break;
            default:
                break;
            }
            break;
        }
        default:
            break;
        }

        //qDebug() << ">>>> mouseClick" << event->cardinality;
    }

    void DocumentViewPrivate::mouseDrag(PageViewMouseEvent * event)
    {
        switch (interaction.mode) {
        case DocumentView::SelectingMode: {
            switch (interactionState()) {
            case ActivatingSelectionState:
            case SelectingImageState: {
                QString selectionText;
                Spine::Area selectionArea;

                if (interactionState() == ActivatingSelectionState) {
                    // For dragging text
                    selectionText = Papyro::qStringFromUnicode(document->selectionText());
                    // For dragging a single rectangular area
                    Spine::AreaSet areaSelection = document->areaSelection();
                    if (areaSelection.size() == 1) {
                        selectionArea = *areaSelection.begin();
                    }
                }

                if (interactionState() == ActivatingSelectionState && !selectionText.isEmpty()) {
                    setInteractionState(DraggingSelectionState);
                    QDrag * drag = new QDrag(documentView);
                    QMimeData * mimeData = new QMimeData;
                    mimeData->setText(selectionText);
                    drag->setMimeData(mimeData);
                    //Qt::DropAction dropAction = drag->exec(Qt::CopyAction);
                    interaction.isDragging = false;
                    interaction.isPressed = false;
                    setInteractionState(IdleState);
                } else if (interactionState() == SelectingImageState || selectionArea.boundingBox.width() > 0) {
                    setInteractionState(interactionState() == SelectingImageState ? DraggingImageState : DraggingSelectionState);
                    QImage imageToCopy;
                    Spine::BoundingBox boundingBoxToCopy;
                    QDrag * drag = new QDrag(documentView);
                    QMimeData * mimeData = new QMimeData;
                    PageView * pageView = 0;

                    if (interactionState() == DraggingImageState) {
                        pageView = event->pageView;
                        boundingBoxToCopy = interaction.mouseTextCursor->image()->boundingBox();
                        imageToCopy = qImageFromSpineImage(interaction.mouseTextCursor->image());
                    } else {
                        pageView = pageViews.at(selectionArea.page-1);
                        boundingBoxToCopy = selectionArea.boundingBox;
                        Spine::Image tmpImage = pageView->page()->renderArea(boundingBoxToCopy, double(150.0));
                        imageToCopy = qImageFromSpineImage(&tmpImage);
                    }

                    QByteArray ba;
                    QBuffer buffer(&ba);
                    buffer.open(QIODevice::WriteOnly);
                    QTemporaryFile * tempFile = new QTemporaryFile(QDir::tempPath() + "/utopia_img_XXXXXX.png", this);
                    tempFile->open();
                    imageToCopy.save(&buffer, "PNG");
                    tempFile->write(ba);
                    tempFile->close();
                    buffer.close();
                    mimeData->setData("image/png", ba);
                    mimeData->setImageData(imageToCopy);
                    mimeData->setUrls(QList< QUrl >() << QUrl::fromLocalFile(tempFile->fileName()));
                    QSize imageImprintSize(pageView->transformFromPage(QSizeF(boundingBoxToCopy.width(), boundingBoxToCopy.height())));
                    QPoint imageImprintPos(pageView->transformFromPage(QPointF(boundingBoxToCopy.x1, boundingBoxToCopy.y1)));
                    Spine::Image spineImage = pageView->page()->renderArea(boundingBoxToCopy, size_t(imageImprintSize.width()), size_t(imageImprintSize.height()));
                    QPixmap capturedPixmap = QPixmap::fromImage(qImageFromSpineImage(&spineImage));
                    QPixmap pixmap(capturedPixmap.size());
                    pixmap.fill(QColor(0, 0, 0, 0));
                    QPainter painter(&pixmap);
                    painter.setOpacity(0.4);
                    painter.drawPixmap(0, 0, capturedPixmap);
                    painter.setOpacity(1.0);
                    painter.setPen(QColor(0, 0, 0, 100));
                    painter.setBrush(Qt::NoBrush);
                    painter.drawRect(pixmap.rect().adjusted(0, 0, -1, -1));
                    painter.end();
                    drag->setPixmap(pixmap);
                    drag->setHotSpot(event->pos - imageImprintPos);
                    drag->setMimeData(mimeData);
                    //Qt::DropAction dropAction = drag->exec(Qt::CopyAction);
                    interaction.isDragging = false;
                    interaction.isPressed = false;
                    setInteractionState(IdleState);
                }
                break;
            }
            case SelectingStaticTextState:
                applyActiveTextSelection();
                setInteractionState(IdleState);
                break;
            default:
                break;
            }
            break;
        }
        default:
            break;
        }

        //qDebug() << ">>>> mouseDrag" << event->cardinality;
    }

    void DocumentViewPrivate::mouseHold(PageViewMouseEvent * event)
    {
        switch (interaction.mode) {
        case DocumentView::SelectingMode: {
            if (interactionState() == SelectingAreaState && event->cardinality == 1 && selection.activeAreaToPoint == selection.activeAreaFromPoint) {
                if (isMouseOverImage()) {
                    setInteractionState(SelectingImageState);
                }
            }
            break;
        }
        default:
            break;
        }
        //qDebug() << ">>>> mouseHold" << event->cardinality;
    }

    void DocumentViewPrivate::mouseMove(PageViewMouseEvent * event)
    {
        switch (interaction.mode) {
        case DocumentView::SelectingMode:
        case DocumentView::HighlightingMode: {
            switch (interactionState()) {
            case IdleState: {
                // Decide what cursor to show according to what is under the mouse
                interaction.mouseTextCursor = textCursorAt(event->pageView, event->pagePos);
                updateAnnotationsUnderMouse(event->pageView, event->pagePos);
                QCursor suggestedCursor;
                if (current.annotation && rendering.bounds.contains(current.annotation)) {
                    suggestedCursor = rendering.bounds[current.annotation].first->cursor();
                }
                if (suggestedCursor.shape() != QCursor().shape()) {
                    documentView->setCursor(suggestedCursor);
                } else if (isMouseOverText()) {
                    documentView->setCursor(Qt::IBeamCursor);
                } else {
                    documentView->setCursor(Qt::ArrowCursor);
                }
                break;
            }
            case SelectingAreaState:
                if (event->pageView && selection.activeAreaPageView) {
                    documentView->setCursor(Qt::CrossCursor);
                    // Transform the current position into the coordinate system of the initiating page view
                    selection.activeAreaToPoint = selection.activeAreaPageView->transformToPage(selection.activeAreaPageView->mapFrom(documentView->viewport(), event->pageView->mapTo(documentView->viewport(), event->pos)));
                    updateActiveAreaSelection();
                }
                break;
            case SelectingTextState:
                selection.activeTextToCursor = textCursorAt(event->pageView, event->pagePos);
                updateActiveTextSelection();
                break;
            default:
                break;
            }
            break;
        }
        default:
            break;
        }
        //qDebug() << ">>>> mouseMove";
    }

    void DocumentViewPrivate::mousePress(PageViewMouseEvent * event)
    {
        static QRegExp letters("\\w+");

        //qDebug() << "mousePress" << event->cardinality << interactionState();

        // If the default mode is automatic, work out what to do according to what
        // is under the mouse and which buttons are pressed
        switch (interaction.mode) {
        case DocumentView::SelectingMode:
        case DocumentView::HighlightingMode: {
            bool automaticMode = interaction.mode == DocumentView::SelectingMode;
            if (event->button == Qt::LeftButton) {
                updateAnnotationsUnderMouse(event->pageView, event->pagePos);
                // First check to see if the mouse is over the current selection
                if (automaticMode && event->cardinality == 1 && pageViewOverlays.value(event->pageView).selectionHighlight.contains(event->pagePos)) {
                    setInteractionState(ActivatingSelectionState);
                } else if (automaticMode && isMouseOverImage() && event->cardinality > 1) {
                    setInteractionState(SelectingImageState);
                } else if (isMouseOverText()) {
                    //qDebug() << "mousePress" << event->cardinality;
                    if (event->cardinality > 1) {
                        setInteractionState(SelectingStaticTextState);
                    } else {
                        setInteractionState(SelectingTextState);
                    }
                } else {
                    setInteractionState(SelectingAreaState);
                }
            }
            break;
        }
        case DocumentView::DoodlingMode:
            break;
        default:
            break;
        }

        // If selecting, clear any current selection if the CTRL modifier is absent
        if ((interactionState() & SelectingFlag) && (event->modifiers & Qt::CTRL) == 0 && event->cardinality == 1) {
            document->clearSelection();
        }

        // Depending on the interaction state, do different things
        switch (interactionState()) {
        case SelectingAreaState:
            // Set up the state required to deal with an area selection
            selection.activeAreaPageView = event->pageView;
            selection.activeAreaFromPoint = event->pagePos;
            selection.activeAreaToPoint = selection.activeAreaFromPoint;
            updateActiveAreaSelection();
            break;
        case SelectingImageState:
            // FIXME remember image
            break;
        case SelectingStaticTextState:
        case SelectingTextState:
            // Depending on the cardinality, do different things
            switch (event->cardinality) {
            case 1:
                // Set up the state required to deal with a text selection
                selection.activeTextFromCursor = textCursorAt(event->pageView, event->pagePos);
                selection.activeTextToCursor = selection.activeTextFromCursor->clone();
                updateActiveTextSelection();
                break;
            case 2:
                // Select a whole world
                selection.activeTextFromCursor = textCursorAt(event->pageView, event->pagePos);
                selection.activeTextToCursor = selection.activeTextFromCursor->clone();
                while (const Spine::Character * previous = selection.activeTextFromCursor->peekPreviousCharacter()) {
                    if (letters.exactMatch(qStringFromUnicode(previous->text()))) {
                        selection.activeTextFromCursor->previousCharacter();
                    } else {
                        break;
                    }
                }
                while (const Spine::Character * next = selection.activeTextToCursor->nextCharacter()) {
                    if (!letters.exactMatch(qStringFromUnicode(next->text()))) {
                        break;
                    }
                }
                updateActiveTextSelection();
                break;
            case 3:
                // Select a whole line
                selection.activeTextFromCursor = textCursorAt(event->pageView, event->pagePos);
                selection.activeTextToCursor = selection.activeTextFromCursor->clone();
                selection.activeTextFromCursor->toFrontWord();
                selection.activeTextToCursor->toBackWord();
                selection.activeTextToCursor->previousWord();
                selection.activeTextToCursor->toBackCharacter();
                updateActiveTextSelection();
                break;
            case 4:
                // Select a whole block
                selection.activeTextFromCursor = textCursorAt(event->pageView, event->pagePos);
                selection.activeTextToCursor = selection.activeTextFromCursor->clone();
                selection.activeTextFromCursor->toFrontLine();
                selection.activeTextToCursor->toBackLine();
                selection.activeTextToCursor->previousLine();
                selection.activeTextToCursor->toBackWord();
                selection.activeTextToCursor->previousWord();
                selection.activeTextToCursor->toBackCharacter();
                updateActiveTextSelection();
                break;
            default:
                break;
            }
        default:
            break;
        }

        //qDebug() << ">>>> mousePress" << event->cardinality;
    }

    void DocumentViewPrivate::mouseRelease(PageViewMouseEvent * event)
    {
        //qDebug() << "mouseRelease" << event->cardinality << interactionState();
        switch (interaction.mode) {
        case DocumentView::SelectingMode:
        case DocumentView::HighlightingMode: {
            switch (interactionState()) {
            case SelectingAreaState:
                applyActiveAreaSelection();
                break;
            case SelectingTextState:
                applyActiveTextSelection();
                break;
            case SelectingImageState:
            case SelectingStaticTextState:
                return; // Do not return to the IdleState, and await the click event
                break;
            case ActivatingAnnotationState:
                updateAnnotationsUnderMouse(event->pageView, event->pagePos);
                if (!current.annotations.empty()) {
                    QVariantMap context;
                    context["pos"] = interaction.previousPressEvent.pagePos;
                    context["page"] = event->pageView->pageNumber();
                    emit annotationsActivated(current.annotations, context);
                }
                break;
            case ActivatingSelectionState:
                document->clearSelection();
                break;
            default:
                break;
            }

            setInteractionState(IdleState);
            break;
        }
        default:
            break;
        }

        //qDebug() << ">>>> mouseRelease" << event->cardinality;
    }

    void DocumentViewPrivate::onDocumentAnnotationsChanged(const std::string & name, const Spine::AnnotationSet & annotations, bool added)
    {
        if (document) {
            if (name.empty()) {
                // To keep track of lists that need recomputing
                QSet< QPair< OverlayRenderer *, OverlayRenderer::State > > dirty;

                // Make sure the annotations are rendered properly
                if (added) {
                    foreach (Spine::AnnotationHandle annotation, annotations) {
                        OverlayRenderer * overlayRenderer = 0;
                        foreach (OverlayRendererMapper * candidate, overlayRendererMappers) {
                            QString rendererId = candidate->mapToId(document, annotation);
                            if (!rendererId.isEmpty() && overlayRenderers.contains(rendererId)) {
                                overlayRenderer = overlayRenderers[rendererId];
                                break;
                            }
                        }
                        if (overlayRenderer == 0) {
                            overlayRenderer = &rendering.defaultOverlayRenderer;
                        }

                        rendering.bounds[annotation] = qMakePair(overlayRenderer, overlayRenderer->bounds(document, annotation));
                        rendering.hoverPictures[annotation] = overlayRenderer->render(document, annotation, OverlayRenderer::Hover);
                        if (rendering.pictures[overlayRenderer][OverlayRenderer::Idle].first.insert(annotation).second) {
                            dirty << qMakePair(overlayRenderer, OverlayRenderer::Idle);
                        }
                    }
                } else {
                    foreach (Spine::AnnotationHandle annotation, annotations) {
                        if (rendering.bounds.contains(annotation)) {
                            OverlayRenderer * renderer = rendering.bounds[annotation].first;
                            rendering.bounds.remove(annotation);
                            QMutableMapIterator< OverlayRenderer::State, QPair< Spine::AnnotationSet, QMap< int, QPicture > > > iter(rendering.pictures[renderer]);
                            while (iter.hasNext()) {
                                iter.next();
                                if (iter.value().first.erase(annotation) > 0) {
                                    dirty << qMakePair(renderer, iter.key());
                                }
                            }
                        }
                        if (rendering.hoverPictures.contains(annotation)) {
                            rendering.hoverPictures.remove(annotation);
                        }
                    }
                }

                // Recompute dirty lists
                typedef QPair< OverlayRenderer *, OverlayRenderer::State > Pair;
                foreach (const Pair & pair, dirty) {
                    OverlayRenderer * renderer = pair.first;
                    OverlayRenderer::State state = pair.second;
                    rendering.pictures[renderer][state].second = renderer->render(document, rendering.pictures[renderer][state].first, state);
                }
            }
        }
    }

    void DocumentViewPrivate::onDocumentAreaSelectionChanged(const std::string & name, const Spine::AreaSet & areas, bool added)
    {
        //qDebug() << "=== onDocumentAreaSelectionChanged" << qStringFromUnicode(name) << areas.size() << added;

        // Normal mouse selection
        if (name.empty()) {
            // Remember which pages have changed
            QSet< int > changedPageViews;

            // Find all the page views that have changed and reset their saved highlights
            foreach (const Spine::Area & area, areas) {
                if (!changedPageViews.contains(area.page)) {
                    if (PageView * pageView = (area.page > 0 && area.page <= pageViews.size()) ? pageViews.at(area.page - 1) : 0) {
                        changedPageViews.insert(area.page);
                        PageViewOverlay & overlay = pageViewOverlays[pageView];
                        overlay.savedSelectionHighlight = QPainterPath();
                        overlay.savedSelectionHighlight.setFillRule(Qt::WindingFill);
                    }
                }
            }

            // Make sure those affected pages are recomputed
            updateSavedSelection(changedPageViews);
        }
    }

    void DocumentViewPrivate::onDocumentTextSelectionChanged(const std::string & name, const Spine::TextExtentSet & extents, bool added)
    {
        // Normal mouse selection
        if (name.empty()) {
            // Remember which pages have changed
            QSet< int > changedPageViews;

            // Find all the page views that have changed and reset their saved highlights
            foreach (const Spine::TextExtentHandle & extent, extents) {
                for (int page = extent->first.cursor()->page()->pageNumber(); page <= extent->second.cursor()->page()->pageNumber(); ++page) {
                    if (!changedPageViews.contains(page)) {
                        if (PageView * pageView = (page > 0 && page <= pageViews.size()) ? pageViews.at(page - 1) : 0) {
                            changedPageViews.insert(page);
                            PageViewOverlay & overlay = pageViewOverlays[pageView];
                            overlay.savedSelectionHighlight = QPainterPath();
                            overlay.savedSelectionHighlight.setFillRule(Qt::WindingFill);
                        }
                    }
                }
            }

            // Make sure those affected pages are recomputed
            updateSavedSelection(changedPageViews);
        }
    }

    void DocumentViewPrivate::onFitToHeight()
    {
        documentView->setZoomMode(DocumentView::FitToHeight);
    }

    void DocumentViewPrivate::onFitToWidth()
    {
        documentView->setZoomMode(DocumentView::FitToWidth);
    }

    void DocumentViewPrivate::onFitToWindow()
    {
        documentView->setZoomMode(DocumentView::FitToWindow);
    }

    void DocumentViewPrivate::onHorizontalScrollBarValueChanged(int value)
    {
        //qDebug() << "onHorizontalScrollBarValueChanged" << value;
        layout_updatePageViewPositions();
        layout_calculateHorizontalOrigin();
    }

    void DocumentViewPrivate::onLeftToRightFlow()
    {
        documentView->setPageFlowDirection(DocumentView::LeftToRight);
    }

    void DocumentViewPrivate::onOnePage()
    {
        documentView->setPageMode(DocumentView::OneUp);
        documentView->setPageFlow(DocumentView::Separate);
    }

    void DocumentViewPrivate::onOnePageContinuous()
    {
        documentView->setPageMode(DocumentView::OneUp);
        documentView->setPageFlow(DocumentView::Continuous);
    }

    void DocumentViewPrivate::onSelectionFinished()
    {}

    void DocumentViewPrivate::onSelectionStarted()
    {}

    void DocumentViewPrivate::onTextSelectionChanged()
    {
        if (!document) { return; }

        //PageView * caller = static_cast< PageView * >(sender());

        emit selectionChanged(document->textSelection());
    }

    void DocumentViewPrivate::onTopDownFlow()
    {
        documentView->setPageFlowDirection(DocumentView::TopDown);
    }

    void DocumentViewPrivate::onTwoPages()
    {
        documentView->setPageMode(DocumentView::TwoUp);
        documentView->setPageFlow(DocumentView::Separate);
    }

    void DocumentViewPrivate::onTwoPagesContinuous()
    {
        documentView->setPageMode(DocumentView::TwoUp);
        documentView->setPageFlow(DocumentView::Continuous);
    }

    void DocumentViewPrivate::layout_calculateHorizontalOrigin()
    {
        //qDebug() << "=== layout_calculateHorizontalOrigin" << pageViews.size();
        if (interaction.updateOrigin && !pageViews.isEmpty()) {
            // Find the left-most visible page
            Layout::SpacingMap::const_iterator leftMost(--layout.columnSpacing.upper_bound(documentView->horizontalScrollBar()->sliderPosition()));
            layout.horizontalOriginPageViewGridCoords = QPoint(-1, -1);
            if (leftMost != layout.columnSpacing.end()) {
                int c = leftMost->second.index;
                for (size_t r = 0; r < layout.rowSpacing.size(); ++r) {
                    //qDebug() << "  - r,c" << r << c;
                    if (layout.matrix[r][c].pageView == leftMost->second.largestPageView) {
                        layout.horizontalOriginPageViewGridCoords = QPoint(r, c);
                        layout.horizontalOrigin = leftMost->second.largestPageView->pageSize(true).width() * (-leftMost->second.largestPageView->pos().x()) / (qreal) leftMost->second.largestPageView->width();
                        break;
                    }
                }
            }
        }
    }

    void DocumentViewPrivate::layout_calculateVerticalOrigin()
    {
        //qDebug() << "=== layout_calculateVerticalOrigin" << pageViews.size();
        if (interaction.updateOrigin && !pageViews.isEmpty()) {
            // Find the top-most visible page
            Layout::SpacingMap::const_iterator topMost(--layout.rowSpacing.upper_bound(documentView->verticalScrollBar()->sliderPosition()));
            layout.verticalOriginPageViewGridCoords = QPoint(-1, -1);
            if (topMost != layout.rowSpacing.end()) {
                int r = topMost->second.index;
                for (size_t c = 0; c < layout.columnSpacing.size(); ++c) {
                    //qDebug() << "  - r,c" << r << c;
                    if (layout.matrix[r][c].pageView == topMost->second.largestPageView) {
                        layout.verticalOriginPageViewGridCoords = QPoint(r, c);
                        layout.verticalOrigin = topMost->second.largestPageView->pageSize(true).height() * (-topMost->second.largestPageView->pos().y()) / (qreal) topMost->second.largestPageView->height();
                        break;
                    }
                }
            }
        }
    }

    void DocumentViewPrivate::onVerticalScrollBarValueChanged(int value)
    {
        //qDebug() << "onVerticalScrollBarValueChanged" << value;
        layout_updatePageViewPositions();
        layout_calculateVerticalOrigin();
    }

    void DocumentViewPrivate::onWaitingForDblClickTimeout()
    {
        interaction.numberOfClickTimeouts += 1;
        if (interaction.numberOfClicks == interaction.numberOfClickTimeouts) {
            PageViewMouseEvent pvme(interaction.previousPressEvent.pageView,
                                    interaction.previousPressEvent.pos,
                                    interaction.previousPressEvent.button,
                                    interaction.previousPressEvent.buttons,
                                    interaction.previousPressEvent.modifiers,
                                    interaction.numberOfClicks);
            interaction.numberOfClicks = 0;
            interaction.numberOfClickTimeouts = 0;
            if (!interaction.isPressed) {
                if ((interaction.previousPressEvent.pos - interaction.previousReleasePos).manhattanLength() <= QApplication::startDragDistance()) {
                    mouseClick(&pvme);
                }
                interaction.pendingClickEvent = PageViewMouseEvent();
            } else {
                interaction.pendingClickEvent = pvme;
            }
        }
    }

    void DocumentViewPrivate::onWaitingForHoldTimeout()
    {
        interaction.numberOfHoldTimeouts += 1;
        if (interaction.numberOfHolds == interaction.numberOfHoldTimeouts) {
            int holds = interaction.numberOfHolds;
            interaction.numberOfHolds = 0;
            interaction.numberOfHoldTimeouts = 0;
            if (interaction.isPressed) {
                PageViewMouseEvent pvme(interaction.previousPressEvent.pageView,
                                        interaction.previousPressEvent.pos,
                                        interaction.previousPressEvent.button,
                                        interaction.previousPressEvent.buttons,
                                        interaction.previousPressEvent.modifiers,
                                        holds);
                mouseHold(&pvme);
            }
        }
    }

    DocumentViewPrivate::InteractionState DocumentViewPrivate::primaryInteractionState() const
    {
        return interaction.states.isEmpty() ? IdleState : interaction.states.first();
    }

    QColor DocumentViewPrivate::selectionColor() const
    {
        QColor color = interaction.mode == DocumentView::HighlightingMode ? selection.color : QApplication::palette().highlight().color();
        if (!documentView->isActiveWindow()) {
            int gray = qGray(color.rgb());
            color = QColor(gray, gray, gray);
        }
        return color;
    }

    void DocumentViewPrivate::setAnnotationState(const Spine::AnnotationSet & annotations, OverlayRenderer::State state)
    {
        QSet< QPair< OverlayRenderer *, OverlayRenderer::State > > dirty;

        // First, collect annotations according to renderer
        QMap< OverlayRenderer *, Spine::AnnotationSet > collected;
        foreach (Spine::AnnotationHandle annotation, annotations) {
            if (rendering.bounds.contains(annotation)) {
                collected[rendering.bounds[annotation].first].insert(annotation);
            }
        }

        // Remove annotations from old states, and add them to the new state
        QMapIterator< OverlayRenderer *, Spine::AnnotationSet > c_iter(collected);
        while (c_iter.hasNext()) {
            c_iter.next();
            OverlayRenderer * renderer = c_iter.key();
            if (!rendering.pictures[renderer].contains(state)) {
                rendering.pictures[renderer][state]; // To ensure a state actually exists in the map
            }
            QMutableMapIterator< OverlayRenderer::State, QPair< Spine::AnnotationSet, QMap< int, QPicture > > > iter(rendering.pictures[renderer]);
            while (iter.hasNext()) {
                iter.next();
                if (iter.key() == state) {
                    bool added = false;
                    foreach (Spine::AnnotationHandle for_insert, c_iter.value()) {
                        added = iter.value().first.insert(for_insert).second || added;
                    }
                    if (added) {
                        dirty << qMakePair(renderer, iter.key());
                    }
                } else {
                    bool removed = false;
                    foreach (Spine::AnnotationHandle to_erase, c_iter.value()) {
                        removed = iter.value().first.erase(to_erase) || removed;
                    }
                    if (removed) {
                        dirty << qMakePair(renderer, iter.key());
                    }
                }
            }
        }


        // Recompute dirty lists
        typedef QPair< OverlayRenderer *, OverlayRenderer::State > Pair;
        foreach (const Pair & pair, dirty) {
            OverlayRenderer * renderer = pair.first;
            OverlayRenderer::State state = pair.second;
            rendering.pictures[renderer][state].second = renderer->render(document, rendering.pictures[renderer][state].first, state);
        }

        if (!dirty.isEmpty()) {
            documentView->update();
        }
    }

    void DocumentViewPrivate::setInteractionState(InteractionState state)
    {
        QList< InteractionState > states;
        states << state;
        setInteractionStates(states);
    }

    void DocumentViewPrivate::setInteractionStates(const QList< DocumentViewPrivate::InteractionState > & states)
    {
        InteractionState primaryState(states.isEmpty() ? IdleState : states.first());

        switch (primaryState) {
        case SelectingStaticTextState:
        case SelectingTextState:
            documentView->setCursor(Qt::IBeamCursor);
            break;
        case SelectingAreaState:
        case SelectingImageState:
        case IdleState:
        default:
            if (isMouseOverText()) {
                documentView->setCursor(Qt::IBeamCursor);
            } else {
                documentView->setCursor(Qt::ArrowCursor);
            }
            break;
        }

        interaction.states = states;
        interaction.states.removeAll(IdleState);
    }

    void DocumentViewPrivate::setZoom(double newZoom)
    {
        //qDebug() << "setZoom" << newZoom;
        if (newZoom > 0) {
            foreach (PageView * pageView, pageViews) {
                pageView->setZoom(newZoom);
            }
            zoom = newZoom;
            emit zoomChanged();
        }
    }

    Spine::CursorHandle DocumentViewPrivate::textCursorAt(PageView * pageView, const QPointF & point, Spine::DocumentElement element) const
    {
        Spine::CursorHandle cursor;

        for (int i = 0; i < 2; ++i) {
            qreal grace = i * 6.0;

            // Drill down from page cursor
            cursor = pageView->newCursor();
            Spine::CursorHandle prevCursor = cursor;

            while (const Spine::Image * image = cursor->image()) {
                if (!document->imageBased() && contains(image->boundingBox(), point)) {
                    break;
                }
                cursor->nextImage();
            }

            while (const Spine::Region * region = cursor->region()) {
                if (contains(region->boundingBox(), point, grace)) {
                    if (element == Spine::ElementRegion)
                        return cursor;

                    while (const Spine::Block * block = cursor->block()) {
                        if (contains(block->boundingBox(), point, grace)) {
                            if (element == Spine::ElementBlock)
                                return cursor;

                            const Spine::Line * previousLine = 0;
                            while (const Spine::Line * line = cursor->line()) {
                                const Spine::Line * nextLine = cursor->peekNextLine();
                                Spine::BoundingBox bbLine(line->boundingBox());
                                bbLine.x1 -= grace;
                                bbLine.x2 += grace;
                                if (previousLine) { bbLine.y1 = (bbLine.y1 + previousLine->boundingBox().y2) / 2.0; }
                                else { bbLine.y1 -= grace; }
                                if (nextLine) { bbLine.y2 = (bbLine.y2 + nextLine->boundingBox().y1) / 2.0; }
                                else { bbLine.y2 += grace; }
                                if (contains(bbLine, point)) {
                                    if (element == Spine::ElementLine)
                                        return cursor;

                                    while (const Spine::Word * word = cursor->word()) {
                                        Spine::BoundingBox bbWord(word->boundingBox());
                                        bbWord.y1 = bbLine.y1;
                                        bbWord.y2 = bbLine.y2;
                                        if (!cursor->hasPreviousWord()) { bbWord.x1 = bbLine.x1; }
                                        if (!cursor->hasNextWord()) { bbWord.x2 = bbLine.x2; }
                                        if (contains(bbWord, point)) {
                                            if (element == Spine::ElementWord)
                                                return resolveCursor(cursor, point, grace);

                                            while (const Spine::Character * character = cursor->character()) {
                                                Spine::BoundingBox bb(character->boundingBox());
                                                bb.y1 = bbLine.y1;
                                                bb.y2 = bbLine.y2;
                                                if (!cursor->hasPreviousCharacter()) { bb.x1 = bbWord.x1; }
                                                if (!cursor->hasNextCharacter()) { bb.x2 = bbWord.x2; }
                                                if (contains(bb, point)) {
                                                    return resolveCursor(cursor, point, grace);
                                                }
                                                cursor->nextCharacter();
                                            }
                                        } else if (!containsOrRightOf(bbLine, bbWord, point)) {
                                            if (element == Spine::ElementWord)
                                                return resolveCursor(prevCursor, point, grace);

                                            cursor = prevCursor;
                                            while (cursor->character()) {
                                                cursor->nextCharacter();
                                            }
                                            return resolveCursor(cursor, point, grace);
                                        }
                                        prevCursor = cursor->clone();
                                        cursor->nextWord();
                                    }
                                } else if (!containsOrBelow(block->boundingBox(), bbLine, point, grace)) {
                                    // start on next line
                                    return resolveCursor(cursor, point, grace);

                                    // FIXME - what about selecting the inter-line space?
                                }
                                prevCursor = cursor->clone();
                                cursor->nextLine();
                                previousLine = line;
                            }
                        }
                        cursor->nextBlock();
                    }
                }
                cursor->nextRegion();
            }

            // FIXME - at this point we need to look left of the mouse position to
            // see if a line can be found, then above the mouse position to see if
            // a block can be found, otherwise some other sensible way of deciding
            // what text cursor this should represent.
        }

        return resolveCursor(cursor, point);
    }

    void DocumentViewPrivate::toggleBespokeZoom(bool)
    {
        delete actionBespokeZoom;
        actionBespokeZoom = 0;
    }

    void DocumentViewPrivate::toggleFirstPageOdd(bool odd)
    {
        documentView->setBindingMode(odd ? DocumentView::Odd : DocumentView::Even);
    }

    void DocumentViewPrivate::updateActions()
    {
        switch (pageFlow) {
        case DocumentView::Separate:
            actionFitToHeight->setEnabled(true);
            actionFitToWidth->setEnabled(true);
            actionFitToWindow->setEnabled(true);
            groupDirection->setEnabled(false);
            break;
        case DocumentView::Continuous:
            actionFitToWindow->setEnabled(false);
            groupDirection->setEnabled(true);
            switch (pageFlowDirection) {
            case DocumentView::LeftToRight:
                actionFitToHeight->setEnabled(true);
                actionFitToWidth->setEnabled(false);
                break;
            case DocumentView::TopDown:
                actionFitToHeight->setEnabled(false);
                actionFitToWidth->setEnabled(true);
                break;
            }
            break;
        }
        if (pageMode == DocumentView::TwoUp) {
            actionFirstPageOdd->setEnabled(true);
            if (pageFlow == DocumentView::Separate) {
                actionTwoPages->setChecked(true);
            } else {
                actionTwoPagesContinuous->setChecked(true);
            }
        } else {
            actionFirstPageOdd->setEnabled(false);
            if (pageFlow == DocumentView::Separate) {
                actionOnePage->setChecked(true);
            } else {
                actionOnePageContinuous->setChecked(true);
            }
        }
    }

    void DocumentViewPrivate::updateActiveAreaSelection()
    {
        QMutableMapIterator< PageView *, PageViewOverlay > iter(pageViewOverlays);
        while (iter.hasNext()) {
            iter.next();
            PageViewOverlay & overlay = iter.value();
            bool needsUpdate = false;
            if (interactionState() == SelectingAreaState && iter.key() == selection.activeAreaPageView) {
                needsUpdate = true;
                overlay.activeSelectionHighlight = QPainterPath();
                overlay.activeSelectionHighlight.addRect(bound(selection.activeAreaPageView, QRectF(selection.activeAreaFromPoint, selection.activeAreaToPoint).normalized()));
            } else {
                if (!overlay.activeSelectionHighlight.isEmpty()) {
                    needsUpdate = true;
                    overlay.activeSelectionHighlight = QPainterPath();
                }
            }
            if (needsUpdate) {
                updateSelection(iter.key());
            }
        }
    }

    void DocumentViewPrivate::updateActiveTextSelection()
    {
        Spine::CursorHandle from(selection.activeTextFromCursor);
        Spine::CursorHandle to(selection.activeTextToCursor);
        if (from && to) {
            Spine::order(from, to);
            selection.activeTextExtent = Spine::TextExtentHandle(new Spine::TextExtent(from, to));
            QMap< int, QPainterPath > paths(asPaths(selection.activeTextExtent));
            QMapIterator< int, QPainterPath > iter(paths);
            while (iter.hasNext()) {
                iter.next();
                if (PageView * pageView = (iter.key() > 0 && iter.key() <= pageViews.size()) ? pageViews.at(iter.key() - 1) : 0) {
                    PageViewOverlay & overlay = pageViewOverlays[pageView];
                    overlay.activeSelectionHighlight = QPainterPath();
                    overlay.activeSelectionHighlight.addPath(iter.value());
                    overlay.activeSelectionHighlight.setFillRule(Qt::WindingFill);
                    updateSelection(pageView);
                }
            }
        } else {
            foreach (PageView * pageView, pageViews) {
                PageViewOverlay & overlay = pageViewOverlays[pageView];
                overlay.activeSelectionHighlight = QPainterPath();
                updateSelection(pageView);
            }
        }
    }

    void DocumentViewPrivate::updateAnnotationsUnderMouse(PageView * pageView, const QPointF & pagePos)
    {
        setAnnotationState(current.annotations, OverlayRenderer::Idle);

        current.annotations.clear();
        current.annotation.reset();
        if (pageView) {
            int pageNumber = pageView->pageNumber();
            QMapIterator< Spine::AnnotationHandle, QPair< OverlayRenderer *, QMap< int, QPainterPath > > > b_iter(rendering.bounds);
            while (b_iter.hasNext()) {
                b_iter.next();
                if (b_iter.value().second.contains(pageNumber) && b_iter.value().second[pageNumber].contains(pagePos)) {
                    current.annotations.insert(b_iter.key());
                }
            }
            if (!current.annotations.empty()) {
                current.annotation = *current.annotations.begin();
                setAnnotationState(current.annotations, OverlayRenderer::Hover);
            }
        }
		foreach (PageViewOverlay overlay, pageViewOverlays) {
			overlay.widget->update();
		}
    }

    // Main layout calculation method
    void DocumentViewPrivate::layout_calculateGrid()
    {
        //qDebug() << "=== layout_calculateGrid" << pageViews.size();
        // How many pages we have in the whole document
        int pageCount = pageViews.size();

        // Assuming we have any pages at all
        if (pageCount > 0) {
            // Clear old layout information
            layout.columnSpacing.clear();
            layout.rowSpacing.clear();
            layout.matrix.resize(boost::extents[0][0]);

            // The index of the 'current' page
            int pageIndex = pageNumber - 1;

            // Default grid size of 1x1
            int columnCount = 1;
            int rowCount = 1;

            // Default zero offset (the index of the first visible placeholder)
            int offset = 0;

            // Calculate grid size depending on the various view options
            if (pageFlow == DocumentView::Separate || pageFlowDirection == DocumentView::TopDown) { // Vertical or Grid layouts
                // 1-up or 2-up?
                if (pageMode == DocumentView::TwoUp) {
                    columnCount = 2;
                }
                // Offset for an odd starting page
                if (columnCount > 1 && bindingMode == DocumentView::Odd) {
                    offset = -1;
                }
                // For single-row separate mode...
                if (pageFlow == DocumentView::Separate) {
                    // Offset to currently visible page
                    pageIndex = ((pageIndex - offset) / columnCount) * columnCount;
                    offset += pageIndex;
                } else {
                    // Calculate the number of rows needed
                    rowCount = (pageCount - offset) / columnCount;
                    // Compensate for orphans
                    if ((pageCount - offset) % columnCount > 0) {
                        rowCount += 1;
                    }
                }
            } else { // Horizontal layouts
                // One long row of all the pages
                columnCount = pageCount;
            }

            // Resize grid data structure
            layout.matrix.resize(boost::extents[rowCount][columnCount]);
            layout.matrix = Layout::PageViewMatrix(boost::extents[rowCount][columnCount]);

            // Begin by laying out page views logically
            // Keep track of tallest/widest pages in each row/column
            for (int r = 0; r < rowCount; ++r) {
                for (int c = 0; c < columnCount; ++c) {
                    // Index of this cell's page view
                    int i = (columnCount * r) + c + offset;
                    if (i >= 0 && i < pageCount) {
                        PageView * pageView = pageViews.at(i);
                        layout.matrix[r][c].pageView = pageView;

                        layout.columnSpacing[c].index = c;
                        if (layout.columnSpacing[c].largestPageView == 0 ||
                            pageView->pageSize(true).width() > layout.columnSpacing[c].largestPageView->pageSize(true).width()) {
                            layout.columnSpacing[c].largestPageView = pageView;
                        }

                        layout.rowSpacing[r].index = r;
                        if (layout.rowSpacing[r].largestPageView == 0 ||
                            pageView->pageSize(true).height() > layout.rowSpacing[r].largestPageView->pageSize(true).height()) {
                            layout.rowSpacing[r].largestPageView = pageView;
                        }
                    }
                }
            }
        }

        //qDebug() << "<<< layout_calculateGrid" << pageViews.size();
    }

    void DocumentViewPrivate::layout_calculateWhitespace()
    {
        //qDebug() << "=== layout_calculateWhitespace" << pageViews.size();
        if (!pageViews.isEmpty()) {
            // Reset variables
            layout.horizontalWhitespace = 0;
            layout.verticalWhitespace = 0;

            // Grid dimensions
            int columnCount = layout.columnSpacing.size();
            int rowCount = layout.rowSpacing.size();

            // Calculate whitespace required between pages
            if (pageMode == DocumentView::OneUp) {
                layout.horizontalWhitespace = columnCount - 1;
            } else {
                // Special case with odd-paged horizontal continous mode
                if (pageFlow == DocumentView::Continuous &&
                    pageFlowDirection != DocumentView::TopDown &&
                    bindingMode == DocumentView::Odd) {
                    layout.horizontalWhitespace = columnCount / 2;
                } else {
                    layout.horizontalWhitespace = (columnCount - 1) / 2;
                }
            }
            layout.verticalWhitespace = rowCount - 1;
        }
        //qDebug() << "<<< layout_calculateWhitespace" << pageViews.size();
    }

    // Main layout calculation method
    void DocumentViewPrivate::update_layout(int changed)
    {
        static bool running = false;
        //qDebug() << "=== update_layout" << changed;

        if (!running) {
            running = true;
            if (changed & GridChange) {
                layout_calculateGrid();
                layout_calculateWhitespace();
                changed |= SizeChange;
            }
            if (changed & SizeChange) {
                foreach (PageView * pageView, pageViews) { pageView->hide(); }

                layout_updatePageViewSizes();
                layout_calculatePageViewPositions();
                layout_updatePageViewPositions();

                updateScrollBars();
            }
            running = false;
        }
        //qDebug() << "<<< update_layout" << changed;
    }

    void DocumentViewPrivate::layout_updatePageViewSizes()
    {
        //qDebug() << "=== layout_updatePageViewSizes" << pageViews.size();
        // Assuming there are pages at all
        if (!pageViews.isEmpty()) {
            // Get the grid's dimensions
            //int columnCount = layout.columnSpacing.size();
            //int rowCount = layout.rowSpacing.size();
            //qDebug() << "   " << rowCount << columnCount;

            // Resize widgets according to view options
            if (zoomMode != DocumentView::CustomZoom) {
                // Calculate required zoom to fit the constraints
                qreal verticalZoom = 0.0;
                qreal horizontalZoom = 0.0;

                // Calculate the zoom factor required to fit things in vertically
                if (zoomMode == DocumentView::FitToHeight || zoomMode == DocumentView::FitToWindow) {
                    // For each row, find its largest contributing page view,
                    // and use it to calculate how tall the full canvas would
                    // need to be
                    PageView * tallest = 0;
                    qreal paper = 0.0;
                    Layout::SpacingMap::const_iterator i(layout.rowSpacing.begin());
                    Layout::SpacingMap::const_iterator e(layout.rowSpacing.end());
                    for (; i != e; ++i) {
                        qreal height = i->second.largestPageView->pageSize(true).height();
                        paper += height;
                        if (tallest == 0 || tallest->pageSize(true).height() < height) {
                            tallest = i->second.largestPageView;
                        }
                    }

                    // Now work out what vertical zoom would fit the rows in
                    // as snuggly as possible
                    int screen = documentView->maximumViewportSize().height() - layout.verticalWhitespace;
                    tallest->resizeToHeight(qFloor(screen * tallest->pageSize(true).height() / paper));
                    if (zoomMode == DocumentView::FitToHeight) {
                        zoom = tallest->verticalZoom();
                    } else {
                        verticalZoom = tallest->verticalZoom();
                    }
                }

                // Calculate the zoom factor required to fit things in vertically
                if (zoomMode == DocumentView::FitToWidth || zoomMode == DocumentView::FitToWindow) {
                    // For each column, find its largest contributing page view,
                    // and use it to calculate how wide the full canvas would
                    // need to be
                    PageView * widest = 0;
                    qreal paper = 0.0;
                    Layout::SpacingMap::const_iterator i(layout.columnSpacing.begin());
                    Layout::SpacingMap::const_iterator e(layout.columnSpacing.end());
                    for (; i != e; ++i) {
                        qreal width = i->second.largestPageView->pageSize(true).width();
                        paper += width;
                        if (widest == 0 || widest->pageSize(true).width() < width) {
                            widest = i->second.largestPageView;
                        }
                    }

                    // Now work out what horizontal zoom would fit the columns in
                    // as snuggly as possible
                    int screen = documentView->maximumViewportSize().width() - layout.horizontalWhitespace;
                    int target = layout.columnSpacing.size() == 1 ? screen : qFloor(screen * widest->pageSize(true).width() / paper);
                    widest->resizeToWidth(target);
                    if (zoomMode == DocumentView::FitToWidth) {
                        zoom = widest->horizontalZoom();
                    } else {
                        horizontalZoom = widest->horizontalZoom();
                    }
                }

                // For the case of fitting to the window, take the smaller of the two zoom factors
                if (zoomMode == DocumentView::FitToWindow) {
                    zoom = qMin(horizontalZoom, verticalZoom);
                }
            }

            // Apply calculated zoom factor to all pages
            setZoom(zoom);
        }
        //qDebug() << "<<< layout_updatePageViewSizes" << pageViews.size();
    }

    void DocumentViewPrivate::layout_calculatePageViewPositions()
    {
        //qDebug() << "=== layout_calculatePageViewPositions" << pageViews.size();
        // Assuming there are pages at all
        if (!pageViews.isEmpty()) {
            // Calculate geometry of page views
            Layout::SpacingMap rowSpacing(layout.rowSpacing);
            Layout::SpacingMap columnSpacing(layout.columnSpacing);

            // Start with vertical positioning
            {
                // Keep track of the vertical position
                int y = 0;

                // For each row, rewrite the rowSpacing map to relate not the
                // row index, but rather the vertical position to the page view
                layout.rowSpacing.clear();
                Layout::SpacingMap::const_iterator ri(rowSpacing.begin());
                Layout::SpacingMap::const_iterator re(rowSpacing.end());
                for (; ri != re; ++ri) {
                    layout.rowSpacing[y] = ri->second;
                    int height = ri->second.largestPageView->height();
                    Layout::SpacingMap::const_iterator ci(columnSpacing.begin());
                    Layout::SpacingMap::const_iterator ce(columnSpacing.end());
                    for (; ci != ce; ++ci) {
                        if (PageView * pageView = layout.matrix[ri->second.index][ci->second.index].pageView) {
                            // Center align vertically in this row
                            layout.matrix[ri->second.index][ci->second.index].pos.setY(y + (height - pageView->height()) / 2);
                        }
                    }
                    // +1 vertical spacing between rows
                    y += height + 1;
                }
            }
            // Then with horizontal positioning
            {
                // Keep track of the vertical position
                int x = 0;

                bool oneup = pageMode == DocumentView::OneUp;
                bool odd = bindingMode == DocumentView::Odd;

                // For each row, rewrite the columnSpacing map to relate not the
                // column index, but rather the horizontal position to the page view
                layout.columnSpacing.clear();
                Layout::SpacingMap::const_iterator ci(columnSpacing.begin());
                Layout::SpacingMap::const_iterator ce(columnSpacing.end());
                for (; ci != ce; ++ci) {
                    layout.columnSpacing[x] = ci->second;
                    int width = ci->second.largestPageView->width();

                    // Spacing... (for 2nd/4th/6th... columns only, when in 2-up)
                    int space = (oneup ? 1 : ci->second.index % 2);

                    Layout::SpacingMap::const_iterator ri(rowSpacing.begin());
                    Layout::SpacingMap::const_iterator re(rowSpacing.end());
                    for (; ri != re; ++ri) {
                        if (PageView * pageView = layout.matrix[ri->second.index][ci->second.index].pageView) {
                            if (oneup) { // Center align when in one-up mode
                                layout.matrix[ri->second.index][ci->second.index].pos.setX(x + (width - pageView->width()) / 2);
                            } else if (pageView->pageNumber() % 2 == (odd ? 0 : 1)) { // Align to right
                                // If this column has spacing, and we're two-up, the right-aligned
                                // cells ought to use that spacing
                                layout.matrix[ri->second.index][ci->second.index].pos.setX(x + (oneup ? 0 : space) + (width - pageView->width()));
                            } else { // Align to left
                                layout.matrix[ri->second.index][ci->second.index].pos.setX(x);
                            }
                        }
                    }
                    // + horizontal spacing
                    x += width + space;
                }
            }

            // Calculate total extent of layed-out pages
            Layout::SpacingMap::const_iterator lastColumn(--layout.columnSpacing.end());
            Layout::SpacingMap::const_iterator lastRow(--layout.rowSpacing.end());
            layout.size = QSize(lastColumn->first + lastColumn->second.largestPageView->width(),
                                lastRow->first + lastRow->second.largestPageView->height());
        }
        //qDebug() << "<<< layout_calculatePageViewPositions" << pageViews.size();
    }

    void DocumentViewPrivate::updateSavedSelection(const QSet< int > & changedPageViews)
    {
        foreach (const Spine::Area & area, document->areaSelection()) {
            if (changedPageViews.contains(area.page)) {
                PageView * pageView = pageViews.at(area.page - 1);
                PageViewOverlay & overlay = pageViewOverlays[pageView];
                overlay.savedSelectionHighlight.addRect(QRectF(area.boundingBox.x1,
                                                               area.boundingBox.y1,
                                                               area.boundingBox.width(),
                                                               area.boundingBox.height()));
            }
        }

        QMap< int, QPainterPath > paths(asPaths(document->textSelection()));
        QMapIterator< int, QPainterPath > iter(paths);
        while (iter.hasNext()) {
            iter.next();
            if (changedPageViews.contains(iter.key())) {
                PageView * pageView = pageViews.at(iter.key() - 1);
                PageViewOverlay & overlay = pageViewOverlays[pageView];
                overlay.savedSelectionHighlight.addPath(iter.value());
            }
        }

        // Update each changed page view
        foreach (int page, changedPageViews) {
            PageView * pageView = pageViews.at(page - 1);
            updateSelection(pageView);
        }
    }

    void DocumentViewPrivate::updateScrollBars()
    {
        interaction.updateOrigin = false;

        //qDebug() << "updateScrollBars()";
        updateScrollBarPolicies();

        //qDebug() << "updateScrollBars";
        if (!documentView->isEmpty()) {
            // Keep scrollbar ranges sensible
            documentView->verticalScrollBar()->setRange(0, qMax(0, layout.size.height() - documentView->viewport()->height()));
            documentView->verticalScrollBar()->setPageStep(documentView->viewport()->height());
            documentView->horizontalScrollBar()->setRange(0, qMax(0, layout.size.width() - documentView->viewport()->width()));
            documentView->horizontalScrollBar()->setPageStep(documentView->viewport()->width());

            // Bound origin indices to the new structure, in case the grid
            // dimensions have changed
            layout.verticalOriginPageViewGridCoords.rx() = qBound(-1, layout.verticalOriginPageViewGridCoords.x(), (int) layout.matrix.shape()[0] - 1);
            layout.verticalOriginPageViewGridCoords.ry() = qBound(-1, layout.verticalOriginPageViewGridCoords.y(), (int) layout.matrix.shape()[1] - 1);
            layout.horizontalOriginPageViewGridCoords.rx() = qBound(-1, layout.horizontalOriginPageViewGridCoords.x(), (int) layout.matrix.shape()[0] - 1);
            layout.horizontalOriginPageViewGridCoords.ry() = qBound(-1, layout.horizontalOriginPageViewGridCoords.y(), (int) layout.matrix.shape()[1] - 1);

            // Set scrollbar value such that the same page coordinate is at the viewport's origin
            if (layout.verticalOriginPageViewGridCoords.x() >= 0 && layout.verticalOriginPageViewGridCoords.y() >= 0) {
                const Layout::Cell & verticalOriginCell = layout.matrix[layout.verticalOriginPageViewGridCoords.x()][layout.verticalOriginPageViewGridCoords.y()];
                if (verticalOriginCell.pageView) {
                    //qDebug() << "--- V" << documentView->verticalScrollBar()->value() << (int) (verticalOriginCell.pos.y() + verticalOriginCell.pageView->height() * layout.verticalOrigin / verticalOriginCell.pageView->pageSize(true).height());
                    documentView->verticalScrollBar()->setValue(verticalOriginCell.pos.y() + verticalOriginCell.pageView->height() * layout.verticalOrigin / verticalOriginCell.pageView->pageSize(true).height());
                }
            }
            // Set scrollbar value such that the same page coordinate is at the viewport's origin
            if (layout.horizontalOriginPageViewGridCoords.x() >= 0 && layout.horizontalOriginPageViewGridCoords.y() >= 0) {
                const Layout::Cell & horizontalOriginCell = layout.matrix[layout.horizontalOriginPageViewGridCoords.x()][layout.horizontalOriginPageViewGridCoords.y()];
                if (horizontalOriginCell.pageView) {
                    //qDebug() << "--- H" << documentView->horizontalScrollBar()->value() << (int) (horizontalOriginCell.pos.x() + horizontalOriginCell.pageView->width() * layout.horizontalOrigin / horizontalOriginCell.pageView->pageSize(true).width());
                    documentView->horizontalScrollBar()->setValue(horizontalOriginCell.pos.x() + horizontalOriginCell.pageView->width() * layout.horizontalOrigin / horizontalOriginCell.pageView->pageSize(true).width());
                }
            }
        }

        interaction.updateOrigin = true;
    }

    void DocumentViewPrivate::updateScrollBarPolicies()
    {
        if (documentView->autoScrollBars()) {
            if (documentView->isEmpty()) {
                documentView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
                documentView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            } else {
                switch (zoomMode) {
                case DocumentView::CustomZoom:
                    documentView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
                    documentView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
                    break;
                case DocumentView::FitToHeight:
                    actionFitToHeight->setChecked(true);
                    documentView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
                    documentView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
                    break;
                case DocumentView::FitToWidth:
                    actionFitToWidth->setChecked(true);
                    documentView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
                    documentView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
                    break;
                case DocumentView::FitToWindow:
                    actionFitToWindow->setChecked(true);
                    documentView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
                    documentView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
                    break;
                }
            }
        }
    }

    void DocumentViewPrivate::updateSelection(PageView * pageView)
    {
        if (pageView) {
            PageViewOverlay & overlay = pageViewOverlays[pageView];
            overlay.selectionHighlight = QPainterPath();
            overlay.selectionHighlight.setFillRule(Qt::WindingFill);
            overlay.selectionHighlight.addPath(overlay.activeSelectionHighlight);
            overlay.selectionHighlight.addPath(overlay.savedSelectionHighlight);
            overlay.selectionHighlight = overlay.selectionHighlight.simplified();
            overlay.selectionHighlight.setFillRule(Qt::WindingFill);
            overlay.widget->update();
        }
    }

    // Update the page outlines
    void DocumentViewPrivate::updatePageOutlines()
    {
        pageOutlines = QPicture();

        // Generate the grey lines that run between pages
        QPainter painter(&pageOutlines);

        // Grid dimensions
        int rc = layout.rowSpacing.size();
        int cc = layout.columnSpacing.size();

        // Go through each row/column and draw appropriate lines
        Layout::SpacingMap::const_iterator ri(layout.rowSpacing.begin());
        Layout::SpacingMap::const_iterator re(layout.rowSpacing.end());
        for (; ri != re; ++ri) {
            int r = ri->second.index;
            Layout::SpacingMap::const_iterator ci(layout.columnSpacing.begin());
            Layout::SpacingMap::const_iterator ce(layout.columnSpacing.end());
            for (; ci != ce; ++ci) {
                int c = ci->second.index;
                if (PageView * pageView = layout.matrix[r][c].pageView) {
                    if (pageView->isVisible()) {
                        // Update page outlines
                        QRect rect(pageView->geometry());
                        if (c > 0 && layout.matrix[r][c-1].pageView) {
                            painter.drawLine(QLineF(rect.topLeft(), rect.bottomLeft()).translated(-1, 0));
                        }
                        if (c < cc - 1 && layout.matrix[r][c+1].pageView) {
                            painter.drawLine(QLineF(rect.topRight(), rect.bottomRight()).translated(1, 0));
                        }
                        if (r > 0 && layout.matrix[r-1][c].pageView) {
                            painter.drawLine(QLineF(rect.topLeft(), rect.topRight()).translated(0, -1));
                        }
                        if (r < rc - 1 && layout.matrix[r+1][c].pageView) {
                            painter.drawLine(QLineF(rect.bottomLeft(), rect.bottomRight()).translated(0, 1));
                        }
                    }
                }
            }
        }
    }

    // Apply geometry to visible page views, hiding invisible views
    void DocumentViewPrivate::layout_updatePageViewPositions()
    {
        //qDebug() << "=== layout_updatePageViewPositions" << pageViews.size();

        // Only if there are pages to lay out
        if (!pageViews.isEmpty()) {
            // Grid dimensions
            //int columnCount = layout.columnSpacing.size();
            //int rowCount = layout.rowSpacing.size();

            // Viewport dimensions
            QRect viewRect(QPoint(documentView->horizontalScrollBar()->value(),
                                  documentView->verticalScrollBar()->value()), documentView->viewport()->size());

            // If the viewport is larger than required in either axis, shrink it
            // and position it to fit (centre aligned)
            if (layout.size.width() < viewRect.width()) {
                viewRect.moveLeft(- (viewRect.width() - layout.size.width()) / 2);
            }
            if (layout.size.height() < viewRect.height()) {
                viewRect.moveTop(- (viewRect.height() - layout.size.height()) / 2);
            }

            //Layout::SpacingMap::const_iterator rb(--layout.rowSpacing.upper_bound(viewRect.top()));
            //Layout::SpacingMap::const_iterator re(layout.rowSpacing.upper_bound(viewRect.bottom()));
            //Layout::SpacingMap::const_iterator cb(--layout.columnSpacing.upper_bound(viewRect.left()));
            //Layout::SpacingMap::const_iterator ce(layout.columnSpacing.upper_bound(viewRect.right()));
            //for (Layout::SpacingMap::const_iterator r = rb; r != re; ++r) {
            //    for (Layout::SpacingMap::const_iterator c = cb; c != ce; ++c) {
            //        matrix
            //    }
            //}

            QSet< PageView * > toHide;
            foreach (PageView * pageView, pageViews) {
                toHide.insert(pageView);
            }
            //qDebug() << "+++";
            //int rc = layout.rowSpacing.size();
            //int cc = layout.columnSpacing.size();
            Layout::SpacingMap::const_iterator ri(layout.rowSpacing.begin());
            Layout::SpacingMap::const_iterator re(layout.rowSpacing.end());
            for (; ri != re; ++ri) {
                int r = ri->second.index;
                //qDebug() << "   R" << r << "/" << rc;
                Layout::SpacingMap::const_iterator ci(layout.columnSpacing.begin());
                Layout::SpacingMap::const_iterator ce(layout.columnSpacing.end());
                for (; ci != ce; ++ci) {
                    int c = ci->second.index;
                    //qDebug() << "   C" << c << "/" << cc;
                    if (PageView * pageView = layout.matrix[r][c].pageView) {
                        QPoint pos = layout.matrix[r][c].pos - viewRect.topLeft();
                        QSize size = pageView->size();
                        if (pos.x() <= viewRect.width() &&
                            pos.x() + size.width() >= 0 &&
                            pos.y() <= viewRect.height() &&
                            pos.y() + size.height() >= 0) {
                            pageView->move(pos);
                            pageView->show();
                            toHide.remove(pageView);
                        }
                    }
                }
            }
            //qDebug() << "---";

            foreach (PageView * pageView, toHide) {
                pageView->hide();
            }

            updatePageOutlines();
        }

        //qDebug() << "<<< layout_updatePageViewPositions" << pageViews.size();
    }




    /// DocumentView //////////////////////////////////////////////////////////////////////////////

    DocumentView::DocumentView(QWidget * parent)
        : QAbstractScrollArea(parent), d(new DocumentViewPrivate(this))
    {
        d->initialise();
    }

    DocumentView::DocumentView(Spine::DocumentHandle document, QWidget * parent)
        : QAbstractScrollArea(parent), d(new DocumentViewPrivate(this))
    {
        d->initialise();
        setDocument(document);
    }

    DocumentView::~DocumentView()
    {
        clear();

        foreach (OverlayRenderer * doomed, d->overlayRenderers.values()) {
            delete doomed;
        }
        foreach (OverlayRendererMapper * doomed, d->overlayRendererMappers) {
            delete doomed;
        }
    }

    Spine::AnnotationSet DocumentView::activeAnnotations() const
    {
        return d->current.annotations;
    }

    bool DocumentView::autoScrollBars() const
    {
        return d->autoScrollBars;
    }

    DocumentView::BindingMode DocumentView::bindingMode() const
    {
        return d->bindingMode;
    }

    void DocumentView::clear()
    {
        // Disconnect from model
        d->documentProxy.reset();

        // Clear all state for this document
        clearSearch();
        d->clearPageViews();
        d->document.reset();
        d->pageNumber = 0;

        // Disable menu items that no longer make any sense
        d->layoutMenu->setEnabled(false);
        d->zoomMenu->setEnabled(false);

        update();
    }

    void DocumentView::clearSearch()
    {
        d->spotlights.clear();
        d->activeSpotlight = 0;

        // Pass results on to pageViews
        foreach (PageView * pageView, d->pageViews) {
            pageView->clearActiveSpotlight();
            pageView->clearSpotlights();
        }

        update();
    }

    void DocumentView::contextMenuEvent(QContextMenuEvent * event)
    {
        if (!document()) { return; }

        QMenu menu(this);

        // Layout options
        menu.addMenu(d->layoutMenu);

        // Zoom options
        menu.addMenu(d->zoomMenu);

        // Give the page view a chance to modify the menu
        foreach (PageView * pageView, d->pageViews) {
            if (pageView->isVisible()) {
                QPoint pageViewPos(pageView->mapFrom(this, event->pos()));
                if (pageView->rect().contains(pageViewPos)) {
                    pageView->populateContextMenuAt(&menu, pageViewPos);
                    menu.addSeparator();
                    break;
                }
            }
        }

        // Give other components a chance to modify the menu
        emit contextMenuAboutToShow(&menu, document(), d->interaction.mouseTextCursor);

        // Execute menu
        menu.exec(event->globalPos());
    }

    void DocumentView::copySelectedText()
    {
        if (!document()) { return; }

        std::string selectionText(document()->selectionText());

        if (!selectionText.empty()) {
            QClipboard * clipboard = QApplication::clipboard();
            clipboard->setText(qStringFromUnicode(selectionText), QClipboard::Clipboard);
        }
    }

    Spine::DocumentHandle DocumentView::document() const
    {
        return d->document;
    }

    void DocumentView::focusOutEvent(QFocusEvent * event)
    {
    }

    void DocumentView::focusNextSpotlight()
    {
        if (d->activeSpotlight < d->spotlights.size())
        {
            Spine::TextExtentHandle curr = d->spotlights.at(d->activeSpotlight);
            d->activeSpotlight = (d->activeSpotlight + 1) % d->spotlights.size();
            Spine::TextExtentHandle next = d->spotlights.at(d->activeSpotlight);
            showPage(next);

            // Pass results on to pageViews
            foreach (PageView * pageView, d->pageViews) {
                pageView->setActiveSpotlight(next);
            }
        }
    }

    void DocumentView::focusPreviousSpotlight()
    {
        if (d->activeSpotlight < d->spotlights.size())
        {
            Spine::TextExtentHandle curr = d->spotlights.at(d->activeSpotlight);
            d->activeSpotlight = (d->activeSpotlight + d->spotlights.size() - 1) % d->spotlights.size();
            Spine::TextExtentHandle prev = d->spotlights.at(d->activeSpotlight);
            showPage(prev);

            // Pass results on to pageViews
            foreach (PageView * pageView, d->pageViews) {
                pageView->setActiveSpotlight(prev);
            }
        }
    }

    void DocumentView::hideSpotlights()
    {
        // Pass on to pageViews
        foreach (PageView * pageView, d->pageViews) {
            pageView->hideSpotlights();
        }

        emit spotlightsHidden();
    }

    QColor DocumentView::highlightColor() const
    {
        return d->selection.color;
    }

    void DocumentView::highlightSelection()
    {
        if (d->document) {
            foreach (Spine::TextExtentHandle extent, d->document->textSelection()) {
                d->createHighlight(0, extent);
            }
            foreach (const Spine::Area & area, d->document->areaSelection()) {
                d->createHighlight(&area, Spine::TextExtentHandle());
            }
            d->document->clearSelection();
        }
    }

    DocumentView::InteractionMode DocumentView::interactionMode() const
    {
        return d->interaction.mode;
    }

    bool DocumentView::isEmpty() const
    {
        return !d->document;
    }

    bool DocumentView::isExposing() const
    {
        return d->interaction.isExposing;
    }

    void DocumentView::loadState(const OptionState & state)
    {
        setBindingMode(state.bindingMode);
        setPageFlow(state.pageFlow);
        setPageFlowDirection(state.pageFlowDirection);
        setPageMode(state.pageMode);
        setZoomMode(state.zoomMode);
        setZoom(state.zoom);
    }

    QMenu * DocumentView::layoutMenu() const
    {
        return d->layoutMenu;
    }

    DocumentView::PageFlow DocumentView::pageFlow() const
    {
        return d->pageFlow;
    }

    DocumentView::PageFlowDirection DocumentView::pageFlowDirection() const
    {
        return d->pageFlowDirection;
    }

    DocumentView::PageMode DocumentView::pageMode() const
    {
        return d->pageMode;
    }

    PageView * DocumentView::pageView(int page) const
    {
        if (page > 0 && page <= d->pageViews.count())
        {
            return d->pageViews.at(page - 1);
        }
        else
        {
            return 0;
        }
    }

    void DocumentView::paintEvent(QPaintEvent * event)
    {
        if (!isEmpty()) {
            QPainter painter(viewport());
            d->pageOutlines.play(&painter);
        }
    }

    void DocumentView::resizeEvent(QResizeEvent * event)
    {
        d->update_layout(SizeChange);
    }

    DocumentView::OptionState DocumentView::saveState() const
    {
        OptionState state;
        state.bindingMode = bindingMode();
        state.pageFlow = pageFlow();
        state.pageFlowDirection = pageFlowDirection();
        state.pageMode = pageMode();
        state.zoomMode = zoomMode();
        state.zoom = zoom();
        return state;
    }

    Spine::TextExtentSet DocumentView::search(const QString & term, int options)
    {
        Spine::TextExtentSet results;

        if (document())
        {
            //qDebug() << "start DocumentView::search()";
            if (term.isEmpty())
            {
                clearSearch();
            }
            else
            {
                //qDebug() << "start Document::search()";
                Spine::TextExtentSet extents = document()->search(unicodeFromQString(term), options);
                //qDebug() << "finished Document::search()";
                d->spotlights.assign(extents.begin(), extents.end());
                results.insert(extents.begin(), extents.end());
                d->activeSpotlight = 0;
                Spine::TextExtentHandle first;

                if (extents.size() > 0)
                {
                    // Find next visible hit and set to active
                    if ((*--extents.end())->first.cursor()->page()->pageNumber() < d->pageNumber)
                    {
                        first = *extents.begin();
                        showPage(first);
                    }
                    else
                    {
                        // Find next visible
                        BOOST_FOREACH(Spine::TextExtentHandle extent, extents)
                        {
                            if (extent->first.cursor()->page()->pageNumber() >= d->pageNumber)
                            {
                                first = extent;
                                showPage(extent);
                                break;
                            }
                            ++d->activeSpotlight;
                        }
                    }

                    // Pass results on to pageViews
                    foreach (PageView * pageView, d->pageViews) {
                        pageView->setSpotlights(extents);
                        pageView->setActiveSpotlight(first);
                    }
                }
                else
                {
                    clearSearch();
                }

                update();
            }
            //qDebug() << "finished DocumentView::search()";
        }

        return results;
    }

    void DocumentView::selectNone()
    {
        if (!document()) { return; }

        document()->clearSelection();

        // FIXME notify pageViews
    }

    void DocumentView::setAutoScrollBars(bool value)
    {
        d->autoScrollBars = value;
        d->updateScrollBarPolicies();
    }

    void DocumentView::setBindingMode(BindingMode mode)
    {
        BindingMode oldBindingMode = d->bindingMode;
        d->bindingMode = mode;

        if (pageFlow() == Separate) {
            showPage(d->pageNumber);
        }
        d->update_layout(GridChange);
        update();

        if (oldBindingMode != mode) {
            emit bindingModeChanged();
        }
    }

    void DocumentView::setDocument(Spine::DocumentHandle document, int pageNumber, const QRectF & pageRect)
    {
        clear();
        d->document = document;
        if (document) {
            d->pageNumber = 1;
            d->updateScrollBarPolicies();
            d->createPageViews();
            showPage(pageNumber, pageRect);
            d->layoutMenu->setEnabled(true);
            d->zoomMenu->setEnabled(true);

            d->documentProxy.reset(new DocumentProxy(this));
            connect(d->documentProxy.get(), SIGNAL(annotationsChanged(const std::string &, const Spine::AnnotationSet &, bool)),
                    d, SLOT(onDocumentAnnotationsChanged(const std::string &, const Spine::AnnotationSet &, bool)));
            connect(d->documentProxy.get(), SIGNAL(areaSelectionChanged(const std::string &, const Spine::AreaSet &, bool)),
                    d, SLOT(onDocumentAreaSelectionChanged(const std::string &, const Spine::AreaSet &, bool)));
            connect(d->documentProxy.get(), SIGNAL(textSelectionChanged(const std::string &, const Spine::TextExtentSet &, bool)),
                    d, SLOT(onDocumentTextSelectionChanged(const std::string &, const Spine::TextExtentSet &, bool)));
            d->documentProxy->setDocument(document);

            // register existing annotations
            foreach (const std::string & name, document->annotationLists()) {
                d->onDocumentAnnotationsChanged(name, document->annotations(name), true);
            }
        }
        update();
    }

    void DocumentView::setExposing(bool exposing)
    {
        if (exposing != d->interaction.isExposing) {
            d->interaction.isExposing = exposing;
			foreach (PageViewOverlay overlay, d->pageViewOverlays) {
				overlay.widget->update();
			}
        }
    }

    void DocumentView::setHighlightColor(const QColor & color)
    {
        if (d->selection.color != color) {
            d->selection.color = color;
            update();
        }
    }

    void DocumentView::setInteractionMode(InteractionMode interactionMode)
    {
        if (d->interaction.mode != interactionMode) {
            d->interaction.mode = interactionMode;
        }
    }

    void DocumentView::setPageFlow(PageFlow flow)
    {
        PageFlow oldPageFlow = d->pageFlow;

        d->pageFlow = flow;

        // Ensure certain actions are disabled / labeled correctly
        bool oneup = (pageMode() == OneUp);

        switch (flow) {
        case Separate: {
            d->actionOnePage->setChecked(oneup);
            d->actionTwoPages->setChecked(!oneup);
            d->updateActions();
            d->update_layout(GridChange);
            update();
            break;
        }
        case Continuous: {
            d->actionOnePageContinuous->setChecked(oneup);
            d->actionTwoPagesContinuous->setChecked(!oneup);
            ZoomMode newZoomMode = zoomMode();
            if (zoomMode() == FitToWindow) {
                if (pageFlowDirection() == TopDown) {
                    newZoomMode = FitToWidth;
                } else {
                    newZoomMode = FitToHeight;
                }
            } else if (zoomMode() == FitToWidth &&
                       pageFlowDirection() == LeftToRight) {
                newZoomMode = FitToHeight;
            } else if (zoomMode() == FitToHeight &&
                       pageFlowDirection() == TopDown) {
                newZoomMode = FitToWidth;
            }

            d->updateActions();
            if (newZoomMode != zoomMode()) {
                setZoomMode(newZoomMode);
            } else {
                d->update_layout(GridChange);
                update();
            }
            break;
        }
        }

        if (oldPageFlow != flow) {
            showPage(d->pageNumber);
            emit pageFlowChanged();
        }
    }

    void DocumentView::setPageFlowDirection(PageFlowDirection direction)
    {
        PageFlowDirection oldPageFlowDirection = d->pageFlowDirection;

        d->pageFlowDirection = direction;

        // Ensure certain actions are disabled / labeled correctly
        switch (direction) {
        case LeftToRight:
            d->actionLeftToRightFlow->setChecked(true);
            if (zoomMode() == FitToWidth) {
                setZoomMode(FitToHeight);
            }
            break;
        case TopDown:
            d->actionTopDownFlow->setChecked(true);
            if (zoomMode() == FitToHeight) {
                setZoomMode(FitToWidth);
            }
            break;
        }

        d->updateActions();
        d->update_layout(GridChange);
        update();

        if (oldPageFlowDirection != direction) {
            emit pageFlowDirectionChanged();
        }
    }

    void DocumentView::setPageMode(PageMode mode)
    {
        PageMode oldPageMode = d->pageMode;

        d->pageMode = mode;
        d->updateActions();
        d->update_layout(GridChange);
        update();

        if (oldPageMode != mode) {
            emit pageModeChanged();
        }
    }

    void DocumentView::setZoom(double zoom)
    {
        if (zoom > 0) {
            // Change zoom mode if needs be
            bool zoomModeHasChanged = false;
            if (zoomMode() != CustomZoom) {
                d->zoomMode = CustomZoom;
                zoomModeHasChanged = true;
                d->updateScrollBarPolicies();
            }

            int percentage = qRound(zoom * 100);

            // Add into menu if not exact
            if (!d->zoomPercentages.contains(percentage)) {
                QString str = QString("%1").arg(percentage);
                QAction * actionBespokeZoom = new QAction(str + "%", this);
                QAction * after = 0;
                QMap< int, QAction * >::iterator afterIter = d->zoomPercentages.upperBound(percentage);
                if (afterIter != d->zoomPercentages.end()) {
                    after = *afterIter;
                }
                d->zoomMenu->insertAction(after, actionBespokeZoom);
                actionBespokeZoom->setCheckable(true);
                d->zoomActionGroup->addAction(actionBespokeZoom);
                actionBespokeZoom->setChecked(true);
                QObject::connect(actionBespokeZoom, SIGNAL(toggled(bool)), d, SLOT(toggleBespokeZoom(bool)));
                d->actionBespokeZoom = actionBespokeZoom;
            } else {
                d->zoomPercentages[percentage]->setChecked(true);
            }

            d->zoom = zoom;
            d->update_layout(SizeChange);
            update();

            if (zoomModeHasChanged) {
                emit zoomModeChanged();
            }
        }
    }

    void DocumentView::setZoom(int percentage)
    {
        double zoom = percentage / (double) 100;
        setZoom(zoom);
    }

    void DocumentView::setZoomMode(ZoomMode mode)
    {
        if (d->zoomMode != mode) {
            d->zoomMode = mode;

            switch (mode) {
            case CustomZoom:
                break;
            case FitToHeight:
                d->actionFitToHeight->setChecked(true);
                break;
            case FitToWidth:
                d->actionFitToWidth->setChecked(true);
                break;
            case FitToWindow:
                d->actionFitToWindow->setChecked(true);
                break;
            }

            d->updateScrollBarPolicies();

            if (mode == CustomZoom) {
                setZoom(zoom());
            } else {
                // recalculate layout if this originally came from such a request
                if (sender() != d->actionFitToHeight &&
                    sender() != d->actionFitToWidth &&
                    sender() != d->actionFitToWindow) {
                    d->update_layout(GridChange);
                } else {
                    // Otherwise just recalculate zoom factors
                    d->update_layout(SizeChange);
                }
                viewport()->update();
                emit zoomModeChanged();
            }
        }
    }

    void DocumentView::showAnnotation(Spine::AnnotationHandle annotation)
    {
        Spine::TextExtentSet extents(annotation->extents());
        if (!extents.empty()) {
            Spine::TextExtentHandle extent = *extents.begin();
            showPage(extent);
        } else {
            Spine::AreaSet areas(annotation->areas());
            if (!areas.empty()) {
                Spine::Area area = *areas.begin();
                showPage(area.page, QRectF(area.boundingBox.x1, area.boundingBox.y1, area.boundingBox.width(), area.boundingBox.height()));
            }
        }
    }

    void DocumentView::showFirstPage()
    {
        if (pageFlow() == Separate) {
            showPage(1);
        } else {
            verticalScrollBar()->triggerAction(QAbstractSlider::SliderToMinimum);
        }
    }

    void DocumentView::showLastPage()
    {
        if (!document()) { return; }

        if (pageFlow() == Separate) {
            showPage(document()->numberOfPages());
        } else {
            verticalScrollBar()->triggerAction(QAbstractSlider::SliderToMaximum);
        }
    }

    void DocumentView::showNextPage()
    {
        if (pageFlow() == Separate) {
            showPage(d->pageNumber + (pageFlow() == Separate ? d->layout.columnSpacing.size() : 1));
        } else {
            verticalScrollBar()->triggerAction(QAbstractSlider::SliderPageStepAdd);
        }
    }

    void DocumentView::showPage(Spine::TextExtentHandle extent)
    {
        Spine::BoundingBox bb;
        bool first = true;
        int page = 1;
        foreach (Spine::Area area, extent->areas()) {
            if (first) {
                page = area.page;
                first = false;
                bb = area.boundingBox;
            } else if (page == area.page) {
                bb |= area.boundingBox;
            }
        }
        showPage(page, QRectF(bb.x1, bb.y1, bb.width(), bb.height()));
    }

    void DocumentView::showPage(const QVariantMap & params)
    {
        bool hasPage = params.contains("page");
        bool hasPos = params.contains("pos");
        bool hasRect = params.contains("rect");
        bool hasAnchor = params.contains("anchor");
        bool hasText = params.contains("text");
        QString show = params.value("show").toString();

        // Unpack params
        int page = hasPage ? params.value("page").toInt() : 1;
        QRectF rect;
        QString anchor;
        QString text;
        if (hasPos) {
            rect = QRectF(params.value("pos").toPointF(), QSizeF(0, 0));
        } else if (hasRect) {
            rect = params.value("rect").toRectF();
        } else if (hasAnchor) {
            anchor = params.value("anchor").toString();
        } else if (hasText) {
            text = params.value("text").toString();
        }

        // Send to a region on the page
        if (hasPos || hasRect) {
            showPage((size_t) page, rect);
        } else if (hasAnchor || hasText) {
            Spine::TextExtentSet extents;
            Spine::AreaSet areas;
            if (hasAnchor) {
                // Find the appropriate anchor
                foreach (Spine::AnnotationHandle annotation, document()->annotations()) {
                    if (annotation->getFirstProperty("property:anchor") == unicodeFromQString(anchor)) {
                        extents = annotation->extents();
                        areas = annotation->areas();
                        break;
                    }
                }
            } else { // if hasText
                extents = document()->search(unicodeFromQString(text));
            }

            if (!extents.empty()) {
                Spine::TextExtentHandle extent = *extents.begin();
                showPage(extent);
            } else {
                if (!areas.empty()) {
                    Spine::Area area = *areas.begin();
                    showPage(area.page, QRectF(area.boundingBox.x1, area.boundingBox.y1, area.boundingBox.width(), area.boundingBox.height()));
                }
            }

            if (show == "select") {
                document()->clearSelection();
                document()->setTextSelection(Spine::TextSelection(extents));
                document()->setAreaSelection(areas);
            } else if (show == "highlight" && !extents.empty()) {
                Spine::AnnotationHandle highlight = d->createHighlight(0, *extents.begin(), false, false);
                highlight->setProperty("displayTooltip", "You were brought to this article because of this phrase");
                highlight->removeProperty("property:color");
                highlight->setProperty("property:color", "#FFE8D8");
                document()->addAnnotation(highlight);
            }
            // FIXME have other forms of showing
        } else if (hasPage) {
            showPage((size_t) page);
        }
    }

    void DocumentView::showPage(int pageNumber, const QRectF & pageRect)
    {
        if (!document()) { return; }

        //qDebug() << "___________" << pageNumber << pageRect;

        // Clamp page number to document range
        pageNumber = qBound< size_t >(1, pageNumber, document()->numberOfPages());

        if (PageView * pageView = d->pageViews.at(pageNumber - 1)) {
            // Only change the page if required
            if (d->pageNumber != pageNumber) {
                d->pageNumber = pageNumber;

                emit pageFocusChanged(pageNumber);

                QLineEdit * focussedLineEdit = qobject_cast< QLineEdit * >(QApplication::focusWidget());
                pageView->setFocus(Qt::OtherFocusReason);
                if (focussedLineEdit) {
                    focussedLineEdit->setFocus(Qt::OtherFocusReason);
                }

                if (pageFlow() == Separate) {
                    d->update_layout(GridChange);
                }
            }

            // FIXME remove this when not debugging
            //document()->setAreaSelection(Spine::Area(pageNumber, 0, Spine::BoundingBox(pageRect.left(), pageRect.top(), pageRect.right(), pageRect.bottom())));

            // Show given page / rectangle

            // Start by finding the target rectangle in widget coordinates
            QRect targetRect(pageView->transformFromPage(pageRect));
            QRect targetPageRect(pageView->rect());
            QRect targetBlockRect(targetPageRect);
            QSize visibleSize(viewport()->size());

            // Grow target to at least a single pixel square
            targetRect.setSize(targetRect.size().expandedTo(QSize(1, 1)));

            // Grow the target rectangle and find the block / page rects in which it resides
            // Try and find the block(s) that holds the coordinates
            bool first = true;
            Spine::CursorHandle cursor = pageView->newCursor();
            Spine::BoundingBox blockBB;
            Spine::BoundingBox targetBB(pageRect.left(), pageRect.top(),
                                        pageRect.width() > 0 ? pageRect.right() : pageRect.left() + 1.0,
                                        pageRect.height() > 0 ? pageRect.bottom() : pageRect.top() + 1.0);
            while (const Spine::Region * block = cursor->nextRegion(Spine::WithinPage)) {
                // If intersects with target rect
                if (targetBB.x2>=block->boundingBox().x1 &&
                    targetBB.x1<=block->boundingBox().x2 &&
                    targetBB.y2>=block->boundingBox().y1 &&
                    targetBB.y1<=block->boundingBox().y2) {
                    if (first) {
                        first = false;
                        blockBB = block->boundingBox();
                    } else {
                        blockBB |= block->boundingBox();
                    }
                }
            }
            if (blockBB.isValid()) {
                const float padding = 8.0;
                targetBlockRect = pageView->transformFromPage(QRectF(blockBB.x1, blockBB.y1, blockBB.width(), blockBB.height()).adjusted(-padding, -padding, padding, padding));
            }

            // Clamp the rectangles to the size of the page
            targetRect = targetRect.intersected(targetPageRect);
            targetBlockRect = targetBlockRect.intersected(targetPageRect);

            //qDebug() << "=====P" << targetPageRect;
            //qDebug() << "=====B" << targetBlockRect;
            //qDebug() << "=====R" << targetRect;

            // Now find the view coordinates of the targeted page
            QPoint targetPos;
            for (DocumentViewPrivate::Layout::Cell * cell = d->layout.matrix.data(); cell < (d->layout.matrix.data() + d->layout.matrix.num_elements()); ++cell) {
                if (cell->pageView == pageView) {
                    targetPos = cell->pos;
                    break;
                }
            }

            // So now we choose where to scroll the viewport to in order to ensure the
            // requested target rectangle is visible. To do this we use the page rect
            // and the block rect to help us. Start by trying to show the whole page,
            // then the block, then just the rect itself. No zooming is applied.

            // Start by translating all the target rects into canvas coordinates
            targetRect.translate(targetPos);
            targetPageRect.translate(targetPos);
            targetBlockRect.translate(targetPos);

            // Align page with centre of the viewport if possible
            QSize diff((visibleSize - targetPageRect.size()) / 2);
            targetPos -= QPoint(qMax(0, diff.width()), qMax(0, diff.height()));

            // If block is not entirely visible, modify to ensure it is
            QRect visibleRect(targetPos, visibleSize);
            if (!visibleRect.contains(targetBlockRect)) {
                // Modify the horizontal position if required

                // attempt a right-align if we've clipped the right of the block
                if (targetBlockRect.right() > visibleRect.right()) {
                    visibleRect.moveLeft(targetPageRect.right() - visibleRect.width());
                }
                // align to the left of the block if we're still clipped
                if (targetBlockRect.left() < visibleRect.left()) {
                    visibleRect.moveLeft(targetBlockRect.left());
                }

                // Modify the vertical position if required

                // attempt a bottom-align if we've clipped the bottom of the block
                if (targetBlockRect.bottom() > visibleRect.bottom()) {
                    visibleRect.moveTop(targetPageRect.bottom() - visibleRect.height());
                }
                // align to the top of the block if we're still clipped
                if (targetBlockRect.top() < visibleRect.top()) {
                    visibleRect.moveTop(targetBlockRect.top());
                }
            }

            // If rect is not entirely visible, modify to ensure it is
            if (!visibleRect.contains(targetRect)) {
                // Modify the horizontal position if required

                // attempt a right-align if we've clipped the right of the rect
                if (targetRect.right() > visibleRect.right()) {
                    visibleRect.moveLeft(targetBlockRect.right() - visibleRect.width());
                }
                // align to the left of the rect if we're still clipped
                if (targetRect.left() < visibleRect.left()) {
                    visibleRect.moveLeft(targetRect.left());
                }

                // Modify the vertical position if required

                // attempt a bottom-align if we've clipped the bottom of the rect
                if (targetRect.bottom() > visibleRect.bottom()) {
                    visibleRect.moveTop(targetRect.bottom() - visibleRect.height());
                }
                // align to the top of the rect if we're still clipped
                if (targetRect.top() < visibleRect.top()) {
                    visibleRect.moveTop(targetRect.top());
                }
            }

            verticalScrollBar()->setValue(visibleRect.top());
            horizontalScrollBar()->setValue(visibleRect.left());
        }

        update();
    }

    void DocumentView::showPreviousPage()
    {
        if (pageFlow() == Separate) {
            int step = pageFlow() == Separate ? d->layout.columnSpacing.size() : 1;
            if (step <= d->pageNumber)
            {
                showPage(d->pageNumber - step);
            }
        } else {
            verticalScrollBar()->triggerAction(QAbstractSlider::SliderPageStepSub);
        }
    }

    Spine::TextSelection DocumentView::textSelection() const
    {
        return document() ? document()->textSelection() : Spine::TextSelection();
    }

    QList< QAction * > DocumentView::toolBarActions() const
    {
        QList< QAction * > actions;
        actions << d->actionOnePage;
        actions << d->actionTwoPages;
        actions << d->actionFitToWindow;
        actions << d->actionFitToWidth;
        return actions;
    }

    // FIXME This is doomed to be replaced
    void DocumentView::updateAnnotations()
    {
        foreach (PageView * pageView, d->pageViews) {
            pageView->updateAnnotations(std::string(), Spine::AnnotationSet(), true);
            pageView->update();
        }
    }

    void DocumentView::wheelEvent(QWheelEvent * event)
    {
        if (event->modifiers() & Qt::ControlModifier)
        {
            //qDebug() << event->delta() << "        " << event->delta() / 500.0;
            //double newZoom = zoom() + event->delta() / 12000.0;
            double inc = event->delta() / 500.0;
            zoomIn(inc);
        }
        else
        {
            QAbstractScrollArea::wheelEvent(event);
        }
    }

    double DocumentView::zoom() const
    {
        return d->zoom;
    }

    void DocumentView::zoomIn(double delta)
    {
        const double x = 1.4;
        double newZoom = pow(pow(zoom(), 1/x) + delta, x);
        setZoom(qBound(0.1, newZoom, 8.0));
    }

    void DocumentView::zoomOut(double delta)
    {
        zoomIn(-delta);
    }

    QMenu * DocumentView::zoomMenu() const
    {
        return d->zoomMenu;
    }

    DocumentView::ZoomMode DocumentView::zoomMode() const
    {
        return d->zoomMode;
    }

}
