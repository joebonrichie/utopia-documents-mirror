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

#? name: SciBite
#? www: http://scibite.com/
#? urls: http://scibite.com/


import common.utils
import json
import spineapi
import string
import urllib
import utopia.document
import urllib2
from lxml import etree

class Scibite(utopia.document.Annotator, utopia.document.Visualiser):
    """Generate Scibite information"""

    iconMap = {
        'News Item': 'clinicalnewsalert.png',
        'Publication': 'literaturealert.png',
        'Clinical Trial': 'clinicaltrialalert.png',
        'Patent': 'patentalert.png',
    }

    app_uri = 'https://services.scibite.com/site/sciapi/v3_1'
    app_id = '224cb3d7'
    app_key = '2300790e1a9c1642c8e4ec466fa5bcf7'

    def renderBite(self, bite):
        title = bite['biteTitle']
        citation = bite['biteCitation']
        url = bite['biteScibiteUrl']
        links = ['<a href="{0}">[{1}]</a>'.format(entity['bestLink'], entity['entityName']) for entity in bite['biteEntities']]
        return '<div class="box"><strong>{0}</strong>. {1} <a href="{2}">[View]</a><br /><br />Related entities: {3}</div>'.format(title, citation, url, ' '.join(links))

    def populate(self, document):
        pmid = common.utils.metadata(document, 'pmid')
        if pmid is not None:
            xhtml = ''

            params = {
                'app_id': self.app_id,
                'app_key': self.app_key,
                'i': pmid,
            }
            url = '{0}/DocumentEntitiesService?{1}'.format(self.app_uri, urllib.urlencode(params))
            response = urllib2.urlopen(url, timeout=15).read()
            results = json.loads(response.decode('latin1'))
            if results['RESP_SYS_STATUS'] == 'STAT_OK' and 'RESP_PAYLOAD' in results and len(results['RESP_PAYLOAD']) > 0:
                xhtml += '<h2>Related entities</h2>'
                for entity in results['RESP_PAYLOAD']:
                    xhtml += '<p><strong><a href="{0}">{1}</a></strong> ({2})</p>'.format(entity['bestLink'], entity['entityName'], entity['entityTypeDisplay'])

            params = {
                'app_id': self.app_id,
                'app_key': self.app_key,
                'i': pmid,
                'n': '10',
            }
            url = '{0}/DocumentToNewsService?{1}'.format(self.app_uri, urllib.urlencode(params))
            response = urllib2.urlopen(url, timeout=15).read()
            results = json.loads(response.decode('latin1'))
            if results['RESP_SYS_STATUS'] == 'STAT_OK' and 'RESP_PAYLOAD' in results and len(results['RESP_PAYLOAD']) > 0:
                xhtml += '<h2>Related news</h2>'
                for bite in results['RESP_PAYLOAD']:
                    xhtml += self.renderBite(bite)

            if len(xhtml) > 0:
                a = spineapi.Annotation()
                a['concept'] = 'SciBite'
                a['property:pmid'] = pmid
                a['property:name'] = 'SciBite'
                a['property:sourceDatabase'] = 'scibite'
                a['property:xhtml'] = xhtml
                a['property:description'] = 'Biomedical News & Intelligence'
                a['property:sourceDescription'] = '<p><a href="http://scibite.com/">SciBite</a> scans 1000s of papers, patents, blogs, newsfeeds and more to bring you daily alerts on critical topics in biomedicine.</p>'
                document.addAnnotation(a)

    def lookup(self, phrase, document):

        params = {
            'app_id': self.app_id,
            'app_key': self.app_key,
            'q': phrase.encode('utf-8'),
            'n': '10',
        }
        url = '{0}/EntityTextToNewsService?{1}'.format(self.app_uri, urllib.urlencode(params))
        response = urllib2.urlopen(url, timeout=15)
        results = json.loads(response.read().decode('latin1'))

        if results['RESP_SYS_STATUS'] == 'STAT_OK':
            if 'RESP_NORESULTS' in results or len(results['RESP_PAYLOAD']['body']) == 0:
                #print "Nothing sensible returned from scibite"
                return []
            else:
                scibitePayload = results['RESP_PAYLOAD']

                xhtml = '<h2>News related to the %s "%s"</h2>' % (scibitePayload['header'][0]['entityTypeDisplay'].lower(), scibitePayload['header'][0]['entityName'])

                for bite in scibitePayload["body"][:10]:
                    xhtml += self.renderBite(bite)

                url = 'http://scibite.com/site/topic/{0}:{1}'.format(scibitePayload['header'][0]['entityType'], scibitePayload['header'][0]['entityScid'])
                xhtml += '<p class="right"><a href="{0}">See more in SciBite...</a></p>'.format(url)

                a = spineapi.Annotation()
                a['concept'] = 'SciBite'
                a['property:name'] = 'SciBite'
                a['property:description'] = 'Biomedical News & Intelligence'
                a['property:sourceDatabase'] = 'scibite'
                a['property:sourceDescription'] = '<p><a href="http://scibite.com/">SciBite</a> scans 1000s of papers, patents, blogs, newsfeeds and more to bring you daily alerts on critical topics in biomedicine.</p>'
                a['property:xhtml'] = xhtml
                a['displayRelevance'] = '850'
                a['displayRank'] = '850'

                return [a]

    def visualisable(self, a):
        return a.get('concept') == 'SciBite' and 'property:xhtml' in a

    def visualise(self, a):
        return a.get('property:xhtml')


