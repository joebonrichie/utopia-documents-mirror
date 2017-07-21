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

#? name: GEO
#? www: http://www.ncbi.nlm.nih.gov/geo/
#? urls: http://www.ncbi.nlm.nih.gov/geo/ https://insilico.ulb.ac.be/


import spineapi
import utopia.document
import utopia.tools.utils
import urllib2

class GeneExpressionOmnibusAnnotator(utopia.document.Annotator, utopia.document.Visualiser):
    """Find GEO Data Sets"""

    def _fetchGEO(self, gse):

        url='https://www.ncbi.nlm.nih.gov/geo/query/acc.cgi?acc={0}&form=text&view=full'.format(gse)
        response = urllib2.urlopen(url, timeout=8)
        data= response.read().decode('utf8')

        entry={}
        keys=('title', 'summary', 'pubmed_id', 'submission_date', 'last_update_date',
              'geo_accession', 'overall_design', 'type', 'contributor', 'supplementary_file')

        for line in data.split('\n'):
            if len(line)==0 or line[0]!='!':
                continue

            key,value= line.replace('!Series_','').strip().split(' = ')

            if key in keys:
                try:
                    entry[key].append(value)
                except KeyError:
                    entry[key]=[value]

        contributors=[]
        for c in entry['contributor']:
            contrib=[x.strip() for x in c.split(',')]
            last_name=contrib.pop()
            initials=''.join([c[0].upper() if len(c)>0 else '' for c in contrib])
            contributors.append("%s %s" % (last_name, initials))

        # flatten lists of duplicate values for keys
        for k,v in entry.iteritems():
            entry[k]=' '.join(v)

        entry['contributors']=', '.join(contributors)
        entry['InSilicoDB_url'] = 'https://insilico.ulb.ac.be/app/browse?q=%s' % entry['geo_accession']
        entry['GEO_url'] = 'http://www.ncbi.nlm.nih.gov/geo/query/acc.cgi?acc=%s' % entry['geo_accession']

        return entry


    @utopia.document.buffer
    def on_ready_event(self, document):
        # Find distinguishing ID
        pmid = utopia.tools.utils.metadata(document, 'identifiers[pubmed]')

        # Compile distinct GEO IDs in the text
        matches = {}
        for match in document.search(r'GSE\d+', spineapi.IgnoreCase + spineapi.WholeWordsOnly + spineapi.RegExp):
            matches.setdefault(match.text(), [])
            matches[match.text()].append(match)

        # Create annotations for each GEO ID
        for gse, extents in matches.iteritems():
            entry = self._fetchGEO(gse)

            print entry

            dates = u'Submitted {0}'.format(entry['submission_date'])
            if 'last_update_date' in entry:
                dates += u'; last updated {0}'.format(entry['last_update_date'])
            dates += '.'

            dataCitation=u'''<p>{0}. <strong>{1}</strong>.</p><p>{2}<br>({3})</p><p>{4}</p>'''.format(
                entry['contributors'],
                entry['title'],
                entry.get('overall_design', ''),
                entry['type'],
                dates)

            xhtml = u'<div class="box">{0}{{0}}<p>GEO Accession: <a href="{1}">{2}</a></p></div>'.format(
                dataCitation, entry['GEO_url'], gse)

            # Removed broken InSilicoDB link
            #xhtml += u'<p><a href="{0}">Explore in InSilico DB...</a></p>'.format(entry['InSilicoDB_url'])

            srcdesc='''<p>The <a href="http://www.ncbi.nlm.nih.gov/geo">Gene
                       Expression Omnibus (GEO)</a> is a public repository
                       that archives and freely distributes microarray,
                       next-generation sequencing, and other forms of
                       high-throughput functional genomic data submitted
                       by the scientific community.</p>'''

            if entry.get('pubmed_id') == pmid:
                # add a global annotation
                annotation = spineapi.Annotation()
                annotation['concept'] = 'GEO'
                annotation['property:name'] = 'Gene Expression Omnibus'
                annotation['property:sourceDatabase'] = 'geo'
                annotation['property:description'] = '{0} (Data associated with this article)'.format(gse)
                annotation['property:sourceDescription'] = srcdesc
                annotation['property:xhtml'] = xhtml.format('') # Keep summary blank
                document.addAnnotation(annotation)

            # Generate summary
            summary = entry.get('summary', '')
            if len(summary) > 0:
                summary_words = summary.split(' ')
                summary = u'<p><em>Summary:</em> '
                summary += u'{0}'.format(' '.join(summary_words[:32]))
                if len(summary_words) > 32:
                    summary += u' <span class="readmore">{0}</span>'.format(' '.join(summary_words[32:]))
                summary += u'</p>'

            # local annotation
            annotation = spineapi.Annotation()
            annotation['concept'] = 'GEO'
            annotation['property:name'] = 'Gene Expression Omnibus'
            annotation['property:sourceDatabase'] = 'geo'
            annotation['property:description'] = gse
            annotation['property:sourceDescription'] = srcdesc
            annotation['property:xhtml'] = xhtml.format(summary)
            for extent in extents:
                annotation.addExtent(extent)
            document.addAnnotation(annotation)

    def visualisable(self, a):
        return a.get('concept') == 'GEO'

    def visualise(self, a):
        return a.get('property:xhtml')
