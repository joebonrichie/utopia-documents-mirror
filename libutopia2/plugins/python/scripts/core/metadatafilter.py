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

import common.arxiv
import common.doi
import common.utils
import re
import spineapi
import urllib
import utopia.document



# Make sure labels are sorted numerically (such that 10 > 2)
def sortfn(c):
    v = c.get('property:label')
    if v is not None:
        num = re.match('[0-9]*', v).group()
        if len(num) > 0:
            v = re.sub('^[0-9]*', '{0:020d}'.format(int(num)), v)
    else:
        v = c.get('property:authors', c.get('property:id', '0')).lower()
    return v


class MetadataAnnotator(utopia.document.Annotator):
    """Metadata filter"""

    def populate(self, document):
        # Scrape title and DOI from document
        title = common.utils.metadata(document, 'title', '')
        doi = common.utils.metadata(document, 'doi', '')
        if len(title) > 0 or len(doi) > 0:
            # Make metadata link
            link = spineapi.Annotation()
            link['session:volatile'] = '1'
            link['concept'] = 'MetadataSource'
            link['rank'] = '1000'
            link['source'] = 'Content'
            link['listName'] = 'ContentMetadata'
            document.addAnnotation(link)

            # Store actual metadata
            annotation = spineapi.Annotation()
            annotation['session:volatile'] = '1'
            annotation['concept'] = 'DocumentMetadata'
            annotation['property:source'] = 'Content'
            if len(title) > 0:
                annotation['property:title'] = title
            if len(doi) > 0:
                annotation['property:doi'] = doi
            document.addAnnotation(annotation, link['listName'])


    def reducePopulate(self, document):
        print 'Formatting metadata'

        # Find highest matching metadata accumulation list for references
        source = None
        for accListLink in document.getAccLists('metadata'):
            matches = document.annotationsIf({'concept': 'DocumentReference'}, accListLink['scratch'])
            if len(matches) > 0:
                print 'Selected for [DocumentReference] list %s with rank %s' % (accListLink['scratch'], repr(accListLink.get('rank', 0)))
                source = accListLink
                bibliography = list(matches)
                bibliography.sort(key=sortfn)
                rt=''
                for annotation in bibliography:
                    links = []
                    type = annotation.get('property:type')
                    doi = annotation.get('property:doi')
                    pmid = annotation.get('property:pmid')
                    pmcid = annotation.get('property:pmcid')
                    title = annotation.get('property:title')
                    url = annotation.get('property:url')
                    sourceTitle = annotation.get('property:source', '')
                    if 'property:displayText' in annotation:
                        displayText = annotation['property:displayText']
                    else:
                        displayText = common.utils.format_citation(annotation)
                    displayTextStripped = re.sub(r'<[^>]*>', '', displayText)
                    if type == 'other':
                        doi = doi or common.doi.search(displayTextStripped)
                        if doi is not None:
                            links.append({'url': 'http://dx.doi.org/{0}'.format(doi),
                                          'title': 'See article on publisher\'s website...',
                                          'name': 'Link'})
                        else:
                            links.append({'url': 'http://scholar.google.com/scholar?{0}'.format(urllib.urlencode({'q': displayTextStripped.encode('utf8')})),
                                          'title': 'Search for article...',
                                          'name': 'Find'})
                    else:
                        if url is not None:
                            links.append({'url': url,
                                          'title': url,
                                          'name': 'Link'})
                        elif sourceTitle.lower().startswith('arxiv:'):
                            links.append({'url': common.arxiv.url(sourceTitle[6:]),
                                          'title': 'See article in arXiv...',
                                          'name': 'Link'})
                        if pmcid is not None:
                            links.append({'url': 'http://www.ncbi.nlm.nih.gov/pmc/articles/{0}'.format(pmcid),
                                          'title': 'See article in PubMed Central...',
                                          'name': 'Link'})
                            links.append({'url': 'http://www.ncbi.nlm.nih.gov/pmc/articles/{0}/pdf/'.format(pmcid),
                                          'title': 'Download article...',
                                          'target': 'pdf',
                                          'name': 'PDF'})
                        elif doi is not None:
                            links.append({'url': 'http://dx.doi.org/{0}'.format(doi),
                                          'title': 'See article on publisher\'s website...',
                                          'name': 'Link'})
                        elif pmid is not None:
                            links.append({'url': 'http://www.ncbi.nlm.nih.gov/pubmed/{0}'.format(pmid),
                                          'title': 'See article in PubMed...',
                                          'name': 'Link'})
                        elif len(links) == 0 and title is not None:
                            links.append({'url': 'http://scholar.google.co.uk/scholar?q={0}'.format(urllib.quote(title.encode('utf8'))),
                                          'title': 'Search for article in Google Scholar...',
                                          'name': 'Find'})
                        elif len(links) == 0 and displayTextStripped:
                            print 'http://scholar.google.com/scholar?{0}'.format(urllib.urlencode({'q': displayTextStripped.encode('utf8')}))
                            links.append({'url': 'http://scholar.google.com/scholar?{0}'.format(urllib.urlencode({'q': displayTextStripped.encode('utf8')})),
                                          'title': 'Search for article...',
                                          'name': 'Find'})
                    anchors = ''
                    if len(links) > 0:
                        for link in links:
                            target = link.get('target', '')
                            if len(target) > 0:
                                target = ' target="{0}"'.format(target)
                            anchors += u' <a href="{0}" title="{1}"{2}>[{3}]</a>'.format(link.get('url'), link.get('title'), target, link.get('name'))
                    rt += u'<div class="box">{0}{1}</div>'.format(displayText, anchors)

                if len(rt) > 0:
                    references=spineapi.Annotation()
                    references['displayBibliography']=rt
                    references['concept']='BibliographyMetadata'
                    references['property:identifier']='#bibliography'
                    references['property:name']='Bibliography'
                    references['displayName']='Bibliography'
                    references['displayRelevance']='800'
                    if accListLink is not None:
                        for i in ('sourceIcon', 'sourceTitle', 'sourceDescription', 'sourceDatabase'):
                            k = 'property:{0}'.format(i)
                            if k in accListLink:
                                references[k] = accListLink[k]
                        references['property:description'] = 'From ' + accListLink['property:sourceTitle']
                    document.addAnnotation(references)
                break
        if source is None:
            print 'No metadata found'



        # Find highest matching metadata accumulation list for in-text citations
        for accListLink in document.getAccLists('metadata'):
            matches = document.annotationsIf({'concept': 'ForwardCitation'}, accListLink['scratch'])
            if len(matches) > 0:
                print 'Selected for [ForwardCitation] list %s with rank %s' % (accListLink['scratch'], repr(accListLink.get('rank', 0)))
                document.addAnnotations(matches)
                break


        # Find highest matching metadata accumulation list for in-text citations
        for accListLink in document.getAccLists('metadata'):
            matches = document.annotationsIf({'concept': 'Table'}, accListLink['scratch'])
            if len(matches) > 0:
                print 'Selected for [Table] list %s with rank %s' % (accListLink['scratch'], repr(accListLink.get('rank', 0)))
                document.addAnnotations(matches)
                break





        metadata=None
        if source is not None:
            for annotation in document.annotations(source['scratch']):
                if annotation.get('concept')=='DocumentMetadata':
                    metadata=annotation
            if metadata:
                metadata['displayName']='Document Information'
                metadata['displayRelevance']='1000'
                document.addAnnotation(metadata, 'Document Metadata')

