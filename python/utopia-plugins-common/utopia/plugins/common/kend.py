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

import kend.client
import kend.model
import utopia.citation

###############################################################################
### Given general information, try to find as many identifiers as possible.
###############################################################################

class DocumentIdentifierResolver(utopia.citation.Resolver):
    '''Resolve a Utopia URI for this document.'''

    def _unidentifiedDocumentRef(self, document):
        '''Compile a document reference from a document's fingerprints'''
        evidence = [kend.model.Evidence(type='fingerprint',
                                        data=f,
                                        srctype='document') for f in document.fingerprints()]
        return kend.model.DocumentReference(evidence=evidence)

    def _identifyDocumentRef(self, documentref):
        '''Find a URI from a document reference, resolving it if necessary'''
        id = getattr(documentref, 'id', None)
        if id is None:
            documentref = kend.client.Client().documents(documentref)
            id = getattr(documentref, 'id', None)
        return id

    def _resolveDocumentId(self, document):
        '''Return the document URI for the given document'''
        documentref = self._unidentifiedDocumentRef(document)
        return self._identifyDocumentRef(documentref)

    def resolve(self, citations, document = None):
        if document is not None:
            utopia_id = utopia.citation.pick_from(citations, 'identifiers[utopia]', None)
            if utopia_id is None:
                # See if the document URI is in the document already
                utopia_id = utopia.tools.utils.metadata(document, 'identifiers[utopia]')
                if utopia_id is None:
                    # Resolve this document's URI and store it in the document
                    utopia_id = self._resolveDocumentId(document)
                    # We should now have a document URI
                    return {'identifiers': {'utopia': utopia_id}}

    def provenance(self):
        return {'whence': 'kend'}

    def purposes(self):
        return 'identify'

    def weight(self):
        return -10000
