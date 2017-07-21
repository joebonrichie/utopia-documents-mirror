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

#? name: ScienceWISE
#? www: http://sciencewise.info/
#? urls: http://beta.sciencewise.info/ http://sciencewise.info/


import utopia.tools.utils
import hashlib
import json
import spineapi
import urllib
import utopia.document
import urllib2

class ScienceWISE(utopia.document.Annotator, utopia.document.Visualiser):
    """Annotate with ScienceWISE"""

    @utopia.document.buffer
    def on_activate_event(self, document):
        text = document.text().encode('utf8')
        text_hash = hashlib.md5(text).hexdigest()

        url = 'http://beta.sciencewise.info/api/utopia'
        payload = urllib.urlencode({ 'text': text, 'chksum': text_hash })
        response = urllib2.urlopen(url, payload, timeout=8).read()
        results = json.loads(response)
        annotations = []

        for result in results:
            before = result.get('context', {}).get('before', '')
            term = result.get('value', '')
            after = result.get('context', {}).get('after', '')
            link = result.get('link')
            definitions = []
            for definition in result.get('definitions', []):
                definitions.append((definition.get('url'), definition.get('title')))

            if len(term) > 0 and len(before) + len(term) + len(after) > 0 and link is not None:
                matches = document.findInContext(before, term, after)
                if len(matches) > 0:
                    annotation = spineapi.Annotation()
                    annotation['concept'] = 'ScienceWISE'
                    annotation['property:webpageUrl'] = link
                    annotation['property:term'] = term
                    annotation['property:name'] = 'Definitions of {0}'.format(term)
                    annotation['property:description'] = 'ScienceWISE ontology definitions'
                    annotation['property:sourceDatabase'] = 'sciencewise'
                    annotation['property:sourceDescription'] = '<p><a href="http://sciencewise.info/">ScienceWISE</a> provides phycists with article annotation and scientific bookmarking.</p>'
                    for url, title in definitions:
                        annotation.insertProperty('property:definitions', '{0} {1}'.format(url, title))
                    for match in matches:
                        annotation.addExtent(match)
                    annotations.append(annotation)

        if len(annotations) > 0:
            document.addAnnotations(annotations)

    def visualisable(self, annotation):
        return annotation.get('concept') == 'ScienceWISE'

    def visualise(self, annotation):
        xhtml = '<div>Term: <strong>{0}</strong></div>{1}<div><br /><a href="{2}">See term in ScienceWISE</a></div>'
        definitions_xhtml = ''
        if 'property:definitions' in annotation:
            definitions = []
            for definition in annotation.getAllProperties('property:definitions'):
                url, _, title = definition.partition(' ')
                definitions.append(' &mdash; <a href="{0}">{1}</a>'.format(url, title))
            if len(definitions) > 0:
                definitions_xhtml += '<br/>External definitions:<br/>'
                definitions_xhtml += '<br/>'.join(definitions)
        xhtml = xhtml.format(annotation['property:term'], definitions_xhtml, annotation['property:webpageUrl'])
        return xhtml
