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

#include <utopia2/bus.h>
#include <utopia2/bus_p.h>
#include <utopia2/busagent.h>
#include <utopia2/localsocketbusagent.h>

#include <QSet>
#include <QString>
#include <QStringList>

#include <QDebug>

namespace Utopia
{

    BusPrivate::BusPrivate(Bus * bus)
        : bus(bus)
    {}

    void BusPrivate::routeMessage(const QString & senderBusId, const QString & recipientBusId, const QVariant & data)
    {
        QSet< BusAgent * > recipients;

        /////////////////////////////////////////////////////////////////////////////////
        /// DIRECT ROUTING
        if (!recipientBusId.isEmpty()) {

            /////////////////////////////////////////////////////////////////////////////
            /// TO OPEN SUBSCRIPTIONS
            if (listeners.contains(recipientBusId)) {
                foreach (BusAgent * listener, listeners.value(recipientBusId)) {
                    recipients.insert(listener);
                }
            }

            /////////////////////////////////////////////////////////////////////////////
            /// TO ROUTED SUBSCRIPTIONS
            if (!senderBusId.isEmpty()) {
                foreach (BusAgent * listener, routes.value(senderBusId)) {
                    if (listener->busId() == recipientBusId) {
                        recipients.insert(listener);
                    }
                }
            }

        /////////////////////////////////////////////////////////////////////////////////
        /// BROADCAST ROUTING
        } else {

            /////////////////////////////////////////////////////////////////////////////
            /// TO OPEN SUBSCRIPTIONS
            QMapIterator< QString, QSet< BusAgent * > > iter(listeners);
            while (iter.hasNext()) {
                iter.next();
                foreach (BusAgent * listener, iter.value()) {
                    recipients.insert(listener);
                }
            }

            /////////////////////////////////////////////////////////////////////////////
            /// TO ROUTED SUBSCRIPTIONS
            foreach (BusAgent * listener, routes.value(senderBusId)) {
                recipients.insert(listener);
            }

        }

        // Send messages to recipients
        foreach (BusAgent * recipient, recipients) {
            //qDebug() << "  -->" << recipient;
            recipient->receiveFromBus(senderBusId, data);
        }
    }



    namespace
    {

        void _remove_from_map(QMap< QString, QSet< BusAgent * > > & map, BusAgent * doomed)
        {
            QMutableMapIterator< QString, QSet< BusAgent * > > iter(map);
            while (iter.hasNext()) {
                iter.next();
                iter.value().remove(doomed);
                if (iter.value().isEmpty()) {
                    iter.remove();
                }
            }
        }

    }




    Bus::Bus(QObject * parent)
        : QObject(parent), d(new BusPrivate(this))
    {}

    // FIXME what are these for?
    QString Bus::createChildProcessAgent(const QString & program, const QStringList & arguments, QString privilegedUuid)
    {
        return QString();
    }

    // FIXME what are these for?
    QString Bus::createChildPythonProcessAgent(const QString & program, const QStringList & arguments, QString privilegedUuid)
    {
        return QString();
    }

    QString Bus::createLocalServerAgent(QString serverName, QString privilegedUuid)
    {
        LocalSocketBusAgent * agent = new LocalSocketBusAgent(serverName, privilegedUuid, this);
        if (privilegedUuid.isEmpty()) {
            subscribe(agent);
        } else {
            subscribeTo(agent, privilegedUuid);
        }
        return agent->busId();
    }

    void Bus::sendTo(BusAgent * sender, const QString & recipientBusId, const QVariant & data)
    {
        //qDebug() << "sendTo(" << dynamic_cast< QObject * >(sender) << "," << recipientBusId << "," << data;
        QString senderBusId = sender ? sender->busId() : QString();

        // Asynchronously send messages
        QMetaObject::invokeMethod(d, "routeMessage", Qt::QueuedConnection,
                                  Q_ARG(QString, senderBusId), Q_ARG(QString, recipientBusId), Q_ARG(QVariant, data));
    }

    void Bus::send(BusAgent * sender, const QVariant & data)
    {
        sendTo(sender, QString(), data);
    }

    void Bus::subscribe(BusAgent * listener)
    {
        d->listeners[listener->busId()].insert(listener);
    }

    void Bus::subscribeTo(BusAgent * listener, const QString & sender)
    {
        d->routes[sender].insert(listener);
    }

    void Bus::unsubscribe(BusAgent * listener)
    {
        _remove_from_map(d->listeners, listener);
    }

    void Bus::unsubscribeFrom(BusAgent * listener, const QString & sender)
    {
        if (d->routes.contains(sender)) {
            d->routes[sender].remove(listener);
            if (d->routes[sender].isEmpty()) {
                d->routes.remove(sender);
            }
        }
    }

    void Bus::unsubscribeFromAll(BusAgent * listener)
    {
        unsubscribe(listener);
        _remove_from_map(d->routes, listener);
    }

} // namespace Utopia
