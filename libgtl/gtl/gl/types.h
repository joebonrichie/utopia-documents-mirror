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

#ifndef GTL_GL_TYPES_INCL_
#define GTL_GL_TYPES_INCL_

#include <GL/glew.h>
#include <gtl/config.h>
#include <gtl/vector.h>
#include <gtl/matrix.h>

namespace gtl
{

    // Vector convenience typedefs for GLSL compatibility
    typedef vector< GLfloat, 2 > vec2;
    typedef vector< GLfloat, 3 > vec3;
    typedef vector< GLfloat, 4 > vec4;
    typedef vector< GLboolean, 2 > bvec2;
    typedef vector< GLboolean, 3 > bvec3;
    typedef vector< GLboolean, 4 > bvec4;
    typedef vector< GLint, 2 > ivec2;
    typedef vector< GLint, 3 > ivec3;
    typedef vector< GLint, 4 > ivec4;

    // Matrix convenience typedefs for GLSL compatibility
    typedef matrix< GLfloat, 2 > mat2;
    typedef matrix< GLfloat, 3 > mat3;
    typedef matrix< GLfloat, 4 > mat4;

    // Convenience template for converting C++ types to GL types.
    template< typename _ValueType >
    class GLTypes
    {}; /* class GLTypes */
    template<>
    class GLTypes< GLbyte >
    {
    public:
        static const GLenum type = GL_BYTE;
    }; /* class GLTypes< GLbyte > */
    template<>
    class GLTypes< GLubyte >
    {
    public:
        static const GLenum type = GL_UNSIGNED_BYTE;
    }; /* class GLTypes< GLubyte > */
    template<>
    class GLTypes< GLshort >
    {
    public:
        static const GLenum type = GL_SHORT;
    }; /* class GLTypes< GLshort > */
    template<>
    class GLTypes< GLushort >
    {
    public:
        static const GLenum type = GL_UNSIGNED_SHORT;
    }; /* class GLTypes< GLushort > */
    template<>
    class GLTypes< GLint >
    {
    public:
        static const GLenum type = GL_INT;
    }; /* class GLTypes< GLint > */
    template<>
    class GLTypes< GLuint >
    {
    public:
        static const GLenum type = GL_UNSIGNED_INT;
    }; /* class GLTypes< GLuint > */
    template<>
    class GLTypes< GLfloat >
    {
    public:
        static const GLenum type = GL_FLOAT;
    }; /* class GLTypes< GLfloat > */
    template<>
    class GLTypes< GLdouble >
    {
    public:
        static const GLenum type = GL_DOUBLE;
    }; /* class GLTypes< GLdouble > */

} /* namespace gtl */

#endif /* GTL_GL_TYPES_INCL_ */
