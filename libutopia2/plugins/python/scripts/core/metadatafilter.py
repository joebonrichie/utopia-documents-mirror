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

import utopia.tools.arxiv
import utopia.tools.doi
import utopia.tools.utils
import re
import urllib
import utopia.document



try:
    import spineapi
except ImportError:
    spineapi = None

# Make sure labels are sorted numerically (such that 10 > 2)
def sortfn(c):
    v = c.get('property:label')
    if v is None:
        v = c.get('property:order')
    if v is not None:
        num = re.match('[0-9]*', v).group()
        if len(num) > 0:
            v = re.sub('^[0-9]*', '{0:020d}'.format(int(num)), v)
    else:
        v = c.get('property:authors', c.get('property:id', '0')).lower()
    return v


class MetadataAnnotator(utopia.document.Annotator):
    """Metadata filter"""

    def on_ready_event(self, document):
        # Scrape title and DOI from document
        title = utopia.tools.utils.metadata(document, 'title')
        doi = utopia.tools.utils.metadata(document, 'identifiers[doi]')
        if title is not None or doi is not None:
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
            if title is not None:
                annotation['property:title'] = title
            if doi is not None:
                annotation['property:doi'] = doi
            document.addAnnotation(annotation, link['listName'])


    def after_ready_event(self, document):
        print 'Formatting metadata'

        # Find highest matching metadata accumulation list for references
        source = None
        for accListLink in document.getAccLists('metadata'):
            matches = document.annotationsIf({'concept': 'Citation'}, accListLink['scratch'])
            if len(matches) > 0:
                print 'Selected for [Citation] list %s with rank %s' % (accListLink['scratch'], repr(accListLink.get('rank', 0)))
                source = accListLink
                bibliography = list(matches)
                bibliography.sort(key=sortfn)
                rt=''
                for annotation in bibliography:
                    citation = utopia.tools.utils.citation_from_annotation(annotation)
                    rt += utopia.citation.render(citation, links=True)

                if len(bibliography) > 0:
                    # Create Metadata link annotation
                    link = document.newAccList('citation_list')
                    link['property:list_name'] = 'Bibliography'
                    document.addAnnotations(bibliography, link['scratch'])

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

