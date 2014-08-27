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

import json
import re
import spineapi
import urllib
import utopia.document

import urllib2
from lxml import etree

class OpenPhactsAnnotator(utopia.document.Annotator, utopia.document.Visualiser):

	searchTag = '07a84994-e464-4bbf-812a-a4b96fa3d197'
	searchUrl = 'https://beta.openphacts.org/1.3/search/byTag?'

	infoUrl = 'http://beta.openphacts.org/1.3/compound?'
	cwUriTemplate = 'http://www.conceptwiki.org/concept/{0}'
	cwConcept = ''

	appId = 'd318f02d'
	appKey = '5101509adbed44c32b9efd874dad5688'

	itemQueries = {
		'http://www.ebi.ac.uk/chembl': ('chembl', ('mw_freebase', 'type')),
		'http://ops.rsc.org': ('chemspider', ('ro5_violations', 'psa', 'logp', 'hbd', 'hba', 'smiles', 'inchi', 'inchikey', 'rtb', 'molweight', 'molformula')),
		'http://linkedlifedata.com/resource/drugbank': ('drugbank', ('toxicity', 'description', 'proteinBinding', 'biotransformation')),
	}
	itemTransformations = {
		'description': 'molecularDescription',
		'molformula': 'moleculeFormula',
		'biotransformation': 'bioTransformation',
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

	            prefLabel = topic.findtext('prefLabel')
	            uuid = topic.attrib['href']
	            for item in topic.xpath('exactMatch/item'):
	                hrefs = item.xpath('inDataset/@href')
	                if len(hrefs) == 1:
	                    items[hrefs[0]] = item

	            for item_type, item_elem in items.iteritems():
	                queries = self.itemQueries.get(item_type, ())
	                prefix = queries[0]
	                values['{0}_id'.format(prefix)] = item_elem.attrib['href']
	                for query in queries[1]:
	                    value = item_elem.findtext(query)
	                    if value is not None:
	                        values[self.itemTransformations.get(query, query)] = value

	            if len(values) > 0:
	                a = spineapi.Annotation()
	                a['concept'] = 'OPENPHACTS'
	                a['property:cwUUID'] = cwConcept
	                for key, value in values.iteritems():
	                    a['property:{0}'.format(key)] = value
	                a['property:name'] = 'Open PHACTS'
	                a['property:description'] = prefLabel
	                a['property:sourceIcon'] = utopia.get_plugin_data_as_url('images/openphacts.png', 'image/png')
	                a['property:sourceDescription'] = '<p><a href="http://www.openphacts.org/">Open PHACTS</a> is building an Open Pharmacological Space (OPS), a freely available platform, integrating pharmacological data from a variety of information resources and providing tools and services to question this integrated data to support pharmacological research.</p>'
	                return(a,)

	def visualisable(self, a):
	    return a.get('concept') == 'OPENPHACTS'

	def visualise(self, a):
	    bioTransformation = a.get('property:bioTransformation')
	    description = a.get('property:molecularDescription')
	    moleculeFormula = a.get('property:moleculeFormula')
	    chemspider_id = a.get('property:chemspider_id', '')[19:]
	    smiles = a.get('property:smiles')
	    inchi = a.get('property:inchi')
	    inchikey = a.get('property:inchikey')
	    alogp = a.get('property:logp')
	    hba = a.get('property:hba')
	    hbd = a.get('property:hbd')
	    molWeight = a.get('property:molweight')
	    mwFreebase = a.get('property:mw_freebase')
	    psa = a.get('property:psa')
	    rbonds = a.get('property:ro5_violations')
	    concept = a.get('property:cwUUID')
	    imageUrl = None
	    if len(chemspider_id) > 0:
	        imageUrl = 'http://ops.rsc.org/{0}/image'.format(chemspider_id)
	        #print imageUrl

	    html = u'<style>.formula {font-weight:bold; font-size: 1.2em;} .formula sub{font-size:xx-small; position:relative; bottom:-0.3em;}</style>'
	    if moleculeFormula is not None:
	        moleculeFormula = moleculeFormula.replace(' ', '')
	        moleculeFormula = re.sub(r'([A-Z][a-z]?)', r'<span class="element">\1</span>', moleculeFormula)
	        moleculeFormula = re.sub(r'(\d+)', r'<sub>\1</sub>', moleculeFormula)
	        html += u'<p><span class="formula">{0}</span></p>'.format(moleculeFormula)
	    if description is not None:
	        description = description.split(' ')
	        html += u'<p><strong>Description:</strong> '
	        html += u'{0}'.format(' '.join(description[:32]))
	        if len(description) > 32:
	            html += u' <span class="readmore">{0}</span>'.format(' '.join(description[32:]))
	        html += u'</p>'
	    if bioTransformation is not None:
	        bioTransformation = bioTransformation.split(' ')
	        html += u'<p><strong>Biological Transformation:</strong> '
	        html += u'{0}'.format(' '.join(bioTransformation[:32]))
	        if len(bioTransformation) > 32:
	            html += u' <span class="readmore">{0}</span>'.format(' '.join(bioTransformation[32:]))
	        html += u'</p>'
	    if imageUrl is not None:
	        html += u'<center><img width="128" height="128" src="{0}" /></center>'.format(imageUrl)
	    html += u'<div class="expandable" title="SMILES and InChI"><div class="box">'
	    if smiles is not None:
	        html += '<p><strong>SMILES: </strong><p>' + smiles
	        html += u'</p>'
	    if inchi is not None:
	        html += u'<p><strong>InChI: </strong><p>' + inchi
	        html += u'</p>'
	    if inchikey is not None:
	        html += u'<p><strong>InChIKey: </strong><p>' + inchikey + '</div></div>'
	        html += u'</p>'
	    html += u'<div class="expandable" title="Compound Properties"><div class="box" style="width:100%">'
	    html += u'<table width="200px" style="border:0"><col width="50%"><col width="50%">'
	    if alogp is not None:
	        html += u'<tr><td style="text-align:right;font-weight: bold">AlogP: </td><td style="text-align:left">' + alogp + '</td></tr>'
	    #html += u'<p><strong>AlogP: </strong>' + alogp
	    # html += u'</p>'
	    if hba is not None:
	        html += u'<tr><td style="text-align:right"><strong># H-Bond Acceptors: </strong></td><td style="text-align:left">' + hba + '</td></tr>'
	    #html += u'<p><strong># H-Bond Acceptors: </strong>' + hba
	    #html += u'</p>'
	    if hbd is not None:
	        html += u'<tr><td style="text-align:right"><strong># H-Bond Donors: </strong></td><td style="text-align:left">' + hbd + '</td></tr>'
	    #html += u'<p><strong># H-Bond Donors: </strong>' + hda
	    #html += u'</p>'
	    if molWeight is not None:
	        html += u'<tr><td style="text-align:right"><strong>Mol Weight: </strong></td><td style="text-align:left">' + molWeight + '</td></tr>'
	    #html += u'<p><strong>Mol Weight: </strong>' + molWeight
	    #html += u'</p>'
	    if mwFreebase is not None:
	        html += u'<tr><td style="text-align:right"><strong>MW Freebase: </strong></td><td style="text-align:left">' + mwFreebase + '</td></tr>'
	    #html += u'<p><strong>MW Freebase: </strong>' + mwFreebase
	    #html += u'</p>'
	    if psa is not None:
	        html += u'<tr><td style="text-align:right"><strong>Polar Surface Area: </strong></td><td style="text-align:left">' + psa + '</td></tr>'
	    #html += u'<p><strong>Polar Surface Area: </strong>' + psa
	    #html += u'</p>'
	    if rbonds is not None:
	        html += u'<tr><td style="text-align:right"><strong># Rotatable Bonds: </strong></td><td style="text-align:left">' + rbonds + '</td></tr>'
	        html += '</table></div></div>'
	    #print 'concept' + self.cwConcept
	    html += '<div align="right"><a href="http://explorer.openphacts.org/#!p=CmpdByNameForm&u=' + concept + '">More information</a></div>'
	    return html
