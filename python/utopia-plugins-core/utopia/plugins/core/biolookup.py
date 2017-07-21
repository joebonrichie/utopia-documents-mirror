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

#? name: Utopia Documents standard biological lookups
#? www: http://utopiadocs.com/
#? urls: https://utopia.cs.manchester.ac.uk/


import kend.client
import utopia.tools.converters.Annotation
import re
import spineapi
import utopia.document




class CollatedVisualiser(utopia.document.Visualiser):
    '''Visualise new (temporary) style collated definition annotations.'''

    def visualisable(self, a):
        return a.get('concept') == 'Collated' and 'property:html' in a

    def visualise(self, a):
        return a.getAllProperties('property:html')

class ChEMBLVisualiser(utopia.document.Visualiser):

    def visualisable(self, annotation):
        return annotation.get('concept') == 'Definition' and 'property:name' in annotation and annotation.get('property:sourceDatabase') == 'ChEMBL'

    def visualise(self, annotation):
        entry = ''
        moleculeFormula = annotation.get('property:moleculeFormula')
        webpageUrl = annotation.get('property:webpageUrl')
        imageUrl = annotation.get('property:imageUrl')
        smiles = annotation.get('property:moleculeCanonicalSmiles')
        inchi = annotation.get('property:moleculeStandardInchi')
        synonyms = list(annotation.getAllProperties('property:synonym'))
        description = annotation.get('property:description')
        if moleculeFormula is not None:
            moleculeFormula = moleculeFormula.replace(' ', '')
            moleculeFormula = re.sub(r'([A-Z][a-z]?)', r'<span class="element">\1</span>', moleculeFormula)
            moleculeFormula = re.sub(r'(\d+)', r'<sub>\1</sub>', moleculeFormula)
            entry += u'<p><span class="formula">{0}</span></p>'.format(moleculeFormula)
        if description is not None:
            description = description.split(' ')
            entry += u'<p>'
            entry += u'{0}'.format(' '.join(description[:32]))
            if len(description) > 32:
                entry += u' <span class="readmore">{0}</span>'.format(' '.join(description[32:]))
            entry += u'</p>'
        if len(synonyms) > 0:
            entry += u'<p>'
            entry += u'Synonyms: {0}'.format(', '.join(synonyms[:10]))
            if len(synonyms) > 10:
                entry += u', <span class="readmore">{0}</span>'.format(', '.join(synonyms[10:]))
            entry += u'</p>'
        if webpageUrl is not None:
            entry += u'<p><a href="{0}">See in ChEMBL...</a></p>'.format(webpageUrl)
        if imageUrl is not None:
            entry += u'<center><img src="{0}" /></center>'.format(imageUrl)
        return entry

class BioLookupChEMBLAnnotator(utopia.document.Annotator):
    """ChEMBL lookup"""

    visualiser = ChEMBLVisualiser()

    def on_explore_event(self, phrase, document):
        results = []
        client = kend.client.Client()

        annotations = client.lookup(term='"%s"'%phrase, database='ChEMBL', limit='30')

        xhtml = ''

        # Query to see if a page is returned
        # Extent information needs to be included FIXME
        if annotations is not None and len(annotations) >= 1:
            for group in annotations:
                for result in group.annotations:
                    annotation=utopia.tools.converters.Annotation.kend2spineapi(result)
                    entry = self.visualiser.visualisable(annotation) and self.visualiser.visualise(annotation) or ''
                    if len(entry) > 0:
                        name = annotation['property:name']
                        if 'property:moleculeType' in annotation:
                            name += ' ({0})'.format(annotation['property:moleculeType'])
                        xhtml += u'<div class="expandable" title="{0}">{1}</div>'.format(name, entry)

        if len(xhtml) > 0:
            xhtml = '<style>.formula {font-weight:bold; font-size: 1.2em;}</style>' + xhtml
            annotation = spineapi.Annotation()
            annotation['concept'] = 'Collated'
            annotation['property:html'] = xhtml
            annotation['property:name'] = 'ChEMBL'
            annotation['property:description'] = 'Related bioactive small molecules'
            annotation['property:sourceDatabase'] = 'chembl'
            annotation['property:sourceDescription'] = '<p><a href="https://www.ebi.ac.uk/chembl/">ChEMBL</a> is a database of bioactive drug-like small molecules.</p>'
            annotation['displayRelevance']='1000'
            annotation['displayRank']="%d" % -len(annotation['property:name'])
            results.append(annotation)

        return results



class DrugBankVisualiser(utopia.document.Visualiser):

    def visualisable(self, annotation):
        return annotation.get('concept') == 'Definition' and 'property:name' in annotation and annotation.get('property:sourceDatabase') == 'Drugbank'

    def visualise(self, annotation):
        entry = ''
        moleculeFormula = annotation.get('property:moleculeFormula')
        webpageUrl = annotation.get('property:webpageUrl')
        imageUrl = annotation.get('property:imageUrl')
        smiles = annotation.get('property:smiles')
        description = annotation.get('property:description')
        inchi = annotation.get('property:inchi')
        synonyms = list(annotation.getAllProperties('property:synonym'))
        if moleculeFormula is not None:
            moleculeFormula = moleculeFormula.replace(' ', '')
            moleculeFormula = re.sub(r'([A-Z][a-z]?)', r'<span class="element">\1</span>', moleculeFormula)
            moleculeFormula = re.sub(r'(\d+)', r'<sub>\1</sub>', moleculeFormula)
            entry += u'<p><span class="formula">{0}</span></p>'.format(moleculeFormula)
        if description is not None:
            description = description.split(' ')
            entry += u'<p>'
            entry += u'{0}'.format(' '.join(description[:32]))
            if len(description) > 32:
                entry += u' <span class="readmore">{0}</span>'.format(' '.join(description[32:]))
            entry += u'</p>'
        if len(synonyms) > 0:
            entry += u'<p>'
            entry += u'Synonyms: {0}'.format(', '.join(synonyms[:10]))
            if len(synonyms) > 10:
                entry += u', <span class="readmore">{0}</span>'.format(', '.join(synonyms[10:]))
            entry += u'</p>'
        if webpageUrl is not None:
            entry += u'<p><a href="{0}">See in DrugBank...</a></p>'.format(webpageUrl)
        if imageUrl is not None:
            entry += u'<center><img src="{0}" /></center>'.format(imageUrl)
        return entry

class BioLookupDrugBankAnnotator(utopia.document.Annotator):
    """DrugBank lookup"""

    visualiser = DrugBankVisualiser()

    def on_explore_event(self, phrase, document):
        results = []
        client = kend.client.Client()

        annotations = client.lookup(term='"%s"'%phrase, database='drugbank', limit='30')

        xhtml = ''

        # Query to see if a page is returned
        # Extent information needs to be included FIXME
        if annotations is not None and len(annotations) >= 1:
            for group in annotations:
                for result in group.annotations:
                    annotation=utopia.tools.converters.Annotation.kend2spineapi(result)
                    entry = self.visualiser.visualisable(annotation) and self.visualiser.visualise(annotation) or ''
                    if len(entry) > 0:
                        name = annotation['property:name']
                        if 'property:entryType' in annotation:
                            name += ' ({0})'.format(annotation['property:entryType'])
                        xhtml += u'<div class="expandable" title="{0}">{1}</div>'.format(name, entry)

        if len(xhtml) > 0:
            xhtml = '<style>.formula {font-weight:bold; font-size: 1.2em;} .formula sub{font-size:xx-small; position:relative; bottom:-0.3em;}</style>' + xhtml
            annotation = spineapi.Annotation()
            annotation['concept'] = 'Collated'
            annotation['property:html'] = xhtml
            annotation['property:name'] = 'DrugBank'
            annotation['property:description'] = 'Related drugs and their targets'
            annotation['property:sourceDatabase'] = 'drugbank'
            annotation['property:sourceDescription'] = '<p><a href="http://www.drugbank.ca">DrugBank</a> is a database that combines detailed drug data with comprehensive drug target information.</p>'
            annotation['displayRelevance']='1000'
            annotation['displayRank']="%d" % -len(annotation['property:name'])
            results.append(annotation)

        return results




class GlickVisualiser(utopia.document.Visualiser):

    def visualisable(self, annotation):
        return annotation.get('concept') == 'Definition' and 'property:description' in annotation and annotation.get('property:sourceDatabase') == 'glick'

    def visualise(self, annotation):
        if 'property:description' in annotation:
            return u'<p>{0}</p>'.format(annotation['property:description'])

class BioLookupGlickAnnotator(utopia.document.Annotator):
    """Glick lookup"""

    visualiser = GlickVisualiser()

    def on_explore_event(self, phrase, document):
        results = []
        client = kend.client.Client()

        annotations = client.lookup(term='"%s"'%phrase, database='Glick', limit='30')

        xhtml = ''

        # Query to see if a page is returned
        # Extent information needs to be included FIXME
        if annotations is not None and len(annotations) >= 1:
            for group in annotations:
                for result in group.annotations:
                    annotation=utopia.tools.converters.Annotation.kend2spineapi(result)
                    annotation['property:sourceDatabase'] = 'glick'
                    entry = self.visualiser.visualisable(annotation) and self.visualiser.visualise(annotation) or ''
                    if len(entry) > 0:
                        xhtml += u'<div class="expandable" title="{0}">{1}</div>'.format(annotation['property:name'], entry)

        if len(xhtml) > 0:
            xhtml += '<p><em>(Glick Glossary/Portland Press Ltd.)</em></p>'

            annotation = spineapi.Annotation()
            annotation['concept'] = 'Collated'
            annotation['property:html'] = xhtml
            annotation['property:name'] = 'Glick glossary entries'
            annotation['property:description'] = 'Biomedical definitions'
            annotation['property:sourceDatabase'] = 'glick'
            annotation['property:sourceDescription'] = '<p>David M. Glick\'s <a href="http://www.portlandpress.com/pp/books/online/glick/search.htm">Glossary of Biochemistry and Molecular Biology</a>.</p><p>Made available by <a href="http://www.portlandpress.com/">Portland Press Limited</a>.</p>'
            annotation['displayRelevance']='1000'
            annotation['displayRank']="%d" % -len(annotation['property:name'])
            results.append(annotation)

        return results




class PDBVisualiser(utopia.document.Visualiser):

    def visualisable(self, annotation):
        return annotation.get('concept') == 'Definition' and 'property:name' in annotation and annotation.get('property:sourceDatabase') == 'pdb'

    def visualise(self, annotation):
        entry = ''
        desc = annotation['property:name'][0:200]
        if len(desc) > 200:
            desc += '...'
        pdbcode = None
        if 'property:name' in annotation:
            entry += u'<p>{0}</p>'.format(annotation['property:name'])
        if 'property:identifier' in annotation:
            pdbcode = annotation['property:identifier'].upper()[-4:]
            entry += u'<p>%s</p>' % ('PDB entry %s' % pdbcode).strip()
        if 'property:molecularDescription' in annotation:
            entry += u'<p>{0}</p>'.format(annotation['property:molecularDescription'])
        if pdbcode is not None:
            entry += u'<div class="molecule-viewer" data-id="pdb:{0}"></div>'.format(pdbcode)
            entry += u'</div>'
        if 'property:webpageUrl' in annotation:
            entry += u'<p style="text-align:right"><a href="%s">View RCSB web page...</a></p>' % annotation['property:webpageUrl']
        return entry

class BioLookupPDBAnnotator(utopia.document.Annotator):
    """BioLookup PDB queries"""

    visualiser = PDBVisualiser()

    def on_explore_event(self, phrase, document):
        results = []

        client = kend.client.Client()

        annotations = client.lookup(term='"%s"'%phrase, database='PDB', limit='30')

        xhtml = ''

        if annotations is not None and len(annotations) >= 1:
            for group in annotations:
                for result in group.annotations:
                    annotation=utopia.tools.converters.Annotation.kend2spineapi(result)
                    annotation['property:sourceDatabase'] = 'pdb'
                    entry = self.visualiser.visualisable(annotation) and self.visualiser.visualise(annotation) or ''
                    if len(entry) > 0:
                        xhtml += entry

        if len(xhtml) > 0:
            annotation = spineapi.Annotation()
            annotation['concept'] = 'Collated'
            annotation['property:html'] = xhtml
            annotation['property:name'] = 'Protein Data Bank structures'
            annotation['property:description'] = 'Biological macromolecular structures'
            annotation['property:sourceDatabase'] = 'pdb'
            annotation['property:sourceDescription'] = '<p>The <a href="http://www.rcsb.org/">Protein Data Bank</a> of the Research Collaboratory for Structural Bioinformatics (<a href="http://home.rcsb.org/">RCSB</a>).</p>'
            annotation['displayRelevance']='800'
            annotation['displayRank']="%d" % -len(annotation['property:name'])
            results.append(annotation)

        return results




class UniProtVisualiser(utopia.document.Visualiser):

    def visualisable(self, annotation):
        return annotation.get('concept') == 'Definition' and 'property:webpageUrl' in annotation and annotation.get('property:sourceDatabase') == 'uniprot'

    def visualise(self, annotation):
        entry = ''
        if 'property:description' in annotation:
            entry += u'<p>{0}<p>'.format(annotation['property:description'])
        if 'property:webpageUrl' in annotation:
            entry += u'<p><a href="%s">View UniProt web page...</a></p>' % annotation['property:webpageUrl']
        return entry

class BioLookupUniProtAnnotator(utopia.document.Annotator):
    """BioLookup UniProt queries"""

    visualiser = UniProtVisualiser()

    def on_explore_event(self, phrase, document):
        results = []

        client = kend.client.Client()

        annotations = client.lookup(term='"%s"'%phrase, database='UniProt', limit='30')

        xhtml = ''

        if annotations is not None and len(annotations) >= 1:
            for group in annotations[:10]:
                for result in group.annotations[:10]:
                    annotation=utopia.tools.converters.Annotation.kend2spineapi(result)
                    annotation['property:sourceDatabase'] = 'uniprot'
                    entry = self.visualiser.visualisable(annotation) and self.visualiser.visualise(annotation) or ''
                    if len(entry) > 0:
                        xhtml += u'<div class="expandable" title="{0}">{1}</div>'.format('UniProt entry %s' % annotation.get('property:mnemonic', '???'), entry)

        if len(xhtml) > 0:
            annotation = spineapi.Annotation()
            annotation['concept'] = 'Collated'
            annotation['property:html'] = xhtml
            annotation['property:name'] = 'UniProt sequences'
            annotation['property:description'] = 'Protein sequence information'
            annotation['property:sourceDatabase'] = 'uniprot'
            annotation['property:sourceDescription'] = '<p>The Universal Protein Resource (<a href="http://www.uniprot.org/">UniProt</a>) is a comprehensive resource for protein sequence and annotation data.</p>'
            annotation['displayRelevance']='700'
            annotation['displayRank']="%d" % -len(annotation['property:name'])
            results.append(annotation)

        return results




class DBpediaVisualiser(utopia.document.Visualiser):

    def visualisable(self, annotation):
        return annotation.get('concept') == 'Definition' and 'property:summary' in annotation and annotation.get('property:identifier', '').startswith('http://dbpedia.org/resource/')

    def visualise(self, annotation):
        entry = u'<p>{0}<p>'.format(annotation['property:summary'])
        if 'property:imageUrl' in annotation:
            entry += u'<center><img src="%s" width="200" heighth="200" /></center>' % annotation['property:imageUrl']
        if 'property:webpageUrl' in annotation:
            webpageUrl = annotation['property:webpageUrl']
            if '?' in webpageUrl:
                webpageUrl += '&'
            else:
                webpageUrl += '?'
            webpageUrl += 'useformat=mobile'
            entry += u'<p><a href="%s" target="sidebar">View Wikipedia web page...</a></p>' % webpageUrl
        return entry

class BioLookupDBpediaAnnotator(utopia.document.Annotator):
    """BioLookup DBpedia queries"""

    visualiser = DBpediaVisualiser()

    def on_explore_event(self, phrase, document):
        results = []

        client = kend.client.Client()

        annotations = client.lookupDBpedia(term='"%s"'%phrase, category=['Molecular_biology', 'Biochemistry', 'UtopiaBiology'], limit='30')

        xhtml = ''

        if annotations is not None and len(annotations) >= 1:
            for group in annotations[:10]:
                for result in group.annotations[:10]:
                    annotation=utopia.tools.converters.Annotation.kend2spineapi(result)
                    entry = self.visualiser.visualisable(annotation) and self.visualiser.visualise(annotation) or ''
                    if len(entry) > 0:
                        xhtml += u'<div class="expandable" title="{0}">{1}</div>'.format(annotation['property:name'], entry)

        if len(xhtml) > 0:
            annotation = spineapi.Annotation()
            annotation['concept'] = 'Collated'
            annotation['property:html'] = xhtml
            annotation['property:name'] = 'Wikipedia entries'
            annotation['property:description'] = 'Community curated encyclopedia entries'
            annotation['property:sourceDatabase'] = 'wikipedia'
            annotation['property:sourceDescription'] = '<p>Structured <a href="http://www.wikipedia.org/">Wikipedia</a> information provided by the <a href="http://DBpedia.org/">DBpedia</a> project.</p>'
            annotation['displayRelevance']='900'
            annotation['displayRank']="%d" % -len(annotation['property:name'])
            results.append(annotation)

        return results





# class BioLookupGlickAnnotator(utopia.document.Annotator):
#     """Glick lookup"""
#
#     def on_explore_event(self, phrase, document):
#         results = []
#         client = kend.client.Client()
#
#         annotations = client.lookup(term='"%s"'%phrase.encode('utf-8'), database='Glick', limit='30')
#
#         # Query to see if a page is returned
#         # Extent information needs to be included FIXME
#         if annotations is not None and len(annotations) >= 1:
#             for group in annotations[:10]:
#               for result in group.annotations[:10]:
#                 annotation=utopia.tools.converters.Annotation.kend2spineapi(result)
#                 annotation['property:html'] = annotation['property:description']
#                 annotation['property:description'] = 'Glick glossary entry'
#                 annotation['property:sourceDatabase'] = 'glick'
#                 annotation['displayRelevance']='1000'
#                 annotation['displayRank']="%d" % -len(annotation['property:name'])
#                 results.append(annotation)
#
#         return results
#
# class BioLookupPDBAnnotator(utopia.document.Annotator):
#     """BioLookup PDB queries"""
#
#     def on_explore_event(self, phrase, document):
#         results = []
#
#         client = kend.client.Client()
#
#         annotations = client.lookup(term='"%s"'%phrase.encode('utf-8'), database='PDB', limit='30')
#
#         if annotations is not None and len(annotations) >= 1:
#             for group in annotations[:10]:
#               for result in group.annotations[:10]:
#                 annotation=utopia.tools.converters.Annotation.kend2spineapi(result)
#                 annotation['property:description']=('PDB entry %s' % annotation.get('property:identifier', '').upper()[-4:]).strip()
#                 annotation['property:sourceDatabase'] = 'pdb'
#                 annotation['displayRelevance']='800'
#                 annotation['displayRank']="%d" % -len(annotation['property:name'])
#                 results.append(annotation)
#
#         return results
#
# class BioLookupUniProtAnnotator(utopia.document.Annotator):
#     """BioLookup UniProt queries"""
#
#     def on_explore_event(self, phrase, document):
#         results = []
#
#         client = kend.client.Client()
#         annotations = client.lookup(term='"%s"'%phrase.encode('utf-8'), database='UniProt', limit='30')
#         if annotations is not None and len(annotations) >= 1:
#             for group in annotations[:10]:
#               for result in group.annotations[:10]:
#                 annotation=utopia.tools.converters.Annotation.kend2spineapi(result)
#                 annotation['property:description'] = ('UniProt entry %s' % annotation.get('property:mnemonic', '')).strip()
#                 annotation['property:sourceDatabase'] = 'uniprot'
#                 annotation['displayRelevance']='700'
#                 annotation['displayRank']="%d" % -len(annotation['property:name'])
#                 results.append(annotation)
#
#         return results
#
# class BioLookupDBpediaAnnotator(utopia.document.Annotator):
#     """BioLookup DBpedia queries"""
#
#     def on_explore_event(self, phrase, document):
#         results = []
#
#         client = kend.client.Client()
#
#         annotations = client.lookupDBpedia(term='"%s"'%phrase.encode('utf-8'), category=['Molecular_biology', 'Biochemistry', 'UtopiaBiology'], limit='30')
#
#         if annotations is not None and len(annotations) >= 1:
#             for group in annotations[:10]:
#               for result in group.annotations[:10]:
#                 annotation=utopia.tools.converters.Annotation.kend2spineapi(result)
#                 annotation['property:sourceDatabase'] = 'wikipedia'
#                 annotation['displayRelevance']='900'
#                 annotation['displayRank']="%d" % -len(annotation['property:name'])
#                 results.append(annotation)
#
#         return results
