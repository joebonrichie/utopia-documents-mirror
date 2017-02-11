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

#ifndef Utopia_QT_THUMBNAILCHOOSER_H
#define Utopia_QT_THUMBNAILCHOOSER_H

#include <QPixmap>
#include <QRectF>
#include <boost/scoped_ptr.hpp>
#include <QStringList>
#include <QWidget>

class QLabel;

namespace Utopia
{

    class ThumbnailChooserPrivate;
    class ThumbnailChooser : public QWidget
    {
        Q_OBJECT
        Q_PROPERTY(QColor controlColor
                   READ controlColor
                   WRITE setControlColor)

    public:
        ThumbnailChooser(QWidget * parent = 0, Qt::WindowFlags f = 0);
        ThumbnailChooser(const QPixmap & source, const QSize & size, QWidget * parent = 0, Qt::WindowFlags f = 0);
        ~ThumbnailChooser();

        QColor backgroundColor() const;
        bool clampZoom() const;
        QColor controlColor() const;
        QRectF cropRect() const;
        int handleSize() const;
        bool isValidThumbnail() const;
        QSize minimumSizeHint() const;
        void setBackgroundColor(const QColor & color);
        void setClampZoom(bool clamp);
        void setControlColor(const QColor & color);
        void setCropRect(const QRectF & rect);
        void setHandleSize(int size);
        void setSmoothUpdates(bool smooth);
        void setSourceImage(const QPixmap & source);
        void setThumbnailSize(const QSize & size);
        void setZoom(qreal zoom);
        bool smoothUpdates() const;
        QPixmap sourceImage() const;
        QPixmap thumbnail(bool best = true) const;
        QLabel * thumbnailPreview();
        QSize thumbnailSize() const;
        qreal zoom() const;

    signals:
        void thumbnailSizeChanged(const QSize & size);
        void thumbnailChanged();

    protected:
        bool event(QEvent * event);
        void init();
        void mouseDoubleClickEvent(QMouseEvent * event);
        void mouseMoveEvent(QMouseEvent * event);
        void mousePressEvent(QMouseEvent * event);
        void mouseReleaseEvent(QMouseEvent * event);
        void paintEvent(QPaintEvent * event);
        void resizeEvent(QResizeEvent * event);

        boost::scoped_ptr< ThumbnailChooserPrivate > d;

    };

}

#endif // Utopia_QT_THUMBNAILCHOOSER_H
