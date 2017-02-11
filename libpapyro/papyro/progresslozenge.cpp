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

#include <papyro/progresslozenge.h>

#include <QMutex>
#include <QMutexLocker>
#include <QPainter>
#include <QTime>
#include <QTimer>

#include <QDebug>

namespace Papyro
{

    class ProgressLozengePrivate
    {
    public:
        int period;
        QTime timer;
        bool started;
        qreal progress;
        QString label;
        QColor color;
    };

    ProgressLozenge::ProgressLozenge(QString label, QColor color, QWidget * parent)
        : QWidget(parent), d(new ProgressLozengePrivate)
    {
        d->period = 200;
        d->started = false;
        d->progress = -1;
        d->color = color;
        d->label = label;

        setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
        QFont f(font());
        f.setPixelSize(10);
        setFont(f);
    }

    ProgressLozenge::~ProgressLozenge()
    {
        delete d;
    }

    QColor ProgressLozenge::color() const
    {
        return d->color;
    }

    QString ProgressLozenge::label() const
    {
        return d->label;
    }

    void ProgressLozenge::paintEvent(QPaintEvent * event)
    {
        int radius = (height() - 1) / 2;
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::TextAntialiasing);
        QFontMetrics metrics(fontMetrics());
        QString text(metrics.elidedText(d->label, Qt::ElideRight , width() - 2 - 4 * radius));
        int width = metrics.width(text) + 2 + 4 * radius;

        // Lozenge
        p.setOpacity(0.6);
        p.setBrush(d->color);
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(rect().adjusted(1, 0, -1, -1), radius, radius, Qt::AbsoluteSize);

        // Label
        p.setOpacity(1.0);
        p.setPen(QColor(200, 200, 200));
        p.drawText(rect().adjusted(1 + radius, 0, -1 - 2 * radius, -1), Qt::AlignVCenter | Qt::AlignLeft, text);

        // Spinner
        if (d->started)
        {
            QRectF iconRect(width - 2 * (radius - 1), 3, 2 * (radius - 3), 2 * (radius - 3));
            if (d->progress < 0.0)
            {
                int startAngle = d->timer.elapsed() * 7;
                int spanAngle = 120*16*2;
                QPen pen(p.pen());
                pen.setWidth(3.0);
                p.setPen(pen);
                p.drawArc(iconRect, -startAngle, spanAngle);
                QTimer::singleShot(40, this, SLOT(update()));
            }
            else
            {
                qreal ratio = qBound(0.0, d->progress, 1.0);
                QPen pen(p.pen());
                pen.setWidth(1.0);
                p.setPen(pen);
                p.drawPie(iconRect, 16*90.0, -16*360.0*ratio);
                p.setBrush(Qt::NoBrush);
                p.drawEllipse(iconRect);
            }
        }
    }

    int ProgressLozenge::period() const
    {
        return d->period;
    }

    void ProgressLozenge::setColor(QColor color)
    {
        d->color = color;
        update();
    }

    void ProgressLozenge::setLabel(QString label)
    {
        d->label = label;
        updateGeometry();
    }

    void ProgressLozenge::setPeriod(int msecs)
    {
        d->period = msecs;
    }

    void ProgressLozenge::setProgress(qreal progress)
    {
        d->progress = progress;
        update();
    }

    QSize ProgressLozenge::sizeHint() const
    {
        int w = fontMetrics().width(d->label) + 34;
        int h = 17;
        return QSize(w, h);
    }

    void ProgressLozenge::start()
    {
        d->started = true;
        d->timer.start();
        update();
    }

    void ProgressLozenge::stop()
    {
        d->started = false;
        update();
    }

}
