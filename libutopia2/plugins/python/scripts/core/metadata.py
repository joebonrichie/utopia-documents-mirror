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

import common.arxiv
import common.crossref
import common.doi
import common.pmc
import common.pubmed
import common.title
import common.utils
import kend.client
import kend.model
import re
import socket
import spineapi
import traceback
import urllib2
import utopia.document
from lxml import etree



class MetadataStorage(utopia.document.Annotator):
    """Submit standard metadata"""

    def _resolve(self, document):
        # Start with evidence from fingerprinting
        evidence = [kend.model.Evidence(type='fingerprint', data=f, srctype='document') for f in document.fingerprints()]
        documentref = kend.model.DocumentReference(evidence=evidence)
        documentref = kend.client.Client().documents(documentref)

        try:
            return documentref.id
        except AttributeError:
            pass

    def after_ready_event(self, document):
        keys = ('publication-title', 'publisher', 'issn', 'doi', 'pmid', 'pmcid', 'pii',
                'title', 'volume', 'issue', 'pages', 'pagefrom', 'pageto', 'year',
                'month', 'authors[]') # Authors? FIXME

        metadata = {}
        for key in keys:
            value = common.utils.metadata(document, key, all=True)
            if value is not None and len(value) > 0:
                metadata[key] = value

        if len(metadata) > 0:
            doc = kend.model.Document()
            for key, values in metadata.iteritems():
                for value in values:
                    whence = None
                    if common.utils.hasprovenance(value):
                        prov = common.utils.provenance(value)
                        whence = prov.get('whence')
                    if key[-2:] == '[]':
                        meta = kend.model.Evidence(type=key[:-2], data='; '.join(value), srctype='kend/{0}.{1}.{2}'.format(*utopia.version_info[:3]), src=whence)
                    else:
                        meta = kend.model.Evidence(type=key, data=value, srctype='kend/{0}.{1}.{2}'.format(*utopia.version_info[:3]), src=whence)
                    doc.metadata.append(meta)

        document_id = self._resolve(document)
        uri = urllib2.urlopen(document_id).headers.getheader('Content-Location')
        kend.client.Client().submitMetadata(uri, doc).metadata




class Metadata(utopia.document.Annotator):
    """Assemble standard metadata"""

    def on_load_event(self, document):

        # Keep track of errors so that we can inform the user
        def add_error(component, method, category = None, message = None, exception = None):
            if exception is not None:
                if isinstance(exception, urllib2.URLError) and isinstance(exception.reason, socket.timeout):
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

        metadata = {
            'scraped': {},
            'arxiv': {},
            'pubmed': {},
            'pmc': {},
            'crossref': {},
            'utopia': {},
        }

        authors = []
        publication = None
        volume = None
        issue = None
        year = None
        pages = None

        #################################################################################
        # Scrape DOI and title

        doi = common.doi.scrape(document)
        metadata['scraped']['doi'] = doi
        print 'scraper: doi:', (doi and doi.encode('utf8'))
        title = common.title.scrape(document)
        metadata['scraped']['title'] = title
        print 'scraper: title:', (title and title.encode('utf8'))

        #################################################################################
        # Scrape arXiv ID

        arxivid = common.arxiv.scrape(document)
        if arxivid is not None:
            metadata['scraped']['arxivid'] = arxivid
            try:
                arxiv_results = common.arxiv.resolve(arxivid)
                if arxiv_results is not None:
                    arxiv_results.update({':whence': 'arxiv', ':weight': 10})
                    common.utils.store_metadata(document, **arxiv_results)
            except Exception as e:
                add_error('ArXiv', 'resolve', exception=e)
                traceback.print_exc()
            else:
                add_success('ArXiv', 'resolve')

        #################################################################################
        # Fold in the CrossRef data

        issn = common.utils.metadata(document, 'issn')
        if title is not None or doi is not None:
            if doi is None:
                try:
                    xref_results = common.crossref.search(title)
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
                    xref_results = common.crossref.resolve(doi)
                    xref_results.update({':whence': 'crossref', ':weight': 20})
                    xref_title = xref_results.get('title', '')
                    if len(xref_title) > 0:
                        print 'crossref: resolved title:', xref_title.encode('utf8')
                        if re.sub(r'[^\w]+', ' ', title).strip() == re.sub(r'[^\w]+', ' ', xref_title).strip(): # Fuzzy match
                            print 'crossref: titles match precisely'
                            common.utils.store_metadata(document, **xref_results)
                        else:
                            # Accept the crossref title over the scraped title, if present in the document
                            matches = document.findInContext('', xref_title, '') # Fuzzy match
                            if len(matches) > 0:
                                common.utils.store_metadata(document, **xref_results)
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
        pii = common.utils.metadata(document, 'pii')
        pmid = common.utils.metadata(document, 'pmid')
        pmcid = common.utils.metadata(document, 'pmcid')
        if pmid is None and doi is not None: # resolve on DOI
            try:
                pmid = common.pubmed.resolve(doi, 'doi')
            except Exception as e:
                add_error('PubMed', 'resolve', exception=e)
                traceback.print_exc()
            else:
                add_success('PubMed', 'resolve')
        if pmid is None and title is not None: # resolve on title
            try:
                pubmed_results = common.pubmed.search(title)
                pubmed_title = pubmed_results.get('title', '').strip(' .')
                if len(pubmed_title) > 0:
                    print 'pubmed: resolved title:', pubmed_title.encode('utf8')
                    pubmed_pmid = pubmed_results.get('pmid')
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
                nlm = common.pubmed.fetch(pmid)
                if nlm is not None:
                    xml = etree.fromstring(nlm)

                    pubmed_authors = []
                    for author in xml.findall('PubmedArticle/MedlineCitation/Article/AuthorList/Author'):
                        name = u''
                        lastName = author.findtext('LastName')
                        forename = author.findtext('ForeName')
                        if lastName is not None:
                            name = lastName + u', '
                        if forename is not None:
                            name += forename
                        if len(name) > 0:
                            pubmed_authors.append(name)
                    if len(pubmed_authors) == 0:
                        pubmed_authors = None

                    pubmed_pmid = xml.findtext('PubmedArticle/MedlineCitation/PMID')

                    common.utils.store_metadata(document, **{
                        ':whence': 'pubmed',
                        ':weight': 10,
                        'raw_pubmed_nlm': nlm,
                        'authors': pubmed_authors,
                        'pmid': pubmed_pmid,
                        'title': xml.findtext('PubmedArticle/MedlineCitation/Article[1]/ArticleTitle'),
                        'issn': xml.findtext('PubmedArticle/MedlineCitation/Article/Journal/ISSN[1]'),
                        'doi': xml.findtext('PubmedArticle/PubmedData/ArticleIdList/ArticleId[@IdType="doi"]'),
                        'pmcid': xml.findtext('PubmedArticle/PubmedData/ArticleIdList/ArticleId[@IdType="pmc"]'),
                        'pii': xml.findtext('PubmedArticle/PubmedData/ArticleIdList/ArticleId[@IdType="pii"]'),
                        'publication-title': xml.findtext('PubmedArticle/MedlineCitation/Article/Journal/Title'),
                        'volume': xml.findtext('PubmedArticle/MedlineCitation/Article/Journal/JournalIssue/Volume'),
                        'issue': xml.findtext('PubmedArticle/MedlineCitation/Article/Journal/JournalIssue/Issue'),
                        'year': xml.findtext('PubmedArticle/MedlineCitation/Article/Journal/JournalIssue/PubDate/Year'),
                        'pages': xml.findtext('PubmedArticle/MedlineCitation/Article[1]/Pagination/MedlinePgn'),
                        'abstract': xml.findtext('PubmedArticle/MedlineCitation/Article[1]/Abstract/AbstractText'),
                    })
                    pmid = pubmed_pmid or pmid

                    # FIXME I'm sure the above should be in common.pubmed
            except Exception as e:
                add_error('PubMed', 'fetch', exception=e)
                traceback.print_exc()
            else:
                add_success('PubMed', 'fetch')

        ###########################################################################################
        # Fold in the PubMedCentral data
        if pmcid is None and doi is not None: # resolve on DOI
            try:
                pmcid = common.pmc.resolve(doi, 'doi')
            except Exception as e:
                add_error('PubMed Central', 'resolve', exception=e)
                traceback.print_exc()
            else:
                add_success('PubMed Central', 'resolve')
        if pmcid is None and pmid is not None: # resolve on PubMed ID
            try:
                pmcid = common.pmc.resolve(pmid, 'pmid')
            except Exception as e:
                add_error('PubMed Central', 'resolve', exception=e)
                traceback.print_exc()
            else:
                add_success('PubMed Central', 'resolve')
        if pmcid is not None:
            common.utils.store_metadata(document, **{':whence': 'pmc', ':weight': 10, 'pmcid': pmcid})
            try:
                nlm = common.pmc.fetch(pmcid)
                if nlm is not None:
                    common.utils.store_metadata(document, **{':whence': 'pmc', ':weight': 10, 'raw_pmc_nlm': nlm})
            except Exception as e:
                add_error('PubMed Central', 'fetch', exception=e)
                traceback.print_exc()
            else:
                add_success('PubMed Central', 'fetch')

        ###########################################################################################

        scraped = metadata['scraped']
        scraped.update({':whence': 'document', ':weight': 5})
        common.utils.store_metadata(document, **scraped)




    def after_load_event(self, document):
        # Put errors together in a sensible way
        errors = {}
        failures = 0
        successes = 0
        for error in document.annotations('errors.metadata'):
            if error.get('concept') == 'Success':
                successes += 1
            elif error.get('concept') == 'Error':
                failures += 1

            component = error.get('property:component')
            errors.setdefault(component, {})

            category = error.get('property:category')
            errors[component].setdefault(category, [])

            method = error.get('property:method')
            message = error.get('property:message', '')
            errors[component][category].append((method, message))
        categories = {}
        for component, details in errors.iteritems():
            for category in details.keys():
                categories.setdefault(category, 0)
                categories[category] += 1

        # If there are errors, provide feedback to the user
        if failures > 0:
            # Check for likely client problems
            if categories.get('connection', 0) == failures and successes == 0:
                summary = '''
                    Utopia could not reach any of the online services it would
                    normally use to identify this document, meaning you are
                    likely to see limited or no information below. You might
                    wish to check your Internet connection and reload the
                    document.
                    '''
            elif categories.get('timeout', 0) > 1:
                if categories.get('timeout', 0) == failures and successes == 0:
                    many = ''
                else:
                    many = 'some of'
                summary = '''
                    Utopia gave up contacting {0} the online services it would
                    normally use to identify this document because they were
                    taking too long to respond. You are likely to see limited
                    or no information below. You might wish to check your
                    Internet connection and reload the document.
                    '''.format(many)
            else:
                if failures == 1:
                    noun = 'An error'
                else:
                    noun = 'Errors'
                summary = '''
                    {0} occurred when trying to discover the identity
                    of this document. You are likely to see limited or no
                    information below.
                    '''.format(noun)
            html = '''
                <div class="box error">
                    <strong>Warning</strong>
                    <p>
                        {0}
                    </p>
                    <div class="expandable" title="Details...">
                    <ul>
            '''.format(summary)
            for component, details in errors.iteritems():
                for category, methods in details.iteritems():
                    if category != 'success':
                        summary = {
                            'timeout': '{0} did not respond',
                            'connection': 'Could not connect to {0}',
                            'server': '{0} behaved unexpectedly',
                        }.get(category, 'An error occurred accessing {0}')
                        methods_html = ', '.join(('<span title="{1}">{0}</span>'.format(method, message) for method, message in methods))
                        html += '<li>{0} (when accessing: {1}).</li>'.format(summary.format('<strong>' + component + '</strong>'), methods_html)
            html += '''
                    </ul>
                    </div>
                <div>
            '''
            annotation = spineapi.Annotation()
            annotation['concept'] = 'Collated'
            annotation['property:html'] = html
            annotation['property:name'] = 'Error'
            annotation['session:weight'] = '1000'
            annotation['session:default'] = '1'
            annotation['session:headless'] = '1'
            document.addAnnotation(annotation)

        print errors




    def after_ready_event(self, document):
        # Make an annotation for all these metadata
        ids = {
            'doi': ('DOI', u'<a href="http://dx.doi.org/{0}">{0}</a>'),
            'issn': ('ISSN', u'<strong>{0}</strong>'),
            'pii': ('PII', u'<strong>{0}</strong>'),
            'pmid': ('Pubmed', u'<a href="http://www.ncbi.nlm.nih.gov/pubmed/{0}">{0}</a>'),
            'pmcid': ('PMC', u'<a href="http://www.ncbi.nlm.nih.gov/pmc/articles/{0}">{0}</a>'),
            'arxivid': ('arXiv', u'<a href="http://arxiv.org/abs/{0}">{0}</a>'),
        }
        # Build list of fragments
        fragments = []
        pub_icon = ''
        html = '''
            <style>
              .fancy_quotes {
                position: relative;
              }
              .fancy_quotes:before {
                content: "\\201C";
              }
              .fancy_quotes:after {
                content: "\\201D";
              }
            </style>
        '''
        for key, (name, format) in ids.iteritems():
            id = common.utils.metadata(document, key)
            if id is not None:
                fragments.append(u'<td style="text-align: right; opacity: 0.7">{0}:</td><td>{1}</td>'.format(name, format.format(id)))
        # Resolve publisher info
        for annotation in document.annotations('PublisherMetadata'):
            if annotation.get('concept') == 'PublisherIdentity':
                logo = annotation.get('property:logo')
                title = annotation.get('property:title')
                webpageUrl = annotation.get('property:webpageUrl')
                if None not in (logo, title, webpageUrl):
                    pub_icon = u'<a href="{0}" title="{2}"><img src="{1}" alt="{2}" /></a></td>'.format(webpageUrl, logo, title)
                    break
        # Compile fragments
        title = common.utils.metadata(document, 'title')
        if title is not None or len(pub_icon) > 0:
            html += u'<table style="border: none; margin: 0 0 1em 0;">'
            html +=   u'<tr>'
            if title is not None:
                html +=   u'<td style="text-align:left; vertical-align: middle;"><strong class="nohyphenate fancy_quotes">{0}</strong></td>'.format(title.strip())
            if len(pub_icon) > 0:
                html +=   u'<td style="text-align:right; vertical-align: middle; width: 80px;">{0}</td>'.format(pub_icon)
            html +=   u'</tr>'
            html += u'</table>'
        if len(fragments) > 0:
            html += u'<div class="box">'
            html +=   u'<table style="border: none">'
            html +=     u'<tr>'
            html +=     u'</tr><tr>'.join(fragments)
            html +=     u'</tr>'
            html +=   u'</table>'
            html += u'</div>'

            annotation = spineapi.Annotation()
            annotation['concept'] = 'Collated'
            annotation['property:html'] = html
            annotation['property:name'] = 'About this article'
            annotation['session:weight'] = '100'
            annotation['session:default'] = '1'
            annotation['session:headless'] = '1'
            document.addAnnotation(annotation)
