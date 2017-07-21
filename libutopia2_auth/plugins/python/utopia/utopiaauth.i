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

/* Swig Python interface to utopia auth */
%feature("autodoc", "0");
%module (docstring="Utopia Documents Authentication API") utopiaauth
%include std_string.i
%{

#define SWIG_FILE_WITH_INIT

#include <utopia2/auth/authagent.h>
#include <utopia2/auth/service.h>
#include <utopia2/auth/servicemanager.h>
#include <string>

#include <QEventLoop>
#include <QDebug>

std::string _getNewAuthToken(const std::string & serverUri, const std::string & oldToken)
{
    boost::shared_ptr< Kend::ServiceManager > serviceManager(Kend::ServiceManager::instance());
    if (serviceManager->count() == 1) {
        Kend::Service * service = serviceManager->serviceAt(0);
        if (service && service->isLoggedIn()) {
            if (service->authenticationToken() == QString::fromStdString(oldToken)) {
                QEventLoop loop;
                QObject::connect(service, SIGNAL(serviceStarted()), &loop, SLOT(quit()));
                QObject::connect(service, SIGNAL(serviceError()), &loop, SLOT(quit()));

                boost::shared_ptr< Kend::AuthAgent > authAgent(Kend::AuthAgent::instance());
                if (QMetaObject::invokeMethod(authAgent.get(), "logIn", Qt::QueuedConnection,
                                              Q_ARG(Kend::Service *, service))) {
                    loop.exec();
                }
                if (service->isLoggedIn()) {
                    return service->authenticationToken().toUtf8().data();
                }
            } else {
                return service->authenticationToken().toUtf8().data();
            }
        }
    }

    return ""; // This will happen if there is not exactly one logged in service running
}

std::string _getServiceBaseUri(const std::string & resource)
{
    boost::shared_ptr< Kend::ServiceManager > serviceManager(Kend::ServiceManager::instance());
    if (serviceManager->count() == 1) {
        Kend::Service * service = serviceManager->serviceAt(0);
        if (service) {
            if (resource == "documents") {
                return service->resourceUrl(Kend::Service::DocumentsResource).toString().toUtf8().data();
            } else if (resource == "annotations") {
                return service->resourceUrl(Kend::Service::AnnotationsResource).toString().toUtf8().data();
            } else if (resource == "authentication") {
                return service->resourceUrl(Kend::Service::AnnotationsResource).toString().toUtf8().data();
            } else if (resource == "definitions") {
                return service->resourceUrl(Kend::Service::DefinitionsResource).toString().toUtf8().data();
            }
        }
    }

    return ""; // This will happen if there is not exactly one logged in service running
}

%}

std::string _getNewAuthToken(const std::string & serverUri, const std::string & oldToken);
std::string _getServiceBaseUri(const std::string & resource);

%exception
{
    Py_BEGIN_ALLOW_THREADS
    $action
    Py_END_ALLOW_THREADS
}

%pythoncode %{
    import threading

    class TokenGenerator:
        states = {}

        def __init__(self, server):
            if server in self.states:
                self.state = self.states[server]
            else:
                self.state = {}
                self.states[server] = self.state
                self.state['server'] = server
                self.state['authToken'] = None
                self.state['lock'] = threading.Lock()

        def __call__(self, oldToken):
            # Only one simulataneous call allowed per auth server
            # All waiting calls block and they all share the returned token
            newToken = self.state['authToken']
            if (newToken is None) or oldToken == newToken:
                if not self.state['lock'].acquire(False):
                    self.state['lock'].acquire()
                else:
                    self.state['authToken'] = _getNewAuthToken(str(self.state['server']), str(oldToken));
                newToken = self.state['authToken']
                self.state['lock'].release()
            return newToken
%}
