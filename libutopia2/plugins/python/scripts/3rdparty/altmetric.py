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

#? name: Altmetric
#? www: http://www.altmetric.com/
#? urls: http://api.altmetric.com/ http://www.altmetric.com/

import common.utils
import json
import socket
import spineapi
import utopia.document
# from coda_network ? FIXME coda_network fails to read from this URL
import urllib2

class Altmetric(utopia.document.Annotator, utopia.document.Visualiser):
    """Generate Altmetric information"""

    api_version = 'v1'
    key = '46a63bea7a7f245bf46fad25aced4d28'

    def populate(self, document):
        doi = common.utils.metadata(document, 'doi')
        if doi is not None:
            try:
                # Check to see if the DOI is known
                url = 'http://api.altmetric.com/{0}/doi/{2}?key={1}'.format(self.api_version, self.key, doi)
                data = urllib2.urlopen(url, timeout=8).read()
                json.loads(data) # Just check this is possible - throws exception otherwise

                a = spineapi.Annotation()
                a['concept'] = 'Altmetric'
                a['property:doi'] = doi
                a['property:json'] = data
                a['property:name'] = 'Altmetric'
                a['property:description'] = 'Who is talking about this article?'
                a['property:sourceDatabase'] = 'altmetric'
                a['property:sourceDescription'] = '<p>Discover, track and analyse online activity related to this article with <a href="http://www.altmetric.com/">Altmetric</a>.</p>'
                a['session:weight'] = '1'
                a['session:default'] = '1'
                document.addAnnotation(a)
            except (urllib2.URLError, socket.timeout):
                pass

    def visualisable(self, a):
        return a.get('concept') == 'Altmetric' and 'property:doi' in a

    def visualise(self, a):
        # Generate unique ID
        data = json.loads(a.get('property:json'))
        id = 'alt_{0}'.format(data['altmetric_id'])

        return '''
            <style>
              #{0} .citation .scorebox {{
                float: none;
                margin: 0px auto;
                height: auto;
              }}
              #{0} .citation .scorebox .pie {{
                height: auto;
              }}
              #{0} .citation .details {{
                margin-left: 0px;
                text-align: center;
              }}
              #{0} .citation .details h3 {{
                margin-bottom: 10px;
              }}
              #{0} .citation .details small {{
                display: block;
                font-style: italic;
                color: #777;
              }}
              #{0} .citation .details .tq {{
                border: solid 5px black;
                -webkit-border-image: url(qrc:/images/border-rounded-box.png) 5 stretch stretch;
                padding: 1px;
                margin: 4px 0;
                text-align: left;
                color: inherit;
                font-size: 0.9em;
                opacity: 1;
                width: 100%;
              }}
            </style>
            <div id="{0}" class="altmetric" />
        '''.format(id), '''
            <script>
            $('#{0}').altmetric({{api_version: self.api_version, doi: '{1}'}});
            </script>
        '''.format(id, a['property:doi'])

