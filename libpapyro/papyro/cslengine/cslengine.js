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

function installStyle(code, description, style)
{
    Utopia.styles[code] = {
        name: description,
        json: style,
    };
}

function installLocale(code, description, locale)
{
    Utopia.locales[code] = {
        name: description,
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
    if (Utopia.styles[style]) {
        style = Utopia.styles[style].json;
    } else if (Utopia.styles[defaultStyle]) {
        style = Utopia.styles[defaultStyle].json;
    } else if (Utopia.styles['apa']) {
        style = Utopia.styles['apa'].json;
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
    var bib;
    if ("string" === typeof label) {
        bib = citeproc.makeBibliography("CITATION_LABEL");
    } else {
        bib = citeproc.makeBibliography();
    }
    var formatted = bib[1][0];

    // Put label in place
    if ("string" === typeof label) {
        if (formatted.indexOf("CITATION_LABEL") == -1) {
            formatted = 'CITATION_LABEL. ' + formatted;
        }
        formatted = formatted.replace('CITATION_LABEL', '<strong>' + label + '</strong>');
    } else {
        formatted = formatted.replace(/[^a-zA-Z>]*CITATION_LABEL[^a-zA-Z<]*/, '');
    }

    // Swap divs for spans
    formatted = formatted.replace('<div', '<span').replace('</div', '</span');

    return formatted;
}
