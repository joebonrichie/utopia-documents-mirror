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
import urlparse
import utopia.citation

from lxml import etree
from StringIO import StringIO


class IEEEResolver(utopia.citation.Resolver):
    """Resolve PDF link from an IEEE page"""

    def resolve(self, citations, document = None):
        citation = {}
        if not utopia.citation.has_link(citations, {'mime': 'application/pdf'}, {'whence': 'ieee'}):
            resolved_links = utopia.citation.filter_links(citations, {'resolved_url': None})
            for link in resolved_links:
                url = link['resolved_url']
                if 'ieeexplore.ieee.org' in url:
                    parser = etree.HTMLParser()
                    resource = urllib2.urlopen(url, timeout=12)
                    html = resource.read()
                    dom = etree.parse(StringIO(html), parser)

                    # look for the PDF link
                    download_pdf_urls = dom.xpath('//a[@id="full-text-pdf"]/@href')
                    for pdf_url in download_pdf_urls:
                        pdf_url = urlparse.urljoin(url, pdf_url)
                        if pdf_url != resource.geturl(): # Check for cyclic references
                            # follow the link and find the iframe src
                            resource = urllib2.urlopen(pdf_url, timeout=12)
                            html = resource.read()
                            dom = etree.parse(StringIO(html), parser)

                            # developing time-frequency features for prediction
                            download_pdf_urls = dom.xpath("//frame[contains(@src, 'pdf')]/@src")
                            for pdf_url in download_pdf_urls:
                                pdf_url = urlparse.urljoin(url, pdf_url)
                                citation.setdefault('links', [])
                                citation['links'].append({
                                    'url': pdf_url,
                                    'mime': 'application/pdf',
                                    'type': 'article',
                                    'title': 'Download article from IEEEXplore',
                                    })
        return citation

    def provenance(self):
        return {'whence': 'ieee'}

    def purposes(self):
        return 'dereference'

    def weight(self):
        return 103

