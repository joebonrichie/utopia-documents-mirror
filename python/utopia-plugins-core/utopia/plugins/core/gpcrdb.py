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

#? name: GPCRDB
#? www: http://www.gpcr.org/
#? urls: http://www.gpcr.org/


# encoding: UTF-8

import base64
import utopia.tools.eutils
import utopia.tools.utils
import re
import spineapi
import suds.client
import urllib
import utopia.document
import urllib2
from lxml import etree

class GPCRDBAnnotator(utopia.document.Annotator):
    '''Annotate with GPCRDB'''

    def getMentions(self, text, pubmedId):
        print 'initialising now ...'

        serviceUrl = 'http://www.gpcr.org/7tm/webservice'
        wsdlUrl = '%s?wsdl' % serviceUrl
        gc = suds.client.Client(wsdlUrl)
        gs = gc.service

        textBytes = base64.b64encode(text.encode('utf-8'))
        if pubmedId != None:
            pubmedId = pubmedId.encode('utf-8')
        else:
            pubmedId = ''

        #print text
        body = """
        <SOAP-ENV:Envelope
         xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/">
         <SOAP-ENV:Body>
          <ns:getMentions xmlns:ns="http://webservice.web.mcsis.cmbi.ru.nl/">
           <text>{0}</text>
           <pubmedId>{1}</pubmedId>
          </ns:getMentions>
         </SOAP-ENV:Body>
        </SOAP-ENV:Envelope>
        """.format(textBytes, pubmedId).encode('utf8')
        res = urllib2.urlopen(serviceUrl, body, timeout=8)
        res = res.read()
        return gs.getMentions(__inject={'reply': res})

    def _populate(self, document):
        padding = r'[\s\p{Pd}]'
        regex = r'(gpcr|g%sprotein%scoupled%sreceptor)' % (padding, padding, padding)
        # If GPCR is found in the paper, auto-annotate
        mentions = document.search(regex, spineapi.IgnoreCase + spineapi.RegExp)
        if len(mentions) > 0:
            self.annotate(document)

    @utopia.document.buffer
    def on_activate_event(self, document):
        if len(document.annotations('GPCRDB cache')) == 0:
            print 'annotating stuff . . .'

            pubmedId = utopia.tools.utils.metadata(document, 'identifiers[pubmed]')
            if pubmedId is not None:
                print 'found pubmed id: ' + pubmedId
            else:
                print 'did not find pubmed id'

            ns = {'r': 'GPCR'}

            textMentions = self.getMentions(document.text(), pubmedId)

            objectlist = []
            mention_cache = {}
            for mention in textMentions:
                if mention.mentionType != 'SPECIES':
                    mention_cache.setdefault(mention.html, [])
                    mention_cache[mention.html].append(mention)

            for html, mentions in mention_cache.iteritems():
                annotation = self.createAnnotation(document, html, mentions)
                annotation['displayRelevance']='2000'
                annotation['displayRank']= '2000'
                document.addAnnotation(annotation)

            document.addAnnotation(spineapi.Annotation(), 'GPCRDB cache')

    def createAnnotation(self, document, html, mentions):
        annotation = spineapi.Annotation()
        annotation['concept'] = 'GPCRInformation'
        annotation['property:name'] = '%s: "%s"' % (mentions[0].mentionType.title(), mentions[0].formalRepresentation)
        annotation['property:description'] = 'GPCRDB %s record' % mentions[0].mentionType.title()
        annotation['property:sourceDatabase'] = 'gpcrdb'
        annotation['property:sourceDescription'] = '<p>The <a href="http://www.gpcr.org/7tm/">GPCRDB</a> is a molecular-class information system that collects, combines, validates and stores large amounts of heterogenous data on G protein-coupled receptors.</p>'
        annotation['property:html'] = html

        for mention in mentions:
            #print mention
            start = int(mention.textStart)
            end = int(mention.textEnd)
            match = document.substr(start, end-start)
            annotation.addExtent(match);

        return annotation

class GPCRDBVisualiser(utopia.document.Visualiser):
    """Viualiser for GPCRDB entries"""

    def visualisable(self, annotation):
        return annotation.get('concept') == 'GPCRInformation' and 'property:html' in annotation

    def visualise(self, annotation):
        html = annotation['property:html']
        if html.endswith('</tr>'):
            html += '</table>'
        return html

__all__ = ['GPCRDBAnnotator', 'GPCRDBVisualiser']
