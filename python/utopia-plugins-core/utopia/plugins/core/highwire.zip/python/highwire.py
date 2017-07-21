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

#? name: Highwire metadata
#? www: http://highwire.stanford.edu/
#? urls: http://dx.doi.org/


import base64
import utopia.tools.eutils
import utopia.tools.nlm
import utopia.tools.utils
import re
import spineapi
import urllib
import urlparse
import utopia.document

import urllib2
from lxml import etree


namespaces = {'xlink':'http://www.w3.org/1999/xlink'}

# Query an element according to a dict of xpath queries
def query(element, queries):
    matches = {}
    for key, query in queries.iteritems():
        is_text = query.endswith('/text()')
        if is_text:
            query = query[:len(query)-7]
        results = element.xpath(query, namespaces=namespaces)
        if len(results) > 0:
            matches[key] = []
            if is_text:
                results = [etree.tostring(results[0], method="text", encoding=unicode)]
            for result in results:
                if isinstance(result, str):
                    result = result.decode('utf8')
                if isinstance(result, unicode):
                    result = re.sub(r'\s+', ' ', result.strip())
                if len(results) > 1:
                    matches[key].append(result)
                else:
                    matches[key] = result
    return matches

# Turn a string into a matching regex with a bit of fuzz
def fuzz(input, strict = False):
    if strict:
        ANY = r'.?'
    else:
        ANY = r'.{0,4}'
    return r'\.'.join((re.sub(r'[^\w\d.]', ANY,
                       re.sub(r'(^[a-zA-Z0-9#]*;|&[^;]+;|&[a-zA-Z0-9#]*$)', r'.', token))
                       for token in input.split('.')))

class HighwireXHTML(utopia.document.Annotator):
    """Highwire-hosted XHTML parsing"""

    @utopia.document.buffer
    def on_ready_event(self, document):

        doi = utopia.tools.utils.metadata(document, 'identifiers[doi]')
        if doi is not None:
            info = {}

            # Resolve the DOI to find the publisher's website
            response = urllib2.urlopen('http://dx.doi.org/{0}'.format(doi), timeout=8)

            # Parse page to find (if there) the full text URL
            parser = etree.HTMLParser()
            html = etree.parse(response, parser)

            # Only continue if this is a highwire HTML page
            if len(html.xpath("/html/head/meta[@name='HW.identifier']")) == 0:
                return

            # Now make sure we have the full text XHTML
            citation_fulltext_html_url = html.xpath("/html/head/meta[@name='citation_fulltext_html_url']/@content")
            if len(citation_fulltext_html_url) > 0:
                citation_fulltext_html_url = citation_fulltext_html_url[0]

                # Fetch that full text page (if different to the current one)
                if citation_fulltext_html_url != response.geturl():
                    response = urllib2.urlopen(citation_fulltext_html_url, timeout=8)
                    html = etree.parse(response, parser)

                #print etree.tostring(html, pretty_print=True, encoding='utf8')

                # Now parse out the bibliography
                info['citations'] = []
                info['citations_by_id'] = {}

                for bibitem in html.xpath("//div[contains(concat(' ', normalize-space(@class), ' '), ' ref-list ')]//ol[contains(concat(' ', normalize-space(@class), ' '), ' cit-list ')]/li"):
                    citation = query(bibitem, {
                        'id': 'a/@id',
                        'label': ".//*[contains(concat(' ', normalize-space(@class), ' '), ' ref-label ')]/text()",
                        'title': ".//*[contains(concat(' ', normalize-space(@class), ' '), ' cit-article-title ')]/text()",
                        'year': ".//*[contains(concat(' ', normalize-space(@class), ' '), ' cit-pub-date ')]/text()",
                        'publication-title': ".//*[contains(concat(' ', normalize-space(@class), ' '), ' cit-jnl-abbrev ')]/text()",
                        'volume': ".//*[contains(concat(' ', normalize-space(@class), ' '), ' cit-vol ')]/text()",
                        'issue': ".//*[contains(concat(' ', normalize-space(@class), ' '), ' cit-issue ')]/text()",
                        'pagefrom': ".//*[contains(concat(' ', normalize-space(@class), ' '), ' cit-fpage ')]/text()",
                        'pageto': ".//*[contains(concat(' ', normalize-space(@class), ' '), ' cit-lpage ')]/text()",
                        'pmid': ".//*[contains(concat(' ', normalize-space(@class), ' '), ' cit-pub-id-pmid ')]/text()",
                        'doi': ".//*[contains(concat(' ', normalize-space(@class), ' '), ' cit-pub-id-doi ')]/text()",
                        'etree': ".//*[contains(concat(' ', normalize-space(@class), ' '), ' cit-metadata ')]",
                    })
                    authors = []
                    for a in bibitem.xpath(".//*[contains(concat(' ', normalize-space(@class), ' '), ' cit-auth ')]"):
                        surname = a.xpath(".//*[contains(concat(' ', normalize-space(@class), ' '), ' cit-name-surname ')]/text()")
                        given_names = a.xpath(".//*[contains(concat(' ', normalize-space(@class), ' '), ' cit-name-given-names ')]/text()")
                        if len(surname) > 0 and len(given_names) > 0:
                            authors.append(u'{0}, {1}'.format(surname[0], given_names[0]).strip(', '))
                    if len(authors) > 0:
                        citation['authors'] = authors
                    citation['contexts'] = []
                    citation['displayText'] = utopia.citation.format(citation)

                    info['citations'].append(citation)
                    info['citations_by_id'][citation['id']] = citation
                    #print citation


                #######################################################################################
                # Parse in-text citations if present

                min_length = 10
                max_length = 20
                for paragraph in html.xpath("//div[contains(concat(' ', normalize-space(@class), ' '), ' article ')]//p"):
                    text_stack = [paragraph.text or '']
                    xref_stack = [None]
                    for elem in paragraph:
                        if len(elem.xpath('self::a[@class="xref-bibr"]')) > 0:
                            text_stack.append(etree.tostring(elem, method='text', encoding=unicode, with_tail=False))
                            text_stack.append(elem.tail or '')
                            xref = info['citations_by_id'].get(elem.get('href', '')[1:])
                            if xref is not None:
                                xref_stack += [[xref], None]
                            else:
                                xref_stack += [[], None]
                        elif isinstance(elem, etree._Entity):
                            points = entities.get(elem.text[1:-1])
                            if points is not None:
                                text_stack[-1] += ''.join((unichr(p) for p in points))
                            else:
                                text_stack[-1] += etree.tostring(elem, encoding=unicode)
                        else:
                            if elem.get('position') == 'float':
                                text_stack[-1] += elem.tail or ''
                            else:
                                text_stack[-1] += etree.tostring(elem, method='text', encoding=unicode)
                    # Find and collapse ranges in the text
                    for i in xrange(len(xref_stack) - 3, 1, -2):
                        text = text_stack[i].strip()
                        #print len(text), (text in u'-\u2010\u2011\u2012\u2013\u2014\u2015'), u''.join(text_stack[i-1:i+2]).encode('utf8')
                        # if this text is a dash, we need to coalesce the text fragments
                        if len(text) == 1 and text in u'-\u2010\u2011\u2012\u2013\u2014\u2015':
                            text_stack[i-1:i+2] = [u''.join(text_stack[i-1:i+2])]
                            xref_stack[i-1:i+2] = [xref_stack[i-1] + xref_stack[i+1]]
                    #for text in text_stack:
                    #    print text.encode('utf8')
                    # Then make sure we resolve the implied citations
                    for i in xrange(1, len(xref_stack), 2):
                        # Get actual cross references
                        xrefs = xref_stack[i]

                        # Expand cross references
                        try:
                            if len(xrefs) == 2:
                                labelfrom = int(xrefs[0].get('label'))
                                labelto = int(xrefs[1].get('label'))
                                candidates = {}
                                midlabels = [unicode(midlabel) for midlabel in xrange(labelfrom+1, labelto)]
                                for candidate in info['citations']:
                                    if candidate.get('label') in midlabels:
                                        candidates[int(candidate.get('label'))] = candidate
                                xrefs[1:-1] = candidates.values()
                        except:
                            raise
                    # Find and collapse lists in the text
                    for i in xrange(len(xref_stack) - 3, 1, -2):
                        text = text_stack[i].strip()
                        # if this text is a comma, we need to coalesce the text fragments
                        if len(text) == 1 and text == ',':
                            text_stack[i-1:i+2] = [u''.join(text_stack[i-1:i+2])]
                            xref_stack[i-1:i+2] = [xref_stack[i-1] + xref_stack[i+1]]
                    # Expand citations to include brackets (on both sides)
                    for i in xrange(len(xref_stack) - 2, 0, -2):
                        before = text_stack[i-1].strip()[-1:]
                        text = text_stack[i].strip()
                        after = text_stack[i+1].strip()[:1]
                        # if this text is a comma, we need to coalesce the text fragments
                        #print before.encode('utf'), after.encode('utf')
                        if len(before) > 0 and before in '({[' and len(after) > 0 and after in ')}]':
                            text_stack[i-1] = re.sub(r'[({[](\s*)$', r'\1', text_stack[i-1])
                            text_stack[i+1] = re.sub(r'^(\s*)[)}\]]', r'\1', text_stack[i+1])
                            text_stack[i] = before + text_stack[i] + after
                    #print repr(text_stack)
                    for i in xrange(1, len(xref_stack), 2):
                        # Get context
                        before = u' '.join(text_stack[:i]).strip()
                        label = text_stack[i].strip()
                        after = u' '.join(text_stack[i+1:]).strip()
                        # Strip out extraneous brackets
                        if len(xref_stack[i]) > 1: # Hack to differentiate single / multiple citations
                                                   # as multiple numbers tend not to have spaces between them
                            label = re.sub(ur'[)}\]]?([-\u2010\u2011\u2012\u2013\u2014\u2015,])\s?[({[]?', r'\1', label)
                        else:
                            label = re.sub(ur'[)}\]]?([-\u2010\u2011\u2012\u2013\u2014\u2015,]\s?)[({[]?', r'\1', label)
                        # Normalise context
                        before = re.sub(r'\s+', ' ', before)[-max_length:].strip()
                        label = re.sub(r'\s+', ' ', label)
                        after = re.sub(r'\s+', ' ', after)[:max_length].strip()
                        #print (before.encode('utf8'), label.encode('utf8'), after.encode('utf8'))
                        if len(before + after) > min_length:
                            for xref in xref_stack[i]:
                                xref['contexts'].append((before, label, after))
                        #print xref_stack[i]

                #######################################################################################
                # Parse tables if present

                info['tables'] = {}
                for table_url in html.xpath("//div[contains(concat(' ', normalize-space(@class), ' '), ' table-inline ')]/div[contains(concat(' ', normalize-space(@class), ' '), ' callout ')]//li[1]/a/@href"):
                    table_url = urlparse.urljoin(citation_fulltext_html_url, table_url)
                    #print table_url
                    response = urllib2.urlopen(table_url, timeout=8)
                    table_html = etree.parse(response, parser)
                    for table_expansion in table_html.xpath("//div[contains(concat(' ', normalize-space(@class), ' '), ' table-expansion ')]"):
                        id = table_expansion.get('id')
                        table = {}
                        table['xml'] = table_expansion.xpath('.//table[1]')[0]
                        table['caption_raw'] = table_expansion.xpath(".//span[contains(concat(' ', normalize-space(@class), ' '), ' caption-title ')][1]")[0]
                        if 'caption' not in table and 'caption_raw' in table:
                            table['caption'] = table['caption_raw']
                        if 'caption' in table:
                            table['caption'] = re.sub(r'\s+', ' ', etree.tostring(table['caption'], method='text', encoding=unicode).strip())
                        if 'xml' in table: table['xml'] = etree.tostring(table['xml'], encoding='utf8')
                        info['tables'][id] = table

                        #print table









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
                link = document.newAccList('metadata', 90)
                link['property:sourceIcon'] = ''
                link['property:sourceTitle'] = publisher
                link['property:sourceDescription'] = '''
                    <p>This information was provided by {0}{1}.</p>
                    '''.format(publisher, journalTitleSuffix)

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
                        #print (pre, label, post)
                        matches = document.findInContext(pre, label, post)
                        #print matches
                        if len(matches) > 0:
                            try:
                                annotation = spineapi.Annotation()
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

