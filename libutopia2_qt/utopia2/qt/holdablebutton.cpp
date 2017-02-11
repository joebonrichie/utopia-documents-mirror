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

#include <utopia2/qt/holdablebutton.h>

#include <QTimer>

#include <QDebug>

namespace Utopia
{

    /////////////////////////////////////////////////////////////////////////////////////
    /// HoldableButtonPrivate

    class HoldableButtonPrivate
    {
    public:
        QTimer holdTimer;
    }; // class HoldableButtonPrivate




    /////////////////////////////////////////////////////////////////////////////////////
    /// HoldableButton

    HoldableButton::HoldableButton(QWidget * parent)
        : QToolButton(parent), d(new HoldableButtonPrivate)
    {
        d->holdTimer.setSingleShot(true);
        d->holdTimer.setInterval(300);
        QObject::connect(&d->holdTimer, SIGNAL(timeout()), this, SIGNAL(held()));
    }

    int HoldableButton::holdInterval() const
    {
        return d->holdTimer.interval();
    }

    void HoldableButton::mousePressEvent(QMouseEvent * event)
    {
        d->holdTimer.start();
        QToolButton::mousePressEvent(event);
    }

    void HoldableButton::mouseReleaseEvent(QMouseEvent * event)
    {
        d->holdTimer.stop();
        QToolButton::mouseReleaseEvent(event);
    }

    void HoldableButton::setHoldInterval(int interval)
    {
        d->holdTimer.setInterval(interval);
    }

} // namespace Utopia
