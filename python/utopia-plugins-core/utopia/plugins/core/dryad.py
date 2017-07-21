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

#? name: Dryad
#? www: http://datadryad.org/
#? urls: http://datadryad.org/ https://utopia.cs.manchester.ac.uk/


import utopia.tools.utils
import json
import re
import socket
import spineapi
import utopia.document
import urllib2
from lxml import etree
import string

ns = {
    'mets': 'http://www.loc.gov/METS/',
    'dim': 'http://www.dspace.org/xmlns/dspace/dim',
    'kend': 'http://utopia.cs.manchester.ac.uk/kend',
    'property': 'http://utopia.cs.manchester.ac.uk/kend/property#'
}

class Dryad(utopia.document.Annotator, utopia.document.Visualiser):
    """Generate Dryad information"""

    def on_ready_event(self, document):
        #print "RUNNING DRYAD PLUGIN"
        doi = utopia.tools.utils.metadata(document, 'identifiers[doi]')

        if doi is not None:

            # see if kend knows about this DOI as a Dryad record
            response = urllib2.urlopen('https://utopia.cs.manchester.ac.uk/kend/0.7/define/lookup?database=dryad&term=%s&limit=1000' % doi, timeout=8)
            root = etree.fromstring(response.read())
            dryadShortHandle = root.findtext('kend:group/kend:annotation/kend:properties/property:databaseTerm', namespaces=ns)
            if dryadShortHandle is not None:
                # then we have a dryad short-form doi, so can now safely go to dryad to get the rest

                response = urllib2.urlopen('http://datadryad.org/solr/search/select/?q=dc.relation.isreferencedby:%s&fl=dc.identifier,dc.title_ac,dc.identifier.uri,dc.contributor.author,dc.date.issued.year,dc.identifier.citation,dc.description' % doi, timeout=8)
                root = etree.fromstring(response.read())
                #print etree.tostring(root, pretty_print=True, encoding='utf8')
                result = root.find('result')

                if result.attrib['numFound'] != '0':
                    # then we have found some datasets for this article DOI

                    packageDetails = urllib2.urlopen('http://datadryad.org/metadata/handle/%s/mets.xml' % dryadShortHandle, timeout=8)
                    root = etree.fromstring(packageDetails.read())
                    #print etree.tostring(root, pretty_print=True, encoding='utf8')

                    identifiers = root.findall('mets:dmdSec/mets:mdWrap/mets:xmlData/dim:dim/dim:field[@element="identifier"]', namespaces=ns)
                    packageDOI = None
                    for identifier in identifiers:
                        if identifier.text.startswith('doi:'):
                            packageDOI = identifier.text[4:]
                            break

                    contributors = root.findall('mets:dmdSec/mets:mdWrap/mets:xmlData/dim:dim/dim:field[@qualifier="author"]', namespaces=ns)
                    dataCitation = {
                        'year': result.findtext("doc/arr[@name='dc.date.issued.year']/int", namespaces=ns),
                        'title': root.findtext('mets:dmdSec/mets:mdWrap/mets:xmlData/dim:dim/dim:field[@element="title"]', namespaces=ns),
                        'authors': [string.capwords(a.text) for a in contributors],
                        'source': 'Dryad Digital Repository',
                        'doi': packageDOI,
                    }
                    articleCitation = root.findtext('mets:dmdSec/mets:mdWrap/mets:xmlData/dim:dim/dim:field[@qualifier="citation"][@element="identifier"]', namespaces=ns)

                    xhtml = '''
                        <p>
                          The data associated with this article are available via Dryad. When using
                          these data, please cite both the article:
                        </p>
                        <div class="box">{0}<br /><a href="http://dx.doi.org/{1}">doi:{1}</a></div>
                        <p>
                          and also the data package:
                        </p>
                        <div class="box">{2}<br /><a href="http://dx.doi.org/{3}">doi:{3}</a></div>
                    '''.format(articleCitation, doi, utopia.citation.format(dataCitation), dataCitation['doi'])

                    a = spineapi.Annotation()
                    a['concept'] = 'Dryad'
                    a['property:name'] = 'Dryad'
                    a['property:sourceDatabase'] = 'dryad'
                    a['property:sourceDescription'] = '<p><a href="http://datadryad.org/">Dryad</a> is an international repository of data underlying peer-reviewed articles in the basic and applied biosciences.</p>'
                    a['property:description'] = 'Data associated with this article'
                    a['property:xhtml'] = xhtml
                    document.addAnnotation(a)

    def visualisable(self, a):
        return a.get('concept') == 'Dryad'

    def visualise(self, a):
        return a.get('property:xhtml')
