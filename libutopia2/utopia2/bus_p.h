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

#ifndef UTOPIA_BUS_P_H
#define UTOPIA_BUS_P_H

#include <QLocalServer>
#include <QLocalSocket>
#include <QMap>
#include <QObject>
#include <QQueue>
#include <QSet>
#include <QString>
#include <QVariant>

namespace Utopia
{

    class BusAgent;
    class Bus;
    class BusPrivate : public QObject
    {
        Q_OBJECT

    public:
        BusPrivate(Bus * bus);

        Bus * bus;

        QMap< QString, QSet< BusAgent * > > listeners;
        QMap< QString, QSet< BusAgent * > > routes;

        QQueue< QPair< QString, QVariant > > messages;
        QMap< QLocalSocket *, QString > localSockets;

    public slots:
        void routeMessage(const QString & senderBusId, const QString & recipientBusId, const QVariant & data);

    }; // class Bus

} // namespace Utopia

#endif // UTOPIA_BUS_P_H
