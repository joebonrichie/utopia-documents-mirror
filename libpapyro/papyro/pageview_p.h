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

#ifndef LIBPAPYRO_PAGEVIEW_P_H
#define LIBPAPYRO_PAGEVIEW_P_H

#include <papyro/config.h>

#include <papyro/documentproxy.h>
#include <papyro/embeddedframe.h>
#include <papyro/pageview.h>
#include <papyro/phraselookup.h>

#if !defined(Q_MOC_RUN) || QT_VERSION >= 0x050000
#  include <spine/Annotation.h>
#  include <spine/BoundingBox.h>
#  include <spine/Block.h>
#  include <spine/Line.h>
#  include <spine/Word.h>
#  include <spine/Character.h>
#  include <spine/Document.h>
#  include <spine/Area.h>
#  include <spine/Region.h>
#  include <spine/TextIterator.h>
#  include <spine/TextSelection.h>
#endif

#include <utopia2/auth/qt/conversation.h>
#include <utopia2/networkaccessmanager.h>
#include <utopia2/qt/cache.h>
#include <utopia2/qt/imageformatmanager.h>

#include <QColor>
#include <QImage>
#include <QList>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QPicture>
#include <QPixmap>
#include <QPainterPath>
#include <QPoint>
#include <QPointer>
#include <QPointF>
#include <QRect>
#include <QRectF>
#include <QSize>
#include <QString>
#include <QSvgRenderer>
#include <QThread>
#include <QTime>
#include <QTimer>
#include <QTransform>

class QSignalMapper;

namespace Papyro
{

    class PageViewRenderThread : public QThread
    {
        Q_OBJECT

    public:
        PageViewRenderThread(PageView * pageView);
        ~PageViewRenderThread();

        bool isDirty();
        QImage image();
        void run();
        void setTarget(QSize size, QColor color);

    protected:
        void getTarget(QSize * size, QColor * color);

    private:
        QPointer< PageView > _pageView;
        QSize _size;
        QColor _color;
        QMutex _mutex;
        QImage _image;
        bool _dirty;

        static QMutex _globalMutex;

    }; // class PageViewRenderThread





    typedef enum
    {
        NoInteraction = 0,
        SelectingArea = 1,
        SelectingText = 2
    } InteractionMode;

    class PageViewPrivate : public QObject, public Utopia::NetworkAccessManagerMixin
    {
        Q_OBJECT

    public:
        PageViewPrivate(PageView * pageView);

        // Public widget
        PageView * pageView;

        // Page
        Spine::DocumentHandle document;
        Spine::CursorHandle cursor;
        boost::scoped_ptr< DocumentProxy > documentProxy;

        // Transformations
        QRectF mediaRect() const;
        QSizeF mediaSize() const;
        QRectF pageRect() const;
        QSizeF pageSize() const;
        int userTransformDegrees;
        QTransform userTransform;
        QTransform userTransformInverse;
        QRectF transformedPageRect;
        void setUserTransform(int degrees);
        QPointF applyUserTransform(const QPointF & rect);
        QRectF applyUserTransform(const QRectF & rect);
        QSizeF applyUserTransform(const QSizeF & size);
        QPointF unapplyUserTransform(const QPointF & rect);
        QRectF unapplyUserTransform(const QRectF & rect);
        QSizeF unapplyUserTransform(const QSizeF & size);
        QSignalMapper * rotateMapper;
        QMenu * rotateMenu;

        // Decorations
        PageView::PageDecorations decorations;

        // Image cache
        QString cacheName;
        boost::scoped_ptr< PageViewRenderThread > renderThread;
        Utopia::Cache< QPixmap > imageCache;

        // Mouse press/release variables
        QPoint mousePressPos;
        QPointF mousePressPagePos;
        QPoint mouseMovePos;
        QPointF mouseMovePagePos;
        QPoint mouseReleasePos;
        QPointF mouseReleasePagePos;
        QTime mousePressTime;
        QTimer mousePressTimer;
        bool dragging;
        bool multiClick;
        bool tripleClick;
        Spine::CursorHandle previousSelectedImageCursor;
        bool repressLookup;

        // Utilities
        boost::shared_ptr< Utopia::ImageFormatManager > imageFormatManager;
        std::auto_ptr< Utopia::Conversation > conversationWidget;

        // Selection
        InteractionMode interactionMode;

        // Search spotlights
        Spine::TextExtentSet spotlights;
        Spine::TextExtentHandle activeSpotlight;
        QPainterPath darkness;
        QPainterPath bubble;
        bool spotlightsHidden;

        Spine::TextExtentSet temporaryFocusExtents;
        Spine::AreaSet temporaryFocusAreas;
        bool temporaryFocusHidden;
        QPainterPath temporaryFocus;

        // Phrase lookup
        QSignalMapper * phraseLookupMapper;
        QList< Papyro::PhraseLookup * > phraseLookups;
        QString lookupPhrase;
        QMenu * phraseLookupMenu;

        // Browse mapper
        QSignalMapper * browseToMapper;
        QSignalMapper * deleteAnnotationMapper;

        // Embedded widgets
        QMap< Spine::AnnotationHandle, EmbeddedFrame * > embeddedWidgets;
        QMap< QRectF, Spine::AnnotationHandle > embeddedRects;
        QMap< Spine::AnnotationHandle, QWidget * > linkedWidgets;

        // Set interaction state for mouse press
        void setMousePressPos(const QPoint & pos);

        // Set interaction state for mouse move
        void setMouseMovePos(const QPoint & pos);

        // Set interaction state for mouse release
        void setMouseReleasePos(const QPoint & pos);

    public slots:
        // URL browsing (and PDF catching)
        void browseUrl(const QString & url, const QString & target = QString());
        void browseUrl(const QUrl & url, const QString & target = QString());

    signals:
        void urlRequested(const QUrl & url, const QString & target);

    };

}

#endif // LIBPAPYRO_PAGEVIEW_P_H
