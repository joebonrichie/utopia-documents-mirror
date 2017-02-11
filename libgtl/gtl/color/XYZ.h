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

#ifndef GTL_COLOR_XYZ_INCL_
#define GTL_COLOR_XYZ_INCL_

#include <gtl/config.h>
#include <cstring>

namespace gtl
{

    // Forward declaration (for use by other colour spaces)
    template< typename _ComponentType >
    struct XYZ;

} /* namespace gtl */

// Include all other colour spaces
#include <gtl/color/spaces.h>

namespace gtl
{

    /**
     *  \class  XYZ
     *  \brief  The XYZ colour space.
     *  \ingroup  gl_color
     *
     *  Holds the X, Y and Z tristimilus values of the CIE 1931 color space.
     */
    template< typename _ComponentType >
    struct XYZ
    {
        // Convenience typedef
        typedef XYZ< _ComponentType > _Self;

    public:
        // Convenience typedefs
        typedef _ComponentType component_type;

        // Inspection static member
        static const char * name() { return "XYZ"; }

        /**  \name  Construction, destruction and assignment  */
        //@{

        /**  Default constructor.  */
        XYZ()
            : x(0), y(0), z(0)
            {}

        /**  Explict constructor.  */
        XYZ(const component_type & x_,
            const component_type & y_,
            const component_type & z_)
            : x(x_), y(y_), z(z_)
            {}

        /**  Copy constructor.  */
        XYZ(const _Self & rhs_)
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

        /**  Conversion from the RGB colour space.  */
        _Self & from(const RGB< component_type > & source_)
            {
                matrix< component_type, 3 > transform(
                    0.412453, 0.212671, 0.019334,
                    0.357580, 0.715160, 0.119193,
                    0.180423, 0.072169, 0.950227);
                vector< component_type, 3 > colour(
                    source_.r,
                    source_.g,
                    source_.b);

                colour = transform * colour * 100.0;

                this->x = colour.x();
                this->y = colour.y();
                this->z = colour.z();
                return *this;
            }

        /**  Conversion from the HSV colour space.  */
        _Self & from(const HSV< component_type > & source_)
            { return this->from(static_cast< RGB< component_type > >(source_)); }

        /**  Conversion from the CMY colour space.  */
        _Self & from(const CMY< component_type > & source_)
            { return this->from(static_cast< RGB< component_type > >(source_)); }

        /**  Conversion from the Lab colour space.  */
        _Self & from(const Lab< component_type > & source_)
            {
                component_type y_contribution = (source_.l + 16.0) / 116.0;
                component_type x_contribution = source_.a / 500.0 + y_contribution;
                component_type z_contribution = y_contribution - source_.b / 200.0;

                if (std::pow(y_contribution, 3.0) > 0.008856) {
                    y_contribution = std::pow(y_contribution, 3.0);
                } else {
                    y_contribution = (y_contribution - 16.0 / 116.0) / 7.787;
                }
                if (std::pow(x_contribution, 3.0) > 0.008856) {
                    x_contribution = std::pow(x_contribution, 3.0);
                } else {
                    x_contribution = (x_contribution - 16.0 / 116.0) / 7.787;
                }
                if (std::pow(z_contribution, 3.0) > 0.008856) {
                    z_contribution = std::pow(z_contribution, 3.0);
                } else {
                    z_contribution = (z_contribution - 16.0 / 116.0) / 7.787;
                }

                this->x = 95.057 * x_contribution;
                this->y = 100.0 * y_contribution;
                this->z = 108.883 * z_contribution;
                return *this;
            }

        /**  Conversion from the YIQ colour space.  */
        _Self & from(const YIQ< component_type > & source_)
            {
                matrix< component_type, 3 > transform(
                    0.950456, 1, 1.08875,
                    0.0973953, -0.0711247, -1.06584,
                    0.332204, -0.207887, 1.55461);
                vector< component_type, 3 > colour(
                    source_.y,
                    source_.i,
                    source_.q);

                colour = transform * colour * 100.0;

                this->x = colour.x();
                this->y = colour.y();
                this->z = colour.z();
                return *this;
            }

        /**  Conversion from the YUV colour space.  */
        _Self & from(const YUV< component_type > & source_)
            {
                matrix< component_type, 3 > transform(
                    0.950456, 1, 1.08875,
                    0.225375, -0.135841, 1.88378,
                    0.262442, -0.173063, -0.0472104);
                vector< component_type, 3 > colour(
                    source_.y,
                    source_.u,
                    source_.v);

                colour = transform * colour * 100.0;

                this->x = colour.x();
                this->y = colour.y();
                this->z = colour.z();
                return *this;
            }

        //@}

        /**  Red colour component.  */
        component_type x;
        /**  Green colour component.  */
        component_type y;
        /**  Blue colour component.  */
        component_type z;

    }; /* class XYZ */

} /* namespace gtl */

#endif /* GTL_COLOR_XYZ_INCL_ */
