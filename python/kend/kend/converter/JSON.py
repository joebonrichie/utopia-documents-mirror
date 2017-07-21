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

from kend.converter.base import *
import kend.model
import json


MIME_TYPE = '%s+json' % MIME_TYPE_BASE


def _apply(src, tgt, attrs):
    for k in attrs:
        v = getattr(src, k, None)
        if v is not None:
            tgt[k] = v

def _unapply(src, tgt, attrs):
    for k, v in tgt.iteritems():
        if k in attrs:
            setattr(tgt, k, v)


class _KendEncoder(json.JSONEncoder):
    def default(self, obj):
        json_obj = {}
        if isinstance(obj, kend.model.PageBox):
            _apply(obj, json_obj, ('page', 'rotation', 'left', 'top', 'right', 'bottom'))
        elif isinstance(obj, kend.model.Extent):
            json_obj.update({
                'text': obj.text,
                'bounds': [self.default(pagebox) for pagebox in obj],
            })
        elif isinstance(obj, kend.model.Evidence):
            _apply(obj, json_obj, ('type', 'data', 'srctype', 'src'))
        elif isinstance(obj, kend.model.DocumentReference):
            if obj.id is not None:
                json_obj['id'] = obj.id
            else:
                json_obj['evidence'] = [self.default(evidence) for evidence in obj.evidence]
            if isinstance(obj, kend.model.DocumentAnchor):
                json_obj.update({
                    'extents': [self.default(extent) for extent in obj.extents],
                    'areas': [self.default(area) for area in obj.areas],
                })
        elif isinstance(obj, kend.model.AnnotationAnchor):
            json_obj['id'] = obj.id
        elif isinstance(obj, kend.model.Media):
            _apply(obj, json_obj, ('type', 'name', 'src', 'data'))
        elif isinstance(obj, kend.model.Annotation):
            _apply(obj, json_obj, ('author', 'concept', 'created', 'updated', 'id', 'revision', 'edit', 'status', 'context'))
            json_obj.update({
                'properties': dict(obj),
                'anchors': [self.default(anchor) for anchor in obj.anchors],
                'media': [self.default(media) for media in obj.media],
            })
        elif isinstance(obj, kend.model.Fault):
            _apply(obj, json_obj, ('code', 'string', 'source', 'details'))
        elif isinstance(obj, kend.model.Group):
            if obj.fault is not None:
                json_obj['fault'] = self.default(obj.fault)
            else:
                json_obj.update({
                    'input': [self.default(parameter) for parameter in obj.input],
                    'ignored': [self.default(parameter) for parameter in obj.ignored],
                    'output': [self.default(parameter) for parameter in obj.output],
                    'annotations': [self.default(annotation) for annotation in obj],
                })
        elif isinstance(obj, kend.model.Annotations):
            if obj.fault is not None:
                json_obj['fault'] = self.default(obj.fault)
            else:
                json_obj['groups'] = [self.default(group) for group in obj]

        if len(json_obj) > 0:
            json_obj['__kend__'] = type(obj).__name__
            return json_obj
        return json.JSONEncoder.default(self, obj)


def _KendDecoder(json_obj):
    if '__kend__' in json_obj:
        cls = getattr(kend.model, json_obj['__kend__'])
        obj = cls()
        if cls is kend.model.PageBox:
            _apply(obj, json_obj, ('page', 'rotation', 'left', 'top', 'right', 'bottom'))
        elif cls is kend.model.Extent:
            json_obj.update({
                'text': obj.text,
                'bounds': [self.default(pagebox) for pagebox in obj],
            })
        elif cls is kend.model.Evidence:
            _apply(obj, json_obj, ('type', 'evidence', 'srctype', 'src'))
        elif cls is kend.model.DocumentReference:
            if obj.id is not None:
                json_obj['id'] = obj.id
            else:
                json_obj['evidence'] = [self.default(evidence) for evidence in obj.evidence]
        elif cls is kend.model.DocumentAnchor:
            json_obj.update({
                'extents': [self.default(extent) for extent in obj.extents],
                'areas': [self.default(area) for area in obj.areas],
            })
        elif cls is kend.model.AnnotationAnchor:
            json_obj['id'] = obj.id
        elif cls is kend.model.Media:
            _apply(obj, json_obj, ('type', 'name', 'src', 'data'))
        elif cls is kend.model.Annotation:
            _apply(obj, json_obj, ('author', 'concept', 'created', 'updated', 'id', 'revision', 'edit', 'status', 'context'))
            json_obj.update({
                'properties': dict(obj),
                'anchors': [self.default(anchor) for anchor in obj.anchors],
                'media': [self.default(media) for media in obj.media],
            })
        elif cls is kend.model.Fault:
            _apply(obj, json_obj, ('code', 'string', 'source', 'details'))
        elif cls is kend.model.Group:
            if obj.fault is not None:
                json_obj['fault'] = self.default(obj.fault)
            else:
                json_obj.update({
                    'parameters': [self.default(parameter) for parameter in obj.parameters],
                    'annotations': [self.default(annotation) for annotation in obj],
                })
        elif cls is kend.model.Annotations:
            if obj.fault is not None:
                json_obj['fault'] = self.default(obj.fault)
            else:
                json_obj['groups'] = [self.default(group) for group in obj]

        if len(json_obj) > 0:
            json_obj['__kend__'] = type(obj).__name__
            return json_obj
    return json_obj


def parse(obj, cls):
    return json.load(obj, cls=_KendDecoder)

def serialise(obj, cls = None):
    return json.dumps(obj, cls=_KendEncoder, separators=(',', ':'))

