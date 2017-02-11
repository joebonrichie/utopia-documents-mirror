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

#include <utopia2/qt/imagedropper.h>

#include <QPainter>
#include <QDebug>
#include <QMimeData>
#include <QMouseEvent>
#include <QImage>
#include <QUrl>

namespace Utopia
{
    ImageDropper::ImageDropper(QWidget * parent) : QLabel(parent)
    {
        this->setAcceptDrops(true);
        _dragHighlighted = false;
        _highlighted = false;
        _noImage = true;
        QPixmap pixmap(":/images/noimage.png");
        QPixmap scaledPixmap = pixmap.scaled(256, 256, Qt::KeepAspectRatio);
        setPixmap(scaledPixmap);

        // Make this connection after the default image has been set
        //connect(this, SIGNAL(imageUpdate(QWidget *, const QVariant *)), this->parent(), SLOT(commitChange(QWidget *, const QVariant *)));
    }

    void ImageDropper::paintEvent(QPaintEvent * event)
    {
//        QLabel::paintEvent(event);
        QPainter painter(this);

        QPixmap blank(this->pixmap()->size());
        blank.fill(QColor(0,0,0,0));

        QPixmap avatar(*this->pixmap());

        QPainter compPainter(&blank);
        compPainter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);
        compPainter.setBrush(Qt::white);
        compPainter.setPen(QColor(0,0,0,0));
        compPainter.drawRoundedRect(avatar.rect(), 5,5);
        compPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        compPainter.drawPixmap(QPoint(0,0), avatar);

        if (_dragHighlighted)
        {
            compPainter.setCompositionMode(QPainter::CompositionMode_SourceOver);
            compPainter.setPen(QColor(0, 0, 0, 0));
            compPainter.setBrush(QColor(107,117,255,150));
            compPainter.drawRoundedRect(this->rect(), 5, 5);
        }
        if (_highlighted)
        {
            compPainter.setCompositionMode(QPainter::CompositionMode_SourceOver);
            compPainter.setPen(QColor(255, 255, 255, 0));
            compPainter.setBrush(QColor(207,217,227,150));
            compPainter.drawRoundedRect(this->rect(), 5, 5);
            QFontMetrics fm(this->font());
            QString message("Drop to replace");
            compPainter.setPen(QColor(255, 255, 255, 0));
            QSize offset(fm.width(message), fm.height());
            compPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
            compPainter.drawText(this->rect().x() + (this->size().width() / 2) - (offset.width() / 2),
                                 this->rect().y() + (this->size().height() / 2), message);
        }
        painter.drawPixmap(QPoint(0,0), blank);
    }

    void ImageDropper::dragEnterEvent(QDragEnterEvent * event)
    {
        if (event->mimeData()->hasImage() || event->mimeData()->hasUrls())
        {
            event->acceptProposedAction();
            _dragHighlighted = true;
        }
        this->update();
    }

    void ImageDropper::dragLeaveEvent(QDragLeaveEvent *)
    {
        _dragHighlighted = false;
        this->update();
    }

    void ImageDropper::dropEvent(QDropEvent * event)
    {
        if (event->mimeData()->hasImage())
        {
            event->accept();
            QPixmap pixmap = QPixmap::fromImage(qvariant_cast< QImage >(event->mimeData()->imageData()));
            QPixmap squaredPixmap = pixmap.scaled(256, 256, Qt::KeepAspectRatio);
            this->setPixmap(squaredPixmap);
            _dragHighlighted = false;
            this->update();
        }
        else if (event->mimeData()->hasUrls())
        {
            QListIterator< QUrl > urls(event->mimeData()->urls());
            while (urls.hasNext())
            {
                QUrl url(urls.next());
                if (url.scheme() == "file")
                {
                    QPixmap pixmap(url.toLocalFile());
                    if (!pixmap.isNull())
                    {
                        event->accept();
                        QPixmap squaredPixmap = pixmap.scaled(256, 256, Qt::KeepAspectRatioByExpanding).copy(0, 0, 256, 256);
                        this->setPixmap(squaredPixmap);
                        _dragHighlighted = false;
                        this->update();
                    }
                }
            }
        }
    }

    void ImageDropper::enterEvent(QEvent * event)
    {
        _highlighted = true;
        this->update();
    }

    const QPixmap * ImageDropper::fullPixmap() const
    {
        return &_fullPixmap;
    }

    void ImageDropper::leaveEvent(QEvent * event)
    {
        _highlighted = false;
        _dragHighlighted = false;
        this->update();
    }

    void ImageDropper::setPixmap(const QPixmap& newpixmap)
    {
        _noImage = false;
        if (this->pixmap())
        {
            _undoImage = this->pixmap()->copy();
        }
        QPixmap scaledPixmap = newpixmap.scaled(128, 128, Qt::KeepAspectRatio);
        _fullPixmap = newpixmap.scaled(256, 256, Qt::KeepAspectRatio);
        QImage image = scaledPixmap.toImage().convertToFormat(QImage::Format_ARGB32);
        scaledPixmap = QPixmap::fromImage(image);

        QLabel::setPixmap(scaledPixmap);

        emit updateImage();
    }

    void ImageDropper::setPixmapManually(const QPixmap& newpixmap)
    {
        _noImage = false;
        if (this->pixmap())
        {
            _undoImage = this->pixmap()->copy();
        }
        QPixmap scaledPixmap = newpixmap.scaled(128, 128, Qt::KeepAspectRatio);
        _fullPixmap = newpixmap.scaled(256, 256, Qt::KeepAspectRatio);
        QImage image = scaledPixmap.toImage().convertToFormat(QImage::Format_ARGB32);
        scaledPixmap = QPixmap::fromImage(image);

        QLabel::setPixmap(scaledPixmap);
    }

} // namespace Utopia
