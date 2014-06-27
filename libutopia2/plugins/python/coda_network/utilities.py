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

#!/usr/bin/env python2.6
# encoding: utf-8
##############################################################################
##############################################################################
##############################################################################
###
### utilities.py
###
### This is additional stuff for the cool network shiznit.
###
##############################################################################
##############################################################################
##############################################################################

import time

from coda_network import urllib2

###########################################################################
#
def build_opener(cert=None, proxy_handlers=None, auth_handlers=None, keepalive=False):
    """Builds the list of opener objects required for the specific type of request."""
    handlers = [urllib2.HTTPSHandler(tcp_keepalive=keepalive, cert=cert)]

    if proxy_handlers:
        handlers.extend(proxy_handlers)

    if auth_handlers:
        handlers.extend(auth_handlers)

    return urllib2.build_opener(*handlers)
#
###########################################################################


###########################################################################
#
def create_download_handle(url, postdata, proxy_handlers, auth_handlers, cert, last_modified=None, etag=None, headers=None, method=None, timeout_sec=None):
    """
    Wraps handle download creation.
    """
    if postdata is None:
        url_req = urllib2.Request(url, method=method)
    else:
        url_req = urllib2.Request(url, postdata, method=method)

    if headers:
        for item, val in headers:
            url_req.add_header(item, val)

    # These headers are used for checking whether there has been any modifications.
    # It will return a 304 by way of an exception on open()
    if last_modified:
        url_req.add_header('If-Modified-Since', time.strftime('%a, %d %b %Y %H:%M:%S GMT', time.localtime(last_modified)))
    if etag:
        url_req.add_header('If-None-Match', etag)

    url_opener = build_opener(cert, proxy_handlers, auth_handlers)

    if timeout_sec:
        return url_opener.open(url_req, timeout=timeout_sec)

    return url_opener.open(url_req)
#
###########################################################################


##########################################################################
#
def do_download(url, postdata, proxy_handlers=None, auth_handlers=None, cert=None, timeout_sec=None):
    """Perform a simple download.  Returns (code, data)."""
    handle = None
    try:
        handle = create_download_handle(url, postdata, proxy_handlers, auth_handlers, cert, timeout_sec=timeout_sec)
        return 200, handle.read()

    except urllib2.HTTPError, e:
        return e.code, None

    except Exception, e:
        return 404, None

    finally:
        if handle:
            handle.close()
#
##########################################################################
