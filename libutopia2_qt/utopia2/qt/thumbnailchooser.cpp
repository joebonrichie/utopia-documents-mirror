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

#include <utopia2/qt/thumbnailchooser.h>
#include <utopia2/qt/thumbnailpreview.h>

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileInfo>
#include <QImageReader>
#include <QImageWriter>
#include <QMouseEvent>
#include <QPainter>
#include <QPointer>
#include <QUrl>
#include <QVector>

#include <math.h>

#include <QDebug>

namespace Utopia
{

    namespace
    {
        typedef enum
        {
            None,
            MoveCrop,
            ResizeCropTopLeft,
            ResizeCropTop,
            ResizeCropTopRight,
            ResizeCropRight,
            ResizeCropBottomRight,
            ResizeCropBottom,
            ResizeCropBottomLeft,
            ResizeCropLeft,
        } InteractionMode;

        typedef enum
        {
            OnHandleResize,
            OnWidgetResize,
            OnZoomChanged,
            OnSourceImageChanged,
            OnCropRectChanged,
            OnThumbnailSizeChanged,
            OnLargestCropSizeChanged,
            OnVisibleCropRectChanged,
            OnVisibleSourceImageRectChanged,
            OnVisibleZoomChanged,
            OnVisibleHandleChanged
        } CacheInvalidationReason;
    }


    //// ThumbnailChooserPrivate //////////////////////////////////////////////////////////////////

    class ThumbnailChooserPrivate
    {
    public:
        ThumbnailChooserPrivate(ThumbnailChooser * widget)
            : widget(widget)
            {
                controlColor = Qt::black;

                smoothUpdates = false;
                handleSize = 3;
                interactionMode = None;
                interacting = false;
                zoom = 0.0;
                clampZoom = true;

                cache.visibleZoom = -1.0;

                // Resize modes
                resizeModes << ResizeCropTopLeft;
                resizeModes << ResizeCropTop;
                resizeModes << ResizeCropTopRight;
                resizeModes << ResizeCropRight;
                resizeModes << ResizeCropBottomRight;
                resizeModes << ResizeCropBottom;
                resizeModes << ResizeCropBottomLeft;
                resizeModes << ResizeCropLeft;

                // Cursor shapes for resize interaction
                cursors[ResizeCropTopLeft] = Qt::SizeFDiagCursor;
                cursors[ResizeCropTop] = Qt::SizeVerCursor;
                cursors[ResizeCropTopRight] = Qt::SizeBDiagCursor;
                cursors[ResizeCropRight] = Qt::SizeHorCursor;
                cursors[ResizeCropBottomRight] = Qt::SizeFDiagCursor;
                cursors[ResizeCropBottom] = Qt::SizeVerCursor;
                cursors[ResizeCropBottomLeft] = Qt::SizeBDiagCursor;
                cursors[ResizeCropLeft] = Qt::SizeHorCursor;
            }

        ThumbnailChooser * widget;

        // UI features
        QColor backgroundColor;
        QColor controlColor;
        int handleSize;
        bool smoothUpdates;
        QPointer< ThumbnailPreview > thumbnailPreview;

        // Source image
        QPixmap sourceImage;
        qreal zoom;
        bool clampZoom;

        // Thumbnail crop state
        QRectF cropRect;
        QRectF previousCropRect;
        QSize thumbnailSize;
        QPointF resizeAnchor;

        // Interaction state
        InteractionMode interactionMode;
        QPoint mouseAnchor;
        bool interacting;
        QVector< InteractionMode > resizeModes;
        QMap< InteractionMode, Qt::CursorShape > cursors;

        // Cached values
        struct
        {
            QSizeF largestCropSize;
            QRect visibleCropRect;
            QRect visibleSourceImageRect;
            qreal visibleZoom;
            QMap< InteractionMode, QRect > visibleHandle;
        } cache;

        // Invalidate cache
        void invalidateCache(CacheInvalidationReason reason)
            {
                switch (reason)
                {
                case OnHandleResize:
                    invalidateCache(OnVisibleSourceImageRectChanged);
                    invalidateCache(OnVisibleHandleChanged);
                    break;
                case OnWidgetResize:
                    invalidateCache(OnVisibleSourceImageRectChanged);
                    invalidateCache(OnVisibleCropRectChanged);
                    break;
                case OnZoomChanged:
                    invalidateCache(OnVisibleSourceImageRectChanged);
                    invalidateCache(OnVisibleCropRectChanged);
                    invalidateCache(OnVisibleZoomChanged);
                    break;
                case OnSourceImageChanged:
                    invalidateCache(OnLargestCropSizeChanged);
                    invalidateCache(OnVisibleSourceImageRectChanged);
                    invalidateCache(OnVisibleZoomChanged);
                    break;
                case OnCropRectChanged:
                    invalidateCache(OnVisibleCropRectChanged);
                    invalidateCache(OnVisibleHandleChanged);
                    break;
                case OnThumbnailSizeChanged:
                    invalidateCache(OnLargestCropSizeChanged);
                    break;
                case OnLargestCropSizeChanged:
                    if (cache.largestCropSize.isValid())
                    {
                        cache.largestCropSize = QSizeF();
                    }
                    break;
                case OnVisibleSourceImageRectChanged:
                    if (cache.visibleSourceImageRect.isValid())
                    {
                        cache.visibleSourceImageRect = QRect();
                        invalidateCache(OnVisibleZoomChanged);
                        invalidateCache(OnVisibleHandleChanged);
                    }
                    break;
                case OnVisibleZoomChanged:
                    if (cache.visibleZoom >= 0.0)
                    {
                        cache.visibleZoom = -1.0;
                        invalidateCache(OnVisibleSourceImageRectChanged);
                        invalidateCache(OnVisibleCropRectChanged);
                    }
                    break;
                case OnVisibleCropRectChanged:
                    if (cache.visibleCropRect.isValid())
                    {
                        cache.visibleCropRect = QRect();
                        invalidateCache(OnVisibleHandleChanged);
                    }
                    break;
                case OnVisibleHandleChanged:
                    if (!cache.visibleHandle.isEmpty())
                    {
                        cache.visibleHandle.clear();
                    }
                    break;
                }
            }

        // How big in pixels is the source image going to be on screen, once zooming has been
        // applied?
        QRect visibleSourceImageRect()
            {
                if (!cache.visibleSourceImageRect.isValid())
                {
                    QSize sourceImageSize(sourceImage.size());
                    if (zoom > 0.0)
                    {
                        sourceImageSize *= zoom;
                    }
                    else
                    {
                        QSize bounds(widget->size());
                        if (clampZoom)
                        {
                            bounds = bounds.boundedTo(sourceImageSize);
                        }
                        sourceImageSize.scale(bounds, Qt::KeepAspectRatio);
                    }
                    QRect sourceImageRect(QPoint(0, 0), sourceImageSize);
                    sourceImageRect.moveCenter(widget->rect().center());
                    sourceImageRect.adjust(handleSize, handleSize, -handleSize, -handleSize);
                    cache.visibleSourceImageRect = sourceImageRect;
                }

                return cache.visibleSourceImageRect;
            }

        // How big in pixels is the crop rect going to be on screen, once zooming has been
        // applied? This is relative to the origin of the visible source image rect.
        QRect visibleCropRect()
            {
                if (!cache.visibleCropRect.isValid())
                {
                    qreal zoom(visibleZoom());
                    cache.visibleCropRect = QRectF(cropRect.topLeft() * zoom, cropRect.size() * zoom).toAlignedRect();
                }

                return cache.visibleCropRect;
            }

        // What is the effective zoom value for the currently visible source image?
        qreal visibleZoom()
            {
                if (cache.visibleZoom < 0.0)
                {
                    if (zoom > 0.0)
                    {
                        cache.visibleZoom = clampZoom ? qMin(zoom, 1.0) : zoom;
                    }
                    else
                    {
                        qreal computedZoom = visibleSourceImageRect().width() / (qreal) sourceImage.width();
                        cache.visibleZoom = clampZoom ? qMin(computedZoom, 1.0) : computedZoom;
                    }
                }

                return cache.visibleZoom;
            }

        // What is the largest possible crop size in image coordinates?
        QSizeF largestCropSize()
            {
                if (!cache.largestCropSize.isValid())
                {
                    QSizeF size(thumbnailSize);
                    size.scale(sourceImage.size(), Qt::KeepAspectRatio);
                    cache.largestCropSize = size;
                }

                return cache.largestCropSize;
            }

        // Clamp the crop rect to a valid size
        QRectF clampCropRect(QRectF rect)
            {
                // Clamp to size of source image
                QSizeF size(thumbnailSize);
                size.scale(rect.size(), Qt::KeepAspectRatio);
                QSizeF minimum(QSizeF(widget->minimumSizeHint()) / visibleZoom());
                // Grow if too small for widget
                if (size.width() < minimum.width() || size.height() < minimum.height())
                {
                    size.scale(minimum, Qt::KeepAspectRatioByExpanding);
                }
                // Shrink if too big for source image
                if (size.width() > largestCropSize().width())
                {
                    size.scale(sourceImage.size(), Qt::KeepAspectRatio);
                }
                rect.setSize(size);
                return rect;
            }

        // Modify incoming rect to suit the bounding box of source image,
        // translating in order to resolve a valid crop rect.
        QRectF translateCropRect(QRectF rect)
            {
                rect = clampCropRect(rect);

                // Move horizontally to fit
                if (rect.left() < 0.0)
                {
                    rect.moveLeft(0.0);
                }
                else if (rect.right() >= (qreal) sourceImage.width())
                {
                    rect.moveRight((qreal) sourceImage.width());
                }

                // Move vertically to fit
                if (rect.top() < 0.0)
                {
                    rect.moveTop(0.0);
                }
                else if (rect.bottom() >= (qreal) sourceImage.height())
                {
                    rect.moveBottom((qreal) sourceImage.height());
                }

                return rect;
            }

        // Modify incoming rect to suit the bounding box of source image,
        // scaling around centre in order to resolve a valid crop rect.
        QRectF scaleCropRect(QRectF rect, const QPointF & centre)
            {
/*
// Disregard invalid centre points. Invalid if:
//   //1. centre is outside of the source image bounds
//   2. centre is outside of the incoming crop rect
//   3. source image bounds and incoming crop rect do not intersect
if (!rect.adjusted(-0.001, -0.001, 0.001, 0.001).contains(centre) ||
!QRectF(sourceImage.rect().adjusted(-0.001, -0.001, 0.001, 0.001)).intersects(rect))
{
qDebug() << qSetRealNumberPrecision(64) << rect << centre;
qDebug() << "ERROR" << rect.contains(centre) << QRectF(sourceImage.rect()).intersects(rect);
return QRectF();
}
*/
                    // Deal with only invalid incoming rectangles
                    if (!QRectF(sourceImage.rect()).contains(rect))
                    {
                        QSizeF space;
                        QSizeF quadrant;
                        QSizeF expanded;
                        qreal scale = 0.0;

                        // Top Left Quadrant
                        space = QSizeF(centre.x(), centre.y());
                        quadrant = expanded = QSizeF(centre.x() - rect.left(), centre.y() - rect.top());
                        if (!quadrant.isEmpty())
                        {
                            expanded.scale(space, Qt::KeepAspectRatio);
                            scale = expanded.width() / quadrant.width();
                        }

                        // Top Right Quadrant
                        space = QSizeF(sourceImage.width() - centre.x(), centre.y());
                        quadrant = expanded = QSizeF(rect.right() - centre.x(), centre.y() - rect.top());
                        if (!quadrant.isEmpty())
                        {
                            expanded.scale(space, Qt::KeepAspectRatio);
                            if (scale == 0.0)
                            {
                                scale = expanded.width() / quadrant.width();
                            }
                            else
                            {
                                scale = qMin(scale, expanded.width() / quadrant.width());
                            }
                        }

                        // Bottom Left Quadrant
                        space = QSizeF(centre.x(), sourceImage.height() - centre.y());
                        quadrant = expanded = QSizeF(centre.x() - rect.left(), rect.bottom() - centre.y());
                        if (!quadrant.isEmpty())
                        {
                            expanded.scale(space, Qt::KeepAspectRatio);
                            if (scale == 0.0)
                            {
                                scale = expanded.width() / quadrant.width();
                            }
                            else
                            {
                                scale = qMin(scale, expanded.width() / quadrant.width());
                            }
                        }

                        // Bottom Right Quadrant
                        space = QSizeF(sourceImage.width() - centre.x(), sourceImage.height() - centre.y());
                        quadrant = expanded = QSizeF(rect.right() - centre.x(), rect.bottom() - centre.y());
                        if (!quadrant.isEmpty())
                        {
                            expanded.scale(space, Qt::KeepAspectRatio);
                            if (scale == 0.0)
                            {
                                scale = expanded.width() / quadrant.width();
                            }
                            else
                            {
                                scale = qMin(scale, expanded.width() / quadrant.width());
                            }
                        }

                        // Invalid if a scaling of zero is needed
                        if (scale == 0.0)
                        {
                            return QRectF();
                        }

                        // Scale accordingly
                        return QRectF(QPointF(centre.x() - scale * (centre.x() - rect.left()),
                                              centre.y() - scale * (centre.y() - rect.top())),
                                      rect.size() * scale);
                    }

                    return rect;
            }

        QRect visibleHandle(InteractionMode mode)
            {
                if (cache.visibleHandle.isEmpty())
                {
                    QRect cropRect(visibleCropRect().translated(visibleSourceImageRect().topLeft()));
                    QPoint cropRectCenter(cropRect.center());
                    QRect handleRect(- handleSize, - handleSize, 2 * handleSize, 2 * handleSize);
                    cache.visibleHandle[ResizeCropTopLeft] = handleRect.translated(cropRect.topLeft());
                    cache.visibleHandle[ResizeCropTop] = handleRect.translated(cropRectCenter.x(), cropRect.top());
                    cache.visibleHandle[ResizeCropTopRight] = handleRect.translated(cropRect.topRight());
                    cache.visibleHandle[ResizeCropRight] = handleRect.translated(cropRect.right(), cropRectCenter.y());
                    cache.visibleHandle[ResizeCropBottomRight] = handleRect.translated(cropRect.bottomRight());
                    cache.visibleHandle[ResizeCropBottom] = handleRect.translated(cropRectCenter.x(), cropRect.bottom());
                    cache.visibleHandle[ResizeCropBottomLeft] = handleRect.translated(cropRect.bottomLeft());
                    cache.visibleHandle[ResizeCropLeft] = handleRect.translated(cropRect.left(), cropRectCenter.y());
                }

                return cache.visibleHandle.value(mode, QRect());
            }

        void setInteractionMode(QPoint pos)
            {
                QVectorIterator< InteractionMode > m_iter(resizeModes);
                bool cursorSet = false;
                while (m_iter.hasNext())
                {
                    InteractionMode mode = m_iter.next();
                    QRect handleRect(visibleHandle(mode));
                    if (handleRect.contains(pos))
                    {
                        cursorSet = true;
                        interactionMode = mode;
                        widget->setCursor(cursors[mode]);
                        break;
                    }
                }
                if (!cursorSet)
                {
                    if (visibleCropRect().translated(visibleSourceImageRect().topLeft()).contains(pos))
                    {
                        widget->setCursor(Qt::OpenHandCursor);
                        interactionMode = MoveCrop;
                    }
                    else
                    {
                        widget->setCursor(Qt::ArrowCursor);
                        interactionMode = None;
                    }
                }
            }
    };


    //// ThumbnailChooser /////////////////////////////////////////////////////////////////////////

    ThumbnailChooser::ThumbnailChooser(QWidget * parent, Qt::WindowFlags f)
        : QWidget(parent, f), d(new ThumbnailChooserPrivate(this))
    {
        init();
    }

    ThumbnailChooser::ThumbnailChooser(const QPixmap & source, const QSize & size, QWidget * parent, Qt::WindowFlags f)
        : QWidget(parent, f), d(new ThumbnailChooserPrivate(this))
    {
        init();

        setThumbnailSize(size);
        setSourceImage(source);
    }

    ThumbnailChooser::~ThumbnailChooser()
    {}

    QColor ThumbnailChooser::backgroundColor() const
    {
        return d->backgroundColor;
    }

    bool ThumbnailChooser::clampZoom() const
    {
        return d->clampZoom;
    }

    QColor ThumbnailChooser::controlColor() const
    {
        return d->controlColor;
    }

    QRectF ThumbnailChooser::cropRect() const
    {
        return d->cropRect;
    }

    bool ThumbnailChooser::event(QEvent * event)
    {
        if (event->type() == QEvent::NonClientAreaMouseMove && !d->interacting)
        {
            setCursor(Qt::ArrowCursor);
        }
        return QWidget::event(event);
    }

    int ThumbnailChooser::handleSize() const
    {
        return d->handleSize;
    }

    void ThumbnailChooser::init()
    {
        setMouseTracking(true);
        setMinimumSize(100, 100);
    }

    bool ThumbnailChooser::isValidThumbnail() const
    {
        return d->sourceImage.size().isValid() && d->thumbnailSize.isValid() && d->cropRect.isValid();
    }

    QSize ThumbnailChooser::minimumSizeHint() const
    {
        return QSize(6 * d->handleSize - 1, 6 * d->handleSize - 1);
    }

    void ThumbnailChooser::mouseDoubleClickEvent(QMouseEvent * /*event*/)
    {
        if (d->sourceImage.isNull() || d->cropRect.isNull() || !d->thumbnailSize.isValid()) { return; }

        setCropRect(d->translateCropRect(QRect(cropRect().topLeft().toPoint(), thumbnailSize())));
    }

    void ThumbnailChooser::mouseMoveEvent(QMouseEvent * event)
    {
        if (d->sourceImage.isNull() || d->cropRect.isNull()) { return; }

        if (d->interacting)
        {
            if (d->interactionMode == MoveCrop)
            {
                QRectF cropRect = d->translateCropRect(d->previousCropRect.translated((event->pos() - d->mouseAnchor) / d->visibleZoom()));
                if (cropRect != d->cropRect)
                {
                    blockSignals(true);
                    setCropRect(cropRect);
                    blockSignals(false);
                    if (smoothUpdates()) { emit thumbnailChanged(); }
                }
            }
            else if (d->interactionMode != None)
            {
                QRectF scaledCropRect(d->previousCropRect.translated(- d->resizeAnchor));
                QPoint resizeAnchor((d->resizeAnchor * d->visibleZoom()).toPoint() + d->visibleSourceImageRect().topLeft());
                QPointF p1(event->pos() - resizeAnchor);
                QPointF p2(d->mouseAnchor - resizeAnchor);
                qreal scale = sqrt(p1.x() * p1.x() + p1.y() * p1.y()) / sqrt(p2.x() * p2.x() + p2.y() * p2.y());
                scaledCropRect = QRectF(scaledCropRect.topLeft() * scale, scaledCropRect.size() * scale);
                scaledCropRect.translate(d->resizeAnchor);

                QRectF cropRect = d->translateCropRect(d->scaleCropRect(scaledCropRect, d->resizeAnchor));
                if (cropRect != d->cropRect)
                {
                    blockSignals(true);
                    setCropRect(cropRect);
                    blockSignals(false);
                    if (smoothUpdates()) { emit thumbnailChanged(); }
                }
            }
        }
        else
        {
            d->setInteractionMode(event->pos());
        }
    }

    void ThumbnailChooser::mousePressEvent(QMouseEvent * event)
    {
        if (d->interactionMode == MoveCrop)
        {
            setCursor(Qt::ClosedHandCursor);
            d->mouseAnchor = event->pos();
            d->interacting = true;
            d->previousCropRect = d->cropRect;
            update();
        }
        else if (d->interactionMode != None)
        {
            d->mouseAnchor = event->pos();
            d->interacting = true;
            d->previousCropRect = d->cropRect;

            QPointF cropRectCenter(d->cropRect.center());
            switch (d->interactionMode)
            {
            case ResizeCropTopLeft:
                d->resizeAnchor = d->cropRect.bottomRight();
                break;
            case ResizeCropTop:
                d->resizeAnchor = QPointF(cropRectCenter.x(), d->cropRect.bottom());
                break;
            case ResizeCropTopRight:
                d->resizeAnchor = d->cropRect.bottomLeft();
                break;
            case ResizeCropRight:
                d->resizeAnchor = QPointF(d->cropRect.left(), cropRectCenter.y());
                break;
            case ResizeCropBottomRight:
                d->resizeAnchor = d->cropRect.topLeft();
                break;
            case ResizeCropBottom:
                d->resizeAnchor = QPointF(cropRectCenter.x(), d->cropRect.top());
                break;
            case ResizeCropBottomLeft:
                d->resizeAnchor = d->cropRect.topRight();
                break;
            case ResizeCropLeft:
                d->resizeAnchor = QPointF(d->cropRect.right(), cropRectCenter.y());
                break;
            default: break;
            }

            update();
        }
    }

    void ThumbnailChooser::mouseReleaseEvent(QMouseEvent * event)
    {
        if (d->interacting)
        {
            if (d->interactionMode == MoveCrop)
            {
                setCursor(Qt::OpenHandCursor);
            }

            d->setInteractionMode(event->pos());
            d->interacting = false;

            if (d->previousCropRect != d->cropRect)
            {
                emit thumbnailChanged();
            }
            update();
        }
    }

    void ThumbnailChooser::paintEvent(QPaintEvent * /*event*/)
    {
        // Set up canvas on which to draw the image and crop marks
        QPainter painter(this);
        if (d->backgroundColor.isValid()) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(d->backgroundColor);
            painter.drawRect(rect());
        }

        // Draw image
        if (d->sourceImage.isNull()) { return; }
        QRect sourceImageRect(d->visibleSourceImageRect());
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
        painter.drawPixmap(sourceImageRect, d->sourceImage);
        painter.translate(sourceImageRect.topLeft());

        // Draw crop marks
        if (d->cropRect.isNull()) { return; }
        QRect cropRect(d->visibleCropRect().adjusted(0, 0, -1, -1));
        // Start with dark overlay
        painter.save();
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 0, 0, 140));
        painter.setClipRegion(QRegion(rect().translated(-sourceImageRect.topLeft())) - cropRect.adjusted(0, 0, 1, 1));
        painter.drawRect(rect().translated(-sourceImageRect.topLeft()));
        painter.restore();
        // Light grey bounding box
        painter.setPen(d->controlColor);
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(cropRect);
        // Handles
        painter.setPen(Qt::NoPen);
        painter.setBrush(d->controlColor);
        QRect handleRect(- handleSize(), - handleSize(), 2 * handleSize() + 1, 2 * handleSize() + 1);
        cropRect.adjust(0, 0, 1, 1);
        QPoint cropRectCenter(cropRect.center());
        static QVector< QPoint > handlePoints;
        handlePoints.clear();
        handlePoints << cropRect.topLeft();
        handlePoints << QPoint(cropRectCenter.x(), cropRect.top());
        handlePoints << cropRect.topRight();
        handlePoints << QPoint(cropRect.left(), cropRectCenter.y());
        handlePoints << cropRect.bottomLeft();
        handlePoints << QPoint(cropRectCenter.x(), cropRect.bottom());
        handlePoints << cropRect.bottomRight();
        handlePoints << QPoint(cropRect.right(), cropRectCenter.y());
        QVectorIterator< QPoint > iter(handlePoints);
        while (iter.hasNext()) {
            QPoint handlePoint(iter.next());
            painter.save();
            painter.setTransform(QTransform::fromTranslate(handlePoint.x(), handlePoint.y()), true);
            painter.drawRect(handleRect);
            painter.restore();
        }
    }

    void ThumbnailChooser::resizeEvent(QResizeEvent * event)
    {
        d->invalidateCache(OnWidgetResize);
        QWidget::resizeEvent(event);
    }

    void ThumbnailChooser::setBackgroundColor(const QColor & color)
    {
        d->backgroundColor = color;
        update();
    }

    void ThumbnailChooser::setClampZoom(bool clamp)
    {
        d->clampZoom = clamp;
        update();
    }

    void ThumbnailChooser::setControlColor(const QColor & color)
    {
        d->controlColor = color;
        update();
    }

    void ThumbnailChooser::setCropRect(const QRectF & rect)
    {
        d->invalidateCache(OnCropRectChanged);

        if (d->sourceImage.size().isValid())
        {
            QRectF translated(d->translateCropRect(rect));
            if (d->cropRect != translated)
            {
                d->cropRect = translated;
                update();
            }
            emit thumbnailChanged();
        }
        else
        {
            d->cropRect = rect;
        }
    }

    void ThumbnailChooser::setHandleSize(int size)
    {
        size = qMax(size, 3);
        if (d->handleSize != size)
        {
            d->invalidateCache(OnHandleResize);
            d->handleSize = size;
            updateGeometry();
            update();
        }
    }

    void ThumbnailChooser::setThumbnailSize(const QSize & size)
    {
        d->invalidateCache(OnThumbnailSizeChanged);
        d->thumbnailSize = size;
        setCropRect(d->translateCropRect(d->cropRect));
        update();
        emit thumbnailSizeChanged(size);
    }

    void ThumbnailChooser::setSmoothUpdates(bool smooth)
    {
        d->smoothUpdates = smooth;
    }

    void ThumbnailChooser::setSourceImage(const QPixmap & source)
    {
        d->invalidateCache(OnSourceImageChanged);
        d->sourceImage = source;
        if (!d->cropRect.isValid())
        {
            d->cropRect = QRectF(QPointF(0, 0), d->thumbnailSize);
            d->cropRect.moveCenter(QRectF(QPointF(0, 0), source.size()).center());
        }
        setCropRect(d->translateCropRect(d->cropRect.toRect()));
        update();
    }

    void ThumbnailChooser::setZoom(qreal zoom)
    {
        d->invalidateCache(OnZoomChanged);
        d->zoom = zoom;
        update();
    }

    bool ThumbnailChooser::smoothUpdates() const
    {
        return d->smoothUpdates;
    }

    QPixmap ThumbnailChooser::sourceImage() const
    {
        return d->sourceImage;
    }

    QPixmap ThumbnailChooser::thumbnail(bool best) const
    {
        return sourceImage().copy(cropRect().toAlignedRect()).scaled(thumbnailSize(), Qt::IgnoreAspectRatio, best ? Qt::SmoothTransformation : Qt::FastTransformation);
    }

    QLabel * ThumbnailChooser::thumbnailPreview()
    {
        if (d->thumbnailPreview.isNull())
        {
            d->thumbnailPreview = new ThumbnailPreview(this);
            if (isValidThumbnail())
            {
                emit thumbnailChanged();
            }
        }
        return d->thumbnailPreview;
    }

    QSize ThumbnailChooser::thumbnailSize() const
    {
        return d->thumbnailSize;
    }

    qreal ThumbnailChooser::zoom() const
    {
        return d->zoom;
    }

}
