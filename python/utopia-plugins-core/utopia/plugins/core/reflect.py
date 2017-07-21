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

#? name: REFLECT
#? www: http://reflect.ws/
#? urls: http://reflect.ws/


# import the Utopia Documents plugin superclass
import utopia.document
# import the Spine API
from spineapi import Annotation, IgnoreCase, WholeWordsOnly, RegExp
# import utilities
import urllib2
import urllib, re
from lxml import etree

class ReflectAnnotator(utopia.document.Annotator):
    """Reflect this document"""

    def __init__(self):
        self.parser = etree.XMLParser(recover = True, resolve_entities = True)

    @utopia.document.buffer
    def on_activate_event(self, document):
        ns = {'r': 'Reflect'}

        maxTextFragmentSize = 1000000
        textFragments = []
        seenItemNames = set()
        ignoredEntityTypes = [-11]

        # Retrieve the full text of the document, split into fragments
        for page in document.pages():
            pageText = re.sub(r'\s+', r' ', page.pageText())
            if len(textFragments) == 0 or len(textFragments[-1][0]) + len(pageText) > maxTextFragmentSize:
                textFragments.append([pageText, page])
            else:
                textFragments[-1][0] = textFragments[-1][0] + ' ' + pageText

        for text, page in textFragments:
            # Package it as URL encoded form encoding
            payload = 'document=%s' % urllib.quote(text.encode('utf8'))
            # Send it off to the reflect server
            response = urllib2.urlopen("http://reflect.ws/REST/GetEntities", payload, timeout=8)
            # Parse response
            root = etree.fromstring(response.read(), self.parser)

            reflections = {}
            annotations = {}

            for item in root.xpath('//r:item', namespaces = ns):
                itemName = etree.tostring(item.find('{%s}name' % ns['r']), method="text", encoding=unicode, with_tail=False).lower().strip()
                if itemName not in seenItemNames:
                    for entity in item.xpath('.//r:entity', namespaces = ns):
                        entityType = entity.findtext('{%s}type' % ns['r'])
                        if entityType is not None:
                            entityType = int(entityType)
                        if entityType not in ignoredEntityTypes:
                            entityIdentifier = entity.findtext('{%s}identifier' % ns['r'])
                            if itemName not in reflections:
                                reflections[itemName] = set()
                            reflections[itemName].add((entityType, entityIdentifier))

            # For each match, create an annotation that the UI will handle later
            regex = '(%s)' % '|'.join([re.escape(key) for key in reflections.iterkeys()])
            matches = document.search(regex, IgnoreCase + WholeWordsOnly + RegExp, start = page)
            for match in matches:
                if match.begin().wordArea()[1] == 0:
                    itemName = match.text().lower().strip()
                    annotation = annotations.get(itemName, None)
                    if annotation is None and itemName in reflections:
                        annotation = Annotation()
                        annotation['concept'] = 'Reflection'
                        annotation['property:webpageUrl'] = \
                            'http://reflect.ws/fcgi-bin/solveAmbig.fcgi?entities=%s' % \
                            ';'.join(['%d.%s' % (t, id) for (t, id) in reflections[itemName]])
                        annotation['property:name'] = itemName
                        annotation['session:overlay'] = 'hyperlink'
                        annotation['session:color'] = '#0A0'
                        annotations[itemName] = annotation
                        seenItemNames.add(itemName)
                    if annotation is not None:
                        annotation.addExtent(match)
                    else:
                        print "ERROR: matched '%s' but could not find in reflections map" % itemName.encode('utf8')
                        print reflections.keys()

            document.addAnnotations(annotations.values())

