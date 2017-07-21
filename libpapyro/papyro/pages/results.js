/*****************************************************************************
 *  
 *   This file is part of the Utopia Documents application.
 *       Copyright (c) 2008-2017 Lost Island Labs
 *           <info@utopiadocs.com>
 *   
 *   Utopia Documents is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU GENERAL PUBLIC LICENSE VERSION 3 as
 *   published by the Free Software Foundation.
 *   
 *   Utopia Documents is distributed in the hope that it will be useful, but
 *   WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 *   Public License for more details.
 *   
 *   In addition, as a special exception, the copyright holders give
 *   permission to link the code of portions of this program with the OpenSSL
 *   library under certain conditions as described in each individual source
 *   file, and distribute linked combinations including the two.
 *   
 *   You must obey the GNU General Public License in all respects for all of
 *   the code used other than OpenSSL. If you modify file(s) with this
 *   exception, you may extend this exception to your version of the file(s),
 *   but you are not obligated to do so. If you do not wish to do so, delete
 *   this exception statement from your version.
 *   
 *   You should have received a copy of the GNU General Public License
 *   along with Utopia Documents. If not, see <http://www.gnu.org/licenses/>
 *  
 *****************************************************************************/

(function ($) {
  $.fn.oldReady = $.fn.ready;
  $.fn.ready = function (fn) {
    return $.fn.oldReady(function () {
      try {
        if (fn) fn.apply($,arguments);
      } catch (e) {
        console.log(e);
      }
    });
  }
})(jQuery);

var utopia = {

    searchRemote:
        window.control.searchRemote,

    citation:
        {
            ///////////////////////////////////////////////////////////////////
            /// Internal methods

            _resolveMetadata: function (metadata, purpose, fn) {
                // Begin resolution
                var future = window.control.resolveMetadata(metadata, purpose);
                // Lock the future and test for completion
                future.lock();
                if (future.ready) {
                    // If the results are already ready, simply pass them on
                    // and delete the future
                    fn(future.results);
                    future.doom();
                } else {
                    future.completed.connect(function (results) {
                        fn(results);
                        future.doom();
                    });
                }
                future.unlock();
            },
            _imbue: function () {
                // Make sure it's not already imbued
                var container = $(this);
                if (container.data('status') != 'live') {
                    // Get hold of data
                    var citation = container.data('citation');
                    var process = container.data('process');
                    var callback = container.data('callback');
                    var include_links = container.data('links');

                    // Sort out data
                    container.removeAttr('data-citation');
                    container.removeAttr('data-process');
                    container.removeData('process');
                    container.data('status', 'live');

                    // Generate internal structure
                    var content = $('<span class="content"></span>').append(container.contents()).appendTo(container);
                    var links = $('<span></span>').addClass('links').appendTo(container);

                    function renderCitation(citation) {
                        // Save the citation data for later use
                        var initial_citation = container.data('citation');
                        container.data('initial_citation', initial_citation);
                        citation.label = initial_citation.label;
                        citation.order = initial_citation.order;
                        console.log(citation.unstructured, citation.label);
                        if (citation.unstructured && citation.label) {
                            citation.unstructured = citation.unstructured.replace(new RegExp('^[^a-z0-9]*'+citation.label+'[^a-z0-9]*', 'i'), '');
                        }
                        container.data('citation', citation);
                        // Format and add content
                        utopia.citation.reformat(container);
                        // Include links
                        if (include_links) {
                            links.append(utopia.citation._renderLinks(citation));
                        }
                        // Execute callback if present
                        if (callback) {
                            callback(container);
                        }
                    }

                    // Optionally process the citation before rendering it
                    if (process) {
                        function final_step (citation) {
                            $('.auto-spinner', container).remove();
                            renderCitation(citation);
                        }
                        var spinner = $('<span><span>').css({'display':'inline-block', 'vertical-align':'middle'}).addClass('auto-spinner').appendTo(container);
                        //Spinners.create(container);
                        //Spinners.play(container);
                        utopia.citation.identify(citation, function (citation) {
                            utopia.citation.expand(citation, function (citation) {
                                if (include_links) {
                                    utopia.citation.dereference(citation, final_step);
                                } else {
                                    final_step(citation);
                                }
                            });
                        });
                    } else {
                        if (include_links) {
                            var links_spinner = $('<span><span>').css({'display':'inline-block', 'vertical-align':'middle'}).addClass('auto-spinner').appendTo(links).before(' [').after(']');
                            utopia.citation.identify(citation, function (citation) {
                                utopia.citation.dereference(citation, function (citation) {
                                    Spinners.remove(links_spinner);
                                    links.empty();
                                    renderCitation(citation);
                                });
                            });
                        } else {
                            renderCitation(citation);
                        }
                    }
                }
            },
            _orderLinks: function (citation) {
                if (citation.links) {
                    citation.links.sort(function (c1, c2) {
                        // First sort on mime type to make PDF first
                        var pdf1 = (c1.mime == 'application/pdf');
                        var pdf2 = (c2.mime == 'application/pdf');
                        if (pdf1 && !pdf2) {
                            return -1;
                        } else if (!pdf1 && pdf2) {
                            return 1;
                        } else {
                            // Next sort articles ahead of abstracts, ahead of searches
                            var order = ['search', 'abstract', 'article']
                            var t1 = order.indexOf(c1.type);
                            var t2 = order.indexOf(c2.type);
                            if (t1 != t2) {
                                return t2 - t1;
                            } else {
                                // Finally, sort on weights making higher weights first
                                var w1 = c1[':weight'];
                                var w2 = c2[':weight'];
                                return w2 - w1;
                            }
                        }
                    });
                }
            },
            _renderLink: function (link) {
                var isPDF = (link.mime == 'application/pdf');
                var isSearch = (link.type == 'search');
                return $('<a></a>').attr({
                    href: link.url,
                    title: link.title,
                    target: (isPDF ? 'pdf' : 'www')
                }).text(isPDF ? 'PDF' : isSearch ? 'Find...' : 'Link').addClass('citation');
            },
            _renderLinks: function (citation) {
                var hasPdf = citation.links.some(function (link) {
                    return (link.mime == 'application/pdf');
                });
                var hasArticleOrAbstract = citation.links.some(function (link) {
                    return (link.type == 'article' || link.type == 'abstract');
                });
                var title = 'Open article...';
                var text = 'PDF';
                var target = 'tab';
                if (!hasPdf) {
                    if (hasArticleOrAbstract) {
                        text = 'Link';
                    } else {
                        text = 'Find';
                        title = 'Find article...';
                    }
                    target = 'www';
                }
                var a = $('<a></a>').attr({
                    href: '#citation',
                    title: title,
                    target: target
                }).text(text);
                // Surround each link with brackets
                a = a.wrap($('<span>[<span></span>]</span>'));
                return a;
            },
            _reformatAll: function () {
                // When the default citation format has changed, update all visible citations
                $('.-papyro-internal-citation').each(function () {
                    var container = $(this);
                    if (container.data('status') == 'live') {
                        this.reformat(container);
                    }
                });
            },
            reformat: function (container) {
                var citation = container.data('citation');
                var content = container.find('.content');
                var oldHeight = container.height();
                console.log(container);
                var html = this.format(citation);
                content.html(html);
                var newHeight = container.height();
                container.height(oldHeight);
                container.animate({height: newHeight}, 'fast', function() {
                    container.height('auto');
                });
            },

            ///////////////////////////////////////////////////////////////////
            /// Public methods available to plugin developers

            styles: window.control.availableCitationStyles,
            defaultStyle: window.control.defaultCitationStyle,
            format: window.control.formatCitation,
            identify: function (metadata, fn) {
                utopia.citation._resolveMetadata(metadata, 'identify', fn);
            },
            expand: function (metadata, fn) {
                utopia.citation._resolveMetadata(metadata, 'expand', fn);
            },
            dereference: function (metadata, fn) {
                utopia.citation._resolveMetadata(metadata, 'dereference', fn);
            },
            render: function (citation, process, links, fn) {
                var container = $('<div class="-papyro-internal-citation"></div>');
                container.data('citation', citation);
                container.data('process', (process === true));
                container.data('links', (links === true));
                container.data('callback', fn);
                return container.each(utopia.citation._imbue);
            }
        },

    elements:
        {},

    templates:
        {},

    onLoad:
        // Initially prepare the HTML document
        function () {
            // Set up AJAZ global defaults
            $.ajaxSetup({
                // A 10 second timeout on AJAX queries
                timeout: 10000
            });

            // Get element references and template nodes
            utopia.elements['sections'] = $('#-papyro-internal-sections').first();
            utopia.elements['active'] = $('#-papyro-internal-active').first();
            utopia.templates['result'] = $('#-papyro-internal-result_template').first().detach();

            // Hijack link actions on an element
            $('body').on('click', 'a', function (e) {
                var href = this.getAttribute('href');
                if (href == undefined) {
                    href = this.getAttributeNS('http://www.w3.org/1999/xlink', 'href');
                }
                var target = this.getAttribute('target');
                if (target == undefined) {
                    target = this.getAttributeNS('http://www.w3.org/1999/xlink', 'show');
                }
                if (href == '#citation') {
                    var citation = $(this).closest('.-papyro-internal-citation').data('citation')
                    window.control.activateCitation(citation, target);
                } else {
                    window.control.activateLink(href, target);
                }
                e.preventDefault();
            });

            // Remove failed images by default
            $('body').on('error', 'img', function () { $(this).remove(); });

            // Set up connections
            window.control.resultAdded.connect(utopia.onResultItemAdded);

            // Look out for new spinners
            $(document).on('DOMNodeInserted', function(e) {
                var element = e.target;
                $(element).filter('.auto-spinner').add('.auto-spinner', element).each(function () {
                    var params = {
                        radius: $(this).data('radius') || 3,
                        height: $(this).data('height') || 2,
                        padding: $(this).data('padding') || 0,
                        dashes: $(this).data('dashes') || 7,
                    };
                    Spinners.create($(this), params);
                    Spinners.play($(this));
                });
            });

            // Inform the control that we're done setting everything up
            window.control.onLoadComplete();
        },

    registerPolisher:
        // Assign fn as a handler for papyro:polish
        function (fn) {
            $('#-papyro-internal-papyro').on('papyro:polish', '.-papyro-internal-content', fn);
        },

    clear:
        // Clear all results from list FIXME
        function () {
            $('.-papyro-internal-active').empty();
            $('.-papyro-internal-section').empty();
        },

    setExploreTerm:
        function (term, explore) {
            // What's the section's term?
            var section_term = $.trim(term.toLowerCase());

            // Remove duplicate section(s) if present
            var section = $('.-papyro-internal-section').filter(function (i, div) {
                if ($(div).data('term') == section_term) {
                    if (i == 0) {
                        return true; // The first occurrance that matches stays
                    } else {
                        $(div).remove(); // Duplicates need to be removed
                    }
                }
                return false;
            }).empty();

            // If there are none at all, we need to create a section
            if (section.length == 0) {
                section = $('<div class="-papyro-internal-section"></div>');
                section.data({'term': section_term});
                $('#-papyro-internal-sections').append(section);
            }

            section.append(
                $('<div class="-papyro-internal-legend"></div>').text(section_term)
            ).append(
                $('<div class="-papyro-internal-active"></div>')
            ).append(
                $('<div class="-papyro-internal-sub-legend"></div>').text('')
            ).append(
                $('<div class="-papyro-internal-explored"></div>')
            );

            if (explore) {
                $(function () {
                    window.control.explore(term);
                });
            }

            return section;
        },

    setExploreTerms:
        // Remove all explore sections and set up new ones
        function (terms, explore) {
            $.each(terms, function (i, term) {
                utopia.setExploreTerm(term, explore);
            });
        },

    result:
        // Get access to the control object for a given result
        function (obj, c) {
            if (c) {
                return $(obj).closest('.-papyro-internal-result').data('control', c);
            } else {
                return $(obj).closest('.-papyro-internal-result').data('control');
            }
        },

    toggleSlide:
        // Function to toggle an object's content
        function (obj) {
            var resultelem = $(obj).closest('.-papyro-internal-result');
            resultelem.find('.-papyro-internal-body').each(function () {
                if ($(this).is(':hidden')) {
                    utopia.onResultItemOpened(obj);
                    $(this).css({opacity: 0}).slideDown(100).animate({opacity: 1}, 100);
                    resultelem.addClass('expanded');
                    resultelem.removeClass('collapsed');
                } else {
                    $(this).animate({opacity: 0}, 100).slideUp(100);
                    resultelem.addClass('collapsed');
                    resultelem.removeClass('expanded');
                    utopia.onResultItemClosed(obj);
                }
            });
        },

    onResultItemAdded:
        // Prepare a newly added result element
        function (obj) {
            // Add a new result element
            result = obj.element = utopia.templates['result'].clone().get(0);
            utopia.result(result, obj);

            // Generic sorting info
            var section_term = '';
            var is_explored = (obj.value('session:origin') == 'explore');
            var semantic_term = obj.value('session:semanticTerm');

            // Into which section should this be inserted?
            var section;
            if (is_explored) {
                section_term = $.trim(obj.value('session:exploredTerm').toLowerCase());
            } else if (semantic_term) {
                section_term = semantic_term.toLowerCase();
            } else if (obj.context.term) {
                section_term = obj.context.term.toLowerCase();
            }
            section = $('.-papyro-internal-section').filter(function (i, div) {
                return $(div).data('term') == section_term;
            }).first();
            if (section.length == 0) {
                section = utopia.setExploreTerm(section_term, !is_explored);
            }

            // Into which container within the section should this be inserted?
            var container = $('.-papyro-internal-'+(is_explored ? 'explored' : 'active'), section);
            subsequent = $('.-papyro-internal-result', container).filter(function (idx) {
                var candidate = utopia.result(this);
                return (obj.headless || !candidate.headless) && (candidate.weight < obj.weight);
            });
            if (subsequent.length > 0) {
                subsequent.first().before(result);
            } else {
                container.append(result);
            }

            // Connect content handler
            obj.insertContent.connect(utopia.onResultItemContentAdded)

            // Hide content by default
            $('.-papyro-internal-body', result).slideUp(0);

            if (!obj.headless) {
                // Remove unreachable graphicss
                $('.-papyro-internal-header .-papyro-internal-graphics img', result).on('error', function () { $(this).remove(); });
                $('.-papyro-internal-header .-papyro-internal-graphics img.-papyro-internal-source', result).on('click', function (event) {
                    // Stop event from reaching the parent
                    event.stopPropagation();
                    // Send the result object off to be activated
                    window.control.activateSource(utopia.result(this));
                });
                $('.-papyro-internal-header .-papyro-internal-graphics img.-papyro-internal-author', result).on('click', function (event) {
                    // Stop event from reaching the parent
                    event.stopPropagation();
                    // Send the result object off to be activated
                    window.control.activateAuthor(utopia.result(this));
                });

                // Set visible information
                var title = $("<div></div>").html(obj.title).text();
                var description = $("<div></div>").html(obj.description).text();
                $('.-papyro-internal-header .-papyro-internal-title', result).text(title);
                if (obj.description) {
                    $('.-papyro-internal-header .-papyro-internal-description', result).text(description);
                //} else {
                //    $('.header .description', result).text('-');
                }
                if (obj.sourceIcon) {
                    $('.-papyro-internal-header .-papyro-internal-graphics img.-papyro-internal-source', result).attr('src', obj.sourceIcon);
                } else if (obj.sourceDatabase) {
                    $('.-papyro-internal-header .-papyro-internal-graphics img.-papyro-internal-source', result).attr('src', 'http://utopia.cs.manchester.ac.uk/images/' + obj.sourceDatabase + '.png');
                } else {
                    $('.-papyro-internal-header .-papyro-internal-graphics img.-papyro-internal-source', result).remove();
                }
                if (obj.authorUri) {
                    $('.-papyro-internal-header .-papyro-internal-graphics img.-papyro-internal-author', result).attr('src', obj.authorUri + '/avatar');
                } else {
                    $('.-papyro-internal-header .-papyro-internal-graphics img.-papyro-internal-author', result).remove();
                }
            }

            // Set up interaction events
            $('.-papyro-internal-header', result).click(function () { utopia.result(this).toggleContent(); });

            // Mouse over colours
            $(result).bind('mouseenter', function () { $(this).addClass('selected'); });
            $(result).bind('mouseleave', function () { $(this).removeClass('selected'); });

            // Animate the result's appearance
            $(result).css({opacity: 0}).slideDown(100).animate({opacity: 1}, 100);

            // Create and start a spinner
            $('.-papyro-internal-body .-papyro-internal-loading', result).each(function () {
                Spinners.create(this);
            });

            // Open if requested
            if (obj.openByDefault) {
                $('.-papyro-internal-header', result).click();
            }

            // HACK to set the highlight colour
            if (obj.highlight) {
                //alert(obj.highlight);
                //$('.-papyro-internal-header', result).css({'borderLeft': 'solid 4px ' + obj.highlight, 'paddingLeft': '6px'});
            }

            if (obj.headless) {
                $(result).addClass('headless');
                $('.-papyro-internal-header', result).remove();
            }
        },

    processNewContent:
        // Process new content
        function (obj) {
            // Wrap in jQuery
            obj = $(obj);

            // Make sure we don't get error images anywhere
            obj.find('img').hide().on('load', function () {
                $(this).css({opacity: 0}).slideDown(100).animate({opacity: 1}, 100);
            });

            // Modify the DOM to include expandy/contracty DIVs
            obj.find('.expandable[title]').add(obj.filter('.expandable[title]')).addClass('collapsed').wrapInner('<div class="expansion"></div>').each(function () {
                var expandable = $(this);
                var caption = $('<div class="caption"></div>');

                caption.append(expandable.attr('title'));
                caption.on('click', function () {
                    expandable.trigger('papyro:expandable:toggle');
                });

                expandable.on('papyro:expandable:toggle', function () {
                    if (expandable.hasClass('collapsed')) {
                        expandable.trigger('papyro:expandable:expand');
                    } else {
                        expandable.trigger('papyro:expandable:collapse');
                    }
                }).on('papyro:expandable:expand', function () {
                    if (expandable.hasClass('collapsed')) {
                        expandable.removeClass('collapsed').addClass('expanded');
                        expandable.find('> .expansion').css({opacity: 0}).slideDown(100).animate({opacity: 1}, 100);
                    }
                }).on('papyro:expandable:collapse', function () {
                    if (expandable.hasClass('expanded')) {
                        expandable.removeClass('expanded').addClass('collapsed');
                        expandable.find('> .expansion').animate({opacity: 0}, 100).slideUp(100);
                    }
                });
                expandable.removeAttr('title');
                expandable.prepend(caption);
            }).children('.expansion').hide();

            // Modify the DOM to include More... links
            obj.find('.readmore').add(obj.filter('.readmore')).wrapInner('<span class="expansion"></span>').each(function () {
                var expandable = $(this);
                var readmore = $('<span>&hellip; <a class="morelink" title="Show more&hellip;" href="#">[more]</a></span>');
                var readless = $('<span> <a class="lesslink" title="Show less." href="#">[less]</a></span>').hide();
                readmore.on('click', function () {
                    e.preventDefault();
                    $(this).next('.expansion').show();
                    $(this).hide();
                    readless.show();
                });
                readless.on('click', function (e) {
                    e.preventDefault();
                    $(this).prev('.expansion').hide();
                    readmore.show();
                    $(this).hide();
                });
                expandable.prepend(readmore);
                expandable.append(readless);
            }).children('.expansion').hide();

            obj.find('.limited-height').add(obj.filter('.limited-height')).wrapInner('<div class="expansion"></span>').each(function () {
                var expandable = $(this);
                var more = $('<a class="overlay" href="#">&#x25BE; More</a>').on('click', function (e) {
                    e.preventDefault();
                    $(this).hide();
                    expandable.css({'max-height': 'inherit'});
                });
                expandable.append(more);
            });

            // Make sure citations are dealt with properly
            obj.find('.-papyro-internal-citation[data-citation]').each(utopia.citation._imbue);

            // Hyphenate appropriate elements
            //Hyphenator.config({
            //    selectorfunction: function() {
            //        return obj.find('p, li, .hyphenate').add(obj.filter('p, li, .hyphenate')).not('.nohyphenate').get();
            //    }
            //});
            // FIXME soft hyphens went bonkers in the latest Qt 4.8 code, so turn them off.
            //Hyphenator.run();
        },

    onResultItemContentAdded:
        // Prepare a newly added element for inclusion
        function (obj, stringlist) {
            // Add content to object
            var styles = $();
            var scripts = $();
            var div = $('<div>').addClass('-papyro-internal-content');
            var elementsToAdd = $();

            $.each(stringlist, function (i, string) {
                div.append($(string));
            });

            styles = $('style', div).detach();
            scripts = $('script', div).detach();

            utopia.processNewContent(div);
            elementsToAdd = elementsToAdd.add(styles)
                                         .add(div)
                                         .add(scripts);

            var last_content = $('.-papyro-internal-body > *:not(.-papyro-internal-loading)', obj).last();
            if (last_content.length > 0) {
                last_content.after(elementsToAdd);
            } else {
                $('.-papyro-internal-body', obj).prepend(elementsToAdd);
            }

            $('.limited-height', div).each(function () {
                var comment = $(this);
                var expansion = comment.children('.expansion');
                if (expansion.height() < 140) {
                    comment.children('.overlay').trigger('click');
                }
            });

            div.trigger('papyro:polish');
        },

    onResultItemClosed:
        // Do generic processing when a result is being closed
        function (obj) {
            $('.-papyro-internal-body .-papyro-internal-loading', obj).each(function () {
                Spinners.pause(this);
            });
        },

    onResultItemOpened:
        // Do generic processing when a result is being opened
        function (obj) {
            $('.-papyro-internal-body .-papyro-internal-loading', obj).each(function () {
                Spinners.play(this);
            });
        },

    onResultItemContentFinished:
        // Do generic processing when a result has been fully rendered
        function (obj) {
            $('.-papyro-internal-body .-papyro-internal-loading', obj).remove();
        },

};

$(function () {
    utopia.onLoad();
});
