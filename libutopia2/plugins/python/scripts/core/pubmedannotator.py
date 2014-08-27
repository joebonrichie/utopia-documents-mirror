###############################################################################
#   
#    This file is part of the Utopia Documents application.
#        Copyright (c) 2008-2014 Lost Island Labs
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

# -*- coding: utf-8 -*-

import common.utils
import common.eutils
import spineapi
import utopia.document

from lxml import etree


class _PubmedAnnotator(utopia.document.Annotator):
    """Pubmed NLM Fetch"""

    class JournalXMLParser(object):
        """Simple Parser for NLM Journal Publishing XML"""

        def __init__(self, src):
            from lxml import etree

            # setup tolerant custom parser
            parser = etree.XMLParser(ns_clean=True, recover=True)

            # parse xml file
            self.dom = etree.fromstring(src, parser)

        def tostring(self):
            return etree.tostring(self.dom, encoding='utf8', pretty_print=True)

        def elementAsText(self, element):
            """convert element to *utf-8* text (ignores embedded html tags)"""
            from lxml import etree
            return etree.tostring(element, encoding=unicode, method='text').strip().encode('utf-8')

        def firstElementAsText(self, query, element=None):
            """return first element of list as text or None if empty"""

            if element is None:
                element = self.dom
            try:
                return self.elementAsText(element.xpath(query)[0])
            except IndexError, e:
                return None

        # JOURNAL META DATA

        def journalTitle(self):
            return self.firstElementAsText('/pmc-articleset/article/front/journal-meta/abbrev-journal-title')

        def journalPublisher(self):
            return self.firstElementAsText('/pmc-articleset/article/front/journal-meta/publisher/publisher-name')

        def journalISSN(self, type = 'ppub'):
            return self.firstElementAsText("/pmc-articleset/article/front/journal-meta/issn[@pub-type='%s']" % type)
        # CONTRIBUTOR META DATA

        def articleAuthorAffiliationList(self):

            affiliations = {}
            for aff in self.dom.xpath('/pmc-articleset/article/front/article-meta/contrib-group/aff'):
                affiliations[aff.get('id')] = self.elementAsText(aff)

            results = []
            for author in self.dom.xpath('/pmc-articleset/article/front/article-meta/contrib-group/contrib[@contrib-type="author"]'):
                try:
                    affiliation = affiliations[author.xpath('xref[@ref-type="aff"]/@rid')[0]]
                except (IndexError, KeyError):
                    affiliation = None

                results.append((self.firstElementAsText('name/surname', author),
                                self.firstElementAsText('name/given-names', author),
                                affiliation))
            return results

        def articleAuthors(self):
            return '; '.join(["%s, %s" % (surname, givennames)
                              for surname, givennames, affiliation in self.articleAuthorAffiliationList()])

        # ARTICLE META DATA

        def articleTitle(self):
            return self.firstElementAsText('/pmc-articleset/article/front/article-meta/title-group/article-title')

        def articleDOI(self):
            return self.firstElementAsText('/pmc-articleset/article/front/article-meta/article-id[@pub-id-type="doi"]')

        def articlePMID(self):
            return self.firstElementAsText('/pmc-articleset/article/front/article-meta/article-id[@pub-id-type="pmid"]')

        def articlePublisherID(self):
            return self.firstElementAsText('/pmc-articleset/article/front/article-meta/article-id[@pub-id-type="publisher-id"]')

        def articlePublicationDate(self, type = 'ppub'):
            try:
                date = self.dom.xpath("/pmc-articleset/article/front/article-meta/pub-date[@pub-type='%s']" % type)[0]

                try:
                  year = int(self.firstElementAsText('year', date))
                except TypeError:
                  year = 0

                try:
                  month = int(self.firstElementAsText('month', date))
                except TypeError:
                  month = 0

                try:
                  day = int(self.firstElementAsText('day', date))
                except TypeError:
                  day = 0

                if (year == 0 and month == 0 and day == 0):
                  return "Unknown"
                elif (month == 0):
                  return "%0d" % year
                elif (day == 0):
                  return "%0d-%02d" % (year, month)
                else:
                  return "%04d-%02d-%02d" % (year, month, day)

            except IndexError:
                return None

        def articleVolume(self):
            return self.firstElementAsText('/pmc-articleset/article/front/article-meta/volume')

        def articleIssue(self):
            return self.firstElementAsText('/pmc-articleset/article/front/article-meta/issue')

        def articlePages(self):
            try:
              start = self.firstElementAsText('/pmc-articleset/article/front/article-meta/fpage')
              end = self.firstElementAsText('/pmc-articleset/article/front/article-meta/lpage')
              return (start, end)
            except TypeError:
              return None

        def articleAbstract(self):
            return self.firstElementAsText('/pmc-articleset/article/front/article-meta/abstract')

        def articleKeywordList(self):
            return [self.elementAsText(kwd) for kwd in
                    self.dom.xpath('/pmc-articleset/article/front/article-meta/kwd-group[@kwd-group-type="kwd"]/kwd')]

        def articleAbbreviationsDictionary(self):
            result = {}
            for kwd in self.dom.xpath('/pmc-articleset/article/front/article-meta/kwd-group[@kwd-group-type="abbr"]/kwd'):
                key, _, value = self.elementAsText(kwd).partition(',')
                result[key.strip()] = value.strip()
            return result

        def articleKeywords(self):
            return '; '.join(self.articleKeywordList())

        def articleAbbreviations(self):
            result = []
            for key, value in self.articleAbbreviationsDictionary().iteritems():
                result.append("%s: %s" % (key, value))
            return '; '.join(result)

        # BIBLIOGRAPHIC DATA


        def articleReferenceList(self):
            "Return list of dictionaries of references"
            from lxml import etree

            result = []

            for ref in self.dom.xpath('//back/ref-list/ref'):
                entry = {}
                citation = ref.xpath('(element-citation|mixed-citation|nlm-citation)')
                type = None
                authors = None
                editors = None
                volume = None
                issue = None
                fpage = None
                lpage = None
                year = None
                title = None
                source = None
                publisher = None
                if citation:
                    authorlist = []
                    for person in ref.xpath('(element-citation|mixed-citation|nlm-citation)/person-group[@person-group-type="author"]/name'):
                        authorlist.append("%s, %s" % (self.firstElementAsText('surname', person),
                                                      self.firstElementAsText('given-names', person)))

                    editorlist = []
                    for person in ref.xpath('(element-citation|mixed-citation|nlm-citation)/person-group[@person-group-type="editor"]/name'):
                        editorlist.append("%s, %s" % (self.firstElementAsText('surname', person),
                                                      self.firstElementAsText('given-names', person)))

                    def insertIf(dictionary, key, value):
                        "Helper function to insert non-empty items into a dictionary"
                        if value:
                            dictionary[key] = value

                    insertIf(entry, 'authors', '; '.join(authorlist))
                    insertIf(entry, 'editors', '; '.join(editorlist))
                    insertIf(entry, 'type', citation[0].get('citation-type'))
                    insertIf(entry, 'volume', self.firstElementAsText('(element-citation|mixed-citation|nlm-citation)/volume', ref))
                    insertIf(entry, 'issue', self.firstElementAsText('(element-citation|mixed-citation|nlm-citation)/issue', ref))
                    insertIf(entry, 'start_page', self.firstElementAsText('(element-citation|mixed-citation|nlm-citation)/fpage', ref))
                    insertIf(entry, 'end_page', self.firstElementAsText('(element-citation|mixed-citation|nlm-citation)/lpage', ref))
                    insertIf(entry, 'year', self.firstElementAsText('(element-citation|mixed-citation|nlm-citation)/year', ref))
                    insertIf(entry, 'title', self.firstElementAsText('(element-citation|mixed-citation|nlm-citation)/article-title', ref))
                    insertIf(entry, 'publication-title', self.firstElementAsText('(element-citation|mixed-citation|nlm-citation)/source', ref))
                    insertIf(entry, 'publisher', self.firstElementAsText('(element-citation|mixed-citation|nlm-citation)/publisher-name', ref))
                    insertIf(entry, 'pmid', self.firstElementAsText('(element-citation|mixed-citation|nlm-citation)/pub-id[@pub-id-type="pmid"]', ref))
                    insertIf(entry, 'label', self.firstElementAsText('label', ref))

                    result.append(entry)

            return result

        # TABLE DATA

        def tableList(self):
            "Return list of tables"

            result = []
            for table in self.dom.xpath('//table-wrap'):
                headings = []
                label = self.firstElementAsText('label', table)
                caption = self.firstElementAsText('caption', table)
                for heading in table.xpath('table/thead/tr/th'):
                    pass #TODO
            return result







    def _populate(self, document):
        # Start by seeing what is already known about this document
        nlm = common.utils.metadata(document, 'raw_pmc_nlm')
        doi = common.utils.metadata(document, 'doi')

        if nlm is not None:
            info = self.JournalXMLParser(nlm)

            try:
                nlmdoi = info.articleDOI().lower()
            except: # FIXME which exception(s)?
                nlmdoi = None
                print "PMC returned nothing"

            if doi != nlmdoi:
                print "PMC returned wrong article:", info.articleDOI()
            else:
                print "PMC returned information about article:", info.articleTitle()

                link = document.newAccList('metadata')
                link['property:sourceDatabase'] = 'pubmed'
                link['property:sourceTitle'] = 'PubMed'
                link['property:sourceDescription'] = '<p><a href="http://www.ncbi.nlm.nih.gov/pubmed/">PubMed</a> comprises more than 21 million citations for biomedical literature from MEDLINE, life science journals, and online books.</p>'

                annotation = spineapi.Annotation()
                annotation['concept'] = 'DocumentMetadata'

                # print nlm.articlePublicationDate('epub')
                # print nlm.articlePublicationDate('epreprint')
                # print nlm.journalISSN('epub')

                annotation["property:identifier"] = 'info:doi%s' % info.articleDOI()
                annotation["property:source"] = 'Publisher/NLM'
                annotation["property:curatedBy"] = "PMC"

                annotation["property:journalTitle"] = info.journalTitle()
                annotation["property:journalPublisher"] = info.journalPublisher()
                annotation["property:journalISSN"] = info.journalISSN()
                annotation["property:articleAuthors"] = info.articleAuthors()
                annotation["property:articleTitle"] = info.articleTitle()
                annotation["property:articleDOI"] = info.articleDOI()
                annotation["property:articlePMID"] = info.articlePMID()
                annotation["property:articlePublisherID"] = info.articlePublisherID()
                annotation["property:articlePublicationDate"] = info.articlePublicationDate()
                annotation["property:articleVolume"] = info.articleVolume()
                annotation["property:articleIssue"] = info.articleIssue()
                if info.articlePages() is not None:
                    annotation["property:articlePages"] = "%s-%s" % info.articlePages()
                annotation["property:articleAbstract"] = info.articleAbstract()
                annotation["property:articleKeywords"] = info.articleKeywords()
                annotation["property:articleAbbreviations"] = info.articleAbbreviations()

                document.addAnnotation(annotation, link['scratch'])

                # FIXME: Annotation properties need to be lists
                for surname, forename, aff in info.articleAuthorAffiliationList():
                    annotation = spineapi.Annotation()
                    annotation['concept'] = "AuthorAffiliation"
                    annotation["property:curatedBy"] = "PMC"
                    annotation["property:authorSurname"] = surname
                    annotation["property:authorForename"] = forename
                    annotation["property:articleAuthor"] = "%s, %s" %(surname, forename)
                    annotation["property:affiliation"] = aff
                    document.addAnnotation(annotation, link['scratch'])

                for ref in info.articleReferenceList():
                    annotation = spineapi.Annotation()
                    annotation['concept'] = "DocumentReference"
                    if 'doi' in ref:
                        annotation["property:doi"] = ref['doi']
                    if 'pmid' in ref:
                        annotation["property:pmid"] = ref['pmid']
                    if 'title' in ref:
                        annotation["property:title"] = ref['title']
                    if 'label' in ref:
                        annotation["property:label"] = ref['label']
                    if 'authors' in ref:
                        annotation["property:authors"] = ref['authors']
                    if 'editors' in ref:
                        annotation["property:articleEditors"] = ref['editors']
                    if 'publication-title' in ref:
                        annotation["property:publication-title"] = ref['publication-title']
                    if 'type' in ref:
                        annotation["property:publicationType"] = ref['type']
                    if 'volume' in ref:
                        annotation["property:volume"] = ref['volume']
                    if 'issue' in ref:
                        annotation["property:issue"] = ref['issue']
                    if 'publisher' in ref:
                        annotation["property:publisher"] = ref['publisher']
                    if 'fpage' in ref and 'lpage' in ref:
                        annotation["property:pages"] = "%s-%s" % (ref['fpage'],ref['lpage'])
                    if 'year' in ref:
                        annotation["property:year"] = ref['year']

                    document.addAnnotation(annotation, link['scratch'])
