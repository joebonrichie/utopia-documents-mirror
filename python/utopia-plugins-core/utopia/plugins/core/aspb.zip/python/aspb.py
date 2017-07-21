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

#? name: American Society of Plant Biologists
#? www: http://my.aspb.org/
#? urls: http://rest.kegg.jp/


import utopia.tools.utils
import re
import socket
import spineapi
import utopia.document

import urllib2

class ASPBDatabaseLookups(utopia.document.Annotator, utopia.document.Visualiser):
    '''ASPB Lookups'''

    agiRegex = r'AT\d{1,2}G\d{3,}'
    findTpl = 'http://rest.kegg.jp/find/genes/{0}'
    getTpl = 'http://rest.kegg.jp/get/{0}'
    findResultRegex = re.compile(r'^(?P<qualid>(?P<org>[^:]+):(?P<id>[^\s]+))\t(?P<gene>[^;]+);\s+(?P<description>.*)$')

    @utopia.document.buffer
    def on_ready_event(self, document):
        doi = utopia.tools.utils.metadata(document, 'identifiers[doi]')

        # Find and aggregate AGI instances in the document
        matches_by_agi = {}
        for match in document.search(self.agiRegex, spineapi.RegExp + spineapi.WholeWordsOnly):
            agi = match.text()
            matches_by_agi.setdefault(agi, [])
            matches_by_agi[agi].append(match)

        # For each AGI add a new bit of HTML
        if len(matches_by_agi) > 0:
            for agi, matches in matches_by_agi.iteritems():
                html = '''
                <p style="overflow: auto; width: 100%">
                  <strong>{0}</strong>
                  <span style="float: right">{1}</span>
                </p>
                '''

                annotation = spineapi.Annotation()
                annotation['concept'] = 'AGI'
                annotation['property:agi'] = agi
                annotation['property:name'] = 'Plant gene databases'
                annotation['property:description'] = 'American Society of Plant Biologists'
                annotation['property:sourceIcon'] = utopia.get_plugin_data_as_url('images/aspb_logo.png', 'image/png')
                if doi is not None:
                    if doi.startswith('10.1104/'):
                        annotation['property:sourceIcon'] = utopia.get_plugin_data_as_url('images/pp_logo.png', 'image/png')
                        annotation['property:description'] = 'From Plant Physiology'
                    elif doi.startswith('10.1105/'):
                        annotation['property:sourceIcon'] = utopia.get_plugin_data_as_url('images/tpc_logo.png', 'image/png')
                        annotation['property:description'] = 'From Plant Cell'
                annotation['property:sourceDescription'] = '''
                  <p>
                    The <a href="http://www.aspb.org/">American Society of Plant Biologists</a>
                    have deemed these linked databases important sources of information.
                  </p>
                '''
                annotation.addExtents(matches)
                document.addAnnotation(annotation)

    def visualisable(self, annotation):
        return annotation.get('concept') == 'AGI' and annotation.get('property:agi') is not None

    def visualise(self, annotation):
        # Try to find information about this AGI from KEGG gene database
        agi = annotation['property:agi']
        qualid, orgid, id, gene, description, organism, details = '', '', '', '', agi, '', ''
        try:
            # Search for the AGI in KEGG
            url = self.findTpl.format(annotation['property:agi'].encode('utf8'))
            results = urllib2.urlopen(url, timeout=8).read()
            match = self.findResultRegex.match(results)
            if match is not None:
                qualid, orgid, id, gene, description = match.groups()

                # Get the DB entry for this record
                url = self.getTpl.format(qualid.encode('utf8'))
                results = urllib2.urlopen(url, timeout=8).read()
                field = ''
                for line in results.splitlines():
                    field = line[:12].strip() or field
                    data = line[12:].strip()
                    if field == 'ORGANISM':
                        organism = re.match(r'[^\s]+\s+(?P<organism>.*)', data).group('organism')
        except (urllib2.URLError, socket.timeout):
            pass

        if len(organism) > 0:
            organism = ' (<em>{0}</em>)'.format(organism)

        if len(description) > 0:
            details = '''
            <strong>{0}</strong>{1}: {2}
            '''.format(gene, organism, description)

        icons = [
            '''
            <a title="The Nottingham Arabidopsis Stock Centre (NASC)"
               href="http://arabidopsis.info/BasicResult?input={0}&constraint=all&submit=Go">
                <img src="{1}" />
            </a>
            '''.format(agi, utopia.get_plugin_data_as_url('images/nasc_db_logo.png', 'image/png')),
            '''
            <a title="Gramene: A Resource for Comparative Grass Genomics"
               href="http://www.gramene.org/db/searches/quick_search?search_for={0}&search_box_id=search_for_on_results&x=0&y=0">
                <img src="{1}" />
            </a>
            '''.format(agi, utopia.get_plugin_data_as_url('images/gramene_db_logo.png', 'image/png')),
            '''
            <a title="The Arabidopsis Information Resource (TAIR)"
               href="http://www.arabidopsis.org/servlets/TairObject?type=locus&name={0}">
                <img src="{1}" />
            </a>
            '''.format(agi, utopia.get_plugin_data_as_url('images/tair_db_logo.png', 'image/png')),
        ]

        if len(qualid) > 0:
            icons.append('''
                <a title="Kyoto Encyclopedia of Genes and Genomes (KEGG)"
                   href="http://www.kegg.jp/dbget-bin/www_bget?{0}">
                    <img src="{1}" />
                </a>
                '''.format(qualid, utopia.get_plugin_data_as_url('images/kegg_db_logo.png', 'image/png')))

        icons = '&nbsp;'.join(icons)

        return'''
        <style>
            .aspb .aspb_icons { text-align: right; }
            .aspb .aspb_icons img { vertical-align: middle; border: 1px solid #004; height: 30px; width: 30px; }
        </style>
        ''', '''
        <div class="box aspb">
        <h1>{0}</h1>
        <p class="nohyphenate">{1}</p>
        <div class="aspb_icons">Explore in:&nbsp;&nbsp;&nbsp;{2}</div>
        </div>
        '''.format(agi, details, icons)
