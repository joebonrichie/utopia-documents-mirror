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

#? name: Google
#? www: http://www.google.com/
#? urls: http://www.google.com/ http://scholar.google.com/


import re
import urllib
import utopia.citation
import utopia.document


class GoogleScholarLinkFinder(utopia.document.LinkFinder):
    """Find in Google Scholar"""

    def findable(self, annotation):
        return annotation.get('concept') in ('Citation', 'ForwardCitation') and ('property:title' in annotation or 'property:displayText' in annotation)

    def findLink(self, annotation):
        if 'property:title' in annotation:
            return [('10Find in Google Scholar', 'http://scholar.google.com/scholar?q=%s' % annotation['property:title'])]
        else:
            displayTextStripped = re.sub(r'<[^>]*>', '', annotation['property:displayText'])
            return [('10Find in Google Scholar', 'http://scholar.google.com/scholar?{0}'.format(urllib.urlencode({'q': displayTextStripped.encode('utf8')})))]


class GooglePhraseLookup(utopia.document.PhraseLookup):
    """8Search Google"""

    def lookup(self, phrase):
        return "http://www.google.com/search?q=%s" % urllib.quote(re.sub(r'\W+', ' ', phrase).strip().encode('utf-8'))


if utopia.bridge is not None:
    class GoogleScholarResolver(utopia.citation.Resolver):
        """Resolve search link for google scholar"""

        def resolve(self, citations, document = None):
            citation = {}
            if not utopia.citation.has_link(citations, {'type': 'search', 'whence': 'google'}):
                #import pprint
                #pprint.PrettyPrinter(indent=2).pprint(citations)
                displayText = utopia.citation.format(utopia.citation.flatten(citations))
                displayTextStripped = re.sub(r'<[^>]*>', '', displayText)
                citation['links'] = [{
                    'url': 'http://scholar.google.com/scholar?{0}'.format(urllib.urlencode({'q': displayTextStripped.encode('utf8')})),
                    'mime': 'text/html',
                    'title': 'Search for article...',
                    'type': 'search',
                    }]
                return citation

        def provenance(self):
            return {'whence': 'google'}

        def purposes(self):
            return 'dereference'

        def weight(self):
            return 100
