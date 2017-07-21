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

import logging
import re
import utopia.citation
import utopia.tools.crossref

try:
    import spineapi
except ImportError:
    logging.debug("No 'spineapi' module found. PDF scraping will be disabled.")
    spineapi = None


class CrossRefIdentifier(utopia.citation.Resolver):
    '''Given a title, try to identify the document.'''

    def resolve(self, citations, document = None):
        citation = {}
        doi = utopia.citation.pick_from(citations, 'identifiers[doi]', default=None, record_in=citation)
        title = utopia.citation.pick_from(citations, 'title', default=None, record_in=citation)
        if doi is not None or title is not None:
            if doi is None:
                xref_results = utopia.tools.crossref.search(title)
                if len(xref_results) == 1:
                    best = xref_results[0]
                    xref_title = best.get('title', '').strip(' .')
                    if len(xref_title) > 0:
                        matched = False
                        if document is not None:
                            # Accept the crossref title if present in the document (do magic dash pattern thing)
                            xref_title = re.sub(ur'[^-\u002D\u007E\u00AD\u058A\u05BE\u1400\u1806\u2010-\u2015\u2053\u207B\u208B\u2212\u2E17\u2E3A\u2E3B\u301C\u3030\u30A0\uFE31\uFE32\uFE58\uFE63\uFF0D]+', lambda x: re.escape(x.group(0)), xref_title)
                            xref_title = re.sub(ur'[\u002D\u007E\u00AD\u058A\u05BE\u1400\u1806\u2010-\u2015\u2053\u207B\u208B\u2212\u2E17\u2E3A\u2E3B\u301C\u3030\u30A0\uFE31\uFE32\uFE58\uFE63\uFF0D-]+', lambda x: r'\p{{Pd}}{{{0}}}'.format(len(x.group(0))), xref_title)
                            matches = document.search(xref_title, spineapi.RegExp + spineapi.IgnoreCase)
                            matched = (len(matches) > 0)
                        else:
                            matched = (xref_title.lower() == title)
                        if matched:
                            citation.update(best)
                            doi = citation.get('identifiers', {}).get('doi')
                            if doi is not None and doi.startswith('http://dx.doi.org/'):
                                doi = doi[18:]
                                citation['identifiers']['doi'] = doi
            if doi is not None:
                if None not in (document, title):
                    # What is this DOI's article's title according to crossref?
                    try:
                        xref_results = utopia.tools.crossref.resolve(doi)
                        xref_title = xref_results.get('title', '')
                        if len(xref_title) > 0:
                            print 'crossref: resolved title:', xref_title.encode('utf8')

                            if re.sub(r'[^\w]+', ' ', title).strip() == re.sub(r'[^\w]+', ' ', xref_title).strip(): # Fuzzy match
                                print 'crossref: titles match precisely'
                                citation.update(xref_results)
                            else:
                                # Accept the crossref title over the scraped title, if present in the document
                                matches = document.findInContext('', xref_title, '') # Fuzzy match
                                if len(matches) > 0:
                                    citation.update(xref_results)
                                    print 'crossref: overriding scraped title with crossref title'
                                else:
                                    print 'crossref: ignoring resolved citations'
                                    # FIXME should we discard the DOI at this point?
                    except Exception as e:
                        import traceback
                        traceback.print_exc()

        return citation

    def provenance(self):
        return {'whence': 'crossref'}

    def purposes(self):
        return 'identify'

    def weight(self):
        return 0


class CrossRefExpander(utopia.citation.Resolver):
    '''From a DOI, expand the citations'''

    def resolve(self, citations, document = None):
        # If a DOI is present, look it up
        citation = {}
        doi = utopia.citation.pick_from(citations, 'identifiers[doi]', default=None, record_in=citation)
        if doi is not None:
            citation.update(utopia.tools.crossref.resolve(doi))
        return citation

    def provenance(self):
        return {'whence': 'crossref'}

    def purposes(self):
        return 'expand'

    def weight(self):
        return 12


class DOIResolver(utopia.citation.Resolver):
    """Resolve a URL from a DOI"""

    def resolve(self, citations, document = None):
        citation = {}
        # If a DOI is present, but no CrossRef link, we can generate one
        doi = utopia.citation.pick_from(citations, 'identifiers[doi]', default=None, record_in=citation)
        if doi is not None and not utopia.citation.has_link(citations, {'type': 'article'}, {'whence': 'crossref'}):
            citation['links'] = [{
                'url': 'http://dx.doi.org/{0}'.format(doi),
                'mime': 'text/html',
                'type': 'article',
                'title': "Show on publisher's website",
            }]
        return citation

    def provenance(self):
        return {'whence': 'crossref'}

    def purposes(self):
        return 'dereference'

    def weight(self):
        return 100

