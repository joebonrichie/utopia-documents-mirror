###############################################################################
#   
#    This file is part of the Utopia Documents application.
#        Copyright (c) 2008-2014 Lost Island Labs
#    
#    Utopia Documents is free software: you can redistribute it and/or modify
#    it under the terms of the GNU GENERAL PUBLIC LICENSE VERSION 3 as
#    published by the Free Software Foundation.
#    
#    Utopia Documents is distributed in the hope that it will be useful, but
#    WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
#    Public License for more details.
#    
#    In addition, as a special exception, the copyright holders give
#    permission to link the code of portions of this program with the OpenSSL
#    library under certain conditions as described in each individual source
#    file, and distribute linked combinations including the two.
#    
#    You must obey the GNU General Public License in all respects for all of
#    the code used other than OpenSSL. If you modify file(s) with this
#    exception, you may extend this exception to your version of the file(s),
#    but you are not obligated to do so. If you do not wish to do so, delete
#    this exception statement from your version.
#    
#    You should have received a copy of the GNU General Public License
#    along with Utopia Documents. If not, see <http://www.gnu.org/licenses/>
#   
###############################################################################

#!/usr/bin/env python
# encoding: utf-8
##############################################################################
##############################################################################
##############################################################################
###
### oauth_auth.py
###
### This is for OAUTH authentication.
###
##############################################################################
##############################################################################
##############################################################################

import re

from coda_network import urllib2
from coda_network.oauth import get_oauth_header, AUTH_HEADER_OAUTH

class AbstractOauthAuthHandler:

    # allow for double- and single-quoted realm values
    # (single quotes are a violation of the RFC, but appear in the wild)
    #rx = re.compile('(?:.*,)*[ \t]*([^ \t]+)[ \t]+'
    #                'realm=(["\'])(.*?)\\2', re.I)
    rx = re.compile('([^\"\'=]*)=[\"\']([^\"\'=]*)[\"\'][ \t,]*', re.IGNORECASE)
    def __init__(self, password_mgr=None):
        if password_mgr is None:
            password_mgr = urllib2.HTTPPasswordMgr()
        self.passwd = password_mgr
        self.add_password = self.passwd.add_password
        self.retried = 0

    def http_error_auth_reqed(self, authreq, host, req, headers):
        # host may be an authority (without userinfo) or a URL with an
        # authority
        # XXX could be multiple headers
        authreq = headers.get(authreq, None)

        if self.retried > 2:
            # retry sending the username:password 5 times before failing.
            raise urllib2.HTTPError(req.get_full_url(), 401, "oauth auth failed",
                            headers, None)
        else:
            self.retried += 1

        if authreq:
            auth_items = authreq.split(' ')
            scheme = auth_items[0].strip().lower()
            auth_dict = {}
            for auth in auth_items[1:]:
                mo = AbstractOauthAuthHandler.rx.search(auth)
                if mo:
                    name, val = mo.groups()
                    auth_dict[name] = val
            if scheme.lower() == 'oauth':
                return self.retry_http_oauth_auth(host, req, auth_dict.get('realm', ''))

    def retry_http_oauth_auth(self, host, req, realm):
        auth = self.generate_oauth_header(host, req)
        if auth:
            req.add_unredirected_header(self.auth_header, auth)
            return self.parent.open(req, timeout=req.timeout)
        else:
            return None

    def generate_oauth_header(self, host, req):
        consumer, token = self.passwd.find_user_password(None, host)
        if consumer is not None:
            auth = get_oauth_header(req, consumer, token)
            if req.headers.get(self.auth_header, None) != auth:
                return auth
        return None

class HTTPOauthAuthHandler(AbstractOauthAuthHandler, urllib2.BaseHandler):

    auth_header = AUTH_HEADER_OAUTH

    def http_error_401(self, req, fp, code, msg, headers):
        url = req.get_full_url()
        return self.http_error_auth_reqed('www-authenticate',
                                          url, req, headers)
