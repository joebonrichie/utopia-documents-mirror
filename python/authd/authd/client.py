###############################################################################
#   
#    This file is part of the Utopia Documents application.
#        Copyright (c) 2008-2017 Lost Island Labs
#            <info@utopiadocs.com>
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

import authd.model
import authd.converter
import os
import sys
import urllib2

from lxml import etree

import logging
log = logging.getLogger(__name__)


class Client:
    '''Client class to talk to an authd server.'''

    def __init__(self, uri = None, token_callback = None, converter = authd.converter.XML):
        self.agent = 'Authd/{0} Python-urllib2/{1}'.format(authd.model.VERSION, '.'.join((str(i) for i in sys.version_info[:3])))
        self.uri = os.getenv('AUTHD_SERVER_URI', uri)
        self.converter = converter
        self.user = None
        self.credentials_callback = credentials_callback
        if self.credentials_callback is None:
            try:
                from utopia import auth, agent_string
                self.credentials_callback = auth.TokenGenerator(self.uri)
                self.agent = '%s %s' % (agent_string, self.agent)
            except ImportError:
                def null_token(old_token):
                    return None
                self.credentials_callback = null_token
                log.error('No login callback set - authentication impossible')
        self._options()

    def _make_request(self, url, data = None, method = 'GET', headers = {}):
        # Start with the user agent
        accum = {'User-Agent': self.agent}
        # Overlay the provided headers
        accum.update(headers)
        # Authentication information may need to be added
        if self.user is not None:
            token = self.user.token
            accum['Authorization'] = 'Kend {}'.format(token)
        # Ensure the correct method is used
        request = urllib2.Request(url, data, accum)
        request.get_method = lambda: method
        return request

    def _options(self):
        # Execute an OPTIONS request on the authd endpoint
        request = self._make_request(self.uri, method='OPTIONS')
        response = urllib2.urlopen(request)
        xml = etree.parse(response).getroot()

        # Check the response matches this library's version number
        if xml.get('version') != authd.model.VERSION:
            raise RuntimeError('Client version ({0}) does not match server version ({1})'.format(authd.model.VERSION, xml.get('version')))

        # Collect authentication methods
        self.methods = xml.xpath('authd:backend/@name', namespaces=self.converter.NS)

    def authenticate(self, username = None, password = None, method = None):
        # Choose first appropriate service if none is provided
        if method is None:
            if 'kend' in self.methods:
                method = 'kend'
            elif len(self.methods) > 0:
                method = self.methods[0]
            else:
                raise RuntimeError('No appropriate authentication method found')
        elif method not in self.methods:
            raise RuntimeError('Requested authentication method ({0}) not supported'.format(method))
        self.method = method

        # Create an authentication request and post it to the server
        credentials = authd.model.LoginCredentials(service=self.method, credentials=dict(user=username, password=password))
        data, mime_type = self.converter.serialise(credentials)
        url = '{0}/{1}'.format(self.uri, 'signin')
        request = urllib2.Request(url, data, headers={'Content-Type': mime_type, 'User-Agent': self.agent})
        response = urllib2.urlopen(request)

        # Parse the response
        self.user = self.converter.parse(response, authd.model.LoginResult)



#########################################################################################

if __name__ == '__main__':

    client = Client('https://utopia.cs.manchester.ac.uk/authd/0.3')
    client.authenticate(username='davethorne@gmail.com', password='rutherford')
    print client.user.uid
    print client.user.token
