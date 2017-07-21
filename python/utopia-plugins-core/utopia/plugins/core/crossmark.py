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

#? name: CrossMark
#? www: http://www.crossref.org/crossmark/
#? urls: http://crossmark.crossref.org/


import utopia.tools.utils
import json
import random
import spineapi
import string
import utopia.document

import urllib2
from lxml import etree
from lxml.cssselect import CSSSelector


def select(node, selector):
    return CSSSelector(selector)(node)


def id_gen(size = 6, chars = string.ascii_lowercase + string.digits, initial_chars = string.ascii_lowercase):
    return random.choice(initial_chars) + ''.join(random.choice(chars) for x in range(size - 1))


class CrossMarkAnnotator(utopia.document.Annotator):
    '''Annotate the document with CrossMark information'''

    def on_ready_event(self, document):
        # See if there is any CrossMark information available for this document

        # Firstly find the document's DOI
        doi = utopia.tools.utils.metadata(document, 'identifiers[doi]')
        if doi is not None:

            # Then attempt to access CrossMark API
            try:
                url = 'http://crossmark.crossref.org/crossmark/?doi={0}'.format(doi)
                headers = {'Accept': 'application/json'}
                request = urllib2.Request(url, None, headers)
                cm = json.loads(urllib2.urlopen(request, timeout=8).read())
            # Not found
            except urllib2.HTTPError as e:
                if e.code == 404: # just ignore 404
                    return
                raise

            # If successful, create an annotation to be visualised
            annotation = spineapi.Annotation()
            annotation['concept'] = 'CrossMarkNotice'
            annotation['property:doi'] = doi
            annotation['property:name'] = 'CrossMark'
            annotation['property:description'] = 'Information on updates, corrections and retractions'
            annotation['property:sourceDatabase'] = 'crossmark'
            annotation['property:sourceDescription'] = '<div><a href="http://www.crossref.org/crossmark/">CrossMark</a> gives scholars the information they need to verify that they are using the most recent and reliable versions of a document.</div>'
            document.addAnnotation(annotation)


class CrossMarkVisualiser(utopia.document.Visualiser):
    '''Visualise the CrossMark information'''

    def visualisable(self, annotation):
        return annotation.get('concept') == 'CrossMarkNotice' and 'property:doi' in annotation

    def visualise(self, annotation):
        # Check for HTML in annotation
        if 'property:html' in annotation:
            return annotation['property:html']
        else:
            # Download the HTML dialog from CrossMark
            doi = annotation['property:doi']
            url = 'http://crossmark.crossref.org/dialog/?domain=http://utopiadocs.com/&doi={0}'.format(doi)
            html = urllib2.urlopen(url, timeout=8).read()

            # Parse out fragment of interest
            dom = etree.HTML(html)

            # Turn off hyphenation
            for p in dom.iter('p'):
                p.set('class', 'nohyphenate')

            # Modify icon
            icon = select(dom, '.cm-status-icon')
            if len(icon) > 0:
                icon = icon[0]
                icon.set('alt', icon.text)
                if len(select(icon, '.cm-status-icon-updates')) > 0:
                    icon.text = '!'
                elif len(select(icon, '.cm-status-icon-current')) > 0:
                    icon.text = u'\u2713'
                elif len(select(icon, '.cm-status-icon-unknown')) > 0:
                    icon.text = '?'

            # Get parts and compile HTML
            id = id_gen()
            html = '<div id="{0}">'.format(id)

            cm_status = select(dom, '.cm-status')
            if len(cm_status) > 0:
                html += etree.tostring(cm_status[0], pretty_print=True, method='html')

            cm_data = select(dom, '.cm-data')
            if len(cm_data) > 0:
                cm_data = cm_data[0]
                cm_correction = select(cm_data, '.cm-correction')
                if len(cm_correction) > 0:
                    cm_correction = cm_correction[0]
                    cm_data.remove(cm_correction)
                    html += etree.tostring(cm_correction, pretty_print=True, method='html')
                html += '<div class="box">'
                html += etree.tostring(cm_data, pretty_print=True, method='html')

                cm_record = select(dom, '.cm-record')
                if len(cm_record) > 0:
                    cm_record = cm_record[0]
                    cm_record.set('class', 'expandable ' + cm_record.get('class'))
                    cm_record.set('title', 'CrossMark Full Record')
                    html += etree.tostring(cm_record, pretty_print=True, method='html')

                html += '</div>'

            html += '</div>'

            #print html

            css = '''
            <style>
                #{0} .cm-status {{ text-align: center; margin: 0.5em 0 1em 0; }}
                #{0} .cm-status-icon {{ font-size: 4em; font-weight: bold; display: inline-block; padding: 0.2em; }}
                #{0} .cm-status-icon-updates {{ color: #00c; }}
                #{0} .cm-status-icon-current {{ color: #0c0; }}
                #{0} .cm-status-icon-unknown {{ color: #c00; }}
                #{0} .cm-correction, #{0} .cm-correction p {{ text-align: center !important; margin: 0 0 1em 0; }}
                #{0} .cm-record {{ margin-top: 2em; }}
                #{0} .cm-group {{ padding: 0.5em 0; border-bottom: 1px solid #aaa; }}
                #{0} .cm-group:last-child {{ border-bottom: none; }}
                #{0} .cm-field {{ padding-left: 1em; }}
                #{0} .cm-field p {{ margin: 0 0 0.5em 0; }}
                #{0} .cm-field:last-child p {{ margin: 0; }}
                #{0} .cm-field img {{ vertical-align: middle; }}
            </style>
            '''.format(id)

            # Cache HTML
            annotation['property:html'] = css + html

            return css, html
