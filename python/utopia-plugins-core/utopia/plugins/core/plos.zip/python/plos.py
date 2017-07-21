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

#? name: PLOS
#? www: http://www.plos.org/
#? urls: http://www.plos.org/ http://alm.plos.org/ http://www.ploscompbiol.org/

import utopia.tools.utils
import json
import spineapi
import urllib
import utopia.document

import urllib2


class PLOSALMAnnotator(utopia.document.Annotator):
    '''Annotate document with PLOS article level metrics'''

    api_key = 'dtR6hV3mH4XJyahmfh5L'

    def on_ready_event(self, document):
        # Get resolved DOI
        doi = utopia.tools.utils.metadata(document, 'identifiers[doi]')

        # Only for PLOS DOIs should this plugin do anything
        if doi is not None and doi.startswith('10.1371/'):

            # Record the publisher identity information
            annotation = spineapi.Annotation()
            annotation['concept'] = 'PublisherIdentity'
            annotation['property:logo'] = utopia.get_plugin_data_as_url('images/large_logo.jpg', 'image/jpg')
            annotation['property:title'] = 'PLOS'
            annotation['property:webpageUrl'] = 'http://www.plos.org/'
            document.addAnnotation(annotation, 'PublisherMetadata')

            # Attempt to get ALMs from PLOS API
            query = { 'api_key': self.api_key, 'info': 'detail', 'ids': doi, 'type': 'doi' }
            url = 'http://alm.plos.org/api/v5/articles?{0}'.format(urllib.urlencode(query))
            request = urllib2.Request(url, headers={'Accepts': 'application/json'})
            try:
                data = urllib2.urlopen(request, timeout=8).read()
                alm = json.loads(data)
            # Not found
            except urllib2.HTTPError as e:
                if e.code == 404: # just ignore 404
                    return
                raise

            articles = alm.get('data', [])
            if len(articles) > 0:
                article = articles[0]
                metrics = dict(((source.get('name'), source.get('metrics')) for source in article.get('sources', [])))

                plos_pdf_views = metrics.get('counter', {}).get('pdf') or 0
                plos_html_views = metrics.get('counter', {}).get('html') or 0
                pmc_pdf_views = metrics.get('pmc', {}).get('pdf') or 0
                pmc_html_views = metrics.get('pmc', {}).get('html') or 0

                annotation = spineapi.Annotation()
                annotation['concept'] = 'PLOSALMRecord'
                annotation['property:doi'] = doi
                annotation['property:name'] = 'PLOS'
                annotation['property:description'] = 'Download statistics'
                annotation['property:plos_pdf_views'] = plos_pdf_views
                annotation['property:plos_html_views'] = plos_html_views
                annotation['property:pmc_pdf_views'] = pmc_pdf_views
                annotation['property:pmc_html_views'] = pmc_html_views
                annotation['property:sourceIcon'] = utopia.get_plugin_data_as_url('images/small_logo.png', 'image/png')
                annotation['property:sourceDescription'] = '<p><a href="http://www.plos.org/">PLOS</a> article level metrics for downloads.</p>'
                document.addAnnotation(annotation)


class PLOSALMVisualiser(utopia.document.Visualiser):

    def visualisable(self, annotation):
        return annotation.get('concept') == 'PLOSALMRecord'

    def visualise(self, annotation):
        doi = annotation['property:doi']
        plos_pdf_views = int(annotation['property:plos_pdf_views'])
        plos_html_views = int(annotation['property:plos_html_views'])
        pmc_pdf_views = int(annotation['property:pmc_pdf_views'])
        pmc_html_views = int(annotation['property:pmc_html_views'])

        html = '''
            <div>
              <p>Combined PLOS and PMC download statistics.</p>
              <p>&nbsp;&nbsp;&nbsp;&nbsp;HTML: <strong>{0} views</strong></p>
              <p>&nbsp;&nbsp;&nbsp;&nbsp;PDF: <strong>{1} views</strong></p>
              <p><a href="http://dx.doi.org/{2}">Explore in PLOS&hellip;</a></p>
            </div>
        '''.format(plos_html_views+pmc_html_views, plos_pdf_views+pmc_pdf_views, doi)

        return html
