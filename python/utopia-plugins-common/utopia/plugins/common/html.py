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

import urllib2
import utopia.citation

from lxml import etree
from StringIO import StringIO


class HTMLResolver(utopia.citation.Resolver):
    """Resolve PDF link from an article's web page"""

    def resolve(self, citations, document = None):
        citation = {}
        if not utopia.citation.has_link(citations, {'mime': 'application/pdf'}, {'whence': 'html'}):
            article_links = utopia.citation.filter_links(citations, {'type': 'article', 'mime': 'text/html'})
            for article_link in article_links:
                url = article_link['url']
                parser = etree.HTMLParser()
                try:
                    request = urllib2.Request(url, headers={'Accept-Content': 'gzip'})
                    resource = urllib2.urlopen(request, timeout=12)
                except urllib2.HTTPError as e:
                    if e.getcode() == 401:
                        resource = e
                    else:
                        raise

                html = resource.read()
                article_link['resolved_url'] = resource.geturl() # FIXME should modification of previous citations be allowed?
                dom = etree.parse(StringIO(html), parser)

                # look for the PDF link
                citations_pdf_urls = dom.xpath('/html/head/meta[@name="citations_pdf_url"]/@content')
                for pdf_url in citations_pdf_urls:
                    if pdf_url != resource.geturl(): # Check for cyclic references
                        citation.setdefault('links', [])
                        citation['links'].append({
                            'url': pdf_url,
                            'mime': 'application/pdf',
                            'type': 'article',
                            'title': 'Download article',
                            })
        return citation

    def provenance(self):
        return {'whence': 'html'}

    def purposes(self):
        return 'dereference'

    def weight(self):
        return 102


