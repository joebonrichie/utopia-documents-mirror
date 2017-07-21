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

#? name: Portland Press
#? www: http://www.portlandpress.com/
#? urls: https://utopia.cs.manchester.ac.uk/ext/portland


import base64
import utopia.tools.nlm
import utopia.tools.utils
import re
import spineapi
import urllib
import utopia.document

import urllib2
from lxml import etree


registrants = {
    '10.1042': 'Portland Press',
}


# Turn a string into a matching regex with a bit of fuzz
def fuzz(input, strict = False):
    if strict:
        ANY = r'.?'
    else:
        ANY = r'.{0,4}'
    return r'\.'.join((re.sub(r'[^\w\d.]', ANY,
                       re.sub(r'(^[a-zA-Z0-9#]*;|&[^;]+;|&[a-zA-Z0-9#]*$)', r'.', token))
                       for token in input.split('.')))


class PortlandNLM(utopia.document.Annotator):
    '''Portland NLM annotator'''

    resourceRegExp = re.compile(r'.*bj(\d{3})(\d{4}).*', re.IGNORECASE)

    @utopia.document.buffer
    def on_ready_event(self, document):
        volume, page = None, None

        # Only send if the DOI has a Portland prefix
        doi = utopia.tools.utils.metadata(document, 'identifiers[doi]')
        if doi is not None and doi[:7] in registrants:
            crossref_unixref = utopia.tools.utils.metadata(document, 'raw_crossref_unixref')
            if crossref_unixref is not None:
                # Parse CrossRef redirect URL
                dom = etree.fromstring(crossref_unixref.encode('utf8'))
                resource = dom.findtext('doi_record/crossref/journal/journal_article/doi_data/resource')
                if resource is not None:
                    match = self.resourceRegExp.match(resource)
                    if match is not None:
                        volume, page = match.groups()

                ### FIXME What information should be shown? Portland? BJ?
                #annotation = spineapi.Annotation()
                #annotation['concept'] = 'PublisherIdentity'
                #annotation['property:logo'] = utopia.get_plugin_data_as_url('images/logo.png', 'image/png')
                #annotation['property:title'] = 'Portland Press Limited'
                #annotation['property:webpageUrl'] = 'http://www.portlandpress.com/'
                #document.addAnnotation(annotation, 'PublisherMetadata')

        # If this document was resolved, off we go to fetch the NLM
        if None not in (volume, page):
            # Make a request to the utopia ext web service
            url = 'https://utopia.cs.manchester.ac.uk/ext/portland/nlm?{0}'
            url = url.format(urllib.urlencode({'volume': volume, 'page': page}))
            try:
                nlm = urllib2.urlopen(url, timeout=8).read()
            except:
                raise
                return

            info = utopia.tools.nlm.parse(nlm)
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
                                    #print 'KEY', key_name, id

                # Create Metadata link annotation
                link = document.newAccList('metadata', 100)
                link['property:sourceIcon'] = utopia.get_plugin_data_as_url('images/biochemj.png', 'image/png')
                link['property:sourceTitle'] = 'Portland'
                link['property:sourceDescription'] = '''
                    <p><a href="http://www.portlandpress.com/">Portland Press Limited</a>.</p>
                    '''

                # Create Metadata annotation
                annotation = utopia.tools.utils.citation_to_annotation(info.get('self', {}), 'DocumentMetadata')
                document.addAnnotation(annotation, link['scratch'])

                # Create Bibliography annotations
                for citation in info.get('citations', []):
                    annotation = utopia.tools.utils.citation_to_annotation(citation)
                    document.addAnnotation(annotation, link['scratch'])

                #######################################################################################
                # Apply parsed data to document

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

                for id, table in info.get('tables', {}).iteritems():
                    if 'caption' in table and 'xml' in table:
                        regex = fuzz(table['caption'], strict = True)
                        #print regex
                        matches = document.search(regex, spineapi.RegExp + spineapi.IgnoreCase)
                        if len(matches) == 1:
                            annotation = spineapi.Annotation()
                            annotation['concept'] = 'Table'
                            annotation['session:upload_files'] = 'data:application/xml;name=data.xml;base64,%s' % base64.standard_b64encode(table['xml'])
                            annotation['session:volatile'] = '1'
                            annotation.addExtent(matches[0])
                            document.addAnnotation(annotation, link['scratch'])
                        else:
                            print '*********** failed to match table:', id




class PortlandLegacy(utopia.document.Annotator):
    '''Modify portland legacy annotations to work in the new UI'''

    @utopia.document.buffer
    def on_filter_event(self, document, data = None):
        for a in document.annotations():
            if a.get('author') == 'http://utopia.cs.manchester.ac.uk/users/11679' and a.get('concept') in ('Definition', 'DatabaseEntry') and 'session:legacy' not in a:
                document.removeAnnotation(a)
                identifier = a.get('property:identifier', '')
                if identifier.startswith('http://bio2rdf.org/pdb:'):
                    # PDB entry
                    a2 = spineapi.Annotation()
                    a2['concept'] = 'DatabaseEntry'
                    a2['author'] = a['author']
                    a2['session:volatile'] = '1'
                    a2['session:legacy'] = '1'
                    a2['property:sourceDatabase'] = 'pdb'
                    a2['property:sourceDescription'] = '<p>The <a href="http://www.rcsb.org/">Protein Data Bank</a> of the Research Collaboratory for Structural Bioinformatics (<a href="http://home.rcsb.org/">RCSB</a>).</p>'
                    a2['property:identifier'] = identifier
                    a2['property:description'] = 'PDB entry {0}'.format(identifier[-4:].upper())
                    if 'property:name' in a:
                        a2['property:name'] = a['property:name'][:-11]
                    if 'property:imageUrl' in a:
                        a2['property:imageUrl'] = a['property:imageUrl']
                    if 'property:molecularDescription' in a:
                        a2['property:molecularDescription'] = a['property:molecularDescription']
                    if 'property:webpageUrl' in a:
                        a2['property:webpageUrl'] = a['property:webpageUrl']
                    if 'property:embedded' in a:
                        a2['property:embedded'] = a['property:embedded']
                    for extent in a.extents():
                        a2.addExtent(extent)
                    for area in a.areas():
                        a2.addArea(area)
                    document.addAnnotation(a2)
                if identifier.startswith('http://dbpedia.org/resource/'):
                    # Wikipedia entry
                    a2 = spineapi.Annotation()
                    a2['concept'] = 'Definition'
                    a2['author'] = a['author']
                    a2['session:volatile'] = '1'
                    a2['session:legacy'] = '1'
                    a2['property:sourceDatabase'] = 'wikipedia'
                    a2['property:sourceDescription'] = '<p>Structured <a href="http://www.wikipedia.org/">Wikipedia</a> information provided by the <a href="http://DBpedia.org/">DBpedia</a> project.</p>'
                    a2['property:description'] = a.get('property:summary', 'Wikipedia entry')
                    if 'property:name' in a:
                        a2['property:name'] = a['property:name']
                    if 'property:identifier' in a:
                        a2['property:identifier'] = a['property:identifier']
                    if 'property:imageUrl' in a:
                        a2['property:imageUrl'] = a['property:imageUrl']
                    if 'property:summary' in a:
                        a2['property:summary'] = a['property:summary']
                    if 'property:webpageUrl' in a:
                        a2['property:webpageUrl'] = a['property:webpageUrl']
                    for extent in a.extents():
                        a2.addExtent(extent)
                    for area in a.areas():
                        a2.addArea(area)
                    document.addAnnotation(a2)
                if identifier.startswith('http://www.portlandpress.com/utopia/glick/'):
                    # Wikipedia entry
                    a2 = spineapi.Annotation()
                    a2['concept'] = 'Definition'
                    a2['author'] = a['author']
                    a2['session:volatile'] = '1'
                    a2['session:legacy'] = '1'
                    a2['property:sourceDatabase'] = 'glick'
                    a2['property:sourceDescription'] = '<p>David M. Glick\'s <a href="http://www.portlandpress.com/pp/books/online/glick/search.htm">Glossary of Biochemistry and Molecular Biology</a>.</p><p>Made available by <a href="http://www.portlandpress.com/">Portland Press Limited</a>.</p>'
                    a2['property:description'] = a['property:description'] + '<p><em>(Glick Glossary/Portland Press Ltd.)</em></p>'
                    a2['property:name'] = a['property:name']
                    for extent in a.extents():
                        a2.addExtent(extent)
                    for area in a.areas():
                        a2.addArea(area)
                    document.addAnnotation(a2)
