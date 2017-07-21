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

var domainId;
var proteinId;
var number3d;

/**
 * Links to 3dm start with this.
 */
var webhost = "3dm.bio-prodict.nl";

/**
 * REST requests start with this.
 */
var bearerToken = "#TOKEN#";
var restBaseUrl = "https://api.bio-prodict.nl/persistence3dm/";

var mutationsByAminoId = {};
var mutationsByResidueNumber = {};
var mutationsByRepresentation = {};

var equivalentMutationsByAminoId = {};
var equivalentMutationsByProteinId = {};
var equivalentMutationsByResidueNumber = {};
var equivalentMutationsByRepresentation = {};

var aaTypes = {
    'Ala' : 'A',
    'Cys' : 'C',
    'Asp' : 'D',
    'Glu' : 'E',
    'Phe' : 'F',
    'Gly' : 'G',
    'His' : 'H',
    'Ile' : 'I',
    'Lys' : 'K',
    'Leu' : 'L',
    'Met' : 'M',
    'Asn' : 'N',
    'Pro' : 'P',
    'Gln' : 'Q',
    'Arg' : 'R',
    'Ser' : 'S',
    'Thr' : 'T',
    'Val' : 'V',
    'Trp' : 'W',
    'Tyr' : 'Y'
};

function unique(array) {
    return $.grep(array, function(el, index) {
        return index == $.inArray(el, array);
    });
}

var parseMutationData = function(data) {
    var mutationList = data['mutation'];
    for (var i = 0; i < mutationList.length; i++) {
        var mutation = mutationList[i];
        if (!mutationsByAminoId[mutation.aminoId]) {
            mutationsByAminoId[mutation.aminoId] = [];
        }
        mutationsByAminoId[mutation.aminoId].push(mutation);

        if (!mutationsByResidueNumber[mutation.residueNumber]) {
            mutationsByResidueNumber[mutation.residueNumber] = [];
        }
        mutationsByResidueNumber[mutation.residueNumber].push(mutation);

        var representation = mutation.fromType + mutation.residueNumber + mutation.toType;
        if (!mutationsByRepresentation[representation]) {
            mutationsByRepresentation[representation] = [];
        }
        mutationsByRepresentation[representation].push(mutation);

    }
};

var parseMutationDataAtEquivalentPositions = function(data) {
    var mutationList = data['mutation'];
    console.log(mutationList);
    for (var i = 0; i < mutationList.length; i++) {
        var mutation = mutationList[i];
        if (!equivalentMutationsByAminoId[mutation.aminoId]) {
            equivalentMutationsByAminoId[mutation.aminoId] = [];
        }
        equivalentMutationsByAminoId[mutation.aminoId].push(mutation);

        if (!equivalentMutationsByResidueNumber[mutation.residueNumber]) {
            equivalentMutationsByResidueNumber[mutation.residueNumber] = [];
        }
        equivalentMutationsByResidueNumber[mutation.residueNumber].push(mutation);

        var proteinDisplay = mutation.proteinUniprotId;
        if (!proteinDisplay) {
            proteinDisplay = mutation.proteinAccession;
        }
        var representation = proteinDisplay + "|" + mutation.fromType + mutation.residueNumber + mutation.toType;
        if (!equivalentMutationsByRepresentation[representation]) {
            equivalentMutationsByRepresentation[representation] = [];
        }
        equivalentMutationsByRepresentation[representation].push(mutation);

        if (!equivalentMutationsByProteinId[proteinDisplay]) {
            equivalentMutationsByProteinId[proteinDisplay] = [];
        }
        var localRepresentation = mutation.fromType + mutation.residueNumber + mutation.toType;
        if ($.inArray(localRepresentation, equivalentMutationsByProteinId[proteinDisplay]) == -1) {
            equivalentMutationsByProteinId[proteinDisplay].push(localRepresentation);
        }
    }
};

function getUniqueMutationsForEquivalentTable() {
    var table = "<table class=\"mutationsAtEquivalentPositionsTable hor-minimalist-b\">";
    table += "<thead><tr><th class=\"proteinId\">Protein</th><th>From</th><th>#</th><th>To</th><th>Details</th></tr></thead><tbody>";
    for ( var proteinId in equivalentMutationsByProteinId) {
        for (var i = 0; i < equivalentMutationsByProteinId[proteinId].length; i++) {
            var mutationString = equivalentMutationsByProteinId[proteinId][i];
            var representation = proteinId + "|" + mutationString;

            var mutation = equivalentMutationsByRepresentation[representation][0];
            // console.log(mutation)
            //var aminoId = mutation.aminoId;
            //var toType = mutation.toType;
            //var domainId = mutation.domainId;
            //var proteinDbId = mutation.proteinId;
            var proteinAc = mutation.proteinAccession;
            var mutations = equivalentMutationsByRepresentation[representation];
            // console.log(mutations);
            var pubmedUrl = getPubmedUrl(mutations);
            // table += "<tr><td class=\"proteinId\">" + proteinId + "</td><td>" + mutationString[0] + "</td><td>" + mutationString.slice(1,-1) + "</td><td>" + mutationString.slice(-1) + "</td><td class=\"mutationDetailsLink\" representation=\"" + representation + "\">Pubmed (" + equivalentMutationsByRepresentation[representation].length + ")</td></tr>";
            table += "<tr><td class=\"proteinId\"><a href=\"https://" + webhost
                    + "/index.php?&amp;mode=pdetail&amp;proteinName=" + proteinAc
                    + "&amp;familyid=1&amp;filterid=1&amp;numberingscheme=-1&amp;sfamid=" + domainId + "\">"
                    + proteinId + "</a></td><td>" + mutationString[0] + "</td><td>" + mutationString.slice(1, -1)
                    + "</td><td>" + mutationString.slice(-1) + "</td><td><a href=\"" + pubmedUrl + "\">" + "Pubmed ("
                    + getUniquePubmedIds(mutations).length + "</a>)</td></tr>";
        }
    }
    table += "</tbody></table>";
    return table;
}

function getUniqueMutationsForPosition(residueNumber) {
    var setset = {};
    var mutations = mutationsByResidueNumber[residueNumber];
    if (mutations == null) {
        return [];
    }
    for (var i = 0; i < mutations.length; i++) {
        var mutation = mutations[i];
        var representation = mutation.fromType + mutation.residueNumber + mutation.toType;
        setset[representation] = true;
    }

    var mutationList = [];
    for ( var mutation in setset) {
        mutationList.push(mutation);
    }
    return mutationList;
}

function getUniqueMutationsForPositionTable(residueNumber) {
    var mutationList = getUniqueMutationsForPosition(residueNumber);
    if (mutationList.length == 0) {
        return '';
    }

    var table = "<table class=\"mutationsAtThisPositionTable hor-minimalist-b\">";
    table += "<thead><tr><th>From</th><th>#</th><th>To</th><th>Details</th></tr></thead><tbody>";
    for (var i = 0; i < mutationList.length; i++) {
        var mutationString = mutationList[i];
        var mutations = mutationsByRepresentation[mutationString];
        //var mutation = mutationsByRepresentation[mutationString][0];
        //var aminoId = mutation.aminoId;
        //var toType = mutation.toType;
        var pubmedUrl = getPubmedUrl(mutations);
        table += "<tr><td>" + mutationString[0] + "</td><td>" + mutationString.slice(1, -1) + "</td><td>"
                + mutationString.slice(-1) + "</td><td><a href=\"" + pubmedUrl + "\">" + "Pubmed ("
                + getUniquePubmedIds(mutations).length + "</a>)</td></tr>";
    }
    table += "</tbody></table>";
    return table;
}

function getPubmedUrl(mutations) {
    var pubmedIds = [];
    for (var i = 0; i < mutations.length; i++) {
        var mutation = mutations[i];
        if ($.inArray(mutation.pubmedId, pubmedIds) == -1) {
            pubmedIds.push(mutation.pubmedId);
        }
    }
    var pubmedUrl = 'http://www.ncbi.nlm.nih.gov/pubmed?term=' + pubmedIds.join();
    return pubmedUrl;
}

function getUniquePubmedIds(mutations) {
    var pubmedIds = [];
    for (var i = 0; i < mutations.length; i++) {
        var mutation = mutations[i];
        if ($.inArray(mutation.pubmedId, pubmedIds) == -1) {
            pubmedIds.push(mutation.pubmedId);
        }
    }
    return pubmedIds;
}

var renderer = function(url, successHandler) {
    console.log('renderer url: ' + url);
    $.ajax({
        type : 'GET',
        url : encodeURI(url),
        beforeSend : function(xhr) {
            xhr.setRequestHeader("Authorization", bearerToken);
            xhr.setRequestHeader("SFAMID", domainId);
        },
        error : function(xhr, ajaxOptions, thrownError) {
            console.log(xhr.statusText);
            console.log(xhr.responseText);
            console.log(xhr.status);
            console.log(thrownError);
        },
        success : successHandler,
        dataType : 'json'
    });
};

var callbackFMutations = function(returnData) {
    parseMutationData(returnData);
    table = getUniqueMutationsForPositionTable($('.residueNumber3dm').text());
    if (table.length == 0) {
        $('.mutationsInProtein')
                .html(
                        '<div class="accordionText" style="margin-top: 10px;">No mutations were found for this position.</div>');
    }
    else {
        $('.mutationsInProtein').html(table);
        $('.mutationsInProtein table').tablesorter();
        $('.mutationDetailsLink').click(function() {
            var representation = $(this).attr('representation');
            var mutations = mutationsByRepresentation[representation];

            var proteinDisplay = mutations[0].proteinUniprotId;
            if (!proteinDisplay) {
                proteinDisplay = mutations[0].proteinAccession;
            }

            var pubmedIds = [];
            for (var i = 0; i < mutations.length; i++) {
                var mutation = mutations[i];
                if ($.inArray(mutation.pubmedId, pubmedIds) == -1) {
                    pubmedIds.push(mutation.pubmedId);
                }
            }

            var details = {
                representation : representation,
                mutations : mutations,
                proteinDisplay : $('.residueProteinUrl3dm').text(),
                proteinDbId : mutations[0].proteinId,
                domainId : domainId,
                residueNumber : mutations[0].residueNumber,
                residueNumber3d : number3d,
                pubmedIds : pubmedIds,
                renderTo : '.mutationsInProteinLiterature'
            };

            renderPubmedDetails(details);
        });
    }
};

var callbackFMutationsAtEquivalentPositions = function(returnData) {
    parseMutationDataAtEquivalentPositions(returnData);
    // console.log(equivalentMutationsByProteinId);
    table = getUniqueMutationsForEquivalentTable();
    $('.mutationsAtEquivalentPositions').html(table);
    $('.mutationsAtEquivalentPositions table').tablesorter();
    $('.mutationDetailsLink').click(function() {
        var representation = $(this).attr('representation');
        var mutations = equivalentMutationsByRepresentation[representation];
        console.log(mutations);

        var proteinDisplay = mutations[0].proteinUniprotId;
        if (!proteinDisplay) {
            proteinDisplay = mutations[0].proteinAccession;
        }

        var pubmedIds = [];
        for (var i = 0; i < mutations.length; i++) {
            var mutation = mutations[i];
            if ($.inArray(mutation.pubmedId, pubmedIds) == -1) {
                pubmedIds.push(mutation.pubmedId);
            }
        }
        var pubmedUrl = 'http://www.ncbi.nlm.nih.gov/pubmed?term=' + pubmedIds.join();
        console.log(pubmedUrl);
        window.location = pubmedUrl;
    });
};

$(function() {
    $('.mutationInfoContainer3dm').slice(1).remove();

    domainId = $('.domainId').text();
    proteinId = $('.proteinDbId').text();
    number3d = $('.residueNumber3d3dm').text();

    //$('.info3dNumber').text('asdfasdfadf').show();
    console.log(number3d);
    if (parseInt(number3d) < 1) {
        $('.info3dNumber').hide();
        if (parseInt(number3d) == 0) {
            $('.residueProteinUrl3dm').after('<br/><br/><span><i>Note: unaligned protein</i></span>');
        }
    }

    var conservationData = [];
    if ($('.conservationData').text().length) {
        console.log($('.conservationData'));
        conservationData = $.parseJSON($('.conservationData').text())['data'];
        console.log(conservationData);

        dataPoints = [];
        for ( var i in conservationData) {
            var data = [];
            data[0] = i;
            data[1] = parseFloat(conservationData[i]);
            console.log(data);
            dataPoints.push(data);
        }
        console.log(dataPoints);

        var chart = jQuery.jqplot('container3dmConservation', [ dataPoints ], {
            highlighter : {
                formatString : '%s',
                tooltipLocation : 'sw',
                useAxesFormatters : false
            },
            seriesDefaults : {
                renderer : jQuery.jqplot.PieRenderer,
                rendererOptions : {
                    // Turn off filling of slices.
                    // fill: false,
                    showDataLabels : true,
                    dataLabelNudge : 10,
                    // Add a margin to seperate the slices.
                    sliceMargin : 4,
                    // stroke the slices with a little thicker line.
                    lineWidth : 2,
                    dataLabels : 'label'
                }
            },
            grid : {
                background : 'white',
                borderColor : 'white',
                shadow : false
            },
            legend : {
                show : false,
                location : 'e'
            }
        });
        $('#container3dmConservation').bind(
                'jqplotDataClick',
                function(ev, seriesIndex, pointIndex, data) {
                    if (aaTypes[data[0]] != undefined) {
                        var url = 'https://' + webhost + '/index.php?&mode=sequencelist&positions['
                                + number3d + ']=' + aaTypes[data[0]]
                                + '&familyid=1&filterid=1&numberingscheme=-1&sfamid=' + domainId;
                        console.log(url);
                        window.control.activateLink(url, '');
                    }
                    // window.open(url);
                });
        $('.jqplot-data-label').css('color', 'white');
    }
    console.log(conservationData);

    var familyId = 1;
    mutationsInProteinUrl = restBaseUrl + "getMutationsByProteinId/" + proteinId;
    mutationsAtEquivalentPositionsUrl = restBaseUrl + "getMutationsBy3dNumber/" + familyId + "/" + number3d;

    urls = {};
    urls['mutationsInProteinUrl'] = mutationsInProteinUrl;
    urls['mutationsAtEquivalentPositionsUrl'] = mutationsAtEquivalentPositionsUrl;
    $('.accordion')
            .accordion(
                    {
                        collapsible : true,
                        heightStyle : 'content',
                        active : true,
                        beforeActivate : function(event, ui) {
                            console.log(ui.newHeader.attr('data-url'));
                            url = urls[ui.newHeader.attr('data-url')];
                            if (ui.newHeader.attr('data-url') == "mutationsInProteinUrl") {
                                if ($('.mutationsInProtein').html().length == 0) {
                                    $('.mutationsInProtein')
                                            .html(
                                                    "<div align=\"center\"><img src=\"https://3dm.bio-prodict.nl/utopia/ajax-loader.gif\"/></div>");
                                    renderer(url, callbackFMutations);
                                }
                            }
                            if (ui.newHeader.attr('data-url') == "mutationsAtEquivalentPositionsUrl") {
                                if ($('.mutationsAtEquivalentPositions').html().length == 0) {
                                    $('.mutationsAtEquivalentPositions')
                                            .html(
                                                    "<div align=\"center\"><img src=\"https://3dm.bio-prodict.nl/utopia/ajax-loader.gif\"/></div>");
                                    renderer(url, callbackFMutationsAtEquivalentPositions);
                                }
                            }
                        },
                        activate : function(event, ui) {
                            $(this).accordion('refresh');
                        }
                    });

    console.log($('.accordion'));
    $('.accordion').accordion('option', 'active', 0);

    $('.papersLink').click(function() {
        $(".dialog").text($(this).attr('title')).dialog();
    });

    return true;
});