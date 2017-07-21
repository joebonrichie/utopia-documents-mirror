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

#? name: GeneCards
#? www: http://www.genecards.org/
#? urls: http://www.genecards.org/


import kend.client
import utopia.tools.converters.Annotation
import re
import spineapi
import utopia.document
import utopia.tools.utils

class GenecardsVisualiser(utopia.document.Visualiser):

    def visualisable(self, annotation):
        return annotation.get('concept') == 'Definition' and annotation.get('property:sourceDatabase') == 'genecards' and 'property:geneSymbol' in annotation

    def visualise(self, annotation):
        gc_url='http://www.genecards.org/cgi-bin/carddisp.pl?utm_source=utopia&utm_campaign=utopia_gc&gene={gene}{fragment}'
        links = {
            'Aliases': ('Aliases', '#aliases_descriptions'),
            'Summaries': ('Summaries', '#summaries'),
            'Pathways': ('Pathways', '#pathways_interactions'),
            'ProteinSize_ProteinName': ('Proteins', '#proteins'),
            'GenomicViews': ('Genomic Views', '#genomic_location'),
            'Disorders': ('Disorders', '#diseases'),
        }
        misc_links = (
            ('Publications', '#publications'),
            ('Function', '#function'),
            ('Expression', '#expression'),
            ('Drugs', '#drugs_compounds'),
            ('Orthologs', '#orthologs'),
        )

        entry = u'<div class="box">'

        gene=annotation['property:geneSymbol']

        for k in ['Aliases', 'Summaries', 'Pathways', 'ProteinSize_ProteinName', 'GenomicViews', 'Disorders']:
            line=annotation.get('property:%s' % k, '')

            if len(line) > 0:
                if k=='ProteinSize_ProteinName':
                    line=line.replace('|', 'aa; ')

                elif k=='GenomicViews':
                    line='Chr ' + line.replace('|', '; ')

                line= line.replace('|', '; ')
                if line[-1] not in ('.', ';'):
                    if k=='Summaries' or k=='Disorders':
                        line+='&nbsp;...'
                    else:
                        line+='.'

                entry+=u'<p><a href="{0}">{1}</a>:&nbsp;{2}</p>'.format(gc_url.format(gene=gene, fragment=links[k][1]), links[k][0], line)
#                entry+='<p style="text-align: right"><a href="%s">(more)</a></p>' %

        misc_links = ('<a href="{0}">{1}</a>'.format(gc_url.format(gene=gene, fragment=fragment), title) for title, fragment in misc_links)
        entry += u'<p>' + u' | '.join(misc_links) + '</p>'
        entry += u"""<br><p style="text-align: right">
<a href="{0}">See complete {1} GeneCard...</a>
</p>""".format(gc_url.format(gene=gene, fragment=''), gene)
        entry+='</div>'

        return entry

class BioLookupGenecardsAnnotator(utopia.document.Annotator):
    """GeneCards queries"""

    visualiser = GenecardsVisualiser()

    @utopia.document.buffer
    def on_ready_event(self, document):
        # Find distinguishing ID
        pmid = utopia.tools.utils.metadata(document, 'identifiers[pubmed]')
        if pmid:
            print "Found pmid:", pmid
            for annotation in self.on_explore_event(phrase=pmid, document=document):
                annotation['property:description'] = 'Human genomic information related to this article'
                document.addAnnotation(annotation)

    def on_explore_event(self, phrase, document):
        results = []

        client = kend.client.Client()

        annotations = client.lookup(term='"%s"' % phrase.strip(), database='genecards', limit='30')

        xhtml = u''

        if annotations is not None and len(annotations) >= 1:
            for group in annotations[:10]:
                for result in group.annotations[:10]:
                    annotation=utopia.tools.converters.Annotation.kend2spineapi(result)
                    annotation['property:sourceDatabase'] = 'genecards'
                    entry = self.visualiser.visualisable(annotation) and self.visualiser.visualise(annotation) or ''
                    if len(entry) > 0:
                        symbol=annotation.get('property:geneSymbol', u'')
                        name=annotation.get('property:geneName', u'')
                        if name:
                            name=u'(%s)' % name

                        #print symbol
                        #print type(symbol)

                        #print name
                        #print type(name)

                        xhtml += u'<div class="expandable" title="Gene {0} {1}">{2}</div>'.format(symbol, name, entry)

        if len(xhtml) > 0:
            annotation = spineapi.Annotation()
            annotation['concept'] = 'Collated'
            annotation['property:html'] = xhtml
            annotation['property:name'] = 'GeneCards'
            annotation['property:description'] = 'Human genomic information'
            annotation['property:sourceIcon'] = utopia.get_plugin_data_as_url('images/genecards.png', 'image/png')
            annotation['property:sourceDescription'] = '<p><a href="http://www.genecards.org">GeneCards</a> is a database of concise genomic related information on all known and predicted human genes.</p>'
            annotation['displayRelevance']='700'
            annotation['displayRank']="%d" % -len(annotation['property:name'])
            results.append(annotation)

        return results
