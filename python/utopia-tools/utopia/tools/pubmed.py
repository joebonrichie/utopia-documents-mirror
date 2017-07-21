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
import difflib
import htmlentitydefs
import json
import re
import urllib
import urllib2
from lxml import etree



def strict(func):
    def wrapper(*args, **kwargs):
        if None in args or None in kwargs.values():
            return None
        else:
            return func(*args, **kwargs)
    return wrapper


def cache(func):
    _cache = {}
    def wrapper(*args, **kwargs):
        if func in _cache and args == _cache[func].get('previous_args', []) and kwargs == _cache[func].get('previous_kwargs', {}):
            return _cache[func].get('previous_return', None)
        else:
            _cache[func] = {}
            _cache[func]['previous_args'] = args
            _cache[func]['previous_kwargs'] = kwargs.copy()
            _cache[func]['previous_return'] = func(*args, **kwargs)
            return _cache[func]['previous_return']
    return wrapper


@strict
@cache
def _unescape(text):
    def fixup(m):
        text = m.group(0)
        if text[:2] == "&#":
            # character reference
            try:
                if text[:3] == "&#x":
                    return unichr(int(text[3:-1], 16))
                else:
                    return unichr(int(text[2:-1]))
            except ValueError:
                pass
        else:
            # named entity
            try:
                text = unichr(htmlentitydefs.name2codepoint[text[1:-1]])
            except KeyError:
                pass
        return text # leave as is
    return re.sub("&#?\w+;", fixup, text)


@strict
@cache
def _sanitise(title):
    # Strip out naughty characters, because these interfere with the pubmed query
    # FIXME -- is there a better way of escaping these?
    title = re.sub(r'[\[\]:\s()-]+', r' ', title)

    # insert a space between any floating point number and 'A'
    # i.e. "1.5A" => "1.5 A"
    # otherwise pubmed won't find the title
    title = re.sub(r'(\d+\.\d+)A', r'\1 A', title)

    return title

@strict
@cache
def _split(title):
    # Remove all none word characters
    title = re.sub(r'[^\w\d.]+', r' ', title)

    # Insert a space before angstroms
    title = re.sub(r'(\d+\.\d+)A', r'\1 A', title)

    # Tokenise and logically combine them
    title = ' AND '.join((word + '[titl]'
                               for word in title.split()
                               if not word.lower() in utopia.tools.eutils.STOPWORDS))

    return title

@strict
@cache
def _correct(title):
    # Correct title using espell service
    correction = utopia.tools.eutils.espell(term = title)
    xml = etree.fromstring(correction)
    return xml.findtext('CorrectedQuery')


def _fetch(title, munged):
    # Fetch results for title using esearch
    xml = etree.fromstring(utopia.tools.eutils.esearch(term = munged))
    ids = [id.text for id in xml.findall('IdList/Id')]

    title = _sanitise(title)
    found_articles = []

    if len(ids) > 0:
        results = utopia.tools.eutils.efetch(id = ','.join(ids))
        #print results
        xml = etree.fromstring(results)

        for article in xml.findall('PubmedArticle'):
            info = {'identifiers': {}}

            pmid = article.findtext('MedlineCitation/PMID')
            if pmid is not None:
                info['identifiers']['pubmed'] = pmid

            issn = article.findtext('MedlineCitation/Article/Journal/ISSN[@IssnType="Electronic"]')
            if issn is not None:
                info['publication-issn'] = issn

            doi = article.findtext('PubmedData/ArticleIdList/ArticleId[@IdType="doi"]')
            if doi is not None:
                info['identifiers']['doi'] = doi

            pmcid = article.findtext('PubmedData/ArticleIdList/ArticleId[@IdType="pmc"]')
            if pmcid is not None:
                info['identifiers']['pmc'] = pmcid

            pii = article.findtext('PubmedData/ArticleIdList/ArticleId[@IdType="pii"]')
            if pii is not None:
                info['identifiers']['pii'] = pii

            found_title = etree.tostring(article.find('MedlineCitation/Article/ArticleTitle'), method="text", encoding=unicode, with_tail=False)
            info['title'] = found_title
            info['match'] = difflib.SequenceMatcher(None, title, _sanitise(found_title)).ratio()

            found_articles.append(info)

    found_articles = sorted(found_articles, key = lambda article: article['match'], reverse = True)

    # Perhaps we need to include the match criteria once the title scraper is better FIXME
    if len(found_articles) > 0: #  and found_articles[0]['match'] > 0.8
        return found_articles[0]


_attempts = (
    lambda title: _sanitise(title),
    lambda title: _split(title),
    lambda title: _correct(title),
    lambda title: _split(_correct(title)),
)



def fetchXML(pmid):
    return utopia.tools.eutils.efetch(id=pmid)

def search(title):
    title = _unescape(title)
    for munge in _attempts:
        article = _fetch(title, munge(title))
        if article is not None:
            return article
    return {}

def identify(id, field):
    results = utopia.tools.eutils.esearch(term='"{0}"[{1}]'.format(id, field.upper()), retmax=1)
    parser = etree.XMLParser(ns_clean=True, recover=True)
    ids = etree.fromstring(results, parser).xpath('//Id')
    if len(ids) == 1:
        return ids[0].text




_PubmedArticle_fields = {
    'MedlineCitation/Article[1]/ArticleTitle/text()': 'title',
    'MedlineCitation/Article[1]/Journal/Title/text()': 'publication-title',
    'MedlineCitation/Article[1]/Journal/JournalIssue[1]/Volume/text()': 'volume',
    'MedlineCitation/Article[1]/Journal/JournalIssue[1]/Issue/text()': 'issue',
    'MedlineCitation/Article[1]/Journal/JournalIssue[1]/PubDate/Year/text()': 'year',
    'MedlineCitation/Article[1]/Pagination/MedlinePgn/text()': 'pages',
    'MedlineCitation/Article[1]/Abstract/AbstractText/text()': 'abstract',
}

def parse_PubmedArticle(article):
    citation = {}
    for path, key in _PubmedArticle_fields.iteritems():
        try:
            value = article.xpath(path)
            if value is not None and len(value) > 0:
                citation[key] = value[0]
        except:
            raise
        keywords = article.xpath('MedlineCitation/KeywordList/Keyword/text()')
        if len(keywords) > 0:
            citation['keywords'] = keywords
        authors = []
        for author_elem in article.xpath('MedlineCitation/Article/AuthorList/Author'):
            authors.append((u' '.join(author_elem.xpath('LastName/text()')) + u', ' + u' '.join(author_elem.xpath('ForeName/text()'))).strip(', '))
        if len(authors) > 0:
            citation['authors'] = authors
        identifiers = {}
        for issn in article.xpath('MedlineCitation/Article[1]/Journal/ISSN[1]/text()'):
            citation['publication-issn'] = issn
            break
        for id_elem in article.xpath('PubmedData/ArticleIdList/ArticleId'):
            identifiers[id_elem.get('IdType')] = id_elem.text
        if len(identifiers) > 0:
            citation['identifiers'] = identifiers
    return citation




def resolve(pmid):
    response = fetch(pmid)
    xml = etree.fromstring(response)
    articles = xml.findall('PubmedArticle')
    if len(articles) > 0:
        data = parse_PubmedArticle(articles[0])
        data['raw_pubmed_nlm'] = response
        return data


# Legacy
fetch = fetchXML