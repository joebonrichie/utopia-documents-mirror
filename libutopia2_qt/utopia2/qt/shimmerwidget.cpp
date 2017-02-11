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

#include "utopia2/qt/shimmerwidget.h"

#include <QPainter>
#include <QTimer>
#include <QDebug>
#include <QApplication>

#include <math.h>

namespace Utopia
{

    ShimmerWidget::ShimmerWidget(QWidget * parent) : QWidget(parent), _rippleOn(false)
    {
        this->_svgRenderer.load(QString(":/images/utopia-spiral-black.svg"));
        QTimer::singleShot(1000, this, SLOT(startShimmer()));
        //              QTimer::singleShot(2000, this, SLOT(startPulse()));

    }

    void ShimmerWidget::paintEvent(QPaintEvent * event)
    {

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setOpacity(0.4);
        painter.setPen(Qt::NoPen);
        QSize size = this->_svgRenderer.defaultSize();
        size.scale(150, 150, Qt::KeepAspectRatio);
        QRect sRect(QPoint(0, 0), size);
        sRect.moveCenter(this->rect().center());
        QPixmap overlay(sRect.size());
        overlay.fill(QColor(0, 0, 0, 0));
        QPainter overlayPainter(&overlay);
        this->_svgRenderer.render(&overlayPainter, QRect(QPoint(0, 0), sRect.size()));
        qreal offset = this->_shimmer.isValid() ? (this->_shimmer.elapsed() / 1000.0) : -1.0;

        if (this->_rippleOn)
        {
            float opacity = (sin(this->_ripple.elapsed()/800.0) + 1) / 2.0;

            overlayPainter.setBrush(QColor(255,255,255,opacity * 128));
            overlayPainter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
            overlayPainter.drawRect(overlay.rect());
            QTimer::singleShot(80, this, SLOT(update()));
        }
        else
        {
            if (offset > 5.0)
            {
                this->_shimmer.restart();
                QTimer::singleShot(40, this, SLOT(update()));
            }
            else if (offset > 1.0)
            {
                QTimer::singleShot(90000, this, SLOT(update()));
            }
            else if (offset >= 0.0)
            {
                QLinearGradient grad(QPointF(-0.5 * sRect.width(), 500), QPointF(sRect.width() * 1.5, 0));
                grad.setColorAt(qBound(0.0, offset, 1.0), QColor(0, 0, 0));
                grad.setColorAt(qBound(0.0, offset + 0.05, 1.0), QColor(200, 230, 255));
                grad.setColorAt(qBound(0.0, offset + 0.1, 1.0), QColor(0, 0, 0));
                overlayPainter.setBrush(grad);
                overlayPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
                overlayPainter.drawRect(QRect(QPoint(0, 0), sRect.size()));

                QTimer::singleShot(40, this, SLOT(update()));
            }

            if (this->_pulse.isValid() && this->_pulse.elapsed() < 300)
            {
                //              float opacity = cos((this->_pulse.elapsed()/500.0) * (3.141 / 2.0));
                float opacity = 1.0 - (this->_pulse.elapsed()/300.0);

                //              qDebug() << "opacity = " << opacity;
                overlayPainter.setBrush(QColor(255,0,0,opacity * 255));
                overlayPainter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
                overlayPainter.drawRect(overlay.rect());
                QTimer::singleShot(40, this, SLOT(update()));
            }
        }
        painter.drawPixmap(sRect.topLeft(), overlay);
    }

    void ShimmerWidget::startPulse()
    {
        QApplication::beep();
        this->_pulse.start();
        this->update();
    }

    void ShimmerWidget::startRipple()
    {
        this->_rippleOn = true;
        this->_ripple.start();
        this->update();
    }

    void ShimmerWidget::stopRipple()
    {
        this->_rippleOn = false;
        this->update();
    }

    void ShimmerWidget::startShimmer()
    {
        this->_shimmer.start();
        this->update();
    }

} // namespace Utopia
