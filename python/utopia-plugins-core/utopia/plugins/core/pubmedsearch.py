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

import utopia.tools.eutils
import utopia.tools.pubmed
import datetime
import utopia.library
from lxml import etree


class PubMedRemoteQuery(utopia.library.RemoteQuery):
    """PubMed search"""

    def parse(self, article):
        #print etree.tostring(article, pretty_print=True, encoding='utf8')
        info = utopia.tools.pubmed.parse_PubmedArticle(article)
        if len(info) > 0:
            # For PubMed entries, the uid is the PubMed ID
            unique_id = info.get('identifiers', {}).get('pubmed')
            if unique_id is not None:
                info['__id__'] = 'pubmed:{0}'.format(unique_id)
        return info

    def fetch(self, query, offset, limit):
        # Get the date range to search in
        last_searched = self.get_property('last_searched', '1970/01/01')
        today = datetime.date.today().strftime('%Y/%m/%d')

        # Get previous search metadata
        webEnv = self.get_property('WebEnv')
        queryKey = self.get_property('QueryKey')
        count = int(self.get_property('Count', 0))
        last_id = self.get_property('last_id')

        #print webEnv, queryKey, count, last_id

        # Known domains
        domain_map = {
            'title': 'Title',
            'authors': 'Author',
            'abstract': 'Title/Abstract',
        }

        # Unpack the search term(s)
        domain = domain_map.get(query.get('domain'), '')
        term = query.get('query')
        if term is not None:
            # Remove hashtags
            term = u' '.join((t for t in term.split() if not t.startswith('#')))
        if term is not None and len(term) > 0:
            # Remove hashtags
            term = u' '.join((t for t in term.split() if not t.startswith('#')))

            # List to hold the results
            results = []

            # If no previous search exists, search now
            if webEnv is None:
                # Fetch from PubMed any matching documents
                searchresult = utopia.tools.eutils.esearch(**{
                    'db': 'pubmed',
                    'retmax': limit,
                    'term': term,
                    'field': domain,
                    'datetype': 'pdat',
                    'mindate': last_searched,
                    'maxdate': today,
                })
                root = etree.fromstring(searchresult)
                #print etree.tostring(root, pretty_print=True, encoding='utf8')

                # Store history information
                webEnv = root.findtext('WebEnv')
                queryKey = root.findtext('QueryKey')
                count = int(root.findtext('Count'))
                self.set_property('WebEnv', webEnv)
                self.set_property('QueryKey', queryKey)
                self.set_property('Count', count)
            else:
                # If previous search exists, then get everything from then until now
                searchresult = utopia.tools.eutils.esearch(**{
                    'db': 'pubmed',
                    'retmax': limit,
                    'term': term,
                    'field': domain,
                    'datetype': 'pdat',
                    'mindate': last_searched,
                    'maxdate': today,
                })
                root = etree.fromstring(searchresult)
                #print etree.tostring(root, pretty_print=True, encoding='utf8')
                newWebEnv = root.findtext('WebEnv')
                newQueryKey = root.findtext('QueryKey')
                newCount = int(root.findtext('Count'))
                exhausted = False
                for newOffset in xrange(0, newCount, limit):
                    if exhausted:
                        break

                    # Fetch the next set of new results
                    #print "-- fetching", newOffset, limit, newWebEnv, newQueryKey
                    searchresult = utopia.tools.eutils.efetch(**{
                        'db': 'pubmed',
                        'retmax': limit,
                        'WebEnv': newWebEnv,
                        'query_key': newQueryKey,
                        'retstart': newOffset,
                    })

                    #print searchresult
                    root = etree.fromstring(searchresult)
                    #print etree.tostring(root, pretty_print=True, encoding='utf8')
                    #print etree.tostring(root.find('PubmedArticle'), pretty_print=True, encoding='utf8')

                    for article in root.findall("PubmedArticle"):
                        info = self.parse(article)
                        if info.get('__id__') == last_id:
                            exhausted = True
                            break
                        if len(info) > 0:
                            info['flags'] = ['unread']
                            #print info
                            results.append(info)
                    if len(results) > 0:
                        if '__id__' in results[0]:
                            last_id = results[0]['__id__']


            # Fetch the next set of results
            #print "-- fetching", offset, limit, webEnv, queryKey
            searchresult = utopia.tools.eutils.efetch(**{
                'db': 'pubmed',
                'retmax': limit,
                'WebEnv': webEnv,
                'query_key': queryKey,
                'retstart': offset,
            })

            #print searchresult
            root = etree.fromstring(searchresult)
            #print etree.tostring(root, pretty_print=True, encoding='utf8')
            #print etree.tostring(root.find('PubmedArticle'), pretty_print=True, encoding='utf8')

            if root.find('ERROR') is None:
                for article in root.findall("PubmedArticle"):
                    info = self.parse(article)
                    if len(info) > 0:
                        #print info
                        results.append(info)

                if last_id is None and len(results) > 0:
                    if '__id__' in results[0]:
                        last_id = results[0]['__id__']
                        self.set_property('last_id', last_id)

                # Reset last searched date to today
                self.set_property('last_searched', today)

                #print (offset, limit, count, results)
                return (offset, limit, count, results)
            else:
                print etree.tostring(root, pretty_print=True, encoding='utf8')

        # Default
        return (0, 0, 0, [])
