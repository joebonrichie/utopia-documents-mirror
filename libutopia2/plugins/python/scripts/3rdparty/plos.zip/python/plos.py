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

#? name: PLOS
#? www: http://www.plos.org/
#? urls: http://www.plos.org/ http://alm.plos.org/ http://www.ploscompbiol.org/

import common.utils
import json
import spineapi
import urllib
import utopia.document

import urllib2


class PLOSALMAnnotator(utopia.document.Annotator):
    '''Annotate document with PLOS article level metrics'''

    api_key = 'RECOCHzR7Ib9juq'

    def populate(self, document):
        # Get resolved DOI
        doi = common.utils.metadata(document, 'doi', '')

        # Only for PLOS DOIs should this plugin do anything
        if doi.startswith('10.1371/'):

            # Record the publisher identity information
            annotation = spineapi.Annotation()
            annotation['concept'] = 'PublisherIdentity'
            annotation['property:logo'] = utopia.get_plugin_data_as_url('images/large_logo.jpg', 'image/jpg')
            annotation['property:title'] = 'PLOS'
            annotation['property:webpageUrl'] = 'http://www.plos.org/'
            document.addAnnotation(annotation, 'PublisherMetadata')

            # Attempt to get ALMs from PLOS API
            url = 'http://alm.plos.org/articles/{0}.json?{{0}}'.format(doi)
            query = { 'api_key': self.api_key, 'events': '1', 'source': 'counter,pmc' }
            url = url.format(urllib.urlencode(query))
            try:
                alm_events = json.loads(urllib2.urlopen(url, timeout=8).read())
            # Not found
            except urllib2.HTTPError as e:
                if e.code == 404: # just ignore 404
                    return
                raise

            plos_pdf_views = 0
            plos_html_views = 0
            pmc_pdf_views = 0
            pmc_html_views = 0

            for source in alm_events.get('article', {}).get('source', []):
                if source.get('source') == 'Counter':
                    events = source.get('events', [])
                    plos_pdf_views, plos_html_views = reduce(lambda accum,event: (accum[0]+int(event.get('pdf_views', 0)),accum[1]+int(event.get('html_views', 0))), events, (0, 0))
                elif source.get('source') == 'PubMed Central Usage Stats':
                    events = source.get('events', [])
                    pmc_pdf_views, pmc_html_views = reduce(lambda accum,event: (accum[0]+int(event.get('pdf', 0)),accum[1]+int(event.get('full-text', 0))), events, (0, 0))

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
