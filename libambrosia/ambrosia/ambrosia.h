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

#ifndef AMBROSIA_AMBROSIA_H
#define AMBROSIA_AMBROSIA_H

#include <ambrosia/config.h>
#include <ambrosia/buffer.h>
#include <ambrosia/buffermanager.h>
#include <ambrosia/shader.h>
#include <ambrosia/colour.h>
#include <ambrosia/selection.h>
#include <utopia2/utopia2.h>
#include <list>
#include <map>
using namespace std;

namespace AMBROSIA {

    class Atom;
    class Bond;
    class RenderableManager;
    class Command;

    //
    // Ambrosia class
    //

    class LIBAMBROSIA_API Ambrosia {

    public:
        // typedefs
        // Render passes
        typedef enum {
            SHADOW_MAP_PASS = 0,
            STENCIL_PASS,
            DRAW_PASS,
            DEPTH_SHADE_PASS,
            DRAW_SHADE_PASS,
            DEPTH_TRANSPARENT_PASS,
            DRAW_TRANSPARENT_PASS,
            DRAW_OUTLINE_PASS,
            NAME_PASS
        } RenderPass;
        // Rendering tags
        typedef enum {
            SOLID = 0,
            SHADE,
            Am_TRANSPARENT,
            OUTLINE
        } RenderTag;
        // Rendering options
        typedef enum {
            SPECULAR = 0,
            SHADOWS,
            RENDEROPTIONS
        } RenderOption;
        // Rendering selections
        typedef enum {
            ALL = 0,
            ATOMS,
            BONDS,
            BACKBONE,
            SIDECHAIN,
            AMINOACIDS,
            NUCLEOTIDES,
            RESIDUES,
            CHAINS,
            PROTEINS,
            NUCLEICACIDS,
            HYDROGENS,
            HETEROGENS,
            WATER,
            METALS,
            SULPHUR,
            TEMP,
            CUSTOM
        } RenderSelection;

        // Constructors
        Ambrosia();
        Ambrosia(string);
        Ambrosia(Utopia::Node *);
        // Destructor
        ~Ambrosia();

        // Model loading methods
        bool load(string);
        bool load(Utopia::Node *);
        Utopia::Node * getComplex();
        void clear();
        void setAutoDelete(bool);
        float getRadius();
        void incRefCount();
        void decRefCount();
        unsigned int getRefCount();
        void build();
        bool built();

        // GL methods
        void name();
        void render();
        void render(RenderPass);

        // Render methods
        void setDisplay(bool, RenderSelection = ALL, Selection * = 0);
        void setVisible(bool, RenderSelection = ALL, Selection * = 0);
        void setRenderFormat(unsigned int, RenderSelection = ALL, Selection * = 0);
        void setRenderOption(unsigned int, bool, RenderSelection = ALL, Selection * = 0);
        void setColour(Colour *, RenderSelection = ALL, Selection * = 0);
        void setAlpha(unsigned char, RenderSelection = ALL, Selection * = 0);
        void setTintColour(Colour *, RenderSelection = ALL, Selection * = 0);
        void setHighlightColour(Colour *, RenderSelection = ALL, Selection * = 0);
        void setRenderTag(RenderTag, RenderSelection = ALL, Selection * = 0);
        void setBackground(Colour * = 0);

        // Render options
        void enable(RenderOption);
        void disable(RenderOption);
        void enable(RenderOption, bool);
        bool isEnabled(RenderOption);

        // Token methods
        static unsigned int getToken(string, string);
        static map< string, unsigned int > getTokens(string);
        static string getTokenName(unsigned int);
        static string getTokenClass(unsigned int);

    private:
        // Model
        Utopia::Node * complex;
        float x;
        float y;
        float z;
        float r;
        float lod;
        bool autoDelete;
        unsigned int refCount;
        bool _built;

        // Renderables
        RenderableManager * atomRenderableManager;
        RenderableManager * chainRenderableManager;

        // Render options
        bool options[RENDEROPTIONS];

        // Default selections
        map< RenderSelection, Selection > selections;

        // Internal methods
        void init();
        void orient();
        Selection & getSelection(RenderSelection);
        void applyCommand(Command *, Utopia::Node *);
        void applyCommand(Command *, RenderSelection, Selection *);

        // Tokens
        static map< string, map< string, unsigned int > > tokens;
        static unsigned int nextToken;

    }; // class Ambrosia

    // General functions
    LIBAMBROSIA_EXPORT string versionString();

} // namespace AMBROSIA

#endif // AMBROSIA_AMBROSIA_H
