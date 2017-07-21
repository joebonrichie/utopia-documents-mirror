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
import json
import re
import spineapi
from utopia.tools.provenance import *



def _splitKey(path):
    '''Parse off the base name of this structure path.'''
    try:
        delim = path.index('[')
        base = path[:delim]
        path = path[delim:]
        return base, path
    except:
        return path, ''

def _inject(key, value, existing):
    '''Parse off the base name of this structure path, then generate an
       appropriate structure from the path.'''
    try:
        base, path = _splitKey(key)
        steps = path.strip('[]').split('][')
        structure = existing
        if len(steps) > 1 or steps[0] != '':
            local_structure = structure
            for step in steps[:-1]:
                step = step.strip()
                local_structure.setdefault(step, {})
                local_structure = local_structure[step]
            step = steps[-1].strip()
            local_structure[step] = value
            key, value = base, structure
        elif isinstance(value, dict):
            underlay = existing.copy()
            underlay.update(value)
            value = underlay
    except:
        pass

    return key, value

def _extract(key, value):
    '''Parse off the base name of this structure path, then generate an
       appropriate structure from the path.'''
    try:
        base, path = _splitKey(key)
        steps = path.strip('[]').split('][')
        for step in steps:
            value = value.get(step)
            if value is None:
                break
        return value
    except:
        pass

    return None




def citation_to_annotation(citation, concept = 'Citation'):
    '''
        Given a citation dictionary, create a Citation annotation.
    '''

    # These special keys are used for provenance, so should get their own
    # prefix
    special = {
        ':whence': 'provenance',
        ':when': 'provenance',
        ':confidence': 'provenance',
        ':weight': 'provenance',
        }

    # Process the arguments to take into account nested dictionaries
    processed = {}
    for key, value in citation.items():
        base, path = _splitKey(key)
        # If the key includes square brackets, treat it as though it were
        # a path into a json structure, generating the appropriate structure
        key, value = _inject(key, value, processed.get(base, {}))
        processed[key] = value
    citation = processed

    # Create an annotation
    annotation = spineapi.Annotation()
    annotation['concept'] = concept
    annotation['session:volatile'] = '1'

    # Now store the values in the annotation
    for key, value in citation.items():
        if isinstance(value, basestring) or not isinstance(value, collections.Sequence):
            value = [value]
        # If the value is a dictionary, serialise it to json before storing it
        for i in xrange(len(value)):
            if isinstance(value[i], dict):
                value[i] = 'json:{0}'.format(json.dumps(value[i]))
        # Remove None objects
        value = [item for item in value if item is not None]
        if len(value) > 0:
            annotation[u'{0}:{1}'.format(special.get(key, 'property'), unicode(key).strip(':'))] = value

    return annotation

def citation_from_annotation(annotation):
    '''
        Given a Citation annotation, create a citation dictionary.
    '''

    # Force lists for these keys
    lists = ('authors', 'links', 'keywords')

    # Compile information from annotation
    provdata = {}
    metadata = {}
    for key, value in annotation.iteritems():
        # Begin by parsing any JSON fields
        for i in xrange(len(value)):
            if value[i].startswith('json:'):
                value[i] = json.loads(value[i][5:])
        # Save provenance data separately...
        if key.startswith('provenance:') and len(value) > 0:
            key = key[11:]
            provdata[key] = value[0]
            metadata[':{0}'.format(key)] = value[0]
        # ...to normal properties
        elif key.startswith('property:'):
            key = key[9:]
            # Store all the values
            if key in lists:
                metadata[key] = value
            elif len(value) > 0:
                metadata[key] = value[0]

    # Apply provenance and return
    return wrap_provenance(metadata, **provdata)




def metadata(document, needle = None, as_list = False, all = False, whence = None):
    '''
        Get metadata from the document. Either all metadata can be found, or
        just those matching a given key.

        The optional needle is the key to look for, and can be either simple or
        nested:
            'title' would return the data from the title property.
            'authors[]' would return the data as a list.
            'identifiers[doi]' would return the data from the identifiers
                dictionary that represents the 'doi' key.
            'links[][url]' would return all the data from the links
                dictionaries that represents the 'url' key, as a list.
        In the case when a key to a complex structure is provided, the entire
        structure is returned.
    '''

    # Place to accumulate metadata before returning
    data = {}

    # Should the value(s) be seen as a list?
    if needle is not None:
        # Get rid of the list modifier
        subbed = re.sub(r'^(\w+)\[\](.*)$', r'\1\2', needle)
        if subbed != needle:
            as_list = True
            needle = subbed
        # Split needle up in case it includes a path
        needle_base, needle_path = _splitKey(needle)
    else:
        needle_base, needle_path = '', ''

    # Find all metadata annotations in the appropriate scratch list
    for annotation in document.annotations('Document Metadata'):
        # Check the kinds of annotations that hold citation information
        if annotation.get('concept') in ('Citation',):
            # Compile information from annotation
            provdata = {}
            metadata = {}
            for key, value in citation_from_annotation(annotation).iteritems():
                # Save provenance data separately...
                if key.startswith(':'):
                    provdata[key[1:]] = value
                # ...to normal properties
                else:
                    if not isinstance(value, collections.Sequence) or isinstance(value, basestring):
                        value = [value]
                    if needle is None or needle_base == key:
                        # If the needle has a path
                        if len(needle_path) > 0:
                            # Extract values and remove None objects
                            value = [_extract(needle, item) for item in value]
                            value = [item for item in value if item is not None]
                        # Store all the values
                        if as_list:
                            metadata[key] = value
                        elif len(value) > 0:
                            metadata[key] = value[0]
            # Skip unmatching provenance
            if whence is not None and provdata.get('whence') != whence:
                continue
            # Apply provenance and store metadata
            for key, value in metadata.iteritems():
                data.setdefault(key, [])
                data[key].append(wrap_provenance(value, **provdata))

    # Sort metadata according to weight / when provenance fields
    def cmp_provenance(left, right):
        lprov, rprov = provenance(left), provenance(right)
        lweight, rweight = int(lprov.get('weight', '1')), int(rprov.get('weight', '0'))
        lwhen, rwhen = lprov.get('when'), rprov.get('when')
        if lweight > rweight:
            return -1
        elif lweight == rweight:
            return cmp(lwhen, rwhen)
        else:
            return 1
    for key, value in data.iteritems():
        value.sort(cmp_provenance)

    # If not requesting everything, only choose the highest weighted values
    if not all:
        chosen = {}
        for key, values in data.iteritems():
            if isinstance(values[0], dict):
                overlay = {}
                values.reverse()
                for value in values:
                    overlay.update(value)
                chosen[key] = overlay
            else:
                chosen[key] = values[0]
        data = chosen

    # If not searching for a specific key, only choose that key
    if needle is not None:
        data = data.get(needle_base)

    # Return requested metadata
    return data



def store_metadata(document, **kwargs):
    '''
        Store the provided metadata in the document in the form of an
        annotation.

        If the value is a string or unicode object, it will be stored as such.
        If the value is some other sequence (such as a list or tuple), its
        items will be stored as an ordered set of unicode strings. If the value
        is a dict, the dict's hiearchical structure will be turned into a set
        of key/value pairs before being stored.

        To store a value into an existing structure, the key can use brackets
        to specify where the value should end up. For example:
            "key[]" will append the value to the sequence of name 'key'
            "key[1]" will place the value into the list at position 1
            "key[a]" will place the value into the dictionary at position 'a'
    '''

    # Deal with the incoming keyword arguments
    if len(kwargs) > 0:
        annotation = citation_to_annotation(kwargs)
        document.addAnnotation(annotation, 'Document Metadata')

        # Log the metadata being stored DEBUG
        for key, value in annotation.iteritems():
            if len(value) == 1:
                value = value[0]
            if isinstance(value, str) or isinstance(value, unicode):
                if len(value) > 63:
                    value = value[:60] + '...'
                value = value.replace('\n', r'\n').replace('\r', r'\r')
                print("Stored metadata '{0}' / {2}: {1}".format(key, value.encode('utf8'), kwargs.get(':weight', '0')))
            else:
                print("Stored metadata '{0}' / {2}: {1}".format(key, repr(value).encode('utf8'), kwargs.get(':weight', '0')))

        return annotation
