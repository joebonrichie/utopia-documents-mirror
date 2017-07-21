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
import utopia.tools.pmc



class PMCResolver(utopia.citation.Resolver):
    """Resolve PDF link from a PMC ID"""

    def resolve(self, citations, document = None):
        citation = {}
        if not utopia.citation.has_link(citations, {'mime': 'application/pdf'}, {'whence': 'pmc'}):
            # Try to resolve the PMC ID from either the DOI or the PubMed ID
            pmcid = utopia.citation.pick_from(citations, 'identifiers/pmc', default=None)
            if pmcid is None:
                doi = utopia.citation.pick_from(citations, 'identifiers/doi', default=None, record_in=citation)
                pmid = utopia.citation.pick_from(citations, 'identifiers/pubmed', default=None, record_in=citation)
                if doi is not None and pmcid is None:
                    pmcid = utopia.tools.pmc.identify(doi, 'doi')
                if pmid is not None and pmcid is None:
                    pmcid = utopia.tools.pmc.identify(pmid, 'pmid')

            # Generate PMC link to PDF
            if pmcid is not None:
                pdf_url = 'http://www.ncbi.nlm.nih.gov/pmc/articles/{0}/pdf/'.format(pmcid)
                citation.update({
                    'links': [{
                        'url': pdf_url,
                        'mime': 'application/pdf',
                        'type': 'article',
                        'title': 'Download article from PubMed Central',
                    }],
                    'identifiers': {'pmc': pmcid}
                })
                return citation

    def provenance(self):
        return {'whence': 'pmc'}

    def purposes(self):
        return 'dereference'

    def weight(self):
        return 104


