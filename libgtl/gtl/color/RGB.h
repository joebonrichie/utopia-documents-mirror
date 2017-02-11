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

#ifndef GTL_COLOR_RGB_INCL_
#define GTL_COLOR_RGB_INCL_

#include <gtl/config.h>
#include <cstring>
#include <cmath>

namespace gtl
{

    // Forward declaration (for use by other colour spaces)
    template< typename _ComponentType >
    struct RGB;

} /* namespace gtl */

// Include all other colour spaces
#include <gtl/color/spaces.h>
#include <gtl/vector.h>
#include <gtl/matrix.h>

namespace gtl
{

    /**
     *  \class  RGB
     *  \brief  The RGB colour space.
     *  \ingroup  gl_color
     *
     *  Holds red, green and blue colour components.
     */
    template< typename _ComponentType >
    struct RGB
    {
        // Convenience typedef
        typedef RGB< _ComponentType > _Self;

    public:
        // Convenience typedefs
        typedef _ComponentType component_type;

        // Inspection static member
        static const char * name() { return "RGB"; }

        /**  \name  Construction, destruction and assignment  */
        //@{

        /**  Default constructor.  */
        RGB()
            : r(0), g(0), b(0)
            {}

        /**  Explict constructor.  */
        RGB(const component_type & r_,
            const component_type & g_,
            const component_type & b_)
            : r(r_), g(g_), b(b_)
            {}

        /**  Copy constructor.  */
        RGB(const _Self & rhs_)
            { this->operator=(rhs_); }

        /**  Assignment operator.  */
        _Self & operator = (const _Self & rhs_)
            {
                std::memcpy(this, &rhs_, sizeof(_Self));
                return *this;
            }

        //@}
        /**  \name  Conversion methods  */
        //@{

        /**  Conversion from the XYZ colour space.  */
        _Self & from(const XYZ< component_type > & source_)
            {
                matrix< component_type, 3 > transform(3.240479, -0.969256, 0.055648,
                                                      -1.537150, 1.875992, -0.204043,
                                                      -0.498535, 0.041556, 1.057311);
                vector< component_type, 3 > colour(source_.x, source_.y, source_.z);

                colour = transform * colour;

                this->r = colour.r();
                this->g = colour.g();
                this->b = colour.b();
                return *this;
            }

        /**  Conversion from the CMY colour space.  */
        _Self & from(const CMY< component_type > & source_)
            {
                this->r = 1.0 - source_.c;
                this->g = 1.0 - source_.m;
                this->b = 1.0 - source_.y;
                return *this;
            }

        /**  Conversion from the HSV colour space.  */
        _Self & from(const HSV< component_type > & source_)
            {
                if (source_.s == 0) {
                    // Achromatic
                    this->r = source_.v;
                    this->g = source_.v;
                    this->b = source_.v;
                } else {
                    component_type Hi = (source_.h / 60) % 6;
                    component_type f = source_.h / 60.0 - Hi;
                    component_type p = source_.v * (1 - source_.s);
                    component_type q = source_.v * (1 - f * source_.s);
                    component_type t = source_.v * (1 - (1 - f) * source_.s);
                    switch (Hi) {
                    case 0:
                        this->r = source_.v;
                        this->g = t;
                        this->b = p;
                        break;
                    case 1:
                        this->r = q;
                        this->g = source_.v;
                        this->b = p;
                        break;
                    case 2:
                        this->r = p;
                        this->g = source_.v;
                        this->b = t;
                        break;
                    case 3:
                        this->r = p;
                        this->g = q;
                        this->b = source_.v;
                        break;
                    case 4:
                        this->r = t;
                        this->g = p;
                        this->b = source_.v;
                        break;
                    default:
                        this->r = source_.v;
                        this->g = p;
                        this->b = q;
                        break;
                    }
                }
                return *this;
            }

        /**  Conversion from the YUV colour space.  */
        _Self & from(const YUV< component_type > & source_)
            {
                this->r = source_.y + 1.140 * source_.v;
                this->g = source_.y - 0.395 * source_.u - 0.581 * source_.v;
                this->b = source_.y + 2.032 * source_.u;
                return *this;
            }

        /**  Conversion from the YIQ colour space.  */
        _Self & from(const YIQ< component_type > & source_)
            {
                matrix< component_type, 3 > transform(
                    1, 1, 1,
                    0.956296, -0.272122, -1.10699,
                    0.621024, -0.647381, 1.70461);
                vector< component_type, 3 > colour(
                    source_.y,
                    source_.i,
                    source_.q);

                colour = transform * colour;

                this->r = colour.r();
                this->g = colour.g();
                this->b = colour.b();
                return *this;
            }

        //@}

        /**  Red colour component.  */
        component_type r;
        /**  Green colour component.  */
        component_type g;
        /**  Blue colour component.  */
        component_type b;

    }; /* class RGB */

} /* namespace gtl */

#endif /* GTL_COLOR_RGB_INCL_ */
