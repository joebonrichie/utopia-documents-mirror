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

VERSION = '0.3'

NS = {'authd': 'http://utopia.cs.manchester.ac.uk/authd'}

#################
# ACL XSD Model #
#################

class AccessControlList(object):
    __mime__ = 'acl'
    def __init__(self, target = None, access = None):
        self.target = target # optional [!]
        self.access = access
# Could also implement iter and nonzero, but not sure when these are useful [!]

class Target(object):
    def __init__(self, annotation = None, uid = None, context = None, scope = 'header'):
        self.annotation = annotation # kend annotation [!] Refer to kend model.py?
        self.uid = uid
        self.context = context
        if scope is not None:
             self.scope = scope
        else:
             self.scope = 'header'

class Access(list): # can be empty [!]
    def __init__(self, actions = None):
    	list.__init__(self, actions or [])
# [!] could use a nonzero method if I ever want to test whether this is set, and get true even when it's empty

class Action(object):
    def __init__(self, name = None):
        self.name = name.lower()
        # also, any attributes and any children elements... [!]

    # Added to allow actions to be printed directly
    def __repr__(self):
        return "'%s'" % self.name.lower()

########################
# Authentication Model #
########################

class LoginCredentials(object):
    __mime__ = 'logininput'
    def __init__(self, service = None, credentials = None):
        self.service = service # just a string
        self.credentials = credentials or {} # a dictionary

class LoginResult(object):
    __mime__ = 'loginoutput'
    def __init__(self, uid = None, token = None, userdata = None):
        self.uid = uid # string
        self.token = token # encoded string
        self.userdata = userdata or {} # a dictionary

#################
# Context Model #
#################

class ContextQuery(object):
    __mime__ = 'contextquery'
    def __init__(self, uid = None, tid = None, pid = None):
        self.uid = uid
        self.tid = tid
        self.pid = pid

class ContextList(list):
    __mime__ = 'contextlist'
    def __init__(self, contexts = None):
    	list.__init__(self, contexts or [])

class Context(object):
    def __init__(self, cid = None, uri = None, description = None, task = None):
        self.cid = cid
        self.task = task
        self.uri = uri
        self.description = description

#################
# Account Model #
#################

class User(dict):
    __mime__ = 'user'
    def __init__(self, id = None, created = None, updated = None, edit = None, service = None, avatar = None, avatar_edit = None):
        self.id = id
        self.created = created
        self.updated = updated
        self.edit = edit
        self.service = service
        self.avatar = avatar
        self.avatar_edit = avatar_edit
        dict.__init__(self)
