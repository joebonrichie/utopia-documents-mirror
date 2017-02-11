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

#ifndef GTL_GL_VERTEX_INCL_
#define GTL_GL_VERTEX_INCL_

#include <GL/glew.h>
#include <gtl/config.h>
#include <gtl/vector.h>
#include <cmath>
#include <algorithm>

namespace gtl
{

    /**
     *  \class  vertex
     *  \brief  Vectorial GL data that represents vertex positions.
     */
    template< typename _ComponentType, size_t _size >
    class vertex : public vector< _ComponentType, _size >
    {
    public:
        typedef _ComponentType component_type;
        typedef vector< component_type, _size > vector_type;

        /**  Convenience const of GL type enum.  */
        static const GLenum GLtype = GLTypes< component_type >::type;

        /**  \name  Construction and destruction.  */
        //@{

        /**  Constructor for this vectorial GL data type.  */
        vertex()
            : vector_type()
            {}

        /**  Constructor for this vectorial GL data type.  */
        vertex(const vector_type & rhs_)
            : vector_type(rhs_)
            {}

        /**  Constructor for this vectorial GL data type.  */
        vertex(const component_type & x_, const component_type & y_)
            : vector_type(x_, y_)
            {}

        /**  Constructor for this vectorial GL data type.  */
        vertex(const component_type & x_, const component_type & y_, const component_type & z_)
            : vector_type(x_, y_, z_)
            {}

        /**  Constructor for this vectorial GL data type.  */
        vertex(const component_type & x_, const component_type & y_, const component_type & z_, const component_type & w_)
            : vector_type(x_, y_, z_, w_)
            {}

        //@}
        /**  \name  Static Public GL methods.  */
        //@{

        /**  Enable client state for this vectorial GL data type.  */
        static void enable_client_state()
            { ::glEnableClientState(GL_VERTEX_ARRAY); }

        /**  Set up buffer pointer for this vectorial GL data type.  */
        static void pointer(const GLsizei & stride_, const GLvoid * offset_)
            { ::glVertexPointer(_size, GLtype, stride_, offset_); }

        /**  Disable client state for this vectorial GL data type.  */
        static void disable_client_state()
            { ::glDisableClientState(GL_VERTEX_ARRAY); }

        /**  Call gl command for this vertex.  */
        void gl();

        //@}

    }; /* class vertex */
    typedef class vertex< GLshort, 2 > vertex2s;
    typedef class vertex< GLshort, 3 > vertex3s;
    typedef class vertex< GLshort, 4 > vertex4s;
    typedef class vertex< GLint, 2 > vertex2i;
    typedef class vertex< GLint, 3 > vertex3i;
    typedef class vertex< GLint, 4 > vertex4i;
    typedef class vertex< GLfloat, 2 > vertex2f;
    typedef class vertex< GLfloat, 3 > vertex3f;
    typedef class vertex< GLfloat, 4 > vertex4f;
    typedef class vertex< GLdouble, 2 > vertex2d;
    typedef class vertex< GLdouble, 3 > vertex3d;
    typedef class vertex< GLdouble, 4 > vertex4d;

    /**  glVertex calls on vectors.  */
    template< typename _ComponentType, size_t _size >
    void glVertex(const vector< _ComponentType, _size > &)
    {}
    template<> inline void glVertex< GLshort, 2 >(const vector< GLshort, 2 > & vec_)
    { ::glVertex2sv((const GLshort *) &vec_); }
    template<> inline void glVertex< GLshort, 3 >(const vector< GLshort, 3 > & vec_)
    { ::glVertex3sv((const GLshort *) &vec_); }
    template<> inline void glVertex< GLshort, 4 >(const vector< GLshort, 4 > & vec_)
    { ::glVertex4sv((const GLshort *) &vec_); }
    template<> inline void glVertex< GLint, 2 >(const vector< GLint, 2 > & vec_)
    { ::glVertex2iv((const GLint *) &vec_); }
    template<> inline void glVertex< GLint, 3 >(const vector< GLint, 3 > & vec_)
    { ::glVertex3iv((const GLint *) &vec_); }
    template<> inline void glVertex< GLint, 4 >(const vector< GLint, 4 > & vec_)
    { ::glVertex4iv((const GLint *) &vec_); }
    template<> inline void glVertex< GLfloat, 2 >(const vector< GLfloat, 2 > & vec_)
    { ::glVertex2fv((const GLfloat *) &vec_); }
    template<> inline void glVertex< GLfloat, 3 >(const vector< GLfloat, 3 > & vec_)
    { ::glVertex3fv((const GLfloat *) &vec_); }
    template<> inline void glVertex< GLfloat, 4 >(const vector< GLfloat, 4 > & vec_)
    { ::glVertex4fv((const GLfloat *) &vec_); }
    template<> inline void glVertex< GLdouble, 2 >(const vector< GLdouble, 2 > & vec_)
    { ::glVertex2dv((const GLdouble *) &vec_); }
    template<> inline void glVertex< GLdouble, 3 >(const vector< GLdouble, 3 > & vec_)
    { ::glVertex3dv((const GLdouble *) &vec_); }
    template<> inline void glVertex< GLdouble, 4 >(const vector< GLdouble, 4 > & vec_)
    { ::glVertex4dv((const GLdouble *) &vec_); }
    template< typename _ComponentType, size_t _size >
    void vertex< _ComponentType, _size >::gl()
    { glVertex(*this); }




    /**
     *  \struct  normal
     *  \brief  Vectorial GL data that represents vertex normals.
     */
    template< typename _ComponentType, size_t _size >
    class normal : public vector< _ComponentType, _size >
    {
    public:
        typedef _ComponentType component_type;
        typedef vector< component_type, _size > vector_type;

        /**  Convenience const of GL type enum.  */
        static const GLenum GLtype = GLTypes< component_type >::type;

        /**  \name  Construction and destruction.  */
        //@{

        /**  Constructor for this vectorial GL data type.  */
        normal()
            : vector_type()
            {}

        /**  Constructor for this vectorial GL data type.  */
        normal(const vector_type & rhs_)
            : vector_type(rhs_)
            {}

        /**  Constructor for this vectorial GL data type.  */
        normal(const component_type & x_, const component_type & y_, const component_type & z_)
            : vector_type(x_, y_, z_)
            {}

        //@}
        /**  \name  Static Public GL methods.  */
        //@{

        /**  Enable client state for this vectorial GL data type.  */
        static void enable_client_state()
            { ::glEnableClientState(GL_NORMAL_ARRAY); }

        /**  Set up buffer pointer for this vectorial GL data type.  */
        static void pointer(const GLsizei & stride_, const GLvoid * offset_)
            { ::glNormalPointer(GLtype, stride_, offset_); }

        /**  Disable client state for this vectorial GL data type.  */
        static void disable_client_state()
            { ::glDisableClientState(GL_NORMAL_ARRAY); }

        /**  Call gl command for this vertex.  */
        void gl();

        //@}

    }; /* class vertex */
    typedef class normal< GLbyte, 3 > normal3b;
    typedef class normal< GLshort, 3 > normal3s;
    typedef class normal< GLint, 3 > normal3i;
    typedef class normal< GLfloat, 3 > normal3f;
    typedef class normal< GLdouble, 3 > normal3d;

    /**  glNormal calls on vectors.  */
    template< typename _ComponentType, size_t _size >
    inline void glNormal(const vector< _ComponentType, _size > &)
    {}
    template<> inline void glNormal< GLbyte, 3 >(const vector< GLbyte, 3 > & vec_)
    { ::glNormal3bv((const GLbyte *) &vec_); }
    template<> inline void glNormal< GLshort, 3 >(const vector< GLshort, 3 > & vec_)
    { ::glNormal3sv((const GLshort *) &vec_); }
    template<> inline void glNormal< GLint, 3 >(const vector< GLint, 3 > & vec_)
    { ::glNormal3iv((const GLint *) &vec_); }
    template<> inline void glNormal< GLfloat, 3 >(const vector< GLfloat, 3 > & vec_)
    { ::glNormal3fv((const GLfloat *) &vec_); }
    template<> inline void glNormal< GLdouble, 3 >(const vector< GLdouble, 3 > & vec_)
    { ::glNormal3dv((const GLdouble *) &vec_); }
    template< typename _ComponentType, size_t _size >
    void normal< _ComponentType, _size >::gl()
    { glNormal(*this); }




    /**
     *  \struct  color
     *  \brief  Vectorial GL data that represents vertex colours.
     */
    template< typename _ComponentType, size_t _size >
    class color : public vector< _ComponentType, _size >
    {
    public:
        typedef _ComponentType component_type;
        typedef vector< component_type, _size > vector_type;

        /**  Convenience const of GL type enum.  */
        static const GLenum GLtype = GLTypes< component_type >::type;

        /**  \name  Construction and destruction.  */
        //@{

        /**  Constructor for this vectorial GL data type.  */
        color()
            : vector_type()
            {}

        /**  Constructor for this vectorial GL data type.  */
        color(const vector_type & rhs_)
            : vector_type(rhs_)
            {}

        /**  Constructor for this vectorial GL data type.  */
        color(const component_type & x_, const component_type & y_,
              const component_type & z_)
            : vector_type(x_, y_, z_)
            {}

        /**  Constructor for this vectorial GL data type.  */
        color(const component_type & x_, const component_type & y_,
              const component_type & z_, const component_type & w_)
            : vector_type(x_, y_, z_, w_)
            {}

        /**  Modify the colour by the given gamma.  */
        void gamma (GLfloat gamma_)
            {
                this->r(std::pow (this->r(), gamma_));
                this->g(std::pow (this->g(), gamma_));
                this->b(std::pow (this->b(), gamma_));
            }


        // Should use matrices:
        // http://cs.haifa.ac.il/hagit/courses/ist/Lectures/Demos/ColorApplet2/t_convert.html

        /**  Return XYZ representation of colour.  */
        void toXYZ (double &X_, double &Y_, double &Z_) const
            {
                // see http://www.easyrgb.com/math.php

                double var_R = this->r();
                double var_G = this->g();
                double var_B = this->b();

                if ( var_R > 0.04045 ) {
                    var_R = std::pow (( ( var_R + 0.055 ) / 1.055 ), 2.4);
                } else {
                    var_R = var_R / 12.92;
                }

                if ( var_G > 0.04045 ) {
                    var_G = std::pow (( ( var_G + 0.055 ) / 1.055 ), 2.4);
                } else {
                    var_G = var_G / 12.92;
                }

                if ( var_B > 0.04045 ) {
                    var_B = std::pow (( ( var_B + 0.055 ) / 1.055 ), 2.4);
                } else {
                    var_B = var_B / 12.92;
                }

                var_R = var_R * 100.0;
                var_G = var_G * 100.0;
                var_B = var_B * 100.0;

                //Observer. = 2°, Illuminant = D65
                X_ = var_R * 0.4124 + var_G * 0.3576 + var_B * 0.1805;
                Y_ = var_R * 0.2126 + var_G * 0.7152 + var_B * 0.0722;
                Z_ = var_R * 0.0193 + var_G * 0.1192 + var_B * 0.9505;
            }

        /**  Set colour from its XYZ representation.  */
        void fromXYZ (double X_, double Y_, double Z_)
            {
                // see http://www.easyrgb.com/math.php

                double var_X = X_ / 100.0;
                double var_Y = Y_ / 100.0;
                double var_Z = Z_ / 100.0;

                double var_R = var_X *  3.2406 + var_Y * -1.5372 + var_Z * -0.4986;
                double var_G = var_X * -0.9689 + var_Y *  1.8758 + var_Z *  0.0415;
                double var_B = var_X *  0.0557 + var_Y * -0.2040 + var_Z *  1.0570;

                if ( var_R > 0.0031308 ) {
                    var_R = 1.055 * ( std::pow (var_R, 1.0/2.4)) - 0.055;
                } else {
                    var_R = 12.92 * var_R;
                }

                if ( var_G > 0.0031308 ) {
                    var_G = 1.055 * ( std::pow (var_G, 1.0/2.4)) - 0.055;
                } else {
                    var_G = 12.92 * var_G;
                }

                if ( var_B > 0.0031308 ) {
                    var_B = 1.055 * ( std::pow (var_B  , 1.0/2.4)) - 0.055;
                } else {
                    var_B = 12.92 * var_B;
                }

                this->r (var_R);
                this->g (var_G);
                this->b (var_B);
            }

        /**  Return L*a*b representation of colour.  */
        void toLab (double &L_, double &a_, double &b_)
            {
                // see http://www.easyrgb.com/math.php

                double var_X, var_Y, var_Z;

                this->toXYZ (var_X, var_Y, var_Z);

                var_X /= 95.047;
                var_Y /= 100.000;
                var_Z /= 108.883;


                if ( var_X > 0.008856 ) {
                    var_X = std::pow (var_X, 1/3.0);
                }
                else {
                    var_X = ( 7.787 * var_X ) + ( 16 / 116.0 );
                }

                if ( var_Y > 0.008856 ) {
                    var_Y = std::pow (var_Y, 1/3.0);
                } else {
                    var_Y = ( 7.787 * var_Y ) + ( 16 / 116.0 );
                }

                if ( var_Z > 0.008856 ) {
                    var_Z = std::pow (var_Z, 1/3.0);
                } else {
                    var_Z = ( 7.787 * var_Z ) + ( 16 / 116.0 );
                }

                L_ = ( 116.0 * var_Y ) - 16;
                a_ = 500.0 * ( var_X - var_Y );
                b_ = 200.0 * ( var_Y - var_Z );
            }

        /**  Set colour from its L*a*b representation.  */
        void fromLab (double L_, double a_, double b_)
            {
                // see http://www.easyrgb.com/math.php

                double var_Y = ( L_ + 16 ) / 116.0;
                double var_X = a_ / 500.0 + var_Y;
                double var_Z = var_Y - b_ / 200.0;

                if ( std::pow (var_Y, 3) > 0.008856 ) {
                    var_Y = std::pow (var_Y, 3);
                } else {
                    var_Y = ( var_Y - 16 / 116.0 ) / 7.787;
                }

                if ( std::pow (var_X,3) > 0.008856 ) {
                    var_X = std::pow (var_X,3);
                } else {
                    var_X = ( var_X - 16 / 116.0 ) / 7.787;
                }

                if ( std::pow (var_Z,3) > 0.008856 ) {
                    var_Z = std::pow (var_Z,3);
                } else {
                    var_Z = ( var_Z - 16 / 116.0 ) / 7.787;
                }

                double X = 95.047 * var_X;
                double Y = 100.0 * var_Y;
                double Z = 108.883 * var_Z;

                this->fromXYZ (X, Y, Z);
            }

        /**  Return the luminance of the colour.  */
        GLfloat luminance ()
            {
                return this->r() * 0.299 +  this->g() * 0.587 + this->b() * 0.114;
            }

        //@}
        /**  \name  Static Public GL methods.  */
        //@{

        /**  Enable client state for this vectorial GL data type.  */
        static void enable_client_state()
            { ::glEnableClientState(GL_COLOR_ARRAY); }

        /**  Set up buffer pointer for this vectorial GL data type.  */
        static void pointer(const GLsizei & stride_, const GLvoid * offset_)
            { ::glColorPointer(_size, GLtype, stride_, offset_); }

        /**  Disable client state for this vectorial GL data type.  */
        static void disable_client_state()
            { ::glDisableClientState(GL_COLOR_ARRAY); }

        /**  Call gl command for this colour.  */
        void gl();

        //@}

    }; /* class color */
    typedef class color< GLbyte, 3 > color3b;
    typedef class color< GLbyte, 4 > color4b;
    typedef class color< GLubyte, 3 > color3ub;
    typedef class color< GLubyte, 4 > color4ub;
    typedef class color< GLshort, 3 > color3s;
    typedef class color< GLshort, 4 > color4s;
    typedef class color< GLushort, 3 > color3us;
    typedef class color< GLushort, 4 > color4us;
    typedef class color< GLint, 3 > color3i;
    typedef class color< GLint, 4 > color4i;
    typedef class color< GLuint, 3 > color3ui;
    typedef class color< GLuint, 4 > color4ui;
    typedef class color< GLfloat, 3 > color3f;
    typedef class color< GLfloat, 4 > color4f;
    typedef class color< GLdouble, 3 > color3d;
    typedef class color< GLdouble, 4 > color4d;

    /**  glColor calls on vectors.  */
    template< typename _ComponentType, size_t _size >
    void glColor(const vector< _ComponentType, _size > &)
    {}
    template<> inline void glColor< GLbyte, 3 >(const vector< GLbyte, 3 > & vec_)
    { ::glColor3bv((const GLbyte *) &vec_); }
    template<> inline void glColor< GLbyte, 4 >(const vector< GLbyte, 4 > & vec_)
    { ::glColor4bv((const GLbyte *) &vec_); }
    template<> inline void glColor< GLubyte, 3 >(const vector< GLubyte, 3 > & vec_)
    { ::glColor3ubv((const GLubyte *) &vec_); }
    template<> inline void glColor< GLubyte, 4 >(const vector< GLubyte, 4 > & vec_)
    { ::glColor4ubv((const GLubyte *) &vec_); }
    template<> inline void glColor< GLshort, 3 >(const vector< GLshort, 3 > & vec_)
    { ::glColor3sv((const GLshort *) &vec_); }
    template<> inline void glColor< GLshort, 4 >(const vector< GLshort, 4 > & vec_)
    { ::glColor4sv((const GLshort *) &vec_); }
    template<> inline void glColor< GLushort, 3 >(const vector< GLushort, 3 > & vec_)
    { ::glColor3usv((const GLushort *) &vec_); }
    template<> inline void glColor< GLushort, 4 >(const vector< GLushort, 4 > & vec_)
    { ::glColor4usv((const GLushort *) &vec_); }
    template<> inline void glColor< GLint, 3 >(const vector< GLint, 3 > & vec_)
    { ::glColor3iv((const GLint *) &vec_); }
    template<> inline void glColor< GLint, 4 >(const vector< GLint, 4 > & vec_)
    { ::glColor4iv((const GLint *) &vec_); }
    template<> inline void glColor< GLuint, 3 >(const vector< GLuint, 3 > & vec_)
    { ::glColor3uiv((const GLuint *) &vec_); }
    template<> inline void glColor< GLuint, 4 >(const vector< GLuint, 4 > & vec_)
    { ::glColor4uiv((const GLuint *) &vec_); }
    template<> inline void glColor< GLfloat, 3 >(const vector< GLfloat, 3 > & vec_)
    { ::glColor3fv((const GLfloat *) &vec_); }
    template<> inline void glColor< GLfloat, 4 >(const vector< GLfloat, 4 > & vec_)
    { ::glColor4fv((const GLfloat *) &vec_); }
    template<> inline void glColor< GLdouble, 3 >(const vector< GLdouble, 3 > & vec_)
    { ::glColor3dv((const GLdouble *) &vec_); }
    template<> inline void glColor< GLdouble, 4 >(const vector< GLdouble, 4 > & vec_)
    { ::glColor4dv((const GLdouble *) &vec_); }
    template< typename _ComponentType, size_t _size >
    void color< _ComponentType, _size >::gl()
    { glColor(*this); }




    /**
     *  \struct  texcoord
     *  \brief  Vectorial GL data that represents vertex texture coordinates.
     */
    template< typename _ComponentType, size_t _size >
    class texcoord : public vector< _ComponentType, _size >
    {
    public:
        typedef _ComponentType component_type;
        typedef vector< component_type, _size > vector_type;

        /**  Convenience const of GL type enum.  */
        static const GLenum GLtype = GLTypes< component_type >::type;

        /**  \name  Construction and destruction.  */
        //@{

        /**  Constructor for this vectorial GL data type.  */
        texcoord()
            : vector_type()
            {}

        /**  Constructor for this vectorial GL data type.  */
        texcoord(const vector_type & rhs_)
            : vector_type(rhs_)
            {}

        /**  Constructor for this vectorial GL data type.  */
        texcoord(const component_type & x_)
            : vector_type(x_)
            {}

        /**  Constructor for this vectorial GL data type.  */
        texcoord(const component_type & x_, const component_type & y_)
            : vector_type(x_, y_)
            {}

        /**  Constructor for this vectorial GL data type.  */
        texcoord(const component_type & x_, const component_type & y_, const component_type & z_)
            : vector_type(x_, y_, z_)
            {}

        /**  Constructor for this vectorial GL data type.  */
        texcoord(const component_type & x_, const component_type & y_, const component_type & z_, const component_type & w_)
            : vector_type(x_, y_, z_, w_)
            {}

        //@}
        /**  \name  Static Public GL methods.  */
        //@{

        /**  Enable client state for this vectorial GL data type.  */
        static void enable_client_state()
            { ::glEnableClientState(GL_TEXTURE_COORD_ARRAY); }

        /**  Set up buffer pointer for this vectorial GL data type.  */
        static void pointer(const GLsizei & stride_, const GLvoid * offset_)
            { ::glTexCoordPointer(_size, GLtype, stride_, offset_); }

        /**  Disable client state for this vectorial GL data type.  */
        static void disable_client_state()
            { ::glDisableClientState(GL_TEXTURE_COORD_ARRAY); }

        /**  Call gl command for this texture coordinate.  */
        void gl();

        //@}

    }; /* class texcoord */
    template< typename _ComponentType >
    class texcoord< _ComponentType, 1 >
    {
    public:
        typedef _ComponentType component_type;

        static const GLenum GLtype = GLTypes< component_type >::type;

        /**  \name  Construction and destruction.  */
        //@{

        /**  Constructor for this vectorial GL coord type.  */
        texcoord(const component_type & coord_ = 0)
            : _coord(coord_)
            {}

        //@}
        /**  \name  Element accessor methods.  */
        //@{

        /**  Set value  */
        void set(const component_type & coord_)
            { this->_coord = coord_; }

        /**  Get value  */
        const component_type & get()
            { return this->_coord; }

        //@}
        /**  \name  Static Public GL methods.  */
        //@{

        /**  Enable client state for this vectorial GL coord type.  */
        static void enable_client_state()
            { ::glEnableClientState(GL_TEXTURE_COORD_ARRAY); }

        /**  Set up buffer pointer for this vectorial GL coord type.  */
        static void pointer(const GLsizei & stride_, const GLvoid * offset_)
            { ::glTexCoordPointer(1, GLtype, stride_, offset_); }

        /**  Disable client state for this vectorial GL coord type.  */
        static void disable_client_state()
            { ::glDisableClientState(GL_TEXTURE_COORD_ARRAY); }

        /**  Call gl command for this texture coordinate.  */
        void gl();

    private:
        // Texture Coordinate
        component_type _coord;

    }; /* class texcoord */
    typedef class texcoord< GLshort, 1 > texcoord1s;
    typedef class texcoord< GLshort, 2 > texcoord2s;
    typedef class texcoord< GLshort, 3 > texcoord3s;
    typedef class texcoord< GLshort, 4 > texcoord4s;
    typedef class texcoord< GLint, 1 > texcoord1i;
    typedef class texcoord< GLint, 2 > texcoord2i;
    typedef class texcoord< GLint, 3 > texcoord3i;
    typedef class texcoord< GLint, 4 > texcoord4i;
    typedef class texcoord< GLfloat, 1 > texcoord1f;
    typedef class texcoord< GLfloat, 2 > texcoord2f;
    typedef class texcoord< GLfloat, 3 > texcoord3f;
    typedef class texcoord< GLfloat, 4 > texcoord4f;
    typedef class texcoord< GLdouble, 1 > texcoord1d;
    typedef class texcoord< GLdouble, 2 > texcoord2d;
    typedef class texcoord< GLdouble, 3 > texcoord3d;
    typedef class texcoord< GLdouble, 4 > texcoord4d;

    /**  glVertex calls on vectors.  */
    template< typename _ComponentType, size_t _size >
    inline void glTexCoord(const vector< _ComponentType, _size > &)
    {}
    template<> inline
    void glTexCoord< GLshort, 2 >(const vector< GLshort, 2 > & vec_)
    { ::glTexCoord2sv((const GLshort *) &vec_); }
    template<> inline
    void glTexCoord< GLshort, 3 >(const vector< GLshort, 3 > & vec_)
    { ::glTexCoord3sv((const GLshort *) &vec_); }
    template<> inline
    void glTexCoord< GLshort, 4 >(const vector< GLshort, 4 > & vec_)
    { ::glTexCoord4sv((const GLshort *) &vec_); }
    template<> inline
    void glTexCoord< GLint, 2 >(const vector< GLint, 2 > & vec_)
    { ::glTexCoord2iv((const GLint *) &vec_); }
    template<> inline
    void glTexCoord< GLint, 3 >(const vector< GLint, 3 > & vec_)
    { ::glTexCoord3iv((const GLint *) &vec_); }
    template<> inline
    void glTexCoord< GLint, 4 >(const vector< GLint, 4 > & vec_)
    { ::glTexCoord4iv((const GLint *) &vec_); }
    template<> inline
    void glTexCoord< GLfloat, 2 >(const vector< GLfloat, 2 > & vec_)
    { ::glTexCoord2fv((const GLfloat *) &vec_); }
    template<> inline
    void glTexCoord< GLfloat, 3 >(const vector< GLfloat, 3 > & vec_)
    { ::glTexCoord3fv((const GLfloat *) &vec_); }
    template<> inline
    void glTexCoord< GLfloat, 4 >(const vector< GLfloat, 4 > & vec_)
    { ::glTexCoord4fv((const GLfloat *) &vec_); }
    template<> inline
    void glTexCoord< GLdouble, 2 >(const vector< GLdouble, 2 > & vec_)
    { ::glTexCoord2dv((const GLdouble *) &vec_); }
    template<> inline
    void glTexCoord< GLdouble, 3 >(const vector< GLdouble, 3 > & vec_)
    { ::glTexCoord3dv((const GLdouble *) &vec_); }
    template<> inline
    void glTexCoord< GLdouble, 4 >(const vector< GLdouble, 4 > & vec_)
    { ::glTexCoord4dv((const GLdouble *) &vec_); }
    inline void glTexCoord(const GLshort & coord_) { ::glTexCoord1s(coord_); }
    inline void glTexCoord(const GLint & coord_) { ::glTexCoord1i(coord_); }
    inline void glTexCoord(const GLfloat & coord_) { ::glTexCoord1f(coord_); }
    inline void glTexCoord(const GLdouble & coord_) { ::glTexCoord1d(coord_); }
    template< typename _ComponentType, size_t _size >
    inline void texcoord< _ComponentType, _size >::gl() { glTexCoord(*this); }
    template< typename _ComponentType >
    inline void texcoord< _ComponentType, 1 >::gl()
    { glTexCoord(_coord); }




    /**
     *  \struct  index
     *  \brief  Vectorial GL data that represents vertex indices.
     */
    template< typename _ComponentType >
    class index
    {
    public:
        typedef _ComponentType component_type;

        /**  Convenience const of GL type enum.  */
        static const GLenum GLtype = GLTypes< component_type >::type;

        /**  \name  Construction and destruction.  */
        //@{

        /**  Constructor for this index GL data type.  */
        index(const component_type & index_ = 0)
            : _index(index_)
            {}

        //@}
        /**  \name  Element accessor methods.  */
        //@{

        /**  Set value  */
        void set(const component_type & index_)
            { this->_index = index_; }

        /**  Get value  */
        const component_type & get()
            { return this->_index; }

        //@}
        /**  \name  Static Public GL methods.  */
        //@{

        /**  Enable client state for this index GL data type.  */
        static void enable_client_state()
            { ::glEnableClientState(GL_INDEX_ARRAY); }

        /**  Set up buffer pointer for this index GL data type.  */
        static void pointer(const GLsizei & stride_, const GLvoid * offset_)
            { ::glIndexPointer(GLtype, stride_, offset_); }

        /**  Disable client state for this index GL data type.  */
        static void disable_client_state()
            { ::glDisableClientState(GL_INDEX_ARRAY); }

        /**  Call gl command for this index.  */
        void gl();

        //@}

    private:
        // index data
        component_type _index;

    }; /* class index */
    typedef class index< GLubyte > indexub;
    typedef class index< GLshort > indexs;
    typedef class index< GLint > indexi;
    typedef class index< GLfloat > indexf;
    typedef class index< GLdouble > indexd;

    /**  glIndex calls on vectors.  */
    template< typename _ComponentType >
    inline void glIndex(const _ComponentType &)
    {}
    template<> inline void glIndex< GLubyte >(const GLubyte & index_)
    { ::glIndexub(index_); }
    template<> inline void glIndex< GLshort >(const GLshort & index_)
    { ::glIndexs(index_); }
    template<> inline void glIndex< GLint >(const GLint & index_)
    { ::glIndexi(index_); }
    template<> inline void glIndex< GLfloat >(const GLfloat & index_)
    { ::glIndexf(index_); }
    template<> inline void glIndex< GLdouble >(const GLdouble & index_)
    { ::glIndexd(index_); }
    template< typename _ComponentType >
    void index< _ComponentType >::gl()
    { glIndex(*this); }




    /**
     *  \struct  edgeflag
     *  \brief  Vectorial GL data that represents vertex edge flags.
     */
    class edgeflag
    {
    public:
        typedef GLboolean component_type;

        /**  \name  Construction and destruction.  */
        //@{

        /**  Constructor for this edge flag GL data type.  */
        edgeflag(const component_type & flag_ = GL_FALSE)
            : _flag(flag_)
            {}

        //@}
        /**  \name  Element accessor methods.  */
        //@{

        /**  Set value  */
        void set(const component_type & flag_)
            { this->_flag = flag_; }

        /**  Get value  */
        const component_type & get()
            { return this->_flag; }

        //@}
        /**  \name  Static Public GL methods.  */
        //@{

        /**  Enable client state for this edge flag GL data type.  */
        static void enable_client_state()
            { ::glEnableClientState(GL_EDGE_FLAG_ARRAY); }

        /**  Set up buffer pointer for this edge flag GL data type.  */
        static void pointer(const GLsizei & stride_, const GLvoid * offset_)
            { ::glEdgeFlagPointer(stride_, offset_); }

        /**  Disable client state for this edge flag GL data type.  */
        static void disable_client_state()
            { ::glDisableClientState(GL_EDGE_FLAG_ARRAY); }

        /**  Call gl command for this edge flag.  */
        void gl();

        //@}

    private:
        // Edge flag
        GLboolean _flag;

    }; /* class edgeflag */

    /**  glEdgeFlag calls on vectors.  */
    template< typename _ComponentType >
    inline void glEdgeFlag(const _ComponentType &)
    {}
    template<> inline void glEdgeFlag< GLboolean >(const GLboolean & flag_)
    { ::glEdgeFlag(flag_); }
    inline void edgeflag::gl()
    { glEdgeFlag(_flag); }

} /* namespace gtl */

#endif /* GTL_GL_VERTEX_INCL_ */
