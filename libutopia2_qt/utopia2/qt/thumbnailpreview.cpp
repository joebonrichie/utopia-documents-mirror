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

#include <utopia2/qt/thumbnailpreview.h>
#include <utopia2/qt/thumbnailchooser.h>

#include <QMouseEvent>
#include <QPointer>
#include <QTimer>

#include <QDebug>

namespace Utopia
{

    //// ThumbnailPreviewPrivate //////////////////////////////////////////////////////////////////

    class ThumbnailPreviewPrivate
    {
    public:
        ThumbnailPreviewPrivate(ThumbnailPreview * preview, ThumbnailChooser * chooser)
            : preview(preview), chooser(chooser), interacting(false)
            {
                idleTimer.setInterval(300);
                idleTimer.setSingleShot(true);
                QObject::connect(&idleTimer, SIGNAL(timeout()), preview, SLOT(updateThumbnail()));
            }

        // Chooser
        ThumbnailPreview * preview;
        QPointer< ThumbnailChooser > chooser;

        // Interaction state
        bool interacting;
        QPoint mouseAnchor;
        QRectF cropRectAnchor;
        QTimer idleTimer;
    };


    //// ThumbnailPreview /////////////////////////////////////////////////////////////////////////

    ThumbnailPreview::ThumbnailPreview(ThumbnailChooser * chooser, QWidget * parent, Qt::WindowFlags f)
        : QLabel(parent, f), d(new ThumbnailPreviewPrivate(this, chooser))
    {
        connect(chooser, SIGNAL(thumbnailChanged()), this, SLOT(updateThumbnail()));

        setMouseTracking(true);
    }

    ThumbnailPreview::~ThumbnailPreview()
    {}

    bool ThumbnailPreview::event(QEvent * event)
    {
        if (event->type() == QEvent::NonClientAreaMouseMove && !d->interacting)
        {
            setCursor(Qt::ArrowCursor);
        }
        return QLabel::event(event);
    }

    void ThumbnailPreview::mouseMoveEvent(QMouseEvent * event)
    {
        if (!d->chooser.isNull())
        {
            if (d->interacting)
            {
                QPointF offset(event->pos() - d->mouseAnchor);
                qreal scale(d->chooser->thumbnailSize().width() / d->chooser->cropRect().width());
                QRectF newCropRect(d->cropRectAnchor.topLeft() - offset / scale, d->cropRectAnchor.size());
                d->chooser->setCropRect(newCropRect);
            }
            else if (rect().contains(event->pos()))
            {
                setCursor(Qt::OpenHandCursor);
            }
        }
    }

    void ThumbnailPreview::mousePressEvent(QMouseEvent * event)
    {
        if (!d->chooser.isNull())
        {
            d->interacting = true;
            d->mouseAnchor = event->pos();
            d->cropRectAnchor = d->chooser->cropRect();
            setCursor(Qt::ClosedHandCursor);
        }
    }

    void ThumbnailPreview::mouseReleaseEvent(QMouseEvent * /*event*/)
    {
        d->interacting = false;
        setCursor(Qt::OpenHandCursor);
    }

    void ThumbnailPreview::updateThumbnail()
    {
        if (!d->chooser.isNull())
        {
            bool timeout = qobject_cast< QTimer * >(sender()) == &d->idleTimer;
            bool active = d->idleTimer.isActive();
            setPixmap(d->chooser->thumbnail(!active));
            if (active || !timeout)
            {
                d->idleTimer.start();
            }
        }
    }

}
