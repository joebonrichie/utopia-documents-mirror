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

#ifndef PAPYRO_OVERLAYRENDERER_H
#define PAPYRO_OVERLAYRENDERER_H

#if !defined(Q_MOC_RUN) || QT_VERSION >= 0x050000
#  include <spine/Annotation.h>
#  include <spine/Document.h>
#endif

#include <utopia2/extension.h>

#include <QCursor>
#include <QImage>
#include <QMap>
#include <QPainter>
#include <QPainterPath>
#include <QPicture>
#include <QSvgRenderer>

namespace Papyro
{

    class OverlayRenderer
    {
    public:
        typedef OverlayRenderer API;

        enum State { Idle, Hover, Active, Selected };

        OverlayRenderer();
        virtual ~OverlayRenderer();

        virtual QMap< int, QPainterPath > bounds(Spine::DocumentHandle document, Spine::AnnotationHandle annotation);
        virtual void configurePainter(QPainter * painter, State state);
        virtual QCursor cursor();
        virtual QString id() = 0;
        virtual QMap< int, QPicture > render(Spine::DocumentHandle document, Spine::AnnotationHandle annotation, State state);
        virtual QMap< int, QPicture > render(Spine::DocumentHandle document, const Spine::AnnotationSet & annotations, State state);
        virtual int weight();

        QBrush brush();
        QPainter::CompositionMode compositionMode();
        QPen pen();
        void setBrush(const QBrush & brush);
        void setCompositionMode(QPainter::CompositionMode compositionMode);
        void setPen(const QPen & pen);

        static QMap< int, QPainterPath > getPaths(const Spine::AnnotationSet & annotations);
        static QMap< int, QPainterPath > getPathsForAreas(const Spine::AnnotationSet & annotations);
        static QMap< int, QPainterPath > getPathsForText(const Spine::AnnotationSet & annotations);
        static QMap< int, QPainterPath > getRoundedPaths(const Spine::AnnotationSet & annotations);
        static QMap< int, QPainterPath > getRoundedPathsForAreas(const Spine::AnnotationSet & annotations);
        static QMap< int, QPainterPath > getRoundedPathsForText(const Spine::AnnotationSet & annotations);

        static QMap< int, QPainterPath > getPaths(Spine::AnnotationHandle a) { Spine::AnnotationSet s; s.insert(a); return getPaths(s); }
        static QMap< int, QPainterPath > getPathsForAreas(Spine::AnnotationHandle a) { Spine::AnnotationSet s; s.insert(a); return getPathsForAreas(s); }
        static QMap< int, QPainterPath > getPathsForText(Spine::AnnotationHandle a) { Spine::AnnotationSet s; s.insert(a); return getPathsForText(s); }
        static QMap< int, QPainterPath > getRoundedPaths(Spine::AnnotationHandle a) { Spine::AnnotationSet s; s.insert(a); return getRoundedPaths(s); }
        static QMap< int, QPainterPath > getRoundedPathsForAreas(Spine::AnnotationHandle a) { Spine::AnnotationSet s; s.insert(a); return getRoundedPathsForAreas(s); }
        static QMap< int, QPainterPath > getRoundedPathsForText(Spine::AnnotationHandle a) { Spine::AnnotationSet s; s.insert(a); return getRoundedPathsForText(s); }

    private:
        QPen _pen;
        QBrush _brush;
        QPainter::CompositionMode _compositionMode;
    }; // class OverlayRenderer




    class NoOverlayRenderer : public OverlayRenderer
    {
    public:
        virtual QMap< int, QPainterPath > bounds(Spine::DocumentHandle document, Spine::AnnotationHandle annotation);
        virtual QString id();
        virtual QMap< int, QPicture > render(Spine::DocumentHandle document, const Spine::AnnotationSet & annotations, State state);
    }; // class NoOverlayRenderer




    class RoundyOverlayRenderer : public OverlayRenderer
    {
    public:
        virtual QMap< int, QPainterPath > bounds(Spine::DocumentHandle document, Spine::AnnotationHandle annotation);
        virtual QMap< int, QPicture > render(Spine::DocumentHandle document, const Spine::AnnotationSet & annotations, State state);
    }; // class RoundyOverlayRenderer




    class DefaultOverlayRenderer : public RoundyOverlayRenderer
    {
    public:
        virtual QCursor cursor();
        virtual QString id();
        virtual QMap< int, QPicture > render(Spine::DocumentHandle document, const Spine::AnnotationSet & annotations, State state);
    }; // class DefaultOverlayRenderer




    class MarginaliaOverlayRenderer : public OverlayRenderer
    {
    public:
        MarginaliaOverlayRenderer();

        virtual QMap< int, QPainterPath > bounds(Spine::DocumentHandle document, Spine::AnnotationHandle annotation);
        virtual void drawImage(QPainter * painter, const QRectF rect) = 0;
        virtual QMap< int, QPicture > render(Spine::DocumentHandle document, const Spine::AnnotationSet & annotations, State state);
    }; // class MarginaliaOverlayRenderer




    class SvgMarginaliaOverlayRenderer : public MarginaliaOverlayRenderer
    {
    public:
        SvgMarginaliaOverlayRenderer(const QString & svgFilename);

        virtual void drawImage(QPainter * painter, const QRectF rect);

    private:
        QSvgRenderer iconRenderer;
    }; // class MarginaliaOverlayRenderer




    class ImageMarginaliaOverlayRenderer : public MarginaliaOverlayRenderer
    {
    public:
        ImageMarginaliaOverlayRenderer(const QString & imgFilename);

        virtual void drawImage(QPainter * painter, const QRectF rect);

    private:
        QImage img;
    }; // class MarginaliaOverlayRenderer

} // namespace Papyro

UTOPIA_DECLARE_EXTENSION_CLASS(LIBPAPYRO, Papyro::OverlayRenderer)

#endif // PAPYRO_OVERLAYRENDERER_H
