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

#? name: Utopia global store
#? www: http://utopiadocs.com/
#? urls: https://utopia.cs.manchester.ac.uk/


import common.utils
import kend.client
import kend.converter
import kend.converter.Annotation
import kend.model
import urllib
import utopia.document


# This has to be here, as for some reason it's not remembered when it's done through
# boost::python during interpreter setp.
utopia.proxyUrllib2()


class GlobalStoreAnnotator(utopia.document.Annotator):
    """Global annotation store"""

    _context_ = 'http://utopia.cs.manchester.ac.uk/contexts/public'
    _delete_ = 'delete %s' % _context_
    _update_ = 'update %s' % _context_
    _persist_ = 'persist %s' % _context_


    def _resolve(self, document):
        # Start with evidence from fingerprinting
        evidence = [kend.model.Evidence(type='fingerprint', data=f, srctype='document') for f in document.fingerprints()]

        # Add scraped / resolved metadata
        fields = {}
        for key in ('doi', 'title', 'arxivid', 'pmid', 'pmcid', 'issn', 'pii'):
            value = common.utils.metadata(document, key)
            if value is not None:
                fields[key] = value
        for key, value in fields.iteritems():
            evidence.append(kend.model.Evidence(type=key, data=value, srctype='algorithm', src='utopia/2.2.1'))

        # Add page count
        evidence.append(kend.model.Evidence(type='pagecount', data=document.numberOfPages(), srctype='document'))

        documentref = kend.model.DocumentReference(evidence=evidence)
        documentref = kend.client.Client().documents(documentref)

        try:
            return documentref.id, fields.get('doi')
        except AttributeError:
            pass

        return None, None





    @utopia.document.buffer
    def populate(self, document):
        document_id, doi = self._resolve(document)
        if document_id is not None:
            kwargs = { 'document': document_id, 'context': self._context_ }
            if doi is not None:
                kwargs['doi'] = doi
            annotations = kend.client.Client().annotations(**kwargs)

            link = None
            if annotations is not None:
                for group in annotations:
                    for ann in group.annotations:
                        a = kend.converter.Annotation.kend2spineapi(ann, document)
                        if a.get('author') == 'http://utopia.cs.manchester.ac.uk/users/11679':
                            if a.get('concept') in ("DocumentMetadata", "AuthorAffiliation", "DocumentReference"):
                                if link is None:
                                    link = document.newAccList('metadata', 100)
                                    link['property:sourceDatabase'] = 'biochemj'
                                    link['property:sourceTitle'] = 'The Semantic Biochemical Journal'
                                    link['property:sourceDescription'] = '<p>Made available by <a href="http://www.portlandpress.com/">Portland Press Limited</a> as part of the <a href="http://www.biochemj.org/bj/semantic_faq.htm">Semantic Biochemical Journal</a>.'

                                # Modify Bibliography Entries
                                if a.get('concept') == 'DocumentReference':
                                    for keyTo, keyFrom in {
                                                'property:title': 'property:articleTitle',
                                                'property:authors': 'property:articleAuthors',
                                                'property:year': 'property:articleYear',
                                                'property:volume': 'property:articleVolume',
                                                'property:source': 'property:journalTitle',
                                            }.iteritems():
                                        if keyFrom in a:
                                            a[keyTo] = a[keyFrom]
                                        a['property:sourceDatabase'] = 'biochemj'
                                        a['property:sourceDescription'] = 'Thingy'

                                document.addAnnotation(a, link['scratch'])
                            else:
                                document.addAnnotation(a)
                        else:
                            document.addAnnotation(a)



    def persist(self, document):
        client = kend.client.Client()

        document_id, doi = self._resolve(document)
        if document_id is not None:
            for annotation in document.annotations('PersistQueue'):
                if 'session:volatile' not in annotation:
                    try:
                        ka = kend.converter.Annotation.spineapi2kend(annotation, document_id)
                        ka.context = self._context_

                        updated = client.persistAnnotation(ka, context = self._context_)

                        if isinstance(updated, kend.model.Annotation):
                            for key in ('id', 'created', 'author', 'revision', 'edit', 'media_edit'):
                                annotation[key] = getattr(updated, key)
                            annotation.removePropertyAll('session:media')
                            for media in updated.media:
                                mediaDict = {}
                                for k in ['name', 'src', 'type', 'revision', 'size', 'edit']:
                                    if hasattr(media, k):
                                        mediaDict[k] = getattr(media, k)
                                annotation.insertProperty('session:media', urllib.urlencode(mediaDict))
                            document.removeAnnotation(annotation, 'PersistQueue')
                            document.addAnnotation(annotation)
                    except:
                        raise
                        pass

            for annotation in document.annotations(document.deletedItemsScratchId()):
                try:
                    if 'session:volatile' not in annotation:
                        ka = kend.converter.Annotation.spineapi2kend(annotation, document_id)
                        client.deleteAnnotation(ka)
                    document.removeAnnotation(annotation, document.deletedItemsScratchId())
                except:
                    raise
                    pass



