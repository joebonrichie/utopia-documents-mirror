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

#ifndef GTL_BUFFER_INCL_
#define GTL_BUFFER_INCL_

typedef unsigned int GLenum;

#include <gtl/config.h>
#include <gtl/gl.h>
#include <gtl/extent.h>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <stdexcept>

namespace gtl
{
    /**
     *  \class cell
     *  \brief cell struct representing a buffer item.
     *
     *  Vertex buffers can hold an arbitrary amount of data per vertex (i.e.
     *  any of position, normal, colour, texture coordinate, edge flags and
     *  vertex indices). A %cell encapsulates up to four of these six
     *  componenets into a struct that can be accessed through the get/set
     *  methods.
     */
    template< typename _A, typename _B = void, typename _C = void, typename _D = void > class cell
    {
        // Convenience typedef
        typedef cell< _A, _B, _C, _D > _Self;

    public:
        /**  \name  Construction and destruction  */
        //@{

        /**
         *  \brief  Default constructor.
         *
         *  This constructor relies on the default constructors of the individual component
         *  types that comprise the cell.
         */
        cell()
            {}

        /**
         *  \brief  Copy constructor.
         *
         *  This constructor initialises each component individually according to the
         *  passed parameters.
         */
        cell(const _Self & rhs_)
            { this->operator=(rhs_); }

        /**
         *  \brief  Explicit constructor.
         *
         *  This constructor initialises each component individually according to the
         *  passed parameters.
         */
        template< typename _G, typename _H, typename _I, typename _J >
        cell(const _G & g_, const _H & h_, const _I & i_, const _J & j_)
            { this->set(g_, h_, i_, j_); }

        //@}
        /**  \name  Component access and manipulation.  */
        //@{

        /**  Set first component to \a a_.  */
        void set(const _A & a_)
            { this->_a = a_; }

        /**  Set second component to \a b_.  */
        void set(const _B & b_)
            { this->_b = b_; }

        /**  Set third component to \a c_.  */
        void set(const _C & c_)
            { this->_c = c_; }

        /**  Set fourth component to \a d_.  */
        void set(const _D & d_)
            { this->_d = d_; }

        /**  Set two components at once.  */
        template< typename _G, typename _H >
        void set(const _G & g_, const _H & h_)
            {
                this->set(g_);
                this->set(h_);
            }

        /**  Set three components at once.  */
        template< typename _G, typename _H, typename _I >
        void set(const _G & g_, const _H & h_, const _I & i_)
            {
                this->set(g_);
                this->set(h_);
                this->set(i_);
            }

        /**  Set four components at once.  */
        template< typename _G, typename _H, typename _I, typename _J >
        void set(const _G & g_, const _H & h_, const _I & i_, const _J & j_)
            {
                this->set(g_);
                this->set(h_);
                this->set(i_);
                this->set(j_);
            }

        /**  Return the first component.  */
        void get(_A & a_)
            { a_ = this->_a; }

        /**  Return the second component.  */
        void get(_B & b_)
            { b_ = this->_b; }

        /**  Return the third component.  */
        void get(_C & c_)
            { c_ = this->_c; }

        /**  Return the fourth component.  */
        void get(_D & d_)
            { d_ = this->_d; }

        /**  Return two components at once.  */
        template< typename _G, typename _H >
        void get(_G & g_, _H & h_)
            {
                this->get(g_);
                this->get(h_);
            }

        /**  Return three components at once.  */
        template< typename _G, typename _H, typename _I >
        void get(_G & g_, _H & h_, _I & i_)
            {
                this->get(g_);
                this->get(h_);
                this->get(i_);
            }

        /**  Return four components at once.  */
        template< typename _G, typename _H, typename _I, typename _J >
        void get(_G & g_, _H & h_, _I & i_, _J & j_)
            {
                this->get(g_);
                this->get(h_);
                this->get(i_);
                this->get(j_);
            }

        //@{
        /**  \name  GL commands.  */
        //@{

        /**  Execute GL commands of components.  */
        void gl()
            {
                this->_a.gl();
                this->_b.gl();
                this->_c.gl();
                this->_d.gl();
            }

        //@}

        // Components
        _A _a;
        _B _b;
        _C _c;
        _D _d;
    };
    template< typename _A > class cell< _A, void, void, void >
    {
        typedef cell< _A > _Self;

    public:
        cell()
            {}

        cell(const _Self & rhs_)
            { this->operator=(rhs_); }

        cell(const _A & a_)
            { this->set(a_); }

        void set(const _A & a_)
            { this->_a = a_; }

        void get(_A & a_)
            { a_ = this->_a; }

        void gl()
            { this->_a.gl(); }

        _A _a;
    };
    template< typename _A, typename _B > class cell< _A, _B, void, void >
    {
        typedef cell< _A, _B > _Self;

    public:
        cell()
            {}

        cell(const _Self & rhs_)
            { this->operator=(rhs_); }

        template< typename _G, typename _H >
        cell(const _G & g_, const _H & h_)
            { this->set(g_, h_); }

        void set(const _A & a_)
            { this->_a = a_; }

        void set(const _B & b_)
            { this->_b = b_; }

        template< typename _G, typename _H >
        void set(const _G & g_, const _H & h_)
            {
                this->set(g_);
                this->set(h_);
            }

        void get(_A & a_)
            { a_ = this->_a; }

        void get(_B & b_)
            { b_ = this->_b; }

        template< typename _G, typename _H >
        void get(_G & g_, _H & h_)
            {
                this->get(g_);
                this->get(h_);
            }

        void gl()
            {
                this->_a.gl();
                this->_b.gl();
            }

        _A _a;
        _B _b;
    };
    template< typename _A, typename _B, typename _C > class cell< _A, _B, _C, void >
    {
        typedef cell< _A, _B, _C > _Self;

    public:
        cell()
            {}

        cell(const _Self & rhs_)
            { this->operator=(rhs_); }

        template< typename _G, typename _H, typename _I >
        cell(const _G & g_, const _H & h_, const _I & i_)
            { this->set(g_, h_, i_); }

        void set(const _A & a_)
            { this->_a = a_; }

        void set(const _B & b_)
            { this->_b = b_; }

        void set(const _C & c_)
            { this->_c = c_; }

        template< typename _G, typename _H >
        void set(const _G & g_, const _H & h_)
            {
                this->set(g_);
                this->set(h_);
            }

        template< typename _G, typename _H, typename _I >
        void set(const _G & g_, const _H & h_, const _I & i_)
            {
                this->set(g_);
                this->set(h_);
                this->set(i_);
            }

        void get(_A & a_)
            { a_ = this->_a; }

        void get(_B & b_)
            { b_ = this->_b; }

        void get(_C & c_)
            { c_ = this->_c; }

        template< typename _G, typename _H >
        void get(_G & g_, _H & h_)
            {
                this->get(g_);
                this->get(h_);
            }

        template< typename _G, typename _H, typename _I >
        void get(_G & g_, _H & h_, _I & i_)
            {
                this->get(g_);
                this->get(h_);
                this->get(i_);
            }

        void gl()
            {
                this->_a.gl();
                this->_b.gl();
                this->_c.gl();
            }

        _A _a;
        _B _b;
        _C _c;
    };


    template< typename _A, typename _B, typename _C >
    std::ostream & operator << (std::ostream & os, const cell< _A, _B, _C > & cell_)
    {
        os << cell_._a << " " <<  cell_._b << " " <<  cell_._c;
        return os;
    }
    template< typename _A, typename _B >
    std::ostream & operator << (std::ostream & os, const cell< _A, _B > & cell_)
    {
        os << cell_._a << " " <<  cell_._b;
        return os;
    }



    template< class _cellType >
    void _hard_reset(std::vector< _cellType > & buffer_, const _cellType & val_, const GLenum & mode_, const size_t offset = 0)
    {
        // Introduce degenerate points in order to affect a hard reset of primitives

        // Irrelevant if the buffer is empty
        if (buffer_.size() - offset <= 0) {
            return;
        }

        // Unwind partial primitives
        size_t size = buffer_.size() - offset;
        size_t pop = 0;
        switch (mode_) {
        case GL_TRIANGLES:
            pop = size % 3;
            break;
        case GL_TRIANGLE_STRIP:
            pop = size < 3 ? size : 0;
            break;
        case GL_QUAD_STRIP:
            pop = size < 4 ? size : size % 2;
            break;
        case GL_QUADS:
            pop = size % 4;
            break;
        default:
            break;
        }
        for (; pop > 0; --pop) {
            buffer_.pop_back();
        }

        // We can exit if the buffer is NOW empty
        if (buffer_.size() - offset <= 0) {
            return;
        }

        // Repeat previous point if necessary
        size_t push_previous = 0;
        switch (mode_) {
        case GL_TRIANGLE_STRIP:
            push_previous = ((buffer_.size() - offset - 1) % 2) + 1;
            break;
        case GL_QUAD_STRIP:
            push_previous = 2;
            break;
        default:
            break;
        }
        for (; push_previous > 0; --push_previous) {
            buffer_.push_back(buffer_.back());
        }

        // Repeat new point if necessary
        size_t push_next = 0;
        switch (mode_) {
        case GL_TRIANGLE_STRIP:
            push_next = 2;
            break;
        case GL_QUAD_STRIP:
            push_next = 2;
            break;
        default:
            break;
        }
        for (; push_next > 0; --push_next) {
            buffer_.push_back(val_);
        }
    }

    /**
       \class _physical_vertex_buffer_base
       \brief Base class for buffers, encapsulating the raw data and cursor management.
    */
    class _physical_vertex_buffer_base
    {
    public:
        _physical_vertex_buffer_base(const GLenum & = GL_TRIANGLE_STRIP);
        ~_physical_vertex_buffer_base();

        // Operators
        _physical_vertex_buffer_base & operator = (const _physical_vertex_buffer_base &);

        // Buffer methods
        GLenum defaultMode();
        void setDefaultMode(const GLenum &);
        void hard_reset(const GLenum & = (GLenum) -1);
        void taint(const extent_size_t & = extent_size_t(0, (size_t) -1));

    protected:
        // VBO members
        GLuint _buffer;
        extent_size_t _taint;

        // GL members
        GLenum _defaultMode;

        // Pending hard reset?
        bool _pendingHardReset;
        GLenum _pendingHardResetMode;

    }; // class _physical_vertex_buffer_base

    /**
       \class physical_vertex_buffer
       \brief A physical buffer that is capable of storing four components.
    */
    template< class _A, class _B = void, class _C = void, class _D = void >
    class physical_vertex_buffer : public _physical_vertex_buffer_base, public std::vector< cell< _A, _B, _C, _D > >
    {
        // Convenience typedef
        typedef physical_vertex_buffer< _A, _B, _C, _D > _Self;

    public:
        // Convenience typedefs
        typedef cell< _A, _B, _C, _D > value_type;
        typedef std::vector< value_type > vector_type;
        typedef size_t size_type;
        typedef typename vector_type::iterator iterator;
        typedef typename vector_type::const_iterator const_iterator;
        typedef typename vector_type::reverse_iterator reverse_iterator;
        typedef typename vector_type::const_reverse_iterator const_reverse_iterator;

        // Reveal shadowed methods
        using vector_type::assign;
        using vector_type::insert;
        using vector_type::push_back;

        /**  \name  Construction and destruction.  */
        //@{

        /**  Constructor of physical_vertex_buffer objects.  */
        physical_vertex_buffer(const GLenum & defaultMode_ = GL_TRIANGLE_STRIP)
            : _physical_vertex_buffer_base(defaultMode_), vector_type()
            {
                value_type dummy;
                if (GLEW_VERSION_1_5 || GLEW_ARB_vertex_buffer_object) {
                    this->_offset_b = reinterpret_cast<const GLvoid *>(reinterpret_cast<unsigned char *>(&dummy._b) - reinterpret_cast<unsigned char *>(&dummy));
                    this->_offset_c = reinterpret_cast<const GLvoid *>(reinterpret_cast<unsigned char *>(&dummy._c) - reinterpret_cast<unsigned char *>(&dummy));
                    this->_offset_d = reinterpret_cast<const GLvoid *>(reinterpret_cast<unsigned char *>(&dummy._d) - reinterpret_cast<unsigned char *>(&dummy));
                }
            }

        /**  Copy constructor of physical_vertex_buffer objects.  */
        physical_vertex_buffer(const _Self & rhs_)
            { *this->operator=(rhs_); }

        //@}
        /**  \name  Buffer operators.  */
        //@{

        /**  Assignment operator.  */
        _Self & operator = (const _Self & rhs_)
            {
                this->_offset_b = rhs_._offset_b;
                this->_offset_c = rhs_._offset_c;
                this->_offset_d = rhs_._offset_d;
                this->vector_type::operator=(rhs_);
                this->_physical_vertex_buffer_base::operator=(rhs_);
            }

        //@}
        /**  \name  STL methods.  */
        //@{

        /**
         *  \brief  Assigns a given value to a %Buffer.
         *  \param  num_  Number of elements to be assigned.
         *  \param  g_  First data component to be added.
         *  \param  h_  Second data component to be added.
         *  \param  i_  Third data component to be added.
         *  \param  j_  Fourth data component to be added.
         *
         *  This function fills a %Buffer with \a num_ copies of the given
         *  value.  Note that the assignment completely changes the
         *  %Buffer and that the resulting %Buffer's size is the same as
         *  the number of elements assigned.  Old data may be lost.
         */
        template< class _G, class _H, class _I, class _J >
        void assign(size_type num_, const _G & g_, const _H & h_, const _I & i_, const _J & j_)
            {
                value_type val;
                val.set(g_, h_, i_, j_);
                this->assign(num_, val);
            }

        /**
         *  \brief  Inserts given value into %Buffer before specified iterator.
         *  \param  loc_  An iterator into the %Buffer.
         *  \param  g_  First data component to be inserted.
         *  \param  h_  Second data component to be inserted.
         *  \param  i_  Third data component to be inserted.
         *  \param  j_  Fourth data component to be inserted.
         *  \return  An iterator that points to the inserted data.
         *
         *  This function will insert a copy of the given value before
         *  the specified location.
         */
        template< class _G, class _H, class _I, class _J >
        iterator insert(iterator loc_, const _G & g_, const _H & h_, const _I & i_, const _J & j_)
            {
                value_type val;
                val.set(g_, h_, i_, j_);
                return this->insert(loc_, val);
            }

        /**
         *  \brief  Inserts a number of copies of given data into the %Buffer.
         *  \param  loc_  An iterator into the %Buffer.
         *  \param  num_  Number of elements to be inserted.
         *  \param  g_  First data component to be inserted.
         *  \param  h_  Second data component to be inserted.
         *  \param  i_  Third data component to be inserted.
         *  \param  j_  Fourth data component to be inserted.
         *
         *  This function will insert a specified number of copies of
         *  the given data before the location specified by \a loc_.
         */
        template< class _G, class _H, class _I, class _J >
        void insert(iterator loc_, size_type num_, const _G & g_, const _H & h_, const _I & i_, const _J & j_)
            {
                value_type val;
                val.set(g_, h_, i_, j_);
                this->insert(loc_, num_, val);
            }

        /**
         *  \brief  Add data to the end of the %Buffer.
         *  \param  val_  Data to be added.
         *
         *  This is a typical stack operation.  The function creates an
         *  element at the end of the %Buffer and assigns the given data
         *  to it.  Due to the nature of a %Buffer this operation can be
         *  done in constant time if the %Buffer has preallocated space
         *  available.
         */
        void push_back(const value_type & val_)
            {
                if (this->_pendingHardReset) {
                    if (this->_pendingHardResetMode == (GLenum) -1) {
                        this->_pendingHardResetMode = this->_defaultMode;
                    }
                    _hard_reset(*this, val_, this->_pendingHardResetMode);
                    this->_pendingHardReset = false;
                }
                this->vector_type::push_back(val_);
            }

        /**
         *  \brief  Add data to the end of the %Buffer.
         *  \param  g_  First data component to be added.
         *  \param  h_  Second data component to be added.
         *  \param  i_  Third data component to be added.
         *  \param  j_  Fourth data component to be added.
         */
        template< class _G, class _H, class _I, class _J >
        void push_back(const _G & g_, const _H & h_, const _I & i_, const _J & j_)
            {
                value_type val;
                val.set(g_, h_, i_, j_);
                this->push_back(val);
            }

        //@}
        /**  \name  GL methods.  */
        //@{

        /**  Enable a particular buffer for use.  */
        void enable()
            {
                // Enable buffer
                _A::enable_client_state();
                _B::enable_client_state();
                _C::enable_client_state();
                _D::enable_client_state();

                // Bind + Flush
                if (GLEW_VERSION_1_5) {
                    if (!this->_buffer) {
                        ::glGenBuffers(1, &this->_buffer);
                        this->_taint = extent_size_t(0, (size_t) -1);
                    }
                    ::glBindBuffer(GL_ARRAY_BUFFER, this->_buffer);
                    if (this->_taint) {
                        if (this->_taint.min() == 0 && this->_taint.max() >= this->size()) {
                            ::glBufferData(GL_ARRAY_BUFFER, this->size() * sizeof(value_type), &this->front(), GL_STATIC_DRAW);
                        } else {
                            this->_taint.max(this->size());
                            ::glBufferSubData(GL_ARRAY_BUFFER, this->_taint.min() * sizeof(value_type), (this->_taint.max() - this->_taint.min()) * sizeof(value_type), &this->front());
                        }
                        this->_taint.clear();
                    }
                } else if (GLEW_ARB_vertex_buffer_object) {
                    if (!this->_buffer) {
                        ::glGenBuffersARB(1, &this->_buffer);
                        this->_taint = extent_size_t(0, (size_t) -1);
                    }
                    ::glBindBufferARB(GL_ARRAY_BUFFER, this->_buffer);
                    if (this->_taint) {
                        if (this->_taint.min() == 0 && this->_taint.max() >= this->size()) {
                            ::glBufferDataARB(GL_ARRAY_BUFFER, this->size() * sizeof(value_type), &this->front(), GL_STATIC_DRAW);
                        } else {
                            this->_taint.max(this->size());
                            ::glBufferSubDataARB(GL_ARRAY_BUFFER, this->_taint.min() * sizeof(value_type), (this->_taint.max() - this->_taint.min()) * sizeof(value_type), &this->front());
                        }
                        this->_taint.clear();
                    }
                }

                // Set pointers
                if (!(GLEW_VERSION_1_5 || GLEW_ARB_vertex_buffer_object)) {
                    this->_offset_b = static_cast<const GLvoid *>(reinterpret_cast<unsigned char *>(&this->front()) + sizeof(_A));
                    this->_offset_c = static_cast<const GLvoid *>(reinterpret_cast<unsigned char *>(&this->front()) + sizeof(_A) + sizeof(_B));
                    this->_offset_d = static_cast<const GLvoid *>(reinterpret_cast<unsigned char *>(&this->front()) + sizeof(_A) + sizeof(_B) + sizeof(_C));
                    _A::pointer(sizeof(value_type), static_cast<const GLvoid *>(&this->front()));
                } else {
                    this->_offset_b = reinterpret_cast<const GLvoid *>(sizeof(_A));
                    this->_offset_c = reinterpret_cast<const GLvoid *>(sizeof(_A) + sizeof(_B));
                    this->_offset_d = reinterpret_cast<const GLvoid *>(sizeof(_A) + sizeof(_B) + sizeof(_C));
                    _A::pointer(sizeof(value_type), 0);
                }
                _B::pointer(sizeof(value_type), this->_offset_b);
                _C::pointer(sizeof(value_type), this->_offset_c);
                _D::pointer(sizeof(value_type), this->_offset_d);
            }

        /**
         *  \brief  Draw a range within this buffer.
         *  \param  mode_  The GL mode in which to draw this buffer.
         *  \param  first_  The index of the first vertex to draw.
         *  \param  count_  The number of vertices to draw.
         */
        void draw(const GLenum & mode_, const size_t & first_, size_t count_)
            {
                if (count_ == (size_t) -1) {
                    count_ = this->size() - first_;
                }

                // Draw buffer
                ::glDrawArrays(mode_, first_, count_);
            }

        /**
         *  \brief  Draw the entire buffer.
         *  \param  mode_  The GL mode in which to draw this buffer.
         *
         *  This is an overloaded instance of the above method.
         */
        void draw(GLenum mode_ = (GLenum) -1)
            {
                if (mode_ == (GLenum) -1) {
                    mode_ = this->_defaultMode;
                }
                this->draw(this->_defaultMode, 0, (size_t) -1);
            }

        /**
         *  \brief  Draw a range within the buffer.
         *  \param  first_  The index of the first vertex to draw.
         *  \param  count_  The number of vertices to draw.
         *
         *  This is an overloaded instance of the above method.
         */
        void draw(const size_t & first_, const size_t & count_)
            { this->draw(this->_defaultMode, first_, count_); }

        /**  Disable a particular buffer for use.  */
        void disable()
            {
                // Enable buffer
                _A::disable_client_state();
                _B::disable_client_state();
                _C::disable_client_state();
                _D::disable_client_state();
            }

        /**
         *  \brief  Render a range within this buffer.
         *  \param  mode_  The GL mode in which to render this buffer.
         *  \param  first_  The index of the first vertex to render.
         *  \param  count_  The number of vertices to render.
         *
         *  This is a convenience method that performs an \c enable(),
         *  \c draw(), and \c disable().
         */
        void render(const GLenum & mode_, const size_t & first_, size_t count_)
            {
                // Render buffer
                this->enable();
                this->draw(mode_, first_, count_);
                this->disable();
            }

        /**
         *  \brief  Render the entire buffer.
         *  \param  mode_  The GL mode in which to render this buffer.
         *
         *  This is an overloaded instance of the above method.
         */
        void render(GLenum mode_ = (GLenum) -1)
            { this->render(this->_defaultMode, 0, (size_t) -1); }

        /**
         *  \brief  Render a range within the buffer.
         *  \param  first_  The index of the first vertex to render.
         *  \param  count_  The number of vertices to render.
         *
         *  This is an overloaded instance of the above method.
         */
        void render(const size_t & first_, const size_t & count_)
            { this->render(this->_defaultMode, first_, count_); }

        //@}

    private:
        // Offsets
        const GLvoid * _offset_b;
        const GLvoid * _offset_c;
        const GLvoid * _offset_d;
    };
    template< class _A > class physical_vertex_buffer< _A, void, void, void > : public _physical_vertex_buffer_base, public std::vector< cell< _A > >
    {
        typedef physical_vertex_buffer< _A > _Self;

    public:
        typedef cell< _A > value_type;
        typedef std::vector< value_type > vector_type;
        typedef size_t size_type;
        typedef typename vector_type::iterator iterator;
        typedef typename vector_type::const_iterator const_iterator;
        typedef typename vector_type::reverse_iterator reverse_iterator;
        typedef typename vector_type::const_reverse_iterator const_reverse_iterator;

        using vector_type::assign;
        using vector_type::insert;
        using vector_type::push_back;

        physical_vertex_buffer(const GLenum & defaultMode_ = GL_TRIANGLE_STRIP)
            : _physical_vertex_buffer_base(defaultMode_), vector_type()
            {}

        physical_vertex_buffer(const _Self & rhs_)
            { *this->operator=(rhs_); }

        void assign(size_type num_, const _A & a_)
            {
                value_type val;
                val.set(a_);
                this->assign(num_, val);
            }

        iterator insert(iterator loc_, const _A & a_)
            {
                value_type val;
                val.set(a_);
                return this->insert(loc_, val);
            }

        void insert(iterator loc_, size_type num_, const _A & a_)
            {
                value_type val;
                val.set(a_);
                this->insert(loc_, num_, val);
            }

        void push_back(const _A & a_)
            {
                value_type val;
                val.set(a_);
                this->push_back(val);
            }

        void push_back(const value_type & val_)
            {
                if (this->_pendingHardReset) {
                    if (this->_pendingHardResetMode == (GLenum) -1) {
                        this->_pendingHardResetMode = this->_defaultMode;
                    }
                    _hard_reset(*this, val_, this->_pendingHardResetMode);
                    this->_pendingHardReset = false;
                }
                this->vector_type::push_back(val_);
            }

        void enable()
            {
                // Enable buffer
                _A::enable_client_state();

                // Bind + Flush
                if (GLEW_VERSION_1_5) {
                    if (!this->_buffer) {
                        ::glGenBuffers(1, &this->_buffer);
                        this->_taint = extent_size_t(0, (size_t) -1);
                    }
                    ::glBindBuffer(GL_ARRAY_BUFFER, this->_buffer);
                    if (this->_taint) {
                        if (this->_taint.min() == 0 && this->_taint.max() >= this->size()) {
                            ::glBufferData(GL_ARRAY_BUFFER, this->size() * sizeof(value_type), &this->front(), GL_STATIC_DRAW);
                        } else {
                            this->_taint.max(this->size());
                            ::glBufferSubData(GL_ARRAY_BUFFER, this->_taint.min() * sizeof(value_type), (this->_taint.max() - this->_taint.min()) * sizeof(value_type), &this->front());
                        }
                        this->_taint.clear();
                    }
                } else if (GLEW_ARB_vertex_buffer_object) {
                    if (!this->_buffer) {
                        ::glGenBuffersARB(1, &this->_buffer);
                        this->_taint = extent_size_t(0, (size_t) -1);
                    }
                    ::glBindBufferARB(GL_ARRAY_BUFFER, this->_buffer);
                    if (this->_taint) {
                        if (this->_taint.min() == 0 && this->_taint.max() >= this->size()) {
                            ::glBufferDataARB(GL_ARRAY_BUFFER, this->size() * sizeof(value_type), &this->front(), GL_STATIC_DRAW);
                        } else {
                            this->_taint.max(this->size());
                            ::glBufferSubDataARB(GL_ARRAY_BUFFER, this->_taint.min() * sizeof(value_type), (this->_taint.max() - this->_taint.min()) * sizeof(value_type), &this->front());
                        }
                        this->_taint.clear();
                    }
                }

                // Set pointers
                if (!(GLEW_VERSION_1_5 || GLEW_ARB_vertex_buffer_object)) {
                    _A::pointer(sizeof(value_type), static_cast<const GLvoid *>(&this->front()));
                } else {
                    _A::pointer(sizeof(value_type), 0);
                }

            }

        void draw(const GLenum & mode_, const size_t & first_, size_t count_)
            {
                if (count_ == (size_t) -1) {
                    count_ = this->size() - first_;
                }

                // Render buffer
                ::glDrawArrays(mode_, first_, count_);
            }

        void draw(GLenum mode_ = (GLenum) -1)
            {
                if (mode_ == (GLenum) -1) {
                    mode_ = this->_defaultMode;
                }
                this->draw(this->_defaultMode, 0, (size_t) -1);
            }

        void draw(const size_t & first_, const size_t & count_)
            { this->draw(this->_defaultMode, first_, count_); }

        void disable()
            {
                // Disable buffer
                _A::disable_client_state();
            }

        void render(const GLenum & mode_, const size_t & first_, size_t count_)
            {
                // Render buffer
                this->enable();
                this->draw(mode_, first_, count_);
                this->disable();
            }

        void render(GLenum mode_ = (GLenum) -1)
            { this->render(this->_defaultMode, 0, (size_t) -1); }

        void render(const size_t & first_, const size_t & count_)
            { this->render(this->_defaultMode, first_, count_); }

    };
    template< class _A, class _B > class physical_vertex_buffer< _A, _B, void, void > : public _physical_vertex_buffer_base, public std::vector< cell< _A, _B > >
    {
        typedef physical_vertex_buffer< _A, _B > _Self;

    public:
        typedef cell< _A, _B > value_type;
        typedef std::vector< value_type > vector_type;
        typedef size_t size_type;
        typedef typename vector_type::iterator iterator;
        typedef typename vector_type::const_iterator const_iterator;
        typedef typename vector_type::reverse_iterator reverse_iterator;
        typedef typename vector_type::const_reverse_iterator const_reverse_iterator;

        using vector_type::assign;
        using vector_type::insert;
        using vector_type::push_back;

        physical_vertex_buffer(const GLenum & defaultMode_ = GL_TRIANGLE_STRIP)
            : _physical_vertex_buffer_base(defaultMode_), vector_type()
            {
                value_type dummy;
                if (GLEW_VERSION_1_5 || GLEW_ARB_vertex_buffer_object) {
                    this->_offset_b = reinterpret_cast<const GLvoid *>(reinterpret_cast<unsigned char *>(&dummy._b) - reinterpret_cast<unsigned char *>(&dummy));
                }
            }

        physical_vertex_buffer(const _Self & rhs_)
            { *this->operator=(rhs_); }

        template< class _G, class _H >
        void assign(size_type num_, const _G & g_, const _H & h_)
            {
                value_type val;
                val.set(g_, h_);
                this->assign(num_, val);
            }

        template< class _G, class _H >
        iterator insert(iterator loc_, const _G & g_, const _H & h_)
            {
                value_type val;
                val.set(g_, h_);
                return this->insert(loc_, val);
            }

        template< class _G, class _H >
        void insert(iterator loc_, size_type num_, const _G & g_, const _H & h_)
            {
                value_type val;
                val.set(g_, h_);
                this->insert(loc_, num_, val);
            }

        template< class _G, class _H >
        void push_back(const _G & g_, const _H & h_)
            {
                value_type val;
                val.set(g_, h_);
                this->push_back(val);
            }

        void push_back(const value_type & val_)
            {
                if (this->_pendingHardReset) {
                    if (this->_pendingHardResetMode == (GLenum) -1) {
                        this->_pendingHardResetMode = this->_defaultMode;
                    }
                    _hard_reset(*this, val_, this->_pendingHardResetMode);
                    this->_pendingHardReset = false;
                }
                this->vector_type::push_back(val_);
            }

        void enable()
            {
                // Enable buffer
                _A::enable_client_state();
                _B::enable_client_state();

                // Bind + Flush
                if (GLEW_VERSION_1_5) {
                    if (!this->_buffer) {
                        ::glGenBuffers(1, &this->_buffer);
                        this->_taint = extent_size_t(0, (size_t) -1);
                    }
                    ::glBindBuffer(GL_ARRAY_BUFFER, this->_buffer);
                    if (this->_taint) {
                        if (this->_taint.min() == 0 && this->_taint.max() >= this->size()) {
                            ::glBufferData(GL_ARRAY_BUFFER, this->size() * sizeof(value_type), &this->front(), GL_STATIC_DRAW);
                        } else {
                            this->_taint.max(this->size());
                            ::glBufferSubData(GL_ARRAY_BUFFER, this->_taint.min() * sizeof(value_type), (this->_taint.max() - this->_taint.min()) * sizeof(value_type), &this->front());
                        }
                        this->_taint.clear();
                    }
                } else if (GLEW_ARB_vertex_buffer_object) {
                    if (!this->_buffer) {
                        ::glGenBuffersARB(1, &this->_buffer);
                        this->_taint = extent_size_t(0, (size_t) -1);
                    }
                    ::glBindBufferARB(GL_ARRAY_BUFFER, this->_buffer);
                    if (this->_taint) {
                        if (this->_taint.min() == 0 && this->_taint.max() >= this->size()) {
                            ::glBufferDataARB(GL_ARRAY_BUFFER, this->size() * sizeof(value_type), &this->front(), GL_STATIC_DRAW);
                        } else {
                            this->_taint.max(this->size());
                            ::glBufferSubDataARB(GL_ARRAY_BUFFER, this->_taint.min() * sizeof(value_type), (this->_taint.max() - this->_taint.min()) * sizeof(value_type), &this->front());
                        }
                        this->_taint.clear();
                    }
                }

                // Set pointers
                if (!(GLEW_VERSION_1_5 || GLEW_ARB_vertex_buffer_object)) {
                    this->_offset_b = static_cast<const GLvoid *>(reinterpret_cast<unsigned char *>(&this->front()) + sizeof(_A));
                    _A::pointer(sizeof(value_type), static_cast<const GLvoid *>(&this->front()));
                } else {
                    this->_offset_b = reinterpret_cast<const GLvoid *>(sizeof(_A));
                    _A::pointer(sizeof(value_type), 0);
                }
                _B::pointer(sizeof(value_type), this->_offset_b);

            }

        void draw(const GLenum & mode_, const size_t & first_, size_t count_)
            {
                if (count_ == (size_t) -1) {
                    count_ = this->size() - first_;
                }

                // Render buffer
                ::glDrawArrays(mode_, first_, count_);
            }

        void draw(GLenum mode_ = (GLenum) -1)
            {
                if (mode_ == (GLenum) -1) {
                    mode_ = this->_defaultMode;
                }
                this->draw(this->_defaultMode, 0, (size_t) -1);
            }

        void draw(const size_t & first_, const size_t & count_)
            { this->draw(this->_defaultMode, first_, count_); }

        void disable()
            {
                // Disable buffer
                _A::disable_client_state();
                _B::disable_client_state();
            }

        void render(const GLenum & mode_, const size_t & first_, size_t count_)
            {
                // Render buffer
                this->enable();
                this->draw(mode_, first_, count_);
                this->disable();
            }

        void render(GLenum mode_ = (GLenum) -1)
            { this->render(this->_defaultMode, 0, (size_t) -1); }

        void render(const size_t & first_, const size_t & count_)
            { this->render(this->_defaultMode, first_, count_); }

    private:
        const GLvoid * _offset_b;

    };
    template< class _A, class _B, class _C > class physical_vertex_buffer< _A, _B, _C, void > : public _physical_vertex_buffer_base, public std::vector< cell< _A, _B, _C > >
    {
        typedef physical_vertex_buffer< _A, _B, _C > _Self;

    public:
        typedef cell< _A, _B, _C > value_type;
        typedef std::vector< value_type > vector_type;
        typedef size_t size_type;
        typedef typename vector_type::iterator iterator;
        typedef typename vector_type::const_iterator const_iterator;
        typedef typename vector_type::reverse_iterator reverse_iterator;
        typedef typename vector_type::const_reverse_iterator const_reverse_iterator;

        using vector_type::assign;
        using vector_type::insert;
        using vector_type::push_back;

        physical_vertex_buffer(const GLenum & defaultMode_ = GL_TRIANGLE_STRIP)
            : _physical_vertex_buffer_base(defaultMode_), vector_type()
            {
                value_type dummy;
                if (GLEW_VERSION_1_5 || GLEW_ARB_vertex_buffer_object) {
                    this->_offset_b = reinterpret_cast<const GLvoid *>(reinterpret_cast<unsigned char *>(&dummy._b) - reinterpret_cast<unsigned char *>(&dummy));
                    this->_offset_c = reinterpret_cast<const GLvoid *>(reinterpret_cast<unsigned char *>(&dummy._c) - reinterpret_cast<unsigned char *>(&dummy));
                }
            }

        physical_vertex_buffer(const _Self & rhs_)
            { *this->operator=(rhs_); }

        template< class _G, class _H, class _I >
        void assign(size_type num_, const _G & g_, const _H & h_, const _I & i_)
            {
                value_type val;
                val.set(g_, h_, i_);
                this->assign(num_, val);
            }

        template< class _G, class _H, class _I >
        iterator insert(iterator loc_, const _G & g_, const _H & h_, const _I & i_)
            {
                value_type val;
                val.set(g_, h_, i_);
                return this->insert(loc_, val);
            }

        template< class _G, class _H, class _I >
        void insert(iterator loc_, size_type num_, const _G & g_, const _H & h_, const _I & i_)
            {
                value_type val;
                val.set(g_, h_, i_);
                this->insert(loc_, num_, val);
            }

        template< class _G, class _H, class _I >
        void push_back(const _G & g_, const _H & h_, const _I & i_)
            {
                value_type val;
                val.set(g_, h_, i_);
                this->push_back(val);
            }

        void push_back(const value_type & val_)
            {
                if (this->_pendingHardReset) {
                    if (this->_pendingHardResetMode == (GLenum) -1) {
                        this->_pendingHardResetMode = this->_defaultMode;
                    }
                    _hard_reset(*this, val_, this->_pendingHardResetMode);
                    this->_pendingHardReset = false;
                }
                this->vector_type::push_back(val_);
            }

        void enable()
            {
                // Enable buffer
                _A::enable_client_state();
                _B::enable_client_state();
                _C::enable_client_state();

                // Bind + Flush
                if (GLEW_VERSION_1_5) {
                    if (!this->_buffer) {
                        ::glGenBuffers(1, &this->_buffer);
                        this->_taint = extent_size_t(0, (size_t) -1);
                    }
                    ::glBindBuffer(GL_ARRAY_BUFFER, this->_buffer);
                    if (this->_taint) {
                        if (this->_taint.min() == 0 && this->_taint.max() >= this->size()) {
                            ::glBufferData(GL_ARRAY_BUFFER, this->size() * sizeof(value_type), &this->front(), GL_STATIC_DRAW);
                        } else {
                            this->_taint.max(this->size());
                            ::glBufferSubData(GL_ARRAY_BUFFER, this->_taint.min() * sizeof(value_type), (this->_taint.max() - this->_taint.min()) * sizeof(value_type), &this->front());
                        }
                        this->_taint.clear();
                    }
                } else if (GLEW_ARB_vertex_buffer_object) {
                    if (!this->_buffer) {
                        ::glGenBuffersARB(1, &this->_buffer);
                        this->_taint = extent_size_t(0, (size_t) -1);
                    }
                    ::glBindBufferARB(GL_ARRAY_BUFFER, this->_buffer);
                    if (this->_taint) {
                        if (this->_taint.min() == 0 && this->_taint.max() >= this->size()) {
                            ::glBufferDataARB(GL_ARRAY_BUFFER, this->size() * sizeof(value_type), &this->front(), GL_STATIC_DRAW);
                        } else {
                            this->_taint.max(this->size());
                            ::glBufferSubDataARB(GL_ARRAY_BUFFER, this->_taint.min() * sizeof(value_type), (this->_taint.max() - this->_taint.min()) * sizeof(value_type), &this->front());
                        }
                        this->_taint.clear();
                    }
                }

                // Set pointers
                if (!(GLEW_VERSION_1_5 || GLEW_ARB_vertex_buffer_object)) {
                    this->_offset_b = static_cast<const GLvoid *>(reinterpret_cast<unsigned char *>(&this->front()) + sizeof(_A));
                    this->_offset_c = static_cast<const GLvoid *>(reinterpret_cast<unsigned char *>(&this->front()) + sizeof(_A) + sizeof(_B));
                    _A::pointer(sizeof(value_type), static_cast<const GLvoid *>(&this->front()));
                } else {
                    this->_offset_b = reinterpret_cast<const GLvoid *>(sizeof(_A));
                    this->_offset_c = reinterpret_cast<const GLvoid *>(sizeof(_A) + sizeof(_B));
                    _A::pointer(sizeof(value_type), 0);
                }
                _B::pointer(sizeof(value_type), this->_offset_b);
                _C::pointer(sizeof(value_type), this->_offset_c);
            }

        void draw(const GLenum & mode_, const size_t & first_, size_t count_)
            {
                if (count_ == (size_t) -1) {
                    count_ = this->size() - first_;
                }

                // Render buffer
                ::glDrawArrays(mode_, first_, count_);
            }

        void draw(GLenum mode_ = (GLenum) -1)
            {
                if (mode_ == (GLenum) -1) {
                    mode_ = this->_defaultMode;
                }
                this->draw(this->_defaultMode, 0, (size_t) -1);
            }

        void draw(const size_t & first_, const size_t & count_)
            { this->draw(this->_defaultMode, first_, count_); }

        void disable()
            {
                // Disable buffer
                _A::disable_client_state();
                _B::disable_client_state();
                _C::disable_client_state();
            }

        void render(const GLenum & mode_, const size_t & first_, size_t count_)
            {
                if (count_ == (size_t) -1) {
                    count_ = this->size() - first_;
                }

                // Render buffer
                this->enable();
                this->draw(mode_, first_, count_);
                this->disable();
            }

        void render(GLenum mode_ = (GLenum) -1)
            { this->render(this->_defaultMode, 0, (size_t) -1); }

        void render(const size_t & first_, const size_t & count_)
            { this->render(this->_defaultMode, first_, count_); }

    private:
        const GLvoid * _offset_b;
        const GLvoid * _offset_c;

    }; // class physical_vertex_buffer3

    // OpenGL standard interleaved arrays
    typedef physical_vertex_buffer< vertex2f > physical_vertex_buffer_V2F;
    typedef physical_vertex_buffer< vertex3f > physical_vertex_buffer_V3F;
    typedef physical_vertex_buffer< color4ub, vertex2f > physical_vertex_buffer_C4UB_V2F;
    typedef physical_vertex_buffer< color4ub, vertex3f > physical_vertex_buffer_C4UB_V3F;
    typedef physical_vertex_buffer< color3f, vertex3f > physical_vertex_buffer_C3F_V3F;
    typedef physical_vertex_buffer< normal3f, vertex3f > physical_vertex_buffer_N3F_V3F;
    typedef physical_vertex_buffer< color4f, normal3f, vertex3f > physical_vertex_buffer_C4F_N3F_V3F;
    typedef physical_vertex_buffer< texcoord2f, vertex3f > physical_vertex_buffer_T2F_V3F;
    typedef physical_vertex_buffer< texcoord4f, vertex4f > physical_vertex_buffer_T4F_V4F;
    typedef physical_vertex_buffer< texcoord2f, color4ub, vertex3f > physical_vertex_buffer_T2F_C4UB_V3F;
    typedef physical_vertex_buffer< texcoord2f, color3f, vertex3f > physical_vertex_buffer_T2F_C3F_V3F;
    typedef physical_vertex_buffer< texcoord2f, normal3f, vertex3f > physical_vertex_buffer_T2F_N3F_V3F;
    typedef physical_vertex_buffer< texcoord2f, color4f, normal3f, vertex3f > physical_vertex_buffer_T2F_C4F_N3F_V3F;
    typedef physical_vertex_buffer< texcoord4f, color4f, normal3f, vertex4f > physical_vertex_buffer_T4F_C4F_N3F_V4F;
    // Other useful arrays
    typedef physical_vertex_buffer< normal3f > physical_vertex_buffer_N3F;
    typedef physical_vertex_buffer< color3f > physical_vertex_buffer_C3F;
    typedef physical_vertex_buffer< color4f > physical_vertex_buffer_C4F;
    typedef physical_vertex_buffer< color3ub > physical_vertex_buffer_C3UB;
    typedef physical_vertex_buffer< color4ub > physical_vertex_buffer_C4UB;

    // Forward...
    template< class _PhysicalBufferType >
    class _logical_vertex_buffer_iterator;
    template< class _PhysicalBufferType >
    class _logical_vertex_buffer_const_iterator;

    /**
     *  \class logical_vertex_buffer
     *  \brief A logical buffer that is capable of storing up to four components.
     */
    template< class _PhysicalBufferType > class logical_vertex_buffer
    {
        // Convenience typedefs
        typedef logical_vertex_buffer< _PhysicalBufferType > _Self;
        typedef std::set< size_t > RestartSet;
        typedef std::map< size_t, std::pair< _PhysicalBufferType *, RestartSet > > BufferMap;

    public:
        // Convenience typedefs
        typedef typename _PhysicalBufferType::value_type value_type;
        typedef typename _PhysicalBufferType::size_type size_type;
        typedef size_t difference_type;

        typedef _logical_vertex_buffer_iterator< _PhysicalBufferType > iterator;
        typedef _logical_vertex_buffer_const_iterator< _PhysicalBufferType > const_iterator;
        typedef std::reverse_iterator< iterator > reverse_iterator;
        typedef std::reverse_iterator< const_iterator > const_reverse_iterator;

        /**  \name  Construction and destruction.  */
        //@{

        /**  Default constructor creates no elements.  */
        logical_vertex_buffer(const GLenum defaultMode_ = GL_TRIANGLE_STRIP)
            : _suggestedBufferSize(0), _defaultMode(defaultMode_)
            {
                this->_add_buffer();
//                _suggestedBufferSize = 409600;
            }

        /**  Copy constructor clones another buffer.  */
        logical_vertex_buffer(const _Self & rhs_)
            { *this->operator=(rhs_); }

        /**  Constructor that initialises to a copy of a range from another buffer.  */
        template< class _IteratorType >
        logical_vertex_buffer(_IteratorType first_, _IteratorType last_)
            : _suggestedBufferSize(0), _defaultMode(0)
            { this->assign(first_, last_); }

        /**  Destructor, erase all elements.  */
        ~logical_vertex_buffer()
            {
                // Clear buffers
                this->_delete_all();
                this->_purge_death_row();
            }

        //@}
        /**  \name  STL vector operators.  */
        //@{

        /**
         *  \brief  %Vector assignment operator.
         *  \param  rhs_  A %Buffer of identical element and allocator types.
         *
         *  All the elements of \a rhs_ are copied, but any extra memory in
         *  \a rhs_ (for fast expansion) will not be copied.
         */
        _Self & operator = (const _Self & rhs_)
            {
                // Clear this logical buffer
                this->clear();

                // Shallow copy members
                this->_buffers = rhs_._buffers;
                this->_suggestedBufferSize = rhs_._suggestedBufferSize;
                this->_defaultMode = rhs_._defaultMode;

                // Deep copy physical buffers
                typename BufferMap::const_iterator iter = this->_buffers.begin();
                typename BufferMap::const_iterator end = this->_buffers.end();
                for (; iter != end; ++iter) {
                    iter->second.first = rhs_._buffers[iter->first].first;
                }

                return *this;
            }

        /**
         *  \brief  Subscript access to the data contained in the %Buffer.
         *  \param  index_  The index of the element for which data should be accessed.
         *  \return  Read/write reference to data.
         *
         *  This operator allows for easy, array-style, data access.
         *  Note that data access with this operator is unchecked and
         *  out_of_range lookups are not defined. (For checked lookups
         *  see at().)
         */
        value_type & operator [] (const size_type & index_)
            {
                _PhysicalBufferType * physicalBuffer;
                size_t physicalIndex;
                this->_map_index(index_, physicalBuffer, physicalIndex);
                return (*physicalBuffer)[physicalIndex];
            }

        /**
         *  \brief  Subscript access to the data contained in the %Buffer.
         *  \param  index_  The index of the element for which data should be accessed.
         *  \return  Read-only reference to data.
         *
         *  This operator allows for easy, array-style, data access.
         *  Note that data access with this operator is unchecked and
         *  out_of_range lookups are not defined. (For checked lookups
         *  see at().)
         */
        const value_type & operator [] (const size_type & index_) const
            {
                _PhysicalBufferType * physicalBuffer;
                size_t physicalIndex;
                this->_map_index(index_, physicalBuffer, physicalIndex);
                return (*physicalBuffer)[physicalIndex];
            }

        //@}
        /**  \name  STL vector methods.  */
        //@{

        /**
         *  Returns a read/write iterator that points to the first
         *  element in the %Buffer.  Iteration is done in ordinary
         *  element order.
         */
        iterator begin()
            { return iterator(this, 0); }

        /**
         *  Returns a read-only iterator that points to the first
         *  element in the %Buffer.  Iteration is done in ordinary
         *  element order.
         */
        const_iterator begin() const
            { return const_iterator(this, 0); }

        /**
         *  Returns a read/write iterator that points one past the last
         *  element in the %Buffer.  Iteration is done in ordinary
         *  element order.
         */
        iterator end()
            { return iterator(this, this->size()); }

        /**
         *  Returns a read-only iterator that points one past the last
         *  element in the %Buffer.  Iteration is done in ordinary
         *  element order.
         */
        const_iterator end() const
            { return const_iterator(this, this->size()); }

        /**
         *  Returns a read/write reverse iterator that points to the
         *  last element in the %Buffer.  Iteration is done in reverse
         *  element order.
         */
        reverse_iterator rbegin()
            { return reverse_iterator(iterator(this, this->size())); }

        /**
         *  Returns a read-only reverse iterator that points to the
         *  last element in the %Buffer.  Iteration is done in reverse
         *  element order.
         */
        const_reverse_iterator rbegin() const
            { return const_reverse_iterator(const_iterator(this, this->size())); }

        /**
         *  Returns a read/write reverse iterator that points to one
         *  before the first element in the %Buffer.  Iteration is done
         *  in reverse element order.
         */
        reverse_iterator rend()
            { return reverse_iterator(iterator(this, 0)); }

        /**
         *  Returns a read-only reverse iterator that points to one
         *  before the first element in the %Buffer.  Iteration is done
         *  in reverse element order.
         */
        const_reverse_iterator rend() const
            { return const_reverse_iterator(const_iterator(this, 0)); }

        /**
         *  \brief  Assigns a given value to a %Buffer.
         *  \param  num_  Number of elements to be assigned.
         *  \param  val_  Value to be assigned.
         *
         *  This function fills a %Buffer with \a num_ copies of the given
         *  value.  Note that the assignment completely changes the
         *  %Buffer and that the resulting %Buffer's size is the same as
         *  the number of elements assigned.  Old data may be lost.
         */
        void assign(size_type num_, const value_type & val_)
            {
                clear();
                this->_back_buffer().assign(num_, val_);
            }

        /**
         *  \brief  Assigns a range to a %Buffer.
         *  \param  first_  An input iterator.
         *  \param  last_  An input iterator.
         *
         *  This function fills a %Buffer with copies of the elements in the
         *  range [first,last). Note that the assignment completely changes
         *  the %Buffer and that the resulting %Buffer's size is the same as
         *  the number of elements assigned.  Old data may be lost.
         */
        template< typename _IteratorType >
        void assign(_IteratorType first_, _IteratorType last_)
            {
                clear();
                this->_back_buffer().assign(first_, last_);
            }

        /**
         *  \brief  Provides access to the data contained in the %Buffer.
         *  \param  index_  The index of the element for which data should be accessed.
         *  \return  Read/write reference to data.
         *  \throw  std::out_of_range  If \a index_ is an invalid index.
         *
         *  This function provides for safer data access.  The parameter is first
         *  checked that it is in the range of the vector.  The function throws
         *  out_of_range if the check fails.
         */
        value_type & at(const size_type & index_)
            {
                if (index_ >= this->size()) {
                    throw std::out_of_range("gtl::logical_vertex_buffer [] access out of range");
                }

                _PhysicalBufferType * physicalBuffer;
                size_t physicalIndex;
                this->_map_index(index_, physicalBuffer, physicalIndex);
                return physicalBuffer->at(physicalIndex);
            }

        /**
         *  \brief  Provides access to the data contained in the %Buffer.
         *  \param  index_  The index of the element for which data should be accessed.
         *  \return  Read-only reference to data.
         *  \throw  std::out_of_range  If \a index_ is an invalid index.
         *
         *  This function provides for safer data access.  The parameter is first
         *  checked that it is in the range of the vector.  The function throws
         *  out_of_range if the check fails.
         */
        const value_type & at(const size_type & index_) const
            {
                if (index_ >= this->size()) {
                    throw std::out_of_range("gtl::logical_vertex_buffer [] access out of range");
                }

                _PhysicalBufferType * physicalBuffer;
                size_t physicalIndex;
                this->_map_index(index_, physicalBuffer, physicalIndex);
                return physicalBuffer->at(physicalIndex);
            }

        /**
         *  Returns a read/write reference to the data at the last
         *  element of the %Buffer.
         */
        value_type & back()
            { return this->_back_buffer().back(); }

        /**
         *  Returns a read-only reference to the data at the last
         *  element of the %Buffer.
         */
        const value_type & back() const
            { return this->_back_buffer().back(); }

        /**
         *  Returns the total number of elements that the %Buffer can hold before
         *  needing to allocate more memory.
         */
        size_type capacity() const
            { return size_type(this->_buffers.rbegin()->first + this->_back_buffer().capacity()); }

        /**  Erases all the elements.  */
        void clear()
            {
                this->_delete_all();
                this->_add_buffer();
            }

        /**
         *  Returns true if the %Buffer is empty.
         */
        bool empty() const
            { return this->_front_buffer().empty(); }

        /**
         *  \brief  Remove element at given position.
         *  \param  loc_  Iterator pointing to element to be erased.
         *  \return  An iterator pointing to the next element (or end()).
         *
         *  This function will erase the element at the given position and thus
         *  shorten the %Buffer by one.
         *
         *  Note This operation could be expensive.
         */
        iterator erase(const iterator & loc_)
            {
                _PhysicalBufferType * physicalBuffer;
                size_t physicalIndex;
                size_t logicalIndex = loc_._index;
                this->_map_index(logicalIndex, physicalBuffer, physicalIndex);

                // Remove actual vertex
                physicalBuffer->erase(physicalBuffer->begin() + physicalIndex);

                // Is this physical buffer now empty?
                if (physicalBuffer->empty()) {
                    // If so, remove it
                    this->_deathRow.push_back(physicalBuffer);
                    this->_buffers.erase(logicalIndex - physicalIndex);
                } else {
                    // Otherwise move subsequent restarts
                    typename RestartSet::iterator restart_iter = this->_buffers[logicalIndex -  physicalIndex].second.lower_bound(physicalIndex + 1);
                    typename RestartSet::iterator restart_end = this->_buffers[logicalIndex - physicalIndex].second.end();
                    for (; restart_iter != restart_end; ++restart_iter) {
                        this->_buffers[logicalIndex - physicalIndex].second.erase(*restart_iter);
                        this->_buffers[logicalIndex - physicalIndex].second.insert(*restart_iter - 1);
                    }

                    // Then taint from this point until the end of the buffer
                    physicalBuffer->taint(extent_size_t(physicalIndex, (size_t) -1));
                }

                // Move subsequent buffer indices
                typename BufferMap::iterator buffer_iter = this->_buffers.lower_bound(logicalIndex - physicalIndex + 1);
                typename BufferMap::iterator buffer_end = this->_buffers.end();
                for (; buffer_iter != buffer_end;) {
                    this->_buffers[buffer_iter->first - 1] = this->_buffers[buffer_iter->first];
                    this->_buffers.erase(buffer_iter++);
                }

                return loc_;
            }

        /**
         *  \brief  Remove a range of elements.
         *  \param  first_  Iterator pointing to the first element to be erased.
         *  \param  last_  Iterator pointing to one past the last element to be
         *                erased.
         *  \return  An iterator pointing to the element pointed to by \a last_
         *           prior to erasing (or end()).
         *
         *  This function will erase the elements in the range [first,last) and
         *  shorten the %Buffer accordingly.
         *
         *  Note This operation could be expensive. FIXME
         */
        iterator erase(iterator first_, iterator last_)
            {
                return this->begin();
            }

        /**
         *  Returns a read/write reference to the data at the first
         *  element of the %Buffer.
         */
        value_type & front()
            { return this->_front_buffer().front(); }

        /**
         *  Returns a read-only reference to the data at the first
         *  element of the %Buffer.
         */
        const value_type & front() const
            { return this->_front_buffer().front(); }

        /**
         *  \brief  Inserts given value into %Buffer before specified iterator.
         *  \param  loc_  An iterator into the %Buffer.
         *  \param  val_  Data to be inserted.
         *  \return  An iterator that points to the inserted data.
         *
         *  This function will insert a copy of the given value before
         *  the specified location.  Note that this kind of operation
         *  could be expensive.
         */
        iterator insert(iterator loc_, const value_type & val_)
            {
                // Get physical buffer in which to add vertices
                typename BufferMap::iterator buffer = loc_._buffer;
                if (buffer == loc_._logicalBuffer->_buffers.end()) {
                    --buffer;
                }

                // Correct restart list
                typename RestartSet::iterator restart_iter = --buffer->second.second.end();
                typename RestartSet::iterator restart_end = buffer->second.second.lower_bound(loc_._index - buffer->first);
                for (; restart_iter != restart_end;) {
                    size_t restart = *restart_iter;
                    --restart_iter;
                    buffer->second.second.erase(restart);
                    buffer->second.second.insert(restart + 1);
                }

                // Insert element
                buffer->second.first->insert(loc_._element, val_);

                // Taint entire buffer
                buffer->second.first->taint();

                return iterator(this, loc_._index);
            }

        /**
         *  \brief  Inserts a number of copies of given data into the %Buffer.
         *  \param  loc_  An iterator into the %Buffer.
         *  \param  num_  Number of elements to be inserted.
         *  \param  val_  Data to be inserted.
         *
         *  This function will insert a specified number of copies of
         *  the given data before the location specified by \a position.
         *
         *  Note that this kind of operation could be expensive.
         */
        void insert(iterator loc_, size_type num_, const value_type & val_)
            {
                // Get physical buffer in which to add vertices
                typename BufferMap::iterator buffer = loc_._buffer;
                if (buffer == loc_._logicalBuffer->_buffers.end()) {
                    --buffer;
                }

                // Correct restart list
                typename RestartSet::iterator restart_iter = --buffer->second.second.end();
                typename RestartSet::iterator restart_end = buffer->second.second.lower_bound(loc_._index - buffer->first);
                for (; restart_iter != restart_end;) {
                    size_t restart = *restart_iter;
                    --restart_iter;
                    buffer->second.second.erase(restart);
                    buffer->second.second.insert(restart + num_);
                }

                // Insert element
                buffer->second.first->insert(loc_._element, num_, val_);

                // Taint entire buffer
                buffer->second.first->taint();
            }

        /**
         *  \brief  Inserts a range into the %Buffer.
         *  \param  loc_  An iterator into the %Buffer.
         *  \param  first_  An input iterator.
         *  \param  last_   An input iterator.
         *
         *  This function will insert copies of the data in the range
         *  [first,last) into the %Buffer before the location specified
         *  by \a loc_.
         *
         *  Note that this kind of operation could be expensive.
         */
        template< typename _IteratorType >
        void insert(iterator loc_, _IteratorType first_, _IteratorType last_)
            {
                // How many elements to add?
                size_t num = last_ - first_;

                // Get physical buffer in which to add vertices
                typename BufferMap::iterator buffer = loc_._buffer;
                if (buffer == loc_._logicalBuffer->_buffers.end()) {
                    --buffer;
                }

                // Correct restart list
                typename RestartSet::iterator restart_iter = --buffer->second.second.end();
                typename RestartSet::iterator restart_end = buffer->second.second.lower_bound(loc_._index - buffer->first);
                for (; restart_iter != restart_end;) {
                    size_t restart = *restart_iter;
                    --restart_iter;
                    buffer->second.second.erase(restart);
                    buffer->second.second.insert(restart + num);
                }

                // Insert element
                buffer->second.first->insert(loc_._element, first_, last_);

                // Taint entire buffer
                buffer->second.first->taint();
            }

        /**  Returns the size() of the largest possible %Buffer.  */
        size_type max_size() const
            { return size_type(-1) / sizeof(value_type); }

        /**
         *  \brief  Removes last element.
         *
         *  This is a typical stack operation. It shrinks the %Buffer by one.
         *
         *  Note that no data is returned, and if the last element's data is
         *  needed, it should be retrieved before pop_back() is called.
         */
        void pop_back()
            {
                _PhysicalBufferType * backBuffer = this->_buffers.rbegin()->second.first;
                backBuffer->pop_back();

                // remove restart if need be
                this->_buffers.rbegin()->second.second.erase(backBuffer->size() + 1);

                // Remove back buffer if need be
                if (this->_buffers.size() > 1 && backBuffer->empty()) {
                    // Add buffer to death row
                    this->_deathRow.push_back(backBuffer);
                    // Remove from main map
                    this->_buffers.erase(this->_buffers.rbegin()->first);
                }
            }

        /**
         *  \brief  Add data to the end of the %Buffer.
         *  \param  g_  Single data component to be added.
         *
         *  This is a typical stack operation.  The function creates an
         *  element at the end of the %Buffer and assigns the given data
         *  to it.  Due to the nature of a %Buffer this operation can be
         *  done in constant time if the %Buffer has preallocated space
         *  available.
         */
        template< class _G >
        void push_back(const _G & g_)
            {
                this->_back_buffer().push_back(g_);

                // Taint entire buffer
                this->_back_buffer().taint();
            }

        /**
         *  \brief  Add data to the end of the %Buffer.
         *  \param  g_  First data component to be added.
         *  \param  h_  Second data component to be added.
         *
         *  This is an overloaded instance of the above method.
         */
        template< class _G, class _H >
        void push_back(const _G & g_, const _H & h_)
            {
                this->_back_buffer().push_back(g_, h_);

                // Taint entire buffer
                this->_back_buffer().taint();
            }

        /**
         *  \brief  Add data to the end of the %Buffer.
         *  \param  g_  First data component to be added.
         *  \param  h_  Second data component to be added.
         *  \param  i_  Third data component to be added.
         *
         *  This is an overloaded instance of the above method.
         */
        template< class _G, class _H, class _I >
        void push_back(const _G & g_, const _H & h_, const _I & i_)
            {
                this->_back_buffer().push_back(g_, h_, i_);

                // Taint entire buffer
                this->_back_buffer().taint();
            }

        /**
         *  \brief  Add data to the end of the %Buffer.
         *  \param  g_  First data component to be added.
         *  \param  h_  Second data component to be added.
         *  \param  i_  Third data component to be added.
         *  \param  j_  Fourth data component to be added.
         *
         *  This is an overloaded instance of the above method.
         */
        template< class _G, class _H, class _I, class _J >
        void push_back(const _G & g_, const _H & h_, const _I & i_, const _J & j_)
            {
                this->_back_buffer().push_back(g_, h_, i_, j_);

                // Taint entire buffer
                this->_back_buffer().taint();
            }

        /**
         *  \brief  Attempt to preallocate enough memory for specified number of elements.
         *  \param  space_required_  Number of elements required.
         *  \throw  std::length_error  If \a n exceeds \c max_size().
         *
         *  This function attempts to reserve enough memory for the
         *  %Buffer to hold the specified number of elements.  If the
         *  number requested is more than max_size(), length_error is
         *  thrown.
         *
         *  The advantage of this function is that if optimal code is a
         *  necessity and the user can determine the number of elements
         *  that will be required, the user can reserve the memory in
         *  %advance, and thus prevent a possible reallocation of memory
         *  and copying of %Buffer data.
         */
        void reserve(size_type space_required_)
            {
                // Take account of the back buffer's index
                space_required_ -= this->_buffers.rbegin()->first;

                // Only reserve if this increases the size
                if (space_required_ > this->back_buffer().size()) {
                    this->back_buffer().reserve(space_required_);
                }
            }

        /**
         *  \brief  Resizes the %Buffer to the specified number of elements.
         *  \param  new_size_  Number of elements the %Buffer should contain.
         *  \param  val_  Data with which new elements should be populated.
         *
         *  This function will %resize the %Buffer to the specified
         *  number of elements.  If the number is smaller than the
         *  %Buffer's current size the %Buffer is truncated, otherwise
         *  the %Buffer is extended and new elements are populated with
         *  given data. FIXME
         */
        void resize(size_type new_size_, const value_type & val_)
            {
                // FIXME to include restarts
                if (new_size_ < this->size()) {
                    this->erase(this->begin() + new_size_, this->end());
                } else {
                    this->insert(this->end(), new_size_ - this->size(), val_);
                }
            }

        /**
         *  \brief  Resizes the %Buffer to the specified number of elements.
         *  \param  new_size_  Number of elements the %Buffer should contain.
         *
         *  This function will resize the %Buffer to the specified
         *  number of elements.  If the number is smaller than the
         *  %Buffer's current size the %Buffer is truncated, otherwise
         *  the %Buffer is extended and new elements are
         *  default-constructed.
         */
        void resize(size_type new_size_)
            { this->resize(new_size_, value_type()); }

        /**  Returns the number of elements in the %Buffer.  */
        size_type size() const
            { return size_type(this->_buffers.rbegin()->first + this->_back_buffer().size()); }

        /**
         *  \brief  Swaps data with another %Buffer.
         *  \param  rhs_  A %Buffer of the same element type.
         *
         *  This exchanges the elements between two buffers in constant time.
         */
        void swap(_Self & rhs_)
            {
                // Swap physical buffers
                this->_buffers.swap(rhs_._buffers);

                // Swap default mode
                GLenum defaultMode = this->_defaultMode;
                this->_defaultMode = rhs_->_defaultMode;
                rhs_->_defaultMode = defaultMode;
            }

        //@}
        /**  \name  Buffer methods.  */
        //@{

        /**
         *  \brief  Introduce a hard reset to the buffer.
         *  \param  mode_  GL mode in which this reset will occur.
         *
         *  Introduces degenerate points in order to affect a hard reset of the
         *  polygon stream.
         */
        void hard_reset(const GLenum & mode_ = (GLenum) -1)
            { this->_back_buffer().hard_reset(mode_); }

        /**
         *  \brief  Introduce a soft reset to the buffer.
         *
         *  Introduces a soft reset marker to the buffer so that rendering actually
         *  causes multiple glDrawArrays.
         */
        void soft_reset()
            { this->_buffers.rbegin()->second.second.insert(this->_back_buffer().size()); }

        /**
         *  \brief  Taints this buffer.
         *
         *  Marks this buffer as tainted (for VBO management).
         */
        void taint()
            {
                typename BufferMap::iterator buffer_iter = this->_buffers.begin();
                typename BufferMap::iterator buffer_end = this->_buffers.end();
                for (; buffer_iter != buffer_end; ++buffer_iter) {
                    buffer_iter->second.first->taint();
                }
            }

        //@}
        /**  \name  GL methods.  */
        //@{

        /**  Returns the current default GL mode.  */
        GLenum defaultMode()
            { return this->_defaultMode; }

        /**  Sets the current default GL mode to \a defaultMode_.  */
        void setDefaultMode(const GLenum & defaultMode_)
            { this->_defaultMode = defaultMode_; }

        /**
         *  \brief  Render a range within this buffer.
         *  \param  mode_  The GL mode in which to render this buffer.
         *  \param  first_  The index of the first vertex to render.
         *  \param  count_  The number of vertices to render.
         *
         *  Renders the specified range of elements within this buffer.
         *  This method may execute numerous GL calls in order to piece
         *  together the internally hidden physical buffers.
         */
        void render(const GLenum & mode_, const size_t & first_, size_t count_)
            {
                // Clear up death row if need be
                this->_purge_death_row();

                // Find suggested buffer size from GL
                if (this->_suggestedBufferSize == 0) {
                    // Find maximum buffer size
                    GLint maxElementsVertices;
                    ::glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &maxElementsVertices);
                    this->_suggestedBufferSize = (size_t) maxElementsVertices;
                }

                // If any buffer exceeds this size, decompose it into smaller
                // buffers
                typename BufferMap::const_iterator buffer = this->_buffers.begin();
                typename BufferMap::const_iterator buffer_end = this->_buffers.end();
                for (; buffer != buffer_end; ++buffer) {
                    if (buffer->second.first->size() > this->_suggestedBufferSize) {
                        this->_decompose_buffer(buffer->first);
                    }
                }

                // Logical index of last buffer
                size_t firstIndex;
                _PhysicalBufferType * firstBuffer;
                size_t firstBufferIndex = 0;

                size_t lastIndex;
                _PhysicalBufferType * lastBuffer;
                size_t lastBufferIndex = 0;

                // Get first buffer...
                this->_map_index(first_, firstBuffer, firstIndex);
                firstBufferIndex = first_ - firstIndex;

                // Fix count_
                if (count_ == (size_t) -1) {
                    lastBufferIndex = this->_buffers.rbegin()->first;
                    lastBuffer = &this->_back_buffer();
                    count_ = lastBufferIndex + lastBuffer->size() - first_;
                    lastIndex = lastBuffer->size() - 1;
                } else {
                    this->_map_index(first_ + count_ - 1, lastBuffer, lastIndex);
                    lastBufferIndex = first_ + count_ - 1 - lastIndex;
                }

//                std::cout << "logical_vertex_buffer::render(" << first_ << ", " << count_ << ")" << std::endl;
//                std::cout << this->_suggestedBufferSize << std::endl;
//                std::cout << count_ << std::endl;

                // Draw buffers
                size_t skip = 0;
                buffer = this->_buffers.find(firstBufferIndex);
                buffer_end = this->_buffers.find(lastBufferIndex);
                do {
                    buffer->second.first->enable();
                    size_t first = skip;
                    size_t count = 0;

                    // First buffer?
                    if (buffer->first == firstBufferIndex) {
                        first = firstIndex;
                    }

                    // Last buffer?
                    if (buffer->first == lastBufferIndex) {
                        count = lastIndex + 1 - first;
                    } else {
                        count = buffer->second.first->size() - first;
                    }

                    // Complete buffer?
                    if (buffer->second.second.empty() || (buffer->second.second.size() == 1 && *buffer->second.second.begin() == buffer->second.first->size())) {
                        buffer->second.first->draw(mode_, first, count);
                    } else {
                        typename RestartSet::const_iterator restart = buffer->second.second.begin();
                        typename RestartSet::const_iterator restart_end = buffer->second.second.end();
                        bool finished;
                        do {
                            finished = false;

                            if (restart == restart_end) {
                                count = buffer->second.first->size() - first;
                            } else {
                                count = *restart - first;
                            }
                            if (buffer->first == lastBufferIndex && first + count - 1 > lastIndex) {
                                count = lastIndex + 1 - first;
                                finished = true;
                            }
                            if (count > 0) {
                                buffer->second.first->draw(mode_, first, count);
                            }
                            first += count;
                        } while (restart++ != restart_end && !finished);
                    }

                    // if not at end, then BRIDGE
                    typename BufferMap::const_iterator buffer_next = buffer;
                    ++buffer_next;
                    if (buffer->first != lastBufferIndex) {
                        skip = _bridge_buffer(buffer->first, buffer_next->first, mode_);
                    }
                    buffer->second.first->disable();
                } while (buffer++ != buffer_end);
            }

        /**
         *  \brief  Render the entire buffer.
         *  \param  mode_  The GL mode in which to render this buffer.
         *
         *  This is an overloaded instance of the above method.
         */
        void render(GLenum mode_ = (GLenum) -1)
            {
                if (mode_ == (GLenum) -1) {
                    mode_ = this->_defaultMode;
                }
                render(this->_defaultMode, 0, (size_t) -1);
            }

        /**
         *  \brief  Render a range within the buffer.
         *  \param  first_  The index of the first vertex to render.
         *  \param  count_  The number of vertices to render.
         *
         *  This is an overloaded instance of the above method.
         */
        void render(const size_t & first_, const size_t & count_)
            { render(this->_defaultMode, first_, count_); }

        //@}

    private:
        //  Buffer data structure.
        BufferMap _buffers;
        //  List of physical buffers that are due for deletion during the next rendering pass.
        std::list< _PhysicalBufferType * > _deathRow;
        //  GL's suggested buffer size (in vertices).
        size_t _suggestedBufferSize;

        //  Default GL draw mode.
        GLenum _defaultMode;

        // Buffer management methods
        _PhysicalBufferType * _add_buffer(size_t bufferIndex_ = (size_t) -1)
            {
                // Find index for this buffer...
                if (bufferIndex_ == (size_t) -1) {
                    bufferIndex_ = 0;
                    if (this->_buffers.size() > 1) {
                        bufferIndex_ = this->_buffers.rbegin()->first;
                    }
                    if (this->_buffers.size() > 0) {
                        bufferIndex_ += this->_buffers.rbegin()->second.first->size();
                    }
                }

                _PhysicalBufferType * newBuffer = new _PhysicalBufferType(this->_defaultMode);
                this->_buffers[bufferIndex_] = std::make_pair(newBuffer, RestartSet());
                return newBuffer;
            }

        void _decompose_buffer(const size_t & bufferIndex_)
            {
                // Current back buffer (the one to be curtailed)
                _PhysicalBufferType * buffer = this->_buffers[bufferIndex_].first;
                RestartSet * restarts = &this->_buffers[bufferIndex_].second;

                // Start / length of the new buffer
                size_t newBufferIndex = bufferIndex_ + this->_suggestedBufferSize;
                size_t newBufferLength = this->_suggestedBufferSize;

                // Iterators for later element insertion
                typename _PhysicalBufferType::const_iterator iter = buffer->begin() + this->_suggestedBufferSize;
                typename _PhysicalBufferType::const_iterator iter_end(iter);

                do {
                    // Add new buffer and get a handle to it
                    _PhysicalBufferType * newBuffer = this->_add_buffer(newBufferIndex);

                    // Copy vertex information
                    iter_end += newBufferLength;
                    if (buffer->end() < iter_end) {
                        iter_end = buffer->end();
                    }
                    newBuffer->insert(newBuffer->end(), iter, iter_end);
                    iter = iter_end;

                    // Copy resets
                    typename RestartSet::const_iterator restart_iter = restarts->lower_bound(newBufferIndex - bufferIndex_);
                    typename RestartSet::const_iterator restart_end = restarts->end();
                    while (restart_iter != restart_end && (*restart_iter - newBufferIndex) < this->_suggestedBufferSize) {
                        size_t restart = *restart_iter;
                        ++restart_iter;
                        restarts->erase(restart);
                        this->_buffers[newBufferIndex].second.insert(restart - newBufferIndex);
                    }

                    // Next buffer...
                    newBufferIndex += this->_suggestedBufferSize;
                } while (newBufferIndex <= buffer->size());

                // Remove duplicates
                buffer->resize(this->_suggestedBufferSize);
            }

        inline _PhysicalBufferType & _back_buffer() const
            { return *this->_buffers.rbegin()->second.first; }

        inline _PhysicalBufferType & _front_buffer() const
            { return *this->_buffers.begin()->second.first; }

        void _delete_all()
            {
                typename BufferMap::const_iterator iter = this->_buffers.begin();
                typename BufferMap::const_iterator iter_end = this->_buffers.end();
                for (; iter != iter_end; ++iter) {
                    this->_deathRow.push_back(iter->second.first);
                }
                this->_buffers.clear();
            }

        void _purge_death_row()
            {
                // Clear death row
                if (!this->_deathRow.empty()) {
                    typename std::list< _PhysicalBufferType * >::iterator iter = this->_deathRow.begin();
                    typename std::list< _PhysicalBufferType * >::iterator end = this->_deathRow.end();
                    for (; iter != end; ++iter) {
                        delete *iter;
                    }
                    this->_deathRow.clear();
                }
            }

        void _map_index(const size_t & lindex_, _PhysicalBufferType * & buffer_, size_t & pindex_) const
            {
                typename BufferMap::const_iterator upper = this->_buffers.upper_bound(lindex_);
                typename BufferMap::const_iterator lower = --upper;
                buffer_ = lower->second.first;
                pindex_ = lindex_ - lower->first;
                if (pindex_ > buffer_->size()) {
                    pindex_ = buffer_->size();
                }
            }

        size_t _bridge_buffer(const size_t & from_, const size_t & to_, const GLenum & mode_)
            {
                _PhysicalBufferType * fromBuffer = this->_buffers.find(from_)->second.first;
                _PhysicalBufferType * toBuffer = this->_buffers.find(to_)->second.first;
                size_t skip = 0;

                // How far into the current primitive are we?
                size_t drawn = 0;
                typename BufferMap::const_iterator from = this->_buffers.find(from_);
                typename BufferMap::const_iterator start = this->_buffers.begin();
                do {
                    if (from->second.second.size() == 0) {
                        drawn += from->second.first->size();
//                        std::cout << "  -  drawn += " << drawn << std::endl;
                    } else {
                        typename RestartSet::iterator restart = from->second.second.end();
                        --restart;
                        drawn += from->second.first->size() - *restart;
//                        std::cout << "  +  drawn += " << drawn << std::endl;
                        break;
                    }
                } while (from-- != start);

                ::glBegin(mode_);

//                std::cout << "drawn = " << drawn << std::endl;

                // Redraw vertices from previous buffer
                size_t redraw = 0;
                size_t predraw = 0;
                switch (mode_) {
                case GL_TRIANGLES:
                    redraw = drawn % 6;
                    predraw = 6 - redraw;
                    skip = predraw;
                    break;
                case GL_TRIANGLE_STRIP:
                    redraw = (drawn == 1) ? 1 : (drawn % 2) + 2;
                    predraw = (redraw % 2) + 2;
                    skip = (redraw % 2);
                    break;
                case GL_QUAD_STRIP:
                    break;
                case GL_QUADS:
                    redraw = drawn % 4;
                    predraw = 4 - redraw;
                    skip = predraw;
                    break;
                default:
                    break;
                }

                typename _PhysicalBufferType::iterator prev = fromBuffer->end() - redraw;
                typename _PhysicalBufferType::iterator prev_end = fromBuffer->end();
                for (; prev != prev_end; ++prev) {
                    prev->gl();
                }

                typename _PhysicalBufferType::iterator next = toBuffer->begin();
                typename _PhysicalBufferType::iterator next_end = toBuffer->end();
                for (; predraw > 0; ++next, --predraw) {
                    next->gl();
                }

                ::glEnd();

                return skip;
            }

        // Friend to iterators
        friend class _logical_vertex_buffer_iterator< _PhysicalBufferType >;
        friend class _logical_vertex_buffer_const_iterator< _PhysicalBufferType >;

    }; /* class logical_vertex_buffer */

    // OpenGL standard interleaved arrays
    typedef logical_vertex_buffer< physical_vertex_buffer_V2F > logical_vertex_buffer_V2F;
    typedef logical_vertex_buffer< physical_vertex_buffer_V3F > logical_vertex_buffer_V3F;
    typedef logical_vertex_buffer< physical_vertex_buffer_C4UB_V2F > logical_vertex_buffer_C4UB_V2F;
    typedef logical_vertex_buffer< physical_vertex_buffer_C4UB_V3F > logical_vertex_buffer_C4UB_V3F;
    typedef logical_vertex_buffer< physical_vertex_buffer_C3F_V3F > logical_vertex_buffer_C3F_V3F;
    typedef logical_vertex_buffer< physical_vertex_buffer_N3F_V3F > logical_vertex_buffer_N3F_V3F;
    typedef logical_vertex_buffer< physical_vertex_buffer_C4F_N3F_V3F > logical_vertex_buffer_C4F_N3F_V3F;
    typedef logical_vertex_buffer< physical_vertex_buffer_T2F_V3F > logical_vertex_buffer_T2F_V3F;
    typedef logical_vertex_buffer< physical_vertex_buffer_T4F_V4F > logical_vertex_buffer_T4F_V4F;
    typedef logical_vertex_buffer< physical_vertex_buffer_T2F_C4UB_V3F > logical_vertex_buffer_T2F_C4UB_V3F;
    typedef logical_vertex_buffer< physical_vertex_buffer_T2F_C3F_V3F > logical_vertex_buffer_T2F_C3F_V3F;
    typedef logical_vertex_buffer< physical_vertex_buffer_T2F_N3F_V3F > logical_vertex_buffer_T2F_N3F_V3F;
    typedef logical_vertex_buffer< physical_vertex_buffer_T2F_C4F_N3F_V3F > logical_vertex_buffer_T2F_C4F_N3F_V3F;
    typedef logical_vertex_buffer< physical_vertex_buffer_T4F_C4F_N3F_V4F > logical_vertex_buffer_T4F_C4F_N3F_V4F;
    // Other useful arrays
    typedef logical_vertex_buffer< physical_vertex_buffer_N3F > logical_vertex_buffer_N3F;
    typedef logical_vertex_buffer< physical_vertex_buffer_C3F > logical_vertex_buffer_C3F;
    typedef logical_vertex_buffer< physical_vertex_buffer_C4F > logical_vertex_buffer_C4F;
    typedef logical_vertex_buffer< physical_vertex_buffer_C3UB > logical_vertex_buffer_C3UB;
    typedef logical_vertex_buffer< physical_vertex_buffer_C4UB > logical_vertex_buffer_C4UB;

    /**
     *  \class _logical_vertex_buffer_iterator
     *  \brief An iterator over logical_vertex_buffers.
     */
    template< class _PhysicalBufferType >
    class _logical_vertex_buffer_iterator
    {
        typedef _logical_vertex_buffer_iterator< _PhysicalBufferType > _Self;

    public:
        typedef std::random_access_iterator_tag iterator_category;
        typedef typename _PhysicalBufferType::value_type value_type;
        typedef typename logical_vertex_buffer< _PhysicalBufferType >::difference_type difference_type;
        typedef value_type * pointer;
        typedef value_type & reference;

        /**
         *  \brief Constructor of an iterator.
         *
         *  Takes the logical index of the required element.
         */
        _logical_vertex_buffer_iterator(logical_vertex_buffer< _PhysicalBufferType > * logicalBuffer_ = 0, size_t index_ = 0)
            : _logicalBuffer(logicalBuffer_), _index(index_)
            {
                if (this->_logicalBuffer) {
                    _PhysicalBufferType * physicalBuffer = 0;
                    size_t physicalIndex = 0;
                    logicalBuffer_->_map_index(index_, physicalBuffer, physicalIndex);
                    this->_buffer = logicalBuffer_->_buffers.find(index_ - physicalIndex);
                    this->_element = this->_buffer->second.first->begin() + physicalIndex;
                }
            }

        /**
         *  \brief Copy constructor of an iterator.
         *
         *  Copies the specified iterator.
         */
        _logical_vertex_buffer_iterator(const _Self & rhs_)
            { *this = rhs_; }

        /**  Assignment operator.  */
        _Self & operator = (const _Self & rhs_)
            {
                std::memcpy(this, &rhs_, sizeof(_Self));
                return *this;
            }

        /**  Equality operator.  */
        bool operator == (const _Self & rhs_) const
            { return this->_element == rhs_._element; }

        /**  Inequality operator.  */
        bool operator != (const _Self & rhs_) const
            { return !(*this == rhs_); }

        /**  Pre-increment operator.  */
        _Self & operator ++ ()
            {
                // If not at end of logical buffer...
                if (this->_buffer != this->_logicalBuffer->_buffers.end()) {
                    // If not at end of physical buffer...
                    if (this->_element != this->_buffer->second.first->end()) {
                        ++this->_element;
                    } else {
                        ++this->_buffer;
                        // If not NOW at end of logical buffer...
                        if (this->_buffer != this->_logicalBuffer->_buffers.end()) {
                            this->_element = this->_buffer->second.first->begin();
                        }
                    }
                    ++this->_index;
                }

                return *this;
            }

        /**  Post-increment operator.  */
        _Self & operator ++ (int)
            {
                _Self tmp(*this);
                ++*this;
                return tmp;
            }

        /**  Addition operator.  */
        _Self operator + (const difference_type & rhs_) const
            {
                _Self tmp(this->_logicalBuffer, this->_index + rhs_);
                return tmp;
            }

        /**  Compound addition operator.  */
        _Self & operator += (const difference_type & rhs_)
            {
                this->operator=(this->operator+(rhs_));
                return *this;
            }

        /**  Pre-decrement operator.  */
        _Self & operator -- ()
            {
                // If not at beginning of logical buffer...
                if (this->_buffer != this->_logicalBuffer->_buffers.begin() || this->_element != this->_buffer->second.first->begin()) {
                    // If not at beginning of physical buffer...
                    if (this->_element != this->_buffer->second.first->begin()) {
                        --this->_element;
                    } else {
                        --this->_buffer;
                        // If not NOW at beginning of logical buffer...
                        if (this->_buffer != this->_logicalBuffer->_buffers.begin()) {
                            this->_element = this->_buffer->second.first->end() - 1;
                        }
                    }
                    --this->_index;
                }

                return *this;
            }

        /**  Post-decrement operator.  */
        _Self & operator -- (int)
            {
                _Self tmp(*this);
                --*this;
                return tmp;
            }

        /**  Subtraction operator.  */
        _Self operator - (difference_type rhs_) const
            {
                if (rhs_ > this->_index) {
                    rhs_ = this->_index;
                }
                _Self tmp(this->_logicalBuffer, this->_index - rhs_);
                return tmp;
            }

        /**  Subtraction operator.  */
        difference_type operator - (const _Self & rhs_) const
            {
                return this->_index - rhs_._index;
            }

        /**  Compound subtraction operator.  */
        _Self & operator -= (const difference_type & rhs_)
            {
                this->operator=(this->operator-(rhs_));
                return *this;
            }

        /**  Less-than operator.  */
        bool operator < (const _Self & rhs_) const
            { return (this->_index < rhs_._index); }

        /**  Greater-than operator.  */
        bool operator > (const _Self & rhs_) const
            { return (this->_index > rhs_._index); }

        /**  Less-than-or-equal-to operator.  */
        bool operator <= (const _Self & rhs_) const
            { return (this->_index <= rhs_._index); }

        /**  Greater-than-or-equal-to operator.  */
        bool operator >= (const _Self & rhs_) const
            { return (this->_index >= rhs_._index); }

        /**  Dereferencing operator.  */
        reference operator * () const
            { return *this->_element; }

        /**  Dereferencing operator.  */
        pointer operator -> () const
            { return &(this->operator*()); }

    private:
        // Logical buffer on which this object iterates
        logical_vertex_buffer< _PhysicalBufferType > * _logicalBuffer;
        // Index into that buffer
        size_t _index;
        // Iterator over physical buffers
        typename logical_vertex_buffer< _PhysicalBufferType >::BufferMap::iterator _buffer;
        // Iterator into current physical buffer
        typename _PhysicalBufferType::iterator _element;

        // Friends
        friend class logical_vertex_buffer< _PhysicalBufferType >;

    }; /* class _logical_vertex_buffer_iterator */

    /**
     *  \class _logical_vertex_buffer_const_iterator
     *  \brief A const iterator over logical_vertex_buffers.
     */
    template< class _PhysicalBufferType >
    class _logical_vertex_buffer_const_iterator : public _logical_vertex_buffer_iterator< _PhysicalBufferType >
    {
    public:
        typedef const typename _PhysicalBufferType::value_type * pointer;
        typedef const typename _PhysicalBufferType::value_type & reference;

        /**
         *  \brief Constructor of an iterator.
         *
         *  Takes the logical index of the required element.
         */
        _logical_vertex_buffer_const_iterator(logical_vertex_buffer< _PhysicalBufferType > * logicalBuffer_ = 0, size_t index_ = 0)
            : _logical_vertex_buffer_iterator< _PhysicalBufferType >(logicalBuffer_, index_)
            {}

        /**
         *  \brief Copy constructor of an iterator.
         *
         *  Copies the specified iterator.
         */
        template< typename _IteratorType >
        _logical_vertex_buffer_const_iterator(const _IteratorType & rhs_)
            : _logical_vertex_buffer_iterator< _PhysicalBufferType >(rhs_)
            {}

        // Friends
        friend class logical_vertex_buffer< _PhysicalBufferType >;

    }; /* class _logical_vertex_buffer_const_iterator */

} // namespace gtl

#endif /* GTL_BUFFER_INCL_ */
