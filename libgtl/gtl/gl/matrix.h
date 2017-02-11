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

#ifndef GTL_GL_MATRIX_INCL_
#define GTL_GL_MATRIX_INCL_

#include <GL/glew.h>
#include <gtl/config.h>
#include <gtl/matrix.h>
#include <gtl/orientation.h>

namespace gtl
{

    /**  glMultMatrix calls  */
    template< class _MatrixType >
    void glMultMatrix(const _MatrixType &)
    {}
    template<> inline void glMultMatrix< matrix< GLfloat, 4 > >(const matrix< GLfloat, 4 > & matrix_)
    {
        // Call gl command
        ::glMultMatrixf(&matrix_[0][0]);
    }
    template<> inline void glMultMatrix< matrix< GLdouble, 4 > >(const matrix< GLdouble, 4 > & matrix_)
    {
        // Call gl command
        ::glMultMatrixd(&matrix_[0][0]);
    }
    template<> inline void glMultMatrix< orientation< GLdouble, 3 > >(const orientation< GLdouble, 3 > & orientation_)
    {
        // Retrieve matrix information
        matrix< GLdouble, 4 > matrix;
        orientation_.to_matrix(matrix);

        // Call gl command
        glMultMatrix(matrix);
    }
    template<> inline void glMultMatrix< orientation< GLfloat, 3 > >(const orientation< GLfloat, 3 > & orientation_)
    {
        // Retrieve matrix information
        matrix< GLfloat, 4 > matrix;
        orientation_.to_matrix(matrix);

        // Call gl command
        glMultMatrix(matrix);
    }

    /**  glLoadMatrix calls  */
    template< class _MatrixType >
    void glLoadMatrix(const _MatrixType &)
    {}
    template<> inline void glLoadMatrix< matrix< GLfloat, 4 > >(const matrix< GLfloat, 4 > & matrix_)
    {
        // Call gl command
        ::glLoadMatrixf(&matrix_[0][0]);
    }
    template<> inline void glLoadMatrix< matrix< GLdouble, 4 > >(const matrix< GLdouble, 4 > & matrix_)
    {
        // Call gl command
        ::glLoadMatrixd(&matrix_[0][0]);
    }
    template<> inline void glLoadMatrix< orientation< GLdouble, 3 > >(const orientation< GLdouble, 3 > & orientation_)
    {
        // Retrieve matrix information
        matrix< GLdouble, 4 > matrix;
        orientation_.to_matrix(matrix);

        // Call gl command
        glLoadMatrix(matrix);
    }
    template<> inline void glLoadMatrix< orientation< GLfloat, 3 > >(const orientation< GLfloat, 3 > & orientation_)
    {
        // Retrieve matrix information
        matrix< GLfloat, 4 > matrix;
        orientation_.to_matrix(matrix);

        // Call gl command
        glLoadMatrix(matrix);
    }

} /* namespace gtl */

#endif // GTL_GL_MATRIX_INCL_
