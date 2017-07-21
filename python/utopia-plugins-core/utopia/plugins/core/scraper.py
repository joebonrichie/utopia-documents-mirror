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

import utopia.citation
import utopia.tools.arxiv
import utopia.tools.doi
import utopia.tools.title



class ScrapingResolver(utopia.citation.Resolver):
    '''Scrape a title/DOI/ArXiv ID from this document.'''

    def resolve(self, citations, document = None):
        citation = {}
        if document is not None:
            # First try to scrape the title
            if 'title' not in citations:
                title = utopia.tools.title.scrape(document)
                if title is not None:
                    print('scraper: title: ' + (title and title.encode('utf8')))
                    citation['title'] = title
            # Then look for a DOI and ArXiv ID
            ids = utopia.citation.pick_from(citations, 'identifiers', {})
            if 'doi' not in ids:
                doi = utopia.tools.doi.scrape(document)
                if doi is not None:
                    print('scraper: doi:' + (doi and doi.encode('utf8')))
                    citation.setdefault('identifiers', {})
                    citation['identifiers']['doi'] = doi
            if 'arxiv' not in ids:
                arxivid = utopia.tools.arxiv.scrape(document)
                if arxivid is not None:
                    print('scraper: arxivid:' + (arxivid and arxivid.encode('utf8')))
                    citation.setdefault('identifiers', {})
                    citation['identifiers']['arxiv'] = arxivid
        return citation

    def provenance(self):
        return {'whence': 'crackle'}

    def purposes(self):
        return 'identify'

    def weight(self):
        return -1900
