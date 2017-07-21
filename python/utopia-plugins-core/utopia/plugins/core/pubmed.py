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

import urllib
import utopia.document
import utopia.citation
import utopia.tools.pubmed
import utopia.tools.eutils

from lxml import etree



class PubMedAnnotator(utopia.document.Annotator, utopia.document.Visualiser):
    """PubMed Recent Articles Lookup"""

    def on_explore_event(self, phrase, document):
        rt = ''

        searchresult = utopia.tools.eutils.esearch(db='pubmed', retmax=5, term=phrase)

        root = etree.fromstring(searchresult)

        ids = root.findall("./IdList/Id")
        if len(ids) > 0:
            idlist = ','.join((id.text for id in ids))

            searchresult = utopia.tools.eutils.efetch(db='pubmed', id=idlist)
            articleset = etree.fromstring(searchresult)

            for article in articleset.findall('./PubmedArticle'):
                citation = utopia.tools.pubmed.parse_PubmedArticle(article)

                doi = citation['doi'] = citation.get('identifiers', {}).get('doi')
                pmid = citation['pmid'] = citation.get('identifiers', {}).get('pubmed')

                if pmid is not None:
                    link = '<a href="http://www.ncbi.nlm.nih.gov/pubmed/{0}">[Link]</a>'.format(pmid)
                else:
                    link = ''

                rt += u'<div class="box">{0} {1}</div>'.format(utopia.citation.format(citation), link)

            rt += '<p class="right"><a href="http://www.ncbi.nlm.nih.gov/pubmed?term=' + urllib.quote_plus(phrase.encode('utf8')) + '">See more in PubMed...</a></p>'

            import spineapi
            annotation = spineapi.Annotation()
            annotation['concept'] = 'PubMedArticleList'
            annotation['property:name'] = "Articles from PubMed"
            annotation['property:description'] = 'Related recent articles'
            annotation['property:content'] = rt
            annotation['property:sourceDatabase'] = 'pubmed'
            annotation['property:sourceDescription'] = '<p><a href="http://www.ncbi.nlm.nih.gov/pubmed/">PubMed</a> comprises more than 21 million citations for biomedical literature from MEDLINE, life science journals, and online books.</p>'
            annotation['displayRelevance'] = '2000'
            annotation['displayRank'] = '2000'

            return [annotation]

        return []

    def visualisable(self, annotation):
        return annotation.get('concept') == 'PubMedArticleList'

    def visualise(self, annotation):
        return annotation.get('property:content')


class PubMedLinkFinder(utopia.document.LinkFinder):
    """Show in PubMed"""

    def findable(self, annotation):
        #print '======', annotation.get('property:doi', '').encode('utf8'), annotation.get('property:pmid', '').encode('utf8'), annotation.get('property:title', '').encode('utf8')
        return annotation.get('concept') in ('Citation', 'ForwardCitation') and ('property:title' in annotation or ('property:doi' in annotation and 'property:pmid' in annotation))

    def findLink(self, annotation):
        links = []

        doi = annotation.get('property:doi')
        pmid = annotation.get('property:pmid')

        if doi is None and pmid is not None:
            # Resolve DOI from PMID
            pass # FIXME
        elif pmid is None and doi is not None:
            # Resolve PMID from DOI
            pass # FIXME
        elif doi is not None and pmid is not None:
            # Resolve what can be resolved from the title
            pass # FIXME

        if doi is not None:
            links.append(("30Show on publisher's website", 'http://dx.doi.org/{0}'.format(doi)))

        if pmid is not None:
            links.append(('20Show in PubMed', 'http://www.ncbi.nlm.nih.gov/pubmed/{0}'.format(pmid)))

        return links

