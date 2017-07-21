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

import json
import hashlib
import urllib2
import utopia.citation
import utopia.tools.cited

class CitedUnstructuredIdentifier(utopia.citation.Resolver):
    '''Give unstructured citations a hash identifier'''

    def resolve(self, citations, document = None):
        # Get all the citations that are unstructured
        for citation in citations:
            unstructured = utopia.citation.pick(citation, 'unstructured', default=None)
            if unstructured is not None:
                # Hash the unstructured text
                unstructured = unstructured.strip()
                hash = hashlib.sha256(unstructured.encode('utf8')).hexdigest()
                utopia.citation.set_by_keyspec(citation, 'identifiers[unstructured]', hash)

    def purposes(self):
        return 'identify'

    def weight(self):
        return -9200

class CitedResolver(utopia.citation.Resolver):
    '''Submit citation identifiers to cited for resolution'''

    def resolve(self, citations, document = None):
        identifiers = utopia.citation.pick_from(citations, 'identifiers', default={})
        if len(identifiers) > 0:
            return utopia.tools.cited.resolve(**identifiers)

    def purposes(self):
        return 'identify'

    def weight(self):
        return -9100

class CitedParser(utopia.citation.Resolver):
    '''Submit unstructured citations to cited for parsing'''

    def resolve(self, citations, document = None):
        # Multiple responses leads to a no-op
        for citation in citations:
            if utopia.citation.pick(citation, 'provenance/whence', default=None) == 'cermine':
                # Bail if cermine results are already present
                return None

        # Get all the citations that don't look structured
        structure_keys = set(['title', 'authors', 'year'])
        citation = {}
        unstructured = utopia.citation.pick_from(citations, 'unstructured', default=None, record_in=citation)
        if unstructured is not None and len(structure_keys & set(unstructured.citation.keys())) == 0:
            structured = utopia.tools.cited.parse(unstructured)
            if len(structured) > 0:
                citation.update(structured[0])
                return citation

    def purposes(self):
        return 'identify'

    def weight(self):
        return -9000

class CitedSubmitter(utopia.citation.Resolver):
    '''Submit citation information to cited'''

    def resolve(self, citations, document = None):
        utopia.tools.cited.submit(citations)

    def provenance(self):
        return {'whence': 'cited'}

    def purposes(self):
        return 'dereference'

    def weight(self):
        return 100000
