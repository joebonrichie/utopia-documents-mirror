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

//
// This CSL engine wrapper works with the CSLEngine class to set up and provide CSL
// processing funationality from within Qt.
//




// Hack to fix a couple of bugs in citeproc
// Alias the XML language attribute
CSL.Attributes["@xml:lang"] = function (state, arg) {
    // Add a dummy version in if it isn't already present
    if (!state.opt.version) {
        state.opt.version = '1.0';
    }
    // Call the old callback
    var csl_lang_fn = CSL.Attributes["@lang"];
    csl_lang_fn(state, arg);
}




// Wrapper state for keeping track of styles / locales
var Utopia = {
    styles: {},
    locales: {},
    defaultStyle: undefined,
};

function installStyle(code, name, style)
{
    // Find the title from the style JSON
    function find(obj, name) {
        if (obj !== null && typeof obj === 'object' && obj.children) {
            var path = name.split('/');
            name = path.shift();
            if (name == 'text()') {
                return obj.children.join(' ');
            } else {
                for (i = 0; i < obj.children.length; i += 1) {
                    if (obj.children[i].name == name) {
                        if (path.length > 0) {
                            return find(obj.children[i], path.join('/'));
                        } else {
                            return obj.children[i];
                        }
                    }
                }
            }
        }
    }

    Utopia.styles[code] = {
        code: code,
        name: find(style, 'info/title/text()'),
        json: style,
    };
}

function installLocale(code, name, locale)
{
    Utopia.locales[code] = {
        code: code,
        name: name,
        json: locale,
    };
}

function getStyles()
{
    var styles = {};
    for (var code in Utopia.styles) {
        styles[code] = Utopia.styles[code].name;
    }
    return styles;
}

function getLocales()
{
    var locales = {};
    for (var code in Utopia.locales) {
        locales[code] = Utopia.locales[code].name;
    }
    return locales;
}




function convert_name(name)
{
    // From ["Thorne, David"] to [{family: "Thorne", given: "David"}]
}

function convert_date(year, month, day)
{
    // From [2014, March, 12] to {date-parts: [[2014, 3, 12]]}
}

function get_case_insensitive(map, needle) {
    var keys = Object.keys(map);
    for (i = 0; i < keys.length; i += 1) {
        var key = keys[i];
        if (key.toLowerCase() == needle.toLowerCase()) {
            return map[key];
        }
    }
    return false;
}

function format(metadata, style, defaultStyle)
{
    // Give the metadata an ID if it doesn't already have one
    if (!metadata.id) {
        if (metadata.label) {
            metadata.id = metadata.label;
        } else {
            metadata.id = '';
        }
    }

    // Resolve style
    found = get_case_insensitive(Utopia.styles, style) ||
            get_case_insensitive(Utopia.styles, defaultStyle) ||
            get_case_insensitive(Utopia.styles, 'apa');
    if (found) {
        style = found.json;
    }

    var sys = {
        retrieveLocale: function (name) {
            if (Utopia.locales[name]) {
                return Utopia.locales[name].json;
            }
        },
        retrieveItem: function (id) {
            return metadata;
        },
        getAbbreviations: function (name) {
            return {};
        },
    };

    var label = metadata['citation-label'];

    var citeproc = new CSL.Engine(sys, style, 'en-GB')
    citeproc.updateItems([metadata.id], true);
    var bib = citeproc.makeBibliography("CITATION_LABEL");
    var formatted = bib[1][0];

    // Put label in place
    if ("string" === typeof label) {
        if (formatted.indexOf("CITATION_LABEL") == -1) {
            formatted = 'CITATION_LABEL ' + formatted;
        }
        formatted = formatted.replace('CITATION_LABEL', '<strong>' + label.replace(/\.+$/g, '') + '</strong>');
    } else {
        formatted = formatted.replace(/[^a-zA-Z>]*CITATION_LABEL[^a-zA-Z<]*/, '');
    }

    // Swap divs for spans
    formatted = formatted.replace(/<div/g, '<span').replace(/<\/div/g, '</span');

    return formatted;
}
