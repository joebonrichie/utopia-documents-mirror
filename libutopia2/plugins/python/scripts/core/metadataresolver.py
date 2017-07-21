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

import datetime
import json
import kend
import re
import urllib2
import urlparse
import utopia.citation
import utopia.tools.arxiv
import utopia.tools.crossref
import utopia.tools.pmc
import utopia.tools.pubmed
import utopia.tools.utils
import utopia.tools.title

from lxml import etree
from StringIO import StringIO



class _LoggingResolver(utopia.citation.Resolver):
    """Log citations"""

    def resolve(self, citations, document = None):
        import pprint
        pp = pprint.PrettyPrinter(indent=4)
        pp.pprint(citations)

    def purposes(self):
        return ('dereference', 'identify', 'expand')

    def weight(self):
        return 10000


class _WaitingResolver(utopia.citation.Resolver):
    """Wait for a while (for testing purposes)"""

    def resolve(self, citations, document = None):
        import time
        for i in xrange(0, 80):
            time.sleep(0.5)

    def purposes(self):
        return ('dereference', 'identify', 'expand')

    def weight(self):
        return 10001


class _ProvenanceResolver(utopia.citation.Resolver):
    """Ensures provenance is available"""

    def resolve(self, citations, document = None):
        anon_user_id = utopia.utopiabridge.context['anonymous-user-id']
        now = datetime.datetime.now().replace(microsecond=0).isoformat()

        provenance = utopia.citation.pick_from(citations, 'provenance/origin', None)
        if provenance is None:
            uri = utopia.citation.pick_from(citations, 'originating-uri', None)
            if uri is not None:
                if uri.startsWith('file:'):
                    provenance = {
                        'action': 'openfile',
                        'anonymous-user-id': anon_user_id,
                        'time': now
                    }
                elif uri.startsWith('http:') or uri.startsWith('https:'):
                    domain_name = urlparse.urlparse(uri).hostname
                    provenance = {
                        'action': 'download',
                        'anonymous-user-id': anon_user_id,
                        'time': now,
                        'context': {
                            'domain': domain_name
                        }
                    }
            ### Not sure if the following should be done
            #if provenance is not None:
            #    return {'provenance': {'origin': provenance}}

        import pprint
        pp = pprint.PrettyPrinter(indent=4).pprint
        pp(provenance)

    def provenance(self):
        return {'whence': 'utopia'}

    def purposes(self):
        return ('dereference', 'identify', 'expand')

    def weight(self):
        return -10000
