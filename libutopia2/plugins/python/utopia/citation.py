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

import json
import re
import sys
import traceback
import utopia.library




def _resolve(metadata, purpose, document = None):
    resolvers = []
    for Resolver in utopia.library.Resolver.types():
        resolvers.append(Resolver())
    resolvers.sort(key=lambda r: r.weight())
    for resolver in resolvers:
        if hasattr(resolver, 'purposes') and resolver.purposes() == purpose:
            try:
                overlay = resolver.resolve(metadata, document)
                if overlay is not None:
                    for k, v in overlay.iteritems():
                        if v is None and k in metadata:
                            del metadata[k]
                        else:
                            metadata[k] = v
            except:
                traceback.print_exc()
    return metadata




def expand(citation, document = None):
    return _resolve(citation, purpose='expand', document=document)

def identify(citation, document = None):
    return _resolve(citation, purpose='identify', document=document)

def dereference(citation, document = None):
    return _resolve(citation, purpose='dereference', document=document)




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

    if html is None:
        html = utopia.citation._formatCSL(citation)

    return html



def _orderLinks(citation):
    if 'links' in citation:
        def cmp(c1, c2):
            # First sort on mime type to make PDF first
            pdf1 = (c1.get('mime') == 'application/pdf')
            pdf2 = (c2.get('mime') == 'application/pdf')
            if pdf1 and not pdf2:
                return -1
            elif not pdf1 and pdf2:
                return 1
            else:
                # Next sort articles ahead of abstracts, ahead of searches
                order = ['search', 'abstract', 'article']
                t1 = c1.get('type')
                t1 = order.index(t1) if t1 in order else -1
                t2 = c2.get('type')
                t2 = order.index(t2) if t2 in order else -1
                if t1 != t2:
                    return t2 - t1
                else:
                    # Finally, sort on weights making higher weights first
                    w1 = c1.get(':weight', 0)
                    w2 = c2.get(':weight', 0)
                    return w2 - w1

        citation['links'].sort(cmp=cmp);



def render(citation, process = False, links = True):
    def jsonify(obj):
        return json.dumps(obj).replace("'", '&#39;').replace('"', '&#34;')
    if process:
        content = ''
    else:
        content = utopia.citation.format(citation)
    return u'''<div class="-papyro-internal-citation" data-citation="{0}" data-process="{1}" data-links="{2}">{3}</div>'''.format(jsonify(citation), jsonify(process), jsonify(links), content)

