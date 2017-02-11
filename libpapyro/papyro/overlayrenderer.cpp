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

#include <papyro/overlayrenderer.h>

#include <QtCore/qmath.h>
#include <QPainter>
#include <QVector2D>

#include <QDebug>

namespace
{

//     static inline bool contains(const Spine::BoundingBox & bb, const QPointF & point, qreal grace = 0.0)
//     {
//         return Spine::BoundingBox(bb.x1 - grace, bb.y1 - grace, bb.x2 + grace, bb.y2 + grace).contains(point.x(), point.y());
//     }

    static inline bool overlapHorizontally(const QRectF & rect1, const QRectF & rect2, float grace = 0.0)
    {
        return (rect1.left() <= (rect2.right() + grace) && rect1.right() >= (rect2.left() - grace));
    }

    static inline bool overlapVertically(const QRectF & rect1, const QRectF & rect2, float grace = 0.0)
    {
        return (rect1.top() <= (rect2.bottom() + grace) && rect1.bottom() >= (rect2.top() - grace));
    }

    static QPointF qRound(const QPointF & p, int places = 0)
    {
        const int factor(qPow(10, places));
        return QPointF(::qRound(p.x() * factor) / (qreal) factor,
                       ::qRound(p.y() * factor) / (qreal) factor);
    }

    static QPolygonF qRound(const QPolygonF & p, int places = 0)
    {
        QPolygonF rounded;
        QPointF previous;
        foreach (const QPointF & point, p) {
            QPointF roundedPoint(qRound(point, places));
            if (previous.isNull() || roundedPoint != previous) {
                rounded << roundedPoint;
            }
            previous = roundedPoint;
        }
        return rounded;
    }

//     static Spine::CursorHandle resolveCursor(Spine::CursorHandle cursor, const QPointF & point, qreal grace = 0.0)
//     {
//         if (cursor && cursor->word()) {
//             if (cursor->character()) { // Mouse over character
//                 // If more than half-way into the character, adjust to include
//                 if (contains(cursor->line()->boundingBox(), point, grace) &&
//                     point.x() >= (cursor->character()->boundingBox().x1 + cursor->character()->boundingBox().x2) / 2.0) {
//                     cursor->nextCharacter();
//                 }
//             } else { // Mouse between characters
//                 float spaceLeft = cursor->word()->boundingBox().x2;
//                 float spaceRight = spaceLeft;
//                 Spine::CursorHandle nextWord(cursor->clone()); nextWord->nextWord();
//                 if (nextWord->word()) {
//                     spaceRight = nextWord->word()->boundingBox().x1;
//                 }
//                 float spaceMid = (spaceLeft + spaceRight) / 2.0;
//                 // If more than half-way into the space, adjust to include
//                 if (point.x() >= spaceMid) {
//                     cursor->nextCharacter(Spine::WithinLine);
//                 }
//             }
//         }
//
//         return cursor;
//     }

    static QPainterPath roundyCorners(const QPolygonF & polygon, const qreal radius = 1.0)
    {
        QPainterPath outline;

        bool newPath = true;
        // Round the polygon
        QPolygonF poly(qRound(polygon, 2));
        // Quick pass to remove co-liner points
        for (int previousIndex = poly.size(); previousIndex > 0; --previousIndex) {
            // Get the three points of interest, and useful vectors
            const int poi = poly.size() - 1; // # of points of interest
            const int currentIndex = (previousIndex + 1) % poi;
            const int nextIndex = (previousIndex + 2) % poi;
            QPointF previous(poly.at(previousIndex % poi));
            QPointF current(poly.at(currentIndex));
            QPointF next(poly.at(nextIndex));
            QVector2D backwards(previous - current);
            QVector2D forwards(next - current);
            backwards.normalize();
            forwards.normalize();
            qreal angle(qAcos(QVector2D::dotProduct(forwards, backwards)));
            if (qAbs(angle - M_PI) < 0.001) {
                poly.remove(currentIndex);
                ++previousIndex;
            }
        }

        // Calculate concavity
        QVector< bool > convexity(poly.size());
        for (int previousIndex = poly.size(); previousIndex > 0; --previousIndex) {
            // Get the three points of interest, and useful vectors
            const int poi = poly.size() - 1; // # of points of interest
            const int currentIndex = (previousIndex + 1) % poi;
            const int nextIndex = (previousIndex + 2) % poi;
            QPointF previous(poly.at(previousIndex % poi));
            QPointF current(poly.at(currentIndex));
            QPointF next(poly.at(nextIndex));
            QVector2D backwards(previous - current);
            QVector2D forwards(next - current);

            // Concave or convex?
            // Uses the z coordinate of the cross product to decide concavity
            convexity[currentIndex] = (forwards.x() * backwards.y() - forwards.y() * backwards.x()) > 0;
        }

        for (int previousIndex = poly.size(); previousIndex > 0; --previousIndex) {
            // Get the three points of interest, and useful vectors
            const int poi = poly.size() - 1; // # of points of interest
            const int currentIndex = (previousIndex + 1) % poi;
            const int nextIndex = (previousIndex + 2) % poi;
            QPointF previous(poly.at(previousIndex % poi));
            QPointF current(poly.at(currentIndex));
            QPointF next(poly.at(nextIndex));
            QVector2D backwards(previous - current);
            QVector2D forwards(next - current);
            qreal backwardsLength(backwards.length());
            qreal forwardsLength(forwards.length());
            backwards.normalize();
            forwards.normalize();

            // Firstly work out the angle of this corner using the dot product
            qreal angle(qAcos(QVector2D::dotProduct(forwards, backwards)));

            // Things to calculate
            QPointF centre; // Centre of the corner arc
            qreal arc_start_angle = 0.0;
            qreal arc_sweep_length = M_PI - angle;
            qreal arc_radius = radius;

            // Concave or convex?
            // Uses the z coordinate of the cross product to decide concavity
            bool convex = (forwards.x() * backwards.y() - forwards.y() * backwards.x()) > 0;

            if (true) {
                // Next calculate the radius of this corner
                // The rounded corner cannot meet the edges a distance of more than half the
                // available space along a side from the corner
                //qreal d1 = backwardsLength / 2.0;
                //qreal d2 = forwardsLength / 2.0;
                qreal tan_half_angle = qTan(angle / 2.0);

                // Centre of arc
                QVector2D a((backwards + forwards).normalized());
                if (convex) {
                    qreal max_l = 2 * radius / tan_half_angle;
                    bool previousConcave(!convexity.at(previousIndex % poi));
                    bool nextConcave(!convexity.at(nextIndex));
                    qreal l = qMin(max_l, qMin(previousConcave ? backwardsLength : backwardsLength / 2.0,
                                               nextConcave ? forwardsLength : forwardsLength / 2.0));
                    qreal r = l * tan_half_angle;
                    qreal h = r / qSin(angle / 2.0);
                    centre = current + h * a.toPointF();
                    arc_radius = r - radius;
                } else {
                    qreal h = radius / qSin(angle / 2.0);
                    qreal r = radius; // qMin(radius, qMin(backwardsLength / 2.0, forwardsLength / 2.0));
                    qreal x = h - r * h / radius;
                    centre = current - x * a.toPointF();
                    arc_radius = r;
                }

                // Arc
                arc_start_angle = qAtan2(backwards.x(), backwards.y()) + M_PI_2;
                if (!convex) {
                    arc_sweep_length = -arc_sweep_length;
                }
            } else {
                // Arc
                arc_start_angle = qAtan2(backwards.x(), backwards.y()) + M_PI_2;
                arc_sweep_length = -arc_sweep_length;

                // Centre of arc
                centre = current;
            }

            arc_start_angle = arc_start_angle * 180.0 * M_1_PI;
            arc_sweep_length = arc_sweep_length * 180.0 * M_1_PI;
            QRectF arcRect(centre - QPointF(arc_radius, arc_radius), QSizeF(2 * arc_radius, 2 * arc_radius));

            if (newPath) {
                outline.arcMoveTo(arcRect, arc_start_angle + arc_sweep_length);
                newPath = false;
            } else {
                outline.arcTo(arcRect, arc_start_angle, arc_sweep_length);
            }
        }
        outline.closeSubpath();

        return outline;
    }

    static QPainterPath roundyCorners(const QPainterPath & path, const qreal radius = 1.0)
    {
        QPainterPath paths(path);
        paths.setFillRule(Qt::WindingFill);

        // For each polygon...
        QPainterPath outlines;
        foreach (const QPolygonF & polygon, paths.simplified().toSubpathPolygons()) {
            outlines.addPath(roundyCorners(polygon, radius));
        }
        return outlines;
    }

    static QPainterPath roundyCorners(const QVector< QRectF > & rects_, const qreal radius = 1.0, const QSizeF & padding = QSizeF(0.0, 1.0))
    {
        // Compile rectangles
        QVector< QRectF > rects(rects_);
        foreach (const QRectF & rect, rects_) {
            rects << rect.adjusted(-(radius + padding.width()), -(radius + padding.height()), radius + padding.width(), radius + padding.height());
        }

        // Modify rects to remove similar x coordinates
        if (rects.size() > 1) {
            bool fixed = true;
            while (fixed) {
                fixed = false;
                int indexFrom = 0;
                qreal conformTo = rects.at(0).right();
                for (int i = 1; i < rects.size(); ++i) {
                    if (qAbs(rects.at(i).right() - rects.at(i-1).right()) >= (2.0 * radius)) {
                        if (i - indexFrom > 1) {
                            // Conform
                            for (int j = indexFrom; j < i; ++j) {
                                if (rects.at(j).right() != conformTo) {
                                    rects[j].setRight(conformTo);
                                    fixed = true;
                                }
                            }
                        }

                        indexFrom = i;
                        conformTo = rects.at(i).right();
                    } else {
                        conformTo = qMax(conformTo, rects.at(i).right());
                    }
                }
                if (indexFrom != rects.size() - 1) {
                    // Conform
                    for (int j = indexFrom; j < rects.size(); ++j) {
                        if (rects.at(j).right() != conformTo) {
                            rects[j].setRight(conformTo);
                            fixed = true;
                        }
                    }
                }
                indexFrom = 0;
                conformTo = rects.at(0).left();
                for (int i = 1; i < rects.size(); ++i) {
                    if (i == rects.size() - 1 || qAbs(rects.at(i).left() - rects.at(i-1).left()) >= (2.0 * radius)) {
                        if (i - indexFrom > 1) {
                            // Conform
                            for (int j = indexFrom; j < i; ++j) {
                                if (rects.at(j).left() != conformTo) {
                                    rects[j].setLeft(conformTo);
                                    fixed = true;
                                }
                            }
                        }

                        indexFrom = i;
                        conformTo = rects.at(i).left();
                    } else {
                        conformTo = qMin(conformTo, rects.at(i).left());
                    }
                }
                if (indexFrom != rects.size() - 1) {
                    // Conform
                    for (int j = indexFrom; j < rects.size(); ++j) {
                        if (rects.at(j).left() != conformTo) {
                            rects[j].setLeft(conformTo);
                            fixed = true;
                        }
                    }
                }
            }
        }

        // Merge rectangles into individual non-overlapping polygons
        QPainterPath paths;
        paths.setFillRule(Qt::WindingFill);
        foreach (const QRectF & rect, rects) {
            paths.addRect(rect);
        }
        return roundyCorners(paths, radius);
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

//     static QMap< int, QPainterPath > asPaths(const Spine::TextSelection & selection)
//     {
//         QMap< int, QPainterPath > paths;
//         foreach (const Spine::TextExtentHandle & extent, selection) {
//             QMap< int, QPainterPath > newPaths(asPaths(extent));
//             QMapIterator< int, QPainterPath > iter(newPaths);
//             while (iter.hasNext()) {
//                 iter.next();
//                 paths[iter.key()].addPath(iter.value());
//             }
//         }
//
//         // Clean up paths
//         QMutableMapIterator< int, QPainterPath > iter(paths);
//         while (iter.hasNext()) {
//             iter.next();
//             iter.value().setFillRule(Qt::WindingFill);
//             iter.value() = iter.value().simplified();
//         }
//         return paths;
//     }

} // Anonymous namespace



namespace Papyro
{

    OverlayRenderer::OverlayRenderer()
        : _pen(Qt::NoPen), _brush(QColor(255, 0, 0, 80)), _compositionMode(QPainter::CompositionMode_Multiply)
    {}

    OverlayRenderer::~OverlayRenderer()
    {}

    QMap< int, QPainterPath > OverlayRenderer::bounds(Spine::DocumentHandle document, Spine::AnnotationHandle annotation)
    {
        Spine::AnnotationSet annotations;
        annotations.insert(annotation);
        return getPaths(annotations);
    }

    QBrush OverlayRenderer::brush()
    {
        return _brush;
    }

    QPainter::CompositionMode OverlayRenderer::compositionMode()
    {
        return _compositionMode;
    }

    void OverlayRenderer::configurePainter(QPainter * painter, State state)
    {
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setRenderHint(QPainter::TextAntialiasing, true);
        painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
        painter->setPen(_pen);
        painter->setBrush(_brush);
        painter->setCompositionMode(_compositionMode);
    }

    QCursor OverlayRenderer::cursor()
    {
        return Qt::PointingHandCursor;
    }

    QMap< int, QPainterPath > OverlayRenderer::getPaths(const Spine::AnnotationSet & annotations)
    {
        QMap< int, QPainterPath > paths;
        foreach (Spine::AnnotationHandle annotation, annotations) {
            foreach (Spine::TextExtentHandle extent, annotation->extents()) {
                QMapIterator< int, QPainterPath > iter(asPaths(extent));
                while (iter.hasNext()) {
                    iter.next();
                    paths[iter.key()].addPath(iter.value());
                }
            }
            foreach (const Spine::Area & area, annotation->areas()) {
                paths[area.page].addRect(QRectF(area.boundingBox.x1, area.boundingBox.y1, area.boundingBox.width(), area.boundingBox.height()));
            }
        }
        QMutableMapIterator< int, QPainterPath > iter(paths);
        while (iter.hasNext()) {
            iter.next();
            iter.value().setFillRule(Qt::WindingFill);
        }
        return paths;
    }

    QMap< int, QPainterPath > OverlayRenderer::getPathsForAreas(const Spine::AnnotationSet & annotations)
    {
        QMap< int, QPainterPath > paths;
        foreach (Spine::AnnotationHandle annotation, annotations) {
            foreach (const Spine::Area & area, annotation->areas()) {
                paths[area.page].addRect(QRectF(area.boundingBox.x1, area.boundingBox.y1, area.boundingBox.width(), area.boundingBox.height()));
            }
        }
        QMutableMapIterator< int, QPainterPath > iter(paths);
        while (iter.hasNext()) {
            iter.next();
            iter.value().setFillRule(Qt::WindingFill);
        }
        return paths;
    }

    QMap< int, QPainterPath > OverlayRenderer::getPathsForText(const Spine::AnnotationSet & annotations)
    {
        QMap< int, QPainterPath > paths;
        foreach (Spine::AnnotationHandle annotation, annotations) {
            foreach (Spine::TextExtentHandle extent, annotation->extents()) {
                QMapIterator< int, QPainterPath > iter(asPaths(extent));
                while (iter.hasNext()) {
                    iter.next();
                    paths[iter.key()].addPath(iter.value());
                }
            }
        }
        QMutableMapIterator< int, QPainterPath > iter(paths);
        while (iter.hasNext()) {
            iter.next();
            iter.value().setFillRule(Qt::WindingFill);
        }
        return paths;
    }

    QMap< int, QPainterPath > OverlayRenderer::getRoundedPaths(const Spine::AnnotationSet & annotations)
    {
        QMap< int, QPainterPath > paths;
        QMap< int, QVector< QRectF > > rects;
        foreach (Spine::AnnotationHandle annotation, annotations) {
            foreach (const Spine::Area & area, *annotation) {
                rects[area.page] << QRectF(area.boundingBox.x1, area.boundingBox.y1, area.boundingBox.width(), area.boundingBox.height());
            }
        }
        QMutableMapIterator< int, QVector< QRectF > > iter(rects);
        while (iter.hasNext()) {
            iter.next();
            paths[iter.key()] = roundyCorners(iter.value());
        }
        return paths;
    }

    QMap< int, QPainterPath > OverlayRenderer::getRoundedPathsForAreas(const Spine::AnnotationSet & annotations)
    {
        QMap< int, QPainterPath > paths;
        QMap< int, QVector< QRectF > > rects;
        foreach (Spine::AnnotationHandle annotation, annotations) {
            foreach (const Spine::Area & area, annotation->areas()) {
                rects[area.page] << QRectF(area.boundingBox.x1, area.boundingBox.y1, area.boundingBox.width(), area.boundingBox.height());
            }
        }
        QMutableMapIterator< int, QVector< QRectF > > iter(rects);
        while (iter.hasNext()) {
            iter.next();
            paths[iter.key()] = roundyCorners(iter.value());
        }
        return paths;
    }

    QMap< int, QPainterPath > OverlayRenderer::getRoundedPathsForText(const Spine::AnnotationSet & annotations)
    {
        QMap< int, QPainterPath > paths;
        QMap< int, QVector< QRectF > > rects;
        foreach (Spine::AnnotationHandle annotation, annotations) {
            foreach (Spine::TextExtentHandle extent, annotation->extents()) {
                foreach (const Spine::Area & area, extent->areas()) {
                    rects[area.page] << QRectF(area.boundingBox.x1, area.boundingBox.y1, area.boundingBox.width(), area.boundingBox.height());
                }
            }
        }
        QMutableMapIterator< int, QVector< QRectF > > iter(rects);
        while (iter.hasNext()) {
            iter.next();
            paths[iter.key()] = roundyCorners(iter.value());
        }
        return paths;
    }

    QPen OverlayRenderer::pen()
    {
        return _pen;
    }

    QMap< int, QPicture > OverlayRenderer::render(Spine::DocumentHandle document, Spine::AnnotationHandle annotation, State state)
    {
        Spine::AnnotationSet annotations;
        annotations.insert(annotation);
        return render(document, annotations, state);
    }

    QMap< int, QPicture > OverlayRenderer::render(Spine::DocumentHandle document, const Spine::AnnotationSet & annotations, State state)
    {
        QMap< int, QPicture > pictures;

        QMapIterator< int, QPainterPath > iter(getPaths(annotations));
        while (iter.hasNext()) {
            iter.next();
            QPainter p(&pictures[iter.key()]);
            configurePainter(&p, state);
            p.drawPath(iter.value());
        }

        return pictures;
    }

    void OverlayRenderer::setBrush(const QBrush & brush)
    {
        _brush = brush;
    }

    void OverlayRenderer::setCompositionMode(QPainter::CompositionMode compositionMode)
    {
        _compositionMode = compositionMode;
    }

    void OverlayRenderer::setPen(const QPen & pen)
    {
        _pen = pen;
    }

    int OverlayRenderer::weight()
    {
        return 0;
    }




    QMap< int, QPainterPath > NoOverlayRenderer::bounds(Spine::DocumentHandle document, Spine::AnnotationHandle annotation)
    {
        QMap< int, QPainterPath > paths;
        //Spine::AnnotationSet annotations;
        //annotations.insert(annotation);
        //return getRoundedPaths(annotations);
        return paths; //getPaths(annotations);
    }

    QString NoOverlayRenderer::id()
    {
        return "none";
    }

    QMap< int, QPicture > NoOverlayRenderer::render(Spine::DocumentHandle document, const Spine::AnnotationSet & annotations, State state)
    {
        QMap< int, QPicture > pictures;

        // Do nothing

        return pictures;
    }




    QMap< int, QPainterPath > RoundyOverlayRenderer::bounds(Spine::DocumentHandle document, Spine::AnnotationHandle annotation)
    {
        Spine::AnnotationSet annotations;
        annotations.insert(annotation);
        //return getRoundedPaths(annotations);
        return getPaths(annotations);
    }

    QMap< int, QPicture > RoundyOverlayRenderer::render(Spine::DocumentHandle document, const Spine::AnnotationSet & annotations, State state)
    {
        QMap< int, QPicture > pictures;

        //QMapIterator< int, QPainterPath > iter(getRoundedPaths(annotations));
        QMapIterator< int, QPainterPath > iter(getPaths(annotations));
        while (iter.hasNext()) {
            iter.next();
            QPainter p(&pictures[iter.key()]);
            configurePainter(&p, state);
            p.drawPath(iter.value());
        }

        return pictures;
    }




    QCursor DefaultOverlayRenderer::cursor()
    {
        return QCursor();
    }

    QString DefaultOverlayRenderer::id()
    {
        return "default";
    }

    QMap< int, QPicture > DefaultOverlayRenderer::render(Spine::DocumentHandle document, const Spine::AnnotationSet & annotations, State state)
    {
        if (state != Idle) {
            return RoundyOverlayRenderer::render(document, annotations, state);
        } else {
            return QMap< int, QPicture >();
        }
    }




    static QRectF marginRect(const QRectF & area, qreal pageWidth)
    {
        // On left or right?
        QPointF center(area.center());
        bool left = center.x() < (pageWidth / 2.0);

        // Bounds of logo
        double logoSize(pageWidth / 30.0);
        QRectF logoRect(0, qMin(area.top(), center.y() - (logoSize / 2.0)), logoSize, logoSize);
        double logoMargin = logoSize / 2.0;
        //double logoPadding = 10;
        if (left) { logoRect.moveLeft(logoMargin); }
        else { logoRect.moveRight(pageWidth - logoMargin); }

        const double prec = 1024.0;
        logoRect = QRectF(qRound(logoRect.left() * prec) / prec,
                          qRound(logoRect.top() * prec) / prec,
                          qRound(logoRect.width() * prec) / prec,
                          qRound(logoRect.height() * prec) / prec);

        return logoRect;
    }

    static QMap< int, QVector< QRectF > > iconRects(Spine::DocumentHandle document, const Spine::AnnotationSet & annotations)
    {
        QMap< int, QVector< QRectF > > rects;
        QMap< int, QPair< qreal, QVector< QRectF > > > areas;
        QMap< const Spine::Block *, Spine::CursorHandle > blocks;

        foreach (Spine::AnnotationHandle annotation, annotations) {
            foreach (Spine::TextExtentHandle extent, annotation->extents()) {
                // Find block for this text extent
                Spine::CursorHandle cursor = extent->first.cursor();
                if (!blocks.contains(cursor->block())) {
                    blocks[cursor->block()] = cursor;
                }
            }
            foreach (const Spine::Area & area, annotation->areas()) {
                QRectF rect(area.boundingBox.x1,
                            area.boundingBox.y1,
                            area.boundingBox.width(),
                            area.boundingBox.height());
                areas[area.page].first = document->newCursor(area.page)->page()->boundingBox().width();
                areas[area.page].second << rect;
            }
        }

        QMapIterator< const Spine::Block *, Spine::CursorHandle > b_iter(blocks);
        while (b_iter.hasNext()) {
            b_iter.next();
            const Spine::Block * block = b_iter.key();
            Spine::CursorHandle cursor = b_iter.value();

            QRectF blockBB(block->boundingBox().x1,
                           block->boundingBox().y1,
                           block->boundingBox().width(),
                           block->boundingBox().height());

            rects[cursor->page()->pageNumber()] << marginRect(blockBB, cursor->page()->boundingBox().width());
        }

        QMapIterator< int, QPair< qreal, QVector< QRectF > > > a_iter(areas);
        while (a_iter.hasNext()) {
            a_iter.next();
            foreach (const QRectF & rect, a_iter.value().second) {
                rects[a_iter.key()] << marginRect(rect, a_iter.value().first);
            }
        }

        return rects;
    }

    MarginaliaOverlayRenderer::MarginaliaOverlayRenderer()
        : OverlayRenderer()
    {
        setBrush(Qt::yellow);
    }

    QMap< int, QPainterPath > MarginaliaOverlayRenderer::bounds(Spine::DocumentHandle document, Spine::AnnotationHandle annotation)
    {
        QMap< int, QPainterPath > hover;

        Spine::AnnotationSet annotations;
        annotations.insert(annotation);
        QMapIterator< int, QVector< QRectF > > r_iter(iconRects(document, annotations));
        while (r_iter.hasNext()) {
            r_iter.next();
            foreach (const QRectF & rect, r_iter.value()) {
                hover[r_iter.key()].addRect(rect);
            }
        }

        QMutableMapIterator< int, QPainterPath > h_iter(hover);
        while (h_iter.hasNext()) {
            h_iter.next();
            h_iter.value().setFillRule(Qt::WindingFill);
        }

        return hover;
    }

    QMap< int, QPicture > MarginaliaOverlayRenderer::render(Spine::DocumentHandle document, const Spine::AnnotationSet & annotations, State state)
    {
        QMap< int, QPicture > pictures;
        QMap< int, QPicture > highlights;

        if (state == Papyro::OverlayRenderer::Hover) {
            highlights = Papyro::OverlayRenderer::render(document, annotations, state);
        }

        QMapIterator< int, QVector< QRectF > > r_iter(iconRects(document, annotations));
        while (r_iter.hasNext()) {
            r_iter.next();
            QPainter painter(&pictures[r_iter.key()]);
            painter.setRenderHint(QPainter::Antialiasing, true);
            painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
            foreach (const QRectF & rect, r_iter.value()) {
                drawImage(&painter, rect);
            }
            painter.drawPicture(0, 0, highlights[r_iter.key()]);
        }

        return pictures;
    }




    SvgMarginaliaOverlayRenderer::SvgMarginaliaOverlayRenderer(const QString & svgFilename)
        : MarginaliaOverlayRenderer()
    {
        iconRenderer.load(svgFilename);
    }

    void SvgMarginaliaOverlayRenderer::drawImage(QPainter * painter, const QRectF rect)
    {
        iconRenderer.render(painter, rect);
    }





    ImageMarginaliaOverlayRenderer::ImageMarginaliaOverlayRenderer(const QString & imgFilename)
        : MarginaliaOverlayRenderer(), img(imgFilename)
    {}

    void ImageMarginaliaOverlayRenderer::drawImage(QPainter * painter, const QRectF rect)
    {
        painter->drawImage(rect, img);
    }

} // namespace Papyro

UTOPIA_DEFINE_EXTENSION_CLASS(Papyro::OverlayRenderer)
