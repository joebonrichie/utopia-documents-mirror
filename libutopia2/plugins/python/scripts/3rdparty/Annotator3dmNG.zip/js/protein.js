var domainId;
var proteinId;

/**
 * REST requests start with this.
 */
var bearerToken = "#TOKEN#";
var restBaseUrl = "https://api.bio-prodict.nl/persistence3dm/";

var mutationsByAminoId = [];
var mutationsByResidueNumber = [];
var mutationsByRepresentation = [];

var sequencePanelLoaded = false;
var structuresPanelLoaded = false;
var mutationsInProteinUrl;
var aminoAcidsUrl;

var aminoAcids;
var mutations;
var pdbIds;

/*
 SETTINGS
 */
var mutationsInfoPanelEnabled = false;
var defaultFadeDuration = 300;

function unique(array) {
    return $.grep(array, function(el, index) {
        return index == $.inArray(el, array);
    });
}

var formatSequence = function(data) {
    aminoAcids = data;
    var aminoAcidsArray = data['aminoAcid'];
    var boxSize = 10;
    var boxesPerRow = 3;
    var colspan = boxesPerRow;
    var box, row;

    console.log(aminoAcidsArray);

    $('.sequencePanel').empty();
    $('.sequencePanel').append('<table>');
    $('.sequencePanel table').append('<tr>');
    $('.sequencePanel tr:last').append('<td>');

    row = $('.sequencePanel tr:last');
    box = $('.sequencePanel td:last');

    for (var i = 0; i < aminoAcidsArray.length; i++) {
        var aa = aminoAcidsArray[i];

        if (i > 0 && (i % boxSize == 0 || i % (boxSize * boxesPerRow) == 0)) {
            if (i % boxSize == 0) {
                box.append('<span class="positionNumber">' + i + '</span>');
                colspan += -1;
            }
            if (i % (boxSize * boxesPerRow) == 0) {
                $('.sequencePanel table').append('<tr>');
                colspan = boxesPerRow;
            }
            row = $('.sequencePanel tr:last');
            row.append('<td>');
            box = $('.sequencePanel td:last');
        }

        if (aa.residueNumber3d > 0) {
            box.append('<span class="aa" residueNumber3d="' + aa.residueNumber3d + '" residueNumber="'
                    + aa.residueNumber + '" aminoId="' + aa.aminoId + '">' + aa.aminoAcidType + '</span>');
        }
        else {
            box.append('<span class="aa" residueNumber="' + aa.residueNumber + '">' + aa.aminoAcidType + '</span>');
        }

    }
    box.append('<span class="positionNumber">' + i + '</span>');

    if (colspan != 0) {
        row.append('<td colspan="' + colspan + '">');
    }

    $('.aa').click(function() {
        console.log('clicked on aa');
        var number = parseInt($(this).attr('residueNumber')) - 1;
        console.log(number);
        setResidueInfo(aminoAcids['aminoAcid'][number]);
        $('.aa').removeClass('selectedAa');
        $(this).addClass('selectedAa');

    });
    $('.colorCores').click(function() {
        disableMutationColors();
        toggleCoreColors($(this));
    });

    $('.colorMutations').click(function() {
        disableCoreColors();
        toggleMutationColors($(this));
    });
};

var parseMutationData = function(data) {
    mutations = data;
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

var sequenceRenderer = function(url, successHandler) {
    console.log('sequencerenderer url ' + url);
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

var pdbRenderer = function(url, successHandler) {
    console.log('pdbrenderer url ' + url);
    $.ajax({
        type : 'GET',
        url : encodeURI(url),
        error : function(xhr, ajaxOptions, thrownError) {
            console.log(xhr.statusText);
            console.log(xhr.responseText);
            console.log(xhr.status);
            console.log(thrownError);
        },
        success : successHandler,
        dataType : 'xml',
        cache : true
    });
};

var getPdbRestUrl = function(pdbIds) {
    return 'http://www.rcsb.org/pdb/rest/describePDB?structureId=' + pdbIds.join();
};

var callbackSequenceRenderer = function(returnData) {
    console.log('callbackSequenceRenderer');
    console.log('sequence return data:');
    console.log(returnData);

    if (returnData['aminoAcid'].length == 0) {
        $('.sequencePanel').empty();
        $('.sequencePanel').append('<div class="content message">This is a non-aligned protein</div>');
    }
    else {
        formatSequence(returnData);
        sequenceRenderer(mutationsInProteinUrl, callbackMutationSequenceRenderer);
    }
};

var callbackMutationSequenceRenderer = function(returnData) {
    console.log('callbackMutationSequenceRenderer');
    console.log(returnData);
    parseMutationData(returnData);

    //enable mutation colors by default.
    for ( var residueNumber in mutationsByResidueNumber) {
        $('.aa[residueNumber="' + residueNumber + '"]').attr('colorMutations', '1');
    }
    // sequenceRenderer(mutationsInProteinUrl, callbackMutationSequenceRenderer);
};

var callbackPdbData = function(returnData) {
    console.log(returnData);

    // console.log(returnData.length);
    if ($(returnData).find('PDB').length > 0) {
        var table = "<table class=\"pdbStructures hor-minimalist-b\">";
        table += "<thead><tr><th>ID</th><th>Method</th><th>Resolution</th><th># Residues</th></tr></thead><tbody>";
        console.log('looping through pdbs');
        // console.log($(returnData));
        // console.log($(returnData).find('PDB'));
        // console.log($(returnData).find('pdbdescription'));

        $(returnData).find('PDB').each(
                function() {
                    console.log($(this));
                    var pdb = $(this);
                    var pdbId = pdb.attr('structureId');
                    var expMethod = pdb.attr('expMethod');
                    var resolution = pdb.attr('resolution');
                    var residues = pdb.attr('nr_residues');
                    table += "<tr><td><a href=\"http://www.ebi.ac.uk/pdbsum/" + pdbId + '">' + pdbId + "</a></td><td>"
                            + expMethod.toLowerCase() + "</td><td>" + resolution + "</td><td>" + residues
                            + "</td></tr>";
                });
        table += "</tbody></table>";
        $('.structuresPanel').html(table);
        $('.structuresPanel table').tablesorter();
    }
};

var disableCoreColors = function() {
    $('.colorCores').text('Show cores');
    $('.aa[residueNumber3d]').removeAttr('colorCores');
};

var disableMutationColors = function() {
    $('.colorMutations').text('Show mutations');
    $('.aa').removeAttr('colorMutations');
};

var toggleCoreColors = function(coreSwitch) {
    if (coreSwitch.text() == 'Show cores') {
        coreSwitch.text('Hide cores');
        $('.aa[residueNumber3d]').attr('colorCores', '');
    }
    else {
        coreSwitch.text('Show cores');
        $('.aa[residueNumber3d]').removeAttr('colorCores');
    }
};

var toggleMutationColors = function(mutationSwitch) {
    if (mutationSwitch.text() == 'Show mutations') {
        mutationSwitch.text('Hide mutations');
        for ( var residueNumber in mutationsByResidueNumber) {
            $('.aa[residueNumber="' + residueNumber + '"]').attr('colorMutations', '');
        }
    }
    else {
        mutationSwitch.text('Show mutations');
        $('.aa').removeAttr('colorMutations');
    }
};

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

function showMutationsInAttributeInfoPanel(residueNumber) {
    $('.residueAttributeLine .mutations').attr('selected', 'true');
    var sequenceInfoPanelContents = $('.sequenceInfoPanelContents');
    uniqueMutations = getUniqueMutationsForPosition(residueNumber);
    if ($('.sequenceInfoPanelContents .attributeInfoPanel').length == 0) {
        sequenceInfoPanelContents.append('<div class="attributeInfoPanel">');
    }
    else {
        $('.sequenceInfoPanelContents .attributeInfoPanel').empty();
    }
    $('.sequenceInfoPanelContents .attributeInfoPanel').hide();

    //Add mutation header and a list of unique mutations
    $('.sequenceInfoPanelContents .attributeInfoPanel').append('<h2>Mutations</h2>').append('<ul>');
    for (var i = 0; i < uniqueMutations.length; i++) {
        $('.sequenceInfoPanelContents .attributeInfoPanel ul').append(
                '<li class="uniqueMutation">' + uniqueMutations[i] + '</li>');
    }
    $('.sequenceInfoPanelContents .attributeInfoPanel').fadeIn(defaultFadeDuration);

    //Define the behaviour for clicking on a unique mutation
    $('.uniqueMutation').click(
            function() {
                $('.uniqueMutation').removeAttr('selected');
                $(this).attr('selected', 'true');
                //If we don't have a attributeInfoDetailPanel, we add one
                if ($('.sequenceInfoPanelContents .attributeInfoDetailPanel').length == 0) {
                    sequenceInfoPanelContents.append('<div class="attributeInfoDetailPanel">');
                }
                else {
                    $('.sequenceInfoPanelContents .attributeInfoDetailPanel').empty();
                }

                //Start with hiding the attributeInfoDetailPanel
                $('.sequenceInfoPanelContents .attributeInfoDetailPanel').hide();

                //Loop through the list of mutations and add them to an <ul>
                var mutations = mutationsByRepresentation[$(this).text()];
                $('.sequenceInfoPanelContents .attributeInfoDetailPanel').append('<h2>Pubmed</h2>').append('<ul>');
                pubmedIds = [];
                for (var i = 0; i < mutations.length; i++) {
                    var mutation = mutations[i];
                    pubmedIds.push(mutation.pubmedId);
                }
                pubmedIds = unique(pubmedIds);
                $('.sequenceInfoPanelContents .attributeInfoDetailPanel').append(
                        '<a href="http://www.pubmed.com/' + pubmedIds.join() + '">PubMed (' + pubmedIds.length
                                + ')</span>');
                $('.sequenceInfoPanelContents .attributeInfoDetailPanel').fadeIn(defaultFadeDuration);
            });
}

function setResidueInfo(residue) {
    console.log('setting residue info');
    console.log(residue);
    var sequenceInfoPanelContents = $('.sequenceInfoPanelContents');
    sequenceInfoPanelContents.empty();
    sequenceInfoPanelContents.hide();
    sequenceInfoPanelContents.append('<div class="residueOverview">');
    var sequenceInfoPanelResidueOverview = $('.sequenceInfoPanelContents .residueOverview');
    sequenceInfoPanelResidueOverview.append('<div class="residueInfo">');
    var sequenceInfoPanelResidueInfo = $('.sequenceInfoPanelContents .residueInfo');

    sequenceInfoPanelResidueInfo.append('<span class="residueInfoLine"><span class="residueType">'
            + residue.aminoAcidType + '</span></span>');
    sequenceInfoPanelResidueInfo.append('<span class="residueInfoLine"><span class="residueNumber">'
            + residue.residueNumber + '</span></span>');
    if (residue.residueNumber3d > 0) {
        sequenceInfoPanelResidueInfo
                .append('<br/><span class="residueInfoLine"><span class="residueNumber3d">3D number: '
                        + residue.residueNumber3d + '</span></span>');
    }

    //Attributes start here
    sequenceInfoPanelResidueOverview.append('<div class="residueAttributes">');
    sequenceInfoPanelResidueAttributes = $('.sequenceInfoPanelContents .residueAttributes');
    if (mutationsByResidueNumber[residue.residueNumber]) {
        sequenceInfoPanelResidueAttributes
                .append('<br/><span class="residueAttributeLine"><span class="mutations">mutations ('
                        + getUniqueMutationsForPosition(residue.residueNumber).length + ')</span></span>');
    }

    if (mutationsInfoPanelEnabled && getUniqueMutationsForPosition(residue.residueNumber).length != 0) {
        showMutationsInAttributeInfoPanel(residue.residueNumber);
    }

    sequenceInfoPanelContents.fadeIn(defaultFadeDuration);

    $('.residueAttributeLine').click(function() {
        $('.residueAttributeLine').removeAttr('selected');
        $('.residueAttributeLine').attr('selected', 'true');
    });

    $('.residueAttributeLine .mutations').click(function() {
        mutationsInfoPanelEnabled = true;
        $('.sequenceInfoPanelContents .attributeInfoDetailPanel').remove();

        showMutationsInAttributeInfoPanel(residue.residueNumber);
    });
}

$(function() {
    domainId = $('.domainId').text();
    proteinId = $('.proteinDbId').text();
    var familyId = 1;

    var urls = {};

    /* Render pdb information */
    pdbIds = [];
    $('.pdbIdentifier').each(function() {
        pdbIds.push($(this).text());
    });

    if (pdbIds.length > 0) {
        var pdbUrl = getPdbRestUrl(pdbIds);
        urls['proteinStructuresUrl'] = pdbUrl;
    }
    // pdbRenderer(url, callbackPdbData);

    mutationsInProteinUrl = restBaseUrl + "getMutationsByProteinId/" + proteinId;
    aminoAcidsUrl = restBaseUrl + "getAminoAcids/" + familyId + "/" + proteinId;

    urls['proteinSequenceUrl'] = aminoAcidsUrl;

    console.log('building accordion');
    $('.accordion')
            .accordion(
                    {
                        collapsible : true,
                        heightStyle : 'content',
                        active : true,
                        beforeActivate : function(event, ui) {
                            console.log(ui.newHeader.attr('data-url'));
                            url = urls[ui.newHeader.attr('data-url')];
                            if (ui.newHeader.attr('data-url') == "proteinSequenceUrl") {
                                if ($('.sequencePanel').html().length == 0) {
                                    $('.sequencePanel')
                                            .html(
                                                    "<div align=\"center\"><img src=\"https://3dm.bio-prodict.nl/utopia/ajax-loader.gif\"/></div>");
                                    console.log(url);
                                    sequenceRenderer(aminoAcidsUrl, callbackSequenceRenderer);
                                }
                            }
                            if (ui.newHeader.attr('data-url') == "proteinStructuresUrl") {
                                if ($('.structuresPanel').html().length == 0) {
                                    $('.structuresPanel')
                                            .html(
                                                    "<div align=\"center\"><img src=\"https://3dm.bio-prodict.nl/utopia/ajax-loader.gif\"/></div>");
                                    console.log(url);
                                    pdbRenderer(url, callbackPdbData);
                                }
                            }
                        },
                        activate : function(event, ui) {
                            $(this).accordion('refresh');
                        }
                    });
    console.log('done building accordion, activating');
    $('.accordion').accordion('option', 'active', 0);
    console.log('done activating accordion');
});