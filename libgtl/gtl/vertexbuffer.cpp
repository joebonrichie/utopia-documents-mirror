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

#include <gtl/vertexbuffer.h>

namespace gtl
{

    /**  \name  Construction and destruction.  */
    //@{

    /**  Constructor for a physical buffer.  */
    _physical_vertex_buffer_base::_physical_vertex_buffer_base(const GLenum & defaultMode_)
        : _buffer(0), _taint(0, (size_t) -1), _defaultMode(defaultMode_), _pendingHardReset(false), _pendingHardResetMode(0)
    {}

    /**  Destructor for a physical buffer.  */
    _physical_vertex_buffer_base::~_physical_vertex_buffer_base()
    {}

    //@}
    /**  \name  Buffer specific methods.  */
    //@{

    /**  Assignment operator.  */
    _physical_vertex_buffer_base & _physical_vertex_buffer_base::operator = (const _physical_vertex_buffer_base & rhs_)
    {
        // Make sure a new VBO is used for this buffer copy
        this->_buffer = 0;

        // Copy other members
        this->_defaultMode = rhs_._defaultMode;
        this->_pendingHardReset = rhs_._pendingHardReset;
        this->_pendingHardResetMode = rhs_._pendingHardResetMode;

        // Taint this buffer!
        this->_taint = extent_size_t(0, (size_t) -1);

        return *this;
    }

    /**  Returns the current default GL mode.  */
    GLenum _physical_vertex_buffer_base::defaultMode()
    { return _defaultMode; }

    /**  Sets the current default GL mode to \a defaultMode_.  */
    void _physical_vertex_buffer_base::setDefaultMode(const GLenum & defaultMode_)
    { this->_defaultMode = defaultMode_; }

    /**
     *  \brief  Introduce a hard reset to the buffer.
     *  \param  mode_  GL mode in which this reset will occur.
     *
     *  Introduces degenerate points in order to affect a hard reset of the
     *  polygon stream.
     */
    void _physical_vertex_buffer_base::hard_reset(const GLenum & mode_)
    {
        this->_pendingHardReset = true;
        if (mode_ == (GLenum) -1) {
            this->_pendingHardResetMode = this->_defaultMode;
        } else {
            this->_pendingHardResetMode = mode_;
        }
    }

    /**  Queue a buffer flush (for changed data) for action at next render.  */
    void _physical_vertex_buffer_base::taint(const extent_size_t & taint_)
    { this->_taint += taint_; }

    //@}

} // namespace gtl
