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

#include <ambrosia/ambrosia.h>
#include <ambrosia/renderable.h>
#include <cmath>
#include "ambrosia/utils.h"
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

using namespace std;

namespace AMBROSIA {

    //
    // Command class
    //


    class Command {
    public:
        // Destructor
        virtual ~Command() {};

        // Functor commands
        virtual void apply(Renderable * renderable)
            {}

    }; // class Command

    //
    // Command classes
    //

    class SetDisplay : public Command {
    public:
        // Constructor
        SetDisplay(bool flag)
            : flag(flag)
            {}
        // Destructor
        ~SetDisplay()
            {}

        // Functor commands
        virtual void apply(Renderable * renderable)
            { if (renderable) renderable->setDisplay(flag); }

    private:
        // member
        bool flag;
    }; // class SetDisplay

    class SetVisible : public Command {
    public:
        // Constructor
        SetVisible(bool flag)
            : flag(flag)
            {}
        // Destructor
        ~SetVisible()
            {}

        // Functor commands
        virtual void apply(Renderable * renderable)
            { if (renderable) renderable->setVisible(flag); }

    private:
        // member
        bool flag;
    }; // class SetVisible

    class SetRenderFormat : public Command {
    public:
        // Constructor
        SetRenderFormat(unsigned int format)
            : format(format)
            {}
        // Destructor
        ~SetRenderFormat()
            {}

        // Functor commands
        virtual void apply(Renderable * renderable)
            { if (renderable) renderable->setRenderFormat(format); }

    private:
        // member
        unsigned int format;
    }; // class SetRenderFormat

    class SetRenderOption : public Command {
    public:
        // Constructor
        SetRenderOption(unsigned int option, bool flag)
            : option(option), flag(flag)
            {}
        // Destructor
        ~SetRenderOption()
            {}

        // Functor commands
        virtual void apply(Renderable * renderable)
            { if (renderable) renderable->setRenderOption(option, flag); }

    private:
        // member
        unsigned int option;
        bool flag;
    }; // class SetRenderOption

    class SetColour : public Command {
    public:
        // Constructor
        SetColour(Colour * colour)
            : colour(colour)
            {}
        // Destructor
        ~SetColour()
            {}

        // Functor commands
        virtual void apply(Renderable * renderable)
            { if (renderable) renderable->setColour(colour); }

    private:
        // member
        Colour * colour;
    }; // class SetColour

    class SetAlpha : public Command {
    public:
        // Constructor
        SetAlpha(unsigned char alpha)
            : alpha(alpha)
            {}
        // Destructor
        ~SetAlpha()
            {}

        // Functor commands
        virtual void apply(Renderable * renderable)
            { if (renderable) renderable->setAlpha(alpha); }

    private:
        // member
        unsigned char alpha;
    }; // class SetAlpha

    class SetTintColour : public Command {
    public:
        // Constructor
        SetTintColour(Colour * colour)
            : colour(colour)
            {}
        // Destructor
        ~SetTintColour()
            {}

        // Functor commands
        virtual void apply(Renderable * renderable)
            { if (renderable) renderable->setTintColour(colour); }

    private:
        // member
        Colour * colour;
    }; // class SetTintColour

    class SetHighlightColour : public Command {
    public:
        // Constructor
        SetHighlightColour(Colour * colour)
            : colour(colour)
            {}
        // Destructor
        ~SetHighlightColour()
            {}

        // Functor commands
        virtual void apply(Renderable * renderable)
            { if (renderable) renderable->setHighlightColour(colour); }

    private:
        // member
        Colour * colour;
    }; // class SetHighlightColour

    class SetRenderTag : public Command {
    public:
        // Constructor
        SetRenderTag(Ambrosia::RenderTag tag)
            : tag(tag)
            {}
        // Destructor
        ~SetRenderTag()
            {}

        // Functor commands
        virtual void apply(Renderable * renderable)
            { if (renderable) renderable->setTag(tag); }

    private:
        // member
        Ambrosia::RenderTag tag;
    }; // class SetRenderTag

    //
    // Ambrosia class
    //

    // Constructor
    Ambrosia::Ambrosia()
    { init(); }
    Ambrosia::Ambrosia(string uri)
    {
        init();
        load(uri);
    }
    Ambrosia::Ambrosia(Utopia::Node * complex)
    {
        init();
        load(complex);
    }
    // Destructor
    Ambrosia::~Ambrosia()
    {
//         std::cout << "Ambrosia::~Ambrosia() " << this << std::endl;
        clear();
        if (atomRenderableManager) {
            delete atomRenderableManager;
        }
        if (chainRenderableManager) {
            delete chainRenderableManager;
        }
    }

    // General methods
    bool Ambrosia::load(string uri)
    {
        // FIXME load(UTOPIA::load(uri));
        return false;
    }
    bool Ambrosia::load(Utopia::Node * complex)
    {
        clear();

        qDebug() << "Ambrosia::load";
        qDebug() << complex;
        qDebug() << complex->type();
        qDebug() << Utopia::Node::getNode("complex");
        qDebug() << Utopia::Node::getNode("complex");
        if (complex && complex->type() == Utopia::Node::getNode("complex")) {
            this->complex = complex;
            build();
            return true;
        }
        return false;
    }
    Utopia::Node * Ambrosia::getComplex()
    { return complex; }
    void Ambrosia::build()
    {
        qDebug() << "Ambrosia::build()" << complex;
        if (complex == 0) return;

        this->_built = true;

        // Initialise renderables if need be
        qDebug() << "atom_basic" << atomRenderableManager;
        if (atomRenderableManager == 0) {
            atomRenderableManager = getRenderableManager("atom", "glsl");
            qDebug() << "atom_glsl" << atomRenderableManager;
            if (atomRenderableManager == 0)
            {
                atomRenderableManager = getRenderableManager("atom", "basic");
                qDebug() << "atom_basic" << atomRenderableManager;
            }
        }
        qDebug() << "atom_basic" << atomRenderableManager;
        qDebug() << "chain_basic" << chainRenderableManager;
        if (chainRenderableManager == 0) {
//            chainRenderableManager = getRenderableManager("chain", "glsl");
            if (chainRenderableManager == 0)
            {
                chainRenderableManager = getRenderableManager("chain", "basic");
                qDebug() << "chain_basic" << chainRenderableManager;
            }
        }
        // OMG What a dirty hack
        Colour::populate("ambrosia.colourmap");

        // Build atoms
        int atomCount = 0;
        x = 0;
        y = 0;
        z = 0;
        float minX = 100000.0;
        float minY = 100000.0;
        float minZ = 100000.0;
        float maxX = -100000.0;
        float maxY = -100000.0;
        float maxZ = -100000.0;

        getSelection(ALL).add(complex);
        Utopia::Node * authority = complex->authority();

        Utopia::List * minions = authority->minions();
        Utopia::List::iterator node_iter = minions->begin();
        Utopia::List::iterator node_end = minions->end();
        for (; node_iter != node_end; ++node_iter)
        {
            Utopia::Node * node = *node_iter;
            if (node->type())
            {
                if (node->type()->relations(Utopia::rdfs.subClassOf).front() == Utopia::UtopiaDomain.term("Element"))
                {
    //                      qDebug() << "FOUND ATOM";
                    Utopia::Node * atom = node;
                    Renderable * atom_renderable = atomRenderableManager->create(atom);
                    atom_renderable->setDisplay(false);
                    double _x = atom->attributes.get("x", 0).toDouble();
                    double _y = atom->attributes.get("y", 0).toDouble();
                    double _z = atom->attributes.get("z", 0).toDouble();
                    x += _x;
                    y += _y;
                    z += _z;
                    if (_x < minX) minX = _x;
                    if (_y < minY) minY = _y;
                    if (_z < minZ) minZ = _z;
                    if (_x > maxX) maxX = _x;
                    if (_y > maxY) maxY = _y;
                    if (_z > maxZ) maxZ = _z;
                    atomCount++;
                    getSelection(ALL).add(atom);
                }
                else if (node->type() == Utopia::Node::getNode("chain"))
                {
    //                      qDebug() << "FOUND ATOM";
                    Utopia::Node * chain = node;
                    Renderable * chain_renderable = chainRenderableManager->create(chain);
                    chain_renderable->setDisplay(false);
                    getSelection(CHAINS).add(chain);
                }
            }
        }

/*        Utopia::Node::descendant_iterator node_iter = complex->descendantsBegin();
          Utopia::Node::descendant_iterator node_end = complex->descendantsEnd();
          for (; node_iter != node_end; ++node_iter) {
          if ((*node_iter)->getNodeClass() == UTOPIA::MODEL::Name::chain) {
          getSelection(CHAINS).add(*node_iter);
          chainRenderableManager->create(*node_iter);
          }
          if ((*node_iter)->getNodeClass() == UTOPIA::MODEL::Name::backbone)
          getSelection(BACKBONE).add(*node_iter);
          if ((*node_iter)->getNodeClass() == UTOPIA::MODEL::Name::sidechain)
          getSelection(SIDECHAIN).add(*node_iter);
          if ((*node_iter)->getNodeClass() == UTOPIA::MODEL::Name::aminoacid) {
          getSelection(AMINOACIDS).add(*node_iter);
          getSelection(RESIDUES).add(*node_iter);
          }
          if ((*node_iter)->getNodeClass() == UTOPIA::MODEL::Name::nucleotide) {
          getSelection(NUCLEOTIDES).add(*node_iter);
          getSelection(RESIDUES).add(*node_iter);
          }
          if ((*node_iter)->getNodeClass() == UTOPIA::MODEL::Name::protein)
          getSelection(PROTEINS).add(*node_iter);
          if ((*node_iter)->getNodeClass() == UTOPIA::MODEL::Name::nucleicacid)
          getSelection(NUCLEICACIDS).add(*node_iter);
          if ((*node_iter)->getNodeClass() == UTOPIA::MODEL::Name::heterogen) {
          getSelection(HETEROGENS).add(*node_iter);
          if ((*node_iter)->hasAttribute("hetID") && (*node_iter)->getAttribute<string>("hetID") == "HOH")
          getSelection(WATER).add(*node_iter);
          }
          Utopia::Node::atom_iterator atom_iter = (*node_iter)->atomsBegin();
          Utopia::Node::atom_iterator atom_end = (*node_iter)->atomsEnd();
          for (; atom_iter != atom_end; ++atom_iter) {
          // Create atom renderable
          Renderable * atom_renderable = atomRenderableManager->create(*atom_iter);
          atom_renderable->setDisplay(false);
          float * xyz = (*atom_iter)->getPosition();
          x += xyz[0];
          y += xyz[1];
          z += xyz[2];
          if (xyz[0] < minX) minX = xyz[0];
          if (xyz[1] < minY) minY = xyz[1];
          if (xyz[2] < minZ) minZ = xyz[2];
          if (xyz[0] > maxX) maxX = xyz[0];
          if (xyz[1] > maxY) maxY = xyz[1];
          if (xyz[2] > maxZ) maxZ = xyz[2];
          atomCount++;

          // set up selections
          getSelection(ATOMS).add(*atom_iter);
          if ((*atom_iter)->getElement()->isMetal())
          getSelection(METALS).add(*atom_iter);
          if ((*atom_iter)->getElement()->getSymbol() == "H")
          getSelection(HYDROGENS).add(*atom_iter);
          if ((*atom_iter)->getElement()->getSymbol() == "S")
          {
          getSelection(SULPHUR).add(*atom_iter);
          }
          }
          }
          Utopia::Node::atom_iterator atom_iter = complex->atomsBegin();
          Utopia::Node::atom_iterator atom_end = complex->atomsEnd();
          for (; atom_iter != atom_end; ++atom_iter) {
          // Create atom renderable
          atomRenderableManager->create(*atom_iter)->setDisplay(false);
          float * xyz = (*atom_iter)->getPosition();
          x += xyz[0];
          y += xyz[1];
          z += xyz[2];
          if (xyz[0] < minX) minX = xyz[0];
          if (xyz[1] < minY) minY = xyz[1];
          if (xyz[2] < minZ) minZ = xyz[2];
          if (xyz[0] > maxX) maxX = xyz[0];
          if (xyz[1] > maxY) maxY = xyz[1];
          if (xyz[2] > maxZ) maxZ = xyz[2];
          atomCount++;

          // set up selections
          getSelection(ATOMS).add(*atom_iter);
          if ((*atom_iter)->getElement()->isMetal())
          getSelection(METALS).add(*atom_iter);
          if ((*atom_iter)->getElement()->getSymbol() == "H")
          getSelection(HYDROGENS).add(*atom_iter);
          if ((*atom_iter)->getElement()->getSymbol() == "S")
          getSelection(SULPHUR).add(*atom_iter);
          }
*/

        // center complex
        x /= (float) atomCount;
        y /= (float) atomCount;
        z /= (float) atomCount;

        // compute maximum radius
        float longest_diagonal = sqrt((maxX - minX)*(maxX - minX) + (maxY - minY)*(maxY - minY) + (maxZ - minZ)*(maxZ - minZ));
        float centre_offset = sqrt(((maxX + minX) / 2.0 - x)*((maxX + minX) / 2.0 - x) + ((maxY + minY) / 2.0 - y)*((maxY + minY) / 2.0 - y) + ((maxZ + minZ) / 2.0 - z)*((maxZ + minZ) / 2.0 - z));
        r = longest_diagonal / 2.0 + centre_offset + 2.0;
    }
    float Ambrosia::getRadius()
    { return r; }
    void Ambrosia::incRefCount()
    { refCount++; }
    void Ambrosia::decRefCount()
    { if (refCount > 0) refCount--; }
    unsigned int Ambrosia::getRefCount()
    { return refCount; }
    void Ambrosia::clear()
    {
        selections.clear();
        if (complex != 0) {
            if (atomRenderableManager != 0)
                atomRenderableManager->clear();
            if (chainRenderableManager != 0)
                chainRenderableManager->clear();
            if (autoDelete) {
//                 cout << "clearing: " << complex << endl;
//                complex->clear();
//                 cout << "deleting: " << complex << endl;
                delete complex;
                complex = 0;
            }
//              std::cout << 5 << this << std::endl;
        }
    }
    void Ambrosia::setAutoDelete(bool flag)
    {
        cout << "setting autoDelete to " << flag << endl;
        autoDelete = flag;
    }
    void Ambrosia::orient()
    {
        glTranslatef(-x, -y, -z);
    }
    bool Ambrosia::built()
    {
        return this->_built;
    }

    // GL methods
    void Ambrosia::name()
    {
        // Render the naming pass
        render(NAME_PASS);
    }
    void Ambrosia::render()
    {
        // Render each of the passes required
        render(SHADOW_MAP_PASS);
        render(STENCIL_PASS);
        render(DRAW_PASS);
        render(DEPTH_SHADE_PASS);
        render(DRAW_SHADE_PASS);
        render(DEPTH_TRANSPARENT_PASS);
        render(DRAW_TRANSPARENT_PASS);
        render(DRAW_PASS);
    }
    void Ambrosia::render(Ambrosia::RenderPass pass)
    {
        // Push attribute stack
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        // Push Matrix stack
        glPushMatrix();
        orient();

        // Set up general state
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        switch (pass) {
        case SHADOW_MAP_PASS:
        {
            if (options[SHADOWS]) {
                // Set state
                glEnable(GL_STENCIL_TEST);
                glStencilFunc(GL_ALWAYS, 1, (unsigned int) -1);
                glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
                glDisable(GL_DEPTH_TEST);
                glDepthMask(GL_FALSE);
                glDisable(GL_BLEND);
                glDisable(GL_LIGHTING);
                glDisable(GL_COLOR_MATERIAL);
                glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                float spec[4] = {0.0, 0.0, 0.0, 0.0};
                glLightfv(GL_LIGHT0, GL_SPECULAR, spec);
                glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
                glMateriali(GL_FRONT, GL_SHININESS, 0);
            }
            break;
        }
        case STENCIL_PASS:
        {
            // Set state
            glEnable(GL_STENCIL_TEST);
            glStencilFunc(GL_ALWAYS, 1, (unsigned int) -1);
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
            glDisable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE);
            glDisable(GL_BLEND);
            glDisable(GL_LIGHTING);
            glDisable(GL_COLOR_MATERIAL);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            float spec[4] = {0.0, 0.0, 0.0, 0.0};
            glLightfv(GL_LIGHT0, GL_SPECULAR, spec);
            glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
            glMateriali(GL_FRONT, GL_SHININESS, 0);
            break;
        }
        case DEPTH_SHADE_PASS:
        case DEPTH_TRANSPARENT_PASS:
        {
            // Set state
            glDisable(GL_STENCIL_TEST);
            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);
            glDepthFunc(GL_LEQUAL);
            glDisable(GL_BLEND);
            glDisable(GL_LIGHTING);
            glDisable(GL_COLOR_MATERIAL);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            float spec[4] = {0.0, 0.0, 0.0, 0.0};
            glLightfv(GL_LIGHT0, GL_SPECULAR, spec);
            glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
            glMateriali(GL_FRONT, GL_SHININESS, 0);
            break;
        }
        case DRAW_SHADE_PASS:
        case DRAW_TRANSPARENT_PASS:
        {
            // Set state
            glDisable(GL_STENCIL_TEST);
            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);
            glDepthFunc(GL_EQUAL);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_LIGHTING);
            glEnable(GL_LIGHT0);
            float spec[4] = {0.0, 0.0, 0.0, 0.0};
            glLightfv(GL_LIGHT0, GL_SPECULAR, spec);
            glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
            glMateriali(GL_FRONT, GL_SHININESS, 0);
            glEnable(GL_COLOR_MATERIAL);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            break;
        }
        case DRAW_PASS:
        {
            // Set state
            glDisable(GL_STENCIL_TEST);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
            glEnable(GL_LIGHTING);
            glEnable(GL_LIGHT0);
            if (options[SPECULAR]) {
                float spec_l[4] = {0.7, 0.7, 0.7, 1.0};
                float spec_m[4] = {0.7, 0.7, 0.7, 1.0};
                glLightfv(GL_LIGHT0, GL_SPECULAR, spec_l);
                glMaterialfv(GL_FRONT, GL_SPECULAR, spec_m);
                glMateriali(GL_FRONT, GL_SHININESS, 30);
            }
            glEnable(GL_COLOR_MATERIAL);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            break;
        }
        case DRAW_OUTLINE_PASS:
        {
            // Set state
            glEnable(GL_STENCIL_TEST);
            glEnable(GL_LINE_SMOOTH);
            glStencilFunc(GL_NOTEQUAL, 1, (unsigned int) -1);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
            glLineWidth(5);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);
            glDisable(GL_LIGHTING);
            glDisable(GL_LIGHT0);
            float spec[4] = {0.0, 0.0, 0.0, 0.0};
            glLightfv(GL_LIGHT0, GL_SPECULAR, spec);
            glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
            glMateriali(GL_FRONT, GL_SHININESS, 0);
            glDisable(GL_COLOR_MATERIAL);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            break;
        }
        case NAME_PASS:
        {
            // Set state
            glDisable(GL_STENCIL_TEST);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
            glDisable(GL_LIGHTING);
            glDisable(GL_COLOR_MATERIAL);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            float spec[4] = {0.0, 0.0, 0.0, 0.0};
            glLightfv(GL_LIGHT0, GL_SPECULAR, spec);
            glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
            glMateriali(GL_FRONT, GL_SHININESS, 0);
            break;
        }
        }

//         std::vector< gtl::matrix_4d >::iterator miter = complex->getAttribute< std::vector< gtl::matrix_4d > >("BIOMT").begin();
//         std::vector< gtl::matrix_4d >::iterator mend = complex->getAttribute< std::vector< gtl::matrix_4d > >("BIOMT").end();
//         for (; miter != mend; ++miter)
//         {
//             glPushMatrix();
//             gtl::glMultMatrix(*miter);

        // Render each element of the scene
        if (chainRenderableManager) chainRenderableManager->render(pass);
        if (atomRenderableManager) atomRenderableManager->render(pass);

//             glPopMatrix();
//         }

        // Pop attribute / matrix stacks
        glPopMatrix();
        glPopAttrib();
    }
    void Ambrosia::applyCommand(Command * command, Utopia::Node * node)
    {
        if (command == 0 || node == 0)
            return;

        if (chainRenderableManager && (node->type() == Utopia::Node::getNode("chain") || node->type() == Utopia::Node::getNode("aminoacid"))) {
            Renderable * renderable = chainRenderableManager->get(node);
            if (renderable) command->apply(renderable);
        }

        if (atomRenderableManager && node->type()->relations(Utopia::rdfs.subClassOf).front() == Utopia::UtopiaDomain.term("Element")) {
            Renderable * renderable = atomRenderableManager->get(node);
            if (renderable) command->apply(renderable);
        }

        // Selection's children
/*        Utopia::Node::child_iterator node_iter = node->childrenBegin();
          Utopia::Node::child_iterator node_end = node->childrenEnd();
          for (; node_iter != node_end; ++node_iter) {
          applyCommand(command, *node_iter);
          } */
    }
    void Ambrosia::applyCommand(Command * command, RenderSelection renderSelection, Selection * selection)
    {
        if (command == 0)
            return;

        if (renderSelection == CUSTOM) {
            if (selection == 0)
                return;
        } else
            selection = &selections[renderSelection];

        // Selection's nodes
        set< Utopia::Node * >::iterator selection_iter = selection->nodes.begin();
        set< Utopia::Node * >::iterator selection_end = selection->nodes.end();
        for (; selection_iter != selection_end; ++selection_iter) {
            applyCommand(command, *selection_iter);
        }

    }
    void Ambrosia::setDisplay(bool display, RenderSelection renderSelection, Selection * selection)
    {
        SetDisplay command(display);
        applyCommand(&command, renderSelection, selection);
    }
    void Ambrosia::setVisible(bool visible, RenderSelection renderSelection, Selection * selection)
    {
        SetVisible command(visible);
        applyCommand(&command, renderSelection, selection);
    }
    void Ambrosia::setRenderFormat(unsigned int renderFormat, RenderSelection renderSelection, Selection * selection)
    {
        SetRenderFormat command(renderFormat);
        applyCommand(&command, renderSelection, selection);
    }
    void Ambrosia::setRenderOption(unsigned int renderOption, bool flag, RenderSelection renderSelection, Selection * selection)
    {
        SetRenderOption command(renderOption, flag);
        applyCommand(&command, renderSelection, selection);
    }
    void Ambrosia::setColour(Colour * colour, RenderSelection renderSelection, Selection * selection)
    {
        SetColour command(colour);
        applyCommand(&command, renderSelection, selection);
    }
    void Ambrosia::setAlpha(unsigned char alpha, RenderSelection renderSelection, Selection * selection)
    {
        SetAlpha command(alpha);
        applyCommand(&command, renderSelection, selection);
    }
    void Ambrosia::setTintColour(Colour * colour, RenderSelection renderSelection, Selection * selection)
    {
        SetTintColour command(colour);
        applyCommand(&command, renderSelection, selection);
    }
    void Ambrosia::setHighlightColour(Colour * colour, RenderSelection renderSelection, Selection * selection)
    {
        SetHighlightColour command(colour);
        applyCommand(&command, renderSelection, selection);
    }
    void Ambrosia::setRenderTag(RenderTag tag, RenderSelection renderSelection, Selection * selection)
    {
        SetRenderTag command(tag);
        applyCommand(&command, renderSelection, selection);
    }

    // Render options
    void Ambrosia::enable(Ambrosia::RenderOption option)
    { enable(option, true); }
    void Ambrosia::disable(Ambrosia::RenderOption option)
    { enable(option, false); }
    void Ambrosia::enable(Ambrosia::RenderOption option, bool enable)
    { options[option] = enable; }
    bool Ambrosia::isEnabled(Ambrosia::RenderOption option)
    { return options[option]; }

    // Internal methods
    void Ambrosia::init()
    {
        this->_built = false;

        // Populate colour map
        Colour::populate("ambrosia.colourmap");

        // Set default options
        options[SPECULAR] = true;
        options[SHADOWS] = false;

        // Initialise ambrosia
        complex = 0;
        x = 0.0;
        y = 0.0;
        z = 0.0;
        r = 0.0;
        lod = 1.0;
        autoDelete = false;
        refCount = 0;

        // Initialise renderable managers
        atomRenderableManager = 0;
        chainRenderableManager = 0;
    }
    Selection & Ambrosia::getSelection(RenderSelection renderSelection)
    {
        if (selections.find(renderSelection) == selections.end())
            selections[renderSelection] = Selection();
        return selections[renderSelection];
    }

    unsigned int Ambrosia::getToken(string className, string tokenName)
    {
        if (tokens[className].find(tokenName) == tokens[className].end())
            return tokens[className][tokenName] = nextToken++;
        else
            return tokens[className][tokenName];
    }
    map< string, unsigned int > Ambrosia::getTokens(string className)
    {
        if (tokens.find(className) == tokens.end()) {
            return map< string, unsigned int >();
        } else {
            return tokens[className];
        }
    }
    string Ambrosia::getTokenName(unsigned int token)
    { return ""; }
    string Ambrosia::getTokenClass(unsigned int token)
    { return ""; }

    map< string, map< string, unsigned int > > Ambrosia::tokens;
    unsigned int Ambrosia::nextToken = 1;

    // General functions
    string versionString()
    { return "GOD KNOWS"; }

} // namespace AMBROSIA
