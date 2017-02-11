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

#ifndef AUTHAGENT_H
#define AUTHAGENT_H

#include <utopia2/auth/config.h>

#include <boost/shared_ptr.hpp>

#include <QObject>
#include <QString>
#include <QVariant>

namespace Kend
{

    class Service;

    class AuthAgentPrivate;
    class LIBUTOPIA_AUTH_API AuthAgent : public QObject
    {
        Q_OBJECT

        public:
            // Shared instance pointer
            static boost::shared_ptr< AuthAgent > instance();

            QStringList supportedAuthenticationSchemas() const;

        public slots:
            void captureCompleted(Kend::Service * service, const QVariantMap & credentials = QVariantMap());
            void logIn(Kend::Service * service);
            void logOut(Kend::Service * service);
            void logOutCompleted(Kend::Service * service);

        protected:
            AuthAgent();

        private:
            AuthAgentPrivate * d;

    };

} // namespace Utopia

#endif // AUTHAGENT_H
