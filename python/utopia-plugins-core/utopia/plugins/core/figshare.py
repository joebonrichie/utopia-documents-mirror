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

#? name: FigShare
#? www: http://figshare.com/
#? urls: http://api.figshare.com/ http://figshare.com/


import utopia.tools.utils
import json
import spineapi
import urllib
import utopia.document

import urllib2 #, oauth_auth, oauth
from lxml import etree


def flatten(html):
    return etree.tostring(etree.HTML('<html>' + html + '</html>'), method='text', encoding='unicode')


class FigShareAnnotator(utopia.document.Annotator):
    '''Connect to FigShare service.'''

    def on_ready_event(self, document):
        doi = utopia.tools.utils.metadata(document, 'identifiers[doi]')
        if doi is not None:
            page = 1
            items_retrieved = 0
            api_search_url = 'http://api.figshare.com/v1/articles/search?'
            query = {'search_for': doi, 'has_link': doi}

            htmls = []

            while True:
                query['page'] = page

                #handler = oauth_auth.HTTPOauthAuthHandler()
                #consumer = oauth.KeySecret('yeiB61W0PYaUGPhhi8pBhA', 'LTOVWR94y8YZwscJhrFg0w')
                #token = oauth.KeySecret('jsWfvZBLPgNMRMjFaQOMbgAst4Rh5LzWmTMDD4HkHOpAjsWfvZXLPgNMRMjFaQOMbg', 'm8iqNc7AQH9Yrqa6e0H5AA')
                #handler.add_password(None, 'api.figshare.com', consumer, token)
                #opener = urllib2.build_opener(handler)

                #print api_search_url + urllib.urlencode(query)
                #response = opener.open(api_search_url + urllib.urlencode(query)).read()
                response = urllib2.urlopen(api_search_url + urllib.urlencode(query), timeout=8).read()
                data = json.loads(response)

                items = data.get('items', [])
                items_found = int(data.get('items_found', 0))

                # Bail if no items found
                if len(items) == 0 or items_found <= 0:
                    break

                # Bail after ten pages of stuff
                if page > 10:
                    break

                items_retrieved += len(items)

                for item in items:
                    title = flatten(item.get('title'))
                    description = flatten(item.get('description'))
                    links = item.get('links', [])
                    url = item.get('url')
                    item_doi = item.get('DOI')
                    article_id = item.get('article_id')
                    authors = item.get('authors', [])
                    published_date = item.get('published_date')
                    type = item.get('type')

                    html = u'''
                      <div id="{article_id}" class="box">
                        <p>
                          <span class="title">{title}</span>
                          <span class="authors">{authors}</span>
                          <a href="{url}" title="Explore FigShare">[Link]</a>
                        </p>
                        <p class="readmore">
                          {description}
                        </p>
                      </div>
                    '''.format(**{
                        'article_id': article_id,
                        'title': title,
                        'url': item_doi,
                        'description': description,
                        'authors': u', '.join((author['author_name'] for author in authors))
                    })

                    htmls.append(html)

                # Stop if we've retrieved the number of items expected
                if items_retrieved >= items_found:
                    break

                page += 1

            if len(htmls) > 0:
                annotation = spineapi.Annotation()
                annotation['concept'] = 'FigShareReference'
                annotation['property:html'] = ''.join(htmls)
                annotation['property:name'] = 'FigShare'
                annotation['property:description'] = 'Publicly available supplementary material'
                annotation['property:sourceDatabase'] = 'figshare'
                annotation['property:sourceDescription'] = '<p><a href="http://figshare.com/">fig<strong>share</strong></a> allows researchers to publish all of their research outputs in seconds in an easily citable, sharable and discoverable manner.</p>'
                document.addAnnotation(annotation)



class FigShareVisualiser(utopia.document.Visualiser):
    '''Visualise FigShare references.'''

    def visualisable(self, annotation):
        return annotation.get('concept') == 'FigShareReference'

    def visualise(self, annotation):
        return ('<style>#figshare .authors { font-style: italic }</style>',
                '<div id="figshare">' + annotation['property:html'] + '</div>')

