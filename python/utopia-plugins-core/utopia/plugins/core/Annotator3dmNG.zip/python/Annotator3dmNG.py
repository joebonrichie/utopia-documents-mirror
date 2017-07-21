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

# encoding: UTF-8
import codecs
import json
import re
import os
import tempfile
import urllib
import urllib2

import utopia.tools.utils
from spineapi import Annotation
import utopia.document


descriptionSeparator = " | "
conservationDataSeparator = " | "
conservationCutoff = 5.0

ensemblProteinRegex = re.compile(r'\w*P\d*')
ensemblTranscriptRegex = re.compile(r'\w*T\d*')
ensemblGeneRegex = re.compile(r'\w*G\d*')

jsFiles = ['js/libs/jquery.tablesorter.js']
cssFiles = ['css/common.css', 'css/protein.css']

accordionOpenCode = '''
<h3>Other data</h3>
<div class="accordion">
'''

accordionCloseCode = '''
</div>
'''

htmlMutationsInProtein = '''
<h3 data-url="mutationsInProteinUrl"><a href="#">Mutations (at this position)</a></h3>
<div>
    <div class="mutationsInProtein"></div>
    <div class="mutationsInProteinLiterature"></div>
</div>
'''


def post_for_json(url, authorization=None, data=None, plain=False):
    request = urllib2.Request(url)
    if authorization:
        request.add_header('Authorization', authorization)
    if data:
        if plain:
            request.add_header('Content-Type', 'text/plain')
        else:
            data = urllib.urlencode(data)
        request.add_data(data)
    output = urllib2.urlopen(request, timeout=12).read()
    return json.loads(output)


def parseConservationData(data):
    values = []
    residueTypeValues = {}

    elements = data.split(conservationDataSeparator)
    for element in elements:
        restype, value = element.split(':')
        value = float(value)
        if value > conservationCutoff and restype != 'Gap':
            values.append(value)
            if value not in residueTypeValues:
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


class Annotator3DM(utopia.document.Annotator):
    '''!Annotate with 3DM'''
    annotatedDomains = None

    # Compose paths to 3DM for links and SOAP web service from parts
    threedmUrl = 'https://3dm.bio-prodict.nl'
    baseurl = 'https://api.bio-prodict.nl/'
    tokenurl = baseurl + 'oauth/oauth/token'
    databasesurl = baseurl + 'miner/databases'
    annotateurl = baseurl + 'miner/annotate/'

    def __init__(self):
        self.css = ''
        self.js = ''
        for file in cssFiles:
            self.css += utopia.get_plugin_data(file).decode('utf-8') + '\n'
        for file in jsFiles:
            self.js += utopia.get_plugin_data(file).decode('utf-8') + '\n'
        self.css = '<style type="text/css">\n' + self.css + '</style>'
        self.proteinJs = '<script type="text/javascript">\n(function(){\n' + self.js + utopia.get_plugin_data('js/protein.js').decode('utf-8') + '\n})();\n</script>'
        self.commonJs = '<script type="text/javascript">\n(function(){\n' + self.js + utopia.get_plugin_data('js/common.js').decode('utf-8') + '\n})();\n</script>'

    def icon(self):
        # Data URI of configuration logo
        return utopia.get_plugin_data_as_url('images/3dm-prefs-logo.png', 'image/png')

    def validUsernameAndPassword(self, username, password):
        return password is not None and username is not None and len(password) > 1 and len(username) > 1

    def on_ready_event(self, document):
        username = self.get_config('username')
        password = self.get_config('password')

        if self.validUsernameAndPassword(username, password):
            # Get a new bearer token
            basic = 'Basic dXRvcGlhLXBsdWdpbjo='  # base64.encodestring('utopia-plugin:').replace('\n', '')
            data = dict(username=username, password=password, grant_type='password')
            content = post_for_json(self.tokenurl, basic, data)
            self.bearer = 'Bearer ' + content['access_token']
            self.proteinJs = self.proteinJs.replace('#TOKEN#', self.bearer)
            self.commonJs = self.commonJs.replace('#TOKEN#', self.bearer)

            # Get available databases for user
            databases = post_for_json(self.databasesurl, self.bearer)
            sorted_databases = sorted(databases.items(), key=lambda item: item[1])
            databaseIds = [item[0] for item in sorted_databases]
            databaseDescriptions = [item[1] for item in sorted_databases]

            annotation = Annotation()
            annotation['concept'] = 'Bio3DMInformation'
            annotation['property:name'] = 'Bio-Prodict 3DM'
            annotation['property:html'] = 'html'
            annotation['session:overlay'] = 'hyperlink'
            annotation['session:color'] = '#336611'
            annotation['property:description'] = '''Annotate using one of your 3DM systems'''
            annotation['property:databaseIds'] = '|'.join(databaseIds)
            annotation['property:databaseDescriptions'] = '|'.join(databaseDescriptions)
            annotation['property:sourceDatabase'] = 'bioprodict'
            annotation['property:sourceDescription'] = '<p><a href="http://www.bio-prodict.nl">Bio-Prodict\'s</a> 3DM information systems provide protein family-specific annotations for this article</p>'

            document.addAnnotation(annotation)

            # FOR DEBUG ONLY
            # self.on_activate_event(document, dict(action='annotate', domain='NR_demo_2008'))

    def getMentions(self, domain, text):
        tmp = tempfile.gettempdir()
        codecs.open(tmp + os.sep + 'documentText.txt', 'w', 'utf-8').write(text)
        text = codecs.open(tmp + os.sep + 'documentText.txt', 'r', 'utf-8').read()
        if self.bearer is not None and len(self.bearer) > 1:
            # Get mentions
            mentions = post_for_json(self.annotateurl + domain, self.bearer, text.encode('utf-8'), plain=True)
            print 'retrieved ' + str(len(mentions)) + ' mentions'
            return mentions
        else:
            print 'no bearer token to call ' + self.annotateurl

    @utopia.document.buffer
    def on_activate_event(self, document, data={}):
        action = data.get('action')
        domain = data.get('domain')

        if self.annotatedDomains is None:
            self.annotatedDomains = []

        if action == 'annotate':
            print 'starting 3DM annotation . . .'
            pubmedId = utopia.tools.utils.metadata(document, 'identifiers[pubmed]')
            if pubmedId is None:
                pubmedId = '0'
            print 'sending text to remote server (' + pubmedId + '). . .'
            textMentions = self.getMentions(domain, document.text())
            print 'received response, adding annotations for domain ' + domain + ' . . .'
            mention_cache = {}
            for mention in textMentions:
                if mention['mentionType'] != 'SPECIES' and mention['mentionType'] != 'PDB':
                    html, css, js = self.buildHtml(domain, mention)
                    mention['html'] = html.encode('utf-8')
                    mention['css'] = css.encode('utf-8')
                    mention['js'] = js.encode('utf-8')
                    mention_cache.setdefault(mention['html'], [])
                    mention_cache[mention['html']].append(mention)

            for html, mentions in mention_cache.iteritems():
                annotation = self.createAnnotation(domain, document, html, mentions)
                annotation['displayRelevance'] = '2000'
                annotation['displayRank'] = '2000'
                document.addAnnotation(annotation)

            document.addAnnotation(Annotation(), domain)
            print 'done adding annotations.'

    def buildProteinHtml(self, domain, mention):
        proteinId = mention['data']['proteinId']
        proteinAc = mention['data']['proteinAc']
        proteinDbId = mention['data']['proteinDbId']
        proteinDescriptions = mention['data']['proteinDescriptions'].split(descriptionSeparator)
        proteinAlternativeNames = ''
        if len(proteinDescriptions) > 1:
            proteinAlternativeNames = '<br/>'.join(proteinDescriptions[1:])
        proteinSpeciesName = mention['data']['speciesName']

        structures = None
        if 'structures' in mention['data']:
            structures = mention['data']['structures'].split(' | ')

        ensembl = None
        if 'ensembl' in mention['data']:
            ensembl = mention['data']['ensembl'].split(' | ')

        mim = None
        if 'mim' in mention['data']:
            mim = mention['data']['mim'].split(' | ')

        mimHtml = ''
        if mim is not None:

            mimHtml = "<h3>Additional info</h3><ul><li>OMIM: "
            mimIdHtmls = []
            for mimId in mim:
                mimIdHtmls.append('<a href="http://omim.org/entry/' + mimId + '">' + mimId + '</a>')
            mimHtml += ', '.join(mimIdHtmls)
            mimHtml += '</li></ul><br/>'

        # ensemblHtml = ''
        ensemblProtein = None
        ensemblTranscript = None
        ensemblGene = None
        geneHtml, ensemblProteinHtml, ensemblTranscriptHtml, ensemblGeneHtml = '', '', '', ''

        if ensembl is not None:
            # ensemblHtml = '<li><b>Ensembl: </b>'
            for ensemblId in ensembl:
                if ensemblProteinRegex.match(ensemblId):
                    ensemblProtein = ensemblProteinRegex.findall(ensemblId)[0]
                    ensemblProteinHtml = '<a href="http://www.ensemblgenomes.org/id/' + ensemblProtein + '">Ensembl Protein</a> | '
                if ensemblGeneRegex.match(ensemblId):
                    ensemblGene = ensemblGeneRegex.findall(ensemblId)[0]
                    ensemblGeneHtml = '<a href="http://www.ensemblgenomes.org/id/' + ensemblGene + '">Ensembl Gene</a>'
                if ensemblTranscriptRegex.match(ensemblId):
                    ensemblTranscript = ensemblTranscriptRegex.findall(ensemblId)[0]
                    ensemblTranscriptHtml = '<a href="http://www.ensemblgenomes.org/id/' + ensemblTranscript + '">Ensembl Transcript</a>'

            geneHtml = '''<h3>Gene</h3>
            <ul>
                <li>{ensemblGeneHtml} | {ensemblTranscriptHtml}</li>
            </ul>'''.format(ensemblGeneHtml=ensemblGeneHtml,
                            ensemblTranscriptHtml=ensemblTranscriptHtml)

        contentArray = []

        structureHtml = ''
        if structures is not None:
            structureHtml += '<h3 data-url="proteinStructuresUrl"><a href="#">Structures</a></h3>'
            structureHtml += '<div class="structures">'
            structureHtml += '<div class="structuresData">'
            for structure in structures:
                structureHtml += '<div class="pdbIdentifier" style="display:none;">' + structure + '</div>'
            structureHtml += '</div>'
            structureHtml += '<div class="structuresPanel"></div>'
            structureHtml += '</div>'

        sequenceHtml = '''
        <h3 data-url="proteinSequenceUrl"><a href="#">Sequence & Mutations</a></h3>
        <div>
            <div class="sequence">
                <div class="sequenceInfoPanel">
                    <div class="sequenceInfoPanelContents"></div>
                </div>
                 <div class="sequencePanel"></div>
            </div>
        </div>'''

        contentArray.append(structureHtml)
        contentArray.append(sequenceHtml)

        accordionCode = accordionOpenCode + '\n'.join(contentArray) + accordionCloseCode

        html = '''
        <div class="proteinInfoContainer3dm">
            <div class="domainId" style="display:none;">{domainId}</div>
            <div class="proteinDbId" style="display:none;">{proteinDbId}</div>
            <div class="header3dm box">
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
                <li>{ensemblProteinHtml} <a href="http://www.uniprot.org/uniprot/{proteinId}">UniProt</a> | <a href="{serverName}/index.php?&amp;mode=pdetail&amp;proteinName={proteinAc}&amp;familyid=1&amp;filterid=1&amp;numberingscheme=-1&amp;sfamid={domainId}">3DM details</a>
            </ul>
            <br/>

            {mimHtml}

            {accordionCode}

        </div>
        '''.format(proteinId=proteinId,
                   proteinAc=proteinAc,
                   proteinDbId=proteinDbId,
                   proteinPrimaryDescription=proteinDescriptions[0],
                   proteinAlternativeNames=proteinAlternativeNames,
                   proteinSpeciesName=proteinSpeciesName,
                   serverName=self.threedmUrl,
                   domainId=domain,
                   ensemblProteinHtml=ensemblProteinHtml,
                   geneHtml=geneHtml,
                   mimHtml=mimHtml,
                   accordionCode=accordionCode)
        return html

    def buildResidueHtml(self, domain, mention):
        residueNumber3d = 0
        subfamilyName = None
        nonAlignedProtein = False

        if 'subfamilyName' in mention['data']:
            subfamilyName = mention['data']['subfamilyName']
            residueNumber3d = mention['data']['residueNumber3d']
        else:
            nonAlignedProtein = True

        proteinAc = mention['data']['proteinAc']
        if "proteinId" in mention['data']:
            proteinId = mention['data']['proteinId']
        else:
            proteinId = proteinAc

        proteinDbId = mention['data']['proteinDbId']
        aminoAcidId = mention['data']['aminoAcidId']
        residueType = mention['data']['residueType']
        residueNumber = mention['data']['residueNumber']
        conservationData = ''

        if 'subfamilyConservation' in mention['data']:
            conservationData = parseConservationData(mention['data']['subfamilyConservation'])

        contentArray = []
        residueLinks = []

        residueBioProdictLink = '<a href="{serverName}/index.php?&amp;mode=aadetail&amp;proteinname={proteinAc}&amp;residuenumber={residueNumber}&amp;familyid=1&amp;filterid=1&amp;numberingscheme=-1&amp;sfamid={domainId}">Bio-Prodict details</a>'

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
                <div class="mutationsAtEquivalentPositionsLiterature"></div>
            </div>
            '''
            contentArray.append(htmlMutationsAtEquivalentPositions)
            residueLinks.append('<a href="{serverName}/index.php?category=pages&amp;mode=yasarascene&amp;numberingscheme=-1&amp;familyid=1&amp;filterid=1&amp;sfamid={domainId}&amp;template={subfamilyName}&amp;manualpositions={residueNumber3d}">YASARA scene</a>')

        residueLinks.append(residueBioProdictLink)
        residueLinksCode = ' | '.join(residueLinks)
        if nonAlignedProtein:
            residueLinksCode = residueLinksCode.format(
                proteinAc=proteinAc,
                serverName=self.threedmUrl,
                domainId=domain,
                residueNumber=residueNumber,
                residueNumber3d=residueNumber3d
            )
        else:
            residueLinksCode = residueLinksCode.format(
                proteinAc=proteinAc,
                serverName=self.threedmUrl,
                domainId=domain,
                subfamilyName=subfamilyName,
                residueNumber=residueNumber,
                residueNumber3d=residueNumber3d
            )

        accordionCode = accordionOpenCode + '\n'.join(contentArray) + accordionCloseCode

        html = '''
        <div class="residueInfoContainer3dm">
            <div class="domainId" style="display:none;">{domainId}</div>
            <div class="aminoAcidId" style="display:none;">{aminoAcidId}</div>
            <div class="proteinDbId" style="display:none;">{proteinDbId}</div>
            <div class="conservationData" style="display:none;">{conservationData}</div>
            <div class="header3dm box">
                <span class="residueType3dm">{residueType}</span>
                <span class="residueNumber3dm">{residueNumber}</span><br/>
                <span class="info3dNumber" style="font-style:italic;">3D number: <span class="residueNumber3d3dm">{residueNumber3d}</span><br/></span>
                <span style="font-style:italic;">in </span><br/>
                <span class="residueProteinUrl3dm">{proteinId}</span>
            </div>

            <h3>Protein</h3>
            <ul>
                <li><a href="http://www.uniprot.org/uniprot/{proteinId}">UniProt</a> | <a href="{serverName}/index.php?&amp;mode=pdetail&amp;proteinName={proteinAc}&amp;familyid=1&amp;filterid=1&amp;numberingscheme=-1&amp;sfamid={domainId}">Bio-Prodict details</a><br/><br/>
            </ul>

            <h3>Residue</h3>
            <ul>
                <li> {residueLinksCode} <br/><br/>
            </ul>

            {accordionCode}

        </div>
        ''' .format(proteinId=proteinId,
                    proteinAc=proteinAc,
                    proteinDbId=proteinDbId,
                    aminoAcidId=aminoAcidId,
                    residueType=residueType,
                    residueNumber=residueNumber,
                    residueNumber3d=residueNumber3d,
                    serverName=self.threedmUrl,
                    domainId=domain,
                    accordionCode=accordionCode,
                    conservationData=conservationData,
                    residueLinksCode=residueLinksCode)
        return html

    def buildMutationHtml(self, domain, mention):
        nonAlignedProtein = False
        residueNumber3d = 0

        proteinAc = mention['data']['proteinAc']
        if "proteinId" in mention['data']:
            proteinId = mention['data']['proteinId']
        else:
            proteinId = proteinAc

        if 'subfamilyName' in mention['data']:
            subfamilyName = mention['data']['subfamilyName']
            residueNumber3d = mention['data']['residueNumber3d']
        else:
            nonAlignedProtein = True
        proteinDbId = mention['data']['proteinDbId']
        aminoAcidId = mention['data']['aminoAcidId']

        residueType = mention['data']['residueType']
        mutatedResidueType = mention['data']['mutatedResidueType']
        residueNumber = mention['data']['residueNumber']
        # mutationPubmedIds = mention['data']['pubmedIds']
        conservationData = ''

        if 'subfamilyConservation' in mention['data']:
            conservationData = parseConservationData(mention['data']['subfamilyConservation'])

        contentArray = []
        residueLinks = []

        residueBioProdictLink = '<a href="{serverName}/index.php?&amp;mode=aadetail&amp;proteinname={proteinAc}&amp;residuenumber={residueNumber}&amp;familyid=1&amp;filterid=1&amp;numberingscheme=-1&amp;sfamid={domainId}">Bio-Prodict details</a>'

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
                <div class="mutationsAtEquivalentPositionsLiterature"></div>
            </div>
            '''
            contentArray.append(htmlMutationsAtEquivalentPositions)
            residueLinks.append('<a href="{serverName}/index.php?category=pages&amp;mode=yasarascene&amp;numberingscheme=-1&amp;familyid=1&amp;filterid=1&amp;sfamid={domainId}&amp;template={subfamilyName}&amp;manualpositions={residueNumber}">YASARA scene</a>')

        residueLinks.append(residueBioProdictLink)
        residueLinksCode = ' | '.join(residueLinks)
        if nonAlignedProtein:
            residueLinksCode = residueLinksCode.format(
                proteinAc=proteinAc,
                serverName=self.threedmUrl,
                domainId=domain,
                residueNumber=residueNumber
            )
        else:
            residueLinksCode = residueLinksCode.format(
                proteinAc=proteinAc,
                serverName=self.threedmUrl,
                domainId=domain,
                subfamilyName=subfamilyName,
                residueNumber=residueNumber
            )

        accordionCode = accordionOpenCode + '\n'.join(contentArray) + accordionCloseCode

        html = '''
        <div class="mutationInfoContainer3dm">
            <div class="domainId" style="display:none;">{domainId}</div>
            <div class="aminoAcidId" style="display:none;">{aminoAcidId}</div>
            <div class="proteinDbId" style="display:none;">{proteinDbId}</div>
            <div class="conservationData" style="display:none;">{conservationData}</div>
            <div class="header3dm box">
                <span class="residueType3dm">{residueType}</span>
                <span class="residueNumber3dm">{residueNumber}</span>
                <span class="residueType3dmMutatedTo">{mutatedResidueType}</span><br/>
                <span class="info3dNumber" style="font-style:italic;">3D number: <span class="residueNumber3d3dm">{residueNumber3d}</span><br/></span>
                <span style="font-style:italic;">in </span><br/>
                <span class="residueProteinUrl3dm">{proteinId}</span>
            </div>


            <h3>Protein</h3>
            <ul>
                <li><a href="http://www.uniprot.org/uniprot/{proteinId}">UniProt</a> | <a href="{serverName}/index.php?&amp;mode=pdetail&amp;proteinName={proteinAc}&amp;familyid=1&amp;filterid=1&amp;numberingscheme=-1&amp;sfamid={domainId}">Bio-Prodict details</a><br/><br/>
            </ul>

            <h3>Residue</h3>
            <ul>
                <li> {residueLinksCode} <br/><br/>
            </ul>

            {accordionCode}

        </div>
        ''' .format(proteinId=proteinId,
                    proteinAc=proteinAc,
                    proteinDbId=proteinDbId,
                    aminoAcidId=aminoAcidId,
                    residueType=residueType,
                    mutatedResidueType=mutatedResidueType,
                    residueNumber=residueNumber,
                    residueNumber3d=residueNumber3d,
                    serverName=self.threedmUrl,
                    domainId=domain,
                    accordionCode=accordionCode,
                    conservationData=conservationData,
                    residueLinksCode=residueLinksCode)
        return html

    def createAnnotation(self, domain, document, html, mentions):
        annotation = Annotation()
        annotation['concept'] = 'Bio3DMInformation'
        annotation['property:name'] = '%s: "%s"' % (mentions[0]['mentionType'].title(), mentions[0]['formalRepresentation'])
        annotation['property:description'] = '3DM %s record' % mentions[0]['mentionType'].title()
        annotation['property:sourceDatabase'] = domain
        annotation['property:html'] = mentions[0]['html']
        annotation['property:css'] = mentions[0]['css']
        annotation['property:js'] = mentions[0]['js']
        annotation['property:sourceDatabase'] = 'bioprodict'
        annotation['property:sourceDescription'] = '<p><a href="http://www.bio-prodict.nl">Bio-Prodict\'s</a> 3DM information systems provide protein family-specific annotations for this article</p>'
        annotation['session:overlay'] = 'hyperlink'
        annotation['session:color'] = '#336611'

        for mention in mentions:
            for textRange in mention['textRangeList']:
                start = int(textRange['start'])
                end = int(textRange['end'])
                match = document.substr(start, end - start)
                annotation.addExtent(match)

        return annotation

    def buildHtml(self, domain, mention):
        if mention['mentionType'] == "PROTEIN":
            html = self.buildProteinHtml(domain, mention)
            return html, self.css, self.proteinJs

        if mention['mentionType'] == "RESIDUE":
            html = self.buildResidueHtml(domain, mention)
            return html, self.css, self.commonJs

        if mention['mentionType'] == "MUTATION":
            html = self.buildMutationHtml(domain, mention)
            return html, self.css, self.commonJs

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
                html += '''<li><a class="domainSelector" style="cursor:pointer; color:inherit; text-decoration: none;" onclick="console.log('hello'); enableDomainlistColors(this); utopia.result(this).postMessage('papyro.queue', {{'uuid': '{0}', 'data': {{'action': 'annotate', 'domain': '{1}'}}}});">'''.format('{c39b4140-9079-11d2-9b7b-0002a5d5c51b}', databaseIds[i]) + databaseDescriptions[i] + '</a></li>'
            # html += '''<li><a class="domainSelector" style="cursor:pointer; color:inherit; text-decoration: none;" onclick="console.log('hello'); enableDomainlistColors(this); utopia.result(this).postMessage('papyro.queue', {{'uuid': '{0}', 'data': {{'action': 'annotate', 'domain': 'hgvs_public'}}}});">'''.format('{c39b4140-9079-11d2-9b7b-0002a5d5c51b}') + 'HGVS variants' + '</a></li>'
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
            return html, js
        html = u'<div class="threedm">' + annotation['property:html'] + u'</div>'
        css = annotation['property:css']
        js = annotation['property:js']
        return css, html, js


class BioProdictConf(utopia.Configurator):

    def form(self):
        # Actual configuration form
        return '''
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