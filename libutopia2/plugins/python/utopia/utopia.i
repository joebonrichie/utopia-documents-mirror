/*****************************************************************************
 *  
 *   This file is part of the Utopia Documents application.
 *       Copyright (c) 2008-2014 Lost Island Labs
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

/* Swig Python interface to utopia globals */
%feature("autodoc", "0");
%module (docstring="Utopia API") utopiacore

%{
#include <utopia2/global.h>
#include "version_p.h"
#include <utopia2/pacproxyfactory.h>
#include <utopia2/networkaccessmanager.h>

#ifdef _WIN32
#include <windows.h>
#define sleep(n) Sleep(1000*n)
#else
#include <unistd.h>
#endif

#include <QCryptographicHash>
#include <QEventLoop>
#include <QNetworkProxyFactory>
#include <QNetworkProxyQuery>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QString>
#include <QTime>
#include <QUrl>
%}

%include std_string.i
%include std_vector.i

namespace std {
   %template(StringList) vector<string>;
}

%inline %{
/* Fetch version info */
int versionMajor() { return Utopia::versionMajor(); }
int versionMinor() { return Utopia::versionMinor(); }
int versionPatch() { return Utopia::versionPatch(); }
std::string versionString() { return Utopia::versionString().toStdString(); }
std::string revisionHash() { return Utopia::revisionHash().toStdString(); }
std::string revisionBranch() { return Utopia::revisionBranch().toStdString(); }

/* Fetch proxy information */
std::string queryProxyString(const std::string & url)
{
    if (Utopia::PACProxyFactory * proxyFactory = Utopia::globalProxyFactory())
    {
        return proxyFactory->queryProxyString(QNetworkProxyQuery(QUrl(QString::fromStdString(url)))).toStdString();
    }
    else
    {
        return "DIRECT";
    }
}

/* Fetch proxy authentication information */
std::vector< std::string > fetchProxyCredentials(const std::string & realm, const std::string & uri, bool refresh)
{
    if (Utopia::PACProxyFactory * proxyFactory = Utopia::globalProxyFactory())
    {
        QString userName;
        QString password;
        proxyFactory->getCredentials(QString::fromStdString(realm),
                                     QString::fromStdString(uri),
                                     refresh,
                                     &userName,
                                     &password);
        std::vector< std::string > credentials;
        credentials.push_back(userName.toStdString());
        credentials.push_back(password.toStdString());
        return credentials;
    }
    else
    {
        return std::vector< std::string >();
    }
}

/* Fetch proxy authentication information */
std::string fetchELS(const std::string & doi_, const std::string & view_)
{
    static QMap< QString, QTime > since;
    QString doi(QString::fromStdString(doi_));
    QString view(QString::fromStdString(view_));
    QUrl url("https://charlie.cs.man.ac.uk/refs/content/article/DOI:" + doi + "?view=" + view);

    if (since.contains(view) && since[view].elapsed() < 15000) {
        sleep(15 - since[view].elapsed() / 1000);
    }

    QNetworkRequest request(url);
    request.setRawHeader("X-ELS-UtopiaKey", "132788d38b8d1173");
    request.setRawHeader("Accept", "text/xml");
    QEventLoop loop;
    boost::shared_ptr< Utopia::NetworkAccessManager > netAccess(Utopia::NetworkAccessManagerMixin().networkAccessManager());
    QNetworkReply * reply = netAccess->getAndBlock(request);
    std::string response(reply->readAll().constData());
    reply->deleteLater();

    since[view].start();

    return response;
}

/* Generate an SD PDF url for Utopia */
std::string checksumSD(const std::string & query)
{
    QCryptographicHash hasher(QCryptographicHash::Md5);
    std::string salted = query + "S1kclGPeB62(5phgVWw-YmJmqRddM-Gs";
    hasher.addData(salted.c_str(), salted.size());
    return hasher.result().toHex().constData();
}

%}

%init
%{
    //Utopia::init();
%}

%exception
%{
  Py_BEGIN_ALLOW_THREADS
  $action
  Py_END_ALLOW_THREADS
%}

%pythoncode
%{

# Global version info
version_info = (versionMajor(), versionMinor(), versionPatch(), revisionHash())
agent_string = "Utopia/%d.%d.%d" % version_info[:3]
%}

%constant int _moduleVersionMajor = UTOPIA_VERSION_MAJOR;
%constant int _moduleVersionMinor = UTOPIA_VERSION_MINOR;
%constant int _moduleVersionPatch = UTOPIA_VERSION_PATCH;
%constant char * _moduleRevisionHash = UTOPIA_REVISION_HASH;

%pythoncode
%{
# Ensure valid version
assert version_info == (_moduleVersionMajor,
                        _moduleVersionMinor,
                        _moduleVersionPatch,
                        _moduleRevisionHash),\
    'Module version does not match installed library version.'

def proxyUrllib2():
    import urllib2, ntlm_auth
    import re

    class UtopiaProxyNtlmAuthHandler(ntlm_auth.ProxyNtlmAuthHandler):
        def http_error_407(self, req, fp, code, msg, headers):
            try:
                return ntlm_auth.ProxyNtlmAuthHandler.http_error_407(self, req, fp, code, msg, headers)
            except urllib2.HTTPError as e:
                if e.code == 407:
                    if req.has_header('Proxy-Authorization'.capitalize()):
                        self.passwd.invalidate(req.get_host())
                raise

    class UtopiaProxyBasicAuthHandler(urllib2.ProxyBasicAuthHandler):
        def http_error_407(self, req, fp, code, msg, headers):
            try:
                return urllib2.ProxyBasicAuthHandler.http_error_407(self, req, fp, code, msg, headers)
            except urllib2.HTTPError as e:
                if e.code == 407:
                    if req.has_header('Proxy-Authorization'.capitalize()):
                        self.passwd.invalidate(req.get_host())
                raise

    class UtopiaProxyDigestAuthHandler(urllib2.ProxyDigestAuthHandler):
        def http_error_407(self, req, fp, code, msg, headers):
            try:
                return urllib2.ProxyDigestAuthHandler.http_error_407(self, req, fp, code, msg, headers)
            except urllib2.HTTPError as e:
                if e.code == 407:
                    if req.has_header('Proxy-Authorization'.capitalize()):
                        self.passwd.invalidate(req.get_host())
                raise

    class UtopiaProxyPasswordMgr(urllib2.HTTPPasswordMgr):
        add_password = None
        cache = {}
        def invalidate(self, authuri):
            if authuri in self.cache:
                del self.cache[authuri]
        def find_user_password(self, realm, authuri):
            #print "find_user_password,", authuri
            # Check from cache
            if authuri not in self.cache:
                # Fetch from Utopia
                self.cache[authuri] = fetchProxyCredentials(realm or "", authuri or "", False)
            #print "  ->", self.cache[authuri][0]
            return self.cache[authuri]

    class UtopiaProxyHandler(urllib2.ProxyHandler):
        def __init__(self):
            self.active = []
            urllib2.ProxyHandler.__init__(self, proxies = {})
        def default_open(self, req):
            if req not in self.active:
                proxies = queryProxyString(req.get_full_url().encode('utf8')).split(';')
                if len(proxies) > 0:
                    for proxy in proxies[:1]:
                        type, space, hostport = proxy.strip().partition(' ')
                        if space == ' ' and type == 'PROXY':
                            req.set_proxy(hostport, req.get_type())
                            self.active.append(req)
                            try:
                                result = self.parent.open(req)
                            except urllib2.URLError as e:
                                print e
                                if isinstance(e, urllib2.HTTPError) and e.code == 407 or proxy == proxies[-1]:
                                    raise
                                else:
                                    result = None
                            finally:
                                self.active.remove(req)
                            if result is not None:
                                return result
            return None

    passwordMgr = UtopiaProxyPasswordMgr()
    opener = urllib2.build_opener(UtopiaProxyHandler(),
                                  UtopiaProxyBasicAuthHandler(passwordMgr),
                                  UtopiaProxyDigestAuthHandler(passwordMgr),
                                  UtopiaProxyNtlmAuthHandler(passwordMgr))
    # relying on Python's standard user agent string breaks some fetching attempts
    #opener.addheaders = [('User-agent', agent_string + ' ' + dict(opener.addheaders).get('User-agent', '')).strip())]
    opener.addheaders = [('User-Agent', agent_string.strip())]
    urllib2.install_opener(opener)

import exceptions

class PluginError(exceptions.RuntimeError):
    pass

class Cancellation(exceptions.RuntimeError):
    '''The user has cancelled this task.'''
    pass

import utopia.extension

class Configurator(utopia.extension.Extension):
	pass

# This will be augmented by boost::python
class Citation:
    pass
citation = Citation()

__all__ = [
    'versionMajor',
    'versionMinor',
    'versionPatch',
    'versionString',
    'revisionHash',
    'revisionBranch',
    'version_info',
    'agent_string',
    'queryProxyString',
    'proxyUrllib2',
    'fetchELS',
    'checksumSD',
    'Configurator',
    'citation',
    ]
%}
