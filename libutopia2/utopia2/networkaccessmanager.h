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

#ifndef UTOPIA_NETWORKACCESSMANAGER_H
#define UTOPIA_NETWORKACCESSMANAGER_H

#include <utopia2/config.h>

#include <boost/shared_ptr.hpp>

#include <QNetworkAccessManager>

namespace Utopia
{

    class NetworkAccessManagerPrivate;
    class LIBUTOPIA_API NetworkAccessManager : public QNetworkAccessManager
    {
        Q_OBJECT

    public:
        NetworkAccessManager(QObject * parent = 0);
        ~NetworkAccessManager();

        void setUserAgentString(const QString & userAgentString);
        QString userAgentString() const;

        QNetworkReply *	getAndBlock(const QNetworkRequest & request);

    protected slots:
        void on_downloadProgress(qint64, qint64);
        void on_finished();
        void on_sslErrors(const QList< QSslError > & errors);
        void on_timeout(QObject * object);
        void on_uploadProgress(qint64, qint64);

    signals:
        void restartTimers();
        void stopTimers();

    protected:
        QNetworkReply *	createRequest(Operation op, const QNetworkRequest & req, QIODevice * outgoingData = 0);

        // FIXME this should not be needed (auth should be done inside this class) [timeouts]
        bool event(QEvent * e); // for dynamic propery changes

        NetworkAccessManagerPrivate * d;
    };




    class NetworkAccessManagerMixin
    {
    public:
        boost::shared_ptr< NetworkAccessManager > networkAccessManager() const;
    private:
        mutable boost::shared_ptr< NetworkAccessManager > _networkAccessManager;
    };

}

#endif // UTOPIA_NETWORKACCESSMANAGER_H
