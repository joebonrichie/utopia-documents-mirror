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

#ifndef AMBROSIA_RENDERABLE_H
#define AMBROSIA_RENDERABLE_H

#include <ambrosia/config.h>

#include <ambrosia/ambrosia.h>
#include <utopia2/extension.h>
#include <ambrosia/token.h>

#include <gtl/vertexbuffer.h>

#include <QString>
#include <set>

namespace AMBROSIA {

    class Renderable;

    //
    // RenderableManager class
    //

    class LIBAMBROSIA_API RenderableManager {

    public:
        typedef RenderableManager API;

        // Destructor
        virtual ~RenderableManager() {};

        // Management methods
        virtual Renderable * create(void *) = 0;
        virtual void destroy(Renderable *) = 0;
        virtual Renderable * get(void *) = 0;
        virtual void clear() = 0;

        // Render methods
        virtual void setLOD(unsigned int = 0) = 0;
        virtual void render(Ambrosia::RenderPass = Ambrosia::DRAW_PASS) = 0;
        virtual bool requiresRedraw() = 0;

        // Discovery methods
        virtual unsigned int * getRenderFormats() = 0;
        virtual unsigned int * getRenderOptions() = 0;

        // v2 Discovery methods
        virtual std::list< token > v2_render_styles() const;
        virtual std::list< token > v2_render_options() const;

    protected:
        // v2 Members
        std::list< token > _v2_render_styles;
        std::list< token > _v2_render_options;

    }; // class RenderableManager

    //
    // Renderable class
    //

    class LIBAMBROSIA_API Renderable {

    public:
        // Convenience
        typedef gtl::logical_vertex_buffer< gtl::physical_vertex_buffer< gtl::color4ub, gtl::normal3f, gtl::vertex3f > > _BufferType;

        // Constructor
        Renderable();
        // Destructor
        virtual ~Renderable() {};

        // General methods
        virtual void setDisplay(bool = true) = 0;
        virtual void setVisible(bool = true) = 0;
        virtual void setRenderFormat(unsigned int) = 0;
        virtual void setRenderOption(unsigned int, bool = true) = 0;
        virtual void setColour(Colour *) = 0;
        virtual void setAlpha(unsigned char) = 0;
        virtual void setTintColour(Colour *) = 0;
        virtual void setHighlightColour(Colour *) = 0;
        virtual void setTag(unsigned int) = 0;
        virtual bool hasTag(unsigned int) = 0;

        // Version 2 methods
        virtual void v2_set_visibility(bool show_ = true);
        virtual bool v2_visibility() const;
        virtual void v2_set_colour(Colour* colour_);
        virtual Colour* v2_colour() const;
        virtual void v2_set_tint(Colour* colour_);
        virtual Colour* v2_tint() const;
        virtual void v2_set_alpha(unsigned char alpha_ = 0xFF);
        virtual unsigned char v2_alpha() const;
        virtual void v2_set_render_style(unsigned int style_);
        virtual unsigned int v2_render_style() const;
        virtual void v2_set_render_pass(unsigned int pass_);
        virtual unsigned int v2_render_pass() const;
        virtual void v2_set_render_option(unsigned int option_);
        virtual void v2_unset_render_option(unsigned int option_);
        virtual bool v2_has_render_option(unsigned int option_) const;
        virtual void v2_set_tag(unsigned int tag_);
        virtual void v2_unset_tag(unsigned int tag_);
        virtual bool v2_has_tag(unsigned int tag_) const;
        virtual bool v2_matches_tags(unsigned int tags_) const;
        virtual void v2_set_dirty();
        virtual bool v2_dirty() const;

        // Version 2 abstract methods
        virtual void v2_build_buffer() = 0;

        // OpenGL methods
        virtual void render(Ambrosia::RenderPass = Ambrosia::DRAW_PASS, unsigned int = Buffer::ALL) = 0;

        // Static v2 methods
        static Renderable* v2_get_from_name(unsigned int name_);

    protected:
        // Version 2 members
        bool _v2_visible;
        Colour* _v2_colour;
        Colour* _v2_tint;
        unsigned int _v2_alpha;
        unsigned int _v2_render_style;
        unsigned int _v2_render_pass;
        std::set< unsigned int > _v2_render_options;
        unsigned int _v2_tags;
        bool _v2_dirty;
        _BufferType* _v2_buffer;
        gtl::extent_ui _v2_buffer_span;
        unsigned int _v2_renderable_name;

        // Static v2 members
        static std::map< unsigned int, Renderable* > _v2_renderables;
        static unsigned int _v2_next_renderable_name;

    }; // class Renderable

    // Plugin loading functions
    LIBAMBROSIA_EXPORT RenderableManager * getRenderableManager(std::string, std::string = "basic");

} // namespace AMBROSIA

UTOPIA_DECLARE_EXTENSION_CLASS(LIBAMBROSIA, AMBROSIA::RenderableManager)

#endif // AMBROSIA_RENDERER_H
