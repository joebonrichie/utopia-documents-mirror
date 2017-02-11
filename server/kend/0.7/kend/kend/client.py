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

from kend import model
import kend.converter.XML

from lxml import etree
import base64
import collections
import exceptions
import json
import urllib2
import urllib
import os
import sys
import re

__all__ = [
    'Client',
    ]


import logging
log = logging.getLogger(__name__)


class Client:
    """Client access to a kend server"""

    def __init__(self, loginCallback = None, converter = kend.converter.XML):
        self.agent = 'Kend/%s Python-urllib2/%d.%d.%d' % ((model.VERSION,) + sys.version_info[:3])
        self.loginCallback = loginCallback
        if self.loginCallback is None:
            try:
                from utopia import bridge, auth
                self.loginCallback = auth.TokenGenerator('https://utopia.cs.manchester.ac.uk/authd')
                self.agent = '%s %s' % (bridge.agent_string, self.agent)
            except:
                import traceback
                traceback.print_exc()
                def null_token(old_token):
                    return None
                self.loginCallback = null_token
                log.error('No login callback set - authentication impossible')
        self.converter = converter
        self.authToken = None
        self._login()

    def _login(self):
        # Refresh current authToken
        if self.loginCallback is not None:
            self.authToken = self.loginCallback(getattr(self, 'authToken', None))
            if isinstance(self.authToken, tuple):
                url = 'https://utopia.cs.manchester.ac.uk/auth/0.1/users/signin'
                data = urllib.urlencode({'email':self.authToken[0], 'password':self.authToken[1]})
                result = json.loads(urllib2.urlopen(url, data).read())
                self.authToken = result.get('token')
            return self.authToken
        else:
            log.error('Login unsuccessful - authentication impossible [loginCallback == None]')
            return None

    def _request(self, url, method = 'GET', body_data = None, query_data = None, headers = {}):
        # Reset login attempt count
        loginAttempts = 3

        # Set User Agent string and optional authentication token
        headers['User-Agent'] = self.agent

        # Encode data if a list or mapping
        data = { 'body': body_data, 'query': query_data }
        for name in data.keys():
            dataiter = None
            named_data = data[name]
            if named_data is not None and not isinstance(named_data, (model.Annotation, model.DocumentReference, model.Document, model.Documents)):
                if isinstance(named_data, collections.Sequence) and not isinstance(named_data, (str, unicode)):
                    dataiter = named_data
                elif isinstance(named_data, collections.Mapping):
                    dataiter = named_data.iteritems()
                if dataiter is not None:
                    pairs = []
                    for key, value in dataiter:
                        if isinstance(value, (str, unicode)) or not isinstance(value, collections.Iterable):
                            value = [value]
                        for val in value:
                            pairs.append((key.encode('utf-8'), val.encode('utf-8')))
                    data[name] = urllib.urlencode(pairs)
                    if name == 'body':
                        headers['Content-Type'] = 'application/x-www-form-urlencoded'

        # Get content type correct for KEND objects
        if isinstance(data['body'], (model.Annotation, model.DocumentReference, model.Document, model.Documents)):
            data['body'], headers['Content-Type'] = self.converter.serialise(data['body'])

        # Optionally modify URL with query string
        url = url.rstrip('/')
        if data['query'] is not None:
            url = '%s?%s' % (url, data['query'])

        # Request data
        response = None
        while loginAttempts > 0:
            # Reduce number of login attempts left
            loginAttempts = loginAttempts - 1
            # Set User Agent string and optional authentication token
            if self.authToken is not None:
                if isinstance(self.authToken, (unicode, str)):
                    headers['Authorization'] = 'Kend %s' % self.authToken
                elif len(self.authToken) == 2:
                    try:
                        headers['Authorization'] = 'Basic %s' % base64.standard_b64encode("%s:%s" % self.authToken)
                    except:
                        pass
            # Make request
            kwargs = {}
            if (method, data['body'] is None) not in (('POST', False), ('GET', True)):
                kwargs = {'method': method}
            request = urllib2.Request(url, data['body'], headers, **kwargs)
            try:
                response = urllib2.urlopen(request, timeout=15)
            except urllib2.HTTPError as e:
                if e.code == 401:
                    # Re-authenticate
                    self._login()
                    continue
                else:
                    raise

            break

        if response is not None:
            mime_type = response.info().gettype()
            params = response.info().getplist()
            if mime_type == self.converter.MIME_TYPE:
                for cls in model.__classes__:
                    if ('type=%s' % cls.__mime__) in params or cls.__mime__ in params:
                        response = self.converter.parse(response, cls)
                        break

        return response

    def documents(self, documentref):
        try:
            from utopia import auth
            return self._request(auth._getServiceBaseUri('documents'), method = 'POST', body_data = documentref)
        except ImportError:
            return self._request('https://utopia.cs.manchester.ac.uk/kend/0.7/documents', method = 'POST', body_data = documentref)

    def document(self, uri):
        try:
            from utopia import auth
            return self._request(auth._getServiceBaseUri('documents'), query_data={'document': uri})
        except ImportError:
            return self._request('https://utopia.cs.manchester.ac.uk/kend/0.7/documents', query_data={'document': uri})

    def searchDocuments(self, **kwargs):
        from utopia import auth
        return self._request(auth._getServiceBaseUri('documents'), query_data=kwargs)

    def submitMetadata(self, edit_uri, document):
        return self._request(edit_uri, method = 'PUT', body_data = document)

    def annotations(self, **kwargs):
        try:
            from utopia import auth
            return self._request(auth._getServiceBaseUri('annotations'), query_data = kwargs)
        except ImportError:
            return self._request('https://utopia.cs.manchester.ac.uk/kend/0.7/annotations', query_data = kwargs)

    def lookup(self, **kwargs):
        return self._request('https://utopia.cs.manchester.ac.uk/kend/0.7/define/lookup', query_data = kwargs)

    def lookupDBpedia(self, **kwargs):
        return self._request('https://utopia.cs.manchester.ac.uk/kend/0.7/define/dbpedia', query_data = kwargs)

    def persistDocument(self, ann, **kwargs):
        try:
            from utopia import auth
            kwargs['annotation'] = ann
            self._request(auth._getServiceBaseUri('documents'), method = 'POST', body_data = kwargs)
        except urllib2.HTTPError, e:
            log.error("Failed to save annotations to server: %s" % repr(e))

    def persistAnnotation(self, annotation, **kwargs):
        try:
            from utopia import auth
            if annotation.edit is not None:
                return self._request(annotation.edit, method = 'PUT', body_data = annotation, query_data = kwargs)
            else:
                return self._request(auth._getServiceBaseUri('annotations'), method = 'POST', body_data = annotation, query_data = kwargs)
        except urllib2.HTTPError, e:
            log.error("Failed to save annotation to server: %s" % repr(e))

    def deleteAnnotation(self, annotation, **kwargs):
        try:
            from utopia import auth
            if annotation.edit is not None:
                return self._request(annotation.edit, method = 'DELETE')
        except urllib2.HTTPError, e:
            log.error("Failed to delete annotation from server: %s" % repr(e))
            raise






    def lookupIdentifier(self, **kwargs):
        return self._request('define/identifier', query_data = kwargs)



###################################################################################################
### Testing purposes ##############################################################################
###################################################################################################

if __name__ == '__main__':
    import sys, StringIO

    ### Read in a response from a file ############################################################

    if len(sys.argv) > 1:
        # Provide the path of the example file on the command line
        f = open(sys.argv[1])

        # XSchema?
        xschema = None
        if len(sys.argv) > 2:
            xschema = etree.XMLSchema(file=open(sys.argv[2]))

        # Parse
        root = etree.parse(f)
        response = model.Annotations(root.getroot(), xschema)

        # Serialise by calling toXML() with a suitable namespace map
        # print etree.tostring(
        #     response.toXML(
        #         nsmap = {'def': 'http://utopia.cs.manchester.ac.uk/utopia/annotation/definition'}
        #         ),
        #     pretty_print=True,
        #     xml_declaration=True,
        #     encoding='utf-8'
        #     )

    ### Create a response from scratch ############################################################

    # Make a new response
    annotations = model.Annotations()

    # Add a query
    group = model.Group(parameters = [('term','needle'), ('database','haystack'), ('database','hayloft')])
    annotations.append(group)

    annotation = model.Annotation(concept='type2')
    annotation.update({'{urn:ns1}key1': 'value1', '{urn:ns2}key2': 'value2', '{urn:ns2}key3': 'value3'})
    group.append(annotation)

    anchor = model.DocumentAnchor('https://utopia.cs.manchester.ac.uk/kend/document/2',
                                  extents = [model.Extent(pageboxes = [model.PageBox(10, 11.0, 22.0, 33.0, 44.0)])],
                                  areas = [model.PageBox(1, 1.0, 2.0, 3.0, 4.0)])
    annotation.anchors.append(anchor)

    from kend.converter import XML
    xml, mime = XML.serialise(annotation)
    print xml

    group = model.Group(parameters = [('test','1')])
    annotations.append(group)

    annotation = model.Annotation(concept='type1')
    annotation.update({'{urn:ns1}key3': 'value3'})
    group.append(annotation)

    # Serialise by calling toXML() with a suitable namespace map
    xml, mime = XML.serialise(annotations)

    # Parse it again...
    annotations = XML.parse(xml, kend.model.Annotations)

    # Serialise it again...
    xml2, mime2 = XML.serialise(annotations)

    assert xml == xml2 and mime == mime2

    documentref = model.DocumentReference()
    documentref.evidence.append(model.Evidence(type='filehash', data='HELLO', srctype='document', src='DAVE'))
    xml, mime = XML.serialise(documentref)
    #print xml

    ### Validate generated XML ####################################################################

    # XSchema?
    if len(sys.argv) > 2:
        xschema = etree.XMLSchema(file=open(sys.argv[2]))

        # Parse
        root = etree.parse(StringIO.StringIO(generated))
        response = model.Annotations(root.getroot(), xschema)

    ### Connect to a server #######################################################################

    #client = Client('http://kraken.cs.man.ac.uk/utopia')
    #client.lookup('emra')
