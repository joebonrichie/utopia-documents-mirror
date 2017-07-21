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

import kend.model
import utopia.tools.converters.PageBox

def kend2spineapi(kExtent, document):
    """Convert a kend.model.Extent into a spineapi.TextExtent for a given document."""
    sExtent = None
    if document is not None and len(kExtent) > 0:
        (page1, _, (left, top), (_, bottom)) = utopia.tools.converters.PageBox.kend2spineapi(kExtent[0])
        x1 = left + 0.01
        y1 = (top + bottom) / 2.0
        (page2, _, (_, top), (right, bottom)) = utopia.tools.converters.PageBox.kend2spineapi(kExtent[-1])
        x2 = right - 0.01
        y2 = (top + bottom) / 2.0
        sExtent = document.resolveExtent(page1, x1, y1, page2, x2, y2)
    return sExtent

def spineapi2kend(sExtent):
    """Convert a spineapi.TextExtent into a kend.model.Extent."""
    kExtent = kend.model.Extent(sExtent.text())
    for sBox in sExtent.areas():
        kExtent.append(utopia.tools.converters.PageBox.spineapi2kend(sBox))
    return kExtent
