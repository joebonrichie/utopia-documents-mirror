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
import kend.client
import kend.model
import utopia.library
from lxml import etree


class KendRemoteQuery(utopia.library.RemoteQuery):
    """Search the kend server for articles matching the search criteria"""

    id_name_map = {
        'doi': 'doi',
        'pmid': 'pubmed',
        'pmcid': 'pmc',
        'arxivid': 'arxiv',
        'pii': 'pii',
    }

    def fetch(self, query, offset, limit):
        # Get the date range to search in
        last_searched = self.get_property('last_searched', '1970/01/01')
        today = datetime.date.today().strftime('%Y/%m/%d')

        # Get previous search metadata
        next_search_uri = self.get_property('next_search_uri')
        count = int(self.get_property('count', 0))
        offset = int(self.get_property('offset', 0))
        limit = int(self.get_property('limit', 100))
        last_id = self.get_property('last_id')

        # Unpack the search term(s)
        term = query.get('query')
        if term is not None:
            # Remove non-hashtags
            term = ' '.join((t for t in term.split() if t.startswith('#')))

            # List to hold the results
            results = []

            # If no previous search exists, search now
            if next_search_uri is None:
                results = []
                documents = kend.client.Client().searchDocuments(**{
                    'q': term,
                    'offset': unicode(offset),
                    'limit': unicode(limit),
                })
                for group in documents:
                    for k, v in group.output:
                        if k == 'next':
                            next_search_uri = v
                        elif k == 'count':
                            count = int(v)
                        elif k == 'offset':
                            offset = int(v)
                        elif k == 'limit':
                            limit = int(v)
                    for document in group.documents:
                        info = {}
                        metadata = {}
                        weights = {'crossref': 20, 'arxiv': 19, 'pubmed': 10, 'pmc': 9, 'document': 0}
                        for e in document.metadata:
                            metadata.setdefault(e.type, [])
                            metadata[e.type].append((e.data, weights.get(e.src, 5)))
                        for k, vs in metadata.iteritems():
                            vs.sort(key=lambda e: e[1])
                            v = vs[-1][0]
                            if k in self.id_name_map:
                                info.setdefault('identifiers', {})
                                info['identifiers'][self.id_name_map.get(k)] = v
                            elif k == 'identifiers':
                                info.setdefault('identifiers', {})
                                if v.startswith('json:'):
                                    v = v[5:]
                                info['identifiers'].update(json.loads(v))
                            else:
                                info[k] = v
                        if 'authors' in info:
                            info['authors'] = info['authors'].split('; ')
                        results.append(info)

                # Store history information
                self.set_property('next_search_uri', next_search_uri)
                self.set_property('count', count)
                self.set_property('offset', offset)
                self.set_property('limit', limit)

                # Return results
                return offset, limit, count, results
