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

import utopialib.arxiv
import utopialib.crossref
import utopialib.pmc
import utopialib.utils
import utopialib.title
import json
import kend
import re
import urllib2
import urlparse
import utopia.library

from lxml import etree
from StringIO import StringIO




def filterLinks(links, criteria):
    filtered = []
    for link in links:
        match = True
        for key, value in criteria.iteritems():
            if key not in link or (value is not None and link[key] != value):
                match = False
                break
        if match:
            filtered.append(link)
    filtered.sort(key=lambda link: link.get(':weight'), reverse=True)
    return filtered

def hasLink(metadata, criteria):
    links = metadata.get('links', [])
    for link in links:
        found = True
        for key, value in criteria.iteritems():
            if link.get(key) != value:
                found = False
                break
        if found:
            return True
    return False




###############################################################################
### Given general information, try to find as many identifiers as possible.
###############################################################################

class DocumentIdentifierResolver(utopia.library.Resolver):
    '''Resolve a Utopia URI for this document.'''

    def unidentifiedDocumentRef(self, document):
        '''Compile a document reference from a document's fingerprints'''
        evidence = [kend.model.Evidence(type='fingerprint', data=f, srctype='document') for f in document.fingerprints()]
        return kend.model.DocumentReference(evidence=evidence)

    def identifyDocumentRef(self, documentref):
        '''Find a URI from a document reference, resolving it if necessary'''
        id = getattr(documentref, 'id', None)
        if id is None:
            documentref = kend.client.Client().documents(documentref)
            id = getattr(documentref, 'id', None)
        return id

    def resolveDocumentId(self, document):
        documentref = self.unidentifiedDocumentRef(document)
        return self.identifyDocumentRef(documentref)

    def resolve(self, metadata, document = None):
        update = {}
        ids = metadata.get('identifiers', {})
        if 'utopia' not in ids and document is not None:
            # Resolve this document's URI and store it in the document
            id = self.resolveDocumentId(document)
            ids['utopia'] = id
            update['identifiers'] = ids
            utopialib.utils.store_metadata(document, identifiers={'utopia': id})

        return update

    def purposes(self):
        return 'identify'

    def weight(self):
        return -1080


class TitleScrapingResolver(utopia.library.Resolver):
    '''Scrape a title from this document.'''

    def resolve(self, metadata, document = None):
        update = {}
        if document is not None:
            ids = metadata.get('identifiers', {})
            if 'title' not in metadata and len(ids) == 0:
                title = utopialib.title.scrape(document)
                if title is not None:
                    print('scraper: title: ' + (title and title.encode('utf8')))
                    update['title'] = title
        return update

    def purposes(self):
        return 'identify'

    def weight(self):
        return -1099


class DOIScrapingResolver(utopia.library.Resolver):
    '''Scrape a DOI from this document.'''

    def resolve(self, metadata, document = None):
        update = {}
        if document is not None:
            ids = metadata.get('identifiers', {})
            if 'doi' not in ids and len(ids) == 0:
                doi = utopialib.doi.scrape(document)
                if doi is not None:
                    print('scraper: doi:' + (doi and doi.encode('utf8')))
                    ids['doi'] = doi
                    update['identifiers'] = ids
        return update

    def purposes(self):
        return 'identify'

    def weight(self):
        return -1098


class ArXivScrapingResolver(utopia.library.Resolver):
    '''Scrape an ArXiv ID from this document.'''

    def resolve(self, metadata, document = None):
        update = {}
        if document is not None:
            ids = metadata.get('identifiers', {})
            if 'arxiv' not in ids:
                arxivid = utopialib.arxiv.scrape(document)
                if arxivid is not None:
                    print('scraper: arxivid:', (arxivid and arxivid.encode('utf8')))
                    ids['arxiv'] = arxivid
                    update['identifiers'] = ids
        return update

    def purposes(self):
        return 'identify'

    def weight(self):
        return -1097


class CrossRefIdentifier(utopia.library.Resolver):
    '''Given a title, try to identify the document.'''

    def resolve(self, metadata, document = None):
        update = {}
        ids = metadata.get('identifiers', {})
        doi = ids.get('doi')
        title = metadata.get('title')
        if doi is not None or title is not None:
            if doi is None:
                xref_results = utopialib.crossref.search(title)
                if len(xref_results) == 1:
                    best = xref_results[0]
                    xref_title = best.get('title', '').strip(' .')
                    if len(xref_title) > 0:
                        matched = False
                        if document is not None:
                            # Accept the crossref title if present in the document (do magic dash pattern thing)
                            xref_title = re.sub(ur'[^-\u002D\u007E\u00AD\u058A\u05BE\u1400\u1806\u2010-\u2015\u2053\u207B\u208B\u2212\u2E17\u2E3A\u2E3B\u301C\u3030\u30A0\uFE31\uFE32\uFE58\uFE63\uFF0D]+', lambda x: re.escape(x.group(0)), xref_title)
                            xref_title = re.sub(ur'[\u002D\u007E\u00AD\u058A\u05BE\u1400\u1806\u2010-\u2015\u2053\u207B\u208B\u2212\u2E17\u2E3A\u2E3B\u301C\u3030\u30A0\uFE31\uFE32\uFE58\uFE63\uFF0D-]+', lambda x: r'\p{{Pd}}{{{0}}}'.format(len(x.group(0))), xref_title)
                            import spineapi
                            matches = document.search(xref_title, spineapi.RegExp + spineapi.IgnoreCase)
                            matched = (len(matches) > 0)
                        else:
                            matched = (xref_title.lower() == title)
                        if matched:
                            update = best
                            doi = best.get('identifiers', {}).get('doi')
                            if doi is not None:
                                if doi.startswith('http://dx.doi.org/'):
                                    doi = doi[18:]
            if doi is not None:
                if None not in (document, title):
                    # What is this DOI's article's title according to crossref?
                    try:
                        xref_results = utopialib.crossref.resolve(doi)
                        xref_title = xref_results.get('title', '')
                        if len(xref_title) > 0:
                            print 'crossref: resolved title:', xref_title.encode('utf8')

                            if re.sub(r'[^\w]+', ' ', title).strip() == re.sub(r'[^\w]+', ' ', xref_title).strip(): # Fuzzy match
                                print 'crossref: titles match precisely'
                                update.update(xref_results)
                            else:
                                # Accept the crossref title over the scraped title, if present in the document
                                matches = document.findInContext('', xref_title, '') # Fuzzy match
                                if len(matches) > 0:
                                    update.update(xref_results)
                                    print 'crossref: overriding scraped title with crossref title'
                                else:
                                    print 'crossref: ignoring resolved metadata'
                                    # FIXME should we discard the DOI at this point?
                    except Exception as e:
                        import traceback
                        traceback.print_exc()

        if doi is not None:
            ids['doi'] = doi
            update['identifiers'] = ids

        return update

    def purposes(self):
        return 'identify'

    def weight(self):
        return 0


class PubMedIdentifier(utopia.library.Resolver):
    '''Given information, try to identify the document in the PubMed database.'''

    def resolve(self, metadata, document = None):
        update = {}
        ids = metadata.get('identifiers', {})
        if 'pubmed' not in ids:
            pubmed_id = None
            if 'doi' in ids:
                pubmed_id = utopialib.pubmed.identify(ids['doi'], 'doi')
                if pubmed_id is not None:
                    ids['pubmed'] = pubmed_id
                    update['identifiers'] = ids
            if pubmed_id is None and 'title' in metadata:
                title = metadata['title'].strip(' .')
                pubmed_results = utopialib.pubmed.search(title)
                pubmed_title = pubmed_results.get('title', '').strip(' .')
                if len(pubmed_title) > 0:
                    matched = False
                    pubmed_pmid = pubmed_results.get('identifiers', {}).get('pubmed')
                    if re.sub(r'[^\w]+', ' ', title).strip().lower() == re.sub(r'[^\w]+', ' ', pubmed_title).strip().lower(): # Fuzzy match
                        matched = True
                    elif document is not None:
                        # Accept the pubmed title over the scraped title, if present in the document
                        matches = document.findInContext('', pubmed_title, '') # Fuzzy match
                        if len(matches) > 0:
                            matched = True
                            pubmed_title = matches[0].text()
                    if matched:
                        ids.update(pubmed_results.get('identifiers', {}))
                        update['identifiers'] = ids
                        update['title'] = pubmed_title
                        pubmed_id = pubmed_pmid
        return update

    def purposes(self):
        return 'identify'

    def weight(self):
        return 1




###############################################################################
### Given particular identifiers, try to find as much information as possible.
###############################################################################

class ArXivExpander(utopia.library.Resolver):
    '''From an ArXiv ID, expand the metadata'''

    def resolve(self, metadata, document = None):
        # If an ArXiv ID is present, look it up
        update = {}
        ids = metadata.get('identifiers', {})
        if 'arxiv' in ids:
            results = utopialib.arxiv.resolve(ids['arxiv'])
            for key, value in results.iteritems():
                if key == 'identifiers':
                    ids.update(value)
                    update['identifiers'] = ids
                elif key == 'links':
                    links = metadata.get('links', [])
                    links.extend(value)
                    update['links'] = links
                else:
                    update[key] = value
        return update

    def purposes(self):
        return 'expand'

    def weight(self):
        return 10


class CrossRefExpander(utopia.library.Resolver):
    '''From a DOI, expand the metadata'''

    def resolve(self, metadata, document = None):
        # If a DOI is present, look it up
        update = {}
        ids = metadata.get('identifiers', {})
        if 'doi' in ids:
            results = utopialib.crossref.resolve(ids['doi'])
            for key, value in results.iteritems():
                if key == 'identifiers':
                    ids.update(value)
                    update['identifiers'] = ids
                elif key == 'links':
                    links = metadata.get('links', [])
                    links.extend(value)
                    update['links'] = links
                else:
                    update[key] = value
        return update

    def purposes(self):
        return 'expand'

    def weight(self):
        return 11


class PubMedExpander(utopia.library.Resolver):
    '''From a PubMed ID, expand the metadata'''

    def resolve(self, metadata, document = None):
        # If a PubMed ID is present, look it up
        update = {}
        ids = metadata.get('identifiers', {})
        if 'pubmed' in ids:
            results = utopialib.pubmed.resolve(ids['pubmed'])
            for key, value in results.iteritems():
                if key in ('identifiers',):
                    ids.update(value)
                    update['identifiers'] = ids
                elif key in ('links',):
                    links = metadata.get('links', [])
                    links.extend(value)
                    update['links'] = links
                else:
                    update[key] = value
        return update

    def purposes(self):
        return 'expand'

    def weight(self):
        return 12




###############################################################################
### Given metadata, try to resolve URL links.
###############################################################################

class DOIResolver(utopia.library.Resolver):
    """Resolve a URL from a DOI"""

    def resolve(self, metadata, document = None):
        # If a DOI is present, but no CrossRef link, we can generate one
        ids = metadata.get('identifiers', {})
        if 'doi' in ids and not hasLink(metadata, {':whence': 'crossref'}):
            links = metadata.get('links', [])
            links.append({
                'url': 'http://dx.doi.org/{0}'.format(ids['doi']),
                'mime': 'text/html',
                'type': 'article',
                'title': "Show on publisher's website",
                ':weight': 100,
                ':whence': 'crossref',
                })
            return {'links': links}

    def purposes(self):
        return 'dereference'

    def weight(self):
        return 100


class PubMedResolver(utopia.library.Resolver):
    """Resolve a URL from a PubMed ID"""

    def resolve(self, metadata, document = None):
        # If a PubMed ID is present, but no PubMed link, we can generate one
        ids = metadata.get('identifiers', {})
        if 'pubmed' in ids and not hasLink(metadata, {':whence': 'pubmed'}):
            links = metadata.get('links', [])
            links.append({
                'url': 'http://www.ncbi.nlm.nih.gov/pubmed/{0}'.format(ids['pubmed']),
                'mime': 'text/html',
                'type': 'abstract',
                'title': 'Show in PubMed',
                ':weight': 80,
                ':whence': 'pubmed',
                })
            return {'links': links}

    def purposes(self):
        return 'dereference'

    def weight(self):
        return 101


class HTMLResolver(utopia.library.Resolver):
    """Resolve PDF link from an article's web page"""

    def resolve(self, metadata, document = None):
        update = {}
        if not hasLink(metadata, {'mime': 'application/pdf', ':whence': 'scraped'}):
            links = metadata.get('links', [])
            article_links = filterLinks(links, {'type': 'article', 'mime': 'text/html'})
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
                article_link['resolved_url'] = resource.geturl()
                dom = etree.parse(StringIO(html), parser)

                # look for the PDF link
                citation_pdf_urls = dom.xpath('/html/head/meta[@name="citation_pdf_url"]/@content')
                for pdf_url in citation_pdf_urls:
                    if pdf_url != resource.geturl(): # Check for cyclic references
                        links.append({
                            'url': pdf_url,
                            'mime': 'application/pdf',
                            'type': 'article',
                            'title': 'Download article',
                            ':weight': article_link.get(':weight', 10),
                            ':whence': 'scraped',
                            })
            update['links'] = links
        return update

    def purposes(self):
        return 'dereference'

    def weight(self):
        return 102


class NatureResolver(utopia.library.Resolver):
    """Resolve PDF link from a Nature page"""

    def resolve(self, metadata, document = None):
        update = {}
        if not hasLink(metadata, {'mime': 'application/pdf', ':whence': 'nature'}):
            links = metadata.get('links', [])
            resolved_links = filterLinks(links, {'resolved_url': None})
            for link in resolved_links:
                url = link['resolved_url']
                if 'www.nature.com' in url:
                    parser = etree.HTMLParser()
                    resource = urllib2.urlopen(url, timeout=12)
                    html = resource.read()
                    dom = etree.parse(StringIO(html), parser)

                    # look for the PDF link
                    download_pdf_urls = dom.xpath('//li[@class="download-pdf"]/a/@href')
                    for pdf_url in download_pdf_urls:
                        pdf_url = urlparse.urljoin(url, pdf_url)
                        if pdf_url != resource.geturl(): # Check for cyclic references
                            links.append({
                                'url': pdf_url,
                                'mime': 'application/pdf',
                                'type': 'article',
                                'title': 'Download article from Nature',
                                ':weight': 20,
                                ':whence': 'nature',
                                })

                    # look for the supplementary PDF link(s)
                    for supp in dom.xpath('//div[@id="supplementary-information"]//dl'):
                        download_supp_pdf_urls = supp.xpath('//dt/a/@href')
                        for pdf_url in download_supp_pdf_urls:
                            pdf_url = urlparse.urljoin(url, pdf_url)
                            if pdf_url != resource.geturl(): # Check for cyclic references
                                links.append({
                                    'url': pdf_url,
                                    'mime': 'application/pdf',
                                    'type': 'supplementary',
                                    'title': 'Download supplementary information from Nature',
                                    ':weight': 20,
                                    ':whence': 'nature',
                                    })

                    update['links'] = links
        return update

    def purposes(self):
        return 'dereference'

    def weight(self):
        return 103


class WileyResolver(utopia.library.Resolver):
    """Resolve PDF link from a Wiley page"""

    def resolve(self, metadata, document = None):
        update = {}
        if not hasLink(metadata, {'mime': 'application/pdf', ':whence': 'wiley'}):
            links = metadata.get('links', [])
            pdf_links = filterLinks(links, {'mime': 'application/pdf'})
            for link in pdf_links:
                url = link['url']
                if 'onlinelibrary.wiley.com' in url:
                    parser = etree.HTMLParser()
                    resource = urllib2.urlopen(url, timeout=12)
                    html = resource.read()
                    dom = etree.parse(StringIO(html), parser)

                    # look for the PDF link
                    download_pdf_urls = dom.xpath('//iframe[@id="pdfDocument"]/@src')
                    for pdf_url in download_pdf_urls:
                        pdf_url = urlparse.urljoin(url, pdf_url)
                        if pdf_url != resource.geturl(): # Check for cyclic references
                            links.append({
                                'url': pdf_url,
                                'mime': 'application/pdf',
                                'type': 'article',
                                'title': 'Download article from Wiley',
                                ':weight': 20,
                                ':whence': 'wiley',
                                })
                    update['links'] = links
        return update

    def purposes(self):
        return 'dereference'

    def weight(self):
        return 103

class ScienceDirectResolver(utopia.library.Resolver):
    """Resolve PDF link from a Science Direct page"""

    def resolve(self, metadata, document = None):
        update = {}
        if not hasLink(metadata, {'mime': 'application/pdf', ':whence': 'sciencedirect'}):
            links = metadata.get('links', [])
            resolved_links = filterLinks(links, {'resolved_url': None})
            for link in resolved_links:
                url = link['resolved_url']
                if 'www.sciencedirect.com' in url:
                    parser = etree.HTMLParser()
                    resource = urllib2.urlopen(url, timeout=12)
                    html = resource.read()
                    dom = etree.parse(StringIO(html), parser)

                    # look for the PDF link
                    download_pdf_urls = dom.xpath('//a[@id="pdfLink"]/@href')
                    for pdf_url in download_pdf_urls:
                        pdf_url = urlparse.urljoin(url, pdf_url)
                        if pdf_url != resource.geturl(): # Check for cyclic references
                            links.append({
                                'url': pdf_url,
                                'mime': 'application/pdf',
                                'type': 'article',
                                'title': 'Download article from Science Direct',
                                ':weight': 20,
                                ':whence': 'sciencedirect',
                                })
                    update['links'] = links
        return update

    def purposes(self):
        return 'dereference'

    def weight(self):
        return 103

class IEEEResolver(utopia.library.Resolver):
    """Resolve PDF link from an IEEE page"""

    def resolve(self, metadata, document = None):
        update = {}
        if not hasLink(metadata, {'mime': 'application/pdf', ':whence': 'ieeexplore'}):
            links = metadata.get('links', [])
            resolved_links = filterLinks(links, {'resolved_url': None})
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
                                links.append({
                                    'url': pdf_url,
                                    'mime': 'application/pdf',
                                    'type': 'article',
                                    'title': 'Download article from IEEEXplore',
                                    ':weight': 20,
                                    ':whence': 'ieeexplore',
                                    })
                    update['links'] = links
        return update

    def purposes(self):
        return 'dereference'

    def weight(self):
        return 103

class ACSResolver(utopia.library.Resolver):
    """Resolve PDF link from an ACS page"""

    def resolve(self, metadata, document = None):
        update = {}
        if not hasLink(metadata, {'mime': 'application/pdf', ':whence': 'acs'}):
            links = metadata.get('links', [])
            resolved_links = filterLinks(links, {'resolved_url': None})
            for link in resolved_links:
                url = link['resolved_url']
                if 'pubs.acs.org' in url:
                    parser = etree.HTMLParser()
                    resource = urllib2.urlopen(url, timeout=12)
                    html = resource.read()
                    dom = etree.parse(StringIO(html), parser)

                    # look for the PDF link
                    download_pdf_urls = dom.xpath('//div[@class="bottomViewLinks"]/a[text()="PDF"]/@href')
                    for pdf_url in download_pdf_urls:
                        pdf_url = urlparse.urljoin(url, pdf_url)
                        if pdf_url != resource.geturl(): # Check for cyclic references
                            links.append({
                                'url': pdf_url,
                                'mime': 'application/pdf',
                                'type': 'article',
                                'title': 'Download article from ACS',
                                ':weight': 20,
                                ':whence': 'acs',
                                })
                    update['links'] = links
        return update

    def purposes(self):
        return 'dereference'

    def weight(self):
        return 103

class PMCResolver(utopia.library.Resolver):
    """Resolve PDF link from a PMC ID"""

    def resolve(self, metadata, document = None):
        if not hasLink(metadata, {'mime': 'application/pdf', ':whence': 'pmc'}):
            identifiers = metadata.get('identifiers', {})

            # Try to resolve the PMC ID
            doi = identifiers.get('doi')
            pmid = identifiers.get('pubmed')
            pmcid = identifiers.get('pmc')
            if doi is not None and pmcid is None:
                pmcid = utopialib.pmc.identify(doi, 'doi')
            if pmid is not None and pmcid is None:
                pmcid = utopialib.pmc.identify(pmid, 'pmid')

            # Generate PMC link to PDF
            if pmcid is not None:
                identifiers['pmc'] = pmcid
                pdf_url = 'http://www.ncbi.nlm.nih.gov/pmc/articles/{0}/pdf/'.format(pmcid)
                links = metadata.get('links', [])
                links.append({
                    'url': pdf_url,
                    'mime': 'application/pdf',
                    'type': 'article',
                    'title': 'Download article from PubMed Central',
                    ':weight': 100,
                    ':whence': 'pmc',
                    })
                return {
                    'identifiers': identifiers,
                    'links': links,
                    }

    def purposes(self):
        return 'dereference'

    def weight(self):
        return 104


class HackyResolver(utopia.library.Resolver):
    """Assign a convenience property"""

    def resolve(self, metadata, document = None):
        links = metadata.get('links', [])
        pdf_links = filterLinks(links, {'mime': 'application/pdf'})
        if len(pdf_links) > 0:
            return {'pdf': pdf_links[0]['url']}

    def purposes(self):
        return 'dereference'

    def weight(self):
        return 9999


class _LoggingResolver(utopia.library.Resolver):
    """Log metadata"""

    def resolve(self, metadata, document = None):
        import pprint
        pp = pprint.PrettyPrinter(indent=4)
        pp.pprint(metadata)

    def purposes(self):
        return ('dereference', 'identify', 'expand')

    def weight(self):
        return 10000


class _WaitingResolver(utopia.library.Resolver):
    """Wait for a while (for testing purposes)"""

    def resolve(self, metadata, document = None):
        import time
        for i in xrange(0, 80):
            time.sleep(0.5)

    def purposes(self):
        return ('dereference', 'identify', 'expand')

    def weight(self):
        return 10001
