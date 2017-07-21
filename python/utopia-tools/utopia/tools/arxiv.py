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

#? name: arXiv
#? www: http://arxiv.org/
#? urls: http://arxiv.org/ http://export.arxiv.org/


import re
import socket
import urllib
import urllib2
from lxml import etree
from utopia.log import logger



try:
    import spineapi
    def scrape(document):
        '''Look for a non-horizontal arXiv ID on the front page (the standard
           layout of ArXiv articles has the ID printed up the left-hand side.'''
        regex = r'arXiv:([\w.-]+/\d{7}|\d{4}\.\d{4,})(v\d+)+'
        for match in document.search(regex, spineapi.RegExp):
            if match.begin().lineArea()[1] > 0:
                return match.text()[6:]
except ImportError:
    logger.debug('spineapi not imported: document scraping of ArXiv IDs will be unavailable')



def url(arxivid):
    '''Given an ArXiv ID, turn it into a link.'''
    return 'http://arxiv.org/abs/{0}'.format(urllib.quote_plus(arxivid))

def fetchXML(arxivid):
    '''Given an ArXiv ID, find its representative XML document.'''
    params = urllib.urlencode({'id_list': arxivid, 'start': '0', 'max_results': '1'})
    url = 'http://export.arxiv.org/api/query?{0}'.format(params)
    return urllib2.urlopen(url, timeout=8).read()

def parseXML(xml):
    '''Parse an ArXiv XML document and return a citation dictionary.'''
    dom = etree.fromstring(xml)

    namespaces = {
        'atom': 'http://www.w3.org/2005/Atom',
        'arxiv': 'http://arxiv.org/schemas/atom'
    }

    data = {}
    identifiers = {}
    links = []

    title = dom.find('./atom:entry/atom:title', namespaces=namespaces)
    if title is not None:
        title = re.sub(r'\s+', ' ', etree.tostring(title, method="text", encoding=unicode, with_tail=False))
        data['title'] = title

    doi = dom.findtext('./atom:entry/arxiv:doi', namespaces=namespaces)
    if doi is None:
        dois = dom.xpath('./atom:entry/atom:link[@title="doi"]/@href', namespaces=namespaces)
        if len(dois) > 0 and dois[0].startswith('http://dx.doi.org/'):
            doi = dois[0][18:]
    if doi is not None:
        identifiers['doi'] = doi

    authors = []
    for author in dom.xpath('./atom:entry/atom:author', namespaces=namespaces):
        name = author.find('atom:name', namespaces=namespaces)
        if name is not None:
            name = etree.tostring(name, method="text", encoding=unicode, with_tail=False)
            parts = name.split()
            name = parts[-1] + u', ' + u' '.join(parts[:-1])
            authors.append(name)
    if len(authors) > 0:
        data['authors'] = authors

    summary = dom.xpath('./atom:entry/atom:summary/text()', namespaces=namespaces)
    if len(summary) > 0:
        data['abstract'] = summary[0].strip()

    published = dom.xpath('./atom:entry/atom:published/text()', namespaces=namespaces)
    if len(published) > 0:
        data['year'] = published[0][:4]

    url = dom.xpath('./atom:entry/atom:link[@type="text/html"]/@href', namespaces=namespaces)
    if len(url) > 0 and url[0] is not None:
        links.append({
            'url': url[0],
            'mime': 'text/html',
            'type': 'article',
            'title': "Show on ArXiv",
            })

    pdf = dom.xpath('./atom:entry/atom:link[@type="application/pdf"]/@href', namespaces=namespaces)
    if len(pdf) > 0 and pdf[0] is not None:
        links.append({
            'url': pdf[0],
            'mime': 'application/pdf',
            'type': 'article',
            'title': 'Download article from ArXiv',
            })

    if len(identifiers) > 0:
        data['identifiers'] = identifiers
    if len(links) > 0:
        data['links'] = links

    return data

def resolve(arxivid):
    '''Given an ArXiv ID, resolve it into a citation dictionary.'''
    xml = fetchXML(arxivid)
    citation = parseXML(xml)
    citation['raw_arxiv_atom'] = xml
    return citation
