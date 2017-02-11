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

import re
import socket
import spineapi
import traceback
import urllib2
import utopialib.arxiv
import utopialib.crossref
import utopialib.doi
import utopialib.pmc
import utopialib.pubmed
import utopialib.title


def resolve_on_content(document):

    # Keep track of errors so that we can inform the user
    def add_error(component, method, category = None, message = None, exception = None):
        if exception is not None:
            if isinstance(exception, urllib2.URLError) and hasattr(exception, 'reason') and isinstance(exception.reason, socket.timeout):
                exception = exception.reason
            if isinstance(exception, socket.timeout):
                category = 'timeout'
                message = 'The server did not respond'
            elif isinstance(exception, urllib2.HTTPError):
                category = 'server'
                message = unicode(getattr(exception, 'reason', 'The server did not respond as expected'))
            elif isinstance(exception, urllib2.URLError):
                category = 'connection'
                message = unicode(getattr(exception, 'reason', 'The server could not be found'))
        error = spineapi.Annotation()
        error['concept'] = 'Error'
        error['property:component'] = component
        error['property:method'] = method
        error['property:category'] = category
        if message is not None:
            error['property:message'] = message
        document.addAnnotation(error, 'errors.metadata')
    def add_success(component, method):
        error = spineapi.Annotation()
        error['concept'] = 'Success'
        error['property:component'] = component
        error['property:method'] = method
        error['property:category'] = 'success'
        document.addAnnotation(error, 'errors.metadata')

    scraped = {'identifiers':{}, 'links':[]}

    authors = []
    publication = None
    volume = None
    issue = None
    year = None
    pages = None

    #################################################################################
    # Scrape DOI and title, if not already present

    doi = utopialib.utils.metadata(document, 'identifiers[doi]')
    if doi is None:
        doi = utopialib.doi.scrape(document)
    if doi is not None:
        scraped['identifiers']['doi'] = doi
    print 'scraper: doi:', (doi and doi.encode('utf8'))
    title = utopialib.utils.metadata(document, 'title')
    if title is None:
        title = utopialib.title.scrape(document)
    if title is not None:
        scraped['title'] = title
    print 'scraper: title:', (title and title.encode('utf8'))

    #################################################################################
    # Scrape arXiv ID

    arxivid = utopialib.arxiv.scrape(document)
    print 'scraper: arxivid:', (arxivid and arxivid.encode('utf8'))
    if arxivid is not None:
        scraped['identifiers']['arxiv'] = arxivid
        try:
            arxiv_results = utopialib.arxiv.resolve(arxivid)
            if arxiv_results is not None:
                arxiv_results.update({':whence': 'arxiv', ':weight': 10})
                doi = arxiv_results.get('identifiers', {}).get('doi', doi)
                title = arxiv_results.get('title', title)
                utopialib.utils.store_metadata(document, **arxiv_results)
        except Exception as e:
            add_error('ArXiv', 'resolve', exception=e)
            traceback.print_exc()
        else:
            add_success('ArXiv', 'resolve')

    #################################################################################
    # Fold in the CrossRef data

    issn = utopialib.utils.metadata(document, 'publication-issn')
    if title is not None or doi is not None:
        if doi is None:
            try:
                xref_results = utopialib.crossref.search(title)
                if len(xref_results) == 1:
                    xref_title = xref_results[0].get('title')
                    if xref_title is not None:
                        print 'crossref: resolved title:', xref_title.encode('utf8')
                        # Accept the crossref title if present in the document (do magic dash pattern thing)
                        xref_title = re.sub(ur'[^-\u002D\u007E\u00AD\u058A\u05BE\u1400\u1806\u2010-\u2015\u2053\u207B\u208B\u2212\u2E17\u2E3A\u2E3B\u301C\u3030\u30A0\uFE31\uFE32\uFE58\uFE63\uFF0D]+', lambda x: re.escape(x.group(0)), xref_title)
                        xref_title = re.sub(ur'[\u002D\u007E\u00AD\u058A\u05BE\u1400\u1806\u2010-\u2015\u2053\u207B\u208B\u2212\u2E17\u2E3A\u2E3B\u301C\u3030\u30A0\uFE31\uFE32\uFE58\uFE63\uFF0D-]+', lambda x: r'\p{{Pd}}{{{0}}}'.format(len(x.group(0))), xref_title)
                        #print 'crossref: resolved title pattern:', xref_title.encode('utf8')
                        matches = document.search(xref_title, spineapi.RegExp + spineapi.IgnoreCase)
                        if len(matches) > 0:
                            doi = xref_results[0].get('doi')
                            print 'crossref: accepting resolved doi'
            except Exception as e:
                add_error('CrossRef', 'search', exception=e)
                traceback.print_exc()
            else:
                add_success('CrossRef', 'search')
        if doi is not None:
            # What is this DOI's article's title according to crossref?
            try:
                xref_results = utopialib.crossref.resolve(doi)
                xref_results.update({':whence': 'crossref', ':weight': 20})
                xref_title = xref_results.get('title', '')
                if len(xref_title) > 0:
                    print 'crossref: resolved title:', xref_title.encode('utf8')
                    if re.sub(r'[^\w]+', ' ', title).strip() == re.sub(r'[^\w]+', ' ', xref_title).strip(): # Fuzzy match
                        print 'crossref: titles match precisely'
                        utopialib.utils.store_metadata(document, **xref_results)
                    else:
                        # Accept the crossref title over the scraped title, if present in the document
                        matches = document.findInContext('', xref_title, '') # Fuzzy match
                        if len(matches) > 0:
                            utopialib.utils.store_metadata(document, **xref_results)
                            title = xref_title
                            print 'crossref: overriding scraped title with crossref title'
                        else:
                            print 'crossref: ignoring resolved metadata'
                            # FIXME should we discard the DOI at this point?
            except Exception as e:
                add_error('CrossRef', 'resolve', exception=e)
                traceback.print_exc()
            else:
                add_success('CrossRef', 'resolve')

    ###########################################################################################
    # Fold in the PubMed data
    pii = utopialib.utils.metadata(document, 'identifiers[pii]')
    pmid = utopialib.utils.metadata(document, 'identifiers[pubmed]')
    pmcid = utopialib.utils.metadata(document, 'identifiers[pmc]')
    if pmid is None and doi is not None: # resolve on DOI
        try:
            pmid = utopialib.pubmed.identify(doi, 'doi')
        except Exception as e:
            add_error('PubMed', 'resolve', exception=e)
            traceback.print_exc()
        else:
            add_success('PubMed', 'resolve')
    if pmid is None and title is not None: # resolve on title
        try:
            pubmed_results = utopialib.pubmed.search(title)
            pubmed_title = pubmed_results.get('title', '').strip(' .')
            if len(pubmed_title) > 0:
                print 'pubmed: resolved title:', pubmed_title.encode('utf8')
                pubmed_pmid = pubmed_results.get('identifiers', {}).get('pubmed')
                print 'pubmed: resolved pmid:', pubmed_pmid
                if re.sub(r'[^\w]+', ' ', title).strip() == re.sub(r'[^\w]+', ' ', pubmed_title).strip(): # Fuzzy match
                    print 'pubmed: titles match precisely'
                    title = pubmed_title
                    pmid = pubmed_pmid
                else:
                    # Accept the pubmed title over the scraped title, if present in the document
                    matches = document.findInContext('', pubmed_title, '') # Fuzzy match
                    if len(matches) > 0:
                        title = matches[0].text()
                        pmid = pubmed_pmid
                        print 'pubmed: overriding scraped title with pubmed title'
                    else:
                        print 'pubmed: ignoring resolved title'
        except Exception as e:
            add_error('PubMed', 'search', exception=e)
            traceback.print_exc()
        else:
            add_success('PubMed', 'search')
    if pmid is not None:
        try:
            pubmed_results = utopialib.pubmed.resolve(pmid)
            if pubmed_results is not None:
                pubmed_results.update({':whence': 'pubmed', ':weight': 10})
                utopialib.utils.store_metadata(document, **pubmed_results)
        except Exception as e:
            add_error('PubMed', 'fetch', exception=e)
            traceback.print_exc()
        else:
            add_success('PubMed', 'fetch')

    ###########################################################################################
    # Fold in the PubMedCentral data
    if pmcid is None and doi is not None: # resolve on DOI
        try:
            pmcid = utopialib.pmc.identify(doi, 'doi')
        except Exception as e:
            add_error('PubMed Central', 'resolve', exception=e)
            traceback.print_exc()
        else:
            add_success('PubMed Central', 'resolve')
    if pmcid is None and pmid is not None: # resolve on PubMed ID
        try:
            pmcid = utopialib.pmc.identify(pmid, 'pmid')
        except Exception as e:
            add_error('PubMed Central', 'resolve', exception=e)
            traceback.print_exc()
        else:
            add_success('PubMed Central', 'resolve')
    if pmcid is not None:
        utopialib.utils.store_metadata(document, **{':whence': 'pmc', ':weight': 10, 'pmcid': pmcid})
        try:
            nlm = utopialib.pmc.fetch(pmcid)
            if nlm is not None:
                utopialib.utils.store_metadata(document, **{':whence': 'pmc', ':weight': 10, 'raw_pmc_nlm': nlm})
        except Exception as e:
            add_error('PubMed Central', 'fetch', exception=e)
            traceback.print_exc()
        else:
            add_success('PubMed Central', 'fetch')

    ###########################################################################################

    scraped.update({':whence': 'document', ':weight': 5})
    utopialib.utils.store_metadata(document, **scraped)

