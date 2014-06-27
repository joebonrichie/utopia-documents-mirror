###############################################################################
#   
#    This file is part of the Utopia Documents application.
#        Copyright (c) 2008-2014 Lost Island Labs
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

import datetime
import re
import spineapi
import utopia



def hasprovenance(value):
    return hasattr(value, '_provenance') and type(value._provenance) == dict

def provenance(value):
    try:
        return value._provenance
    except AttributeError:
        raise RuntimeError('No associated provenance found for this value')

def wrap_provenance(value, **kwargs):
    kwargs.setdefault('when', datetime.datetime.now())
    if isinstance(kwargs['when'], datetime.datetime):
        kwargs['when'] = kwargs['when'].isoformat()
    class sourced(type(value)):
        _provenance = kwargs
    sourced = sourced(value)
    return sourced



def metadata(document, needle = None, as_list = False, all = False, whence = None):
    # Place to accumulate metadata before returning
    data = {}

    # Should the value(s) be seen as a list?
    if needle is not None and needle[-2:] == '[]':
        as_list = True
        needle = needle[:-2]

    # 'Official' metadata keys (i.e. legacy keys)
    alternatives = {
        'articleTitle': 'title',
        'articleDOI': 'doi',
        'articlePMID': 'pmid',
    }

    # Find all metadata annotations in the appropriate scratch list
    for annotation in document.annotations('Document Metadata'):
        # Check the two kinds of annotations that hold metadata
        if annotation.get('concept') in ('DocumentMetadata', 'DocumentIdentifier'):
            # Compile information from annotation
            provdata = {}
            metadata = {}
            for key, value in annotation.iteritems():
                if key.startswith('provenance:') and len(value) > 0:
                    key = key[11:]
                    provdata[key] = value[0]
                elif key.startswith('property:'):
                    key = key[9:]
                    key = alternatives.get(key, key)
                    if needle is None or needle == key:
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
        data = dict(((k, v[0]) for k, v in data.iteritems()))

    # If not searching for a specific key, only choose that key
    if needle is not None:
        data = data.get(needle)

    # Return requested metadata
    return data




def _metadata(document, needle = None, default = None, as_list = False, whence = None):
    # Place to accumulate metadata before returning
    data = {}

    # Should the needle be seen as a list?
    if needle is not None and needle[-2:] == '[]':
        as_list = True
        needle = needle[:-2]

    def set_data(key, weight, value):
        if value is not None:
            data.setdefault(key, {})
            data[key][weight] = value

    def one_or_all(key, value):
        if value is not None:
            if as_list:
                return value
            elif isinstance(value, (set, list, tuple)):
                if len(value) > 0:
                    prov = None
                    if hasprovenance(value):
                        prov = provenance(value)
                    value = tuple(value)[0]
                    if prov is not None:
                        value = wrap_provenance(value, **prov)
                    return value
            else:
                return value

    if whence is None and needle is not None:
        # Check 'official' metadata
        alternatives = {
            'title': 'articleTitle',
            'doi': 'articleDOI',
            'pmid': 'articlePMID',
        }
        if needle in alternatives:
            alternative_key = alternatives.get(needle)
            for annotation in document.annotations('Document Metadata'):
                if annotation.get('concept') == 'DocumentMetadata':
                    value = annotation.get('property:{0}'.format(alternative_key))
                    if value is not None:
                        value = wrap_provenance(value, whence=annotation.get('provenance:whence'))
                        set_data(needle, 0, value)

    # Check already scraped data
    for annotation in document.annotations('Document Metadata'):
        if annotation.get('concept') == 'DocumentIdentifier':
            if whence is None or annotation.get('provenance:whence') == whence:
                for key, value in annotation.iteritems():
                    if key.startswith('property:'):
                        key_name = key[9:]
                        if needle is None or needle == key_name:
                            weight = int(annotation.get('session:weight', 0))
                            value = wrap_provenance(value, whence=annotation.get('provenance:whence'))
                            set_data(key_name, weight, value)

    if needle is not None:
        data = data.get(needle, {})
        if len(data) > 0:
            return one_or_all(needle, data[max(data.keys())])
        else:
            return one_or_all(needle, default)
    else:
        return dict(((k, one_or_all(k, v[max(v.keys())])) for k, v in data.iteritems() if len(v) > 0))


def store_metadata(document, **kwargs):
    special = {
        ':whence': 'provenance',
        ':when': 'provenance',
        ':confidence': 'provenance',
        ':weight': 'provenance',
        }

    if len(kwargs) > 0:
        # Create an annotation
        annotation = spineapi.Annotation()
        annotation['concept'] = 'DocumentIdentifier'
        for key, value in kwargs.items():
            if value is not None:
                annotation['{0}:{1}'.format(special.get(key, 'property'), unicode(key).strip(':'))] = kwargs.get(key)
            else:
                del kwargs[key]
        annotation['session:volatile'] = '1'
        document.addAnnotation(annotation, 'Document Metadata')

        # Log the metadata being stored
        for key, value in kwargs.iteritems():
            if isinstance(value, str) or isinstance(value, unicode):
                if len(value) > 63:
                    value = value[:60] + '...'
                value = value.replace('\n', r'\n').replace('\r', r'\r')
                print "Stored metadata '{0}' / {2}: {1}".format(key, value.encode('utf8'), kwargs.get(':weight', '0'))
            else:
                print "Stored metadata '{0}' / {2}: {1}".format(key, repr(value).encode('utf8'), kwargs.get(':weight', '0'))

        return annotation


def format_citation(citation):
    def format(string, value):
        if value is not None and len(u'{0}'.format(value)) > 0:
            return unicode(string).format(value)
        else:
            return ''
    def get(map, key, default = None):
        value = map.get('property:{0}'.format(key))
        if value is None:
            value = map.get(key, default)
        if value is None:
            return default
        return value

    type = get(citation, 'type')
    html = get(citation, 'unstructured')
    label = get(citation, 'label', '')

    if html is None:
        copy = citation.copy()
        if 'label' in copy:
            del copy['label']
        html = utopia.citation.format(copy)
#         title = re.sub(r'([^!?.])$', r'\1.', get(citation, 'title', ''))
#         publication = get(citation, 'publication-title')
#         volume = get(citation, 'volume')
#         year = get(citation, 'year')
#         pages = get(citation, 'pages')
#         if pages is None and 'pagefrom' in citation and 'pageto' in citation:
#             pages = u'{0}-{1}'.format(get(citation, 'pagefrom', ''), get(citation, 'pageto', ''))
#         authors = get(citation, 'authors', [])
#         if len(authors) > 0:
#             author_list = get(citation, 'authors', [])
#             author_count = len(author_list)
#             max_authors = 4
#             authors = u' and '.join(author_list[:max_authors])
#             if author_count > max_authors:
#                 authors += 'u <em>et al.</em>'
#
#         data = {
#             'title': format('<em>{0}</em>', title),
#             'publication-title': format('{0}', publication),
#             'volume': format('{0}', volume),
#             'year': format('({0})', year),
#             'pages': format('{0}', pages),
#             'authors': format('{0}.', authors),
#         }
#         html = u'{authors} {year} {title} {publication-title}, {volume} {pages}'.format(**data).strip(' -,.').replace('()', '')
    if len(label) > 0:
        html = u'{0}. {1}'.format(format('<strong>{0}</strong>', label), html)
    return re.sub(r'([^!?,.])$', r'\1.', re.sub(r'\s+', ' ', html))
