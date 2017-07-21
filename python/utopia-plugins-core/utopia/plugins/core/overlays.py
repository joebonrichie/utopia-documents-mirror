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

#########################################################################################
##
##  Standard overlay mappers for Utopia Documents' core plugins
##
##

import utopia.document


# Allow an annotation to specify its own preferred overlay renderer
class CustomOverlayRendererMapper(utopia.document.OverlayRendererMapper):
    def mapToId(self, document, annotation):
        return annotation.get('session:overlay')
    def weight(self):
        return 10000


# Embedded annotations should be ignored by the overlay system
class EmbeddedOverlayRendererMapper(utopia.document.OverlayRendererMapper):
    def mapToId(self, document, annotation):
        if annotation.get('property:embedded') == '1':
            return 'none'
    def weight(self):
        return 1000


# Anchor annotations should be ignored by the overlay system (for now) FIXME
class AnchorOverlayRendererMapper(utopia.document.OverlayRendererMapper):
    def mapToId(self, document, annotation):
        if annotation.get('concept') == 'Anchor':
            return 'none'
    def weight(self):
        return 1000


# Tables
class TablingOverlayRendererMapper(utopia.document.OverlayRendererMapper):
    def mapToId(self, document, annotation):
        if annotation.get('concept') in ('Table', 'ElsevierTable'):
            return 'table'
    def weight(self):
        return 10


# Comments
class CommentingOverlayRendererMapper(utopia.document.OverlayRendererMapper):
    def mapToId(self, document, annotation):
        if annotation.get('concept') in ('UserComment',):
            return 'comment'
    def weight(self):
        return 10


# Citations
class CitationOverlayRendererMapper(utopia.document.OverlayRendererMapper):
    def mapToId(self, document, annotation):
        if annotation.get('concept') in ('Citation', 'ForwardCitation'):
            return 'hyperlink'
    def weight(self):
        return 10


# Hyperlink
class HyperlinkOverlayRendererMapper(utopia.document.OverlayRendererMapper):
    def mapToId(self, document, annotation):
        if annotation.get('concept') in ('Hyperlink', 'WebPage'):
            return 'hyperlink'
    def weight(self):
        return 10


# Highlights
class HighlightingOverlayRendererMapper(utopia.document.OverlayRendererMapper):
    def mapToId(self, document, annotation):
        if annotation.get('concept') in ('Highlight',):
            return 'highlight'
    def weight(self):
        return 10
