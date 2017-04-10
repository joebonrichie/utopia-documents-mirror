// Molecules javascript for creating interactive molecule visualisations in the
// sidebar of Utopia Documents

// Set up the molecules plugin
(function ($) {
    // Plugin class names
    var prefix = 'molecule';
    var cls_viewer = prefix+'-viewer';

    // Content generation utilities
    function create_radio_form(name, values) {
        var form = $('<form>');
        $.each(values, function (i, value) {
            form.append($('<input>').attr({type: 'radio', name: name, value: value[0]}));
            form.append($('<label>').attr({title: value[1]}));
        });
        form.children('input').uniqueId().each(function () {
            $(this).next('label').attr({for: this.id});
        });
        return form;
    }
    function create_checkbox_form(values) {
        var form = $('<form>');
        $.each(values, function (i, value) {
            form.append($('<input>').attr({type: 'checkbox', name: value[0]}));
            form.append($('<label>').attr({title: value[1]}));
        });
        form.children('input').uniqueId().each(function () {
            $(this).next('label').attr({for: this.id});
        });
        return form;
    }

    // Main plugin method
    $.fn.molify = function (options) {
        // Default settings
        var settings = $.extend({
            backgroundColor: '#fff',
        }, options);

        // Set up the hud
        function setup_hud(viewer) {
            viewer.empty();
            var hud = $('<div>').addClass('hud');
            var loading = $('<div>').addClass('loading').text('Loading molecular data...');
            var spinner = $('<div>').addClass('spinner');
            loading.prepend(spinner)
            hud.append(loading);
            viewer.append(hud);
            if (Spinners) {
                Spinners.create(spinner);
                Spinners.play(spinner);
            }
        }

        // Set up a macromolecular viewer
        function setup_macro_viewer(viewer) {
            // Create all the required elements
            var canvas = $('<canvas>');
            var hud = $('.hud', viewer).empty();
            var lower = $('<div>').addClass('lower');
            var controls = $('<div>').addClass('controls');

            var repr = create_radio_form('repr', [['Stick', 'Sticks'],
                                                  ['van der Waals Spheres', 'van der Waals spheres'],
                                                  ['Ball and Stick', 'Balls and sticks'],
                                                  ['Wireframe', 'Wireframe']]);
            var chain = create_radio_form('chain', [['ribbons', 'Ribbons'],
                                                    ['pipes', 'Pipes and planks'],
                                                    ['backbone', 'Backbone'],
                                                    ['atoms', 'Atoms']]);
            var color = create_radio_form('color', [['none', 'Default colouring'],
                                                    ['amino', 'Colour by amino acid'],
                                                    ['shapely', 'Shapely colouring'],
                                                    ['polarity', 'Colour by polarity'],
                                                    ['acidity', 'Colour by acidity'],
                                                    ['rainbow', 'Rainbow colouring'],
                                                    ['chain', 'Colour by chain']]);
            var misc = create_checkbox_form([['water', 'Show/hide water']]);

            viewer.uniqueId()
                  .prepend(canvas.uniqueId());
            hud.append(lower);
            lower.append(controls);
            controls.append(repr)
                    .append(chain)
                    .append(color)
                    .append(misc);

            // Set up the canvas as a 3D tranformable canvas
            var chemDoodleCanvas = new ChemDoodle.TransformCanvas3D(canvas.attr('id'), canvas.width(), canvas.height());
            canvas.data('canvas', chemDoodleCanvas);

            // Visualisation specs
            var specularColor = '#222';
            chemDoodleCanvas.specs.atoms_materialSpecularColor_3D = specularColor;
            chemDoodleCanvas.specs.bonds_materialSpecularColor_3D = specularColor;
            chemDoodleCanvas.specs.proteins_materialSpecularColor_3D = specularColor;
            chemDoodleCanvas.specs.nucleics_materialSpecularColor_3D = specularColor;
            chemDoodleCanvas.specs.surfaces_materialSpecularColor_3D = specularColor;
            chemDoodleCanvas.specs.backgroundColor = settings.backgroundColor;

            function applyConfig(key, value) {
                if (key == 'repr') {
                    chemDoodleCanvas.specs.set3DRepresentation(value);
                } else if (key == 'chain') {
                    if (value == 'atoms') {
                        chemDoodleCanvas.specs.macro_displayAtoms = true;
                        chemDoodleCanvas.specs.macro_displayBonds = true;
                        chemDoodleCanvas.specs.proteins_displayRibbon = false;
                        chemDoodleCanvas.specs.proteins_ribbonCartoonize = false;
                        chemDoodleCanvas.specs.proteins_displayBackbone = false;
                        chemDoodleCanvas.specs.proteins_displayPipePlank = false;
                    } else if (value == 'ribbons') {
                        chemDoodleCanvas.specs.macro_displayAtoms = false;
                        chemDoodleCanvas.specs.macro_displayBonds = false;
                        chemDoodleCanvas.specs.proteins_displayRibbon = true;
                        chemDoodleCanvas.specs.proteins_ribbonCartoonize = true;
                        chemDoodleCanvas.specs.proteins_displayBackbone = false;
                        chemDoodleCanvas.specs.proteins_displayPipePlank = false;
                    } else if (value == 'backbone') {
                        chemDoodleCanvas.specs.macro_displayAtoms = false;
                        chemDoodleCanvas.specs.macro_displayBonds = false;
                        chemDoodleCanvas.specs.proteins_displayRibbon = false;
                        chemDoodleCanvas.specs.proteins_ribbonCartoonize = false;
                        chemDoodleCanvas.specs.proteins_displayBackbone = true;
                        chemDoodleCanvas.specs.proteins_displayPipePlank = false;
                    } else if (value == 'pipes') {
                        chemDoodleCanvas.specs.macro_displayAtoms = false;
                        chemDoodleCanvas.specs.macro_displayBonds = false;
                        chemDoodleCanvas.specs.proteins_displayRibbon = false;
                        chemDoodleCanvas.specs.proteins_ribbonCartoonize = false;
                        chemDoodleCanvas.specs.proteins_displayBackbone = false;
                        chemDoodleCanvas.specs.proteins_displayPipePlank = true;

                    }
                } else if (key == 'color') {
                    if (value == 'chain') {
                        chemDoodleCanvas.specs.macro_colorByChain = true;
                        chemDoodleCanvas.specs.proteins_residueColor = 'none';
                    } else {
                        chemDoodleCanvas.specs.macro_colorByChain = false;
                        chemDoodleCanvas.specs.proteins_residueColor = value;
                    }
                } else if (key == 'water') {
                    chemDoodleCanvas.specs.macro_showWater = value;
                }
                chemDoodleCanvas.repaint();
            }

            var forms = viewer.find('.controls form');
            var inputs = forms.find('input');
            var radios = inputs.filter('[type="radio"]');
            var checkboxes = inputs.filter('[type="checkbox"]');

            // Any radio-button forms should revert when
            forms.on('mouseleave', function (e) {
                var checked = $(this).find('input[type="radio"]:checked');
                if (checked.length == 1) {
                    applyConfig(checked.attr('name'), checked.attr('value'));
                }
            });

            // Make sure mouse enter events apply radio styles
            radios.next('label').on('mouseenter', function (e) {
                var input = $(this).prev();
                if (e.shiftKey) {
                    applyConfig(input.attr('name'), input.attr('value'));
                }
            });

            // Make sure mouse click events apply checkbox / radio styles
            checkboxes.on('click', function (e) {
                applyConfig($(this).attr('name'), this.checked);
            });
            radios.on('click', function (e) {
                applyConfig($(this).attr('name'), $(this).attr('value'));
            });

            // Defaults
            $('input[name="repr"]', forms).first().trigger('click');
            $('input[name="chain"]', forms).first().trigger('click');
            $('input[name="color"]', forms).first().trigger('click');
        }

        return this.filter('.'+cls_viewer+':not([data-live])').each(function () {
            var ctx = {
                // The viewer element
                viewer: $(this),
                // Compute the URL of the data
                getUrl: function () {
                    // Start by trying to work out what the URL would be
                    var url = settings.url || this.viewer.data('url') || (this.viewer.data('data') && this.viewer.data('data').url);
                    if (!url) {
                        // Generate a URL according to the database and id
                        var db = settings.db || this.viewer.data('db') || (this.viewer.data('data') && this.viewer.data('data').db);
                        var id = settings.id || this.viewer.data('id') || (this.viewer.data('data') && this.viewer.data('data').id);
                        if (id && !db) {
                            var parts = id.split(':');
                            db = parts.shift();
                            id = parts.join(':');
                        }
                        if (db == 'pdb' && id) {
                            url = 'http://www.rcsb.org/pdb/download/downloadFile.do?fileFormat=pdb&compression=NO&structureId='+id;
                        }
                    }
                    return url;
                },
                // Load data
                load: function () {
                    // PDB requires a macromolecular viewer
                    setup_macro_viewer(ctx.viewer);
                    // Parse data
                    var bytes = ctx.viewer.data('data').bytes;
                    var parser = new ChemDoodle.io.PDBInterpreter();
                    parser.deduceResidueBonds = true;
                    ctx.molecule = parser.read(bytes);
                    ctx.viewer.data('live', true).find('canvas').each(function () {
                        $(this).css({width:'100%'}).data('canvas').loadMolecule(ctx.molecule);
                    });
                },
                // Parsed molecule
                molecule: null,
                // Set up the UI
                setup: function () {
                    // Setup the hud with a spinner
                    setup_hud(ctx.viewer);
                },
                // Fetch then load the data
                fetch: function () {
                    if (typeof ctx.viewer.data('data') === "undefined") {
                        ctx.viewer.data('data', {});
                    }
                    // If we already have data, no need to fetch it
                    if (typeof ctx.viewer.data('data').bytes === "undefined") {
                        // Fetch the data from the url
                        $.ajax({
                            url: this.getUrl(),
                            success: function (data) {
                                ctx.viewer.data('data').bytes = data;
                                ctx.load();
                            },
                            error: function () {
                                var loading = $('.loading', ctx.viewer);
                                loading.addClass('error');
                                loading.text('Something went wrong fetching the data!');
                                loading.append($('<br>'));
                                loading.append($('<a href="#">').on('click', function (e) {
                                    e.preventDefault();
                                    ctx.fetch();
                                }).text('Try again...'));
                            },
                        });
                    } else {
                        ctx.load();
                    }
                }
            };

            ctx.viewer.data('ctx', ctx); // For easier debug

            ctx.setup();
            ctx.fetch();
        });
    };

    $(function () {
        // Register a content polisher with Papyro if it exists
        if (typeof utopia == 'object' && typeof utopia.registerPolisher == 'function') {
            utopia.registerPolisher(function () {
                $('.molecule-viewer[data-id]:not([data-live])', $(this)).molify();
            });
        }
    });
}(jQuery));

function molify(selector, data) {
    var viewer = jQuery(selector);
    viewer.data('data', data);
    viewer.molify();
}
