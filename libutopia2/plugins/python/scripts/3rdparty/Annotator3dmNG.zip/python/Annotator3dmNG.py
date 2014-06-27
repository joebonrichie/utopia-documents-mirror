# encoding: UTF-8

##########################################################################
#
#  3DM plugin for Utopia Documents
#      Copyright (C) 2014 Bio-Prodict B.V., The Netherlands
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program. If not, see <http://www.gnu.org/licenses/>.
#
##########################################################################

import common.utils
import utopia
import utopia.document
import urllib2
from suds import client
from suds.plugin import *
from suds.wsse import *
from suds import WebFault
import codecs
import tempfile
from spineapi import Annotation
import pickle
import spineapi
import re
import logging

logging.getLogger('suds.client').setLevel(logging.CRITICAL)

descriptionSeparator = " | "
conservationDataSeparator = " | "
conservationCutoff = 5.0

ensemblProteinRegex = re.compile('\w*P\d*')
ensemblTranscriptRegex = re.compile('\w*T\d*')
ensemblGeneRegex = re.compile('\w*G\d*')


userHome = os.path.expanduser('~')
fileHome = userHome + '/Library/Utopia/'
if os.name == 'nt':
	fileHome = userHome + '/Utopia/'
jsFiles = ['js/libs/jquery.tablesorter.min.js']
cssFiles = ['css/common.css', 'css/protein.css']

accordionOpenCode = '''
<h3>Other data</h3>
<div id="accordion">
'''

accordionCloseCode = '''
</div>
'''

htmlMutationsInProtein = '''
<h3 data-url="mutationsInProteinUrl"><a href="#">Mutations (at this position)</a></h3>
<div>
    <div class="mutationsInProtein"></div>
    <div id="mutationsInProteinLiterature"></div>
</div>
'''

def parseConservationData(data):
    values = []
    residueTypeValues = {}

    elements = data.split(conservationDataSeparator)
    for element in elements:
        restype,value = element.split(':')
        value = float(value)
        if value > conservationCutoff and restype != 'Gap':
            # print restype
            values.append(value)
            if not residueTypeValues.has_key(value):
                residueTypeValues[value] = []
            residueTypeValues[value].append(restype)

    values.sort()
    values.reverse()
    remaining = 100 - sum(values)
    returnElements = []

    for value in values:
        for residueType in residueTypeValues[value]:
             returnElements.append('"' + residueType + '":' + "%.2f" % value)
    returnElements.append(('"*":%.2f') % (remaining))
    return '{"data":{' + ','.join(returnElements) + '}}'


class MyPlugin(MessagePlugin):
    def marshalled(self, context):
        body = context.envelope.getChild('Header')
        foo = body[0][0][1]
        foo.set('Type', "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-username-token-profile-1.0#PasswordText")

class Annotator3DM(utopia.document.Annotator):
    '''!Annotate with 3DM'''
    annotatedDomains = None
    webrootUrl = '3dm.bio-prodict.nl'
    css = ''
    for file in cssFiles:
        css = css + utopia.get_plugin_data(file).decode('utf-8') + '\n'

    js = ''
    for file in jsFiles:
        js = js + utopia.get_plugin_data(file).decode('utf-8') + '\n'

    css = '<style type="text/css">\n' + css + '</style>'
    proteinJs =  '<script type="text/javascript">\n' + js + utopia.get_plugin_data('js/protein.js').decode('utf-8') + '</script>'
    mutationJs =  '<script type="text/javascript">\n' + js + utopia.get_plugin_data('js/common.js').decode('utf-8') + '</script>'
    residueJs =  '<script type="text/javascript">\n' + js + utopia.get_plugin_data('js/common.js').decode('utf-8') + '</script>'

    def validUsernameAndPassword(self, username, password):
        return password != None and username != None and len(password) > 1 and len(username) > 1

    def populate(self, document):
        # Place a link on the document to test the Javascript messaging functionality

        # self.postToBus('bioprodict', 'prepare')
        username = self.get_config('username')
        password = self.get_config('password')

        if self.validUsernameAndPassword(username, password):
            try:
                databases = self.getAvailableDatabases(username, password)
                databaseIds = []
                databaseDescriptions = []
                for database in databases:
                    databaseIds.append(database['databaseId'])
                    databaseDescriptions.append(database['databaseDescription'])

                annotation = Annotation()
                annotation['concept'] = 'Bio3DMInformation'
                annotation['property:name'] = 'Bio-Prodict 3DM'
                annotation['property:html'] = 'html'
                annotation['property:description'] = '''Annotate this document with one of your 3DM systems'''
                annotation['property:databaseIds'] = '|'.join(databaseIds)
                annotation['property:databaseDescriptions'] = '|'.join(databaseDescriptions)
                annotation['property:sourceDatabase'] = 'bioprodict'
                annotation['property:sourceDescription'] = '<p><a href="http://www.bio-prodict.nl">Bio-Prodict\'s</a> 3DM information systems provide protein family-specific annotations for this article</p>'

                 # a.addExtent(document.substr(100, 300))
                document.addAnnotation(annotation)
            except WebFault as detail:
                print 'Exception:', detail


    def getAvailableDatabases(self, username, password):
        print 'retrieving available databases for user', username

        rootUrl = '3dm.bio-prodict.nl'
        service = '/jwebapp/mcsis-miner-web/webservice/SOAP/'
        wsdl = 'https://' + rootUrl + service + '?wsdl'

        security = Security()
        token = UsernameToken(username, password)
        security.tokens.append(token)

        proxies = utopia.queryProxyString(wsdl)
        proxyDict = {}
        gc = None
        if proxies != 'DIRECT':
            proxyDict['https'] = proxies.split()[1]
            gc = client.Client(wsdl, plugins=[MyPlugin()], location='https://3dm.bio-prodict.nl/jwebapp/mcsis-miner-web/webservice/SOAP/', proxy=proxyDict)
        else:
            gc = client.Client(wsdl, plugins=[MyPlugin()], location='https://3dm.bio-prodict.nl/jwebapp/mcsis-miner-web/webservice/SOAP/')

        gc.set_options(wsse=security)
        gs = gc.service
        databases = gs.getAvailableDatabases()
        #print databases
        return databases['databases']

    def getMentions(self, domain, text, pubmedId):
        tmp = tempfile.gettempdir()
        codecs.open(tmp + os.sep + 'documentText.txt', 'w', 'utf-8').write(text)
        text = codecs.open(tmp + os.sep + 'documentText.txt', 'r', 'utf-8').read()
        username = self.get_config('username')
        password = self.get_config('password')
        if self.validUsernameAndPassword(username, password):
            rootUrl = '3dm.bio-prodict.nl'
            service = '/jwebapp/mcsis-miner-web/webservice/SOAP/'
            wsdl = 'https://' + rootUrl + service + '?wsdl'

            self.proteinJs = self.proteinJs.replace('#USERNAME#', urllib2.quote(self.get_config('username')))
            self.proteinJs = self.proteinJs.replace('#PASSWORD#', urllib2.quote(self.get_config('password')))
            self.mutationJs = self.mutationJs.replace('#USERNAME#', urllib2.quote(self.get_config('username')))
            self.mutationJs = self.mutationJs.replace('#PASSWORD#', urllib2.quote(self.get_config('password')))
            self.residueJs = self.residueJs.replace('#USERNAME#', urllib2.quote(self.get_config('username')))
            self.residueJs = self.residueJs.replace('#PASSWORD#', urllib2.quote(self.get_config('password')))



            security = Security()
            token = UsernameToken(username, password)
            security.tokens.append(token)

            proxies = utopia.queryProxyString(wsdl)
            proxyDict = {}
            gc = None
            if proxies != 'DIRECT':
                proxyDict['https'] = proxies.split()[1]
                gc = client.Client(wsdl, plugins=[MyPlugin()], location='https://3dm.bio-prodict.nl/jwebapp/mcsis-miner-web/webservice/SOAP/', proxy=proxyDict)
            else:
                gc = client.Client(wsdl, plugins=[MyPlugin()], location='https://3dm.bio-prodict.nl/jwebapp/mcsis-miner-web/webservice/SOAP/')

            gc.set_options(wsse=security)
            gs = gc.service
            if (domain == 'hgvs_public'):
                mentions = gs.annotateHGVS(text, pubmedId)
            else:
                mentions = gs.annotate(domain, text, pubmedId)
            print 'retrieved ' + str(len(mentions))
            return mentions
        else:
            print 'No credentials specified'

    def strip_control_characters(self, input):
        if input:
            import re
            # unicode invalid characters
            RE_XML_ILLEGAL = u'([\u0000-\u0008\u000b-\u000c\u000e-\u001f\ufffe-\uffff])' + \
                u'|' + \
                u'([%s-%s][^%s-%s])|([^%s-%s][%s-%s])|([%s-%s]$)|(^[%s-%s])' % \
                (unichr(0xd800),unichr(0xdbff),unichr(0xdc00),unichr(0xdfff),
                 unichr(0xd800),unichr(0xdbff),unichr(0xdc00),unichr(0xdfff),
                 unichr(0xd800),unichr(0xdbff),unichr(0xdc00),unichr(0xdfff),
                 )
            input = re.sub(RE_XML_ILLEGAL, " ", input)
            # ascii control characters
            input = re.sub(r"[\x01-\x1F\x7F]", " ", input)

        return input

    def rewriteData(self, mention):
        newData = {}
        if hasattr(mention, 'data'):
            for d in mention.data[0]:
                if 'key' in d and 'value' in d:
                    newData[d['key']] = d['value']
        return newData

    @utopia.document.buffer
    def annotate(self, document, data = {}):
        action = data.get('action')
        domain = data.get('domain')

        if self.annotatedDomains == None:
            self.annotatedDomains = []

        if action == 'annotate':
            print 'starting 3DM anntotation . . .'
            ns = {'r': 'GPCR'}
            pubmedId = common.utils.metadata(document, 'pmid')
            if pubmedId == None:
                pubmedId = '0'
            print 'sending text to remote server (' + pubmedId + '). . .'
            textMentions = self.getMentions(domain, document.text(), pubmedId)
            print 'recieved response, adding annotations for domain ' + domain + ' . . .'
            objectlist = []
            mention_cache = {}
            for mention in textMentions:
                if mention.mentionType != 'SPECIES' and mention.mentionType != 'PDB':
                    newData = self.rewriteData(mention)
                    mention.data = newData
                    html, css, js = self.buildHtml(domain, mention)
                    mention.html = html.encode('utf-8')
                    mention.css = css.encode('utf-8')
                    mention.js = js.encode('utf-8')
                    mention_cache.setdefault(mention.html, [])
                    mention_cache[mention.html].append(mention)

            for html, mentions in mention_cache.iteritems():
                annotation = self.createAnnotation(domain, document, html, mentions)
                annotation['displayRelevance']='2000'
                annotation['displayRank']= '2000'
                document.addAnnotation(annotation)

            document.addAnnotation(Annotation(), domain)
            print 'done adding annotations.'

    def buildProteinHtml(self, domain, mention):
        proteinId = mention.data['proteinId']
        proteinAc = mention.data['proteinAc']
        proteinDbId = mention.data['proteinDbId']
        proteinDescriptions = mention.data['proteinDescriptions'].split(descriptionSeparator)
        proteinAlternativeNames = ''
        if len(proteinDescriptions) > 1:
            proteinAlternativeNames = '<br/>'.join(proteinDescriptions[1:])
        proteinSpeciesName = mention.data['speciesName']

        structures = None
        if 'structures' in mention.data:
            structures =  mention.data['structures'].split(' | ')

        ensembl = None
        if 'ensembl' in mention.data:
            ensembl = mention.data['ensembl'].split(' | ')

        mim = None
        if 'mim' in mention.data:
            mim = mention.data['mim'].split(' | ')

        mimHtml = ''
        if mim != None:

            mimHtml = "<h3>Additional info</h3><ul><li>OMIM: "
            mimIdHtmls = []
            for mimId in mim:
                mimIdHtmls.append('<a href="http://omim.org/entry/' + mimId + '">' + mimId + '</a>')
            mimHtml += ', '.join(mimIdHtmls)
            mimHtml += '</li></ul><br/>'

        ensemblHtml = ''
        ensemblProtein = None
        ensemblTranscript = None
        ensemblGene = None
        geneHtml, ensemblProteinHtml, ensemblTranscriptHtml, ensemblGeneHtml = '', '', '', ''

        if ensembl != None:
            # ensemblHtml = '<li><b>Ensembl: </b>'
            for ensemblId in ensembl:
                if (ensemblProteinRegex.match(ensemblId)):
                    ensemblProtein = ensemblProteinRegex.findall(ensemblId)[0]
                    ensemblProteinHtml = '<a href="http://www.ensemblgenomes.org/id/' + ensemblProtein + '">Ensembl Protein</a> | '
                if (ensemblGeneRegex.match(ensemblId)):
                    ensemblGene = ensemblGeneRegex.findall(ensemblId)[0]
                    ensemblGeneHtml = '<a href="http://www.ensemblgenomes.org/id/' + ensemblGene + '">Ensembl Gene</a>'
                if (ensemblTranscriptRegex.match(ensemblId)):
                    ensemblTranscript = ensemblTranscriptRegex.findall(ensemblId)[0]
                    ensemblTranscriptHtml = '<a href="http://www.ensemblgenomes.org/id/' + ensemblTranscript + '">Ensembl Transcript</a>'

            geneHtml = '''<h3>Gene</h3>
            <ul>
                <li>{ensemblGeneHtml} | {ensemblTranscriptHtml}</li>
            </ul>'''.format(
                    ensemblGeneHtml = ensemblGeneHtml,
                    ensemblTranscriptHtml = ensemblTranscriptHtml
                )

        contentArray = []

        structureHtml = ''
        if structures != None:
            structureHtml += '<h3 data-url="proteinStructuresUrl"><a href="#">Structures</a></h3>'
            structureHtml += '<div id="structures">'
            structureHtml += '<div id="structuresData">'
            for structure in structures:
                structureHtml += '<div class="pdbIdentifier" style="display:none;">' + structure + '</div>'
            structureHtml += '</div>'
            structureHtml += '<div id="structuresPanel"></div>'
            structureHtml += '</div>'

        sequenceHtml = '''
        <h3 data-url="proteinSequenceUrl"><a href="#">Sequence & Mutations</a></h3>
        <div>
            <div id="sequence">
                <div id="sequenceInfoPanel">
                    <div class="sequenceInfoPanelContents"></div>
                </div>
                 <div id="sequencePanel"></div>
            </div>
        </div>'''

        contentArray.append(structureHtml)
        contentArray.append(sequenceHtml)

        accordionCode = accordionOpenCode + '\n'.join(contentArray) + accordionCloseCode



        html = '''
        <div class="proteinInfoContainer3dm">
            <div class="domainId" style="display:none;">{domainId}</div>
            <div class="proteinDbId" style="display:none;">{proteinDbId}</div>
            <div class="header3dm">
                <div class="proteinId3dm">{proteinId}</div>
                <div class="proteinRecommendedName3dm">{proteinPrimaryDescription}</div>
                <div class="proteinAlternativeNames">{proteinAlternativeNames}</div>
                <div class="inbetween">in</div>
                <div class="proteinSpeciesName">{proteinSpeciesName}</div>
            </div>

            <br/>
            {geneHtml}<br/>

            <h3>Protein</h3>
            <ul>
                <li>{ensemblProteinHtml} <a href="http://www.uniprot.org/uniprot/{proteinId}">UniProt</a> | <a href="https://{serverName}/index.php?&amp;mode=pdetail&amp;proteinName={proteinAc}&amp;familyid=1&amp;filterid=1&amp;numberingscheme=-1&amp;sfamid={domainId}">3DM details</a>
            </ul>
            <br/>

            {mimHtml}

            {accordionCode}

        </div>
        '''.format(
                    proteinId=proteinId,
                    proteinAc = proteinAc,
                    proteinDbId = proteinDbId,
                    proteinPrimaryDescription = proteinDescriptions[0],
                    proteinAlternativeNames = proteinAlternativeNames,
                    proteinSpeciesName = proteinSpeciesName,
                    serverName = self.webrootUrl,
                    domainId = domain,
                    ensemblProteinHtml = ensemblProteinHtml,
                    geneHtml = geneHtml,
                    mimHtml = mimHtml,
                    accordionCode = accordionCode
                    )
        return html

    def buildResidueHtml(self, domain, mention):
        residueNumber3d = 0
        subfamilyName = None
        nonAlignedProtein = False

        if 'subfamilyName' in mention.data:
            subfamilyName = mention.data['subfamilyName']
            residueNumber3d = mention.data['residueNumber3d']
        else:
            nonAlignedProtein = True

        proteinAc = mention.data['proteinAc']
        if "proteinId" in mention.data:
            proteinId = mention.data['proteinId']
        else:
            proteinId = proteinAc

        proteinDbId = mention.data['proteinDbId']
        aminoAcidId = mention.data['aminoAcidId']
        residueType = mention.data['residueType']
        residueNumber = mention.data['residueNumber']
        conservationData = ''

        if mention.data.has_key('subfamilyConservation'):
            conservationData = parseConservationData(mention.data['subfamilyConservation'])

        contentArray= []
        residueLinks = []

        residueBioProdictLink = '<a href="http://{serverName}/index.php?&amp;mode=aadetail&amp;proteinname={proteinAc}&amp;residuenumber={residueNumber}&amp;familyid=1&amp;filterid=1&amp;numberingscheme=-1&amp;sfamid={domainId}">Bio-Prodict details</a>'
        accordionCode = ''

        if len(conservationData) > 1:
            htmlConservation = '''
            <h3><a href="#">Conservation</a></h3>
            <div>
                <div class="accordionText">Residue type occurrences at this position in the superfamily.</div>
                <div id="container3dmConservation"></div>
            </div>
            '''
            contentArray.append(htmlConservation)

        contentArray.append(htmlMutationsInProtein)

        if int(residueNumber3d) > 0:
            htmlMutationsAtEquivalentPositions = '''
            <h3 data-url="mutationsAtEquivalentPositionsUrl"><a href="#">Mutations (at equivalent positions)</a></h3>
            <div>
                <div class="mutationsAtEquivalentPositions"></div>
                <div id="mutationsAtEquivalentPositionsLiterature"></div>
            </div>
            '''
            contentArray.append(htmlMutationsAtEquivalentPositions)
            residueLinks.append('<a href="http://{serverName}/index.php?category=pages&amp;mode=yasarascene&amp;numberingscheme=-1&amp;familyid=1&amp;filterid=1&amp;sfamid={domainId}&amp;template={subfamilyName}&amp;manualpositions={residueNumber3d}">YASARA scene</a>')

        residueLinks.append(residueBioProdictLink)
        residueLinksCode = ' | '.join(residueLinks)
        if nonAlignedProtein:
            residueLinksCode = residueLinksCode.format(
                proteinAc = proteinAc,
                serverName = self.webrootUrl,
                domainId = domain,
                residueNumber = residueNumber,
                residueNumber3d = residueNumber3d
            )
        else:
            residueLinksCode = residueLinksCode.format(
                proteinAc = proteinAc,
                serverName = self.webrootUrl,
                domainId = domain,
                subfamilyName = subfamilyName,
                residueNumber = residueNumber,
                residueNumber3d = residueNumber3d
            )

        accordionCode = accordionOpenCode + '\n'.join(contentArray) + accordionCloseCode


        html = '''
        <div class="residueInfoContainer3dm">
            <div class="domainId" style="display:none;">{domainId}</div>
            <div class="aminoAcidId" style="display:none;">{aminoAcidId}</div>
            <div class="proteinDbId" style="display:none;">{proteinDbId}</div>
            <div class="conservationData" style="display:none;">{conservationData}</div>
            <div class="header3dm">
                <span class="residueType3dm">{residueType}</span>
                <span class="residueNumber3dm">{residueNumber}</span><br/>
                <span class="info3dNumber" style="font-style:italic;">3D number: <span class="residueNumber3d3dm">{residueNumber3d}</span><br/></span>
                <span style="font-style:italic;">in </span><br/>
                <span class="residueProteinUrl3dm">{proteinId}</span>
            </div>

            <h3>Protein</h3>
            <ul>
                <li><a href="http://www.uniprot.org/uniprot/{proteinId}">UniProt</a> | <a href="https://{serverName}/index.php?&amp;mode=pdetail&amp;proteinName={proteinAc}&amp;familyid=1&amp;filterid=1&amp;numberingscheme=-1&amp;sfamid={domainId}">Bio-Prodict details</a><br/><br/>
            </ul>

            <h3>Residue</h3>
            <ul>
                <li> {residueLinksCode} <br/><br/>
            </ul>

            {accordionCode}

        </div>
        ''' .format(
                    proteinId=proteinId,
                    proteinAc = proteinAc,
                    proteinDbId = proteinDbId,
                    aminoAcidId = aminoAcidId,
                    residueType = residueType,
                    residueNumber = residueNumber,
                    residueNumber3d = residueNumber3d,
                    serverName = self.webrootUrl,
                    domainId = domain,
                    accordionCode = accordionCode,
                    conservationData = conservationData,
                    residueLinksCode = residueLinksCode
                    )
        return html

    def buildMutationHtml(self, domain, mention):
        #print mention
        nonAlignedProtein = False
        residueNumber3d = 0

        proteinAc = mention.data['proteinAc']
        if "proteinId" in mention.data:
            proteinId = mention.data['proteinId']
        else:
            proteinId = proteinAc

        if 'subfamilyName' in mention.data:
            subfamilyName = mention.data['subfamilyName']
            residueNumber3d = mention.data['residueNumber3d']
        else:
            nonAlignedProtein = True
        proteinDbId = mention.data['proteinDbId']
        aminoAcidId = mention.data['aminoAcidId']

        residueType = mention.data['residueType']
        mutatedResidueType = mention.data['mutatedResidueType']
        residueNumber = mention.data['residueNumber']
        mutationPubmedIds = mention.data['pubmedIds']
        conservationData = ''

        if mention.data.has_key('subfamilyConservation'):
            conservationData = parseConservationData(mention.data['subfamilyConservation'])

        contentArray= []
        accordionCode = ''
        residueLinks = []

        residueBioProdictLink = '<a href="http://{serverName}/index.php?&amp;mode=aadetail&amp;proteinname={proteinAc}&amp;residuenumber={residueNumber}&amp;familyid=1&amp;filterid=1&amp;numberingscheme=-1&amp;sfamid={domainId}">Bio-Prodict details</a>'


        if len(conservationData) > 1:
            htmlConservation = '''
            <h3><a href="#">Conservation</a></h3>
            <div>
                <div class="accordionText">Residue type occurrences at this position in the superfamily.</div>
                <div id="container3dmConservation"></div>
            </div>
            '''
            contentArray.append(htmlConservation)

        contentArray.append(htmlMutationsInProtein)

        if int(residueNumber3d) > 0:
            htmlMutationsAtEquivalentPositions = '''
            <h3 data-url="mutationsAtEquivalentPositionsUrl"><a href="#">Mutations (at equivalent positions)</a></h3>
            <div>
                <div class="mutationsAtEquivalentPositions" style="padding:0px;max-height:400px;" ></div>
                <div id="mutationsAtEquivalentPositionsLiterature"></div>
            </div>
            '''
            contentArray.append(htmlMutationsAtEquivalentPositions)
            residueLinks.append('<a href="http://{serverName}/index.php?category=pages&amp;mode=yasarascene&amp;numberingscheme=-1&amp;familyid=1&amp;filterid=1&amp;sfamid={domainId}&amp;template={subfamilyName}&amp;manualpositions={residueNumber}">YASARA scene</a>')

        residueLinks.append(residueBioProdictLink)
        residueLinksCode = ' | '.join(residueLinks)
        if nonAlignedProtein:
            residueLinksCode = residueLinksCode.format(
                proteinAc = proteinAc,
                serverName = self.webrootUrl,
                domainId = domain,
                residueNumber = residueNumber
            )
        else:
            residueLinksCode = residueLinksCode.format(
                proteinAc = proteinAc,
                serverName = self.webrootUrl,
                domainId = domain,
                subfamilyName = subfamilyName,
                residueNumber = residueNumber
            )

        accordionCode = accordionOpenCode + '\n'.join(contentArray) + accordionCloseCode

        html = '''
        <div class="mutationInfoContainer3dm">
            <div class="domainId" style="display:none;">{domainId}</div>
            <div class="aminoAcidId" style="display:none;">{aminoAcidId}</div>
            <div class="proteinDbId" style="display:none;">{proteinDbId}</div>
            <div class="conservationData" style="display:none;">{conservationData}</div>
            <div class="header3dm">
                <span class="residueType3dm">{residueType}</span>
                <span class="residueNumber3dm">{residueNumber}</span>
                <span class="residueType3dmMutatedTo">{mutatedResidueType}</span><br/>
                <span class="info3dNumber" style="font-style:italic;">3D number: <span class="residueNumber3d3dm">{residueNumber3d}</span><br/></span>
                <span style="font-style:italic;">in </span><br/>
                <span class="residueProteinUrl3dm">{proteinId}</span>
            </div>


            <h3>Protein</h3>
            <ul>
                <li><a href="http://www.uniprot.org/uniprot/{proteinId}">UniProt</a> | <a href="https://{serverName}/index.php?&amp;mode=pdetail&amp;proteinName={proteinAc}&amp;familyid=1&amp;filterid=1&amp;numberingscheme=-1&amp;sfamid={domainId}">Bio-Prodict details</a><br/><br/>
            </ul>

            <h3>Residue</h3>
            <ul>
                <li> {residueLinksCode} <br/><br/>
            </ul>

            {accordionCode}

        </div>
        ''' .format(
                    proteinId=proteinId,
                    proteinAc = proteinAc,
                    proteinDbId = proteinDbId,
                    aminoAcidId = aminoAcidId,
                    residueType = residueType,
                    mutatedResidueType = mutatedResidueType,
                    residueNumber = residueNumber,
                    residueNumber3d = residueNumber3d,
                    serverName = self.webrootUrl,
                    domainId = domain,
                    accordionCode = accordionCode,
                    conservationData = conservationData,
                    residueLinksCode = residueLinksCode
                    )
        return html

    def buildHgvsVariantHtml(self, domain, mention):
        #print mention
        d = mention.data

        referenceId = d.get( "referenceId", None)
        genomicDescription = d.get( "genomicDescription", '')

        sourceGi = d.get("sourceGi", '')
        errors = d.get("errors", None)
        warnings = d.get("warnings", None)
        sourceAccession = d.get("sourceAccession", None)
        sourceVersion = d.get("sourceVersion", None)
        sourceId = d.get("sourceId", None)
        molecule = d.get("molecule", None)
        chromDescription = d.get("chromDescription", None)
        proteinDescriptions = d.get("proteinDescriptions", None)
        transcriptDescriptions = d.get("transcriptDescriptions", None)
        messages = d.get("messages", None)
        rawVariantDescriptions = d.get("rawVariantDescriptions", None)

        transcriptDescriptionHtml = ''
        proteinDescriptionHtml = ''
        rawVariantDescriptionHtml = ''
        referencesHtml = ''

        genomicDescriptionHtml = genomicDescription.replace('n.', '<b>n.</b>')
        if transcriptDescriptions != None:
            transcriptDescriptionHtml = '<h3>Transcript descriptions</h3>' + '</br>'.join(transcriptDescriptions.replace('c.', '<b>c.</b>').split(' | ')) + '<br/>'

        if proteinDescriptions != None:
            proteinDescriptionHtml = '<h3>Protein descriptions</h3>' + '</br>'.join(proteinDescriptions.replace('p.', '<b>p.</b>').split(' | ')) + '<br/>'

        if rawVariantDescriptions != None:
            rawVariantDescriptionHtml = '<br/>' + '<br/>'.join(rawVariantDescriptions.split(' | '))

        referencesHtml = '<div>NCBI: <a href="http://www.ncbi.nlm.nih.gov/nuccore/' + sourceGi + '">' + sourceAccession + '.' + sourceVersion +'</a></div>'

        variantId = genomicDescription[genomicDescription.index(':')+1:]


        html = '''
        <div class="hgvsVariantInfoContainer">
            <div class="domainId" style="display:none;">{domainId}</div>
            <div class="header3dm">
                <span class="variantId">{variantId}</span><br/><br/>
                <span style="font-style:italic;">in </span><br/><br/>
                <span class="referenceId">{referenceId}</span> <br/>
                <span class="rawVariantDescriptionsHgvs">{rawVariantDescriptionHtml}</span>
            </div>

            <h3>References</h3>
            {referencesHtml}<br/>

            <h3>Genomic description</h3>
            <span class="genomicDescriptionHgvs">{genomicDescription}</span></br></br>

            {transcriptDescriptionHtml}<br/>
            {proteinDescriptionHtml}<br/>
        </div>
        ''' .format(
                    domainId = 'hgvs_public',
                    referenceId = referenceId,
                    variantId = variantId,
                    genomicDescription = genomicDescriptionHtml,
                    transcriptDescriptionHtml=transcriptDescriptionHtml,
                    proteinDescriptionHtml = proteinDescriptionHtml,
                    rawVariantDescriptionHtml = rawVariantDescriptionHtml,
                    referencesHtml = referencesHtml
                    )
        return html



    def createAnnotation(self, domain, document, html, mentions):
        annotation = Annotation()
        annotation['concept'] = 'Bio3DMInformation'
        annotation['property:name'] = '%s: "%s"' % (mentions[0].mentionType.title(), mentions[0].formalRepresentation)
        annotation['property:description'] = '3DM %s record' % mentions[0].mentionType.title()
        annotation['property:sourceDatabase'] = domain
        annotation['property:html'] = mentions[0].html
        annotation['property:css'] = mentions[0].css
        annotation['property:js'] = mentions[0].js
        annotation['property:sourceDatabase'] = 'bioprodict'
        annotation['property:sourceDescription'] = '<p><a href="http://www.bio-prodict.nl">Bio-Prodict\'s</a> 3DM information systems provide protein family-specific annotations for this article</p>'

        for mention in mentions:
            for textRange in mention.textRangeList:
                start = int(textRange.start)
                end = int(textRange.end)
                match = document.substr(start, end-start)
                annotation.addExtent(match);

        return annotation

    def buildHtml(self, domain, mention):
        if mention.mentionType == "PROTEIN":
            html = self.buildProteinHtml(domain, mention)
            return html, self.css, self.proteinJs

        if mention.mentionType == "RESIDUE":
            html = self.buildResidueHtml(domain, mention)
            return html, self.css, self.residueJs

        if mention.mentionType == "MUTATION":
            html = self.buildMutationHtml(domain, mention)
            return html, self.css, self.mutationJs
        if mention.mentionType == "HGVSVARIANT":
            html = self.buildHgvsVariantHtml(domain, mention)
            return html, self.css, self.mutationJs


	### MESSAGE BUS METHODS #############################################################

    def busId(self):
        # Name of this plugin on the message bus.
        return 'bioprodict'

    def event(self, sender, data):
        # Act upon incoming messages.
        print 'RECEIVE FROM BUS', sender, data

    def uuid(self):
        # Plugin ID
        return '{c39b4140-9079-11d2-9b7b-0002a5d5c51b}'


class Visualiser3DM(utopia.document.Visualiser):
    """Viualiser for 3DM entries"""

    def visualisable(self, annotation):
        return annotation.get('concept') == 'Bio3DMInformation' and 'property:html' in annotation

    def visualise(self, annotation):
        if 'property:databaseIds' in annotation:
            databaseIds = annotation['property:databaseIds'].split('|')
            databaseDescriptions = annotation['property:databaseDescriptions'].split('|')
            html = '<div><ul>'
            for i in range(len(databaseIds)):
                html += '''<li><a class="domainSelector" style="cursor:pointer; color:inherit; text-decoration: none;" onclick="console.log('hello'); enableDomainlistColors(this); papyro.result(this).postMessage('papyro.queue', {{'uuid': '{0}', 'data': {{'action': 'annotate', 'domain': '{1}'}}}});">'''.format('{c39b4140-9079-11d2-9b7b-0002a5d5c51b}', databaseIds[i]) + databaseDescriptions[i] + '</a></li>'
            #html += '''<li><a class="domainSelector" style="cursor:pointer; color:inherit; text-decoration: none;" onclick="console.log('hello'); enableDomainlistColors(this); papyro.result(this).postMessage('papyro.queue', {{'uuid': '{0}', 'data': {{'action': 'annotate', 'domain': 'hgvs_public'}}}});">'''.format('{c39b4140-9079-11d2-9b7b-0002a5d5c51b}') + 'HGVS variants' + '</a></li>'
            html += '</ul><div>'
            js = '''<script>
                var enableDomainlistColors = function (selectedDomain) {
                    console.log($('.domainSelector'));
                    $('.domainSelector').css("color","lightgrey");
                    $('.domainSelector').attr('onclick', '')
                    $('.domainSelector').css("cursor","default");
                    $(selectedDomain).css("color","grey");
            	    console.log(selectedDomain);
                }</script>'''
            return js, html

        html = u'<div class="threedm">' + annotation['property:html'] + u'</div>'
        css = annotation['property:css']
        js = annotation['property:js']
        return html, css, js

class BioProdictConf(utopia.Configurator):

    def form(self):
        # Actual configuration form
        return '''
            <!DOCTYPE html>
            <html>
              <body>
                <p>
                  Bio-Prodict delivers solutions for scientific research in protein engineering, molecular design and DNA diagnostics.
                  We apply novel approaches to mining, storage and analysis of protein data and combine these with state-of-the art analysis methods and visualization tools to create
                  custom-built information systems for protein superfamilies.
                </p>

                <p>
                  In order to make use of Bio-Prodict's 3DM information systems in Utopia Documents, please contact us for licensing options. More information can be found at <a href="http://www.bio-prodict.nl/">Bio-Prodict.nl</a>.
                </p>
                <table>
                  <tr>
                    <td style="text-align: right"><label for="username">Username:</label></td>
                    <td><input name="username" id="username" type="text" /></td>
                  </tr>
                  <tr>
                    <td style="text-align: right"><label for="password">Password:</label></td>
                    <td><input name="password" id="password" type="password" /></td>
                  </tr>
                </table>
              </body>
            </html>
        '''

    def icon(self):
        # Data URI of configuration logo
        return utopia.get_plugin_data_as_url('images/3dm-prefs-logo.png', 'image/png')

    def title(self):
        # Name of plugin in configuration panel
        return '3DM'

    def uuid(self):
        # Configuration ID
        return '{c39b4140-9079-11d2-9b7b-0002a5d5c51b}'

__all__ = ['Annotator3DM', 'Visualiser3DM']
