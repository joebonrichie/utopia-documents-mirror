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
import json
import operator
import re
import socket
import string
import sys
import urllib2
import utopia
import utopia.extension
import uuid
from utopia.log import logger

###############################################################################
## General values used in this module.
###############################################################################

# Only the following key(spec)s play a role in the algorithms in this module
UUID_KEYSPEC               = 'key'
PROVENANCE_KEYSPEC         = 'provenance'
PROVENANCE_WHENCE_KEY      = 'whence'
PROVENANCE_WHENCE_KEYSPEC  = PROVENANCE_KEYSPEC + '/' + PROVENANCE_WHENCE_KEY
PROVENANCE_WHEN_KEY        = 'when'
PROVENANCE_WHEN_KEYSPEC    = PROVENANCE_KEYSPEC + '/' + PROVENANCE_WHEN_KEY
PROVENANCE_SOURCES_KEY     = 'sources'
PROVENANCE_SOURCES_KEYSPEC = PROVENANCE_KEYSPEC + '/' + PROVENANCE_SOURCES_KEY
PROVENANCE_INPUT_KEY       = 'input'
PROVENANCE_INPUT_KEYSPEC   = PROVENANCE_KEYSPEC + '/' + PROVENANCE_INPUT_KEY
PROVENANCE_REFS_KEY        = 'refs'
PROVENANCE_REFS_KEYSPEC    = PROVENANCE_KEYSPEC + '/' + PROVENANCE_REFS_KEY
PROVENANCE_PLUGIN_KEY      = 'plugin'
PROVENANCE_PLUGIN_KEYSPEC  = PROVENANCE_KEYSPEC + '/' + PROVENANCE_PLUGIN_KEY
# Provenance keyspec suffixes should be delimited by a colon
PROVENANCE_DELIMITER = ':'

# For use in functions to specify the absence of a keyword argument
_kwarg_sentinel = object()

# Which keys should be merged?
mergeable_keys = ('keywords', 'links')

###############################################################################
## Using keyspecs to investigate arbitrary objects (lists / dictionaries).
###############################################################################

# Keyspecs can either use path notation or index notation. This function
# normalises a keyspec to use the path notation. The following are all valid
# keyspecs and are equivalent, but only the first is 'normalised':
#     a/b/c:d
#     a/b/c#d
#     a[b][c]:d
#     a.b.c#d
#     a/b[c]:d
_keyspec_translator = string.maketrans('[.#', '//:')
def normalise_keyspec(keyspec):
    '''Normalise a keyspec into path notation'''
    if isinstance(keyspec, unicode):
        return keyspec.translate({
            ord('['): u'/',
            ord('.'): u'/',
            ord('#'): u':',
            ord(']'): None
        })
    else:
        return keyspec.translate(_keyspec_translator, ']')

# Split a keyspec into its individual keys
def split_keyspec(keyspec):
    '''Normalise and split the keyspec into a list of keys.'''
    return normalise_keyspec(keyspec).split('/')

# Given a dictionary, pick from it the value specified by the keyspec. This
# function does not recognise the source part of a keyspec, it expects only a
# path.
def pick(obj, keyspec, default=_kwarg_sentinel):
    '''Pick a value from a obj according to its keyspec'''
    curr_obj = obj
    has_default = default != _kwarg_sentinel
    try:
        seen = [] # For use in error reporting
        for key in split_keyspec(keyspec):
            if isinstance(curr_obj, collections.Mapping):
                curr_obj = curr_obj[key]
            elif isinstance(curr_obj, collections.Sequence):
                curr_obj = curr_obj[int(key)]
            seen.append(key)
        if isinstance(curr_obj, collections.Sequence) and not isinstance(curr_obj, basestring):
            curr_obj = [wrap_provenance(curr_obj[i], obj, keyspec + '/{}'.format(i)) for i in xrange(0, len(curr_obj))]
        return wrap_provenance(curr_obj, obj, keyspec)
    except (KeyError, IndexError, ValueError):
        if not has_default:
            if len(seen) == 0:
                raise KeyError(key)
            else:
                raise KeyError('/'.join(seen) + '/' + key)
    except:
        if not has_default:
            raise
    return default

###############################################################################
## For reference purposes, a refspec specifies both the citation and the
## keyspec needed to refer to a particular value.
###############################################################################

# Generate a refspec from a citation and its keyspec
def refspec(citation, keyspec=None):
    '''Generate a refspec'''
    # Ensure the citation has a key
    if UUID_KEYSPEC not in citation:
        citation[UUID_KEYSPEC] = str(uuid.uuid4())
    spec = '@{0}'.format(citation[UUID_KEYSPEC])
    if keyspec is not None:
        spec += ':' + normalise_keyspec(keyspec)
    return spec

# Split a refspec up into its constituent parts
def split_refspec(refspec):
    '''Split a refspec into a UUID and a keyspec.'''
    match = re.match(r'@(?P<uuid>[0-9a-f-]+):(?P<keyspec>.*)', refspec, re.I)
    if match is not None:
        return match.group('uuid'), match.group('keyspec')
    else:
        raise ValueError(u'refspec expected in the form @<uuid>:<keyspec>, but received {}'.format(refspec))

###############################################################################
## Imbue arbitrary values with provenance information (citation / keyspec).
###############################################################################

# Wrap a simple value up so we can silently store provenance information
def wrap_provenance(value, citation=None, keyspec=None):
    class sourced(type(value)):
        def __init__(self, value):
            self.citation = citation
            self.keyspec = keyspec
            super(sourced, self).__init__(value)
        @property
        def refspec(self):
            return refspec(self.citation, self.keyspec)
    return sourced(value)

# Check if a value has been wrapped in provenance
def has_provenance(value):
    return hasattr(value, 'citation') and hasattr(value, 'keyspec')

###############################################################################
## Some utility functions for dealing with citations
###############################################################################

# Get a set of all the top-level keyspecs a citation has
def inspect_keyspecs(citation, include_provenance=False):
    '''List a citation's keyspecs'''
    specs = set()
    for key, value in citation.items():
        if include_provenance or key != PROVENANCE_KEYSPEC:
            if isinstance(value, collections.Mapping):
                specs.update(set(['{0}/{1}'.format(key, keyspec) for keyspec in inspect_keyspecs(value)]))
            else:
                specs.add(key)
    return specs

# Set a citation's value according to a keyspec
def set_by_keyspec(citation, keyspec, value):
    '''Set a value for a keyspec on a citation'''
    keys = split_keyspec(keyspec)
    for key in keys[:-1]:
        citation.setdefault(key, {})
        citation = citation[key]
    citation[keys[-1]] = value

# Merge an update into a citation according to their keyspecs
def merge(citation, update):
    '''Merge an update into a citation by keyspec'''
    for keyspec in inspect_keyspecs(update, include_provenance=True):
        set_by_keyspec(citation, keyspec, pick(update, keyspec))

###############################################################################
## How to pick 'correct' values from across multiple citations.
###############################################################################

# Default pick order
pick_order = [
    'utopia',
    'crackle',
    'kend',
    'crossref',
    'pubmed',
    'pmc',
    'arxiv'
]

# Given a list of dictionaries, pick the value specified by the keyspec
def pick_from(citations, keyspec, default=_kwarg_sentinel, order=pick_order, record_in=_kwarg_sentinel):
    '''Pick a value from a list of objects according to its keyspec'''
    has_default = default != _kwarg_sentinel
    if isinstance(order, collections.Sequence):
        def order_fn(a, b):
            a = pick(a, PROVENANCE_WHENCE_KEYSPEC, None)
            b = pick(b, PROVENANCE_WHENCE_KEYSPEC, None)
            a = (a not in order and -1) or order.index(a)
            b = (b not in order and -1) or order.index(b)
            return cmp(b, a)
    elif callable(order): # This will fail to work in Python 3.0/3.1
        order_fn = order
    else:
        raise ValueError('Invalid ordering (expected list or function): {0}'.format(repr(order)))
    citations = sorted(citations, order_fn)
    keyspec, _, whence = keyspec.partition(PROVENANCE_DELIMITER)
    if len(whence) > 0 and whence[-1] == '*':
        is_all = True
        whence = whence[:-1]
    else:
        is_all = False
    is_mergeable = keyspec in mergeable_keys
    all = []
    merged = []
    for citation in citations:
        try:
            if len(whence) == 0 or whence == pick(citation, PROVENANCE_WHENCE_KEYSPEC, None):
                value = pick(citation, keyspec)
                if is_all:
                    all.append(value)
                elif is_mergeable:
                    blocked = is_mergeable and value in merged
                    if not blocked:
                        for i in xrange(0, len(value)):
                            item = value[i]
                            if item not in merged:
                                if record_in != _kwarg_sentinel:
                                    refspec = value.refspec + '/{}'.format(i)
                                    inputs = pick(record_in, PROVENANCE_INPUT_KEYSPEC, [])
                                    if refspec not in inputs:
                                        inputs.append(refspec)
                                        set_by_keyspec(record_in, PROVENANCE_INPUT_KEYSPEC, inputs)
                                merged.append(item)
                else:
                    if record_in != _kwarg_sentinel:
                        refspec = value.refspec
                        inputs = pick(record_in, PROVENANCE_INPUT_KEYSPEC, [])
                        if refspec not in inputs:
                            inputs.append(refspec)
                            set_by_keyspec(record_in, PROVENANCE_INPUT_KEYSPEC, inputs)
                    return value
        except:
            pass
    if is_all and len(all) > 0:
        return all
    if is_mergeable and len(merged) > 0:
        return merged
    if has_default:
        return default
    else:
        key = '/'.join(split_keyspec(keyspec))
        if len(whence) > 0:
            key += ':{}'.format(whence)
        raise KeyError(key)

# Flatten a list of objects according to its keyspecs
def flatten(citations):
    '''Flatten a list of objects by keyspec'''
    flattened = {}
    refs = []
    set_by_keyspec(flattened, PROVENANCE_SOURCES_KEYSPEC, citations)
    set_by_keyspec(flattened, PROVENANCE_REFS_KEYSPEC, refs)
    for keyspec in reduce(operator.or_, [inspect_keyspecs(citation) for citation in citations]):
        is_mergeable = keyspec in mergeable_keys
        value = pick_from(citations, keyspec, None)
        if value is not None:
            if has_provenance(value):
                refs.append(value.refspec)
            elif is_mergeable and isinstance(value, collections.Sequence):
                refs.extend([item.refspec for item in value if has_provenance(item)])
            set_by_keyspec(flattened, keyspec, value)
    return flattened

###############################################################################
## Utilities for working with links
###############################################################################

# Filter links according to criteria
def filter_links(citations, criteria = {}, provenance = {}):
    '''Find all of a citation's links that match the given criteria'''
    filtered = []
    for links in pick_from(citations, 'links*', []):
        for link in links:
            link_provenance = pick(link.citation, PROVENANCE_KEYSPEC, {})
            match = True
            for key, value in criteria.iteritems():
                if (value is None and key not in link) or (value is not None and link.get(key) != value):
                    match = False
                    break
            if match:
                for key, value in provenance.iteritems():
                    if (value is None and key not in link_provenance) or (value is not None and link_provenance.get(key) != value):
                        match = False
                        break
            if match:
                filtered.append(link)
    #filtered.sort(key=lambda link: link.get(':weight'), reverse=True)
    return filtered

# Check to see if a particular link exists
def has_link(citations, criteria = {}, provenance = {}):
    '''Find if a citation has a link that matches the given criteria'''
    return len(filter_links(citations, criteria=criteria, provenance=provenance)) > 0

###############################################################################
## Plugin mechanism for adding citation resolvers
###############################################################################

# An abstract base class for specifying resolvers
class Resolver(utopia.extension.Extension):
    # Keep track of errors so that we can inform the user
    def execute_resolver(self, citations, document = None):
        # Strip off the 'plugin_XXXXX_' prefix from the module's name
        plugin_name = '{0}.{1}'.format(self.__module__.split('_')[-1], self.__class__.__name__)
        logger.debug('Resolving from {0}: {1}'.format(plugin_name, getattr(self, '__doc__', 'Unknown resolver')))

        # If a source already exists that was generated by this plugin, skip it
        if len([citation for citation in citations if pick(citation, PROVENANCE_PLUGIN_KEYSPEC, None) == plugin_name]) == 0:
            try:
                # Just start by trying to execute the resolver's resolve() method
                resolved = self.resolve(citations, document)
                if resolved is not None:
                    if isinstance(resolved, collections.Mapping):
                        resolved = [resolved]
                    sanitised = []
                    for citation in resolved:
                        # Are there any non-provenance keys?
                        if len([key for key in citation.keys() if key != PROVENANCE_KEYSPEC]) > 0:
                            # Add any resolver provenance to the result
                            if hasattr(self, 'provenance') and callable(self.provenance):
                                provenance = self.provenance()
                                if not isinstance(provenance, collections.Mapping):
                                    provenance = {}
                                # Record the name of the plugin that generated the citation
                                set_by_keyspec(provenance, PROVENANCE_PLUGIN_KEY, plugin_name)
                                existing = pick(citation, PROVENANCE_KEYSPEC, {})
                                provenance.update(existing)
                                set_by_keyspec(citation, PROVENANCE_KEYSPEC, provenance)
                                # Record when the citation was generated locally
                                set_by_keyspec(citation, PROVENANCE_WHEN_KEYSPEC, datetime.datetime.now().isoformat())
                            sanitised.append(citation)
                    if len(sanitised) > 0:
                        return sanitised
            except Exception as exception:
                # Unwrap timeouts
                if isinstance(exception, urllib2.URLError) and hasattr(exception, 'reason') and isinstance(exception.reason, socket.timeout):
                    exception = exception.reason
                # Record the name of the plugin that generated the error
                error = {}
                set_by_keyspec(error, PROVENANCE_WHEN_KEYSPEC, datetime.datetime.now().isoformat())
                set_by_keyspec(error, PROVENANCE_PLUGIN_KEYSPEC, plugin_name)
                # Get the provenance from the resolver
                if hasattr(self, 'provenance') and callable(self.provenance):
                    provenance = self.provenance()
                    if isinstance(provenance, collections.Mapping):
                        existing = pick(error, PROVENANCE_KEYSPEC, {})
                        existing.update(provenance)
                        set_by_keyspec(error, PROVENANCE_KEYSPEC, existing)
                # Deal with the various known errors
                if isinstance(exception, socket.timeout):
                    category = 'timeout'
                    message = 'The server did not respond'
                elif isinstance(exception, urllib2.HTTPError):
                    category = 'server'
                    message = unicode(getattr(exception, 'reason', 'The server did not respond as expected'))
                elif isinstance(exception, urllib2.URLError):
                    category = 'connection'
                    message = unicode(getattr(exception, 'reason', 'The server could not be found'))
                else:
                    category = 'unknown'
                    message = 'An unexpected error occured'
                error['error'] = {
                    'category': category,
                    'message': message,
                }
                logger.warning('Error in resolver ({0}): {1}'.format(pick(error, PROVENANCE_WHENCE_KEYSPEC, '?'), message), exc_info=True)
                return [error]
        # If the result isn't what we expect, return None
        logger.debug(' -> null result')
        return None

# Resolve a citation
def resolve(citation = None, citations = None, document = None, purposes = ('identify','expand','dereference')):
    '''Push a citation through the resolution pipeline'''
    resolvers = [ResolverType() for ResolverType in Resolver.types()]
    resolvers.sort(key=lambda r: r.weight())
    if citations is None:
        citations = []
    if citation is not None:
        citations.append(citation)
    for purpose in purposes:
        for resolver in resolvers:
            if hasattr(resolver, 'purposes') and resolver.purposes() == purpose:
                resolved = resolver.execute_resolver(citations, document)
                if resolved is not None:
                    citations.extend(resolved)
    return flatten(citations)

# Expand a citation
def expand(citation = None, citations = None, document = None):
    return resolve(citation=citation, citations=citations, document=document, purposes=('expand',))

# Identify a citation
def identify(citation = None, citations = None, document = None):
    return resolve(citation=citation, citations=citations, document=document, purposes=('identify',))

# Dereference a citation
def dereference(citation = None, citations = None, document = None):
    return resolve(citation=citation, citations=citations, document=document, purposes=('dereference',))

###############################################################################
## The rest of this module requires the Utopia bridge to be present
###############################################################################

try:
    import utopiabridge
except ImportError:
    utopiabridge = None

if utopiabridge is not None:
    def format(citation):
        def _format(string, value):
            if value is not None and len(u'{0}'.format(value)) > 0:
                return unicode(string).format(value)
            else:
                return ''
        def _get(map, key, default = None):
            value = map.get('property:{0}'.format(key))
            if value is None:
                value = map.get(key, default)
            if value is None:
                return default
            return value
        def _has(map, key):
            return 'property:{0}'.format(key) in map or key in map

        html = None
        if _has(citation, 'unstructured'):
            if not (_has(citation, 'title') or
                    _has(citation, 'authors') or
                    _has(citation, 'year') or
                    _has(citation, 'publisher') or
                    _has(citation, 'publication-title')):
                html = _get(citation, 'unstructured')

        if html is None and hasattr(utopia.citation, '_formatCSL'):
            html = utopia.citation._formatCSL(citation)

        return html

    def render(citation, process = False, links = True):
        def jsonify(obj):
            return json.dumps(obj).replace("'", '&#39;').replace('"', '&#34;')
        if process:
            content = ''
        else:
            content = utopia.citation.format(citation)
        return u'''<div class="-papyro-internal-citation" data-citation="{0}" data-process="{1}" data-links="{2}">{3}</div>'''.format(jsonify(citation), jsonify(process), jsonify(links), content)
