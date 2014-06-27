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

#include <ambrosia/renderable.h>
#include <cmath>
#include <set>
#include <ambrosia/utils.h>
#include <QString>
#include <QMap>
#include <QSet>
#include <utopia2/extension.h>
#include <utopia2/extensionlibrary.h>

namespace AMBROSIA {

    class AtomRenderable;

    //
    // RenderableManager class
    //

    class AtomRenderableManager : public RenderableManager {

    public:
        // Constructor
        AtomRenderableManager();
        // Destructor
        ~AtomRenderableManager();

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
        unsigned int SPACEFILL;
        unsigned int BALLSANDSTICKS;

        // LOD
        unsigned int lod;
        float * sphere;

        // Shaders
        ShaderProgram * sphereShader;
        ShaderProgram * sphereDepthShader;
        ShaderProgram * circleShader;
        ShaderProgram * circleOutlineShader;

        // Render members
        unsigned int * renderFormats;
        unsigned int * renderOptions;

        // Vertex buffers
        map< unsigned int, map< unsigned int, map< unsigned int, BufferManager * > > > bufferManagers;
        bool validBuffers;

        // Renderables managed by this manager
        map< UTOPIA::MODEL::Atom *, AtomRenderable * > renderables;

    }; // class AtomRenderableManager

    //
    // AtomRenderable class
    //

    class AtomRenderable : public Renderable {

    public:
        // Convenience
        typedef Renderable::_BufferType _BufferType;

        // Constructor
        AtomRenderable(Utopia::Node *, RenderableManager *);
        // Destructor
        ~AtomRenderable();

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

        // Model
        UTOPIA::MODEL::Atom * atom;

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
        AtomRenderableManager * renderableManager;

    }; // class AtomRenderable

    // Constructor
    AtomRenderableManager::AtomRenderableManager()
        : lod(0), sphere(0), sphereShader(0), circleShader(0), validBuffers(false)
    {
        // Initialise level of detail
        setLOD();

        // Log renderableManager
        UTOPIA::Logger::log("Ambrosia: rendering atoms using geometric billboarding", 1);

        // Populate renderFormats
        renderFormats = new unsigned int[3];
        renderFormats[0] = SPACEFILL = Ambrosia::getToken("Render Format", "Spacefill");
        renderFormats[1] = BALLSANDSTICKS = Ambrosia::getToken("Render Format", "Balls and Sticks");
        renderFormats[2] = 0;
        // Populate renderOptions
        renderOptions = new unsigned int[1];
        renderOptions[0] = 0;

        // Shader stuff
        if (Shader::capability() == Shader::GLSL) {
            sphereShader = new ShaderProgram();
            sphereShader->addShader(loadShader((Utopia::resource_path() + "ambrosia/glsl/sphere.vert").toUtf8().constData(), Shader::VERTEX));
            sphereShader->addShader(loadShader((Utopia::resource_path() + "ambrosia/glsl/sphere.frag").toUtf8().constData(), Shader::FRAGMENT));

            sphereDepthShader = new ShaderProgram();
            sphereDepthShader->addShader(loadShader((Utopia::resource_path() + "ambrosia/glsl/sphere-depth.vert").toUtf8().constData(), Shader::VERTEX));
            sphereDepthShader->addShader(loadShader((Utopia::resource_path() + "ambrosia/glsl/sphere-depth.frag").toUtf8().constData(), Shader::FRAGMENT));

            circleShader = new ShaderProgram();
            circleShader->addShader(loadShader((Utopia::resource_path() + "ambrosia/glsl/circle.vert").toUtf8().constData(), Shader::VERTEX));
            circleShader->addShader(loadShader((Utopia::resource_path() + "ambrosia/glsl/circle.frag").toUtf8().constData(), Shader::FRAGMENT));

            circleOutlineShader = new ShaderProgram();
            circleOutlineShader->addShader(loadShader((Utopia::resource_path() + "ambrosia/glsl/circle-outline.vert").toUtf8().constData(), Shader::VERTEX));
            circleOutlineShader->addShader(loadShader((Utopia::resource_path() + "ambrosia/glsl/circle-outline.frag").toUtf8().constData(), Shader::FRAGMENT));
        }
    }
    // Destructor
    AtomRenderableManager::~AtomRenderableManager()
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
        map< UTOPIA::MODEL::Atom *, AtomRenderable * >::iterator atom_iter = renderables.begin();
        map< UTOPIA::MODEL::Atom *, AtomRenderable * >::iterator atom_end = renderables.end();
        for(; atom_iter != atom_end; ++atom_iter)
            destroy(atom_iter->second);

        // Delete shaders
        if (circleShader)
            delete circleShader;
        if (circleOutlineShader)
            delete circleOutlineShader;
        if (sphereShader)
            delete sphereShader;
        if (sphereDepthShader)
            delete sphereDepthShader;
    }

    // Management methods
    Renderable * AtomRenderableManager::create(void * data)
    {
        UTOPIA::MODEL::Atom * atom = (UTOPIA::MODEL::Atom *) data;
        AtomRenderable * newRenderable = new AtomRenderable(atom, this);
        renderables[atom] = newRenderable;
        invalidateBuffers();
        return newRenderable;
    }
    void AtomRenderableManager::destroy(Renderable * renderable)
    {
        AtomRenderable * atomRenderable = (AtomRenderable *) renderable;
        if (atomRenderable) {
            if (atomRenderable->buffer) {
                invalidateBuffers();
                atomRenderable->buffer->invalidate();
                atomRenderable->buffer = 0;
            }
            renderables.erase(atomRenderable->getData());
            delete atomRenderable;
        }
    }
    Renderable * AtomRenderableManager::get(void * data)
    {
        UTOPIA::MODEL::Atom * atom = (UTOPIA::MODEL::Atom *) data;
        if (renderables.find(atom) != renderables.end())
            return renderables[atom];
        else
            return 0;
    }
    void AtomRenderableManager::clear()
    {
        map< UTOPIA::MODEL::Atom *, AtomRenderable * >::iterator iter = renderables.begin();
        map< UTOPIA::MODEL::Atom *, AtomRenderable * >::iterator end = renderables.end();
        for (; iter != end;) {
            AtomRenderable * atomRenderable = iter->second;
            ++iter;
            destroy(atomRenderable);
        }
        renderables.clear();
    }

    // Render methods
    void AtomRenderableManager::setLOD(unsigned int lod)
    {
        // Restrict LOD value
        unsigned int newLod = (lod >= 4) ? lod : 4;

        // If no change, bail
        if (newLod == this->lod)
            return;

        // Set new LOD
        this->lod = newLod;

        // Precompute surface of a sphere
        float PI = 3.1415926535;
        int vertexCount = (newLod + 1) * newLod * 4;
        if (sphere != 0)
            delete sphere;
        sphere = new float[vertexCount * 3];
        float * vertexCursor = sphere;
        for (unsigned int a = 0; a < newLod * 2; a++) {
            float j = ((float) a) / ((float) newLod);
            for (unsigned int b = 0; b <= newLod; b++) {
                float i = ((float) b) / ((float) newLod);
                float latitude = - PI * (0.5 - i);
                float longitude = PI * (j + (1.0 / (float) newLod));

                *(vertexCursor++) = cos(latitude) * cos(longitude);
                *(vertexCursor++) = sin(latitude);
                *(vertexCursor++) = cos(latitude) * sin(longitude);

                longitude = PI * j;
                *(vertexCursor++) = cos(latitude) * cos(longitude);
                *(vertexCursor++) = sin(latitude);
                *(vertexCursor++) = cos(latitude) * sin(longitude);
            }
        }
    }
    void AtomRenderableManager::render(Ambrosia::RenderPass renderPass)
    {
        if (!validBuffers)
            rebuildBuffers();

        switch (renderPass) {
        case Ambrosia::SHADOW_MAP_PASS:
        case Ambrosia::STENCIL_PASS:
            // set shaders
            if (circleShader) circleShader->enable();
            break;
        case Ambrosia::DRAW_OUTLINE_PASS:
            // set shaders
            if (circleOutlineShader) circleOutlineShader->enable();
            break;
        case Ambrosia::DEPTH_SHADE_PASS:
        case Ambrosia::DEPTH_TRANSPARENT_PASS:
        case Ambrosia::NAME_PASS:
            // set shaders
            if (sphereDepthShader) sphereDepthShader->enable();
            break;
        case Ambrosia::DRAW_SHADE_PASS:
        case Ambrosia::DRAW_TRANSPARENT_PASS:
        case Ambrosia::DRAW_PASS:
            // Enable shaders
            if (sphereShader) sphereShader->enable();
            break;
        default:
            break;
        }

        if (renderPass == Ambrosia::DRAW_OUTLINE_PASS || renderPass == Ambrosia::STENCIL_PASS || renderPass == Ambrosia::SHADOW_MAP_PASS) {
            map< UTOPIA::MODEL::Atom *, AtomRenderable * >::iterator atom_iter = renderables.begin();
            map< UTOPIA::MODEL::Atom *, AtomRenderable * >::iterator atom_end = renderables.end();
            for(; atom_iter != atom_end; ++atom_iter) {
                AtomRenderable * atom = atom_iter->second;
                if (atom->hasTag(Ambrosia::OUTLINE))
                    atom->render(renderPass);
            }
        } else if (renderPass == Ambrosia::NAME_PASS) {
            map< UTOPIA::MODEL::Atom *, AtomRenderable * >::iterator atom_iter = renderables.begin();
            map< UTOPIA::MODEL::Atom *, AtomRenderable * >::iterator atom_end = renderables.end();
            for(; atom_iter != atom_end; ++atom_iter) {
                AtomRenderable * atom = atom_iter->second;
                if (atom->hasTag(Ambrosia::SOLID) || atom->hasTag(Ambrosia::OUTLINE))
                    atom->render(renderPass);
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
        if (circleShader) circleShader->disable();
        if (circleOutlineShader) circleOutlineShader->disable();
        if (sphereShader) sphereShader->disable();
        if (sphereDepthShader) sphereDepthShader->disable();
    }
    bool AtomRenderableManager::requiresRedraw()
    {
        return false;
    }

    // Discovery methods
    unsigned int * AtomRenderableManager::getRenderFormats()
    { return renderFormats; }
    unsigned int * AtomRenderableManager::getRenderOptions()
    { return renderOptions; }

    // Buffering methods
    Buffer * AtomRenderableManager::getBuffer(unsigned int renderFormat, unsigned int tag, unsigned int mode, unsigned int verticesRequired)
    {
        if (bufferManagers[renderFormat][tag].find(mode) == bufferManagers[renderFormat][tag].end())
            bufferManagers[renderFormat][tag][mode] = new BufferManager();
        BufferManager * bufferManager = bufferManagers[renderFormat][tag][mode];
        return bufferManager->getBuffer(verticesRequired);
    }
    void AtomRenderableManager::invalidateBuffers()
    { validBuffers = false; }
    void AtomRenderableManager::rebuildBuffers()
    {
        validBuffers = true;

        // Unlink all renderables from invalid buffers...
        map< UTOPIA::MODEL::Atom *, AtomRenderable * >::iterator renderable = renderables.begin();
        map< UTOPIA::MODEL::Atom *, AtomRenderable * >::iterator renderable_end = renderables.end();
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
    AtomRenderable::AtomRenderable(UTOPIA::MODEL::Atom * atom, RenderableManager * renderableManager)
        : Renderable(), atom(atom), display(true), visible(true), alpha(115), tintColour(0), highlightColour(0), tag(Ambrosia::SOLID), buffer(0), bufferIndex(0)
    {
        // Default rendering spec
        colour = Colour::getColour(string("element.") + atom->getElement()->getSymbol());
        this->renderableManager = (AtomRenderableManager *) renderableManager;
        renderFormat = this->renderableManager->SPACEFILL;
    }
    // Destructor
    AtomRenderable::~AtomRenderable()
    {}

    // General methods
    UTOPIA::MODEL::Atom * AtomRenderable::getData()
    { return atom; }
    void AtomRenderable::setDisplay(bool display)
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
    void AtomRenderable::setVisible(bool visible)
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
    void AtomRenderable::setRenderFormat(unsigned int renderFormat)
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
    void AtomRenderable::setRenderOption(unsigned int renderOption, bool flag)
    {
        if ((renderOptions.find(renderOption) != renderOptions.end()) == flag)
            return;
        if (flag) {
            renderOptions.insert(renderOption);
        } else {
            renderOptions.erase(renderOption);
        }

        if (buffer && visible && display) {
            renderableManager->invalidateBuffers();
            buffer->invalidate();
            buffer = 0;
        }
    }
    void AtomRenderable::setColour(Colour * colour)
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
    void AtomRenderable::setAlpha(unsigned char alpha)
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
    void AtomRenderable::setTintColour(Colour * colour)
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
    void AtomRenderable::setHighlightColour(Colour * colour)
    {
        this->highlightColour = colour;
    }
    void AtomRenderable::setTag(unsigned int tag)
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
    bool AtomRenderable::hasTag(unsigned int tag)
    { return this->tag == tag; }

    /** Rebuild vertex buffer for this renderable. */
    void AtomRenderable::v2_build_buffer()
    {
        // FIXME
    }

    // OpenGL methods
    void AtomRenderable::render(Ambrosia::RenderPass renderPass, unsigned int elements)
    {
        if (!(visible && display))
            return;

        if (buffer == 0)
            populateBuffer();

        if (elements & Buffer::COLOUR) {
            switch (renderPass) {
            case Ambrosia::DRAW_OUTLINE_PASS:
                if (highlightColour) {
                    elements &= ~Buffer::COLOUR;
                    glColor3f(highlightColour->r, highlightColour->g, highlightColour->b);
                }
                // set shaders
                if (renderableManager->circleOutlineShader) renderableManager->circleOutlineShader->enable();
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                break;
            case Ambrosia::SHADOW_MAP_PASS:
            case Ambrosia::STENCIL_PASS:
                // set shaders
                if (renderableManager->circleShader) renderableManager->circleShader->enable();
                break;
            case Ambrosia::DEPTH_SHADE_PASS:
            case Ambrosia::DEPTH_TRANSPARENT_PASS:
            case Ambrosia::NAME_PASS:
                // set shaders
                if (renderableManager->sphereShader) renderableManager->sphereShader->enable();
                break;
            case Ambrosia::DRAW_SHADE_PASS:
            case Ambrosia::DRAW_TRANSPARENT_PASS:
            case Ambrosia::DRAW_PASS:
                // Enable shaders
                if (renderableManager->sphereShader) renderableManager->sphereShader->enable();
                break;
            default:
                break;
            }
        }

        // Render buffer
        if (renderFormat == renderableManager->BALLSANDSTICKS || renderFormat == renderableManager->SPACEFILL) {
            // Draw buffer
            if (renderPass == Ambrosia::NAME_PASS)
            {
                glPushName(this->_v2_renderable_name);
            }
            buffer->enable(elements);
            buffer->render(GL_TRIANGLES, bufferIndex, vertexCount());
            buffer->disable();
            if (renderPass == Ambrosia::NAME_PASS)
            {
                glPopName();
            }
        }

        // disable shaders
        if (renderableManager->circleShader) renderableManager->circleShader->disable();
        if (renderableManager->circleOutlineShader) renderableManager->circleOutlineShader->disable();
        if (renderableManager->sphereShader) renderableManager->sphereShader->disable();
        if (renderableManager->sphereDepthShader) renderableManager->sphereDepthShader->disable();
    }

    // Buffering methods
    void AtomRenderable::populateBuffer()
    {
        if (buffer == 0) {
            buffer = renderableManager->getBuffer(renderFormat, tag, GL_TRIANGLES, vertexCount());
            bufferIndex = buffer->usedVertices();
        } else {
            buffer->to(bufferIndex);
        }

        float * xyz = atom->getPosition();
        float x = xyz[0];
        float y = xyz[1];
        float z = xyz[2];
        float r = atom->getElement()->getRadius();
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

        // Compile to buffer
        if (renderFormat == renderableManager->BALLSANDSTICKS)
            // Reduced radius for ball and stick mode
            r /= 4.0;
        if (renderFormat == renderableManager->SPACEFILL || renderFormat == renderableManager->BALLSANDSTICKS) {
            buffer->setNormal(1.0, 1.0, r);
            buffer->setPosition(x, y, z);
            buffer->setColourb(R, G, B, A);
            buffer->next();
            buffer->setNormal(-3.0, 1.0, r);
            buffer->setPosition(x, y, z);
            buffer->setColourb(R, G, B, A);
            buffer->next();
            buffer->setNormal(1.0, -3.0, r);
            buffer->setPosition(x, y, z);
            buffer->setColourb(R, G, B, A);
            buffer->next();
        }
    }

    unsigned int AtomRenderable::vertexCount()
    {
        if (renderFormat == renderableManager->SPACEFILL || renderFormat == renderableManager->BALLSANDSTICKS)
            return 3;
        else
            return (renderableManager->lod + 1) * renderableManager->lod * 4;
    }

} // namespace AMBROSIA






// dlsym handles

extern "C" const char * utopia_apiVersion()
{
    return UTOPIA_EXTENSION_LIBRARY_VERSION;
}

extern "C" const char * utopia_description()
{
    return "Atom Renderer (GLSL)";
}

extern "C" void utopia_registerExtensions()
{
    UTOPIA_REGISTER_EXTENSION_NAMED(AMBROSIA::AtomRenderableManager, "atom_glsl");
}
