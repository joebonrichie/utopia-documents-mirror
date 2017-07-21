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
import urllib
import urllib2
import utopia.citation

def resolve(**identifiers):
    query = urllib.urlencode(identifiers)
    url = 'https://utopia.cs.manchester.ac.uk/cited/resolutions?{}'.format(query)
    response = urllib2.urlopen(url, timeout=8)
    citations = json.load(response)
    return citations

def submit(citations):
    citations = [citation for citation in citations if 'error' not in citation]
    url = 'https://utopia.cs.manchester.ac.uk/cited/resolutions'
    req = urllib2.Request(url,
                          headers={'Content-Type': 'application/json'},
                          data=json.dumps(citations))

    for c in citations:
        p = c.get('provenance', {})
        if p is not None:
            for refspec in p.get('input', []):
                id, keyspec = utopia.citation.split_refspec(refspec)

    res = urllib2.urlopen(req)
    citations = json.load(res)
    return citations

def parse(citations):
    url = 'https://utopia.cs.manchester.ac.uk/cited/parse'
    req = urllib2.Request(url,
                          headers={'Content-Type': 'application/json'},
                          data=json.dumps(citations))
    res = urllib2.urlopen(req)
    citations = json.load(res)
    return citations
