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

#ifndef Utopia_QT_SPINNER_H
#define Utopia_QT_SPINNER_H

#include <utopia2/qt/config.h>

#include <QColor>
#include <QTime>
#include <QTimer>
#include <QFrame>

namespace Utopia
{

    class LIBUTOPIA_QT_API Spinner : public QFrame
    {
        Q_OBJECT

    public:
        // Constructor
        Spinner(QWidget * parent_ = 0, Qt::WindowFlags f_ = 0);

        // Query state
        bool active() const;
        bool autoHide() const;
        const QColor & color() const;
        bool paused() const;
        qreal progress() const;

    signals:
        void progressChanged(qreal);
        void runningChanged(bool);

    public slots:
        // Pause spinner
        void pause();
        // Set auto hide
        void setAutoHide(bool autoHide);
        // Set color
        void setColor(const QColor & color_);
        // Progress spinner
        void setProgress(qreal progress_);
        // Progress spinner
        void setProgress(qint64 progress_, qint64 expected_);
        // Start spinner
        void start();
        // Stop spinner
        void stop();
        // Unpause spinner
        void unpause();

    protected:
        // Paint spinner
        void paintEvent(QPaintEvent * event);

    private:
        bool _active;
        bool _autoHide;
        QColor _color;
        bool _paused;
        qreal _progress;
        QTime _started;
        QTimer _timer;

    }; /* class Spinner */

} // namespace Utopia

#endif /* Utopia_QT_SPINNER_H */
