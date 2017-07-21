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

import json
import kend.client
import kend.model
import spineapi
import urllib2
import utopia.citation
import utopia.document
import utopia.tools.utils




class Metadata(utopia.document.Annotator):
    '''Ensure metadata in this document is up to date'''

    ###########################################################################
    ## Step 2: try to resolve as much information as possible

    def on_load_event(self, document):
        '''Using the document content, try to resolve various bits of metadata'''
        #import pprint

        # Start by getting any citations already in the document
        input_citations = []
        for annotation in document.annotations('Document Metadata'):
            # Check the kinds of annotations that hold citation information
            if annotation.get('concept') in ('Citation',):
                # Compile information from annotation
                input_citations.append(utopia.tools.utils.citation_from_annotation(annotation))
        citations = input_citations[:]
        #pprint.PrettyPrinter(indent=2).pprint(citations)

        # Run the resolution pipeline
        flattened = utopia.citation.resolve(citations=citations, document=document)

        #pprint.PrettyPrinter(indent=2).pprint(citations)

        # Save the resulting citations as annotations in the document
        sources = flattened.get('provenance', {}).get('sources', [])
        for citation in sources:
            if citation not in input_citations and 'error' not in citation:
                utopia.tools.utils.store_metadata(document, **citation)

        # Deal with errors
        errors = {}
        failures = 0
        successes = 0
        for error in [error for error in sources if 'error' in error]:
            provenance = error.get('provenance', {})
            failures += 1

            component = provenance.get('whence')
            plugin = provenance.get('plugin')
            errors.setdefault(component, {})

            error = error.get('error', {})
            category = error.get('category')
            errors[component].setdefault(category, [])

            message = error.get('message', '')
            errors[component][category].append((plugin, message))

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

    ###########################################################################
    ## Step 3: save resolved metadata back to the server

    def _after_ready_event(self, document):
        srctype = 'kend/{0}.{1}.{2}'.format(*utopia.bridge.version_info[:3])
        keys = ('publication-title', 'publisher', 'identifiers',
                'title', 'volume', 'issue', 'pages', 'pagefrom', 'pageto', 'year',
                'month', 'authors[]')

        metadata = {}
        for key in keys:
            value = utopia.tools.utils.metadata(document, key, all=True)
            if value is not None and len(value) > 0:
                metadata[key] = value

        if len(metadata) > 0:
            doc = kend.model.Document()
            for key, values in metadata.iteritems():
                for value in values:
                    whence = None
                    if utopia.tools.utils.hasprovenance(value):
                        prov = utopia.tools.utils.provenance(value)
                        whence = prov.get('whence')
                    if key[-2:] == '[]':
                        meta = kend.model.Evidence(type=key[:-2], data='; '.join(value), srctype=srctype, src=whence)
                    else:
                        if isinstance(value, dict):
                            value = 'json:{0}'.format(json.dumps(value))
                        meta = kend.model.Evidence(type=key, data=value, srctype=srctype, src=whence)
                    doc.metadata.append(meta)

            document_id = utopia.tools.utils.metadata(document, 'identifiers[utopia]')
            print('-------')
            print(document_id)
            uri = urllib2.urlopen(document_id, timeout=12).headers.getheader('Content-Location')
            resolved = kend.client.Client().submitMetadata(uri, doc).metadata



class MetadataSummariser(utopia.document.Annotator):

    def after_ready_event(self, document):
        # Make an annotation for all these metadata
        ids = {
            'doi': ('DOI', u'<a href="http://dx.doi.org/{0}">{0}</a>'),
            'issn': ('ISSN', u'<strong>{0}</strong>'),
            'pii': ('PII', u'<strong>{0}</strong>'),
            'pubmed': ('Pubmed', u'<a href="http://www.ncbi.nlm.nih.gov/pubmed/{0}">{0}</a>'),
            'pmc': ('PMC', u'<a href="http://www.ncbi.nlm.nih.gov/pmc/articles/{0}">{0}</a>'),
            'arxiv': ('arXiv', u'<a href="http://arxiv.org/abs/{0}">{0}</a>'),
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
            id = utopia.tools.utils.metadata(document, 'identifiers[{0}]'.format(key))
            if id is not None:
                fragments.append(u'<td style="text-align: right; opacity: 0.7">{0}:</td><td>{1}</td>'.format(name, format.format(id)))
        issn = utopia.tools.utils.metadata(document, 'publication-issn')
        if issn is not None:
            fragments.append(u'<td style="text-align: right; opacity: 0.7">{0}:</td><td><strong>{1}</strong></td>'.format('ISSN', issn))
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
        title = utopia.tools.utils.metadata(document, 'title')
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
            annotation['session:weight'] = '1000'
            annotation['session:default'] = '1'
            annotation['session:headless'] = '1'
            document.addAnnotation(annotation)
