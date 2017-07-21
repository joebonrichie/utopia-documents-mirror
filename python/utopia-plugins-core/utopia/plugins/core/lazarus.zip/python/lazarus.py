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

#? urls: https://utopia.cs.manchester.ac.uk/lazarus/

import utopia.tools.utils
import utopia.tools.converters.Annotation
import kend.converter.XML
import kend.model
import datetime
import json
import re
import spineapi
import urllib
import urllib2
import urlparse
import uuid
import utopia
import utopia.citation
import utopia.library

try:
    import utopia.document
except:
    class utopia:
     class document:
      class Annotator:
       pass
      @staticmethod
      def buffer(fn):
       def wrap(*args, **kwargs):
        return fn(*args, **kwargs)
       return wrap
from lxml import etree

from pprint import PrettyPrinter
pprint = PrettyPrinter(indent=4).pprint



laz_plugin_uuid = '{a5ab0e7d-0210-4be4-bd69-51f946e16ff2}'

# Base URL of the Lazarus service
laz_version = '0.2'
laz_baseUrl = 'https://utopia.cs.manchester.ac.uk/lazarus/'+laz_version
laz_docUrl = '{0}/document'.format(laz_baseUrl)
laz_docRelUrl = '{0}/related'.format(laz_docUrl)
laz_citUrl = '{0}/citation'.format(laz_baseUrl)
laz_citDefUrl = '{0}/define'.format(laz_citUrl)
laz_citResUrl = '{0}/resolve'.format(laz_citUrl)
laz_interactionUrl = '{0}/interaction'.format(laz_baseUrl)
laz_eventUrl = '{0}/event'.format(laz_interactionUrl)



def send_crowdsource_event(event, document = None):
    try:
        url = laz_eventUrl
        if document is not None:
            params = {'fingerprint': document.fingerprints()}
            url += '?{0}'.format(urllib.urlencode(params, doseq=True))

        # Some debug stuff
        print("Sending event to: " + url)
        import pprint
        pp = pprint.PrettyPrinter(indent=4).pprint
        pp(event)

        request = urllib2.Request(url, data=json.dumps(event),
                                  headers={'Content-Type': 'application/json'})
        response = urllib2.urlopen(request)
        # Are we interested in the response?
    except Exception as e:
        import traceback
        traceback.print_exc()



class _LazarusResolver(utopia.citation.Resolver):
    '''Let the Lazarus server have a crack.'''

    def resolve(self, metadata, document = None):
        update = {}
        ids = metadata.get('identifiers', {})

        # Send this off to Lazarus to try to resolve
        try:
            #params = {'fingerprint': document.fingerprints()}
            url = laz_citResUrl
            #url = url.format(urllib.urlencode(params, doseq=True))
            headers = {'Content-Type': 'application/json'}
            to_dump = {}
            for k, v in metadata.items():
                if isinstance(v, datetime.datetime):
                    to_dump[k] = v.isoformat()
                else:
                    to_dump[k] = v
            data = json.dumps([to_dump])
            #print(url)
            #print(headers)
            #print(data)
            request = urllib2.Request(url, headers=headers, data=data)
            response = urllib2.urlopen(request, timeout=12)
            for k, v in metadata.items():
                update[k] = None
            update.update(json.load(response)[0])
            update['_action'] = 'stop'
            import pprint
            pprint.PrettyPrinter(indent=4).pprint(update)
        except urllib2.HTTPError as e:
            import pprint
            pprint.PrettyPrinter(indent=4).pprint(json.load(e))
            import traceback
            traceback.print_exc()
        except Exception as e:
            import traceback
            traceback.print_exc()

        return update

    def purposes(self):
        return ('dereference', 'identify', 'expand')

    def weight(self):
        return -999


class LazarusAnnotator(utopia.document.Annotator):
    '''Lazarus service integration'''

    # Description info
    sourceDescription = '''
        <div><a href="http://utopiadocs.com/lazarus">Lazarus</a> uncovers inter-connections buried in scientific literature</div>
    '''

    dbs = {
        'scibite:ANAT': {
            'title': 'Anatomy',
        },
        'scibite:BIOPROC': {
            'title': 'Biological Process',
        },
        'scibite:CELLLINE': {
            'title': 'Cell Line',
        },
        'scibite:COMPANY': {
            'title': 'Company/Organisation',
        },
        'scibite:DBSNP': {
            'title': 'dbSNP',
        },
        'scibite:DRUG': {
            'title': 'Drug',
        },
        'scibite:GENE': {
            'title': 'Gene',
        },
        'scibite:GENEONT': {
            'title': 'Gene Ontology',
        },
        'scibite:INDICATION': {
            'title': 'Indication',
        },
        'scibite:LABCHEM': {
            'title': 'Lab Chemical',
        },
        'scibite:LABPROC': {
            'title': 'Lab Process',
        },
        'scibite:MIRNA': {
            'title': 'miRNA',
        },
        'scibite:OSCAR': {
            'title': 'Chemical',
        },
        'scibite:PROTYP': {
            'title': 'Protein Type',
        },
        'scibite:SPECIES': {
            'title': 'Species',
        },
    }

    # Utility function for parsing out header roles from h2c output
    headerRoles = re.compile(r'^h(\d+)$')
    def getHeaderRole(self, annotation):
        '''Return the header role of this annotation, or None if none present'''
        for role in annotation.getAllProperties('structure:role'):
            match = self.headerRoles.match(role)
            if match is not None:
                return (role, int(match.group(1)))
        return (None, -1)

    def after_load_event(self, document):
        anon_user_id = utopia.bridge.context['anonymous-user-id']
        now = datetime.datetime.now().replace(microsecond=0).isoformat()

        event = {
            'action': 'load',
            'anonymous-user-id': anon_user_id,
            'time': now,
            'context': {
                'document': utopia.tools.utils.metadata(document, 'identifiers[utopia]')
            }
        }

        origin = utopia.tools.utils.metadata(document, 'provenance[origin]')
        if origin is not None:
            origin['anonymous-user-id'] = anon_user_id
            event['origin'] = origin
        else:
            uri = utopia.tools.utils.metadata(document, 'originating-uri')
            if uri is not None:
                if uri.startsWith('file:'):
                    event['origin'] = {
                        'action': 'openfile',
                        'anonymous-user-id': anon_user_id,
                        'time': now
                    }
                elif uri.startsWith('http:') or uri.startsWith('https:'):
                    domain_name = urlparse.urlparse(uri).hostname
                    event['origin'] = {
                        'action': 'download',
                        'anonymous-user-id': anon_user_id,
                        'time': now,
                        'context': {
                            'domain': domain_name
                        }
                    }

        send_crowdsource_event(event, document)

    @utopia.document.buffer
    def on_ready_event(self, document):
        '''Fetch information from the Lazarus service'''

        permission = self.get_config('permission', False)
        if permission:
            # If an outline already exists, don't make a new one
            needs_outline = True
            for annotation in document.annotations():
                if annotation.get('concept') == 'OutlineItem':
                    needs_outline = False
                    break

            # The Lazarus server needs to know what this document is
            document_id = utopia.tools.utils.metadata(document, 'identifiers[utopia]')
            this_doi = utopia.tools.utils.metadata(document, 'identifiers[doi]')
            if this_doi is not None:
                this_doi = u'doi:' + this_doi

            # Speak to server
            params = {'fingerprint': document.fingerprints()}
            url = '{0}?{1}'.format(laz_docUrl, urllib.urlencode(params, doseq=True))
            response = urllib2.urlopen(url, timeout=60)
            if response.getcode() == 204:
                request = urllib2.Request(url, data=document.data(),
                                          headers={'Content-Type': 'application/pdf'})
                response = urllib2.urlopen(request, timeout=60)
            #response = open('/Users/dave/Desktop/ananiadou_tibtech06.pdf-response.xml', 'r')

            # Create Metadata link annotation
            link = document.newAccList('metadata', 50)
            link['property:sourceDatabase'] = 'lazarus'
            link['property:sourceTitle'] = 'Lazarus'
            link['property:sourceDescription'] = self.sourceDescription
            link['property:sourceIcon'] = utopia.get_plugin_data_as_url('images/lazarus-prefs-logo.png', 'image/png')

            headers = []
            pos = []
            refs = []
            annotations = []
            concepts = {}
            hits = []
            expression_annotations = []
            for kAnnotation in kend.converter.XML.parse(response, kend.model.Document):
                #print kend.converter.XML.serialise(kAnnotation)[0]
                try:
                    annotations.append(utopia.tools.converters.Annotation.kend2spineapi(kAnnotation, document))
                except:
                    pass
            annotations.sort(key=lambda a: int(a.get('structure:order', 0)))
            for sAnnotation in annotations:
                if sAnnotation['concept'] == 'structure_element':
                    role, level = self.getHeaderRole(sAnnotation)
                    if role is not None and needs_outline:
                        while len(pos) < level:
                            pos.append(0)
                        while len(pos) > level:
                            pos.pop()
                        pos[-1] += 1

                        outline = u'.'.join([unicode(i) for i in pos])
                        anchor_name = '#lazarus.outline.{0}'.format(outline)

                        anchor = spineapi.Annotation()
                        anchor['concept'] = 'Anchor'
                        anchor['property:anchor'] = anchor_name
                        anchor.addExtents(sAnnotation.extents())
                        anchor.addAreas(sAnnotation.areas())
                        document.addAnnotation(anchor)

                        header = spineapi.Annotation()
                        header['concept'] = 'OutlineItem'
                        header['property:outlinePosition'] = outline
                        header['property:outlineTitle'] = u' '.join([e.text() for e in sAnnotation.extents()])
                        header['property:destinationAnchorName'] = anchor_name
                        document.addAnnotation(header)

                        print((u'    '*level + u'.'.join([unicode(i) for i in pos]) + u' ' + u' '.join([e.text() for e in sAnnotation.extents()])).encode('utf8'))
                    elif 'bibitem' in sAnnotation.getAllProperties('structure:role'):
                        #refs.append(sAnnotation)
                        pass
                elif sAnnotation['concept'] == 'Citation':
                    # Hack to fix a mistake in authors property name
                    if 'property:author' in sAnnotation and not 'property:authors' in sAnnotation:
                        sAnnotation['property:authors'] = sAnnotation.getAllProperties('property:author')
                    refs.append(sAnnotation)
                elif sAnnotation['concept'] == 'LazarusConcept':
                    concept_id = sAnnotation.get('property:identifier')
                    if concept_id is not None:
                        sAnnotation['id'] = str(uuid.uuid4())
                        concepts[concept_id] = sAnnotation
                        document.addAnnotation(sAnnotation, 'Lazarus Concept')
                elif sAnnotation['concept'] == 'LazarusConceptHit':
                    hits.append(sAnnotation)
                elif sAnnotation['concept'] == 'LazarusSentenceExpression':
                    expression_annotations.append(sAnnotation)
                else:
                    document.addAnnotation(sAnnotation)

            for ref in refs:
                #print(ref.get('structure:order', '0'))
                pass
            refs = sorted(refs, key=lambda ref: int(ref.get('property:order', '0')))

            for ref in refs:
                #print(ref.get('structure:order', '0'))
                pass
            for ref in refs:
                # Create Bibliography annotations
                #citation = {'unstructured': u' '.join([e.text() for e in ref.extents()])}
                #annotation = utopia.tools.utils.citation_to_annotation(citation)
                #annotation['property:order'] = ref.get('structure:order')
                #annotation.addExtents(ref.extents())
                #annotation.addAreas(ref.areas())
                #document.addAnnotation(annotation, link['scratch'])
                document.addAnnotation(ref, link['scratch'])

            # Now link hits to concepts
            for i, hit in enumerate(hits):
                concept_id = hit.get('property:identifier')
                concept = concepts.get(concept_id)
                if concept is not None:
                    concept_uuid = concept.get('id')
                    hit['property:concept_id'] = concept_uuid

                    identifier = concept.get('property:identifier')
                    name = concept.get('property:name', '???')
                    sources = concept.get('property:externalSources', 'json:[]')
                    if sources.startswith('json:'):
                        sources = json.loads(sources[5:])
                    if 'property:stdInchiKey' in concept:
                        sources.append({'database': ' InchiKey', 'identifier': concept['property:stdInchiKey']})
                    if 'property:canonicalSmiles' in concept:
                        sources.append({'database': ' SMILES', 'identifier': concept['property:canonicalSmiles']})
                    kind = concept.get('property:kind')
                    kind = self.dbs.get(kind, {}).get('title', kind)
                    links = {}
                    for source in sources:
                        uri = source.get('uri')
                        if 'primary' in source.get('relationship', []):
                            links.setdefault('definition', [])
                            links['definition'].append(u'''
                                <a href="{uri}" title="{uri}">{database}</a>
                            '''.format(**source))
                        elif uri is None:
                            if source.get('database') in (' InchiKey', ' SMILES'):
                                links.setdefault('main', [])
                                links['main'].append(u'''
                                    <tr><td>{database}:</td><td>{identifier}</td></tr>
                                '''.format(**source))
                        else:
                            identifier = source.get('identifier')
                            links_category = 'xref'
                            if 'seeAlso' in source.get('relationship', []) or uri is None:
                                links_category = 'seeAlso'
                            links.setdefault(links_category, [])
                            if identifier is not None:
                                links[links_category].append(u'''
                                    <a href="{uri}" title="{uri}">{name}...</a> ({identifier})
                                '''.format(**source))
                            else:
                                links[links_category].append(u'''
                                    <a href="{uri}" title="{uri}">{name}...</a>
                                '''.format(**source))

                    style = u'''
                        <style>
                          .lazarus-table tbody {
                            border: none;
                          }
                          .lazarus-table td:first-of-type {
                            text-align: right;
                            font-weight: bold;
                          }
                          .lazarus-table td {
                            vertical-align: top;
                          }
                          .lazarus-table td:first-of-type {
                            white-space: nowrap;
                          }
                          .lazarus-table td:not(:first-of-type) {
                            word-break: break-all;
                          }
                          .lazarus-table tr td {
                            padding-top: 0ex;
                            padding-bottom: 0ex;
                          }
                          .lazarus-table tbody:not(:first-of-type) tr:first-of-type td {
                            padding-top: 1ex;
                          }
                        </style>
                    '''
                    html = u'''
                        <table class="lazarus-table">
                          <tr><td>Name:</td><td>{name}</td></tr>
                    '''.format(**{'name':name})
                    categories = {'xref': 'Related:', 'seeAlso': 'See also:', 'definition': 'Defined in:'}
                    for links_category in ('main', 'xref', 'seeAlso', 'definition'):
                        links_title = categories.get(links_category)
                        these_links = sorted(list(set(links.get(links_category, []))))
                        if len(these_links) > 0:
                            html += '<tbody>'
                            if links_category != 'main':
                                html += u'<tr><td>{0}</td><td>'.format(links_title)
                                html += u'<br>'.join(these_links)
                                html += '</td></tr>'
                            else:
                                html += ''.join(these_links)
                            html += '</tbody>'
                    #pprint('------------------------')
                    html += u'''
                        </table>
                    '''
                    #print(html)

                    hasLinks = len(links.get('xref', []) + links.get('seeAlso', [])) > 0

                    ann = spineapi.Annotation()
                    ann['concept'] = 'Collated'
                    ann['property:name'] = u'{0}'.format(name)
                    ann['property:description'] = 'Lazarus Concept'
                    ann['session:semanticTerm'] = name
                    ann['property:html'] = [style, html]
                    ann['property:sourceDescription'] = self.sourceDescription
                    ann['property:sourceIcon'] = utopia.get_plugin_data_as_url('images/lazarus-prefs-logo.png', 'image/png')
                    ann['session:overlay'] = 'hyperlink'
                    ann['session:color'] = '#880000'
                    count = 0
                    print('====', 7)
                    if 'property:hitFragments' in hit:
                        hitFragments = hit.getAllProperties('property:hitFragments') or []
                        #pprint(hitFragments)
                        for hitFragment in hitFragments:
                            pre, _, rest = hitFragment.partition('{!')
                            match, _, post = rest.partition('!}')
                            #pprint((pre, match, post))
                            matches = document.findInContext(pre, match, post, fuzzy=True)
                            count += len(matches)
                            ann.addExtents(matches)
                    if hasLinks and count > 0:
                        document.addAnnotation(ann)

            style = u'''
                <style>
                    .lazarus-expression .box {
                        background-color: #FFF0E8;
                        border-color: #EEE0D8;
                    }
                    .lazarus-related {
                        padding-left: 42px;
                        background-image: url(%s);
                        background-repeat: no-repeat;
                        background-position: top left;
                        background-size: 37px 48px;
                        min-height: 53px;
                    }
                    .lazarus-related + .lazarus-related {
                        margin-top: 5px;
                        border-top: 1px dotted #aaa;
                        padding-top: 5px;
                        background-position-y: 5px;
                        min-height: 58px;
                    }
                    .lazarus-sentence {
                        padding-left: 0.5em;
                        color: black;
                    }
                    .lazarus-sentence.negative {
                        border-left: solid 5px #bb0000;
                    }
                    .lazarus-sentence.positive {
                        border-left: solid 5px #008800;
                    }
                    .lazarus-sentence.negative a {
                        color: #bb0000;
                    }
                    .lazarus-sentence.positive a {
                        color: #008800;
                    }
                </style>
            ''' % utopia.get_plugin_data_as_url('images/pdf-page-icon.png', 'image/png')

            expressions = []
            for sAnnotation in expression_annotations:
                exp = sAnnotation.get('property:expressions', 'json:{}')
                if exp.startswith('json:'):
                    exp = json.loads(exp[5:])
                context = sAnnotation.get('property:context')
                if context is not None:
                    if exp.get('negative', False):
                        exp['posneg'] = 'negative'
                    else:
                        exp['posneg'] = 'positive'

                    pprint(context)
                    pprint(exp)

                    matched_context = exp.get('context')
                    matches = []
                    if matched_context is not None:
                        matches = document.search(re.sub(r'\s+', ' ', matched_context))
                        if len(matches) > 0:
                            anchor_id = str(uuid.uuid4())[1:-1]
                            anchor = spineapi.Annotation()
                            anchor['concept'] = 'Anchor'
                            anchor['property:anchor'] = anchor_id
                            anchor.addExtents(matches);
                            document.addAnnotation(anchor)

                            exp.update({'anchor_id': anchor_id, 'sentence': context})
                            expressions.append(exp)

            js = u'''
                <script>
                    $(document).on('DOMNodeInserted', function(e) {
                        var element = e.target;
                        $(element).filter('a[target="tab"]').add('a[target="tab"]', element).each(function () {
                            var fragment = $(this).closest('.-papyro-internal-citation').data('citation')['userdef']['first_fragment'];
                            $(this).attr('target', 'pdf; show=highlight; text=[' + encodeURIComponent(fragment) + ']');
                        });
                    });

                    $(function () {
                        var lazarus = {
                            expressions: %s,
                            fingerprints: %s,
                            relUrl: %s
                        };

                        var more_expressions_link = $('#lazarus-expression > p.more').hide();
                        var more_expressions_spinner = $('#lazarus-expression > div.spinner');

                        Spinners.create(more_expressions_spinner);
                        Spinners.play(more_expressions_spinner);

                        var exp_divs = [];
                        var identifiers = [];
                        for (var e = 0; e < lazarus.expressions.length; e++) {
                            var expression = lazarus.expressions[e];
                            var exp_div = $('<div class="box"></div>');
                            exp_div.data('expression', expression);
                            exp_div.hide();
                            exp_divs.push(exp_div);
                            identifiers.push(expression.identifiers);
                        }
                        var params = {
                            fingerprint: lazarus.fingerprints
                        };
                        var url = lazarus.relUrl + '?' + $.param(params, traditional=true);
                        $.ajax({
                            url: url,
                            type: 'POST',
                            dataType: 'json',
                            data: JSON.stringify(identifiers),
                            contentType: "application/json",
                            error: function (xhr, ajaxOptions, thrownError) {
                                console.log(xhr.statusText);
                                console.log(xhr.responseText);
                                console.log(xhr.status);
                                console.log(thrownError);

                                // FIXME do something here
                                Spinners.remove(more_expressions_spinner);
                            },
                            success: function (related) {
                                // Sort related according to the number of articles found
                                related.results.sort(function (l, r) {
                                    var lv = Object.keys(l.related).length;
                                    var rv = Object.keys(r.related).length;
                                    return (lv > rv) ? -1 : (lv < rv) ? 1 : 0;
                                });
                                $.each(related.results, function (idx, result) {
                                    var exp_div = exp_divs[idx];
                                    var expression = exp_div.data('expression');
                                    expression.related = result.related;
                                    delete expression.related[%s];

                                    split = expression.sentence.split(expression.context);
                                    pre = split[0];
                                    pre = pre.replace(/(\w)$/, '$1 ');
                                    pre = pre.replace(/^\s*/, '');
                                    match = expression.context;
                                    post = split[1];
                                    post = post.replace(/^(\w)/, ' $1');
                                    post = post.replace(/\s*$/, '');
                                    expression.pre = pre;
                                    expression.match = match;
                                    expression.post = post;

                                    // Create expression element
                                    exp_div.append('<p class="lazarus-sentence ' + expression.posneg + '">&ldquo;' + expression.pre + '<a target="pdf; show=select; anchor=' + expression.anchor_id + '"><strong>' + expression.match + '</strong></a>' + expression.post + '&rdquo;</p>');
                                    exp_div.data('expression', expression);

                                    $('#lazarus-expression > .content').append(exp_div);

                                    if (Object.keys(expression.related).length > 0) {
                                        var related_div = $('<div class="expandable" title="Related expressions elsewhere"></div>');
                                        var related_div_content = $('<div></div>').appendTo(related_div);
                                        function on_expand() {
                                            related_div.off('papyro:expandable:expand', on_expand);
                                            $.each(expression.related, function (idx, obj) {
                                                fragments = [];
                                                $.each(obj, function (id, obj) {
                                                    fragments.push(obj.context);
                                                });
                                                fragments.join('\\n');
                                                related_div_content.append($('<div class="lazarus-related unprocessed"></div>').append('<p><strong>&ldquo;&hellip;'+fragments+'&hellip;&rdquo;</strong></p>').hide().data('citation', {identifiers:{doi:idx},userdef:{first_fragment:fragments[0]}}));
                                                // .append(utopia.citation.render({identifiers:{doi:idx},first_fragment:fragments[0]}, true, true))
                                            });
                                            expression.related.length = 0; // empty for future

                                            if ($('.lazarus-related.unprocessed', exp_div).length > 0) {
                                                var more = $('<p class="more right"><a class="more">More related articles...</a></p>');
                                                related_div_content.append(more);
                                                function show_five_related(e) {
                                                    e.preventDefault();

                                                    $('.lazarus-related.unprocessed', exp_div).slice(0, 5).each(function (idx, obj) {
                                                        var citation = $(obj).data('citation');
                                                        $(obj).append(utopia.citation.render(citation, true, true));
                                                        $(obj).show().removeClass('unprocessed');
                                                    });
                                                    if ($('.lazarus-related.unprocessed', exp_div).length == 0) {
                                                        more.remove();
                                                    }
                                                }
                                                more.on('click', show_five_related).click();
                                            }
                                        }
                                        related_div.on('papyro:expandable:expand', on_expand);
                                        exp_div.append(related_div);
                                        utopia.processNewContent(related_div);
                                    }
                                });

                                Spinners.remove(more_expressions_spinner);
                                more_expressions_link.show();
                                $('a.more', more_expressions_link).click();
                            }
                        });

                        function append_five(e) {
                            e.preventDefault();

                            // Show the next five
                            $('#lazarus-expression > .content').children().filter(':hidden').slice(0,5).show();

                            // Hide the 'more' link if everything is now visible
                            if ($('#lazarus-expression > .content').children().filter(':hidden').length == 0) {
                                more_expressions_link.hide();
                            }
                        }

                        // Hook up 'more' link
                        $('#lazarus-expression > p.more > a.more').on('click', append_five).click();
                    });
                </script>
            ''' % (json.dumps(expressions), json.dumps(document.fingerprints()), json.dumps(laz_docRelUrl), json.dumps(this_doi))
            #print(js.encode('utf8'))

            html = u'''
                <div id="lazarus-expression"><div class="content"></div><div class="spinner"></div><p class="more"><a class="more">More expressions...</a></p></div>
            '''

            if len(expressions) > 0:
                ann = spineapi.Annotation()
                ann['concept'] = 'Collated'
                ann['property:name'] = 'Lazarus Expressions'
                ann['property:description'] = u'Summarizing expression(s)'
                ann['property:html'] = [js, style, html]
                ann['property:sourceDescription'] = self.sourceDescription
                ann['property:sourceIcon'] = utopia.get_plugin_data_as_url('images/lazarus-prefs-logo.png', 'image/png')
                document.addAnnotation(ann)

        else: # no permission
            noprompt = self.get_config('noprompt', False)
            if not noprompt:
                annotation = spineapi.Annotation()
                annotation['concept'] = 'Collated'
                params = {
                    'uuid': self.uuid(),
                }
                annotation['property:html'] = utopia.get_plugin_data('tpl/denied.html').format(**params)
                annotation['property:name'] = 'Lazarus'
                annotation['property:description'] = 'Lazarus functionality is turned off'
                annotation['property:sourceDescription'] = self.sourceDescription
                annotation['property:sourceIcon'] = utopia.get_plugin_data_as_url('images/lazarus-prefs-logo.png', 'image/png')
                annotation['session:default'] = '1'
                document.addAnnotation(annotation)

    def busId(self):
        # Name of this plugin on the message bus.
        return 'lazarus'

    def event(self, sender, data):
        # Act upon incoming messages.
        if data == 'hide':
            self.set_config('noprompt', True)

    def uuid(self):
        # Configuration ID
        return laz_plugin_uuid


class LazarusConf(utopia.Configurator):

    def defaults(self):
        return {
            'permission': False,
            'noprompt': False,
        }

    def form(self):
        # Actual configuration form
        return utopia.get_plugin_data('tpl/config.html')

    def icon(self):
        # Data URI of configuration logo
        return utopia.get_plugin_data_as_url('images/lazarus-prefs-logo.png', 'image/png')

    def title(self):
        # Name of plugin in configuration panel
        return 'Lazarus'

    def uuid(self):
        # Configuration ID
        return laz_plugin_uuid


class LazarusLoggingRemoteQuery(utopia.library.RemoteQuery):
    """Lazarus remote query logging"""

    def fetch(self, query, offset, limit):
        # Send the query off to the server
        # FIXME

        event = {
            'action': 'remotesearch',
            'anonymous-user-id': utopia.bridge.context['anonymous-user-id'],
            'time': datetime.datetime.now().replace(microsecond=0).isoformat(),
            'context': query
        }

        send_crowdsource_event(event, document)

        # Return no results
        return (0, 0, 0, [])

if __name__ == '__main__':
    import crackleapi
    document = crackleapi.loadPDF('/Users/dave/Documents/mycobacterium.pdf')
    annotator = LazarusAnnotator()
    annotator.on_load_event(document)
