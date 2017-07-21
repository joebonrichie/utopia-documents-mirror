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

import spineapi
import utopia.document


class DemoLogoOverlayFilter(utopia.document.Annotator):
    @utopia.document.buffer
    def on_filter_event(self, document, data = None):
        for annotation in document.annotations():
            if annotation.get('concept') != 'DemoLogoOverlay' and annotation.get('property:demo_logo') == '1':
                annotation.removePropertyAll('property:demo_logo')

                overlay = spineapi.Annotation()
                overlay['concept'] = 'DemoLogoOverlay'
                overlay['property:demo_logo'] = '1'
                overlay.addExtents(annotation.extents())
                overlay.addAreas(annotation.areas())
                document.addAnnotation(overlay)



class DemoLogoOverlayRendererMapper(utopia.document.OverlayRendererMapper):
    def mapToId(self, document, annotation):
        if annotation.get('concept') == 'DemoLogoOverlay':
            return 'demo_logo'
    def weight(self):
        return 10


