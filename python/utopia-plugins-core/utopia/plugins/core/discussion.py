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

import utopia.tools.utils
import re
import spineapi
import urllib
import urllib2
import utopia.document
import json
from datetime import datetime
from collections import defaultdict

class ExternalDiscussionAnnotator(utopia.document.Annotator):
    """Display discussion threads relating to article"""

    def _get_discussion(self, identifiers):

        url="http://utopia.cs.manchester.ac.uk/lazarus/0.1/document/discussion"
        query = urllib.urlencode(dict(identifiers=json.dumps(identifiers)))
        req = urllib2.Request("{}?{}".format(url, query))
        response = urllib2.urlopen(req, timeout=20)
        return json.loads(response.read())

    def _format_header(self, comment):

        header = u''

        author = comment.get('author', dict(name='Anonymous'))

        author_name = author.get('name','')
        if 'uri' in author:
            author_name=u'<a href="{}">{}</a>'.format(author['uri'], author_name)
        header += '<div class="author">' + author_name + '</div>'

        if 'timestamp' in comment:
            comment_time = u'{}'.format(datetime.strftime(
                datetime.fromtimestamp(int(comment.get('timestamp'))), '%c'))
            if 'uri' in comment:
                comment_time = u'<a href="{}">{}</a>'.format(comment['uri'], comment_time)
            header += '<div class="timestamp">' + comment_time + '</div>'

        return header

    def _identifiers_to_key(self, identifiers):
        return '_'.join('{}:{}'.format(k,v) for k, v in sorted(identifiers.iteritems()))

    @utopia.document.buffer
    def on_ready_event(self, document):
        css = '''
        <style>
            .discussion .author {
                font-weight: bold;
            }
            .discussion .timestamp {
                font-size: 0.9em;
                font-style: italic;
            }
            .discussion .author a {
                color: black !important;
            }
            .discussion .timestamp a {
                color: inherit !important;
            }
        </style>
        '''

        identifiers = utopia.tools.utils.metadata(document, 'identifiers')
        if identifiers is None or len(identifiers) == 0:
            return

        discussions=self._get_discussion(identifiers)

        if discussions is None or not isinstance(discussions, dict):
            return

        for discussion_source in discussions.get('discussions', []):

            comments=[]
            mentions={}

            for comment in sorted(discussion_source.get('comments', []),
                                  key=lambda c:c['timestamp']):

                comments.append(u'<div class="box commnt limited-height">{}<p></p>{}</div>'.format(
                    self._format_header(comment), comment['content']))


            for mention in discussion_source.get('mentions', []):

                key=self._identifiers_to_key(mention['identifiers'])

                mentions[key]=mention['identifiers']

            if not mentions and not comments:
                continue

            html = u'<div class="discussion">'
            a = spineapi.Annotation()

            a['concept'] = 'Collated'
            a['property:name'] = discussion_source['source']['title']
            a['property:sourceDatabase'] = discussion_source['source']['title']
            a['property:sourceDescription'] = '<p>{}</p>'.format(discussion_source['source']['description'])
            a['property:sourceIcon'] = discussion_source['source']['icon']

            a['property:description'] = 'Comments related to this article'

            if comments:

                html+=u''
                html+=u'\n\n'.join(comments)

            if mentions:

                mention_html=u'\n\n'.join([utopia.citation.render(dict(identifiers=citation), process=True, links=True) for citation in mentions.itervalues()])

                html+=u'<div class="box"><p>This article was mentioned by a comment in:</p>{}</div>\n\n'.format(mention_html)

            html += '</div>'
            a['property:html'] = css, html

            document.addAnnotation(a)
