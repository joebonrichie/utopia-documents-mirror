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

#? name: CrossRef
#? www: http://www.crossref.org/
#? urls: http://crossref.org/ http://dx.doi.org/


import json
import re
import socket
import urllib

import urllib2
from lxml import etree


api_key = 'API_KEY'

def fetch(doi):
    url = 'http://dx.doi.org/{0}'.format(doi)
    headers = { 'Accept': 'application/unixref+xml' }
    request = urllib2.Request(url, None, headers)
    return urllib2.urlopen(request, timeout=5).read()

def resolve(doi):
    data = {}

    try:
        response = fetch(doi)
    except (urllib2.URLError, socket.timeout):
        return data

    data['raw_crossref_unixref'] = response
    dom = etree.fromstring(response)

    # Find as much metadata as possible from this crossref record

    # Title of the article
    titleelem = dom.find('doi_record/crossref/journal/journal_article/titles/title')
    if titleelem is not None:
        title = etree.tostring(titleelem, method="text", encoding=unicode)
        if title is not None:
            data['title'] = re.sub(r'\s+', ' ', title)

    # Authors of the article
    persons = dom.findall('doi_record/crossref/journal/journal_article/contributors/person_name[@contributor_role="author"]')
    if len(persons) > 0:
        data['authors'] = [(person.findtext('surname') + ', ' + person.findtext('given_name')).strip(', ') for person in persons]

    # Favour electronic ISSN FIXME should this print one even be used?
    issn = dom.findtext('doi_record/crossref/journal/journal_metadata/issn[@media_type="electronic"]')
    if issn is None:
        issn = dom.findtext('doi_record/crossref/journal/journal_metadata/issn')
    if issn is not None:
        if len(issn) == 8:
            issn = '{0}-{1}'.format(issn[:4], issn[-4:])
        data['issn'] = issn

    def findtext(xpath, field):
        valueelem = dom.find(xpath)
        if valueelem is not None:
            value = etree.tostring(valueelem, method="text", encoding=unicode, with_tail=False)
            if value is not None:
                data[field] = re.sub(r'\s+', ' ', value)

    findtext('doi_record/crossref/journal/journal_metadata/full_title', 'publication-title')
    findtext('doi_record/crossref/journal/journal_issue/publication_date/year', 'year')
    findtext('doi_record/crossref/journal/journal_issue/journal_volume/volume', 'volume')
    findtext('doi_record/crossref/journal/journal_issue/issue', 'issue')
    findtext('doi_record/crossref/journal/journal_article/titles/title', 'title')
    findtext('doi_record/crossref/journal/journal_article/pages/first_page', 'first_page')
    findtext('doi_record/crossref/journal/journal_article/pages/last_page', 'last_page')
    findtext('doi_record/crossref/journal/journal_article/publisher_item/identifier[@id_type="pii"]', 'pii')

    data['url'] = 'http://dx.doi.org/{0}'.format(doi)

    pages = u'-'.join((p for p in (data.get('first_page'), data.get('last_page')) if p is not None))
    if len(pages) > 0:
        data['pages'] = pages

    return data

def search(title):
    data = []
    url = 'http://crossref.org/sigg/sigg/FindWorks?{0}'.format(urllib.urlencode({
        'version': '1',
        'access': api_key,
        'format': 'json',
        'op': 'OR',
        'expression': title.encode('utf8'),
    }))
    response = urllib2.urlopen(url, timeout=8).read()
    data = json.loads(response)
    return data
