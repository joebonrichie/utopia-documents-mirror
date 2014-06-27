/*****************************************************************************
 *  
 *   This file is part of the Utopia Documents application.
 *       Copyright (c) 2008-2014 Lost Island Labs
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


var papyro = {

    citation:
        {
            styles: window.control.availableCitationStyles,
            defaultStyle: window.control.defaultCitationStyle,
            format: window.control.formatCitation,
        },

    elements:
        {},

    templates:
        {},

    onLoad:
        // Initially prepare the HTML document
        function () {
            // Get element references and template nodes
            var results = papyro.elements['results'] = $('#-papyro-internal-results').first();
            papyro.templates['result'] = $('#-papyro-internal-result_template').first().detach();

            // Hijack link actions on an element
            results.delegate('a', 'click', function (e) {
                var href = this.getAttribute('href');
                if (href == undefined) {
                    href = this.getAttributeNS('http://www.w3.org/1999/xlink', 'href');
                }
                var target = this.getAttribute('target');
                if (target == undefined) {
                    target = this.getAttributeNS('http://www.w3.org/1999/xlink', 'show');
                }
                window.control.activateLink(href, target);
            });

            // Remove failed images by default
            results.delegate('img', 'error', function () { $(this).remove(); });

            // Set up connections
            control.resultAdded.connect(papyro.onResultItemAdded);
        },

    clear:
        // Clear all results from list FIXME
        function () {
            $('.-papyro-internal-result').remove();
        },

    result:
        // Get access to the control object for a given result
        function (obj, c) {
            return $(obj).closest('.-papyro-internal-result').data('control', c);
        },

    toggleSlide:
        // Function to toggle an object's content
        function (obj) {
            $(obj).closest('.-papyro-internal-result').find('.-papyro-internal-summary .-papyro-internal-body').each(function () {
                if ($(this).is(':hidden')) {
                    papyro.onResultItemOpened(obj);
                    $(this).css({opacity: 0}).slideDown(100).animate({opacity: 1}, 100);
                } else {
                    $(this).animate({opacity: 0}, 100).slideUp(100);
                    papyro.onResultItemClosed(obj);
                }
            });
        },

    onResultItemAdded:
        // Prepare a newly added result element
        function (obj) {
            // Add a new result element
            result = obj.element = papyro.templates['result'].clone().get(0);
            papyro.result(result, obj);

            // Insert it into the tree (at the top for defaultly open results)
            subsequent = $('.-papyro-internal-result', papyro.elements['results']).filter(function (idx) {
                var candidate = papyro.result(this);
                return (obj.headless || !candidate.headless) && (candidate.weight < obj.weight);
            });
            if (subsequent.length > 0) {
                subsequent.first().before(result);
            } else {
                papyro.elements['results'].append(result);
            }

            // Connect content handler
            obj.insertContent.connect(papyro.onResultItemContentAdded)

            // Hide content by default
            $('.-papyro-internal-summary .-papyro-internal-body', result).slideUp(0);

            if (obj.headless) {
                $('.-papyro-internal-header', result).hide();
            } else {
                // Remove unreachable thumbnails
                $('.-papyro-internal-header .-papyro-internal-thumbnail img', result).error(function () { $(this).remove(); });
                $('.-papyro-internal-header .-papyro-internal-thumbnail img.-papyro-internal-source', result).click(function (event) {
                    // Stop event from reaching the parent
                    event.stopPropagation();
                    // Send the result object off to be activated
                    control.activateSource(papyro.result(this));
                });
                $('.-papyro-internal-header .-papyro-internal-thumbnail img.-papyro-internal-author', result).click(function (event) {
                    // Stop event from reaching the parent
                    event.stopPropagation();
                    // Send the result object off to be activated
                    control.activateAuthor(papyro.result(this));
                });

                // Set visible information
                var title = $("<div/>").html(obj.title).text();
                var description = $("<div/>").html(obj.description).text();
                $('.-papyro-internal-header .-papyro-internal-title', result).text(title);
                if (obj.description) {
                    $('.-papyro-internal-header .-papyro-internal-description', result).text(description);
                //} else {
                //    $('.header .description', result).text('-');
                }
                if (obj.sourceIcon) {
                    $('.-papyro-internal-header .-papyro-internal-thumbnail img.-papyro-internal-source', result).attr('src', obj.sourceIcon);
                } else if (obj.sourceDatabase) {
                    $('.-papyro-internal-header .-papyro-internal-thumbnail img.-papyro-internal-source', result).attr('src', 'http://utopia.cs.manchester.ac.uk/images/' + obj.sourceDatabase + '.png');
                } else {
                    $('.-papyro-internal-header .-papyro-internal-thumbnail img.-papyro-internal-source', result).remove();
                }
                if (obj.authorUri) {
                    $('.-papyro-internal-header .-papyro-internal-thumbnail img.-papyro-internal-author', result).attr('src', obj.authorUri + '/avatar');
                } else {
                    $('.-papyro-internal-header .-papyro-internal-thumbnail img.-papyro-internal-author', result).remove();
                }
            }

            // Set up interaction events
            $('.-papyro-internal-header', result).click(function () { papyro.result(this).toggleContent(); });

            // Mouse over colours
            $(result).bind('mouseenter', function () { $(this).addClass('selected'); });
            $(result).bind('mouseleave', function () { $(this).removeClass('selected'); });

            // Animate the result's appearance
            $(result).css({opacity: 0}).slideDown(100).animate({opacity: 1}, 100);

            // Create and start a spinner
            $('.-papyro-internal-summary .-papyro-internal-body .-papyro-internal-loading', result).each(function () {
                Spinners.create(this);
            });

            // Open if requested
            if (obj.openByDefault) {
                $('.-papyro-internal-header', result).click();
            }

            // HACK to set the highlight colour
            if (obj.highlight) {
                //alert(obj.highlight);
                $('.-papyro-internal-header', result).css({'borderLeft': 'solid 4px ' + obj.highlight, 'paddingLeft': '6px'});
            }
        },

    processNewContent:
        // Process new content
        function (obj) {
            // Wrap in jQuery
            obj = $(obj);

            // Modify the DOM to include expandy/contracty DIVs
            obj.find('.expandable[title]').add(obj.filter('.expandable[title]')).wrapInner('<div class="expansion" />').each(function () {
                var expandable = $(this);
                var caption = $('<div class="caption"/>');
                caption.click(function () {
                    if ($(this).next('.expansion').filter(':visible').size() > 0) {
                        $(this).children('img.arrow').rotate({ angle: 90, animateTo: 0, duration: 200 });
                        $(this).next('.expansion').animate({opacity: 0}, 100).slideUp(100);
                    } else {
                        $(this).children('img.arrow').rotate({ angle: 0, animateTo: 90, duration: 200 });
                        $(this).next('.expansion').css({opacity: 0}).slideDown(100).animate({opacity: 1}, 100);
                    }
                });
                caption.prepend($('<img src="qrc:/icons/expandable_arrow.png" width="10" height="10" class="arrow" />'));
                caption.append(expandable.attr('title'));
                expandable.removeAttr('title');
                expandable.prepend(caption);
            }).children('.expansion').hide();

            // Modify the DOM to include More... links
            obj.find('.readmore').add(obj.filter('.readmore')).wrapInner('<span class="expansion" />').each(function () {
                var expandable = $(this);
                var readmore = $('<span>&hellip; <a class="morelink" title="Show more&hellip;" href="#">[more]</a></span>');
                var readless = $('<span> <a class="lesslink" title="Show less." href="#">[less]</a></span>').hide();
                readmore.click(function () {
                    $(this).next('.expansion').show();
                    $(this).hide();
                    readless.show();
                });
                readless.click(function () {
                    $(this).prev('.expansion').hide();
                    readmore.show();
                    $(this).hide();
                });
                expandable.prepend(readmore);
                expandable.append(readless);
            }).children('.expansion').hide();

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
        function (obj, content) {
            // Add content to object
            var div = $('<div/>');
            div.append(content);
            div.find('img').hide().load(function () {
                $(this).css({opacity: 0}).slideDown(100).animate({opacity: 1}, 100);
            });
            $('.-papyro-internal-content', obj).append(div);

            papyro.processNewContent(div);
        },

    onResultItemClosed:
        // Do generic processing when a result is being closed
        function (obj) {
            $('.-papyro-internal-summary > .-papyro-internal-body .-papyro-internal-loading', obj).each(function () {
                Spinners.pause(this);
            });
        },

    onResultItemOpened:
        // Do generic processing when a result is being opened
        function (obj) {
            $('.-papyro-internal-summary .-papyro-internal-body .-papyro-internal-loading', obj).each(function () {
                Spinners.play(this);
            });
        },

    onResultItemContentFinished:
        // Do generic processing when a result has been fully rendered
        function (obj) {
            $('.-papyro-internal-summary .-papyro-internal-body .-papyro-internal-loading', obj).each(function () {
                Spinners.remove(this);
            });
        },

};

$(function () {
    papyro.onLoad();
});
