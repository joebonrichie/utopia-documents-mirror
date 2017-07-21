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

#? urls: https://beta.openphacts.org/ http://beta.openphacts.org/

import json
import re
import spineapi
import urllib
import utopia.document

import urllib2
from lxml import etree

class OpenPhactsAnnotator(utopia.document.Annotator, utopia.document.Visualiser):

    searchTag = '07a84994-e464-4bbf-812a-a4b96fa3d197'
    searchUrl = 'https://beta.openphacts.org/1.5/search/byTag?'

    infoUrl = 'http://beta.openphacts.org/1.5/compound?'
    cwUriTemplate = 'http://www.conceptwiki.org/concept/{0}'
    cwConcept = ''

    appId = 'd318f02d'
    appKey = '5101509adbed44c32b9efd874dad5688'

    itemQueries = {
        'http://www.ebi.ac.uk/chembl': ('chembl', ('mw_freebase', 'type')),
        'http://ops.rsc.org': ('chemspider', ('ro5_violations', 'psa', 'logp', 'hbd', 'hba', 'smiles', 'inchi', 'inchikey', 'rtb', 'molweight', 'molformula')),
        'http://linkedlifedata.com/resource/drugbank': ('drugbank', ('toxicity', 'description', 'proteinBinding', 'biotransformation')),
    }

    def on_explore_event(self, phrase, document):
        #make the call
        url = self.searchUrl + urllib.urlencode({'app_id' : self.appId, 'app_key' : self.appKey,'q': phrase.encode('utf8'), 'branch': 4, 'uuid': self.searchTag,})
        response = urllib2.urlopen(url, timeout=8).read()

        #print response
        data = json.loads(response)
        print data

        # Get the first result
        if len(data) > 0:
            primaryTopic = data['result']['primaryTopic']['result'][0]
            cwConcept = primaryTopic['_about']

            # Resolve OPS data
            compoundInfoUrl = self.infoUrl + urllib.urlencode({'uri': cwConcept, '_format': 'xml', 'app_id' : self.appId, 'app_key' : self.appKey})
            compoundResponse = urllib2.urlopen(compoundInfoUrl, timeout=8).read()
            dom = etree.fromstring(compoundResponse)

            print etree.tostring(dom, pretty_print=True, encoding='utf8')

            # Parse compound information
            topic = dom.find('primaryTopic')

            if topic is not None:
                items = {}
                values = {}

                prefLabel = etree.tostring(topic.find('prefLabel'), method="text", encoding=unicode, with_tail=False)
                uuid = topic.attrib['href']
                for item in topic.xpath('exactMatch/item'):
                    hrefs = item.xpath('inDataset/@href')
                    if len(hrefs) == 1:
                        items[hrefs[0]] = item

                for item_type, item_elem in items.iteritems():
                    queries = self.itemQueries.get(item_type, ())
                    if len(queries) == 2:
                        prefix = queries[0]
                        values['{0}_id'.format(prefix)] = item_elem.attrib['href']
                        for query in queries[1]:
                            value = etree.tostring(item_elem.find(query), method="text", encoding=unicode, with_tail=False)
                            if value is not None:
                                values[query] = value

                if len(values) > 0:
                    a = spineapi.Annotation()
                    a['concept'] = 'OPENPHACTS'
                    a['property:cwUUID'] = cwConcept
                    a['property:properties'] = json.dumps(values)
                    a['property:name'] = 'Open PHACTS'
                    a['property:description'] = prefLabel
                    a['property:sourceIcon'] = utopia.get_plugin_data_as_url('images/openphacts.png', 'image/png')
                    a['property:sourceDescription'] = '<p><a href="http://www.openphacts.org/">Open PHACTS</a> is building an Open Pharmacological Space (OPS), a freely available platform, integrating pharmacological data from a variety of information resources and providing tools and services to question this integrated data to support pharmacological research.</p>'
                    return(a,)

    def visualisable(self, a):
        return a.get('concept') == 'OPENPHACTS'

    itemDescriptions = {
        'logp': 'AlogP',
        'hba': '# H-Bond Acceptors',
        'hbd': '# H-Bond Donors',
        'molweight': 'Mol Weight',
        'mw_freebase': 'MW Freebase',
        'psa': 'Polar Surface Area',
        'rtb': '# Rotatable Bonds',
    }

    def visualise(self, a):
        row = u'<tr><td style="text-align:right"><strong>{0}: </strong></td><td style="text-align:left">{1}</td></tr>'
        css = u'''
            <style>
                .formula {
                    font-weight: bold;
                    font-size: 1.3em;
                    padding: 0.6em 0 0.8em 14px;
                }
                .formula sub {
                    font-size: xx-small;
                    position: relative;
                    bottom: -0.3em;
                }
            </style>
        '''

        # Start blank
        html = u''

        concept = a.get('property:cwUUID')
        data = json.loads(a.get('property:properties', '{}'))
        smiles = data.get('smiles')
        inchi = data.get('inchi')
        inchikey = data.get('inchikey')
        imageUrl = None

        # Deal with special properties first
        if 'molformula' in data:
            molformula = data['molformula']
            molformula = molformula.replace(' ', '')
            molformula = re.sub(r'([A-Z][a-z]?)', r'<span class="element">\1</span>', molformula)
            molformula = re.sub(r'(\d+)', r'<sub>\1</sub>', molformula)
            html += u'<p class="formula">{0}</p>'.format(molformula)
            del data['molformula']
        if 'chemspider_id' in data:
            imageUrl = '{0}/image'.format(data['chemspider_id'])
            del data['chemspider_id']
        if 'description' in data:
            description = data['description'].split(' ')
            html += u'<p><strong>Description:</strong> '
            html += u' '.join(description[:32])
            if len(description) > 32:
                html += u' <span class="readmore">{0}</span>'.format(' '.join(description[32:]))
            html += u'</p>'
            del data['description']
        if 'biotransformation' in data:
            biotransformation = data['biotransformation'].split(' ')
            html += u'<p><strong>Biological Transformation:</strong> '
            html += u' '.join(biotransformation[:32])
            if len(biotransformation) > 32:
                html += u' <span class="readmore">{0}</span>'.format(' '.join(biotransformation[32:]))
            html += u'</p>'
            del data['biotransformation']
        if imageUrl is not None:
            html += u'<center><img width="128" height="128" src="{0}" /></center>'.format(imageUrl)
        if smiles is not None or inchi is not None or inchikey is not None:
            html += u'<div class="expandable" title="SMILES and InChI"><div class="box">'
            if smiles is not None:
                html += u'<p><strong>SMILES: </strong><br>{0}</p>'.format(smiles)
                del data['smiles']
            if inchi is not None:
                html += u'<p><strong>InChI: </strong><br>{0}</p>'.format(inchi)
                del data['inchi']
            if inchikey is not None:
                html += u'<p><strong>InChIKey: </strong><br>{0}</p>'.format(inchikey)
                del data['inchikey']
            html += u'</div></div>'

        # General properties
        data = dict(((k, v) for k, v in data.items() if k in self.itemDescriptions))
        if len(data) > 0:
            html += u'<div class="expandable" title="Compound Properties"><div class="box" style="width:100%">'
            html += u'<table width="200px" style="border:0"><col width="50%"><col width="50%">'
            for key, value in data.items():
                if key == 'psa':
                    value = u'{0} &Aring;&sup2;'.format(value)
                html += row.format(self.itemDescriptions.get(key, key), value)
            html += u'</table></div></div>'

        html += u'<div align="right"><a href="http://explorer.openphacts.org/#!p=CmpdByNameForm&u=' + concept + '">More information</a></div>'

        return css, html
