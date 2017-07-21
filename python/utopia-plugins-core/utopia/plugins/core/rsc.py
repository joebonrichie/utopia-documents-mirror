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

#? name: Royal Society of Chemistry
#? www: http://www.rsc.org/
#? urls: http://www.rsc.org/ http://www.chemspider.com/ http://goldbook.iupac.org/


import spineapi
import urllib
import urllib2
from lxml import etree
import utopia.document

class RSCAnnotator(utopia.document.Annotator):
    '''Annotate with RSC'''

    @utopia.document.buffer
    def after_ready_event(self, document):
        # Get (if present) the RSCMetadataLink annotation
        for annotation in document.annotations():
            if annotation.get('concept') == 'RSCMetadataLink':
                text = document.text()

                doi = annotation['property:doi'].upper()
                rscId = annotation['property:rscId'].upper()

                xmlquery = '<SearchCriteria><SearchTerm><Category>Journal</Category><ContentType>All</ContentType><Criterias><NameValue><Name>FreeText</Name><Value>"%s"</Value></NameValue></Criterias><Source>Utopia</Source></SearchTerm><PageNo>1</PageNo><PageSize>10</PageSize><SortBy>Relevance</SortBy></SearchCriteria>' % doi

                baseurl = 'http://pubs.rsc.org/en/federated/search'
                params = { 'federatedsearchname': 'Utopia',
                           'inputxml': xmlquery }
                url = baseurl + '?%s' % urllib.urlencode(params)
                searchresult = urllib2.urlopen(url, timeout=14).read()
                root = etree.fromstring(searchresult)
                #print etree.tostring(root, pretty_print=True, encoding='utf8')

                articles = root.findall('./{http://www.rsc.org/schema/rscart38}article')
                #print articles

                # the search use above can return more than one article, so select out only the one with
                # the correct doi

                thearticle = None
                articleID = None
                for article in articles:
                    found_doi = article.findtext("./{http://www.rsc.org/schema/rscart38}metainfo/{http://www.rsc.org/schema/rscart38}meta[@field='doi']")
                    if found_doi is None:
                        found_doi = article.findtext("./{http://www.rsc.org/schema/rscart38}art-admin/{http://www.rsc.org/schema/rscart38}doi")
                    if found_doi is not None and found_doi.upper() == doi:
                        thearticle = article
                        articleIDelem = article.find("./{http://www.rsc.org/schema/rscart38}metainfo/{http://www.rsc.org/schema/rscart38}meta[@field='docid']")
                        if articleIDelem is not None:
                            articleID = articleIDelem.text
                        break

                # if we get back a single valid article...
                if thearticle != None:
                    #print articleID

                    compoundsInArticle = []
                    compoundText = {}

                    annotationsInArticle = []
                    annotationText = {}

                    # create a list of all the compounds that are mentioned in the article body
                    compnames = thearticle.findall('./{http://www.rsc.org/schema/rscart38}art-body/{http://www.rsc.org/schema/rscart38}compname')
                    #print compnames
                    for compname in compnames:
                        # This line removes (erroneous?) elements from inside the XML
                        etree.strip_elements(compname, '{http://www.rsc.org/schema/rscart38}compound', with_tail=False)
                        #print compname.attrib['idrefs'], compname.text
                        compoundsInArticle.append(compname.attrib['idrefs'])
                        compoundText[compname.attrib['idrefs']] = etree.tounicode(compname, method='text')

                    annotationnames = thearticle.findall('./{http://www.rsc.org/schema/rscart38}art-body/{http://www.rsc.org/schema/rscart38}annref')
                    #print annotationnames
                    for annotationname in annotationnames:
                        # This line removes (erroneous?) elements from inside the XML
                        etree.strip_elements(annotationname, '{http://www.rsc.org/schema/rscart38}annotation', with_tail=False)
                        #print annotationname.attrib['idrefs'], annotationname.text
                        annotationsInArticle.append(annotationname.attrib['idrefs'])
                        annotationText[annotationname.attrib['idrefs']] = etree.tounicode(annotationname, method='text')

                    #print compoundText, annotationText
                    #sprint annotationsInArticle

                    # then for all the compounds that are defined in the article back
                    compounds = thearticle.findall('./{http://www.rsc.org/schema/rscart38}art-back/{http://www.rsc.org/schema/rscart38}compound')
                    for compound in compounds:
                        id = compound.attrib['id']
                        if id in compoundsInArticle:
                            url = None
                            id = compound.attrib['id']

                            # if the compound has a CSID, then the URL links to the chemspider page
                            csid = compound.find("./{http://www.rsc.org/schema/rscart38}link[@type='CSID']" )

                            # if the compound has a CSID, create a Chemspider URL for it
                            if csid is not None and csid.text is not None:
                                url = 'http://www.chemspider.com/Chemical-Structure.%s.html' % csid.text[5:]
                            else:
                                # otherwise, use the RSC landing page
                                url = 'http://www.rsc.org/publishing/journals/prospect/cheminfo.asp?XMLID=%s&compoundtext=%s&MSID=%s' % (id[4:], compoundText[id], articleID)

                            if url is not None:
                                options = spineapi.WholeWordsOnly + spineapi.IgnoreCase
                                matches = document.search(compoundText[id], options)
                                annotation = spineapi.Annotation()
                                annotation['concept'] = 'Hyperlink'
                                annotation['property:webpageUrl'] = url
                                for match in matches:
                                    annotation.addExtent(match)
                                document.addAnnotation(annotation)

                    # similarly, for all the annotations
                    annotations = thearticle.findall('./{http://www.rsc.org/schema/rscart38}art-back/{http://www.rsc.org/schema/rscart38}annotation')
                    for annotation in annotations:
                        id = annotation.attrib['id']
                        url = None
                        if id in annotationsInArticle:
                            id = annotation.attrib['id']

                            # get the link id
                            link = annotation.findtext("./{http://www.rsc.org/schema/rscart38}link" )

                            # if the compound has a link, create an RSC ontology landing page for it
                            if link is not None:
                                if link[:3] == 'AU:':
                                    url = 'http://goldbook.iupac.org/%s.html' % link[3:]
                                else:
                                    url = 'http://www.rsc.org/publishing/journals/prospect/ontology.asp?id=%s&MSID=%s' % (link, articleID)

                            if url is not None:
                                matches = document.search(annotationText[id], spineapi.IgnoreCase + spineapi.WholeWordsOnly)
                                annotation = spineapi.Annotation()
                                annotation['concept'] = 'Hyperlink'
                                annotation['property:webpageUrl'] = url
                                for match in matches:
                                    annotation.addExtent(match)
                                document.addAnnotation(annotation)
                break
