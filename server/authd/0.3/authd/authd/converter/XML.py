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
import authd.model
from authd.converter.base import *
from lxml import etree
import datetime

import kend.converter.XML
import kend.model

import logging
log = logging.getLogger(__name__)

###############################
# Version and Namespace stuff #
###############################

MIME_TYPE = '%s+xml' % MIME_TYPE_BASE

NS = authd.model.NS.copy()
NS.update({'xsi': 'http://www.w3.org/2001/XMLSchema-instance'})

NS_SERIALISE = NS.copy()
del NS_SERIALISE['authd']
NS_SERIALISE[None] = NS['authd']

#####################
# Reduce to Element #
#####################

# FIXME this should only ever raise a auth conversion error, but at the moment it could raise others
def _reduceToElement(object):
    """Given some arbitrary input, convert it to an lxml Element node"""
    if not isinstance(object, etree._Element):
        if not isinstance(object, etree._ElementTree):
            try:
                if isinstance(object, str) or isinstance(object, unicode):
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

################
# Finalise XML #
################

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
    xml.set('version', authd.model.VERSION)
    xml.set('{%s}schemaLocation' % NS['xsi'], '%s https://utopia.cs.manchester.ac.uk/authd/%s/xsd/%s' % (NS['authd'], authd.model.VERSION, schema))
    etree.cleanup_namespaces(xml)
    return xml

####################################################################################################
# Base XML converter #
######################

class _XMLConverter(BaseConverter):
    ##################
    # Parse (public) #
    ##################
    @classmethod
    def parse(cls, input):
        input = _reduceToElement(input)
        found_version = input.get('version', None)
        if found_version is not None and found_version != authd.model.VERSION:
            raise VersionError("Expected format version '%s', but found version '%s'" % (authd.model.VERSION, found_version))
        return cls._parse(input) # Ensure input is an ElementTree object

####################################################################################################
# ACL XML converter #
#####################

class _AccessControlList(_XMLConverter):
    ###################
    # Parse (private) #
    ###################
    @staticmethod
    def _parse(input):
        # Parse the child target element (if exists) into a target object
        xml_target = input.find('{%s}target' % NS['authd'])
        if xml_target is not None:
            target = _Target._parse(xml_target)
        else:
            target = None

        # Parse the child access element (MUST exist [!]) into an access object
        xml_access = input.find('{%s}access' % NS['authd'])
        access = _Access._parse(xml_access)
        # Create the AccessControlList object from target and access
        return authd.model.AccessControlList(target, access)

    #######################
    # Serialise (private) #
    #######################
    @staticmethod
    def _serialise(obj, nsmap = {}):
        nsmap.update({None: NS['authd']})
        # Create an acl element
        xml_acl =  etree.Element('{%s}acl' % (NS['authd']), nsmap = nsmap)
        # Add a target child element to the acl element, from the object's target attribute...
        if obj.target is not None: # ... if it exists
            xml_acl.append(_Target._serialise(obj.target, nsmap))
        # Add an access child element to the acl element, from the object's access attribute
        xml_acl.append(_Access._serialise(obj.access, nsmap))
        # Return the acl element
        return xml_acl

    ######################
    # Serialise (public) #
    ######################
    @staticmethod
    def serialise(obj): # For root element only? Returns string version of XML [!]
        obj = _AccessControlList._serialise(obj, nsmap = NS_SERIALISE)
        _finaliseXML(obj, 'acl') # check what finalize XML does? [!] acl == schema name?
        return obj, '%s;version=%s;type=%s' % (MIME_TYPE, authd.model.VERSION, 'acl')
            # check what the name (acl) is supposed to mean [!]

####################################################################################################
# Target XML converter #
########################

class _Target:
    ###################
    # Parse (private) #
    ###################
    @staticmethod
    def _parse(input):
        # Parse the child annotation element into an annotation object
        xml_annotation = input.find('{%s}annotation' % kend.model.NS['kend'])
        if xml_annotation is not None:
            annotation = kend.converter.XML.parseXML(xml_annotation, kend.model.Annotation) # from kend
        else:
            annotation = None
        # Parse the scope attribute (MUST exist [!]) into a scope string
        uid = input.get('uid', None)
        context = input.get('context', None)
        scope = input.get('scope', None)
        # Create the Target object from annotation and scope
        return authd.model.Target(annotation, uid, context, scope)

    #######################
    # Serialise (private) #
    #######################
    @staticmethod
    def _serialise(obj, nsmap = {}):
        nsmap.update({None: NS['authd']})
        # Create a target element
        xml_target =  etree.Element('{%s}target' % (NS['authd']), nsmap = nsmap)
        # Add an annotation child element to the target element, from the object's annotation attribute
        if obj.annotation is not None:
            annotation, mime_type = kend.converter.XML.serialiseXML(obj.annotation)
            xml_target.append(annotation) # kend
        # Add a scope attribute to the target element, from the object's scope attribute
        xml_target.set('scope', obj.scope)

        uid = getattr(obj, 'uid', None)
        if uid is not None:
            xml_target.set('uid', uid)

        context = getattr(obj, 'context', None)
        if context is not None:
            xml_target.set('context', context)

        # Return the target element
        return xml_target

####################################################################################################
# Access XML converter #
########################

class _Access:
    ###################
    # Parse (private) #
    ###################
    @staticmethod
    def _parse(input):
        # Parse all the child action element into a list of action object
        actions = []
        if input is not None:
            actions = [_Action._parse(node) for node in input.xpath('authd:action', namespaces = NS)]
        # Create the Access object from actions
        return authd.model.Access(actions)

    #######################
    # Serialise (private) #
    #######################
    @staticmethod
    def _serialise(obj, nsmap = {}):
        nsmap.update({None: NS['authd']})
        # Create a target element
        xml_access =  etree.Element('{%s}access' % (NS['authd']), nsmap = nsmap)
        # For each item in the object (it's a list), add an action child element to the access element
        for action in obj:
            xml_access.append(_Action._serialise(action, nsmap))
        # Return the access element
        return xml_access

####################################################################################################
# Action XML converter #
########################

class _Action:
    ###################
    # Parse (private) #
    ###################
    @staticmethod
    def _parse(input):
        # Parse the name attribute (MUST exist [!]) into a name string
        name = input.get('name')
        # Create the Action object from name
        return authd.model.Action(name)

    #######################
    # Serialise (private) #
    #######################
    @staticmethod
    def _serialise(obj, nsmap = {}):
        nsmap.update({None: NS['authd']})
        # Create an action element
        xml_action =  etree.Element('{%s}action' % (NS['authd']), nsmap = nsmap)
        # Add a name attribute to the action element, from the object's name attribute
        xml_action.set('name', obj.name)
        # Return the action element
        return xml_action


####################################################################################################
# LoginCredentials converter #
##############################

class _LoginCredentials(_XMLConverter):
    ###################
    # Parse (private) #
    ###################
    @staticmethod
    def _parse(input):
        service = input.findtext('{%s}service' % NS['authd'])
        credentials = {}
        for node in input.xpath('authd:credentials', namespaces = NS):
            credentials[ node.get('key') ] = node.text
        return authd.model.LoginCredentials(service, credentials)

    #######################
    # Serialise (private) #
    #######################
    @staticmethod
    def _serialise(obj, nsmap = {}):
        nsmap.update({None: NS['authd']})

        xml_input = etree.Element('{%s}logininput' % (NS['authd']), nsmap = nsmap)

        xml_service = etree.Element('{%s}service' % (NS['authd']), nsmap = nsmap)
        xml_service.text = obj.service
        xml_input.append(xml_service)

        for key, value in obj.credentials.items():
            xml_credentials = etree.Element('{%s}credentials' % (NS['authd']), nsmap = nsmap)
            xml_credentials.set('key', key)
            xml_credentials.text = value
            xml_input.append(xml_credentials)

        return xml_input

    ######################
    # Serialise (public) #
    ######################
    @staticmethod
    def serialise(obj): # For root element only? Returns string version of XML [!]
        obj = _LoginCredentials._serialise(obj, nsmap = NS_SERIALISE)
        _finaliseXML(obj, 'logininput')
        return obj, '%s;version=%s;type=%s' % (MIME_TYPE, authd.model.VERSION, 'logininput')

####################################################################################################
# LoginResult converter #
#########################

class _LoginResult(_XMLConverter):
    ###################
    # Parse (private) #
    ###################
    @staticmethod
    def _parse(input):
        token = input.findtext('{%s}token' % NS['authd'])
        uid = input.findtext('{%s}uid' % NS['authd'])
        userdata = {}
        for node in input.xpath('authd:userdata', namespaces = NS):
            userdata[ node.get('key') ] = node.text
        return authd.model.LoginResult(uid, token, userdata)

    #######################
    # Serialise (private) #
    #######################
    @staticmethod
    def _serialise(obj, nsmap = {}):
        nsmap.update({None: NS['authd']})

        xml_output = etree.Element('{%s}loginoutput' % (NS['authd']), nsmap = nsmap)

        xml_token = etree.Element('{%s}token' % (NS['authd']), nsmap = nsmap)
        if obj.token is not None:
            xml_token.text = obj.token
        xml_output.append(xml_token)

        xml_uid = etree.Element('{%s}uid' % (NS['authd']), nsmap = nsmap)
        if obj.uid is not None:
            xml_uid.text = obj.uid
        xml_output.append(xml_uid)

        if obj.userdata is not None:
            for key, value in obj.userdata.items():
                xml_userdata = etree.Element('{%s}userdata' % (NS['authd']), nsmap = nsmap)
                xml_userdata.set('key', key)
                xml_userdata.text = value
                xml_output.append(xml_userdata)

        return xml_output

    ######################
    # Serialise (public) #
    ######################
    @staticmethod
    def serialise(obj): # For root element only? Returns string version of XML [!]
        obj = _LoginResult._serialise(obj, nsmap = NS_SERIALISE)
        _finaliseXML(obj, 'loginoutput')
        return obj, '%s;version=%s;type=%s' % (MIME_TYPE, authd.model.VERSION, 'loginoutput')

####################################################################################################
# ContextQuery converter #
##########################

class _ContextQuery(_XMLConverter):
    ###################
    # Parse (private) #
    ###################
    @staticmethod
    def _parse(input):
        xml_user = input.find('{%s}user' % NS['authd'])
        uid = xml_user.text

        xml_task = input.find('{%s}task' % NS['authd'])
        tid = None
        if xml_task is not None:
            tid = xml_task.text

        xml_paper = input.find('{%s}paper' % NS['authd'])
        pid = None
        if xml_paper is not None:
            pid = xml_paper.text

        return authd.model.ContextQuery(uid, tid, pid)

    #######################
    # Serialise (private) #
    #######################
    @staticmethod
    def _serialise(obj, nsmap = {}):
        nsmap.update({None: NS['authd']})

        xml_output = etree.Element('{%s}contextquery' % (NS['authd']), nsmap = nsmap)

        xml_user = etree.Element('{%s}user' % (NS['authd']), nsmap = nsmap)
        xml_user.text = obj.uid
        xml_output.append(xml_user)

        if obj.tid is not None:
            xml_task = etree.Element('{%s}task' % (NS['authd']), nsmap = nsmap)
            xml_task.text = obj.tid
            xml_output.append(xml_task)

        if obj.pid is not None:
            xml_paper = etree.Element('{%s}paper' % (NS['authd']), nsmap = nsmap)
            xml_paper.text = obj.pid
            xml_output.append(xml_paper)

        return xml_output

    ######################
    # Serialise (public) #
    ######################
    @staticmethod
    def serialise(obj): # For root element only? Returns string version of XML [!]
        obj = _ContextQuery._serialise(obj, nsmap = NS_SERIALISE)
        _finaliseXML(obj, 'contextquery')
        return obj, '%s;version=%s;type=%s' % (MIME_TYPE, authd.model.VERSION, 'contextquery')

####################################################################################################
# ContextList converter #
#########################

class _ContextList(_XMLConverter):
    ###################
    # Parse (private) #
    ###################
    @staticmethod
    def _parse(input):
        contexts = authd.model.ContextList()
        for node in input.xpath('authd:context', namespaces = NS):
            contexts.append(_Context._parse(node))
        return contexts

    #######################
    # Serialise (private) #
    #######################
    @staticmethod
    def _serialise(obj, nsmap = {}):
        nsmap.update({None: NS['authd']})

        xml_output = etree.Element('{%s}contextlist' % (NS['authd']), nsmap = nsmap)

        for context in obj:
            xml_output.append(_Context._serialise(context, nsmap))

        return xml_output

    ######################
    # Serialise (public) #
    ######################
    @staticmethod
    def serialise(obj): # For root element only? Returns string version of XML [!]
        obj = _ContextList._serialise(obj, nsmap = NS_SERIALISE)
        _finaliseXML(obj, 'contextlist')
        return obj, '%s;version=%s;type=%s' % (MIME_TYPE, authd.model.VERSION, 'contextlist')

####################################################################################################
# Context converter #
#####################

class _Context(_XMLConverter):
    ###################
    # Parse (private) #
    ###################
    @staticmethod
    def _parse(input):
        description = input.text
        uri = input.get('uri', None)
        cid = input.get('cid', None) # normally None [!] Useless model attribute?
        task = input.get('task', None)
        return authd.model.Context(cid, uri, description, task)

    #######################
    # Serialise (private) #
    #######################
    @staticmethod
    def _serialise(obj, nsmap = {}):
        nsmap.update({None: NS['authd']})

        xml_context = etree.Element('{%s}context' % (NS['authd']), nsmap = nsmap)

        xml_context.set('uri', obj.uri)
        if obj.description is not None:
            xml_context.text = obj.description
        if obj.task is not None:
            xml_context.set('task', obj.task)

        return xml_context

####################################################################################################
# Context converter #
#####################

class _User(_XMLConverter):
    ###################
    # Parse (private) #
    ###################
    @staticmethod
    def _parse(input):
        created, updated = None, None
        try:
            created = datetime.datetime.strptime(input.attrib.get('created')[:19], '%Y-%m-%dT%H:%M:%S')
        except:
            pass # Ignore unparsable datetime fields
        try:
            updated = datetime.datetime.strptime(input.attrib.get('updated')[:19], '%Y-%m-%dT%H:%M:%S')
        except:
            pass # Ignore unparsable datetime fields
        user = authd.model.User(
            input.get('id'),
            created,
            updated,
            input.get('edit'),
            input.get('service'),
            input.get('avatar'),
            input.get(_formatattr('avatar_edit')),
        )
        info = {}
        # Get data
        for node in input.xpath('authd:info', namespaces = NS):
            key = node.get('name', None)
            value = node.text
            if key is not None and value is not None:
                info[key] = value
        user.update(info)
        return user

    #######################
    # Serialise (private) #
    #######################
    @staticmethod
    def _serialise(obj, nsmap = {}):
        nsmap.update({None: NS['authd']})
        xml_user = etree.Element('{%s}user' % (NS['authd']), nsmap = nsmap)
        for key in ('id', 'created', 'updated', 'edit', 'service', 'avatar', 'avatar_edit'):
            value = getattr(obj, key, None)
            if value is not None:
                xml_user.set(_formatattr(key), _format(value))
        for key, value in obj.iteritems():
            xml_info = etree.Element('{%s}info' % (NS['authd']), nsmap = nsmap)
            xml_info.set('name', key)
            xml_info.text = value
            xml_user.append(xml_info)

        return xml_user

    ######################
    # Serialise (public) #
    ######################
    @staticmethod
    def serialise(obj): # For root element only? Returns string version of XML [!]
        obj = _User._serialise(obj, nsmap = NS_SERIALISE)
        _finaliseXML(obj, 'user') # check what finalize XML does? [!] acl == schema name?
        return obj, '%s;version=%s;type=%s' % (MIME_TYPE, authd.model.VERSION, 'user')

####################################################################################################
# List of converters, and global parse and serialise #
######################################################

_converters = {
    authd.model.AccessControlList: _AccessControlList,
    authd.model.LoginResult: _LoginResult,
    authd.model.LoginCredentials: _LoginCredentials,
    authd.model.ContextQuery: _ContextQuery,
    authd.model.ContextList: _ContextList,
    authd.model.User: _User,
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

