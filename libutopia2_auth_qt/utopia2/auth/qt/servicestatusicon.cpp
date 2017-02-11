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

#include <utopia2/auth/qt/servicestatusicon_p.h>
#include <utopia2/auth/qt/servicestatusicon.h>

#include <QPainter>

#include <QtDebug>

namespace Kend
{

    ServiceStatusIconPrivate::ServiceStatusIconPrivate(ServiceStatusIcon * icon)
        : QObject(icon), icon(icon), pixmap(QString(":/icons/servicestatusgrey.png")), serviceManager(ServiceManager::instance())
    {
        connect(serviceManager.get(), SIGNAL(serviceStateChanged(Kend::Service *, Kend::Service::ServiceState)),
                this, SLOT(onServiceStateChanged(Kend::Service *, Kend::Service::ServiceState)));
        connect(this, SIGNAL(update()), icon, SLOT(update()));

        // Initial icon update
        updateIcon();
    }

    void ServiceStatusIconPrivate::onServiceStateChanged(Service *, Service::ServiceState)
    {
        updateIcon();
    }

    void ServiceStatusIconPrivate::updateIcon()
    {
        static struct { int online, offline, busy, error; } state = { 0, 0, 0, 0 };
        serviceManager->getStatistics(&state.online, &state.offline, &state.busy, &state.error);
        if (state.error > 0) {
            pixmap.load(":/icons/servicestatusred.png");
            icon->setToolTip("Service error - click here to resolve");
        } else if (state.online > 0) {
            pixmap.load(":/icons/servicestatusgreen.png");
            icon->setToolTip("Online");
        } else {
            pixmap.load(":/icons/servicestatusgrey.png");
            icon->setToolTip("Offline");
        }
        emit update();
    }




    ServiceStatusIcon::ServiceStatusIcon(QWidget * parent)
        : QFrame(parent), d(new ServiceStatusIconPrivate(this))
    {
        setMinimumSize(QSize(32, 32));
    }

    void ServiceStatusIcon::paintEvent(QPaintEvent * /*event*/)
    {
        QSize pixmapSize(d->pixmap.size());
        pixmapSize.scale(QSize(15, 15), Qt::KeepAspectRatio);
        if (pixmapSize.height() > d->pixmap.height() ||
            pixmapSize.width() > d->pixmap.width()) {
            pixmapSize = d->pixmap.size();
        }
        QRect pixmapRect(QPoint(0, 0), pixmapSize);
        QPainter p(this);
        p.drawPixmap(QRect(rect().center() - pixmapRect.center(), pixmapSize), d->pixmap);
    }

}
