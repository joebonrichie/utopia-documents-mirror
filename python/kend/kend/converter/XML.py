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

import collections
import datetime
import kend.model
from kend.converter.base import *
from lxml import etree


import logging
log = logging.getLogger(__name__)

###############################
# Version and Namespace stuff #
###############################

MIME_TYPE = '%s+xml' % MIME_TYPE_BASE

NS = kend.model.NS.copy()
NS.update({'xsi': 'http://www.w3.org/2001/XMLSchema-instance'})

NS_SERIALISE = NS.copy()
del NS_SERIALISE['kend']
NS_SERIALISE[None] = kend.model.NS['kend']


def _extractProperties(anode, target = None):
    """Extract qualified key / value properties from an annotation element"""

    tmp = {}

    if target is None:
        target = {}

    # Currently will only see the *LAST* type entry due to key uniqueness
    for pnode in anode.xpath('*'):
        if pnode.text is not None:
            if pnode.tag not in tmp:
                tmp[pnode.tag] = []
            tmp[pnode.tag].append(pnode.text)

    for k, vs in tmp.iteritems():
        target[k] = vs

    return target

def _resolvePrefix(str):
    if not str.startswith('{'):
        prefix, colon, suffix = str.partition(':')
        if colon == ':':
            if prefix in NS_SERIALISE:
                str = '{%s}%s' % (NS_SERIALISE[prefix], suffix)
            else:
                return None
    return str

def _injectProperties(source, target = None):
    """Inject qualified key / value properties into a properties element"""

    if target is None:
        target = etree.Element('{%s}properties' % NS['kend'])

    for key, value in source.iteritems():
        key = _resolvePrefix(key)
        if key is not None:
            if isinstance(value, (str, unicode)):
                property = etree.Element(key)
                property.text = value
                target.append(property)
            elif isinstance(value, collections.Iterable):
                for i in value:
                    property = etree.Element(key)
                    property.text = i
                    target.append(property)

    return target

# FIXME this should only ever raise a kend conversion error, but at the moment it could raise others
def _reduceToElement(object):
    """Given some arbitrary input, convert it to an lxml Element node"""
    if not isinstance(object, etree._Element):
        if not isinstance(object, etree._ElementTree):
            try:
                if isinstance(object, (str, unicode)):
                    try:
                        try:
                            object = etree.parse(open(object))
                        except IOError:
                            object = etree.fromstring(object)
                    except etree.XMLSyntaxError as e:
                        log.error('XML parse error: %s, %s' % (repr(type(object)), repr(e.error_log.last_error)))
                        raise ParseError('XML parse error: %s, %s' % (repr(type(object)), repr(e.error_log.last_error)))
                else:
                    object = etree.parse(object)
            except VersionError as e:
                log.error('XML format version error: %s' % repr(e))
                raise
        if isinstance(object, etree._ElementTree):
            object = object.getroot()
    return object

def _formatattr(value):
    return value.replace('_', '-')

def _format(value):
    if isinstance(value, datetime.datetime):
        value = value.strftime('%Y-%m-%dT%H:%M:%S')
    elif isinstance(value, datetime.date):
        value = value.strftime('%Y-%m-%d')
    elif isinstance(value, datetime.time):
        value = value.strftime('%H:%M:%S')
    return unicode(value)

def _finaliseXML(xml, schema):
    xml.set('version', kend.model.VERSION)
    xml.set('{%s}schemaLocation' % NS['xsi'], '%s https://utopia.cs.manchester.ac.uk/kend/%s/xsd/%s' % (NS['kend'], kend.model.VERSION, schema))
    etree.cleanup_namespaces(xml)
    return xml


class _XMLConverter(BaseConverter):
    @classmethod
    def parse(cls, input):
        found_version = input.get('version', None)
        if found_version is not None and found_version != kend.model.VERSION:
            raise VersionError("Expected format version '%s', but found version '%s'" % (kend.model.VERSION, found_version))
        return cls._parse(input) # Ensure input is an ElementTree object

class _PageBox:
    @staticmethod
    def _parse(input):
        return kend.model.PageBox(page     = input.get('page'),
                                  rotation = input.get('rotation', 0),
                                  left     = input.get('left'),
                                  top      = input.get('top'),
                                  right    = input.get('right'),
                                  bottom   = input.get('bottom'))

    @staticmethod
    def _serialise(obj, name = 'area', nsmap = {}):
        nsmap.update({None: NS['kend']})
        pagebox = etree.Element('{%s}%s' % (NS['kend'], name), nsmap = nsmap)
        pagebox.set('page', str(obj.page))
        pagebox.set('rotation', str(obj.rotation))
        pagebox.set('left', '{:.3f}'.format(obj.left))
        pagebox.set('top', '{:.3f}'.format(obj.top))
        pagebox.set('right', '{:.3f}'.format(obj.right))
        pagebox.set('bottom', '{:.3f}'.format(obj.bottom))
        return pagebox


class _Extent:
    @staticmethod
    def _parse(input):
        text = input.findtext('{%s}text' % NS['kend'])
        bounds = [_PageBox._parse(node) for node in input.xpath('kend:bounds', namespaces = NS)]
        return kend.model.Extent(text, bounds)

    @staticmethod
    def _serialise(obj, nsmap = {}):
        nsmap.update({None: NS['kend']})
        extent = etree.Element('{%s}extent' % NS['kend'], nsmap = nsmap)
        if obj.text is not None:
            text = etree.Element('{%s}text' % NS['kend'], nsmap = nsmap)
            text.text = obj.text
            extent.append(text)
        for bound in obj:
            extent.append(_PageBox._serialise(bound, 'bounds', nsmap))
        return extent


class _Evidence:
    @staticmethod
    def _parse(input):
        return kend.model.Evidence(type = input.get('type', None),
                                   data = input.text,
                                   srctype = input.get('srctype', None),
                                   src = input.get('src', None))

    @staticmethod
    def _serialise(obj, nsmap = {}):
        nsmap.update({None: NS['kend']})
        evidence = etree.Element('{%s}evidence' % NS['kend'], nsmap = nsmap)
        if obj.srctype is not None:
            evidence.set('srctype', str(obj.srctype))
        if obj.src is not None:
            evidence.set('src', str(obj.src))
        if obj.type is not None:
            evidence.set('type', str(obj.type))
        if obj.data is not None:
            evidence.text = obj.data
        return evidence


class _DocumentReference(_XMLConverter):
    @staticmethod
    def _parse(input):
        id = input.get('id', None)
        evidence = [_Evidence._parse(node) for node in input.xpath('kend:evidence', namespaces = kend.model.NS)]
        return kend.model.DocumentReference(id, evidence)

    @staticmethod
    def _serialise(obj, nsmap = {}):
        nsmap.update({None: NS['kend']})
        document = etree.Element('{%s}documentref' % NS['kend'], nsmap = nsmap)
        if obj.id is not None:
            document.set('{%s}type' % NS['xsi'], 'IdentifiedDocumentReference')
            document.set('id', obj.id)
        else:
            document.set('{%s}type' % NS['xsi'], 'UnidentifiedDocumentReference')
            for evidence in obj.evidence:
                document.append(_Evidence._serialise(evidence, nsmap))
        return document

    @staticmethod
    def serialise(obj):
        obj = _DocumentReference._serialise(obj, nsmap = NS_SERIALISE)
        _finaliseXML(obj, 'documents')
        return obj, '%s;version=%s;type=%s' % (MIME_TYPE, kend.model.VERSION, 'documentref')


class _DocumentAnchor(_XMLConverter):
    @staticmethod
    def _parse(input):
        id = input.get('id', None)
        evidence = [_Evidence._parse(node) for node in input.xpath('kend:evidence', namespaces = kend.model.NS)]
        extents = [_Extent._parse(node) for node in input.xpath('kend:extent', namespaces = kend.model.NS)]
        areas = [_PageBox._parse(node) for node in input.xpath('kend:area', namespaces = kend.model.NS)]
        return kend.model.DocumentAnchor(id, evidence, extents, areas)

    @staticmethod
    def _serialise(obj, nsmap = {}):
        nsmap.update({None: NS['kend']})
        document = etree.Element('{%s}document' % NS['kend'], nsmap = nsmap)
        if obj.id is not None:
            document.set('id', obj.id)
        else:
            for evidence in obj.evidence:
                document.append(_Evidence._serialise(evidence, nsmap))
        for extent in obj.extents:
            document.append(_Extent._serialise(extent, nsmap))
        for area in obj.areas:
            document.append(_PageBox._serialise(area, 'area', nsmap))
        return document


class _AnnotationAnchor(_XMLConverter):
    @staticmethod
    def _parse(input):
        id = input.get('id', None)
        return kend.model.AnnotationAnchor(id)

    @staticmethod
    def _serialise(obj, nsmap = {}):
        nsmap.update({None: NS['kend']})
        annotation = etree.Element('{%s}annotation' % NS['kend'], nsmap = nsmap)
        if obj.id is not None:
            annotation.set('id', obj.id)
        return annotation


class _Media(_XMLConverter):
    @staticmethod
    def _parse(input):
        try:
            revision = int(input.get('revision'))
        except:
            revision = None
        try:
            size = int(input.get('size'))
        except:
            size = None
        return kend.model.Media(id = input.get('id', None),
                                name = input.get('name', None),
                                src = input.get('src', None),
                                type = input.get('type', None),
                                data = input.text,
                                revision = revision,
                                size = size,
                                edit = input.get('edit', None))

    @staticmethod
    def _serialise(obj, nsmap = {}):
        nsmap.update({None: NS['kend']})
        media = etree.Element('{%s}media' % NS['kend'], nsmap = nsmap)
        if obj.id is not None:
            media.set('id', str(obj.id))
        if obj.name is not None:
            media.set('name', str(obj.name))
        if obj.src is not None:
            media.set('src', str(obj.src))
        if obj.edit is not None:
            media.set('edit', str(obj.edit))
        if obj.revision is not None:
            media.set('revision', str(obj.revision))
        if obj.size is not None:
            media.set('size', str(obj.size))
        if obj.type is not None:
            media.set('type', str(obj.type))
        if obj.data is not None:
            media.text = obj.data
        return media

    @staticmethod
    def serialise(obj):
        obj = _Media._serialise(obj, nsmap = NS_SERIALISE)
        _finaliseXML(obj, schema = 'media')
        return obj, '%s;version=%s;type=%s' % (MIME_TYPE, kend.model.VERSION, 'media-link')


class _MediaSet(_XMLConverter):
    @staticmethod
    def _parse(input):
        media = [_Media._parse(node) for node in input.xpath('kend:media', namespaces = kend.model.NS)]
        return kend.model.MediaSet(media)

    @staticmethod
    def _serialise(obj, nsmap = {}):
        nsmap.update({None: NS['kend']})
        media = etree.Element('{%s}media-set' % NS['kend'], nsmap = nsmap)
        if len(obj) > 0:
            for link in obj:
                media.append(_Media._serialise(link, nsmap))
        return media

    @staticmethod
    def serialise(obj):
        obj = _MediaSet._serialise(obj, nsmap = NS_SERIALISE)
        _finaliseXML(obj, schema = 'media')
        return obj, '%s;version=%s;type=%s' % (MIME_TYPE, kend.model.VERSION, 'media-set')


class _Annotation(_XMLConverter):
    @staticmethod
    def _parse(input):
        kwargs = {}
        for attr in ['author', 'concept', 'id', 'revision', 'edit', 'media_edit', 'status', 'context']:
            kwargs[attr] = input.attrib.get(_formatattr(attr), None)
        for attr in ['created', 'updated']:
            if input.attrib.get(attr) is not None:
                try:
                    kwargs[attr] = datetime.datetime.strptime(input.attrib.get(attr)[:19], '%Y-%m-%dT%H:%M:%S')
                except:
                    pass # Ignore unparsable datetime fields

        # Annotation properties
        properties = {}
        pnode = input.find('{%s}properties' % NS['kend'])
        if pnode is not None:
            properties.update(_extractProperties(pnode))

        anchors = [_DocumentAnchor._parse(node) for node in input.xpath('kend:anchor/kend:document', namespaces = kend.model.NS)]
        anchors += [_AnnotationAnchor._parse(node) for node in input.xpath('kend:anchor/kend:annotation', namespaces = kend.model.NS)]
        media = [_Media._parse(node) for node in input.xpath('kend:media', namespaces = kend.model.NS)]

        annotation = kend.model.Annotation(**kwargs)
        annotation.anchors = anchors
        annotation.media = media
        annotation.update(properties)
        return annotation

    @staticmethod
    def _serialise(obj, nsmap = {}):
        nsmap.update({None: NS['kend']})
        annotation = etree.Element('{%s}annotation' % NS['kend'], nsmap = nsmap)
        for attr in ['author', 'concept', 'created', 'updated', 'id', 'revision', 'edit', 'media_edit', 'status', 'context']:
            if getattr(obj, attr) is not None:
                annotation.set(_formatattr(attr), _format(getattr(obj, attr)))
        if len(obj) > 0:
            annotation.append(_injectProperties(obj, etree.Element('{%s}properties' % NS['kend'], nsmap = nsmap)))
        for media in obj.media:
            annotation.append(_Media._serialise(media, nsmap))
        if len(obj.anchors) > 0:
            anchors = etree.Element('{%s}anchor' % NS['kend'], nsmap = nsmap)
            for anchor in obj.anchors:
                if isinstance(anchor, kend.model.DocumentAnchor):
                    anchors.append(_DocumentAnchor._serialise(anchor, nsmap))
                elif isinstance(anchor, kend.model.AnnotationAnchor):
                    anchors.append(_AnnotationAnchor._serialise(anchor, nsmap))
            annotation.append(anchors)
        return annotation

    @staticmethod
    def serialise(obj):
        obj = _Annotation._serialise(obj, nsmap = NS_SERIALISE)
        _finaliseXML(obj, schema = 'annotation')
        return obj, '%s;version=%s;type=%s' % (MIME_TYPE, kend.model.VERSION, 'annotation')


class _Document(_XMLConverter):
    @staticmethod
    def _parse(input):
        edit = input.attrib.get('edit', None)
        annotations = [_Annotation._parse(node) for node in input.xpath('kend:annotations/kend:annotation', namespaces = kend.model.NS)]
        equivalents = [_DocumentReference._parse(node) for node in input.xpath('kend:equivalents/kend:document', namespaces = kend.model.NS)]
        metadata = [_Evidence._parse(node) for node in input.xpath('kend:meta', namespaces = kend.model.NS)]
        return kend.model.Document(id = input.get('id'), equivalents = equivalents, metadata = metadata, annotations = annotations, edit = edit)

    @staticmethod
    def _serialise(obj, nsmap = {}):
        nsmap.update({None: NS['kend']})
        doc = etree.Element('{%s}document' % NS['kend'], nsmap = nsmap)
        if obj.id is not None:
            doc.set('id', obj.id)
        if obj.edit is not None:
            doc.set('edit', obj.edit)
        if len(obj) > 0:
            annotations = etree.Element('{%s}annotations' % NS['kend'], nsmap = nsmap)
            doc.append(annotations)
            for annotation in obj:
                annotations.append(_Annotation._serialise(annotation, nsmap))
        if len(obj.equivalents) > 0:
            equivalents = etree.Element('{%s}equivalents' % NS['kend'], nsmap = nsmap)
            doc.append(equivalents)
            for equivalent_document in obj.equivalents:
                equivalents.append(_DocumentReference._serialise(equivalent_document, nsmap))
        for evidence in obj.metadata:
            metadata = _Evidence._serialise(evidence, nsmap)
            metadata.tag = '{%s}meta' % NS['kend']
            doc.append(metadata)
        return doc

    @staticmethod
    def serialise(obj):
        obj = _Document._serialise(obj, nsmap = NS_SERIALISE)
        _finaliseXML(obj, schema = 'document')
        return obj, '%s;version=%s;type=%s' % (MIME_TYPE, kend.model.VERSION, 'document')


class _Fault(_XMLConverter):
    @staticmethod
    def _parse(input):
        kwargs = {}
        for attr in ['code', 'string', 'source']:
            value = input.findtext('{%s}%s' % (NS['kend'], attr))
            if value is not None:
                kwargs[attr] = value
        details = input.find('{%s}details' % NS['kend'])
        if details is not None:
            kwargs['details'] = details
        return kend.model.Fault(**kwargs)

    @staticmethod
    def _serialise(obj, nsmap = {}):
        nsmap.update({None: NS['kend']})
        fault = etree.Element('{%s}fault' % NS['kend'], nsmap = nsmap)
        for attr in ['code', 'string', 'source']:
            if getattr(obj, attr) is not None:
                node = etree.Element('{%s}%s' % (NS['kend'], attr), nsmap = nsmap)
                node.text = getattr(obj, attr)
                fault.append(node)
        if obj.details is not None:
            fault.append(obj.details)
        return fault

    @staticmethod
    def serialise(obj):
        obj = _Fault._serialise(obj, nsmap = NS_SERIALISE)
        _finaliseXML(obj, schema = 'fault')
        return obj, '%s;version=%s;type=%s' % (MIME_TYPE, kend.model.VERSION, 'fault')


class _Group:
    @staticmethod
    def _parse(input):
        fnode = input.find('{%s}fault' % NS['kend'])
        if fnode is not None:
            return kend.model.Group(fault=_Fault._parse(fnode))
        else:
            input_parameters = [(node.get('name'), node.text) for node in input.xpath('kend:parameters/input', namespaces = kend.model.NS)]
            ignored_parameters = [(node.get('name'), node.text) for node in input.xpath('kend:parameters/ignored', namespaces = kend.model.NS)]
            output_parameters = [(node.get('name'), node.text) for node in input.xpath('kend:parameters/output', namespaces = kend.model.NS)]
            annotations = [_Annotation._parse(node) for node in input.xpath('kend:annotation', namespaces = kend.model.NS)]
            documents = [_Document._parse(node) for node in input.xpath('kend:document', namespaces = kend.model.NS)]
            return kend.model.Group(input=input_parameters, ignored=ignored_parameters,
                                    output=output_parameters, annotations=annotations,
                                    documents=documents)

    @staticmethod
    def _serialise(obj, nsmap = {}):
        nsmap.update({None: NS['kend']})
        group = etree.Element('{%s}group' % NS['kend'], nsmap = nsmap)
        if obj.fault is not None:
            group.append(_Fault._serialise(obj.fault, nsmap))
        else:
            if len(obj.input) + len(obj.ignored) + len(obj.output) > 0:
                parameters = etree.Element('{%s}parameters' % NS['kend'], nsmap = nsmap)
                group.append(parameters)
                for tag, p, v in [('input', p, v) for p, v in obj.input] + [('ignored', p, v) for p, v in obj.ignored] + [('output', p, v) for p, v in obj.output]:
                    parameter = etree.Element('{%s}%s' % (NS['kend'], tag))
                    parameter.set('name', p)
                    parameter.text = v
                    parameters.append(parameter)
            if len(obj.documents) > 0:
                for doc in obj.documents:
                    group.append(_Document._serialise(doc, nsmap))
            if len(obj.annotations) > 0:
                for ann in obj.annotations:
                    group.append(_Annotation._serialise(ann, nsmap))
        return group


class _Annotations(_XMLConverter):
    @staticmethod
    def _parse(input):
        fnode = input.find('{%s}fault' % NS['kend'])
        if fnode is not None:
            return kend.model.Annotations(fault = _Fault._parse(fnode))
        else:
            return kend.model.Annotations(groups = [_Group._parse(node) for node in input.xpath('kend:group', namespaces = kend.model.NS)])

    @staticmethod
    def _serialise(obj, nsmap = {}):
        nsmap.update({None: NS['kend']})
        annotations = etree.Element('{%s}annotations' % NS['kend'], nsmap = nsmap)
        if obj.fault is not None:
            annotations.append(_Fault._serialise(obj.fault, nsmap))
        else:
            for group in obj:
                annotations.append(_Group._serialise(group, nsmap))
        return annotations

    @staticmethod
    def serialise(obj):
        obj = _Annotations._serialise(obj, nsmap = NS_SERIALISE)
        _finaliseXML(obj, schema = 'annotations')
        return obj, '%s;version=%s;type=%s' % (MIME_TYPE, kend.model.VERSION, 'annotations')


class _Documents(_XMLConverter):
    @staticmethod
    def _parse(input):
        fnode = input.find('{%s}fault' % NS['kend'])
        if fnode is not None:
            return kend.model.Documents(fault = _Fault._parse(fnode))
        else:
            return kend.model.Documents(groups = [_Group._parse(node) for node in input.xpath('kend:group', namespaces = kend.model.NS)])

    @staticmethod
    def _serialise(obj, nsmap = {}):
        nsmap.update({None: NS['kend']})
        documents = etree.Element('{%s}documents' % NS['kend'], nsmap = nsmap)
        if obj.fault is not None:
            documents.append(_Fault._serialise(obj.fault, nsmap))
        else:
            for group in obj:
                documents.append(_Group._serialise(group, nsmap))
        return documents

    @staticmethod
    def serialise(obj):
        obj = _Documents._serialise(obj, nsmap = NS_SERIALISE)
        _finaliseXML(obj, schema = 'documents')
        return obj, '%s;version=%s;type=%s' % (MIME_TYPE, kend.model.VERSION, 'documents')

_converters = {
    kend.model.DocumentReference: _DocumentReference,
    kend.model.Document:          _Document,
    kend.model.Annotation:        _Annotation,
    kend.model.Fault:             _Fault,
    kend.model.MediaSet:          _MediaSet,
    kend.model.Media:             _Media,
    kend.model.Annotations:       _Annotations,
    kend.model.Documents:         _Documents,
}

def parseXML(obj, cls = None, mime_name = None):
    if cls is None and mime_name is not None:
        for model in _converters.keys():
            if getattr(model, '__mime__') == mime_name:
                cls = model
                break
    return _converters.get(cls, InvalidConverter).parse(obj)

def parse(stream, cls = None, mime_name = None):
    obj = _reduceToElement(stream)
    return parseXML(obj, cls, mime_name)

def serialiseXML(obj, cls = None):
    return _converters.get(cls or type(obj), InvalidConverter).serialise(obj)

def serialise(obj, cls = None):
    output, mime_type = serialiseXML(obj, cls)
    return etree.tostring(
        output,
        pretty_print=True,
        xml_declaration=True,
        encoding='utf-8'
        ), mime_type





def convert4To6(xml, doc):
    old_ns = 'http://utopia.cs.manchester.ac.uk/utopia/annotation'
    new_ns = NS['kend']

    old_xml = xml
    xml = etree.Element('{%s}annotation' % new_ns)

    # Make public (everything was public in 0.4)
    xml.append(etree.Element('{%s}public' % new_ns))

    # Convert properties from old NSs to new
    old_properties = old_xml.find('{%s}properties' % old_ns)
    new_properties = etree.Element('{%s}properties' % new_ns)
    curatedBy = None
    if old_properties is not None:
        for old_property in old_properties:
            new_property_key = old_property.tag.replace('http://utopia.cs.manchester.ac.uk/utopia/annotation', 'http://utopia.cs.manchester.ac.uk/kend') \
                                               .replace('http://utopia.cs.man.ac.uk/utopia/annotation', 'http://utopia.cs.manchester.ac.uk/kend')
            new_property_text = old_property.text
            if new_property_key == '{http://utopia.cs.manchester.ac.uk/kend/compat#}created':
                created = new_property_text[:-7]
                xml.attrib['created'] = created
            elif new_property_key == '{http://utopia.cs.manchester.ac.uk/kend}concrete':
                pass # Ignore these
            else:
                if new_property_key == '{http://utopia.cs.manchester.ac.uk/kend/demo}logo':
                    new_property_key = '{http://utopia.cs.manchester.ac.uk/kend/property#}demo_logo'
                    new_property_text = '1'
                elif new_property_key == '{http://utopia.cs.manchester.ac.uk/kend}embedded':
                    new_property_key = '{http://utopia.cs.manchester.ac.uk/kend/property#}embedded'
                    new_property_text = '1'
                elif new_property_key == '{http://utopia.cs.manchester.ac.uk/kend/property#}curatedBy' and (new_property_text is None or new_property_text == 'Portland Press Ltd.'):
                    xml.attrib['author'] = 'Portland Press Ltd.'
                new_property = etree.Element(new_property_key)
                new_property.text = new_property_text
                new_properties.append(new_property)
    sourceVersion = etree.Element('{http://utopia.cs.manchester.ac.uk/kend/session#}sourceVersion')
    sourceVersion.text = 'Kend/0.4'
    new_properties.append(sourceVersion)

    old_concept = old_xml.find('{http://utopia.cs.manchester.ac.uk/utopia/annotation}concept')
    if old_concept is not None:
        xml.attrib['concept'] = old_concept.text
        concept = etree.Element('{http://utopia.cs.manchester.ac.uk/kend/property#}identifier')
        concept.text = old_concept.get('identifier')
        new_properties.append(concept)

    xml.append(new_properties)

    anchors = old_xml.find('{http://utopia.cs.manchester.ac.uk/utopia/annotation}anchor')
    if anchors is not None and doc is not None:
        anchors.tag = anchors.tag.replace('http://utopia.cs.manchester.ac.uk/utopia/annotation', 'http://utopia.cs.manchester.ac.uk/kend')
        document = etree.Element('{%s}document' % new_ns)
        document.attrib['id'] = doc
        for elem in list(anchors.getchildren()):
            elem.tag = elem.tag.replace('http://utopia.cs.manchester.ac.uk/utopia/annotation', 'http://utopia.cs.manchester.ac.uk/kend')
            for subelem in elem.getiterator():
                subelem.tag = subelem.tag.replace('http://utopia.cs.manchester.ac.uk/utopia/annotation', 'http://utopia.cs.manchester.ac.uk/kend')
            anchors.remove(elem)
            document.append(elem)
        anchors.append(document)
        xml.append(anchors)

    # Fix version number
    xml.attrib['version'] = '0.6'
    return xml;

def convert6To7(xml, doc):
    # Convert Public/Private to Published/Unpublished
    public = xml.find('{%s}public' % NS['kend'])
    if public is not None:
        xml.remove(public)
        xml.attrib['status'] = 'published'
    else:
        xml.attrib['status'] = 'unpublished'

    # Add updated timestamp (same as created)
    created = xml.get('created')
    if created is not None:
        xml.set('updated', created)

    # Remove groups
    groups = xml.find('{%s}groups' % NS['kend'])
    if groups is not None:
        xml.remove(groups)

    # Set public context URI
    xml.attrib['context'] = 'http://utopia.cs.manchester.ac.uk/contexts/public'

    # Properties
    properties = xml.find('{%s}properties' % NS['kend'])
    if properties is not None:
        found = False
        for property in properties:
            if property.tag == '{http://utopia.cs.manchester.ac.uk/kend/session#}sourceVersion':
                found = True
                break
        if not found:
            sourceVersion = etree.Element('{http://utopia.cs.manchester.ac.uk/kend/session#}sourceVersion')
            sourceVersion.text = 'Kend/0.6'
            properties.append(sourceVersion)

    # Fix version number
    xml.attrib['version'] = '0.7'
    return xml;

def convertTo7(xml, doc = None, v = None):
    element = _reduceToElement(xml)
    if (v or element.attrib.get('version', None)) == '0.7':
        return element
    elif (v or element.attrib.get('version', None)) == '0.6':
        return convert6To7(element, doc)
    elif (v or element.attrib.get('version', None)) == '0.4':
        return convertTo7(convert4To6(element, doc), doc)
    return None
