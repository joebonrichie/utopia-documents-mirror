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

#include <utopia2/qt/spinner.h>

#include <QPainter>
#include <QPaintEvent>
#include <QPalette>

namespace Utopia
{

    /** Constructor for spinner. */
    Spinner::Spinner(QWidget* parent_, Qt::WindowFlags f_)
        : QFrame(parent_, f_), _active(false), _autoHide(false), _paused(false), _progress(-1.0)
    {
        // Set up timer for updating spinner
        _timer.setInterval(30);
        QObject::connect(&_timer, SIGNAL(timeout()), this, SLOT(update()));
        _started.start();
    }

    /** Query active state. */
    bool Spinner::active() const
    {
        return _active;
    }

    /** Query auto hide. */
    bool Spinner::autoHide() const
    {
        return _autoHide;
    }

    /** Query color state. */
    const QColor & Spinner::color() const
    {
        return _color;
    }

    /** Paint spinner. */
    void Spinner::paintEvent(QPaintEvent * event)
    {
        // Only paint when active
        if (_active)
        {
            QPainter p(this);
            p.setRenderHint(QPainter::Antialiasing, true);
            p.setRenderHint(QPainter::TextAntialiasing, true);
            if (_color.isValid())
            {
                p.setPen(_color);
                p.setBrush(_color);
            }
            else
            {
                p.setPen(palette().dark().color());
                p.setBrush(palette().dark());
            }

            // Largest square, centered
            int sqr = qMin(contentsRect().width(), contentsRect().height());
            qreal penWidth = (qreal) qRound(sqr * 3.0 / 16.0);
            QRectF bounds(QRectF((width() - sqr) / 2.0, (height() - sqr) / 2.0, sqr, sqr).adjusted(1, 1, -1, -1));

            // If paused
            if (_paused)
            {
            }
            else
            {
                // If no progress
                if (_progress < 0.0 || _progress > 1.0)
                {
                    int startAngle = _started.elapsed() * 7;
                    int spanAngle = 120*16*2;
                    QPen pen(p.pen());
                    pen.setWidth(penWidth);
                    p.setPen(pen);
                    p.drawArc(bounds.adjusted(penWidth/2.0, penWidth/2.0, -penWidth/2.0, -penWidth/2.0), -startAngle, spanAngle);
                }
                else
                {
                    // Can fit text in?
                    if (sqr >= 32)
                    {
                        QString text = QString("%1%").arg(qRound(100 * _progress));
                        QFont f(p.font());
                        f.setPixelSize(sqr / 3.5);
                        p.setFont(f);
                        p.drawText(bounds, Qt::AlignCenter, text);
                        QPainterPath clip;
                        clip.addRect(rect());
                        clip.addEllipse(bounds.adjusted(sqr / 10.0, sqr / 10.0, - sqr / 10.0, - sqr / 10.0));
                        p.setClipPath(clip);
                    }
                    else if (false) // Remove the punched out hole
                    {
                        QPainterPath clip;
                        clip.addRect(rect());
                        clip.addEllipse(bounds.adjusted(sqr / 5.0, sqr / 5.0, - sqr / 5.0, - sqr / 5.0));
                        p.setClipPath(clip);
                    }
                    p.drawPie(bounds, 16*90.0, -16*360.0*_progress);
                    p.setBrush(Qt::NoBrush);
                    p.drawEllipse(bounds);
                }
            }
        }

        QFrame::paintEvent(event);
    }

    /** Pause spinner. */
    void Spinner::pause()
    {
        if (!_paused) {
            _paused = true;
            _timer.stop();
            update();
        }
    }

    /** Query paused state. */
    bool Spinner::paused() const
    {
        return _paused;
    }

    /** Query progress. */
    qreal Spinner::progress() const
    {
        return _progress;
    }

    /** Set auto hide */
    void Spinner::setAutoHide(bool autoHide)
    {
        _autoHide = autoHide;
        if (autoHide) {
            setVisible(active());
        }
    }

    /** Set color */
    void Spinner::setColor(const QColor & color_)
    {
        _color = color_;
    }

    /** Progress spinner. */
    void Spinner::setProgress(qint64 progress_, qint64 expected_)
    {
        if (expected_ > 0) {
            if (progress_ == 0) { setProgress(0.0); }
            else if (progress_ == expected_) { setProgress(1.0); }
            else { setProgress(progress_ / (qreal) expected_); }
        }
    }

    /** Progress spinner. */
    void Spinner::setProgress(qreal progress_)
    {
        if (_progress != progress_)
        {
            if (active())
            {
                // Timer needs starting
                if (_progress >= 0.0 && _progress <= 1.0 &&
                    (progress_ < 0.0 || progress_ > 1.0))
                {
                    _timer.start();
                }

                // Timer needs stopping
                if (progress_ >= 0.0 && progress_ <= 1.0 &&
                    (_progress < 0.0 || _progress > 1.0))
                {
                    _timer.stop();
                }
            }

            _progress = progress_;

            update();

            emit progressChanged(_progress);
        }
    }

    /** Start spinner off. */
    void Spinner::start()
    {
        if (!_active) {
            _active = true;
            if (!_paused && (_progress < 0.0 || _progress > 1.0))
            {
                _timer.start();
            }
            if (_autoHide) { this->show(); }
            update();

            emit runningChanged(true);
        }
    }

    /** Stop spinner. */
    void Spinner::stop()
    {
        if (_active) {
            _active = false;
            _timer.stop();
            setProgress(-1.0);
            if (_autoHide) { this->hide(); }
            update();

            emit runningChanged(false);
        }
    }

    /** Unpause spinner. */
    void Spinner::unpause()
    {
        if (_paused) {
            _paused = false;
            if (_progress < 0.0 || _progress > 1.0)
            {
                _timer.start();
            }
            update();
        }
    }

} // namespace Utopia
