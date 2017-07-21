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
import utopia.tools.converters.Extent
import utopia.tools.converters.PageBox
import spineapi
import urllib
import urlparse
import os
import re
import base64
import mimetypes
import datetime

mimetypes.init()

def _format(value):
    if isinstance(value, datetime.datetime):
        value = value.strftime('%Y-%m-%dT%H:%M:%S')
    elif isinstance(value, datetime.date):
        value = value.strftime('%Y-%m-%d')
    elif isinstance(value, datetime.time):
        value = value.strftime('%H:%M:%S')
    return unicode(value)

def kend2spineapi(kAnnotation, document = None):
    """Convert a kend.model.Annotation into a spineapi.Annotation for a given document."""
    sAnnotation = spineapi.Annotation()
    if document is not None:
        for anchor in kAnnotation.anchors:
            if isinstance(anchor, kend.model.AnnotationAnchor):
                sAnnotation.setProperty('parent', anchor.id)
            elif isinstance(anchor, kend.model.DocumentAnchor):
                for kExtent in anchor.extents:
                    sAnnotation.addExtent(utopia.tools.converters.Extent.kend2spineapi(kExtent, document))
                for kBox in anchor.areas:
                    sAnnotation.addArea(utopia.tools.converters.PageBox.kend2spineapi(kBox))
    for k, vs in kAnnotation.iteritems():
        if len(k) > 0 and k[0] == '{' and not k.startswith('{%s}' % kend.model.KEND_NS['uri']):
            ns = k[1:k.find("}")]
            if ns in kend.model.NS_INVERSE:
                prefix = kend.model.NS_INVERSE[ns]
                suffix = k[len(ns)+2:]
                for v in vs:
                    sAnnotation.insertProperty('%s:%s' % (prefix, suffix), v)
                if prefix == kend.model.RELATION_NS['prefix'] and suffix == 'response_to':
                    sAnnotation.setProperty('parent', vs[0])
            else:
                for v in vs:
                    sAnnotation.insertProperty(k, v)
    for media in kAnnotation.media:
        mediaDict = {}
        for k in ['name', 'src', 'type', 'revision', 'size', 'edit']:
            if getattr(media, k, None) is not None:
                mediaDict[k] = getattr(media, k)
        sAnnotation.insertProperty('session:media', urllib.urlencode(mediaDict))
    for k in ['author', 'concept', 'created', 'updated', 'id', 'revision', 'edit', 'media_edit', 'status', 'context']:
        v = getattr(kAnnotation, k)
        if v is not None:
            sAnnotation.setProperty(k, _format(v))
    sAnnotation.setPublic(kAnnotation.status == 'published')
    return sAnnotation

def spineapi2kend(sAnnotation, document_id):
    """Convert a spineapi.Annotation into a kend.model.Annotation."""
    kAnnotation = kend.model.Annotation()
    sExtents = sAnnotation.extents()
    sBoxes = sAnnotation.areas()
    if len(sExtents) + len(sBoxes) > 0:
        kDocumentAnchor = kend.model.DocumentAnchor(id = document_id)
        for sExtent in sExtents:
            kDocumentAnchor.extents.append(utopia.tools.converters.Extent.spineapi2kend(sExtent))
        for sBox in sBoxes:
            kDocumentAnchor.areas.append(utopia.tools.converters.PageBox.spineapi2kend(sBox))
        kAnnotation.anchors.append(kDocumentAnchor)
    for k, v in sAnnotation.iteritems():
        if k in ['author', 'concept', 'id', 'edit', 'media_edit', 'status', 'context'] and len(v) > 0:
            setattr(kAnnotation, k, v[0])
        elif k == 'revision':
            try:
                setattr(kAnnotation, k, int(v[0]))
            except:
                pass
        elif k in ['created', 'updated'] and len(v) > 0:
            try:
                v = datetime.datetime.strptime(v[0][:19], '%Y-%m-%dT%H:%M:%S')
                setattr(kAnnotation, k, v)
            except:
                pass
        elif k == 'parent' and len(v) > 0:
            kAnnotation.anchors.append(kend.model.AnnotationAnchor(id = v[0]))
            kAnnotation['{%s}%s' % (kend.model.RELATION_NS['uri'], 'response_to')] = v[0]
        elif len(k) > 0 and not k.startswith('{%s}' % kend.model.KEND_NS['uri']):
            prefix = k[0:k.find(":")]
            if prefix == 'session':
                suffix = k[len(prefix)+1:]
                print prefix, suffix, v
                if suffix == 'upload_files':
                    for url in v:
                        media = None
                        spliturl = urlparse.urlsplit(url)
                        if spliturl.scheme in ('', 'file'):
                            filename = spliturl.path
                            if os.name == 'nt':
                                filename = filename.lstrip('/')
                            if os.path.isfile(filename):
                                file = open(filename, 'r')
                                data = base64.b64encode(file.read())
                                file.close()
                                if len(data) > 0:
                                    media = kend.model.Media(type = mimetypes.guess_type(filename)[0],
                                                             data = data,
                                                             name = os.path.basename(filename))
                        elif spliturl.scheme == 'data':
                            match = re.match(r'data\:(\w+/\w+)(?:;name=([^;,]+))?(?:;(base64))?,(.*)', url)
                            if match is not None:
                                mime_type, name, isBase64, data = match.groups()
                                media = kend.model.Media(type = mime_type,
                                                         data = data,
                                                         name = name)
                        else:
                            media = kend.model.Media(type = mimetypes.guess_type(spliturl.path)[0],
                                                     src = url,
                                                     name = os.path.basename(spliturl.path))
                        if media is not None:
                            kAnnotation.media.append(media)
            elif prefix in kend.model.NS:
                suffix = k[len(prefix)+1:]
                kAnnotation['{%s}%s' % (kend.model.NS[prefix], suffix)] = v
            elif k[0] == '{':
                kAnnotation[k] = v
    if sAnnotation.isPublic():
        kAnnotation.status = 'published'
    else:
        kAnnotation.status = 'unpublished'
    found = False
    for anchor in kAnnotation.anchors:
        if isinstance(anchor, kend.model.DocumentAnchor):
            found = True
            break
    if not found:
        kAnnotation.anchors.append(kend.model.DocumentAnchor(id = document_id))
    return kAnnotation
