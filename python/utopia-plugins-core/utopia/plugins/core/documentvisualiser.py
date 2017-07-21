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

import utopia.document

class DocumentVisualiser(utopia.document.Visualiser):
    """Visualiser for DocumentReference and DocumentMetadata"""

    def visualisable(self, annotation):
        return annotation.get('concept') in ("DocumentReference", "DocumentMetadata", "BibliographyMetadata")

    def visualise(self, annotation):
        rt = ''

        if annotation.get('concept') == "BibliographyMetadata":
            rt = annotation['displayBibliography']

        elif annotation.get('concept') == "DocumentMetadata":
            if 'property:articleTitle' in annotation:
                rt += '<h3>%s</h3>' % annotation['property:articleTitle']
            if 'property:articleAuthors' in annotation:
                rt += '%s' % annotation['property:articleAuthors']
            if 'property:journalTitle' in annotation:
                rt += '<br/><br/>%s' % annotation['property:journalTitle']
            if 'property:articleVolume' in annotation:
                rt += ' <strong>%s</strong>' % annotation['property:articleVolume']
            if 'property:articleIssue' in annotation:
                rt += ' (%s)' % annotation['property:articleIssue']
            if 'property:articlePages' in annotation:
                rt += '<br/>Pages: %s' % annotation['property:articlePages']
            if 'property:articlePublicationDate' in annotation:
                rt += '<br/>Published: %s' % annotation['property:articlePublicationDate']

            # if 'property:journalPublisher' in annotation:
            #     rt += '<br/>%s' % annotation['property:journalPublisher']

            # if 'property:journalISSN' in annotation:
            #     rt += ' (%s)' % annotation['property:journalISSN']


            if 'property:articleKeywords' in annotation:
                rt += '<br/><br/><strong>Keywords</strong><br/>%s' % annotation['property:articleKeywords']
            if 'property:articleAbbreviations' in annotation:
                rt += '<br/><br/><strong>Abbreviations used in the Document</strong><br/>%s' % annotation['property:articleAbbreviations']
            if 'property:xarticleAbstract' in annotation:
                rt += '<br/><br/><strong>Abstract</strong><br/>%s</hr>' % annotation['property:articleAbstract']

        else:
            if 'property:articleAuthors' in annotation:
                rt += '%s' % annotation['property:articleAuthors']
            if 'property:articleYear' in annotation:
                rt += ' (%s)' % annotation['property:articleYear']
            if 'property:articleTitle' in annotation:
                rt += ' <a style="text-decoration:none" href="http://scholar.google.co.uk/scholar?q=%s">%s</a>' % (annotation['property:articleTitle'],annotation['property:articleTitle'])

            if 'property:journalTitle' in annotation:
                rt += ' %s' % annotation['property:journalTitle']
            if 'property:articleVolume' in annotation:
                rt += ' <strong>%s</strong>' % annotation['property:articleVolume']
            if 'property:articleIssue' in annotation:
                rt += ' (%s)' % annotation['property:articleIssue']
            if 'property:articlePages' in annotation:
                rt += '<br/><strong>Pages: </strong>%s' % annotation['property:articlePages']

        if rt == '':
            rt = None

        return rt
