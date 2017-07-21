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

import logging
log = logging.getLogger(__name__)

###############################
# Version and Namespace stuff #
###############################

VERSION = '0.7'

KEND_NS = {
    'uri'   : 'http://utopia.cs.manchester.ac.uk/kend',
    'prefix': 'kend',
}

def _makeNamespace(prefix):
    return {'uri' : '%s/%s#' % (KEND_NS['uri'], prefix), 'prefix': prefix}

STRUCTURE_NS = _makeNamespace('structure')
RELATION_NS = _makeNamespace('relation')
PARAMETER_NS = _makeNamespace('parameter')
PROPERTY_NS  = _makeNamespace('property')
PROVENANCE_NS  = _makeNamespace('provenance')
SESSION_NS   = _makeNamespace('session')

NS         = dict(((ns['prefix'], ns['uri']) for ns in (PARAMETER_NS, PROPERTY_NS, SESSION_NS, PROVENANCE_NS, STRUCTURE_NS, KEND_NS)))
NS_INVERSE = dict(((ns['uri'], ns['prefix']) for ns in (PARAMETER_NS, PROPERTY_NS, SESSION_NS, PROVENANCE_NS, STRUCTURE_NS)))


##################
# KEND XSD Model #
##################

class PageBox(object):
    def __init__(self, page = 0, rotation = 0, left = 0.0, top = 0.0, right = 0.0, bottom = 0.0):
        self.page = int(page)
        self.rotation = int(rotation)
        self.left = float(left)
        self.top = float(top)
        self.right = float(right)
        self.bottom = float(bottom)

    def __iter__(self):
        return iter(( self.page, self.rotation,
                     (self.left, self.top),
                     (self.right, self.bottom) ))


class Extent(list):
    def __init__(self, text = None, pageboxes = None):
        list.__init__(self, pageboxes or [])
        self.text = text

    def __nonzero__(self):
        return True


class Evidence(object):
    def __init__(self, type = None, data = None, srctype = None, src = None):
        self.srctype = srctype and '%s' % srctype
        self.src = src and '%s' % src
        self.type = type and '%s' % type
        self.data = data and '%s' % data

    def __iter__(self):
        return iter(( self.type, self.data, self.srctype, self.src ))

    def __nonzero__(self):
        return True


class DocumentReference(object):
    __mime__ = 'documentref'
    def __init__(self, id = None, evidence = None):
        self.id = id
        self.evidence = evidence or []

    def __iter__(self):
        return iter(self.evidence)

    def __nonzero__(self):
        return True


class DocumentAnchor(DocumentReference):
    def __init__(self, id = None, evidence = None, extents = None, areas = None):
        DocumentReference.__init__(self, id, evidence)
        self.extents = extents or []
        self.areas = areas or []


class AnnotationAnchor(object):
    def __init__(self, id = None):
        self.id = id


class MediaSet(list):
    __mime__ = 'media-set'
    def __init__(self, media = None):
        list.__init__(self, media or [])

    def __nonzero__(self):
        return True


class Media(object):
    __mime__ = 'media-link'
    def __init__(self, id = None, name = None, src = None, type = None, data = None, revision = None, size = None, edit = None):
        self.id = id
        self.name = name
        self.src = src
        self.type = type
        self.data = data
        self.revision = revision
        self.size = size
        self.edit = edit

    def __iter__(self):
        return iter(( self.name, self.src, self.type, self.data ))


class Annotation(dict):
    __mime__ = 'annotation'
    def __init__(self, **kwargs):
        # Super class construction
        dict.__init__(self)

        # Defaults
        kwargs.setdefault('context', 'http://utopia.cs.manchester.ac.uk/contexts/public')
        kwargs.setdefault('status', 'unpublished')

        # Main attributes
        for attr in ('author', 'concept', 'created', 'updated', 'id', 'revision', 'edit', 'media_edit', 'status', 'context'):
            setattr(self, attr, kwargs.get(attr, None))

        self.anchors = kwargs.get('anchors', [])
        self.media = kwargs.get('media', [])

    def __nonzero__(self):
        return True


class Document(list):
    __mime__ = 'document'
    def __init__(self, id = None, equivalents = None, metadata = None, annotations = None, edit = None):
        list.__init__(self, annotations or [])
        self.id = id
        self.equivalents = equivalents or []
        self.metadata = metadata or []
        self.edit = edit

    def __nonzero__(self):
        return True


class Fault(object):
    __mime__ = 'fault'
    def __init__(self, code = 'UNKNOWN', string = 'Unknown error', source = None, details = None):
        self.code = code
        self.string = string
        self.source = source
        self.details = details

    def __iter__(self):
        return iter(( self.code, self.string, self.source, self.details ))


class Group(object):
    def __init__(self, input = None, ignored = None, output = None, annotations = None, documents = None, fault = None):
        self.annotations = annotations or []
        self.documents = documents or []
        self.input = input or []
        self.ignored = ignored or []
        self.output = output or []
        self.fault = fault

    def __nonzero__(self):
        return True


class Annotations(list):
    __mime__ = 'annotations'
    def __init__(self, groups = None, fault = None):
        list.__init__(self, groups or [])
        self.fault = fault

    def __nonzero__(self):
        return True


class Documents(list):
    __mime__ = 'documents'
    def __init__(self, groups = None, fault = None):
        list.__init__(self, groups or [])
        self.fault = fault

    def __nonzero__(self):
        return True


__classes__ = (Annotations,
               Annotation,
               Documents,
               Document,
               MediaSet,
               Media,
               DocumentReference)
