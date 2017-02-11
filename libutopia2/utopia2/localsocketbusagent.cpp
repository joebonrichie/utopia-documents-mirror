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

#include <utopia2/localsocketbusagent.h>
#include <utopia2/localsocketbusagent_p.h>

#include <QJsonDocument>
#include <QLocalSocket>
#include <QUuid>

#include <QDebug>

namespace Utopia
{

    LocalSocketBusAgentPrivate::LocalSocketBusAgentPrivate(LocalSocketBusAgent * busAgent, QString serverName, QString privilegedUuid)
        : QObject(busAgent), busAgent(busAgent), serverName(serverName), privilegedUuid(privilegedUuid), busId(QUuid::createUuid().toString()), client(0)
    {
        server.listen(serverName);
    }

    LocalSocketBusAgentPrivate::~LocalSocketBusAgentPrivate()
    {}

    void LocalSocketBusAgentPrivate::newConnection()
    {
        // FIXME think about multiple connections! Cleaning up client(s)
        if (!client) {
            client = server.nextPendingConnection();
            connect(client, SIGNAL(readyRead()), this, SLOT(readyRead()));
        }
    }

    void LocalSocketBusAgentPrivate::readyRead()
    {
        if (client) {
            QJsonParseError error;
            QVariant data = QJsonDocument::fromJson(client->readAll(), &error).toVariant();
            if (!data.isNull()) {
                if (privilegedUuid.isEmpty()) {
                    busAgent->postToBus(data);
                } else {
                    busAgent->postToBus(privilegedUuid, data);
                }
            }
        }
    }




    LocalSocketBusAgent::LocalSocketBusAgent(QString serverName, QString privilegedUuid, QObject * parent)
        : QObject(parent), BusAgent(), d(new LocalSocketBusAgentPrivate(this, serverName, privilegedUuid))
    {}

    LocalSocketBusAgent::~LocalSocketBusAgent()
    {}

    QString LocalSocketBusAgent::busId() const
    {
        return d->busId;
    }

    void LocalSocketBusAgent::receiveFromBus(const QString & sender, const QVariant & data)
    {
        if (d->client && (d->privilegedUuid.isEmpty() || sender == d->privilegedUuid) && !data.isNull()) {
            d->client->write(QJsonDocument::fromVariant(data).toJson(QJsonDocument::Compact));
        }
    }

    void LocalSocketBusAgent::resubscribeToBus()
    {}

} // namespace Utopia
