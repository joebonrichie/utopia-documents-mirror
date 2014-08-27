###############################################################################
#   
#    This file is part of the Utopia Documents application.
#        Copyright (c) 2008-2014 Lost Island Labs
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

import common.pmc
import common.utils
import urllib2
import urlparse
import utopia.library

from lxml import etree
from StringIO import StringIO

class DOIResolver(utopia.library.Resolver):
    """Resolve a URL from a DOI"""

    def resolve(self, metadata):
        if 'url' not in metadata:
            ids = metadata.get('identifiers', {})
            if 'doi' in ids:
                return {'url': 'http://dx.doi.org/{0}'.format(ids['doi'])}

    def weight(self):
        return 10


class PubMedResolver(utopia.library.Resolver):
    """Resolve a URL from a PubMed ID"""

    def resolve(self, metadata):
        if 'url' not in metadata:
            ids = metadata.get('identifiers', {})
            if 'pubmed' in ids:
                return {'url': 'http://www.ncbi.nlm.nih.gov/pubmed/{0}'.format(ids['pubmed'])}

    def weight(self):
        return 11


class HTMLResolver(utopia.library.Resolver):
    """Resolve PDF link from an article's web page"""

    def resolve(self, metadata):
        update = {}
        if 'pdf' not in metadata and 'url' in metadata:
            parser = etree.HTMLParser()
            url = metadata['url']
            resource = urllib2.urlopen(url, timeout=8)
            update['redirected_url'] = resource.geturl()
            html = resource.read()
            dom = etree.parse(StringIO(html), parser)

            # look for the PDF link
            citation_pdf_urls = dom.xpath('/html/head/meta[@name="citation_pdf_url"]/@content')
            if len(citation_pdf_urls) > 0:
                pdf_url = citation_pdf_urls[0]
                if pdf_url != resource.geturl():
                    update['pdf'] = pdf_url
        return update

    def weight(self):
        return 100


class NatureResolver(utopia.library.Resolver):
    """Resolve PDF link from a Nature page"""

    def resolve(self, metadata):
        url = metadata.get('redirected_url', '')
        if 'www.nature.com' in url:
            parser = etree.HTMLParser()
            html = urllib2.urlopen(url, timeout=8).read()
            dom = etree.parse(StringIO(html), parser)

            # look for the PDF link
            download_pdf_urls = dom.xpath('//li[@class="download-pdf"]/a/@href')
            if len(download_pdf_urls) > 0:
                return {'pdf': urlparse.urljoin(url, download_pdf_urls[0])}

    def weight(self):
        return 101


class WileyResolver(utopia.library.Resolver):
    """Resolve PDF link from a Wiley page"""

    def resolve(self, metadata):
        pdf = metadata.get('pdf', '')
        if 'onlinelibrary.wiley.com' in pdf:
            parser = etree.HTMLParser()
            html = urllib2.urlopen(pdf, timeout=8).read()
            dom = etree.parse(StringIO(html), parser)

            # look for the PDF link
            download_pdf_urls = dom.xpath('//iframe[@id="pdfDocument"]/@src')
            if len(download_pdf_urls) > 0:
                return {'pdf': urlparse.urljoin(pdf, download_pdf_urls[0])}

    def weight(self):
        return 101

class ScienceDirectResolver(utopia.library.Resolver):
    """Resolve PDF link from a Science Direct page"""

    def resolve(self, metadata):
        url = metadata.get('redirected_url', '')
        if 'www.sciencedirect.com' in url:
            parser = etree.HTMLParser()
            html = urllib2.urlopen(url, timeout=8).read()
            dom = etree.parse(StringIO(html), parser)

            # look for the PDF link
            download_pdf_urls = dom.xpath('//a[@id="pdfLink"][@pdfurl]/@href')
            if len(download_pdf_urls) > 0:
                return {'pdf': urlparse.urljoin(url, download_pdf_urls[0])}

    def weight(self):
        return 101

class ACSResolver(utopia.library.Resolver):
    """Resolve PDF link from an ACS page"""

    def resolve(self, metadata):
        url = metadata.get('redirected_url', '')
        if 'pubs.acs.org' in url:
            parser = etree.HTMLParser()
            html = urllib2.urlopen(url, timeout=8).read()
            dom = etree.parse(StringIO(html), parser)

            # look for the PDF link
            download_pdf_urls = dom.xpath('//div[@class="bottomViewLinks"]/a[text()="PDF"]/@href')
            if len(download_pdf_urls) > 0:
                return {'pdf': urlparse.urljoin(url, download_pdf_urls[0])}

    def weight(self):
        return 101

class PMCResolver(utopia.library.Resolver):
    """Resolve PDF link from a PMC ID"""

    def resolve(self, metadata):
        if 'pdf' not in metadata:
            ids = metadata.get('identifiers', {})
            doi = ids.get('doi')
            pmid = ids.get('pmid')
            pmcid = ids.get('pmcid')
            # Try to resolve the PMC ID
            if doi is not None and pmcid is None:
                pmcid = common.pmc.resolve(doi, 'doi')
            if pmid is not None and pmcid is None:
                pmcid = common.pmc.resolve(pmid, 'pmid')
            if pmcid is not None:
                ids.setdefault('pmcid', pmcid)
                return {
                    'pdf': 'http://www.ncbi.nlm.nih.gov/pmc/articles/{0}/pdf/'.format(pmcid),
                    'identifiers': ids,
                }

    def weight(self):
        return 102


class LoggingResolver(utopia.library.Resolver):
    """Log metadata"""

    def resolve(self, metadata):
        for k, v in metadata.iteritems():
            print repr(k), ':', repr(v)

    def weight(self):
        return 10000
