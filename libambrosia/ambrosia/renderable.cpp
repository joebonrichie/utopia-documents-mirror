/*****************************************************************************
 *  
 *   This file is part of the Utopia Documents application.
 *       Copyright (c) 2008-2014 Lost Island Labs
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

#include <ambrosia/renderable.h>
#include <utopia2/extensionlibrary.h>
#include <utopia2/global.h>
#include <map>

namespace AMBROSIA {

    /** Query available render styles. */
    std::list< token > RenderableManager::v2_render_styles() const
    {
        return this->_v2_render_styles;
    }

    /** Query available render options. */
    std::list< token > RenderableManager::v2_render_options() const
    {
        return this->_v2_render_options;
    }


    /** Constructor. */
    Renderable::Renderable()
    {
        // Register renderable name
        this->_v2_renderable_name = _v2_next_renderable_name++;
        _v2_renderables[this->_v2_renderable_name] = this;
    }

    /** Show renderable. */
    void Renderable::v2_set_visibility(bool visibility_)
    {
        if (this->_v2_visible != visibility_)
        {
            this->_v2_visible = visibility_;
            this->v2_set_dirty();
        }
    }

    /** Query renderable visibility. */
    bool Renderable::v2_visibility() const
    {
        return this->_v2_visible;
    }

    /** Set colour. */
    void Renderable::v2_set_colour(Colour* colour_)
    {
        if (this->_v2_colour != colour_)
        {
            this->_v2_colour = colour_;
            this->v2_set_dirty();
        }
    }

    /** Set colour. */
    Colour* Renderable::v2_colour() const
    {
        return this->_v2_colour;
    }

    /** Set tint. */
    void Renderable::v2_set_tint(Colour* tint_)
    {
        if (this->_v2_tint != tint_)
        {
            this->_v2_tint = tint_;
            this->v2_set_dirty();
        }
    }

    /** Set tint. */
    Colour* Renderable::v2_tint() const
    {
        return this->_v2_tint;
    }

    /** Set alpha transparency. */
    void Renderable::v2_set_alpha(unsigned char alpha_)
    {
        if (this->_v2_alpha != alpha_)
        {
            this->_v2_alpha = alpha_;
            this->v2_set_dirty();
        }
    }

    /** Set alpha transparency. */
    unsigned char Renderable::v2_alpha() const
    {
        return this->_v2_alpha;
    }

    /** Set rendering style for this renderable. */
    void Renderable::v2_set_render_style(unsigned int style_)
    {
        if (this->_v2_render_style != style_)
        {
            this->_v2_render_style = style_;
            this->v2_set_dirty();
        }
    }

    /** Query rendering style. */
    unsigned int Renderable::v2_render_style() const
    {
        return this->_v2_render_style;
    }

    /** Set rendering pass for this renderable. */
    void Renderable::v2_set_render_pass(unsigned int pass_)
    {
        if (this->_v2_render_pass != pass_)
        {
            this->_v2_render_pass = pass_;
            this->v2_set_dirty();
        }
    }

    /** Query rendering pass. */
    unsigned int Renderable::v2_render_pass() const
    {
        return this->_v2_render_pass;
    }

    /** Set render option. */
    void Renderable::v2_set_render_option(unsigned int option_)
    {
        if (!this->v2_has_render_option(option_))
        {
            this->_v2_render_options.insert(option_);
            this->v2_set_dirty();
        }
    }

    /** Unset render option. */
    void Renderable::v2_unset_render_option(unsigned int option_)
    {
        if (this->v2_has_render_option(option_))
        {
            this->_v2_render_options.erase(option_);
            this->v2_set_dirty();
        }
    }

    /** Query render option. */
    bool Renderable::v2_has_render_option(unsigned int option_) const
    {
        return (this->_v2_render_options.find(option_) != this->_v2_render_options.end());
    }

    /** Set renderable tag. */
    void Renderable::v2_set_tag(unsigned int tag_)
    {
        if (!this->v2_has_tag(tag_))
        {
            this->_v2_tags |= tag_;
            this->v2_set_dirty();
        }
    }

    /** Unset renderable tag. */
    void Renderable::v2_unset_tag(unsigned int tag_)
    {
        if (this->v2_has_tag(tag_))
        {
            this->_v2_tags |= ~tag_;
            this->v2_set_dirty();
        }
    }

    /** Query for renderable tag. */
    bool Renderable::v2_has_tag(unsigned int tag_) const
    {
        return (this->_v2_tags & tag_) > 0;
    }

    /** Query for renderable tag match. */
    bool Renderable::v2_matches_tags(unsigned int tags_) const
    {
        return (this->_v2_tags & tags_) == this->_v2_tags;
    }

    /** Set this renderable to be dirty (requires reprocessing before next frame). */
    void Renderable::v2_set_dirty()
    {
        this->_v2_dirty = true;
    }

    /** Query dirtiness of this renderable. */
    bool Renderable::v2_dirty() const
    {
        return this->_v2_dirty;
    }

    /** Get renderable from its name. */
    Renderable* Renderable::v2_get_from_name(unsigned int name_)
    {
        if (_v2_renderables.find(name_) == _v2_renderables.end())
        {
            return 0;
        }
        else
        {
            return _v2_renderables[name_];
        }
    }

    std::map< unsigned int, Renderable* > Renderable::_v2_renderables;
    unsigned int Renderable::_v2_next_renderable_name = 1;

    // Plugin loading functions
    RenderableManager * getRenderableManager(std::string entity, std::string suffix)
    {
        qDebug() << "Attempting to load" << QString::fromStdString(entity + "_" + suffix);
        RenderableManager* man = Utopia::instantiateExtension< AMBROSIA::RenderableManager >(entity + "_" + suffix);
        qDebug() << "Man =" << man;
        return man;
    }

} // namespace AMBROSIA

UTOPIA_DEFINE_EXTENSION_CLASS(AMBROSIA::RenderableManager)
