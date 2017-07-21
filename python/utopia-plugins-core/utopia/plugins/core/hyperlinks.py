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

import re
import spineapi
import utopia.document
import cgi
import itertools
import collections

class OutlineBuilder(utopia.document.Annotator):
    """Build outline navigator"""

    def after_ready_event(self, document):
        outline={}

        for a in document.annotations():
            if a.get('concept') == 'OutlineItem':
                outline[tuple([int(x) for x in a.get('property:outlinePosition').split('.')])]=a

        if len(outline):
#            html='<div><style media="screen" type="text/css">ul { list-style-type: none; }</style><ul>'
            html='<div><ul>'
            plen=1
            for item in (sorted(outline.keys())):

                if len(item) > plen:
                    html+='<ul><li>'
                elif len(item) < plen:
                    html+='</li></ul></li><li>'
                else:
                    html+='</li><li>'
                plen=len(item)

                html += '<a href="#" title="{0}" target="pdf; anchor={0}">{1}</a>'.format(outline[item].get('property:destinationAnchorName'), cgi.escape(outline[item].get('property:outlineTitle'), quote=True).encode('ascii', 'xmlcharrefreplace'),)

            html+="</ul></div>"
            a = spineapi.Annotation()
            a['concept'] = 'Collated'
            a['property:name'] = 'Outline'
            a['property:description'] = 'Document Structure'
            a['session:weight'] = '999'
            a['property:html'] = html
            document.addAnnotation(a)




class HyperlinkTooltipFactory(utopia.document.Annotator):
    """Create tooltips for known hyperlinks"""

    def on_filter_event(self, document, data = None):
        for annotation in document.annotations():
            if annotation.get('concept') in ['Hyperlink', 'WebPage'] and 'displayTooltip' not in annotation and 'property:webpageUrl' in annotation:
                if annotation['property:webpageUrl'].startswith("mailto:"):
                    annotation['displayTooltip'] = '<span>Email:</span><br/>&nbsp;&nbsp;&nbsp;<strong>%s</strong>' % annotation['property:webpageUrl'][7:]
                elif annotation['property:webpageUrl'].startswith("#"):
                    annotation['displayTooltip'] = '<span>Internal&nbsp;Link&nbsp;to:</span><br/>&nbsp;&nbsp;&nbsp;<strong>%s</strong>' % annotation['property:destinationAnchorName']
                else:
                    annotation['displayTooltip'] = '<span>Link&nbsp;to:</span><br/>&nbsp;&nbsp;&nbsp;<strong>%s</strong>' % annotation['property:webpageUrl']




def areas_intersect(list_a, list_b):
    for a in list_a:
        for b in list_b:
            a_page, _, (a_left, a_top), (a_right, a_bottom) = a
            b_page, _, (b_left, b_top), (b_right, b_bottom) = b
            if (a_page == b_page) and (a_left <= b_right) and (b_left <= a_right) and (a_top <= b_bottom) and (b_top <= a_bottom):
                return True
    return False


class HREFFactory(utopia.document.Annotator):
    """Create annotations for links and email addresses"""

    def __init__(self):
        self.atext = '[a-zA-Z0-9%s]' % re.escape("!#$%&'*+-/=?^_`{|}~")
        self.dot_atom = r'%s+(\.%s+)*' % (self.atext, self.atext)
        self.email = r'%s@%s' % (self.dot_atom, self.dot_atom)
        self.http = r'''\b((?:https?://|www\d{0,3}[.]|[a-z0-9.\-]+[.][a-z]{2,4}/)(?:[^\s()<>]+|\(([^\s()<>]+|(\([^\s()<>]+\)))*\))+(?:\(([^\s()<>]+|(\([^\s()<>]+\)))*\)|[^\s`!()\[\]{};:'".,<>?]))'''

    def before_load_event(self, document):
        # Get existing areas
        existing_hyperlinks = [ann for ann in document.annotations() if ann.get('concept') == 'Hyperlink']
        existing_extent_areas = [area for ann in existing_hyperlinks for ext in ann.extents() for area in ext.areas()]
        existing_areas = existing_extent_areas + [area for ann in existing_hyperlinks for area in ann.areas()]
        self.existing_areas = existing_areas

    @utopia.document.buffer
    def on_load_event(self, document):
        # Email links
        for match in document.search(self.email, spineapi.IgnoreCase + spineapi.WholeWordsOnly + spineapi.RegExp):
            if not areas_intersect(match.areas(), self.existing_areas):
                annotation = spineapi.Annotation()
                annotation['concept'] = 'Hyperlink'
                annotation['property:webpageUrl'] = 'mailto:%s' % match.text()
                annotation['session:volatile'] = '1'
                annotation.addExtent(match)
                document.addAnnotation(annotation)
            else:
                print('ignoring clashing email link text:', match.text().encode('utf8'))
        # HTTP(S) links
        for match in document.search(self.http, spineapi.IgnoreCase + spineapi.WholeWordsOnly + spineapi.RegExp):
            if not areas_intersect(match.areas(), self.existing_areas):
                if match.begin().lineArea()[1] == 0: # Only while vertical links are rendered wrongly FIXME
                    url = match.text()
                    if not url.startswith('http'):
                        url = 'http://' + url
                    annotation = spineapi.Annotation()
                    annotation['concept'] = 'Hyperlink'
                    annotation['property:webpageUrl'] = '%s' % url
                    annotation['session:volatile'] = '1'
                    annotation.addExtent(match)
                    document.addAnnotation(annotation)
            else:
                print('ignoring clashing http link text:', match.text().encode('utf8'))
