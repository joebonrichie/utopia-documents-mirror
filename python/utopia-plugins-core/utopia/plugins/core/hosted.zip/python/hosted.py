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

#? name: Utopia hosted metadata
#? www: http://utopiadocs.com/
#? urls: https://utopia.cs.manchester.ac.uk/ext/hosted/


import base64
import utopia.tools.eutils
import utopia.tools.nlm
import utopia.tools.utils
import re
import socket
import spineapi
import urllib
import utopia.document

import urllib2
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

class HostedNLM(utopia.document.Annotator):
    """Hosted NLM parsing"""

    @utopia.document.buffer
    def on_ready_event(self, document):
        # See if we have any publishers' NLM hosted for this DOI
        doi = utopia.tools.utils.metadata(document, 'identifiers[doi]')
        #print '----- DOI', doi
        if doi is not None:
            info = None
            try:
                url = 'https://utopia.cs.manchester.ac.uk/ext/hosted/nlm?'
                url += urllib.urlencode({'doi': doi.encode('utf8')})
                nlm = urllib2.urlopen(url, timeout=8).read()
                info = utopia.tools.nlm.parse(nlm)
            except (urllib2.URLError, socket.timeout):
                # info will remain None
                pass

            #print info
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

                # Generate sensible titles / descriptions / icons?
                journalTitle = info.get('publication-title', '')
                journalTitleSuffix = ''
                publisher = info.get('publisher', 'the publisher')
                if len(journalTitle) > 0:
                    journalTitleSuffix = ' ({0})'.format(journalTitle)

                # Create Metadata link annotation
                link = document.newAccList('metadata', 100)
                link['property:sourceIcon'] = ''
                link['property:sourceTitle'] = publisher
                link['property:sourceDescription'] = '''
                    <p>This information was provided by {0}{1}.</p>
                    '''.format(publisher, journalTitleSuffix)

                # Publisher identity
                if doi[:8] in ('10.1104/', '10.1105/'):
                    annotation = spineapi.Annotation()
                    annotation['concept'] = 'PublisherIdentity'
                    logo = utopia.get_plugin_data_as_url('images/aspb_logo.png', 'image/png')
                    webpageUrl = 'http://www.aspb.org/'
                    title = publisher
                    #print '====', publisher, '---', journalTitle, '---', webpageUrl
                    if doi.startswith('10.1104/'):
                        logo = utopia.get_plugin_data_as_url('images/pp_logo.png', 'image/png')
                        title = journalTitle
                        webpageUrl = 'http://www.plantphysiol.org/'
                    elif doi.startswith('10.1105/'):
                        logo = utopia.get_plugin_data_as_url('images/tpc_logo.png', 'image/png')
                        title = journalTitle
                        webpageUrl = 'http://www.plantcell.org/'

                    annotation['property:logo'] = logo
                    annotation['property:title'] = title
                    annotation['property:webpageUrl'] = webpageUrl
                    document.addAnnotation(annotation, 'PublisherMetadata')

                    link['property:sourceIcon'] = logo
                    link['property:sourceTitle'] = title

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

                        # convert oasis tables
                        ns = {'oasis': 'http://docs.oasis-open.org/ns/oasis-exchange/table'}
                        xml = etree.fromstring(table['xml'])
                        if xml.tag == '{{{0}}}table'.format(ns['oasis']):
                            for tgroup in xml.xpath('//oasis:tgroup', namespaces=ns):
                                columns = {}
                                for colspec in tgroup.xpath('./oasis:colspec', namespaces=ns):
                                    columns[colspec.get('colname')] = int(colspec.get('colnum'))
                                for section in tgroup.xpath('./oasis:thead|./oasis:tbody', namespaces=ns):
                                    isHead = (section.tag == '{{{0}}}thead'.format(ns['oasis']))
                                    for row in section.xpath('./oasis:row', namespaces=ns):
                                        for entry in row.xpath('./oasis:entry', namespaces=ns):
                                            colname = entry.get('colname')
                                            colst = entry.get('namest')
                                            colend = entry.get('nameend')
                                            if colst is not None and colend is not None:
                                                colspan = columns[colend] - columns[colst] + 1
                                            else:
                                                colspan = 1
                                            if colspan > 1:
                                                entry.set('colspan', unicode(colspan))
                                            morerows = entry.get('morerows')
                                            if morerows is not None:
                                                rowspan = int(morerows) + 1
                                            else:
                                                rowspan = 1
                                            if rowspan > 1:
                                                entry.set('rowspan', unicode(rowspan))
                                            entry.tag = 'td'
                                        row.tag = 'tr'
                                    if isHead:
                                        section.tag = 'thead'
                                    else:
                                        section.tag = 'tbody'
                                    xml.append(section)
                                xml.tag = 'table'
                                #print etree.tostring(xml, pretty_print=True, encoding='utf8')
                                table['xml'] = etree.tostring(xml, encoding='utf8')

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

