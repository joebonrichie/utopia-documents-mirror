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

#include "papyro/pageview_p.h"
#include <boost/foreach.hpp>
//#include <papyro/calloutwidget.h>
#include <papyro/citationpopup.h>
#include <papyro/commentdata.h>
//#include <papyro/commentview.h>
#include <papyro/documentfactory.h>
#include <papyro/pageview.h>
//#include <papyro/papyrowindow.h>
#include <papyro/utils.h>
#include <utopia2/qt/filedialog.h>
#include <utopia2/qt/fileformatdialog.h>
#include <utopia2/qt/filefixerdialog.h>
#include <utopia2/qt/hidpi.h>
#include <utopia2/extension.h>
#include <utopia2/node.h>
#include <utopia2/parser.h>

#include <QAction>
#include <QApplication>
#include <QBuffer>
#include <QClipboard>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QDir>
#include <QDrag>
#include <QFont>
#include <QImageWriter>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QMouseEvent>
#include <QMutexLocker>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPainter>
#include <QPaintEvent>
#include <QPicture>
#include <QPoint>
#include <QProcess>
#include <QRegExp>
#include <QResizeEvent>
#include <QRunnable>
#include <boost/scoped_ptr.hpp>
#include <QSettings>
#include <QSignalMapper>
#include <QTemporaryFile>
#include <QTime>
#include <QTimer>
#include <QToolTip>
#include <QVector2D>
#include <QWaitCondition>
#include <QUrl>

#include <QtDebug>
#include <iostream>
#include <math.h>

#include <QtCore/qmath.h>

namespace
{

    static inline bool contains(const Spine::BoundingBox & bb, const QPointF & point, qreal grace = 0.0)
    {
        return Spine::BoundingBox(bb.x1 - grace, bb.y1 - grace, bb.x2 + grace, bb.y2 + grace).contains(point.x(), point.y());
    }

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

//     QRectF extentPageBounds(Spine::TextExtentHandle extent, int pageNumber)
//     {
//         QRectF bounds;
//         BOOST_FOREACH(const Spine::Area & area, extent->areas())
//         {
//             if (area.page == pageNumber)
//             {
//                 const Spine::BoundingBox & bb = area.boundingBox;
//                 bounds |= QRectF(bb.x1, bb.y1, bb.width(), bb.height());
//             }
//         }
//         return bounds;
//     }

//     static QPointF qRound(const QPointF & p, int places = 0)
//     {
//         const int factor(qPow(10, places));
//         return QPointF(::qRound(p.x() * factor) / (qreal) factor,
//                        ::qRound(p.y() * factor) / (qreal) factor);
//     }
//
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

//     static QPainterPath roundyCorners(const QVector< QRectF > & rects_, const qreal radius = 1.0, const QSizeF & padding = QSizeF(0.0, 1.0))
//     {
//         // Compile rectangles
//         QVector< QRectF > rects(rects_);
//         foreach (const QRectF & rect, rects_) {
//             rects << rect.adjusted(-(radius + padding.width()), -(radius + padding.height()), radius + padding.width(), radius + padding.height());
//         }
//
//         // Modify rects to remove similar x coordinates
//         if (rects.size() > 1) {
//             bool fixed = true;
//             while (fixed) {
//                 fixed = false;
//                 int indexFrom = 0;
//                 qreal conformTo = rects.at(0).right();
//                 for (int i = 1; i < rects.size(); ++i) {
//                     if (qAbs(rects.at(i).right() - rects.at(i-1).right()) >= (2.0 * radius)) {
//                         if (i - indexFrom > 1) {
//                             // Conform
//                             for (int j = indexFrom; j < i; ++j) {
//                                 if (rects.at(j).right() != conformTo) {
//                                     rects[j].setRight(conformTo);
//                                     fixed = true;
//                                 }
//                             }
//                         }
//
//                         indexFrom = i;
//                         conformTo = rects.at(i).right();
//                     } else {
//                         conformTo = qMax(conformTo, rects.at(i).right());
//                     }
//                 }
//                 if (indexFrom != rects.size() - 1) {
//                     // Conform
//                     for (int j = indexFrom; j < rects.size(); ++j) {
//                         if (rects.at(j).right() != conformTo) {
//                             rects[j].setRight(conformTo);
//                             fixed = true;
//                         }
//                     }
//                 }
//                 indexFrom = 0;
//                 conformTo = rects.at(0).left();
//                 for (int i = 1; i < rects.size(); ++i) {
//                     if (i == rects.size() - 1 || qAbs(rects.at(i).left() - rects.at(i-1).left()) >= (2.0 * radius)) {
//                         if (i - indexFrom > 1) {
//                             // Conform
//                             for (int j = indexFrom; j < i; ++j) {
//                                 if (rects.at(j).left() != conformTo) {
//                                     rects[j].setLeft(conformTo);
//                                     fixed = true;
//                                 }
//                             }
//                         }
//
//                         indexFrom = i;
//                         conformTo = rects.at(i).left();
//                     } else {
//                         conformTo = qMin(conformTo, rects.at(i).left());
//                     }
//                 }
//                 if (indexFrom != rects.size() - 1) {
//                     // Conform
//                     for (int j = indexFrom; j < rects.size(); ++j) {
//                         if (rects.at(j).left() != conformTo) {
//                             rects[j].setLeft(conformTo);
//                             fixed = true;
//                         }
//                     }
//                 }
//             }
//         }
//
//         // Merge rectangles into individual non-overlapping polygons
//         QPainterPath paths;
//         paths.setFillRule(Qt::WindingFill);
//         foreach (const QRectF & rect, rects) {
//             paths.addRect(rect);
//         }
//         QList< QPolygonF > polygons(paths.simplified().toSubpathPolygons());
//
//         // For each polygon...
//         QPainterPath outlines;
//         foreach (const QPolygonF & polygon, polygons) {
//             bool newPath = true;
//             // Round the polygon
//             QPolygonF poly(qRound(polygon, 2));
//             // Quick pass to remove co-liner points
//             for (int previousIndex = poly.size(); previousIndex > 0; --previousIndex) {
//                 // Get the three points of interest, and useful vectors
//                 const int poi = poly.size() - 1; // # of points of interest
//                 const int currentIndex = (previousIndex + 1) % poi;
//                 const int nextIndex = (previousIndex + 2) % poi;
//                 QPointF previous(poly.at(previousIndex % poi));
//                 QPointF current(poly.at(currentIndex));
//                 QPointF next(poly.at(nextIndex));
//                 QVector2D backwards(previous - current);
//                 QVector2D forwards(next - current);
//                 backwards.normalize();
//                 forwards.normalize();
//                 qreal angle(qAcos(QVector2D::dotProduct(forwards, backwards)));
//                 if (qAbs(angle - M_PI) < 0.001) {
//                     poly.remove(currentIndex);
//                     ++previousIndex;
//                 }
//             }
//
//             // Calculate concavity
//             QVector< bool > convexity(poly.size());
//             for (int previousIndex = poly.size(); previousIndex > 0; --previousIndex) {
//                 // Get the three points of interest, and useful vectors
//                 const int poi = poly.size() - 1; // # of points of interest
//                 const int currentIndex = (previousIndex + 1) % poi;
//                 const int nextIndex = (previousIndex + 2) % poi;
//                 QPointF previous(poly.at(previousIndex % poi));
//                 QPointF current(poly.at(currentIndex));
//                 QPointF next(poly.at(nextIndex));
//                 QVector2D backwards(previous - current);
//                 QVector2D forwards(next - current);
//
//                 // Concave or convex?
//                 // Uses the z coordinate of the cross product to decide concavity
//                 convexity[currentIndex] = (forwards.x() * backwards.y() - forwards.y() * backwards.x()) > 0;
//             }
//
//             for (int previousIndex = poly.size(); previousIndex > 0; --previousIndex) {
//                 // Get the three points of interest, and useful vectors
//                 const int poi = poly.size() - 1; // # of points of interest
//                 const int currentIndex = (previousIndex + 1) % poi;
//                 const int nextIndex = (previousIndex + 2) % poi;
//                 QPointF previous(poly.at(previousIndex % poi));
//                 QPointF current(poly.at(currentIndex));
//                 QPointF next(poly.at(nextIndex));
//                 QVector2D backwards(previous - current);
//                 QVector2D forwards(next - current);
//                 qreal backwardsLength(backwards.length());
//                 qreal forwardsLength(forwards.length());
//                 backwards.normalize();
//                 forwards.normalize();
//
//                 // Firstly work out the angle of this corner using the dot product
//                 qreal angle(qAcos(QVector2D::dotProduct(forwards, backwards)));
//
//                 // Things to calculate
//                 QPointF centre; // Centre of the corner arc
//                 qreal arc_start_angle = 0.0;
//                 qreal arc_sweep_length = M_PI - angle;
//                 qreal arc_radius = radius;
//
//                 // Concave or convex?
//                 // Uses the z coordinate of the cross product to decide concavity
//                 bool convex = (forwards.x() * backwards.y() - forwards.y() * backwards.x()) > 0;
//
//                 if (true) {
//                     // Next calculate the radius of this corner
//                     // The rounded corner cannot meet the edges a distance of more than half the
//                     // available space along a side from the corner
//                     //qreal d1 = backwardsLength / 2.0;
//                     //qreal d2 = forwardsLength / 2.0;
//                     qreal tan_half_angle = qTan(angle / 2.0);
//
//                     // Centre of arc
//                     QVector2D a((backwards + forwards).normalized());
//                     if (convex) {
//                         qreal max_l = 2 * radius / tan_half_angle;
//                         bool previousConcave(!convexity.at(previousIndex % poi));
//                         bool nextConcave(!convexity.at(nextIndex));
//                         qreal l = qMin(max_l, qMin(previousConcave ? backwardsLength : backwardsLength / 2.0,
//                                                    nextConcave ? forwardsLength : forwardsLength / 2.0));
//                         qreal r = l * tan_half_angle;
//                         qreal h = r / qSin(angle / 2.0);
//                         centre = current + h * a.toPointF();
//                         arc_radius = r - radius;
//                     } else {
//                         qreal h = radius / qSin(angle / 2.0);
//                         qreal r = radius; // qMin(radius, qMin(backwardsLength / 2.0, forwardsLength / 2.0));
//                         qreal x = h - r * h / radius;
//                         centre = current - x * a.toPointF();
//                         arc_radius = r;
//                     }
//
//                     // Arc
//                     arc_start_angle = qAtan2(backwards.x(), backwards.y()) + M_PI_2;
//                     if (!convex) {
//                         arc_sweep_length = -arc_sweep_length;
//                     }
//                 } else {
//                     // Arc
//                     arc_start_angle = qAtan2(backwards.x(), backwards.y()) + M_PI_2;
//                     arc_sweep_length = -arc_sweep_length;
//
//                     // Centre of arc
//                     centre = current;
//                 }
//
//                 arc_start_angle = arc_start_angle * 180.0 * M_1_PI;
//                 arc_sweep_length = arc_sweep_length * 180.0 * M_1_PI;
//                 QRectF arcRect(centre - QPointF(arc_radius, arc_radius), QSizeF(2 * arc_radius, 2 * arc_radius));
//
//                 if (newPath) {
//                     outlines.arcMoveTo(arcRect, arc_start_angle + arc_sweep_length);
//                     newPath = false;
//                 } else {
//                     outlines.arcTo(arcRect, arc_start_angle, arc_sweep_length);
//                 }
//             }
//             outlines.closeSubpath();
//         }
//
//         return outlines;
//     }

}

bool operator < (const QRectF & lhs, const QRectF & rhs)
{
    return lhs.top() < rhs.top()
                       || (lhs.top() == rhs.top() && lhs.left() < rhs.left())
        || (lhs.top() == rhs.top() && lhs.left() == rhs.left() && lhs.bottom() < rhs.bottom())
        || (lhs.top() == rhs.top() && lhs.left() == rhs.left() && lhs.bottom() == rhs.bottom() && lhs.right() < rhs.right());
}

namespace Papyro
{

    /// PageViewRenderThread //////////////////////////////////////////////////////////////////////

    PageViewRenderThread::PageViewRenderThread(PageView * pageView)
        : QThread(pageView), _pageView(pageView), _dirty(false)
    {
        QObject::connect(this, SIGNAL(finished()), pageView, SLOT(renderThreadFinished()));
    }

    PageViewRenderThread::~PageViewRenderThread() {}

    bool PageViewRenderThread::isDirty()
    {
        QMutexLocker lock(&this->_mutex);
        return this->_dirty;
    }

    QImage PageViewRenderThread::image()
    {
        QMutexLocker lock(&this->_mutex);
        return this->_image;
    }

    void PageViewRenderThread::run()
    {
        // Get parameters of this rendering
        QSize size;
        QColor paper;
        this->getTarget(&size, &paper);
        Spine::Image i;

        // Render
        {
            QMutexLocker lock(&this->_globalMutex);
            if (this->_pageView) {
              i = this->_pageView->page()->render(size_t(size.width()),
                                                  size_t(size.height()));
            }
        }

        // Set image
        QMutexLocker lock(&this->_mutex);
        this->_image = qImageFromSpineImage(&i);
    }

    void PageViewRenderThread::setTarget(QSize size, QColor color)
    {
        QMutexLocker lock(&this->_mutex);
        if (size != this->_size ||
            color != this->_color)
        {
            this->_dirty = true;
        }
        this->_size = size;
        this->_color = color;
    }

    void PageViewRenderThread::getTarget(QSize * size, QColor * color)
    {
        QMutexLocker lock(&this->_mutex);
        *size = this->_size * Utopia::retinaScaling();
        *color = this->_color;
        this->_dirty = false;
    }

    QMutex PageViewRenderThread::_globalMutex;




    /// PageViewPrivate ///////////////////////////////////////////////////////////////////////////

    PageViewPrivate::PageViewPrivate(PageView * pageView)
        : QObject(pageView),
          pageView(pageView),
          userTransformDegrees(0),
          rotateMapper(0),
          rotateMenu(0),
          renderThread(new PageViewRenderThread(pageView)),
          imageCache(QString(":page-cache:%1").arg((qlonglong) pageView->window())),
          dragging(false),
          multiClick(false),
          tripleClick(false),
          imageFormatManager(Utopia::ImageFormatManager::instance())
    {
        // Seems reasonable that no more than 20 pages will be visible at once
        // in a single window
        imageCache.setMaximumSize(20);
    }

    void PageViewPrivate::browseUrl(const QString & url, const QString & target)
    {
        browseUrl(QUrl(url), target);
    }

    void PageViewPrivate::browseUrl(const QUrl & url, const QString & target)
    {
        emit urlRequested(url, target);
    }

    QRectF PageViewPrivate::mediaRect() const
    {
        Spine::BoundingBox bb(cursor->page()->mediaBox());
        return QRectF(bb.x1, bb.y1, bb.x2-bb.x1, bb.y2-bb.y1);
    }

    QSizeF PageViewPrivate::mediaSize() const
    {
        return mediaRect().size();
    }

    QRectF PageViewPrivate::pageRect() const
    {
        Spine::BoundingBox bb(cursor->page()->boundingBox());
        return QRectF(bb.x1, bb.y1, bb.x2-bb.x1, bb.y2-bb.y1);
    }

    QSizeF PageViewPrivate::pageSize() const
    {
        return pageRect().size();
    }

    // Set interaction state for mouse press
    void PageViewPrivate::setMousePressPos(const QPoint & pos)
    {
        mousePressPos = pos;
        mousePressPagePos = pageView->transformToPage(mousePressPos);
        mousePressTime = QTime::currentTime();
        setMouseMovePos(pos);
    }

    // Set interaction state for mouse move
    void PageViewPrivate::setMouseMovePos(const QPoint & pos)
    {
        mouseMovePos = pos;
        mouseMovePagePos = pageView->transformToPage(mouseMovePos);
    }

    // Set interaction state for mouse release
    void PageViewPrivate::setMouseReleasePos(const QPoint & pos)
    {
        mouseReleasePos = pos;
        mouseReleasePagePos = pageView->transformToPage(mouseReleasePos);
    }

    static QTransform generateTransform(int degrees, const QSizeF & canvasSize)
    {
        QTransform rotation;
        rotation.rotate(degrees);
        QRectF rotatedRect(rotation.mapRect(QRectF(QPointF(0, 0), canvasSize)));
        QTransform translation;
        translation.translate(-rotatedRect.left(), -rotatedRect.top());

        return rotation * translation;
    }

    void PageViewPrivate::setUserTransform(int degrees)
    {
        userTransformDegrees = degrees;
        userTransform = generateTransform(degrees, pageSize());
        userTransformInverse = userTransform.inverted();

        transformedPageRect = applyUserTransform(pageRect());
    }

    QPointF PageViewPrivate::applyUserTransform(const QPointF & point)
    {
        return userTransform.map(point);
    }

    QRectF PageViewPrivate::applyUserTransform(const QRectF & rect)
    {
        return userTransform.mapRect(rect);
    }

    QSizeF PageViewPrivate::applyUserTransform(const QSizeF & size)
    {
        return userTransform.mapRect(QRectF(QPointF(0, 0), size)).size();
    }

    QPointF PageViewPrivate::unapplyUserTransform(const QPointF & point)
    {
        return userTransformInverse.map(point);
    }

    QRectF PageViewPrivate::unapplyUserTransform(const QRectF & rect)
    {
        return userTransformInverse.mapRect(rect);
    }

    QSizeF PageViewPrivate::unapplyUserTransform(const QSizeF & size)
    {
        return userTransformInverse.mapRect(QRectF(QPointF(0, 0), size)).size();
    }




    PageView::PageView(QWidget * parent, Qt::WindowFlags f)
        : QWidget(parent, f), d(new PageViewPrivate(this))
    {
        _initialise();
    }

    PageView::PageView(Spine::DocumentHandle document, int page, QWidget * parent, Qt::WindowFlags f)
        : QWidget(parent, f), d(new PageViewPrivate(this))
    {
        _initialise();
        setPage(document, page);
    }

    PageView::~PageView()
    {
        // Delete phrase lookup extensions
        QListIterator< PhraseLookup * > p_iter(d->phraseLookups);
        while (p_iter.hasNext())
        {
            delete p_iter.next();
        }

        clear();
    }

    Spine::TextExtentHandle PageView::activeSpotlight() const
    {
        return d->activeSpotlight;
    }

    QPainterPath PageView::asPath(const Spine::TextExtentHandle & extent, int pageNumber)
    {
        QPainterPath path;

        // Set sentinels
        Spine::CursorHandle start = extent->first.cursor()->clone();
        Spine::CursorHandle end = extent->second.cursor()->clone();

        // Iterate over lines
        Spine::CursorHandle line = start->clone();
        Spine::CursorHandle prevLine = start->clone();
        QRectF prevLineRect;
        float prevLineBase = 0.0;
        while (line->line() && *line <= *end)
        {
            // Ignore lines not on this page FIXME
            if (line->page()->pageNumber() == pageNumber)
            {
                // Reset previous line information if now in different block
                if (line->block() != prevLine->block())
                {
                    prevLineRect = QRectF();
                    prevLineBase = 0.0;
                }

                // Get bounds of line
                Spine::BoundingBox bb = line->line()->boundingBox();
                QRectF lineRect(QRectF(bb.x1, bb.y1, bb.x2-bb.x1, bb.y2-bb.y1).adjusted(0, -0.5, 0, 0.5));
                if (!prevLineRect.isNull() &&
                    overlapHorizontally(lineRect, prevLineRect) &&
                    overlapVertically(lineRect, prevLineRect, 2.0))
                {
                    prevLineBase = prevLineRect.bottom();
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
                                    if (prevLineBase)
                                    {
                                        characterRect.setTop(prevLineBase);
                                    }

                                    if (!prevWordRect.isNull() &&
                                        *character == *word)
                                    {
                                        characterRect.setLeft(prevWordRect.right());
                                    }

                                    path.addRect(characterRect);

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
                                        if (prevLineBase)
                                        {
                                            spaceRect.setTop(prevLineBase);
                                        }

                                        path.addRect(spaceRect);
                                    }
                                    break;
                                }
                            }
                        }
                        else
                        {
                            // Otherwise add the whole word
                            if (prevLineBase)
                            {
                                wordRect.setTop(prevLineBase);
                            }
                            // Deal with extraneous spaces
                            Spine::CursorHandle nextWord(word->clone()); nextWord->nextWord();
                            if (word->word()->spaceAfter() && nextWord->word())
                            {
                                wordRect.setRight(nextWord->word()->boundingBox().x1);
                            }
                            path.addRect(wordRect);
                        }

                        // Move on to the next word in the selection
                        prevWordRect = wordRect;
                        word->nextWord();
                    }
                }
                else
                {
                    // Otherwise add the whole line
                    if (prevLineBase)
                    {
                        lineRect.setTop(prevLineBase);
                    }
                    path.addRect(lineRect);
                }

                // Move on to the next line in the selection
                prevLineRect = lineRect;
                prevLine = line->clone();
            }

            line->nextLine(Spine::WithinDocument);
        }

        // Set fill rule to UNION and return
        path.setFillRule(Qt::WindingFill);
        return path.simplified();
    }

    QPainterPath PageView::asPath(const Spine::TextSelection & selection, int pageNumber)
    {
        QPainterPath path;

        BOOST_FOREACH(const Spine::TextExtentHandle & extent, selection)
        {
            path.addPath(asPath(extent, pageNumber));
        }

        // Set fill rule to UNION and return
        path.setFillRule(Qt::WindingFill);
        return path.simplified();
    }

    void PageView::clear()
    {
        // Disconnect from model
        d->documentProxy.reset();

        // Wait for render thread
        d->renderThread->wait();

        // Clear overlays
        clearSpotlights();
        clearTemporaryFocus();

        QMapIterator< Spine::AnnotationHandle, EmbeddedFrame * > frames(d->embeddedWidgets);
        while (frames.hasNext())
        {
            frames.next();
            delete frames.value();
        }
        QMapIterator< Spine::AnnotationHandle, QWidget * > linkedWidgets(d->linkedWidgets);
        while (linkedWidgets.hasNext())
        {
            linkedWidgets.next();
            delete linkedWidgets.value();
        }
        d->embeddedWidgets.clear();
        d->linkedWidgets.clear();
        d->embeddedRects.clear();

        // Clear image cache
        d->imageCache.clear();

        // Zero state
        d->previousSelectedImageCursor.reset();
        d->conversationWidget.reset();

        // Reset model pointer
        d->cursor.reset();
        d->document.reset();
    }

    void PageView::clearActiveSpotlight()
    {
        setActiveSpotlight(Spine::TextExtentHandle());
    }

    void PageView::clearSpotlights()
    {
        setSpotlights(Spine::TextExtentSet());
    }

    void PageView::clearTemporaryFocus()
    {
        d->temporaryFocusExtents.clear();
        d->temporaryFocusAreas.clear();
        d->temporaryFocusHidden = false;
        d->temporaryFocus = QPainterPath();
    }

    void PageView::contextMenuEvent(QContextMenuEvent * event)
    {
        // Create empty menu
        QMenu menu(this);

        // Add page view's menu items
        populateContextMenuAt(&menu, event->pos());

        // Execute menu
        menu.exec(event->globalPos());
    }

    void PageView::copyEmailAddress()
    {
        QString address(sender()->property("__u_textToCopy").toString());
        if (!address.isEmpty())
        {
            QApplication::clipboard()->setText(address, QClipboard::Clipboard);
        }
    }

    void PageView::copySelectedText()
    {
        std::string text (document()->selectionText());
        if (!text.empty())
        {
            QClipboard * clipboard = QApplication::clipboard();
            clipboard->setText(qStringFromUnicode(text), QClipboard::Clipboard);
        }
    }

    Spine::CursorHandle PageView::cursorAt(const QPointF & point, Spine::DocumentElement element_) const
    {
        // Drill down from page cursor
        Spine::CursorHandle cursor = newCursor();

        // Ignore images if image Based
        while (const Spine::Image * image = cursor->image())
        {
            if (!document()->imageBased() && contains(image->boundingBox(), point))
            {
                break;
            }
            cursor->nextImage();
        }

        if (cursor->image() == 0)
        {
            while (const Spine::Region * region = cursor->region())
            {
                if (contains(region->boundingBox(), point))
                {
                    if (element_ == Spine::ElementRegion)
                        return cursor;

                    while (const Spine::Block * block = cursor->block())
                    {
                        if (contains(block->boundingBox(), point))
                        {
                            if (element_ == Spine::ElementBlock)
                                return cursor;

                            while (const Spine::Line * line = cursor->line())
                            {
                                if (contains(line->boundingBox(), point))
                                {
                                    if (element_ == Spine::ElementLine)
                                        return cursor;

                                    while (const Spine::Word * word = cursor->word())
                                    {
                                        if (contains(word->boundingBox(), point))
                                        {
                                            if (element_ == Spine::ElementWord)
                                                return cursor;

                                            while (const Spine::Character * character = cursor->character())
                                            {
                                                if (contains(character->boundingBox(), point))
                                                {
                                                    return cursor;
                                                }
                                                cursor->nextCharacter();
                                            }
                                        }
                                        cursor->nextWord();
                                    }
                                }
                                cursor->nextLine();
                            }
                        }
                        cursor->nextBlock();
                    }
                }
                cursor->nextRegion();
            }
        }

        return cursor;
    }

    void PageView::deleteAnnotation(const QString & uri)
    {
        std::set< Spine::AnnotationHandle > annotations = document()->annotationsById(unicodeFromQString(uri));
        qDebug() << "deleting" << uri << annotations.size();
        if (annotations.size() > 0)
        {
            BOOST_FOREACH(Spine::AnnotationHandle annotation, annotations)
            {
                document()->addAnnotation(annotation, document()->deletedItemsScratchId());
            }
            qDebug() << "deleting" << document()->annotations(document()->deletedItemsScratchId()).size();
            emit publishChanges();
            qDebug() << "deleting" << document()->annotations(document()->deletedItemsScratchId()).size();
            std::set< Spine::AnnotationHandle > failed = document()->annotations(document()->deletedItemsScratchId());
            BOOST_FOREACH(Spine::AnnotationHandle annotation, annotations)
            {
                if (failed.find(annotation) == failed.end())
                {
                    document()->removeAnnotation(annotation);
                }
            }
        }
    }

    Spine::DocumentHandle PageView::document() const
    {
        return d->document;
    }

    void PageView::dropEvent(QDropEvent * /* event */ )
    {
    }

    void PageView::dragEnterEvent(QDragEnterEvent * /* event */)
    {
    }

    void PageView::dragMoveEvent(QDragMoveEvent * /* event */)
    {
    }

    bool PageView::event(QEvent * event)
    {
        static QTime clickWhen;
        static QTime doubleClickWhen;

        QTime thingy;

        switch (event->type())
        {
        case QEvent::ToolTip:
        {
            QHelpEvent * helpEvent = static_cast<QHelpEvent *>(event);
            QPointF pagePos(transformToPage(helpEvent->pos()));

            QString toolTipText;
            std::set< Spine::AnnotationHandle > annotations(document()->annotationsAt(pageNumber(), pagePos.x(), pagePos.y()));
            BOOST_FOREACH(Spine::AnnotationHandle annotation, annotations)
            {
                std::string value (annotation->getFirstProperty("displayTooltip"));
                if (!value.empty())
                {
                    if (!toolTipText.isEmpty())
                    {
                        toolTipText += "\n";
                    }
                    toolTipText += qStringFromUnicode(value);
                }
            }
            toolTipText = toolTipText.trimmed();
            if (toolTipText.isEmpty())
            {
                QToolTip::hideText();
            }
            else
            {
                QToolTip::showText(helpEvent->globalPos(), toolTipText);
            }
            break;
        }
        default:
            return QWidget::event(event);
        }

        return true;
    }

    void PageView::executePhraseLookup(int idx)
    {
        if (d->phraseLookups.size() > idx)
        {
            PhraseLookup * phraseLookup = d->phraseLookups.at(idx);
            QString url = qStringFromUnicode(phraseLookup->lookup(unicodeFromQString(d->lookupPhrase)));
            d->browseUrl(url);
        }
    }

    void PageView::hideSpotlights(bool hide)
    {
        if (d->spotlightsHidden != hide)
        {
            d->spotlightsHidden = hide;
            update();
        }
    }

    double PageView::horizontalZoom() const
    {
        if (isNull())
        {
            return 1;
        }
        else
        {
            return width() / pageSize(true).width();
        }
    }

    Spine::CursorHandle PageView::imageCursorAt(const QPointF & point) const
    {
        Spine::CursorHandle cursor = newCursor();
        Spine::CursorHandle first;

        while (const Spine::Image * image = cursor->image())
        {
            if (!document()->imageBased() && contains(image->boundingBox(), point))
            {
                // Remember first
                if (!first) { first = cursor->clone(); }

                // Only break once we've passed under
                if (d->previousSelectedImageCursor && *d->previousSelectedImageCursor == *cursor)
                {
                    d->previousSelectedImageCursor.reset();
                }
                else if (!d->previousSelectedImageCursor)
                {
                    break;
                }
            }
            cursor->nextImage();
        }

        if (cursor->image() == 0 && first) { cursor = first; }

        d->previousSelectedImageCursor = cursor;

        return cursor;
    }

    void PageView::_initialise()
    {
        // Passthru
        connect(d, SIGNAL(urlRequested(const QUrl &, const QString &)), this, SIGNAL(urlRequested(const QUrl &, const QString &)));

        // Phrase lookups
        std::set< PhraseLookup * > phraseLookups = Utopia::instantiateAllExtensions< PhraseLookup >();
        BOOST_FOREACH(PhraseLookup * lookup, phraseLookups)
        {
            d->phraseLookups.append(lookup);
        }
        d->phraseLookupMapper = new QSignalMapper(this);
        QObject::connect(d->phraseLookupMapper, SIGNAL(mapped(int)), this, SLOT(executePhraseLookup(int)));
        d->phraseLookupMenu = new QMenu(this);
        {
            QMap< QString, int > idxByName;
            int idx = 0;
            BOOST_FOREACH(PhraseLookup * lookup, d->phraseLookups)
            {
                QString title = qStringFromUnicode(lookup->title());
                if (title.indexOf(QRegExp("\\d")) != 0)
                {
                    title.prepend("5");
                }
                idxByName[title] = idx;
                ++idx;
            }

            QMapIterator< QString, int > entry(idxByName);
            QChar prev;
            while (entry.hasNext())
            {
                entry.next();
                int idx = entry.value();
                if (!prev.isNull() && entry.key()[0] != prev)
                {
                    d->phraseLookupMenu->addSeparator();
                }
                QRegExp digits("\\d");
                QString title = entry.key();
                title.indexOf(digits);
                title = title.mid(digits.matchedLength());
                d->phraseLookupMapper->setMapping(d->phraseLookupMenu->addAction(title, d->phraseLookupMapper, SLOT(map())), idx);
                prev = entry.key()[0];
            }
        }

        // Browse mapper
        d->browseToMapper = new QSignalMapper(this);
        QObject::connect(d->browseToMapper, SIGNAL(mapped(QString)), d, SLOT(browseUrl(QString)));

        d->deleteAnnotationMapper = new QSignalMapper(this);
        QObject::connect(d->deleteAnnotationMapper, SIGNAL(mapped(QString)), this, SLOT(deleteAnnotation(QString)));

        // Set up widget
        setContextMenuPolicy(Qt::CustomContextMenu);
        setFocusPolicy(Qt::ClickFocus);
        setMouseTracking(true);
        setAutoFillBackground(true);

        // Selection setup
        d->interactionMode = NoInteraction;

        d->spotlightsHidden = true;

    }

    bool PageView::isNull() const
    {
        return page() == 0;
    }

    void PageView::leaveEvent(QEvent * event)
    {
        update();
    }

    QRectF PageView::mediaRect(bool transformed) const
    {
        if (transformed) {
            return d->applyUserTransform(d->mediaRect());
        } else {
            return d->mediaRect();
        }
    }

    QSizeF PageView::mediaSize(bool transformed) const
    {
        return mediaRect(transformed).size();
    }

/*********
                    else if (concept == "Chemical")
                    {
                        QPixmap thumbnail(QPixmap::fromImage(QImage::fromData(QByteArray::fromBase64(qStringFromUnicode(annotation->getFirstProperty("property:thumbnail")).toUtf8()))));
                        QString name(qStringFromUnicode(annotation->getFirstProperty("property:name")));
                        QString url(qStringFromUnicode(annotation->getFirstProperty("property:webpageUrl")));
                        QString inchi(qStringFromUnicode(annotation->getFirstProperty("property:inchi")));
                        QString smiles(qStringFromUnicode(annotation->getFirstProperty("property:smiles")));
                        //qDebug() << thumbnail.isNull() << thumbnail.size() << name << url << inchi << smiles;
                        if (!thumbnail.isNull() && !name.isEmpty() && !inchi.isEmpty() && !url.isEmpty())
                        {
                            QLabel * pixmapLabel = new QLabel;
                            pixmapLabel->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
                            pixmapLabel->setPixmap(thumbnail);
                            QLabel * logoLabel = new QLabel;
                            logoLabel->setAlignment(Qt::AlignRight | Qt::AlignBottom);
                            logoLabel->setPixmap(QPixmap(":/icons/chemspider.png"));
                            if (inchi.size() > 32)
                            {
                                inchi = inchi.mid(0, 29) + "...";
                            }
                            QLabel * infoLabel = new QLabel(QString("<a href=\"%1\">%2</a>").arg(url).arg(inchi));
                            infoLabel->setAlignment(Qt::AlignCenter);
                            infoLabel->setOpenExternalLinks(true);
                            Utopia::BubbleWidget * bubble = new Utopia::BubbleWidget;
                            bubble->setWindowTitle(name);
                            QVBoxLayout * popupLayout = new QVBoxLayout(bubble);
                            QHBoxLayout * pixmapLayout = new QHBoxLayout;
                            pixmapLayout->addStretch(1);
                            pixmapLayout->addWidget(logoLabel);
                            pixmapLayout->addWidget(pixmapLabel);
                            pixmapLayout->addStretch(1);
                            popupLayout->addLayout(pixmapLayout);
                            popupLayout->addWidget(infoLabel);
                            bubble->setAttribute(Qt::WA_DeleteOnClose, true);
                            bubble->setPopup(true);
                            bubble->show(event->globalPos());
                            d->repressLookup = true;
                        }
                    }
**********/

    Spine::CursorHandle PageView::newCursor() const
    {
        return d->cursor->clone();
    }

    void PageView::onMousePressTimeout()
    {
    }

    const Spine::Page * PageView::page() const
    {
        return d->cursor->page();
    }

    QPixmap PageView::pageImage(QSize size, QColor paper)
    {
        QPixmap pageImage = d->imageCache.get(d->cacheName);

        if (size != pageImage.size()) {
            d->renderThread->setTarget(size, paper);
            bool running = d->renderThread->isRunning();
            if (!running) {
                d->renderThread->start();
            }
        }

        return pageImage;
    }

    int PageView::pageNumber() const
    {
        return page()->pageNumber();
    }

    QRectF PageView::pageRect(bool transformed) const
    {
        if (transformed) {
            return d->transformedPageRect;
        } else {
            return d->pageRect();
        }
    }

    QSizeF PageView::pageSize(bool transformed) const
    {
        return pageRect(transformed).size();
    }

    void PageView::paintEvent(QPaintEvent * event)
    {
        QSizeF pSize(pageSize(true));
        QPainter painter(this);

        if (isNull())
        {
            // FIXME Draw invalid page indicator
        }
        else
        {
            // Find the size of the page (before user transform) in screen
            // coordinates
            QSize pImageSize = d->unapplyUserTransform(size()).toSize();
            QPixmap pImage = pageImage(d->unapplyUserTransform(size()).toSize());

            if (pImage.isNull())
            {
                // Draw "PDF" while page is being rendered
                painter.setRenderHint(QPainter::Antialiasing);
                painter.fillRect(rect(), Qt::white);
                QRect spinner = rect();
                if (spinner.width() > spinner.height())
                {
                    spinner.setWidth(spinner.height());
                }
                else
                {
                    spinner.setHeight(spinner.width());
                }
                spinner.moveCenter(rect().center());

                QFont font(painter.font());
                font.setPointSize(spinner.height() / 7);
                font.setBold(true);
                painter.setFont(font);
                painter.setPen(QColor(0, 0, 0, 50));
                painter.drawText(spinner, Qt::AlignCenter, "PDF");

                // If active spotlight, grey out
                if (!d->spotlightsHidden && !d->darkness.isEmpty())
                {
                    painter.setCompositionMode(QPainter::CompositionMode_Multiply);
                    painter.setPen(Qt::NoPen);
                    painter.setBrush(QColor(0, 0, 0, 100));
                    painter.drawRect(rect().adjusted(0, 0, 1, 1));
                }
            }
            else
            {
                QTransform transform(generateTransform(d->userTransformDegrees, pImageSize));
                painter.setTransform(transform, true);

                // Render the page image to the widget
                painter.save();
                painter.setRenderHint(QPainter::SmoothPixmapTransform);
                foreach (QRect rect, event->region().rects()) {
                    rect = transform.mapRect(rect);
                    painter.drawPixmap(rect, pImage, rect);
                }
                painter.drawPixmap(QRect(QPoint(0, 0), pImageSize), pImage);
                painter.restore();

                // Scale to current zoom
                painter.scale(width() / (double) pSize.width(),
                              height() / (double) pSize.height());
                //painter.translate(-0.5, -0.5);

                // Antialias
                painter.setRenderHint(QPainter::Antialiasing, true);
                painter.setRenderHint(QPainter::TextAntialiasing, true);
                painter.setPen(Qt::NoPen);
                painter.setBrush(QColor(255, 0, 0, 80));
                painter.setCompositionMode(QPainter::CompositionMode_Multiply);

                if (!d->spotlightsHidden) {
                    // Render search spotlights
                    painter.save();
                    painter.setCompositionMode(QPainter::CompositionMode_Multiply);

                    painter.setPen(Qt::NoPen);
                    painter.setBrush(QColor(0, 0, 0, 50));
                    painter.drawPath(d->darkness);

                    painter.setPen(QColor(140, 140, 0));
                    QPen pen(painter.pen());
                    pen.setWidth(2);
                    painter.setPen(pen);
                    painter.setBrush(QColor(255, 255, 0, 200));
                    painter.drawPath(d->bubble);
                    painter.restore();
                }

                if (!d->temporaryFocusHidden) {
                    painter.save();
                    painter.setCompositionMode(QPainter::CompositionMode_Multiply);
                    painter.setPen(QColor(0, 200, 0, 220));
                    painter.setBrush(QColor(0, 220, 0, 80));
                    painter.drawPath(d->temporaryFocus);
                    painter.restore();
                }
            }
        }
    }

    void PageView::populateContextMenuAt(QMenu * menu, const QPoint & pos)
    {
        if (d->rotateMenu) {
            // Rotate page
            menu->addMenu(d->rotateMenu);
            menu->addSeparator();
        }
    }

    void PageView::recomputeDarkness()
    {
        // Darken whole page
        d->darkness.addRect(pageRect().translated(-pageRect().topLeft()).adjusted(0, 0, 1, 1));

        if (spotlights().size() > 0)
        {
            // Find punched out holes
            QPainterPath holes;

            BOOST_FOREACH(const Spine::TextExtentHandle & extent, spotlights())
            {
                if (extent->first.cursor()->page()->pageNumber() > pageNumber() ||
                    extent->second.cursor()->page()->pageNumber() < pageNumber())
                {
                    continue;
                }

                double radius = 2;

                BOOST_FOREACH(const Spine::Area & box, extent->areas())
                {
                    int page = box.page;
                    Spine::BoundingBox bb = box.boundingBox;
                    if (page == pageNumber())
                    {
                        QRectF hole(bb.x1, bb.y1, bb.x2-bb.x1, bb.y2-bb.y1);
                        hole.adjust(-radius+1, -radius+1, radius-1, radius-1);
                        holes.addRoundedRect(hole, radius, radius);
                    }
                }
            }

            holes.setFillRule(Qt::WindingFill);
            d->darkness -= holes;
        }
    }

    void PageView::recomputeTemporaryFocus()
    {
        //qDebug() << "recomputeTemporaryFocus()" << d->temporaryFocusExtents.size() << d->temporaryFocusAreas.size();
        d->temporaryFocus = QPainterPath();
        double radius = 2;

        // Highlight any focuses
        BOOST_FOREACH(const Spine::TextExtentHandle & extent, d->temporaryFocusExtents)
        {
            if (extent->first.cursor()->page()->pageNumber() > pageNumber() ||
                extent->second.cursor()->page()->pageNumber() < pageNumber())
            {
                continue;
            }

            BOOST_FOREACH(const Spine::Area & box, extent->areas())
            {
                int page = box.page;
                Spine::BoundingBox bb = box.boundingBox;
                if (page == pageNumber())
                {
                    d->temporaryFocus.addRoundedRect(QRectF(bb.x1, bb.y1, bb.x2-bb.x1, bb.y2-bb.y1).adjusted(-radius+1, -radius+1, radius-1, radius-1), radius, radius);
                }
            }
        }

        BOOST_FOREACH(const Spine::Area & area, d->temporaryFocusAreas)
        {
            if (area.page == pageNumber())
            {
                Spine::BoundingBox bb = area.boundingBox;
                d->temporaryFocus.addRoundedRect(QRectF(bb.x1, bb.y1, bb.x2-bb.x1, bb.y2-bb.y1).adjusted(-radius+1, -radius+1, radius-1, radius-1), radius, radius);
            }
        }

        d->temporaryFocus.setFillRule(Qt::WindingFill);
    }

    void PageView::renderThreadFinished()
    {
        QPixmap pageImage = QPixmap::fromImage(d->renderThread->image());
        d->imageCache.put(pageImage, d->cacheName);
        update();
    }

    void PageView::resizeEvent(QResizeEvent * event)
    {
        QWidget::resizeEvent(event);
        // Placeholder for when we'll later need to resize embedded widgets

        QMapIterator< Spine::AnnotationHandle, EmbeddedFrame * > frames(d->embeddedWidgets);
        while (frames.hasNext())
        {
            frames.next();
            EmbeddedFrame * frame = frames.value();
            frame->setGeometry(transformFromPage(frame->bounds()).adjusted(0, 0, 0, frame->controlSize().height()));
        }

        update();
    }

    void PageView::resizeToHeight(int h)
    {
        if (isNull())
        {
            setFixedSize(width(), h);
        }
        else
        {
            setZoom(h / pageSize(true).height());
        }
    }

    void PageView::resizeToSize(const QSize & size)
    {
        if (isNull())
        {
            setFixedSize(size);
        }
        else
        {
            QSizeF pSize(pageSize(true));
            double pageAspect = pSize.height() / pSize.width();
            double rectAspect = size.height() / size.width();

            if (pageAspect > rectAspect)
            {
                resizeToHeight(size.height());
            }
            else
            {
                resizeToWidth(size.width());
            }
        }
    }

    void PageView::resizeToWidth(int w)
    {
        if (isNull())
        {
            setFixedSize(w, height());
        }
        else
        {
            setZoom(w / pageSize(true).width());
        }
    }

    void PageView::saveImageAs()
    {
/*
        const Spine::Image * i = 0;
        if (d->selection.activeImage && d->selection.activeImage->boundingBox().contains(d->mousePressPagePos.x(), d->mousePressPagePos.y()))
        {
            i = d->selection.activeImage;
        }
        else
        {
            i = cursorAt(transformToPage(d->mousePressPos))->image();
        }

        // Only save if there's an image under the cursor
        if (i)
        {
            // Create QPixmap of the Spine image
            QImage image(qImageFromSpineImage(i));
            Utopia::ImageFormatManager::saveImageFile(this, "Save Image As...", QPixmap::fromImage(image), "Image Copy");
        }
*/ // FIXME
    }

    void PageView::setActiveSpotlight(Spine::TextExtentHandle extent)
    {
        d->activeSpotlight = extent;
        d->bubble = QPainterPath();
        d->spotlightsHidden = false;

        if (extent)
        {
            double radius = 4;

            BOOST_FOREACH(const Spine::Area & box, extent->areas())
            {
                int page = box.page;
                Spine::BoundingBox bb = box.boundingBox;
                if (page == pageNumber())
                {
                    d->bubble.addRoundedRect(QRectF(bb.x1, bb.y1, bb.x2-bb.x1, bb.y2-bb.y1).adjusted(-radius+2, -radius+2, radius-2, radius-2), radius-1, radius-1);
                }
            }

            d->bubble.setFillRule(Qt::WindingFill);
        }

        update();
    }

    void PageView::setHorizontalZoom(double zoom)
    {
        if (!isNull())
        {
            setFixedSize(QSize((int) (pageSize(true).width() * zoom), height()));
        }
    }

    void PageView::setPage(Spine::DocumentHandle document, int page)
    {
        // Clear completely the page view's document-related members
        clear();

        // Set the new document and hook up signals
        d->document = document;
        if (d->document) {
            d->documentProxy.reset(new DocumentProxy(this));
            d->documentProxy->setDocument(d->document);
            connect(d->documentProxy.get(), SIGNAL(annotationsChanged(const std::string &, const Spine::AnnotationSet &, bool)),
                    this, SLOT(updateAnnotations(const std::string &, const Spine::AnnotationSet &, bool)));
            connect(d->documentProxy.get(), SIGNAL(areaSelectionChanged(const std::string &, const Spine::AreaSet &, bool)),
                    this, SLOT(updateAreaSelection(const std::string &, const Spine::AreaSet &, bool)));
            connect(d->documentProxy.get(), SIGNAL(textSelectionChanged(const std::string &, const Spine::TextExtentSet &, bool)),
                    this, SLOT(updateTextSelection(const std::string &, const Spine::TextExtentSet &, bool)));
        }
        d->cursor = document->newCursor();

        setPage(page);
    }

    void PageView::setPage(int pageNumber)
    {
        d->cursor->gotoPage(pageNumber);
        d->setUserTransform(0);

        updateAnnotations(std::string(), Spine::AnnotationSet(), true);
        update();

        if (d->rotateMapper) { delete d->rotateMapper; }
        d->rotateMapper = new QSignalMapper(this);
        QObject::connect(d->rotateMapper, SIGNAL(mapped(int)), this, SLOT(setRotation(int)));
        if (d->rotateMenu) { delete d->rotateMenu; }
        d->rotateMenu = new QMenu("Rotate Page", this);
        QActionGroup * rotateActionGroup = new QActionGroup(this);
        QAction * defaultAction = 0;
        for (int i = 0; i < 4; ++i) {
            int degrees = 90 * i;
            QAction * action = d->rotateMenu->addAction(QString("%1 Degrees").arg(degrees), d->rotateMapper, SLOT(map()));
            action->setCheckable(true);
            if (degrees == page()->rotation()) {
                defaultAction = action;
                action->setChecked(true);
            }
            action->setActionGroup(rotateActionGroup);
            degrees = (degrees - page()->rotation()) % 360;
            d->rotateMapper->setMapping(action, degrees);
        }
        if (defaultAction) {
            d->rotateMenu->addSeparator();
            QAction * resetAction = d->rotateMenu->addAction("Reset Rotation", defaultAction, SLOT(trigger()));
            resetAction->setEnabled(false);
            connect(defaultAction, SIGNAL(toggled(bool)), resetAction, SLOT(setDisabled(bool)));
        }

        d->cacheName = QString("%1-%2").arg(pageNumber).arg((qulonglong) document().get());
    }

    void PageView::setRotation(int degrees)
    {
        double hZ = horizontalZoom();
        d->setUserTransform(degrees);
        setZoom(hZ);
        emit pageRotated();
        update();
    }

    void PageView::setSpotlights(const Spine::TextExtentSet & extents)
    {
        d->spotlights.clear();
        d->darkness = QPainterPath();
        d->spotlightsHidden = false;

        BOOST_FOREACH(Spine::TextExtentHandle extent, extents)
        {
            if (extent->first.cursor()->page()->pageNumber() <= pageNumber() &&
                extent->second.cursor()->page()->pageNumber() >= pageNumber())
            {
                d->spotlights.insert(extent);
            }
        }
        clearActiveSpotlight();

        if (extents.size() > 0)
        {
            d->temporaryFocusHidden = true;
            recomputeDarkness();
        }
        update();
    }

    void PageView::setVerticalZoom(double zoom)
    {
        if (!isNull())
        {
            setFixedSize(QSize(width(), (int) (pageSize(true).height() * zoom)));
        }
    }

    void PageView::setZoom(double zoom)
    {
        if (!isNull())
        {
            QSizeF size(pageSize(true) * zoom);
            setFixedSize(size.toSize());
        }
    }

    const Spine::TextExtentSet & PageView::spotlights() const
    {
        return d->spotlights;
    }

    static Spine::CursorHandle resolveCursor(Spine::CursorHandle cursor, const QPointF & point, qreal grace = 0.0)
    {
        if (cursor && cursor->word())
        {
            // Mouse over character
            if (cursor->character())
            {
                // If more than half-way into the character, adjust to include
                if (contains(cursor->line()->boundingBox(), point, grace) &&
                    point.x() >= (cursor->character()->boundingBox().x1 + cursor->character()->boundingBox().x2) / 2.0)
                {
                    cursor->nextCharacter();
                }
            }
            // Mouse between characters
            else
            {
                float spaceLeft = cursor->word()->boundingBox().x2;
                float spaceRight = spaceLeft;
                Spine::CursorHandle nextWord(cursor->clone()); nextWord->nextWord();
                if (nextWord->word())
                {
                    spaceRight = nextWord->word()->boundingBox().x1;
                }
                float spaceMid = (spaceLeft + spaceRight) / 2.0;
                // If more than half-way into the space, adjust to include
                if (point.x() >= spaceMid)
                {
                    cursor->nextCharacter(Spine::WithinLine);
                }
            }
        }

        return cursor;
    }

    Spine::CursorHandle PageView::textCursorAt(const QPointF & point, Spine::DocumentElement element_) const
    {
        Spine::CursorHandle cursor;

        for (int i = 0; i < 2; ++i)
        {
            qreal grace = i * 6.0;
            // Drill down from page cursor
            cursor = newCursor();
            Spine::CursorHandle prevCursor = cursor;

            while (const Spine::Image * image = cursor->image())
            {
                if (!document()->imageBased() && contains(image->boundingBox(), point))
                {
                    break;
                }
                cursor->nextImage();
            }

            if (cursor->image() == 0)
            {
                while (const Spine::Region * region = cursor->region())
                {
                    if (contains(region->boundingBox(), point, grace))
                    {
                        if (element_ == Spine::ElementRegion)
                            return cursor;

                        while (const Spine::Block * block = cursor->block())
                        {
                            if (contains(block->boundingBox(), point, grace))
                            {
                                if (element_ == Spine::ElementBlock)
                                    return cursor;

                                while (const Spine::Line * line = cursor->line())
                                {
                                    const Spine::Line * nextLine = cursor->peekNextLine();
                                    Spine::BoundingBox bbLine(line->boundingBox());
                                    bbLine.x1 -= grace;
                                    bbLine.x2 += grace;
                                    if (!cursor->hasPreviousLine()) { bbLine.y1 -= grace; }
                                    if (nextLine) { bbLine.y2 = nextLine->boundingBox().y1; }
                                    else { bbLine.y2 += grace; }
                                    if (contains(bbLine, point))
                                    {
                                        if (element_ == Spine::ElementLine)
                                            return cursor;

                                        while (const Spine::Word * word = cursor->word())
                                        {
                                            Spine::BoundingBox bbWord(word->boundingBox());
                                            bbWord.y1 = bbLine.y1;
                                            bbWord.y2 = bbLine.y2;
                                            if (!cursor->hasPreviousWord()) { bbWord.x1 = bbLine.x1; }
                                            if (!cursor->hasNextWord()) { bbWord.x2 = bbLine.x2; }
                                            if (contains(bbWord, point))
                                            {
                                                if (element_ == Spine::ElementWord)
                                                    return resolveCursor(cursor, point, grace);

                                                while (const Spine::Character * character = cursor->character())
                                                {
                                                    Spine::BoundingBox bb(character->boundingBox());
                                                    bb.y1 = bbLine.y1;
                                                    bb.y2 = bbLine.y2;
                                                    if (!cursor->hasPreviousCharacter()) { bb.x1 = bbWord.x1; }
                                                    if (!cursor->hasNextCharacter()) { bb.x2 = bbWord.x2; }
                                                    if (contains(bb, point))
                                                    {
                                                        return resolveCursor(cursor, point, grace);
                                                    }
                                                    cursor->nextCharacter();
                                                }
                                            }
                                            else if (!containsOrRightOf(bbLine, bbWord, point))
                                            {
                                                if (element_ == Spine::ElementWord)
                                                    return resolveCursor(prevCursor, point, grace);

                                                cursor = prevCursor;
                                                while (cursor->character())
                                                {
                                                    cursor->nextCharacter();
                                                }
                                                return resolveCursor(cursor, point, grace);
                                            }
                                            prevCursor = cursor->clone();
                                            cursor->nextWord();
                                        }
                                    }
                                    else if (!containsOrBelow(block->boundingBox(), bbLine, point, grace))
                                    {
                                        // start on next line
                                        return resolveCursor(cursor, point, grace);

                                        // FIXME - what about selecting the inter-line space?
                                    }
                                    prevCursor = cursor->clone();
                                    cursor->nextLine();
                                }
                            }
                            cursor->nextBlock();
                        }
                    }
                    cursor->nextRegion();
                }
            }
        }

        return resolveCursor(cursor, point);
    }

    QPoint PageView::transformFromPage(const QPointF & point) const
    {
        if (isNull())
        {
            return QPoint();
        }
        else
        {
            QPointF transformed(d->applyUserTransform(point));
            QSizeF pSize(pageSize(true));
            return QPointF(width() * transformed.x() / pSize.width(),
                           height() * transformed.y() / pSize.height()).toPoint();
        }
    }

    QRect PageView::transformFromPage(const QRectF & rect) const
    {
        return QRect(transformFromPage(rect.topLeft()), transformFromPage(rect.bottomRight()));
    }

    QSize PageView::transformFromPage(const QSizeF & size) const
    {
        return transformFromPage(QRectF(QPointF(0, 0), size)).size();
    }

    QPointF PageView::transformToPage(const QPoint & point) const
    {
        if (isNull())
        {
            return QPointF();
        }
        else
        {
            QSizeF pSize(pageSize(true));
            double x = 0;
            if (width() > 0)
            {
                x = pSize.width() * (point.x()) / (double) width();
            }
            double y = 0;
            if (height() > 0)
            {
                y = pSize.height() * (point.y()) / (double) height();
            }
            return d->unapplyUserTransform(QPointF(x, y));
        }
    }

    QRectF PageView::transformToPage(const QRect & rect) const
    {
        return QRectF(transformToPage(rect.topLeft()), transformToPage(rect.bottomRight()));
    }

    QSizeF PageView::transformToPage(const QSize & size) const
    {
        return transformToPage(QRect(QPoint(0, 0), size)).size();
    }

    void PageView::updateAnnotations(const std::string & name, const Spine::AnnotationSet & annotations_, bool added)
    {
        //qDebug() << "updateAnnotations" << qStringFromUnicode(name) << annotations_.size() << added;
        if (name.empty())
        {
            // Cache margin stripes and U:D logos where appropriate
            std::set< Spine::AnnotationHandle > annotations(newCursor()->document()->annotations());
            foreach (Spine::AnnotationHandle annotation, annotations)
            {
                // Does this annotation require embedding?
                //bool embedded = annotation->getFirstProperty("property:embedded") == "1";
                EmbeddedFrame * frame = d->embeddedWidgets.value(annotation, 0);
                bool first = true;
                foreach (const Spine::Area & pageBox, annotation->areas())
                {
                    if (pageBox.page == pageNumber())
                    {
                        Spine::BoundingBox bb = pageBox.boundingBox;
                        QRectF bounds(bb.x1, bb.y1, bb.x2-bb.x1, bb.y2-bb.y1);
                        d->embeddedRects[bounds] = annotation;

                        if (first && frame == 0)
                        {
                            frame = EmbeddedFrame::probe(annotation, bounds, this); // FIXME Only one? All?
                            if (frame)
                            {
                                frame->show();
                                frame->setGeometry(transformFromPage(frame->bounds()).adjusted(0, 0, 0, frame->controlSize().height()));
                                d->embeddedWidgets[annotation] = frame;
                            }
                            first = false;
                        }
                    }
                }
            }
            update();
        }
    }

    void PageView::updateAreaSelection(const std::string & name, const Spine::AreaSet & areas, bool added)
    {
        //qDebug() << "updateAreaSelection" << qStringFromUnicode(name);
        if (name == "SelectionManager:selected")
        {
            d->temporaryFocusHidden = false;
            BOOST_FOREACH(const Spine::Area & area, areas)
            {
                if (area.page == pageNumber())
                {
                    if (added)
                    {
                        d->temporaryFocusAreas.insert(area);
                    }
                    else
                    {
                        d->temporaryFocusAreas.erase(area);
                    }
                }
            }
            recomputeTemporaryFocus();
            update();
        }
    }

    void PageView::updateTextSelection(const std::string & name, const Spine::TextExtentSet & extents, bool added)
    {
        //qDebug() << "updateTextSelection" << qStringFromUnicode(name);
        if (name == "SelectionManager:selected")
        {
            d->temporaryFocusHidden = false;
            BOOST_FOREACH(Spine::TextExtentHandle extent, extents)
            {
                if (extent->first.cursor()->page()->pageNumber() <= pageNumber() &&
                    extent->second.cursor()->page()->pageNumber() >= pageNumber())
                {
                    if (added)
                    {
                        d->temporaryFocusExtents.insert(extent);
                    }
                    else
                    {
                        d->temporaryFocusExtents.erase(extent);
                    }
                }
            }
            recomputeTemporaryFocus();
            update();
        }
    }

    QTransform PageView::userTransform() const
    {
        return d->userTransform;
    }

    double PageView::verticalZoom() const
    {
        if (isNull())
        {
            return 1;
        }
        else
        {
            return height() / (double) pageSize(true).height();
        }
    }

} // namespace Papyro
