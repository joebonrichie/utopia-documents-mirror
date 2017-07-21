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

import utopia.tools.utils
import json
import spineapi
import utopia.document
import xml.sax.saxutils

class CitationFormatter(utopia.document.Annotator, utopia.document.Visualiser):

    # Metadata properties to be collected
    properties = ('title', 'authors[]', 'url', 'volume', 'issue', 'year', 'abstract',
                  'publication-title', 'publisher', 'pages', 'label', 'identifiers')

    def after_load_event(self, document):
        '''
        Provide formatted citations for the current document. This should be done quite
        late in the process, in case previous handlers have enriched the metadata.
        '''

        # Start by getting all the best-trusted metadata for this document
        metadata = {}
        for key in self.properties:
            value = utopia.tools.utils.metadata(document, key)
            if value is not None:
                if key[-2:] == '[]':
                    key = key[:-2]
                metadata[key] = value

        # Only if there's some metadata to display FIXME
        if len(metadata) > 0:
            # Now create a citation formatter annotation for the sidebar
            annotation = spineapi.Annotation()
            annotation['concept'] = 'CitationFormatter'
            annotation['property:json'] = json.dumps(metadata)
            annotation['property:name'] = 'Formatted Citation'
            annotation['property:description'] = 'How to cite this document'
            annotation['property:sourceIcon'] = utopia.get_plugin_data_as_url('images/csl.png', 'image/png')
            annotation['property:sourceDescription'] = '''
                Uses <a href="https://bitbucket.org/fbennett/citeproc-js/wiki/Home">citeproc-js</a>
                to format the citation.
            '''
            annotation['session:weight'] = '10'
            annotation['session:default'] = '1'
            annotation['session:volatile'] = '1'
            document.addAnnotation(annotation)

    def visualisable(self, annotation):
        return annotation.get('concept') == 'CitationFormatter' and 'property:json' in annotation

    def visualise(self, annotation):
        metadata = annotation['property:json']

        id = self.cssId();

        html_script = '''
            <script>
                function generate_formatted_citation_{0}() {{
                    var citation = $('#citation_{0}');
                    var metadata = citation.data('metadata');
                    var output = citation.find('.box .output').first();
                    var style = $('.citation_style', citation).val();
                    var formatted = utopia.citation.format(metadata, style);
                    output.html(formatted);
                }}
                function populate_citation_styles_{0}() {{
                    var citation = $('#citation_{0}');
                    var select = $('.citation_style', citation);
                    var styles = utopia.citation.styles();
                    var defaultStyle = utopia.citation.defaultStyle();
                    for (var code in styles) {{
                        var defaultness = '';
                        if (code == defaultStyle) {{
                            defaultness = ' selected="selected"';
                        }}
                        select.append('<option value="' + code + '"' + defaultness + '>' + styles[code] + '</option>');
                    }}
                    generate_formatted_citation_{0}();
                }}
            </script>
        '''.format(id)

        html_style = '''
            <style>
                #citation_{0} select.citation_style {{
                    float: none;
                    display: inline;
                }}
                #citation_{0} {{
                    text-align: left;
                }}
                #citation_{0} .options {{
                    text-align: right;
                    margin-top: 6px;
                }}
                #citation_{0} .options select {{
                    margin: 0;
                }}
            </style>
        '''.format(id)

        html = '''
            <div id="citation_{0}">
                <div class="box">
                    <div class="output">No citation found</div>
                </div>
                <div class="options">
                    Style: <select class="citation_style" onchange="generate_formatted_citation_{0}(this)" />
                </div>
            </div>
        '''.format(id)

        html_init = '''
            <script>
                (function () {{
                    var citation = $('#citation_{0}');
                    citation.data('metadata', ({1}));
                    populate_citation_styles_{0}();
                }})();
            </script>
        '''.format(id, metadata)

        return (html_script, html_style, html, html_init)
