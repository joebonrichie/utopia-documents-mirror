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
#include <cmath>
#include <set>
#include "ambrosia/utils.h"

#include <gtl/extrusion.h>
#include <gtl/vector.h>
#include <gtl/functional.h>
#include <vector>
#include <functional>

#include <utopia2/extension.h>
#include <utopia2/extensionlibrary.h>

#define MIN_SEPERATION 3.4

namespace AMBROSIA {

    /**
     *  \class  SecStr
     *  \brief  Encapsulates the scaling of secondary structure ribbons.
     */
    class SecStr : public std::unary_function< float, gtl::vector_2f >
    {
        // Convenience typedefs
        typedef SecStr _Self;
        typedef std::unary_function< float, gtl::vector_2f > _Base;

    public:
        // Convenience typedefs
        typedef _Base::argument_type argument_type;
        typedef _Base::result_type result_type;

        /**  Default constructor.  */
        SecStr()
            {}

        /**  Add sheet.  */
        void addSheet(const gtl::extent< argument_type > & sheet_)
            { this->_sheets.push_back(sheet_); }

        /**  Add helix.  */
        void addHelix(const gtl::extent< argument_type > & helix_)
            { this->_helices.push_back(helix_); }

        /**  Clear secondary structure.  */
        void clear()
            {
                this->_helices.clear();
                this->_sheets.clear();
            }

        /**  Function operator.  */
        result_type operator () (const argument_type & x_) const
            {
                using std::cos;

                // SecStrs
                std::vector< gtl::extent< argument_type > >::const_iterator iter = this->_sheets.begin();
                std::vector< gtl::extent< argument_type > >::const_iterator end = this->_sheets.end();
                for (; iter != end; ++iter) {
                    const gtl::extent< argument_type > sheet = *iter;
                    if (sheet.includes(x_)) {
                        if (sheet.max() - sheet.min() > 0.1) {
                            // if the sheet is longer that 1 unit...
                            if (sheet.max() - x_ <= 0.4) {
                                return result_type(1.0 + 4.5 * (1.0 + cos(2.5 * (0.4 - (sheet.max() - x_)) * M_PI)), 1.0);
                            } else if (x_ - sheet.min() <= 0.4) {
                                return result_type(1.0 + 4.5 * (1.0 + cos(2.5 * (0.4 - (x_ - sheet.min())) * M_PI)), 1.0);
                            } else {
                                return result_type(10.0, 1.0);
                            }
                        } else {
                            return result_type(10.0, 1.0);
                        }
                    }
                }

                // Helices
                iter = this->_helices.begin();
                end = this->_helices.end();
                for (; iter != end; ++iter) {
                    const gtl::extent< argument_type > helix = *iter;
                    if (helix.includes(x_)) {
                        if (helix.max() - helix.min() > 0.1) {
                            // if the helix is longer that 1 unit...
                            if (helix.max() - x_ <= 0.8) {
                                return result_type(1.0 + 4.5 * (1.0 + cos(1.25 * (0.8 - (helix.max() - x_)) * M_PI)), 1.0);
                            } else if (x_ - helix.min() <= 0.8) {
                                return result_type(1.0 + 4.5 * (1.0 + cos(1.25 * (0.8 - (x_ - helix.min())) * M_PI)), 1.0);
                            } else {
                                return result_type(10.0, 1.0);
                            }
                        } else {
                            return result_type(10.0, 1.0);
                        }
                    }
                }

                // Default (no scaling)
                return result_type(1.0);
            }

    private:
        std::vector< gtl::extent< argument_type > > _sheets;
        std::vector< gtl::extent< argument_type > > _helices;

    }; /* class SecStr */

    class ChainRenderableManager;
    class ChainRenderable;
    class ResidueRenderable;

    //
    // ResidueRenderableManager class
    //

    class ResidueRenderableManager : public RenderableManager {

    public:
        // Constructor
        ResidueRenderableManager(ChainRenderable *);
        // Destructor
        ~ResidueRenderableManager();

        // Management methods
        virtual Renderable * create(void *);
        virtual void destroy(Renderable *);
        virtual Renderable * get(void *);
        virtual void clear();

        // Render methods
        virtual void setLOD(unsigned int = 0);
        virtual void render(Ambrosia::RenderPass = Ambrosia::DRAW_PASS);
        virtual bool requiresRedraw();

        // Discovery methods
        virtual unsigned int * getRenderFormats();
        virtual unsigned int * getRenderOptions();

        // Manager
        ChainRenderable * chainRenderable;

        // Buffering methods
        Buffer * getBuffer(unsigned int, unsigned int, unsigned int, unsigned int);
        void invalidateBuffers();
        void rebuildBuffers();

        // Render Formats
        unsigned int & BACKBONE;
        unsigned int & CARTOON;
        unsigned int & RIBBONS;
        // Render Options
        unsigned int & SMOOTH;
        unsigned int & CHUNKY;

        // LOD
        unsigned int lod;

        // Shaders
        ShaderProgram * & specularShader;

        // Vertex buffers
        map< unsigned int, map< unsigned int, map< unsigned int, BufferManager * > > > bufferManagers;
        bool validBuffers;

        // Renderables managed by this manager
        map< Utopia::Node *, ResidueRenderable * > renderables;

    }; // class ResidueRenderableManager

    //
    // ChainRenderable class
    //

    class ResidueRenderable : public Renderable {

    public:
        // Convenience
        typedef Renderable::_BufferType _BufferType;

        // Constructor
        ResidueRenderable(Utopia::Node *, RenderableManager *, float);
        // Destructor
        ~ResidueRenderable();

        // General methods
        Utopia::Node * getData();
        virtual void setDisplay(bool = true);
        virtual void setVisible(bool = true);
        virtual void setRenderFormat(unsigned int);
        virtual void setRenderOption(unsigned int, bool = true);
        virtual void setColour(Colour * = 0);
        virtual void setAlpha(unsigned char = 100);
        virtual void setTintColour(Colour * = 0);
        virtual void setHighlightColour(Colour * = 0);
        virtual void setTag(unsigned int);
        virtual bool hasTag(unsigned int);

        // Version 2 methods
        virtual void v2_build_buffer();

        // OpenGL methods
        virtual void render(Ambrosia::RenderPass = Ambrosia::DRAW_PASS, unsigned int = Buffer::ALL);

        // Buffer methods
        void populateBuffer();
        gtl::extrusion< gtl::twine_3f, gtl::PartialCentripetalUpVector > * extrusion;
        SecStr * secstr;
        float value;
        gtl::vector_3f xyzStartNormal;
        gtl::vector_3f xyzNormalNext;

        // Model
        Utopia::Node * residue;

        // Render members
        bool display;
        bool visible;
        Colour * colour;
        unsigned char alpha;
        Colour * tintColour;
        Colour * highlightColour;
        unsigned int renderFormat;
        set< unsigned int > renderOptions;
        unsigned int tag;
        unsigned int vertexCount();

        // Vertex buffer
        Buffer * buffer;
        unsigned int bufferIndex;

        // Manager
        ResidueRenderableManager * renderableManager;

    }; // class ResidueRenderable

    //
    // ChainRenderableManager class
    //

    class ChainRenderableManager : public RenderableManager {

    public:
        // Constructor
        ChainRenderableManager();
        // Destructor
        ~ChainRenderableManager();

        // Management methods
        virtual Renderable * create(void *);
        virtual void destroy(Renderable *);
        virtual Renderable * get(void *);
        virtual void clear();

        // Render methods
        virtual void setLOD(unsigned int = 0);
        virtual void render(Ambrosia::RenderPass = Ambrosia::DRAW_PASS);
        virtual bool requiresRedraw();

        // Discovery methods
        virtual unsigned int * getRenderFormats();
        virtual unsigned int * getRenderOptions();

        // Buffering methods
        Buffer * getBuffer(unsigned int, unsigned int, unsigned int, unsigned int);
        void invalidateBuffers();
        void rebuildBuffers();

        // Render Formats
        unsigned int BACKBONE;
        unsigned int CARTOON;
        unsigned int RIBBONS;
        // Render Options
        unsigned int SMOOTH;
        unsigned int CHUNKY;

        // LOD
        unsigned int lod;
        float * circle;

        // Shaders
        ShaderProgram * specularShader;

        // Render members
        unsigned int * renderFormats;
        unsigned int * renderOptions;

        // Vertex buffers
        map< unsigned int, map< unsigned int, map< unsigned int, BufferManager * > > > bufferManagers;
        bool validBuffers;

        // Renderables managed by this manager
        map< Utopia::Node *, ChainRenderable * > renderables;

    }; // class ChainRenderableManager

    //
    // ChainRenderable class
    //

    class ChainRenderable : public Renderable {

    public:
        // Convenience
        typedef Renderable::_BufferType _BufferType;

        // Constructor
        ChainRenderable(Utopia::Node *, RenderableManager *);
        // Destructor
        ~ChainRenderable();

        // General methods
        Utopia::Node * getData();
        virtual void setDisplay(bool = true);
        virtual void setVisible(bool = true);
        virtual void setRenderFormat(unsigned int);
        virtual void setRenderOption(unsigned int, bool = true);
        virtual void setColour(Colour * = 0);
        virtual void setAlpha(unsigned char = 100);
        virtual void setTintColour(Colour * = 0);
        virtual void setHighlightColour(Colour * = 0);
        virtual void setTag(unsigned int);
        virtual bool hasTag(unsigned int);

        // Version 2 methods
        virtual void v2_build_buffer();

        // OpenGL methods
        virtual void render(Ambrosia::RenderPass = Ambrosia::DRAW_PASS, unsigned int = Buffer::ALL);

        // Buffer methods
        void populateBuffer();
        std::vector< gtl::extrusion< gtl::twine_3f, gtl::PartialCentripetalUpVector > * > extrusions;
        std::map< string, gtl::extrusion< gtl::twine_3f, gtl::PartialCentripetalUpVector > * > extrusionsMap;
        std::vector< SecStr * > secstrs;
        std::map< string, SecStr * > secstrsMap;
        SecStr secstr;

        // Model
        Utopia::Node * chain;

        // Render members
        bool display;
        bool visible;
        Colour * colour;
        unsigned char alpha;
        Colour * tintColour;
        Colour * highlightColour;
        unsigned int renderFormat;
        set< unsigned int > renderOptions;
        unsigned int tag;
        unsigned int vertexCount();

        // Vertex buffer
        Buffer * buffer;
        unsigned int bufferIndex;

        // Manager
        ChainRenderableManager * renderableManager;
        ResidueRenderableManager residueRenderableManager;

    }; // class ChainRenderable

    // Constructor
    ResidueRenderableManager::ResidueRenderableManager(ChainRenderable * chainRenderable)
        : chainRenderable(chainRenderable), BACKBONE(chainRenderable->renderableManager->BACKBONE), CARTOON(chainRenderable->renderableManager->CARTOON), RIBBONS(chainRenderable->renderableManager->RIBBONS), SMOOTH(chainRenderable->renderableManager->SMOOTH), CHUNKY(chainRenderable->renderableManager->CHUNKY), lod(0), specularShader(chainRenderable->renderableManager->specularShader), validBuffers(false)
    {
//              qDebug() << "ResidueRenderableManager()";

        // Initialise level of detail
        setLOD(10);
    }
    // Destructor
    ResidueRenderableManager::~ResidueRenderableManager()
    {
        // Delete all buffers
        map< unsigned int, map< unsigned int, map< unsigned int, BufferManager * > > >::iterator format_iter = bufferManagers.begin();
        map< unsigned int, map< unsigned int, map< unsigned int, BufferManager * > > >::iterator format_end = bufferManagers.end();
        for (; format_iter != format_end; ++format_iter) {
            map< unsigned int, map< unsigned int, BufferManager * > >::iterator tag_iter = format_iter->second.begin();
            map< unsigned int, map< unsigned int, BufferManager * > >::iterator tag_end = format_iter->second.end();
            for (; tag_iter != tag_end; ++tag_iter) {
                map< unsigned int, BufferManager * >::iterator mode_iter = tag_iter->second.begin();
                map< unsigned int, BufferManager * >::iterator mode_end = tag_iter->second.end();
                for (; mode_iter != mode_end; ++mode_iter)
                    delete mode_iter->second;
            }
        }

        // Delete renderables
        map< Utopia::Node *, ResidueRenderable * >::iterator residue_iter = renderables.begin();
        map< Utopia::Node *, ResidueRenderable * >::iterator residue_end = renderables.end();
        for(; residue_iter != residue_end; ++residue_iter)
            destroy(residue_iter->second);
    }

    // Management methods
    Renderable * ResidueRenderableManager::create(void * data)
    {
//              qDebug() << "ResidueRenderableManager::create()";

        invalidateBuffers();
        return 0;
    }
    void ResidueRenderableManager::destroy(Renderable * renderable)
    {
        ResidueRenderable * residueRenderable = (ResidueRenderable *) renderable;
        if (residueRenderable) {
            if (residueRenderable->buffer) {
                invalidateBuffers();
                residueRenderable->buffer->invalidate();
                residueRenderable->buffer = 0;
            }
            renderables.erase(residueRenderable->getData());
            delete residueRenderable;
        }
    }
    Renderable * ResidueRenderableManager::get(void * data)
    {
        Utopia::Node * residue = (Utopia::Node *) data;
        if (renderables.find(residue) != renderables.end())
            return renderables[residue];
        else
            return 0;
    }
    void ResidueRenderableManager::clear()
    {
        map< Utopia::Node *, ResidueRenderable * >::iterator iter = renderables.begin();
        map< Utopia::Node *, ResidueRenderable * >::iterator end = renderables.end();
        for (; iter != end;) {
            ResidueRenderable * residueRenderable = iter->second;
            ++iter;
            destroy(residueRenderable);
        }
        renderables.clear();
    }

    // Render methods
    void ResidueRenderableManager::setLOD(unsigned int lod)
    {
        // Restrict LOD value
        unsigned int newLod = (lod >= 10) ? lod : 10;

        // If no change, bail
        if (newLod == this->lod)
            return;

        // Set new LOD
        this->lod = newLod;
    }
    void ResidueRenderableManager::render(Ambrosia::RenderPass renderPass)
    {
        if (!validBuffers)
            rebuildBuffers();

        switch (renderPass) {
        case Ambrosia::SHADOW_MAP_PASS:
        case Ambrosia::STENCIL_PASS:
        case Ambrosia::DRAW_OUTLINE_PASS:
            // set shaders
            if (specularShader) specularShader->disable();
            break;
        case Ambrosia::DEPTH_SHADE_PASS:
        case Ambrosia::DEPTH_TRANSPARENT_PASS:
        case Ambrosia::DRAW_SHADE_PASS:
        case Ambrosia::DRAW_TRANSPARENT_PASS:
        case Ambrosia::DRAW_PASS:
        case Ambrosia::NAME_PASS:
            // set shaders
            if (specularShader) specularShader->enable();
            break;
        default:
            break;
        }

        if (renderPass == Ambrosia::DRAW_OUTLINE_PASS || renderPass == Ambrosia::STENCIL_PASS || renderPass == Ambrosia::SHADOW_MAP_PASS) {
            map< Utopia::Node *, ResidueRenderable * >::iterator residue_iter = renderables.begin();
            map< Utopia::Node *, ResidueRenderable * >::iterator residue_end = renderables.end();
            for(; residue_iter != residue_end; ++residue_iter) {
                ResidueRenderable * residue = residue_iter->second;
                if (residue->hasTag(Ambrosia::OUTLINE))
                    residue->render(renderPass);
            }
        } else if (renderPass == Ambrosia::NAME_PASS) {
            map< Utopia::Node *, ResidueRenderable * >::iterator residue_iter = renderables.begin();
            map< Utopia::Node *, ResidueRenderable * >::iterator residue_end = renderables.end();
            for(; residue_iter != residue_end; ++residue_iter) {
                ResidueRenderable * residue = residue_iter->second;
                if (residue->hasTag(Ambrosia::SOLID) || residue->hasTag(Ambrosia::OUTLINE))
                    residue->render(renderPass);
            }
        } else {
            map< unsigned int, map< unsigned int, map< unsigned int, BufferManager * > > >::iterator format_iter = bufferManagers.begin();
            map< unsigned int, map< unsigned int, map< unsigned int, BufferManager * > > >::iterator format_end = bufferManagers.end();
            for (; format_iter != format_end; ++format_iter) {
                map< unsigned int, map< unsigned int, BufferManager * > >::iterator tag_iter = format_iter->second.begin();
                map< unsigned int, map< unsigned int, BufferManager * > >::iterator tag_end = format_iter->second.end();
                for (; tag_iter != tag_end; ++tag_iter) {
                    if ((renderPass == Ambrosia::SHADOW_MAP_PASS || renderPass == Ambrosia::DRAW_PASS)
                        && (tag_iter->first != Ambrosia::SOLID) && (tag_iter->first != Ambrosia::OUTLINE))
                        continue;
                    if ((renderPass == Ambrosia::STENCIL_PASS || renderPass == Ambrosia::DRAW_OUTLINE_PASS || renderPass == Ambrosia::NAME_PASS)
                        && (tag_iter->first != Ambrosia::SOLID && tag_iter->first != Ambrosia::SHADE) && (tag_iter->first != Ambrosia::OUTLINE))
                        continue;
                    if ((renderPass == Ambrosia::DEPTH_SHADE_PASS || renderPass == Ambrosia::DRAW_SHADE_PASS)
                        && (tag_iter->first != Ambrosia::SHADE) && (tag_iter->first != Ambrosia::OUTLINE))
                        continue;
                    if ((renderPass == Ambrosia::DEPTH_TRANSPARENT_PASS || renderPass == Ambrosia::DRAW_TRANSPARENT_PASS)
                        && (tag_iter->first != Ambrosia::Am_TRANSPARENT) && (tag_iter->first != Ambrosia::OUTLINE))
                        continue;
                    map< unsigned int, BufferManager * >::iterator mode_iter = tag_iter->second.begin();
                    map< unsigned int, BufferManager * >::iterator mode_end = tag_iter->second.end();
                    for (; mode_iter != mode_end; ++mode_iter) {
                        unsigned int mode = mode_iter->first;
                        BufferManager * bufferManager = mode_iter->second;
                        bufferManager->render(mode);
                    }
                }
            }
        }

        // disable shaders
        if (specularShader) specularShader->disable();
    }
    bool ResidueRenderableManager::requiresRedraw()
    {
        return false;
    }

    // Discovery methods
    unsigned int * ResidueRenderableManager::getRenderFormats()
    { return 0; }
    unsigned int * ResidueRenderableManager::getRenderOptions()
    { return 0; }

    // Buffering methods
    Buffer * ResidueRenderableManager::getBuffer(unsigned int renderFormat, unsigned int tag, unsigned int mode, unsigned int verticesRequired)
    {
        //std::cerr << renderFormat << " " << tag << " " << mode << std::endl;
        if (bufferManagers[renderFormat][tag].find(mode) == bufferManagers[renderFormat][tag].end())
            bufferManagers[renderFormat][tag][mode] = new BufferManager();
        BufferManager * bufferManager = bufferManagers[renderFormat][tag][mode];
        return bufferManager->getBuffer(verticesRequired);
    }
    void ResidueRenderableManager::invalidateBuffers()
    { validBuffers = false; }
    void ResidueRenderableManager::rebuildBuffers()
    {
        validBuffers = true;

        // Unlink all renderables from invalid buffers...
        map< Utopia::Node *, ResidueRenderable * >::iterator renderable = renderables.begin();
        map< Utopia::Node *, ResidueRenderable * >::iterator renderable_end = renderables.end();
        for (; renderable != renderable_end; ++renderable)
            if (renderable->second->buffer && !renderable->second->buffer->isValid())
                renderable->second->buffer = 0;

        // Delete invalid buffers
        map< unsigned int, map< unsigned int, map< unsigned int, BufferManager * > > >::iterator format_iter = bufferManagers.begin();
        map< unsigned int, map< unsigned int, map< unsigned int, BufferManager * > > >::iterator format_end = bufferManagers.end();
        for (; format_iter != format_end; ++format_iter) {
            map< unsigned int, map< unsigned int, BufferManager * > >::iterator tag_iter = format_iter->second.begin();
            map< unsigned int, map< unsigned int, BufferManager * > >::iterator tag_end = format_iter->second.end();
            for (; tag_iter != tag_end; ++tag_iter) {
                map< unsigned int, BufferManager * >::iterator mode_iter = tag_iter->second.begin();
                map< unsigned int, BufferManager * >::iterator mode_end = tag_iter->second.end();
                for (; mode_iter != mode_end; ++mode_iter) {
                    list< Buffer * > toBeDeleted;
                    list< Buffer * >::iterator buffer = mode_iter->second->buffers.begin();
                    list< Buffer * >::iterator buffer_end = mode_iter->second->buffers.end();
                    for (; buffer != buffer_end; ++buffer) {
                        if (!(*buffer)->isValid())
                            toBeDeleted.push_back(*buffer);
                    }
                    buffer = toBeDeleted.begin();
                    buffer_end = toBeDeleted.end();
                    for (; buffer != buffer_end; ++buffer) {
                        mode_iter->second->erase(*buffer);
                        delete *buffer;
                    }
                }
            }
        }

        // Rebuild buffers of renderables...
        renderable = renderables.begin();
        renderable_end = renderables.end();
        for (; renderable != renderable_end; ++renderable)
            if (renderable->second->visible && renderable->second->display && !renderable->second->buffer)
                renderable->second->populateBuffer();
    }

    // Constructor
    ChainRenderableManager::ChainRenderableManager()
        : lod(0), circle(0), specularShader(0), validBuffers(false)
    {
//              qDebug() << "ChainRenderableManager()";
        // Initialise level of detail
        setLOD(10);

        // Log renderableManager
//              qDebug() << "Ambrosia: rendering chains using basic OpenGL";

        // Populate renderFormats
        renderFormats = new unsigned int[4];
        renderFormats[0] = BACKBONE = Ambrosia::getToken("Render Format", "Backbone Trace");
        renderFormats[1] = 0;
        /* renderFormats[1] = */ CARTOON = Ambrosia::getToken("Render Format", "Cartoon");
        /* renderFormats[2] = */ RIBBONS = Ambrosia::getToken("Render Format", "Ribbons");
        //renderFormats[3] = 0;

        // Populate renderOptions
        renderOptions = new unsigned int[3];
        renderOptions[0] = SMOOTH = Ambrosia::getToken("Render Option", "Smooth Backbones");
        renderOptions[1] = CHUNKY = Ambrosia::getToken("Render Option", "Chunky Backbones");
        renderOptions[2] = 0;

        // Shader stuff
        if (Shader::capability() == Shader::GLSL) {
            specularShader = new ShaderProgram();
            specularShader->addShader(loadShader((Utopia::resource_path() + "ambrosia/glsl/specular.vert").toUtf8().constData(), Shader::VERTEX));
            specularShader->addShader(loadShader((Utopia::resource_path() + "ambrosia/glsl/specular.frag").toUtf8().constData(), Shader::FRAGMENT));
        }
    }
    // Destructor
    ChainRenderableManager::~ChainRenderableManager()
    {
        delete [] renderFormats;
        delete [] renderOptions;

        // Delete all buffers
        map< unsigned int, map< unsigned int, map< unsigned int, BufferManager * > > >::iterator format_iter = bufferManagers.begin();
        map< unsigned int, map< unsigned int, map< unsigned int, BufferManager * > > >::iterator format_end = bufferManagers.end();
        for (; format_iter != format_end; ++format_iter) {
            map< unsigned int, map< unsigned int, BufferManager * > >::iterator tag_iter = format_iter->second.begin();
            map< unsigned int, map< unsigned int, BufferManager * > >::iterator tag_end = format_iter->second.end();
            for (; tag_iter != tag_end; ++tag_iter) {
                map< unsigned int, BufferManager * >::iterator mode_iter = tag_iter->second.begin();
                map< unsigned int, BufferManager * >::iterator mode_end = tag_iter->second.end();
                for (; mode_iter != mode_end; ++mode_iter)
                    delete mode_iter->second;
            }
        }

        // Delete renderables
        map< Utopia::Node *, ChainRenderable * >::iterator chain_iter = renderables.begin();
        map< Utopia::Node *, ChainRenderable * >::iterator chain_end = renderables.end();
        for(; chain_iter != chain_end; ++chain_iter)
            destroy(chain_iter->second);

        // Delete shaders
        if (specularShader)
            delete specularShader;
    }

    // Management methods
    Renderable * ChainRenderableManager::create(void * data)
    {
//              qDebug() << "ChainRenderableManager::create()";
        Utopia::Node * chain = (Utopia::Node *) data;
        ChainRenderable * newRenderable = new ChainRenderable(chain, this);
        renderables[chain] = newRenderable;
        invalidateBuffers();
        return newRenderable;
    }
    void ChainRenderableManager::destroy(Renderable * renderable)
    {
        ChainRenderable * chainRenderable = (ChainRenderable *) renderable;
        if (chainRenderable) {
            if (chainRenderable->buffer) {
                invalidateBuffers();
                chainRenderable->buffer->invalidate();
                chainRenderable->buffer = 0;
            }
            renderables.erase(chainRenderable->getData());
            delete chainRenderable;
        }
    }
    Renderable * ChainRenderableManager::get(void * data)
    {
        Utopia::Node * chain = (Utopia::Node *) data;
        if (chain && chain->type() == Utopia::Node::getNode("chain") && renderables.find(chain) != renderables.end())
            return renderables[chain];
        else {
            Renderable * renderable = 0;
            if (chain && chain->type() == Utopia::Node::getNode("aminoacid")) {
                map< Utopia::Node *, ChainRenderable * >::iterator iter = renderables.begin();
                map< Utopia::Node *, ChainRenderable * >::iterator end = renderables.end();
                for (; iter != end; ++iter) {
                    renderable = iter->second->residueRenderableManager.get(data);
                    if (renderable)
                        return renderable;
                }
            }
            return 0;
        }
    }
    void ChainRenderableManager::clear()
    {
        map< Utopia::Node *, ChainRenderable * >::iterator iter = renderables.begin();
        map< Utopia::Node *, ChainRenderable * >::iterator end = renderables.end();
        for (; iter != end;) {
            ChainRenderable * chainRenderable = iter->second;
            ++iter;

            // delete subordinate renderables
            chainRenderable->residueRenderableManager.clear();

            destroy(chainRenderable);
        }
        renderables.clear();
    }

    // Render methods
    void ChainRenderableManager::setLOD(unsigned int lod)
    {
        map< Utopia::Node *, ChainRenderable * >::iterator iter = renderables.begin();
        map< Utopia::Node *, ChainRenderable * >::iterator end = renderables.end();
        for (; iter != end; ++iter) {
            iter->second->residueRenderableManager.setLOD(lod);
        }

        // Restrict LOD value
        unsigned int newLod = (lod >= 10) ? lod : 10;

        // If no change, bail
        if (newLod == this->lod)
            return;

        // Set new LOD
        this->lod = newLod;

        // Precompute surface of a sphere
        float PI = 3.1415926535;
        int vertexCount = newLod * 2 + 1;
        if (circle != 0)
            delete circle;
        circle = new float[vertexCount * 2];
        float * vertexCursor = circle;
        for (unsigned int a = 0; a <= newLod * 2; a++) {
            float i = ((float) a) / ((float) newLod);
            float angle = PI * i;

            *(vertexCursor++) = sin(angle);
            *(vertexCursor++) = -cos(angle);
        }
    }
    void ChainRenderableManager::render(Ambrosia::RenderPass renderPass)
    {
        if (!validBuffers)
            rebuildBuffers();

        // render residues
        map< Utopia::Node *, ChainRenderable * >::iterator iter = renderables.begin();
        map< Utopia::Node *, ChainRenderable * >::iterator end = renderables.end();
        for (; iter != end; ++iter) {
            iter->second->residueRenderableManager.render(renderPass);;
        }

        switch (renderPass) {
        case Ambrosia::SHADOW_MAP_PASS:
        case Ambrosia::STENCIL_PASS:
        case Ambrosia::DRAW_OUTLINE_PASS:
            // set shaders
            if (specularShader) specularShader->disable();
            break;
        case Ambrosia::DEPTH_SHADE_PASS:
        case Ambrosia::DEPTH_TRANSPARENT_PASS:
        case Ambrosia::DRAW_SHADE_PASS:
        case Ambrosia::DRAW_TRANSPARENT_PASS:
        case Ambrosia::DRAW_PASS:
        case Ambrosia::NAME_PASS:
            // set shaders
            if (specularShader) specularShader->enable();
            break;
        default:
            break;
        }

        if (renderPass == Ambrosia::DRAW_OUTLINE_PASS || renderPass == Ambrosia::STENCIL_PASS || renderPass == Ambrosia::SHADOW_MAP_PASS) {
            map< Utopia::Node *, ChainRenderable * >::iterator chain_iter = renderables.begin();
            map< Utopia::Node *, ChainRenderable * >::iterator chain_end = renderables.end();
            for(; chain_iter != chain_end; ++chain_iter) {
                ChainRenderable * chain = chain_iter->second;
                if (chain->hasTag(Ambrosia::OUTLINE) || chain->hasTag(Ambrosia::OUTLINE))
                    chain->render(renderPass);
            }
        } else if (renderPass == Ambrosia::NAME_PASS) {
            map< Utopia::Node *, ChainRenderable * >::iterator chain_iter = renderables.begin();
            map< Utopia::Node *, ChainRenderable * >::iterator chain_end = renderables.end();
            for(; chain_iter != chain_end; ++chain_iter) {
                ChainRenderable * chain = chain_iter->second;
                if (chain->hasTag(Ambrosia::SOLID))
                    chain->render(renderPass);
            }
        } else {
            map< unsigned int, map< unsigned int, map< unsigned int, BufferManager * > > >::iterator format_iter = bufferManagers.begin();
            map< unsigned int, map< unsigned int, map< unsigned int, BufferManager * > > >::iterator format_end = bufferManagers.end();
            for (; format_iter != format_end; ++format_iter) {
                map< unsigned int, map< unsigned int, BufferManager * > >::iterator tag_iter = format_iter->second.begin();
                map< unsigned int, map< unsigned int, BufferManager * > >::iterator tag_end = format_iter->second.end();
                for (; tag_iter != tag_end; ++tag_iter) {
                    if ((renderPass == Ambrosia::SHADOW_MAP_PASS || renderPass == Ambrosia::DRAW_PASS)
                        && (tag_iter->first != Ambrosia::SOLID) && (tag_iter->first != Ambrosia::OUTLINE))
                        continue;
                    if ((renderPass == Ambrosia::STENCIL_PASS || renderPass == Ambrosia::DRAW_OUTLINE_PASS || renderPass == Ambrosia::NAME_PASS)
                        && (tag_iter->first != Ambrosia::SOLID && tag_iter->first != Ambrosia::SHADE) && (tag_iter->first != Ambrosia::OUTLINE))
                        continue;
                    if ((renderPass == Ambrosia::DEPTH_SHADE_PASS || renderPass == Ambrosia::DRAW_SHADE_PASS)
                        && (tag_iter->first != Ambrosia::SHADE) && (tag_iter->first != Ambrosia::OUTLINE))
                        continue;
                    if ((renderPass == Ambrosia::DEPTH_TRANSPARENT_PASS || renderPass == Ambrosia::DRAW_TRANSPARENT_PASS)
                        && (tag_iter->first != Ambrosia::Am_TRANSPARENT) && (tag_iter->first != Ambrosia::OUTLINE))
                        continue;
                    map< unsigned int, BufferManager * >::iterator mode_iter = tag_iter->second.begin();
                    map< unsigned int, BufferManager * >::iterator mode_end = tag_iter->second.end();
                    for (; mode_iter != mode_end; ++mode_iter) {
                        unsigned int mode = mode_iter->first;
                        BufferManager * bufferManager = mode_iter->second;
                        bufferManager->render(mode);
                    }
                }
            }
        }

        // disable shaders
        if (specularShader) specularShader->disable();
    }
    bool ChainRenderableManager::requiresRedraw()
    {
        return false;
    }

    // Discovery methods
    unsigned int * ChainRenderableManager::getRenderFormats()
    { return renderFormats; }
    unsigned int * ChainRenderableManager::getRenderOptions()
    { return renderOptions; }

    // Buffering methods
    Buffer * ChainRenderableManager::getBuffer(unsigned int renderFormat, unsigned int tag, unsigned int mode, unsigned int verticesRequired)
    {
        if (bufferManagers[renderFormat][tag].find(mode) == bufferManagers[renderFormat][tag].end())
            bufferManagers[renderFormat][tag][mode] = new BufferManager();
        BufferManager * bufferManager = bufferManagers[renderFormat][tag][mode];
        return bufferManager->getBuffer(verticesRequired);
    }
    void ChainRenderableManager::invalidateBuffers()
    {
        map< Utopia::Node *, ChainRenderable * >::iterator iter = renderables.begin();
        map< Utopia::Node *, ChainRenderable * >::iterator end = renderables.end();
        for (; iter != end; ++iter) {
            iter->second->residueRenderableManager.invalidateBuffers();
        }

        validBuffers = false;
    }
    void ChainRenderableManager::rebuildBuffers()
    {
        validBuffers = true;

        // Rebuild buffers of renderables...
        map< Utopia::Node *, ChainRenderable * >::iterator renderable = renderables.begin();
        map< Utopia::Node *, ChainRenderable * >::iterator renderable_end = renderables.end();
        for (; renderable != renderable_end; ++renderable)
            renderable->second->populateBuffer();

//              // Rebuild buffers of subordinate renderables...
//              map< Utopia::Node *, ChainRenderable * >::iterator iter = renderables.begin();
//              map< Utopia::Node *, ChainRenderable * >::iterator end = renderables.end();
//              for (; iter != end; ++iter) {
//                      iter->second->residueRenderableManager.rebuildBuffers();
//              }
    }

    // Constructor
    ResidueRenderable::ResidueRenderable(Utopia::Node * residue, RenderableManager * renderableManager, float value)
        : Renderable(), value(value), residue(residue), display(true), visible(true), alpha(75), tintColour(0), highlightColour(0), tag(Ambrosia::SOLID), buffer(0), bufferIndex(0)
    {
//              qDebug() << "ResidueRenderable()";

        // Default rendering spec
        if (residue->type()->relations(Utopia::rdfs.subClassOf).front() == Utopia::UtopiaDomain.term("AminoAcid")) {
            colour = Colour::getColour(string("residue.") + residue->type()->attributes.get(Utopia::UtopiaDomain.term("abbreviation"), "?").toString().toUpper().toStdString());
        } else if (residue->type()->relations(Utopia::rdfs.subClassOf).front() == Utopia::UtopiaDomain.term("Nucleoside")) {
            colour = Colour::getColour(string("residue.") + residue->type()->attributes.get(Utopia::UtopiaDomain.term("code"), "?").toString().toUpper().toStdString());
        }
        this->renderableManager = (ResidueRenderableManager *) renderableManager;
        renderFormat = this->renderableManager->BACKBONE;
        xyzStartNormal[0] = xyzStartNormal[1] = xyzStartNormal[2] = 0.0;
        setRenderOption(this->renderableManager->CHUNKY);
        setRenderOption(this->renderableManager->SMOOTH);
    }
    // Destructor
    ResidueRenderable::~ResidueRenderable()
    {}

    // General methods
    Utopia::Node * ResidueRenderable::getData()
    { return residue; }
    void ResidueRenderable::setDisplay(bool display)
    {
        if (this->display == display)
            return;
        this->display = display;

        renderableManager->invalidateBuffers();
        if (buffer && visible) {
            buffer->invalidate();
            buffer = 0;
        }
    }
    void ResidueRenderable::setVisible(bool visible)
    {
        if (this->visible == visible)
            return;
        this->visible = visible;

        renderableManager->invalidateBuffers();
        if (buffer && display) {
            buffer->invalidate();
            buffer = 0;
        }
    }
    void ResidueRenderable::setRenderFormat(unsigned int renderFormat)
    {
        if (this->renderFormat == renderFormat)
            return;
        this->renderFormat = renderFormat;

        if (buffer && visible && display) {
            renderableManager->invalidateBuffers();
            buffer->invalidate();
            buffer = 0;
        }
    }
    void ResidueRenderable::setRenderOption(unsigned int renderOption, bool flag)
    {
        if ((renderOptions.find(renderOption) != renderOptions.end()) == flag)
            return;
        if (flag) {
            renderOptions.insert(renderOption);
        } else {
            renderOptions.erase(renderOption);
        }

        if (buffer) { // && visible && display) {
            renderableManager->invalidateBuffers();
            buffer->invalidate();
            buffer = 0;
        }
    }
    void ResidueRenderable::setColour(Colour * colour)
    {
        if (colour == 0 || colour == this->colour)
            return;
        this->colour = colour;

        // reload buffer
        if (buffer && visible && display) {
            populateBuffer();
            buffer->load(bufferIndex, vertexCount());
        }
    }
    void ResidueRenderable::setAlpha(unsigned char alpha)
    {
        if (alpha == this->alpha)
            return;
        this->alpha = alpha;

        // reload buffer
        if (buffer && visible && display) {
            populateBuffer();
            buffer->load(bufferIndex, vertexCount());
        }
    }
    void ResidueRenderable::setTintColour(Colour * colour)
    {
        if (colour == this->tintColour)
            return;
        this->tintColour = colour;

        // reload buffer
        if (buffer && visible && display) {
            populateBuffer();
            buffer->load(bufferIndex, vertexCount());
        }
    }
    void ResidueRenderable::setHighlightColour(Colour * colour)
    {
        this->highlightColour = colour;
    }
    void ResidueRenderable::setTag(unsigned int tag)
    {
        if (this->tag == tag)
            return;
        this->tag = tag;

        if (buffer && visible && display) {
            renderableManager->invalidateBuffers();
            buffer->invalidate();
            buffer = 0;
        }
    }
    bool ResidueRenderable::hasTag(unsigned int tag)
    { return this->tag == tag; }

    /** Rebuild vertex buffer for this renderable. */
    void ResidueRenderable::v2_build_buffer()
    {
        // FIXME
    }

    // OpenGL methods
    void ResidueRenderable::render(Ambrosia::RenderPass renderPass, unsigned int elements)
    {
        if (!(visible && display))
            return;

        if (buffer == 0)
            populateBuffer();

        if (elements & Buffer::COLOUR) {
            switch (renderPass) {
            case Ambrosia::DRAW_OUTLINE_PASS:
                if (renderPass == Ambrosia::DRAW_OUTLINE_PASS && highlightColour) {
                    elements &= ~Buffer::COLOUR;
                    glColor3f(highlightColour->r, highlightColour->g, highlightColour->b);
                }
            case Ambrosia::SHADOW_MAP_PASS:
            case Ambrosia::STENCIL_PASS:
                // set shaders
                if (renderableManager->specularShader) renderableManager->specularShader->disable();
                break;
            case Ambrosia::DEPTH_SHADE_PASS:
            case Ambrosia::DEPTH_TRANSPARENT_PASS:
            case Ambrosia::DRAW_SHADE_PASS:
            case Ambrosia::DRAW_TRANSPARENT_PASS:
            case Ambrosia::DRAW_PASS:
            case Ambrosia::NAME_PASS:
                // set shaders
                if (renderableManager->specularShader) renderableManager->specularShader->enable();
                break;
            default:
                break;
            }
        }

        // Render buffer
        if (renderFormat == renderableManager->BACKBONE || renderFormat == renderableManager->CARTOON || renderFormat == renderableManager->RIBBONS) {
            // Draw buffer
            buffer->enable(elements);
            buffer->render(GL_TRIANGLE_STRIP, bufferIndex, vertexCount());
            buffer->disable();
        }

        // disable shaders
        if (renderableManager->specularShader) renderableManager->specularShader->disable();
    }

    // Buffering methods
    void ResidueRenderable::populateBuffer()
    {
        if (buffer == 0) {
            buffer = renderableManager->getBuffer(renderFormat, tag, GL_TRIANGLE_STRIP, vertexCount());
            bufferIndex = buffer->usedVertices();
        } else {
            buffer->to(bufferIndex);
        }

        unsigned char R = colour->r;
        unsigned char G = colour->g;
        unsigned char B = colour->b;
        unsigned char A = alpha;
        if (tintColour) {
            // Hack for demo
            R = tintColour->r;
            G = tintColour->g;
            B = tintColour->b;
//                      R = R / 2 + tintColour->r / 2;
//                      G = G / 2 + tintColour->g / 2;
//                      B = B / 2 + tintColour->b / 2;
        }

        // Find Secondary Structure if necessary
        string secStr = "";
/*
  if (renderFormat == renderableManager->CARTOON) {
  Utopia::Node::parent_iterator parent_iter = residue->parentsBegin(&UTOPIA::MODEL::isChain);
  Utopia::Node::parent_iterator parent_end = residue->parentsEnd();
  if (parent_iter != parent_end) {
  int length = -1;
  Utopia::Node::relation::iterator residue_iter = (*parent_iter)->childrenBegin(&UTOPIA::MODEL::isResidue);
  Utopia::Node::relation::iterator residue_end = (*parent_iter)->childrenEnd();
  for (; residue_iter != residue_end; ++residue_iter) {
  Utopia::Node::parent_iterator ann_iter = (*residue_iter)->parentsBegin(&UTOPIA::MODEL::isAnnotation);
  Utopia::Node::parent_iterator ann_end = (*residue_iter)->parentsEnd();
  for (; ann_iter != ann_end; ++ann_iter) {
  if ((*ann_iter)->hasAttribute("name") && (*ann_iter)->hasAttribute("class") && (*ann_iter)->hasAttribute("width") && (*ann_iter)->getAttribute<string>("class") == "extent" && ((*ann_iter)->getAttribute<string>("name") == "Helices" || (*ann_iter)->getAttribute<string>("name") == "Sheets" || (*ann_iter)->getAttribute<string>("name") == "Turns")) {
  if ((*(*ann_iter)->parentsBegin())->getAttribute<string>("name") == "PDB") {
  secStr = (*ann_iter)->getAttribute<string>("name");
  length = (*ann_iter)->getAttribute<int>("width");
  }
  }
  }
  --length;
  if (length == 0) {
  secStr = "";
  } else if (*residue_iter == residue) {
  break;
  }
  }
  }
  }
*/
        std::vector< gtl::vector_2f > xsection_vertices;
        std::vector< gtl::vector_2f > xsection_normals;
        for (unsigned int i = 0; i <= renderableManager->lod * 2; ++i) {
            float x = std::cos((M_PI / (float) renderableManager->lod) * (float) i) * 0.15;
            float y = std::sin((M_PI / (float) renderableManager->lod) * (float) i) * 0.15;

            if (renderFormat == renderableManager->BACKBONE && renderableManager->chainRenderable->renderOptions.find(renderableManager->CHUNKY) != renderableManager->chainRenderable->renderOptions.end())
            {
                xsection_vertices.push_back(gtl::vector_2f(x * 6.0, y * 6.0));
            }
            else
            {
                xsection_vertices.push_back(gtl::vector_2f(x, y));
            }
            xsection_normals.push_back(normalise(gtl::vector_2f(x, y)));
        }
        extrusion->vertices(xsection_vertices);
        extrusion->normals(xsection_normals);

        // Compile to buffer
        if (renderFormat == renderableManager->BACKBONE || renderFormat == renderableManager->CARTOON) {
            // holder of vertices and normals
            std::vector< gtl::vector_3f > vertices;
            std::vector< gtl::vector_3f > normals;
            std::vector< gtl::vector_3f > next_vertices;
            std::vector< gtl::vector_3f > next_normals;

            for (unsigned int j = 0; j < renderableManager->lod; ++j) {
                double valueOffset = ((double) j / (double) renderableManager->lod) - 0.5;
                double nextValueOffset = ((double) (j + 1) / (double) renderableManager->lod) - 0.5;

                if (j == 0) {
                    vertices = extrusion->extrapolate_vertices(value + valueOffset, *secstr);
                    normals = extrusion->extrapolate_normals(value + valueOffset, *secstr);

                    gtl::vector_3f normal, centre, next_centre, ignoredY, ignoredX;
                    extrusion->extrapolate(value + valueOffset, centre, ignoredX, ignoredY);
                    extrusion->extrapolate(value + valueOffset + 0.01, next_centre, ignoredX, ignoredY);
                    normal = -normalise(next_centre - centre);

                    buffer->setPosition(centre[0], centre[1], centre[2]);
                    buffer->setNormal(normal[0], normal[1], normal[2]);
                    buffer->setColourb(R, G, B, A);
                    buffer->next();
                    buffer->setPosition(centre[0], centre[1], centre[2]);
                    buffer->setNormal(normal[0], normal[1], normal[2]);
                    buffer->setColourb(R, G, B, A);
                    buffer->next();

                    for (int k = 0; k < vertices.size(); ++k)
                    {
                        buffer->setPosition(centre[0], centre[1], centre[2]);
                        buffer->setNormal(normal[0], normal[1], normal[2]);
                        buffer->setColourb(R, G, B, A);
                        buffer->next();
                        buffer->setPosition(vertices[k][0], vertices[k][1], vertices[k][2]);
                        buffer->setNormal(normal[0], normal[1], normal[2]);
                        buffer->setColourb(R, G, B, A);
                        buffer->next();
                    }

                    buffer->setPosition(vertices[0][0], vertices[0][1], vertices[0][2]);
                    buffer->setNormal(normal[0], normal[1], normal[2]);
                    buffer->setColourb(R, G, B, A);
                    buffer->next();
                    buffer->setPosition(vertices[0][0], vertices[0][1], vertices[0][2]);
                    buffer->setNormal(normal[0], normal[1], normal[2]);
                    buffer->setColourb(R, G, B, A);
                    buffer->next();
                }

                next_vertices = extrusion->extrapolate_vertices(value + nextValueOffset, *secstr);
                next_normals = extrusion->extrapolate_normals(value + nextValueOffset, *secstr);

                unsigned int i = 0;
                for (i = 0; i < vertices.size(); ++i) {
                    buffer->setPosition(vertices[i][0], vertices[i][1], vertices[i][2]);
                    buffer->setNormal(normals[i][0], normals[i][1], normals[i][2]);
                    buffer->setColourb(R, G, B, A);
                    buffer->next();
                    buffer->setPosition(next_vertices[i][0], next_vertices[i][1], next_vertices[i][2]);
                    buffer->setNormal(next_normals[i][0], next_normals[i][1], next_normals[i][2]);
                    buffer->setColourb(R, G, B, A);
                    buffer->next();
                }

                vertices = next_vertices;
                normals = next_normals;

                if (j == renderableManager->lod - 1) {
                    buffer->setPosition(vertices[i - 1][0], vertices[i - 1][1], vertices[i - 1][2]);
                    buffer->setNormal(normals[i - 1][0], normals[i - 1][1], normals[i - 1][2]);
                    buffer->setColourb(R, G, B, A);
                    buffer->next();
                    buffer->setPosition(vertices[i - 1][0], vertices[i - 1][1], vertices[i - 1][2]);
                    buffer->setNormal(normals[i - 1][0], normals[i - 1][1], normals[i - 1][2]);
                    buffer->setColourb(R, G, B, A);
                    buffer->next();

                    gtl::vector_3f normal, centre, next_centre, ignoredY, ignoredX;
                    extrusion->extrapolate(value + valueOffset, centre, ignoredX, ignoredY);
                    extrusion->extrapolate(value + valueOffset - 0.01, next_centre, ignoredX, ignoredY);
                    normal = -normalise(next_centre - centre);

                    for (int k = 0; k < vertices.size(); ++k)
                    {
                        buffer->setPosition(vertices[k][0], vertices[k][1], vertices[k][2]);
                        buffer->setNormal(normal[0], normal[1], normal[2]);
                        buffer->setColourb(R, G, B, A);
                        buffer->next();
                        buffer->setPosition(centre[0], centre[1], centre[2]);
                        buffer->setNormal(normal[0], normal[1], normal[2]);
                        buffer->setColourb(R, G, B, A);
                        buffer->next();
                    }

                    buffer->setPosition(centre[0], centre[1], centre[2]);
                    buffer->setNormal(normal[0], normal[1], normal[2]);
                    buffer->setColourb(R, G, B, A);
                    buffer->next();
                    buffer->setPosition(centre[0], centre[1], centre[2]);
                    buffer->setNormal(normal[0], normal[1], normal[2]);
                    buffer->setColourb(R, G, B, A);
                    buffer->next();
                }
            }
        }
    }

    unsigned int ResidueRenderable::vertexCount()
    {
        // Find Secondary Structure
        string secStr = "";
        /*
          if (renderFormat == renderableManager->CARTOON) {
          Utopia::Node::parent_iterator parent_iter = residue->parentsBegin(&UTOPIA::MODEL::isChain);
          Utopia::Node::parent_iterator parent_end = residue->parentsEnd();
          if (parent_iter != parent_end) {
          int length = -1;
          Utopia::Node::relation::iterator residue_iter = residue->childrenBegin(&UTOPIA::MODEL::isResidue);
          Utopia::Node::relation::iterator residue_end = residue->childrenEnd();
          for (; residue_iter != residue_end; ++residue_iter) {
          Utopia::Node::parent_iterator ann_iter = residue->parentsBegin(&UTOPIA::MODEL::isAnnotation);
          Utopia::Node::parent_iterator ann_end = residue->parentsEnd();
          for (; ann_iter != ann_end; ++ann_iter) {
          if ((*ann_iter)->hasAttribute("name") && (*ann_iter)->hasAttribute("class") && (*ann_iter)->hasAttribute("width") && (*ann_iter)->getAttribute<string>("class") == "extent" && ((*ann_iter)->getAttribute<string>("name") == "Helices" || (*ann_iter)->getAttribute<string>("name") == "Sheets" || (*ann_iter)->getAttribute<string>("name") == "Turns")) {
          if ((*(*ann_iter)->parentsBegin())->getAttribute<string>("name") == "PDB") {
          secStr = (*ann_iter)->getAttribute<string>("name");
          length = (*ann_iter)->getAttribute<int>("width");
          }
          }
          }
          --length;
          if (length == 0) {
          secStr = "";
          } else if (*residue_iter == residue) {
          break;
          }
          }
          }
          }
        */

        if (renderFormat == renderableManager->BACKBONE || (renderFormat == renderableManager->CARTOON && (secStr == "" || secStr == "Turns"))) {
            return 4 * renderableManager->lod * renderableManager->lod + 10 * renderableManager->lod + 8;
        } else if (renderFormat == renderableManager->CARTOON) {
            return 0;
        }
        return 0;
    }

    // Constructor
    ChainRenderable::ChainRenderable(Utopia::Node * chain, RenderableManager * renderableManager)
        : Renderable(), chain(chain), display(true), visible(true), alpha(75), tintColour(0), highlightColour(0), tag(Ambrosia::SOLID), buffer(0), bufferIndex(0), renderableManager((ChainRenderableManager *) renderableManager), residueRenderableManager(this)
    {
//              qDebug() << "ChainRenderable()";

        // Default rendering spec
        colour = Colour::getColour("helix.?");
        renderFormat = this->renderableManager->BACKBONE;
        setRenderOption(this->renderableManager->CHUNKY);
        setRenderOption(this->renderableManager->SMOOTH);
    }
    // Destructor
    ChainRenderable::~ChainRenderable()
    {}

    // General methods
    Utopia::Node * ChainRenderable::getData()
    { return chain; }
    void ChainRenderable::setDisplay(bool display)
    {
        map< Utopia::Node *, ResidueRenderable * >::iterator iter = residueRenderableManager.renderables.begin();
        map< Utopia::Node *, ResidueRenderable * >::iterator end = residueRenderableManager.renderables.end();
        for (; iter != end; ++iter) {
            iter->second->setDisplay(display);
        }
    }
    void ChainRenderable::setVisible(bool visible)
    {
        map< Utopia::Node *, ResidueRenderable * >::iterator iter = residueRenderableManager.renderables.begin();
        map< Utopia::Node *, ResidueRenderable * >::iterator end = residueRenderableManager.renderables.end();
        for (; iter != end; ++iter) {
            iter->second->setVisible(visible);
        }
    }
    void ChainRenderable::setRenderFormat(unsigned int renderFormat)
    {
        map< Utopia::Node *, ResidueRenderable * >::iterator iter = residueRenderableManager.renderables.begin();
        map< Utopia::Node *, ResidueRenderable * >::iterator end = residueRenderableManager.renderables.end();
        for (; iter != end; ++iter) {
            iter->second->setRenderFormat(renderFormat);
        }
        this->renderFormat = renderFormat;
        this->renderableManager->invalidateBuffers();
    }
    void ChainRenderable::setRenderOption(unsigned int renderOption, bool flag)
    {
//              if (renderOption != renderableManager->SMOOTH) {
        map< Utopia::Node *, ResidueRenderable * >::iterator iter = residueRenderableManager.renderables.begin();
        map< Utopia::Node *, ResidueRenderable * >::iterator end = residueRenderableManager.renderables.end();
        for (; iter != end; ++iter) {
            iter->second->setRenderOption(renderOption, flag);
        }
//              } else {
        if (flag) {
            renderOptions.insert(renderOption);
        } else {
            renderOptions.erase(renderOption);
        }
        renderableManager->invalidateBuffers();
//              }
    }
    void ChainRenderable::setColour(Colour * colour)
    {
        map< Utopia::Node *, ResidueRenderable * >::iterator iter = residueRenderableManager.renderables.begin();
        map< Utopia::Node *, ResidueRenderable * >::iterator end = residueRenderableManager.renderables.end();
        for (; iter != end; ++iter) {
            iter->second->setColour(colour);
        }
    }
    void ChainRenderable::setAlpha(unsigned char alpha)
    {
        map< Utopia::Node *, ResidueRenderable * >::iterator iter = residueRenderableManager.renderables.begin();
        map< Utopia::Node *, ResidueRenderable * >::iterator end = residueRenderableManager.renderables.end();
        for (; iter != end; ++iter) {
            iter->second->setAlpha(alpha);
        }
    }
    void ChainRenderable::setTintColour(Colour * colour)
    {
        map< Utopia::Node *, ResidueRenderable * >::iterator iter = residueRenderableManager.renderables.begin();
        map< Utopia::Node *, ResidueRenderable * >::iterator end = residueRenderableManager.renderables.end();
        for (; iter != end; ++iter) {
            iter->second->setTintColour(colour);
        }
    }
    void ChainRenderable::setHighlightColour(Colour * colour)
    {
        map< Utopia::Node *, ResidueRenderable * >::iterator iter = residueRenderableManager.renderables.begin();
        map< Utopia::Node *, ResidueRenderable * >::iterator end = residueRenderableManager.renderables.end();
        for (; iter != end; ++iter) {
            iter->second->setHighlightColour(colour);
        }
    }
    void ChainRenderable::setTag(unsigned int tag)
    {
        map< Utopia::Node *, ResidueRenderable * >::iterator iter = residueRenderableManager.renderables.begin();
        map< Utopia::Node *, ResidueRenderable * >::iterator end = residueRenderableManager.renderables.end();
        for (; iter != end; ++iter) {
            iter->second->setTag(tag);
        }
    }
    bool ChainRenderable::hasTag(unsigned int tag)
    { return this->tag == tag; }

    /** Rebuild vertex buffer for this renderable. */
    void ChainRenderable::v2_build_buffer()
    {
        // FIXME
    }

    // OpenGL methods
    void ChainRenderable::render(Ambrosia::RenderPass renderPass, unsigned int elements)
    {
        if (!(visible && display))
            return;
        return;

        if (buffer == 0)
            populateBuffer();

        map< Utopia::Node *, ResidueRenderable * >::iterator iter = residueRenderableManager.renderables.begin();
        map< Utopia::Node *, ResidueRenderable * >::iterator end = residueRenderableManager.renderables.end();
        for (; iter != end; ++iter) {
            iter->second->render(renderPass, elements);
        }

        if (elements & Buffer::COLOUR) {
            switch (renderPass) {
            case Ambrosia::DRAW_OUTLINE_PASS:
                if (renderPass == Ambrosia::DRAW_OUTLINE_PASS && highlightColour) {
                    elements &= ~Buffer::COLOUR;
                    glColor3f(highlightColour->r, highlightColour->g, highlightColour->b);
                }
            case Ambrosia::SHADOW_MAP_PASS:
            case Ambrosia::STENCIL_PASS:
                // set shaders
                if (renderableManager->specularShader) renderableManager->specularShader->disable();
                break;
            case Ambrosia::DEPTH_SHADE_PASS:
            case Ambrosia::DEPTH_TRANSPARENT_PASS:
            case Ambrosia::DRAW_SHADE_PASS:
            case Ambrosia::DRAW_TRANSPARENT_PASS:
            case Ambrosia::DRAW_PASS:
            case Ambrosia::NAME_PASS:
                // set shaders
                if (renderableManager->specularShader) renderableManager->specularShader->enable();
                break;
            default:
                break;
            }
        }

        // Render buffer
        if (renderFormat == renderableManager->BACKBONE || renderFormat == renderableManager->CARTOON || renderFormat == renderableManager->RIBBONS) {
            // Draw buffer
            buffer->enable(elements);
            buffer->render(GL_TRIANGLE_STRIP, bufferIndex, vertexCount());
            buffer->disable();
        }

        // disable shaders
        if (renderableManager->specularShader) renderableManager->specularShader->disable();
    }

    // Buffering methods
    void ChainRenderable::populateBuffer()
    {
//              qDebug() << "ChainRenderable::populateBuffer()";

        // Extrusions
        extrusions.clear();
        extrusionsMap.clear();
        secstrs.clear();
        secstrsMap.clear();
        gtl::extrusion< gtl::twine_3f, gtl::PartialCentripetalUpVector > * extrusion = 0;
        gtl::extrusion< gtl::twine_3f, gtl::PartialCentripetalUpVector > * prev_extrusion = 0;
        SecStr * secstr = 0;
        gtl::twine_3f path;

        // Get backbone path
        double value = 0.5;
        Utopia::Node * alphaCarbon = 0;
        Utopia::Node * prevAlphaCarbon = 0;
        Utopia::Node * nitrogen = 0;
        Utopia::Node * betaCarbon = 0;
        Utopia::Node * prevBetaCarbon = 0;
        Utopia::Node * nextAlphaCarbon = 0;
        Utopia::Node * nextNextAlphaCarbon = 0;
        Utopia::Node::relation::iterator residue_iter = chain->relations(Utopia::UtopiaSystem.hasPart).begin();
        Utopia::Node::relation::iterator residue_end = chain->relations(Utopia::UtopiaSystem.hasPart).end();
        int helix = 0;

        int length = -1;
        string secStr = "";
        for (; residue_iter != residue_end; ++residue_iter, --helix) {
//              qDebug() << "found residue" << (*residue_iter)->type()->attributes.get(Utopia::UtopiaSystem.uri).toString();
            // Ensure a backbone
            Utopia::Node * backbone = (*residue_iter)->relations(Utopia::UtopiaSystem.hasPart).front();
            if (backbone == 0) continue;

            if (extrusions.size() == 0) {
                extrusion = new gtl::extrusion< gtl::twine_3f, gtl::PartialCentripetalUpVector >;
                extrusions.push_back(extrusion);
                secstr = new SecStr;
                secstrs.push_back(secstr);
                path.clear();
                value = 0.5;
            }

//            std::cout << renderFormat << std::endl;
            if (renderFormat == renderableManager->CARTOON) {
                /*
                  Utopia::Node::parent_iterator ann_iter = (*residue_iter)->parentsBegin(&UTOPIA::MODEL::isAnnotation);
                  Utopia::Node::parent_iterator ann_end = (*residue_iter)->parentsEnd();
                  for (; ann_iter != ann_end; ++ann_iter) {
                  if ((*ann_iter)->hasAttribute("name") && (*ann_iter)->hasAttribute("class") && (*ann_iter)->hasAttribute("width") && (*ann_iter)->getAttribute<string>("class") == "extent" && ((*ann_iter)->getAttribute<string>("name") == "Helices" || (*ann_iter)->getAttribute<string>("name") == "Sheets" || (*ann_iter)->getAttribute<string>("name") == "Turns")) {
                  if ((*(*ann_iter)->parentsBegin())->getAttribute<string>("name") == "PDB") {
                  secStr = (*ann_iter)->getAttribute<string>("name");
                  length = (*ann_iter)->getAttribute<int>("width");
                  if (secStr == "Sheets") {
                  secstr->addSheet(gtl::extent_f(value - 0.5, value + length - 0.5));
                  } else if (secStr == "Helices") {
                  secstr->addHelix(gtl::extent_f(value - 0.5, value + length - 0.5));
                  }
                  }
                  }
                  }
                */
                --length;
                if (length == 0) {
                    secStr = "";
                }
            }

            if (renderOptions.find(renderableManager->SMOOTH) != renderOptions.end()) {
                // Ensure an Alpha carbon
                if (alphaCarbon == 0) {
                    Utopia::Node::relation::iterator atom_iter = backbone->relations(Utopia::UtopiaSystem.hasPart).begin();
                    Utopia::Node::relation::iterator atom_end = backbone->relations(Utopia::UtopiaSystem.hasPart).end();
                    for (; atom_iter != atom_end; ++atom_iter) {
                        if ((*atom_iter)->type()->attributes.get(Utopia::UtopiaDomain.term("formula"), "").toString() == "C" && (*atom_iter)->attributes.get("remoteness", ' ').toChar() == 'A') {
                            alphaCarbon = *atom_iter;
                        } else if ((*atom_iter)->type()->attributes.get(Utopia::UtopiaDomain.term("formula"), "").toString() == "C" && (*atom_iter)->attributes.get("remoteness", ' ').toChar() == ' ') {
                            betaCarbon = *atom_iter;
                        } else if ((*atom_iter)->type()->attributes.get(Utopia::UtopiaDomain.term("formula"), "").toString() == "N" && (*atom_iter)->attributes.get("remoteness", ' ').toChar() == ' ') {
                            nitrogen = *atom_iter;
                        }
                    }

//                                      qDebug() << backbone->relations(Utopia::UtopiaSystem.hasPart).size() << alphaCarbon << betaCarbon << nitrogen;
                }
                if (alphaCarbon == 0) continue;

                float x = alphaCarbon->attributes.get("x", 0).toDouble();
                float y = alphaCarbon->attributes.get("y", 0).toDouble();
                float z = alphaCarbon->attributes.get("z", 0).toDouble();
                gtl::vector_3f xyz(x, y, z);
                // Get the next Alpha carbon
                Utopia::Node::relation::iterator next_residue_iter = residue_iter;
                ++next_residue_iter;
                nextAlphaCarbon = 0;
                if (next_residue_iter != residue_end) {
                    // Ensure a backbone
                    backbone = (*next_residue_iter)->relations(Utopia::UtopiaSystem.hasPart).front();
                    if (backbone == 0) continue;

                    // Find Alpha carbon
                    Utopia::Node::relation::iterator atom_iter = backbone->relations(Utopia::UtopiaSystem.hasPart).begin();
                    Utopia::Node::relation::iterator atom_end = backbone->relations(Utopia::UtopiaSystem.hasPart).end();
                    for (; atom_iter != atom_end; ++atom_iter) {
                        if ((*atom_iter)->type()->attributes.get(Utopia::UtopiaDomain.term("formula"), "").toString() == "C" && (*atom_iter)->attributes.get("remoteness", ' ').toChar() == 'A') {
                            nextAlphaCarbon = *atom_iter;
                        }
                    }
                }
                if (nextAlphaCarbon) {
                    float x = nextAlphaCarbon->attributes.get("x", 0).toDouble();
                    float y = nextAlphaCarbon->attributes.get("y", 0).toDouble();
                    float z = nextAlphaCarbon->attributes.get("z", 0).toDouble();
                    gtl::vector_3f next(x, y, z);
                    if (gtl::norm(xyz - next) > 4.0) {
                        // Discontinuous chain
                        nextAlphaCarbon = 0;
                    }
                }
                if (next_residue_iter != residue_end)
                {
                    ++next_residue_iter;
                    nextNextAlphaCarbon = 0;
                    if (nextAlphaCarbon && next_residue_iter != residue_end) {
                        // Ensure a backbone
                        backbone = (*next_residue_iter)->relations(Utopia::UtopiaSystem.hasPart).front();
                        if (backbone == 0) continue;

                        // Find Alpha carbon
                        Utopia::Node::relation::iterator atom_iter = backbone->relations(Utopia::UtopiaSystem.hasPart).begin();
                        Utopia::Node::relation::iterator atom_end = backbone->relations(Utopia::UtopiaSystem.hasPart).end();
                        for (; atom_iter != atom_end; ++atom_iter) {
                            if ((*atom_iter)->type()->attributes.get(Utopia::UtopiaDomain.term("formula"), "").toString() == "C" && (*atom_iter)->attributes.get("remoteness", ' ').toChar() == 'A') {
                                nextNextAlphaCarbon = *atom_iter;
                            }
                        }
                    }
                    if (nextAlphaCarbon && nextNextAlphaCarbon) {
                        float x = nextAlphaCarbon->attributes.get("x", 0).toDouble();
                        float y = nextAlphaCarbon->attributes.get("y", 0).toDouble();
                        float z = nextAlphaCarbon->attributes.get("z", 0).toDouble();
                        gtl::vector_3f next(x, y, z);
                        x = nextNextAlphaCarbon->attributes.get("x", 0).toDouble();
                        y = nextNextAlphaCarbon->attributes.get("y", 0).toDouble();
                        z = nextNextAlphaCarbon->attributes.get("z", 0).toDouble();
                        gtl::vector_3f nextNext(x, y, z);
                        if (gtl::norm(nextNext - next) > 4.0) {
                            // Discontinuous chain
                            nextNextAlphaCarbon = 0;
                        }
                    }
                }

                // Check for sane backbone...
                if (nitrogen && prevBetaCarbon) {
                    float x = prevBetaCarbon->attributes.get("x", 0).toDouble();
                    float y = prevBetaCarbon->attributes.get("y", 0).toDouble();
                    float z = prevBetaCarbon->attributes.get("z", 0).toDouble();
                    gtl::vector_3f prev(x, y, z);
                    x = nitrogen->attributes.get("x", 0).toDouble();
                    y = nitrogen->attributes.get("y", 0).toDouble();
                    z = nitrogen->attributes.get("z", 0).toDouble();
                    gtl::vector_3f xyz(x, y, z);
                    if (gtl::norm(prev - xyz) > MIN_SEPERATION) {
                        // Discontinuous chain
                        if (extrusion) {
                            extrusion->path(path, gtl::extent< double >(0.0, value + 0.5), 0.2);
                        }
                        extrusion = new gtl::extrusion< gtl::twine_3f, gtl::PartialCentripetalUpVector >;
                        extrusions.push_back(extrusion);
                        secstr = new SecStr;
                        secstrs.push_back(secstr);
                        path.clear();
                        value = 0.5;
                        prevAlphaCarbon = 0;
                    }
                }

/*
  UTOPIA::MODEL::IsAnnotation isSS("Helices");
  Utopia::Node::parent_iterator parent_iter = (*residue_iter)->parentsBegin(&isSS);
  Utopia::Node::parent_iterator parent_end = (*residue_iter)->parentsEnd();
  if (parent_iter != parent_end && (*parent_iter)->hasAttribute("width")) {
  helix = (*parent_iter)->getAttribute<int>("width");
  }
*/
                if (prevAlphaCarbon && nextAlphaCarbon) {
                    float x = prevAlphaCarbon->attributes.get("x", 0).toDouble();
                    float y = prevAlphaCarbon->attributes.get("y", 0).toDouble();
                    float z = prevAlphaCarbon->attributes.get("z", 0).toDouble();
                    gtl::vector_3f prev(x, y, z);
                    x = nextAlphaCarbon->attributes.get("x", 0).toDouble();
                    y = nextAlphaCarbon->attributes.get("y", 0).toDouble();
                    z = nextAlphaCarbon->attributes.get("z", 0).toDouble();
                    gtl::vector_3f next(x, y, z);
                    if (helix > 0) {
                    } else {
                        // Work out a roughly smoothed path
                        xyz = ((((prev + next) / 2.0) + xyz) / 2.0);
                    }
                }

                path[value] = xyz;
                extrusionsMap[(*residue_iter)->attributes.get("seqId", "").toString().toStdString()] = extrusion;
                secstrsMap[(*residue_iter)->attributes.get("seqId", "").toString().toStdString()] = secstr;
                if (prev_extrusion == extrusion) {
                    prevAlphaCarbon = alphaCarbon;
                    prevBetaCarbon = betaCarbon;
                } else {
                    prevAlphaCarbon = 0;
                    prevBetaCarbon = 0;
                }
                alphaCarbon = nextAlphaCarbon;
            } else {
                // Ensure an Alpha carbon
                alphaCarbon = 0;
                Utopia::Node::relation::iterator atom_iter = backbone->relations(Utopia::UtopiaSystem.hasPart).begin();
                Utopia::Node::relation::iterator atom_end = backbone->relations(Utopia::UtopiaSystem.hasPart).end();
                for (; atom_iter != atom_end; ++atom_iter) {
                    if ((*atom_iter)->type()->attributes.get(Utopia::UtopiaDomain.term("formula"), "").toString() == "C" && (*atom_iter)->attributes.get("remoteness", ' ').toChar() == 'A') {
                        alphaCarbon = *atom_iter;
                    } else if ((*atom_iter)->type()->attributes.get(Utopia::UtopiaDomain.term("formula"), "").toString() == "C" && (*atom_iter)->attributes.get("remoteness", ' ').toChar() == ' ') {
                        betaCarbon = *atom_iter;
                    } else if ((*atom_iter)->type()->attributes.get(Utopia::UtopiaDomain.term("formula"), "").toString() == "N" && (*atom_iter)->attributes.get("remoteness", ' ').toChar() == ' ') {
                        nitrogen = *atom_iter;
                    }
                }
                if (alphaCarbon == 0) continue;

                float x = alphaCarbon->attributes.get("x", 0).toDouble();
                float y = alphaCarbon->attributes.get("y", 0).toDouble();
                float z = alphaCarbon->attributes.get("z", 0).toDouble();
                gtl::vector_3f xyz(x, y, z);
//                 float * xyz1 = alphaCarbon->getPosition();

                // Check for sane backbone...
                if (nitrogen && prevBetaCarbon) {
                    float x = prevBetaCarbon->attributes.get("x", 0).toDouble();
                    float y = prevBetaCarbon->attributes.get("y", 0).toDouble();
                    float z = prevBetaCarbon->attributes.get("z", 0).toDouble();
                    gtl::vector_3f prev(x, y, z);
                    x = nitrogen->attributes.get("x", 0).toDouble();
                    y = nitrogen->attributes.get("y", 0).toDouble();
                    z = nitrogen->attributes.get("z", 0).toDouble();
                    gtl::vector_3f xyz(x, y, z);
                    if (gtl::norm(xyz - prev) > MIN_SEPERATION) {
                        // Discontinuous chain
                        if (extrusion) {
                            extrusion->path(path, gtl::extent< double >(0.0, value + 0.5), 0.2);
                        }
                        extrusion = new gtl::extrusion< gtl::twine_3f, gtl::PartialCentripetalUpVector >;
                        extrusions.push_back(extrusion);
                        secstr = new SecStr;
                        secstrs.push_back(secstr);
                        path.clear();
                        value = 0.5;
                    }
                }

                path[value] = xyz;
                extrusionsMap[(*residue_iter)->attributes.get("seqId", "").toString().toStdString()] = extrusion;
                secstrsMap[(*residue_iter)->attributes.get("seqId", "").toString().toStdString()] = secstr;

                if (prev_extrusion == extrusion) {
                    prevAlphaCarbon = alphaCarbon;
                    prevBetaCarbon = betaCarbon;
                } else {
                    prevAlphaCarbon = 0;
                    prevBetaCarbon = 0;
                }
            }
            value += 1.0;

            // remember previous interpolation
            prev_extrusion = extrusion;
        }

        if (extrusion) {
            extrusion->path(path, gtl::extent< double >(0.0, value + 0.5), 0.2);
        }

        prev_extrusion = 0;

        // Compile to buffer
        if (renderFormat == renderableManager->BACKBONE || renderFormat == renderableManager->CARTOON) {
            value = 0.5;
            residue_iter = chain->relations(Utopia::UtopiaSystem.hasPart).begin();
            residue_end = chain->relations(Utopia::UtopiaSystem.hasPart).end();
            for (; residue_iter != residue_end; ++residue_iter) {
                // Position along residue path
                if (extrusionsMap.find((*residue_iter)->attributes.get("seqId", "").toString().toStdString()) == extrusionsMap.end())
                    continue;
                extrusion = extrusionsMap[(*residue_iter)->attributes.get("seqId", "").toString().toStdString()];
                secstr = secstrsMap[(*residue_iter)->attributes.get("seqId", "").toString().toStdString()];
                if (prev_extrusion != extrusion) {
                    value = 0.5;
                }

                ResidueRenderable * residueRenderable = static_cast<ResidueRenderable *>(residueRenderableManager.get(*residue_iter))   ;
                if (residueRenderable == 0) {
                    residueRenderable = new ResidueRenderable(*residue_iter, &residueRenderableManager, value);
                    residueRenderableManager.renderables[*residue_iter] = residueRenderable;
                    residueRenderableManager.invalidateBuffers();
                }

                residueRenderable->extrusion = extrusion;
                residueRenderable->secstr = secstr;
                residueRenderable->value = value;

                residueRenderable->populateBuffer();
                value += 1.0;

                // remember previous interpolation
                prev_extrusion = extrusion;
            }
        }
    }

    unsigned int ChainRenderable::vertexCount()
    {
//              if (renderFormat == renderableManager->SPACEFILL || renderFormat == renderableManager->BALLSANDSTICKS)
//              return (renderableManager->lod * 2 + 1) * 2 * renderableManager->lod * chain->childCount() + 4;
        return chain->relations(Utopia::UtopiaSystem.hasPart).size() * (4 * renderableManager->lod * renderableManager->lod + 10 * renderableManager->lod + 8);
    }

} // namespace AMBROSIA



// dlsym handles

extern "C" const char * utopia_apiVersion()
{
    return UTOPIA_EXTENSION_LIBRARY_VERSION;
}

extern "C" const char * utopia_description()
{
    return "Chain Renderer (OpenGL)";
}

extern "C" void utopia_registerExtensions()
{
    UTOPIA_REGISTER_EXTENSION_NAMED(AMBROSIA::ChainRenderableManager, "chain_basic");
}
