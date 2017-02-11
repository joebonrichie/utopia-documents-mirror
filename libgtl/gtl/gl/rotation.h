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

#ifndef GTL_GL_ROTATION_INCL_
#define GTL_GL_ROTATION_INCL_

#include <GL/glew.h>
#include <gtl/config.h>
#include <gtl/matrix.h>
#include <gtl/orientation.h>

#ifndef M_PI
#define M_PI 3.141592653589793238462643383279
#endif

namespace gtl
{

    /**  glRotate calls on orientations  */
    template< typename _ComponentType >
    void glRotate(const orientation< _ComponentType, 3 > &)
    {}
    template<> inline void glRotate< GLdouble >(const orientation< GLdouble, 3 > & orientation_)
    {
        // Retrieve angle / axis information
        GLdouble angle;
        vector< GLdouble, 3 > axis;
        orientation_.to_axis(angle, axis);

        // Call gl command
        ::glRotated(angle * static_cast< GLdouble >(180.0 / M_PI), axis[0], axis[1], axis[2]);
    }
    template<> inline void glRotate< GLfloat >(const orientation< GLfloat, 3 > & orientation_)
    {
        // Retrieve angle / axis information
        GLfloat angle;
        vector< GLfloat, 3 > axis;
        orientation_.to_axis(angle, axis);

        // Call gl command
        ::glRotatef(angle * static_cast< GLfloat >(180.0 / M_PI), axis[0], axis[1], axis[2]);
    }

} /* namespace gtl */

#endif // GTL_GL_ROTATION_INCL_
