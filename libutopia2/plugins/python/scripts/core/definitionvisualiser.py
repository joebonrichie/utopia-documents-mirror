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

import utopia.document

class _DefinitionVisualiser(utopia.document.Visualiser):
    """Visualiser for Definition and DatabaseEntry"""

    def visualisable(self, annotation):
        return annotation.get('concept') in ('Definition', 'DatabaseEntry')

    def visualise(self, annotation):
        import kend.converter
        rt = ''

        if annotation.get('concept') in ('Definition', 'DatabaseEntry'):
            # short cut
            if 'property:xhtml' in annotation:
                return '<div>%s</div>' % annotation['property:xhtml']

            if 'property:name' in annotation:
                rt = '<strong>%s</strong><br />' % annotation['property:name']

            description = annotation.get('property:description')
            if description is not None:
                rt += '%s' % description #[0:800]
#                if len(description) > 800:
#                    rt += '...'

            if 'property:molecularDescription' in annotation:
                rt += '<br /><br />%s' % annotation['property:molecularDescription'][0:200]
                if len(annotation['property:molecularDescription']) > 200:
                    rt += '...'

            if 'property:imageUrl' in annotation:
                rt += '<center><img src="%s" width="200" heighth="200" /></center>' % annotation['property:imageUrl']

            if 'property:webpageUrl' in annotation:
                source = ''
                href = annotation['property:webpageUrl']
                target = ''
                source=annotation.get('property:sourceDatabase')
                if source=='pdb':
                    source='RCSB'
                if source in ('wikipedia',):
                    target = 'target="sidebar"'
                    href = href.replace('wikipedia.org', 'm.wikipedia.org')
                rt += '<br /><br /><a href="%s" style="text-decoration:none" %s>View ' % (href, target)
                rt += '%s web page...</a>' % source

            # Just for Portland's benefit before we get source images in place
            if annotation.get('property:sourceDatabase') == 'glick':
                rt += '<br/><em>(Glick Glossary/Portland Press Ltd.)</em>'

        #print rt
        return rt



class DefinitionTooltipFactory(utopia.document.Annotator):
    """Create tooltips for known definitions"""

    def on_filter_event(self, document, data = None):
        for annotation in document.annotations():
            if annotation.get('concept') == 'Definition' and 'displayTooltip' not in annotation and 'property:name' in annotation and 'property:description' in annotation:
                annotation['displayTooltip'] = '<strong>%s</strong>: %s' % (annotation['property:name'], annotation['property:description'])



