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

#ifndef BASIC_BASICCREDENTIALMANAGER_H
#define BASIC_BASICCREDENTIALMANAGER_H

#include <utopia2/auth/authagent.h>
#include <utopia2/auth/credentialmanager.h>
#include <utopia2/auth/service.h>

#include <QString>

namespace Kend
{

    class BasicCredentialManager : public CredentialManager
    {
    public:
        BasicCredentialManager()
            : CredentialManager()
        {}

        void captureCredentials(Service * service, AuthAgent * authAgent)
        {
            // By default, do nothing
            authAgent->captureCompleted(service, service->credentials());
        }

        void logOut(Service * service, AuthAgent * authAgent)
        {
            // By default, do nothing
            authAgent->logOutCompleted(service);
        }

        QString schema() const
        {
            return "http://utopia.cs.manchester.ac.uk/authd/credentials#basic";
        }

    }; /* class BasicCredentialManager */

} // namespace Kend

#endif // BASIC_BASICCREDENTIALMANAGER_H
