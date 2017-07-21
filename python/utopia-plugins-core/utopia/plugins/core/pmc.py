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

import base64
import re
import urllib
import utopia.document
import utopia.tools.eutils
import utopia.tools.nlm
import utopia.tools.utils

from lxml import etree


# Turn a string into a matching regex with a bit of fuzz
def fuzz(input, strict = False):
    if strict:
        ANY = r'.?'
    else:
        ANY = r'.{0,4}'
    return r'\.'.join((re.sub(r'[^\w\d.]', ANY,
                       re.sub(r'(^[a-zA-Z0-9#]*;|&[^;]+;|&[a-zA-Z0-9#]*$)', r'.', token))
                       for token in input.split('.')))


class PubMedCentral(utopia.document.Annotator):
    """PubMedCentral NLM parsing"""

    keys = ('publication-title', 'publisher', 'publication-issn', 'doi', 'pmid', 'pmcid', 'pii', 'authors', 'title',
            'volume', 'issue', 'pages', 'pagefrom', 'pageto', 'abstract', 'keywords', 'year', 'month',
            'abbreviations', 'label', 'url', 'type', 'html', 'displayText')

    @utopia.document.buffer
    def on_ready_event(self, document):
        import spineapi
        info = utopia.tools.nlm.parse(utopia.tools.utils.metadata(document, 'raw_pmc_nlm'))
        if info is not None and len(info) > 0:

            # Enrich citation information with identifiers from PMC
            parser = etree.XMLParser(ns_clean=True, recover=True, remove_blank_text=True, encoding='utf8')
            pmids = dict(((citation['pmid'], citation['id']) for citation in info['citations'] if 'pmid' in citation and 'id' in citation))
            if len(pmids) > 0:
                pubmed_abstracts = etree.fromstring(utopia.tools.eutils.efetch(id=','.join(pmids.keys()), retmode='xml', rettype='abstract'), parser)
                for idList in pubmed_abstracts.xpath('PubmedArticle/PubmedData/ArticleIdList'):
                    #print etree.tostring(idList)
                    pmid = idList.findtext('ArticleId[@IdType="pubmed"]')
                    if pmid in pmids:
                        citation = info['citations_by_id'][pmids[pmid]]
                        for key_name, id_name in (('doi', 'doi'), ('pmcid', 'pmc'), ('pii', 'pii')):
                            id = idList.findtext('ArticleId[@IdType="{0}"]'.format(id_name))
                            if key_name not in citation and id is not None:
                                citation[key_name] = id

            # Create Metadata link annotation
            link = document.newAccList('metadata', 50)
            link['property:sourceDatabase'] = 'pmc'
            link['property:sourceTitle'] = 'PubMed Central'
            link['property:sourceDescription'] = '<p><a href="http://www.ncbi.nlm.nih.gov/pmc/">PubMed Central</a> is the U.S. National Institutes of Health (NIH) digital archive of biomedical and life sciences journal literature.</p>'

            # Create Metadata annotation
            annotation = utopia.tools.utils.citation_to_annotation(info.get('self', {}), 'DocumentMetadata')
            document.addAnnotation(annotation, link['scratch'])

            # Create Bibliography annotations
            for citation in info.get('citations', []):
                annotation = utopia.tools.utils.citation_to_annotation(citation)
                document.addAnnotation(annotation, link['scratch'])

            # Citations
            for citation in info['citations']:
                # Find cross refs
                for pre, label, post in citation.get('contexts', []):
                    matches = document.findInContext(pre, label, post)
                    #print matches
                    if len(matches) > 0:
                        try:
                            annotation = utopia.tools.utils.citation_to_annotation(citation, concept='ForwardCitation')
                            if 'doi' in citation and citation['doi'].startswith('10.1371/'):
                                citation['pdf'] = 'http://www.ploscompbiol.org/article/fetchObjectAttachment.action?uri={0}&representation=PDF'.format('info:doi/{0}'.format(citation['doi']))
                            if 'pmcid' in citation:
                                citation['pdf'] = 'http://www.ncbi.nlm.nih.gov/pmc/articles/{0}/pdf/'.format(citation['pmcid'])
                            for match in matches:
                                annotation.addExtent(match)
                            document.addAnnotation(annotation, link['scratch'])
                        except:
                            raise

            # Tables
            for id, table in info.get('tables', {}).iteritems():
                if 'caption' in table and 'xml' in table:
                    regex = fuzz(table['caption'], strict = True)
                    matches = document.search(regex, spineapi.RegExp + spineapi.IgnoreCase)
                    if len(matches) == 1:
                        annotation = spineapi.Annotation()
                        annotation['concept'] = 'Table'
                        annotation['session:upload_files'] = 'data:application/xml;name=data.xml;base64,%s' % base64.standard_b64encode(table['xml'])
                        annotation.addExtent(matches[0])
                        document.addAnnotation(annotation, link['scratch'])
                    else:
                        print '*********** failed to match table:', id
