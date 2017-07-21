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

#? name: Sherpa/RoMEO
#? www: http://www.sherpa.ac.uk/romeo/
#? urls: http://www.sherpa.ac.uk/


import utopia.tools.utils
import json
import re
import socket
import spineapi
import utopia.document
import urllib
import urllib2
from lxml import etree

class SherpaRomeo(utopia.document.Annotator, utopia.document.Visualiser):
    """Generate Sherpa Romeo information"""

    apiKey = 'TGb5fUBjk9Q'

    def on_ready_event(self, document):
        issn = utopia.tools.utils.metadata(document, 'publication-issn')
        doi = utopia.tools.utils.metadata(document, 'identifiers[doi]')
        if issn is not None:
            params = {
                'versions': 'all',
                'issn': issn,
                'ak': self.apiKey
            }

            url = 'http://www.sherpa.ac.uk/romeo/api29.php?' + urllib.urlencode(params)
            srResponse = urllib2.urlopen(url, timeout=8)
            srData = srResponse.read()

            root = etree.fromstring(srData)
            #print etree.tostring(root, pretty_print=True, encoding='utf8')

            colour = root.find('publishers/publisher/romeocolour')

            if colour is not None:
                a = spineapi.Annotation()
                a['concept'] = 'SherpaRomeo'
                a['property:doi'] = doi
                a['property:name'] = 'Sherpa/RoMEO'
                a['property:sourceDatabase'] = 'sherparomeo'
                a['property:sourceDescription'] = '<p><a href="http://www.sherpa.ac.uk/romeo/">SHERPA/RoMEO</a> provides information about publisher copyright policies for this article.</p>'
                a['property:description'] = "Archiving status is '" +colour.text+ "'."

                explanation = {}
                explanation['green'] = "the author can archive pre-print <em>and</em> post-print or publisher's version/PDF"
                explanation['blue'] = "the author can archive post-print (i.e. final draft post-refereeing) or publisher's version/PDF"
                explanation['yellow'] = "the author can archive pre-print (i.e. pre-refereeing)"
                explanation['white'] = "archiving of this article not formally supported"

                journalTitle = root.find('journals/journal/jtitle')
                publisherName = root.find('publishers/publisher/name')
                publisherURL = root.find('publishers/publisher/homeurl')

                xhtml = "<p>"
                xhtml = xhtml + 'This '+ journalTitle.text + ' article, published by <a href="' + publisherURL.text +'">' + publisherName.text + '</a>, is classified as being <a href="http://www.sherpa.ac.uk/romeo/definitions.php">RoMEO ' + colour.text + '</a>. '
                xhtml = xhtml + 'This means that ' + explanation[colour.text] + '.</p>'
                xhtml = xhtml + '<p>Other <a href="http://www.sherpa.ac.uk/romeo/issn/%s/">details and conditions</a> apply.</p>' % issn
                a['property:xhtml'] = xhtml

                document.addAnnotation(a)

    def visualisable(self, a):
        return a.get('concept') == 'SherpaRomeo' and 'property:doi' in a

    def visualise(self, a):
        return a.get('property:xhtml')

