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

#? name: CrossRef
#? www: http://www.crossref.org/
#? urls: http://data.crossref.org/


import utopialib.utils
import json
import re
import socket
import spineapi
import utopia.document
import urllib2
import logging
import cgi

logger = logging.getLogger(__name__)
#logger.setLevel(logging.DEBUG)

class _CiteProc(utopia.document.Annotator, utopia.document.Visualiser):
    """Generate CiteProc information"""

    crossRefUrl = 'http://data.crossref.org/'
    stylesUrl = 'http://data.crossref.org/styles'
    localesUrl = 'http://data.crossref.org/locales'

    priorityStyles = ["apa","bibtex","chicago-fullnote-bibliography","harvard3","ieee","mla","vancouver"]
    defaultStyle = 'apa'
    defaultLocale = 'en-GB'
    loadingMsg = 'Fetching...'

    '''def generateCitation(self, style, locale):
        text = ''
        if doi is not None:
            finalUrl = self.crossRefUrl.format(doi)
            request = urllib2.Request(finalUrl, None, { 'Accept' : 'text/bibliography;style={0};locale={1}'.format(style, locale) })
            text = urllib2.urlopen(request, timeout=8).read()
            logger.debug('Data returned from citeproc:{0}'.format(text))
        else:
            logger.warn('no doi found in citeproc')
        return text'''

    def on_ready_event(self, document):
        logger.debug('calling citeproc populate')
        doi = utopialib.utils.metadata(document, 'identifiers[doi]')
        crossref_unixref = utopialib.utils.metadata(document, 'raw_crossref_unixref')
        # Only bother for those documents that returned a crossref document
        if doi is not None and crossref_unixref is not None:
            #load styles and locales here
            stylesJson = urllib2.urlopen(self.stylesUrl, timeout=8).read()
            logger.debug(stylesJson)

            localesJson = urllib2.urlopen(self.localesUrl, timeout=8).read()
            logger.debug(localesJson)

            a = spineapi.Annotation()
            a['concept'] = 'CiteProc'
            a['property:doi'] = doi
            a['property:text'] = self.loadingMsg
            a['property:styles'] = stylesJson
            a['property:locales'] = localesJson
            a['property:name'] = 'CrossRef'
            a['property:description'] = 'Formatted citation for this article'
            a['property:sourceDatabase'] = 'crossref'
            a['property:sourceDescription'] = '<p><a href="http://www.crossref.org/">CrossRef</a> is the official DOI link registration agency for scholarly and professional publications.</p>'
            a['session:weight'] = '10'
            a['session:default'] = '1'
            document.addAnnotation(a)

    def visualisable(self, a):
        return a.get('concept') == 'CiteProc' and 'property:doi' in a

    def visualise(self, a):
        logger.debug('in citeproc visualise method')
        list = []
        doi = a.get('property:doi')
        #build javascript here
        script = '''
        <script>
            var utopia_citeproc = {{
                generateCitation:
                    function (element, doi) {{
                        var citation = $(element).closest('#citeproc');
                        var loading = citation.find('.box .loading').first();
                        var output = citation.find('.box .output').first();
                        var style = $('.citation_style', citation).val();
                        var locale = $('.citation_locale', citation).val();
                        output.text('{1}');
                        $.ajax({{
                            url: '{0}' + doi,
                            beforeSend: function (jqXHR) {{
                                // HACK to fix crossref's broken(?) server
                                if (style == 'bibtex') {{
                                    jqXHR.setRequestHeader('Accept', 'text/bibliography;style='+style+';locale='+locale);
                                }} else {{
                                    jqXHR.setRequestHeader('Accept', 'text/bibliography;style='+style+';format=html;locale='+locale);
                                }}
                            }},
                            complete: function (jqXHR) {{
                                html = jqXHR.responseText;
                                if (style == 'bibtex') {{
                                    html = '<pre>' + html + '</pre>';
                                }}
                                output.html(html);
                            }}
                        }});
                    }}
            }};
        </script>
        '''.format(self.crossRefUrl, self.loadingMsg)

        logger.debug('script is: {0}'.format(script))

        # Build html here
        text = a.get('property:text')
        html = '''
            <style>
                #citeproc select.citation_style {
                    float: none;
                    display: inline;
                    width: 70%;
                }
                #citeproc select.citation_locale {
                    float: left;
                    width: 30%;
                    display: none;
                }
                #citeproc {
                    text-align: right;
                }
                #citeproc .box {
                    text-align: left;
                }
            </style>
        '''
        html += '<div id="citeproc" onload="utopia_citeproc.generateCitation(this, \'{0}\')">'.format(doi)
        html += '<div class="box"><div class="output">{0}</div></div>'.format(cgi.escape(text, quote=True).encode('ascii', 'xmlcharrefreplace'))
        html += '<select class="citation_style" onchange="utopia_citeproc.generateCitation(this, \'{0}\')">'.format(doi)
        for style in self.priorityStyles:
            logger.debug('style: {0}'.format(style))
            if style == self.defaultStyle:
                selected = 'selected="selected"'
            else:
                selected = ''
            html += '<option value="{0}" {1}>{0}</option>'.format(style, selected)
        html += '</select><select class="citation_locale" onchange="utopia_citeproc.generateCitation(this, \'{0}\')">'.format(doi)
        for locale in json.loads(a.get('property:locales')):
            logger.debug('locale: {0}'.format(locale))
            if locale == self.defaultLocale:
                selected = 'selected="selected"'
            else:
                selected = ''
            html += '<option value="{0}" {1}>{0}</option>'.format(locale, selected)
        html += '</select>'
        html += '</div>'
        logger.debug('html is: {0}'.format(html))

        init = '''
        <script>
            $('#citeproc').each(function () {{
                utopia_citeproc.generateCitation(this, '{0}');
            }});
        </script>
        '''.format(doi)

        logger.debug('init is: {0}'.format(init))

        list.append(script)
        list.append(html)
        list.append(init)
        return list

