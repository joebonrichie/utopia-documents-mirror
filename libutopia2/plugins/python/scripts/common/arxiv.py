###############################################################################
#   
#    This file is part of the Utopia Documents application.
#        Copyright (c) 2008-2014 Lost Island Labs
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
import spineapi
import urllib

import urllib2
from lxml import etree


regex = r'arXiv:([\w.-]+/\d{7}|\d{4}\.\d{4,})(v\d+)+'

def url(arxivid):
    return 'http://arxiv.org/abs/{0}'.format(urllib.quote_plus(arxivid))

def scrape(document):
    '''Look for a horizontal arXiv ID on the front page'''
    for match in document.search(regex, spineapi.RegExp):
        if match.begin().lineArea()[1] > 0:
            return match.text()[6:]

def fetch(arxivid):
    url = 'http://export.arxiv.org/api/query?{0}'.format(urllib.urlencode({
        'id_list': arxivid,
        'start': '0',
        'max_results': '1',
    }))
    return urllib2.urlopen(url, timeout=8).read()

def resolve(arxivid):
    data = {}

    namespaces = {
        'atom': 'http://www.w3.org/2005/Atom',
        'arxiv': 'http://arxiv.org/schemas/atom'
    }

    response = fetch(arxivid)
    data['raw_arxiv_atom'] = response
    dom = etree.fromstring(response)

    title = dom.findtext('./atom:entry/atom:title', namespaces=namespaces)
    if title is not None:
        title = re.sub(r'\s+', ' ', title)
        data['title'] = title
        print 'arxiv: resolved title:', title.encode('utf8')

    doi = dom.findtext('./atom:entry/arxiv:doi', namespaces=namespaces)
    if doi is None:
        dois = dom.xpath('./atom:entry/atom:link[@title="doi"]/@href', namespaces=namespaces)
        if len(dois) > 0 and dois[0].startswith('http://dx.doi.org/'):
            doi = dois[0][18:]
    if doi is not None:
        data['doi'] = doi
        print 'arxiv: resolved doi:', doi.encode('utf8')

    authors = []
    for author in dom.xpath('./atom:entry/atom:author', namespaces=namespaces):
        name = author.findtext('atom:name', namespaces=namespaces)
        if name is not None:
            parts = name.split()
            name = parts[-1] + u', ' + u' '.join(parts[:-1])
            authors.append(name)
    if len(authors) > 0:
        data['authors'] = authors

    published = dom.xpath('./atom:entry/atom:published/text()', namespaces=namespaces)
    if len(published) > 0:
        data['year'] = published[0][:4]

    url = dom.xpath('./atom:entry/atom:link[@type="text/html"]/@href', namespaces=namespaces)
    if len(url) > 0 and url[0] is not None:
        data['url'] = url[0]

    pdf = dom.xpath('./atom:entry/atom:link[@type="application/pdf"]/@href', namespaces=namespaces)
    if len(pdf) > 0 and pdf[0] is not None:
        data['pdf'] = pdf[0]

    return data
