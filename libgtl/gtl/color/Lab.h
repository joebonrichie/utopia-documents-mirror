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

#ifndef GTL_COLOR_LAB_INCL_
#define GTL_COLOR_LAB_INCL_

#include <gtl/config.h>
#include <cstring>
#include <cmath>

namespace gtl
{

    // Forward declaration (for use by other colour spaces)
    template< typename _ComponentType >
    struct Lab;

} /* namespace gtl */

// Include all other colour spaces
#include <gtl/color/spaces.h>

namespace gtl
{

    /**
     *  \class  Lab
     *  \brief  The Lab colour space.
     *  \ingroup  gl_color
     *
     *  Holds the X, Y and Z tristimilus values of the CIE 1931 color space.
     */
    template< typename _ComponentType >
    struct Lab
    {
        // Convenience typedef
        typedef Lab< _ComponentType > _Self;

    public:
        // Convenience typedefs
        typedef _ComponentType component_type;

        // Inspection static member
        static const char * name() { return "Lab"; }

        /**  \name  Construction, destruction and assignment  */
        //@{

        /**  Default constructor.  */
        Lab()
            : l(0), a(0), b(0)
            {}

        /**  Explict constructor.  */
        Lab(const component_type & l_,
            const component_type & a_,
            const component_type & b_)
            : l(l_), a(a_), b(b_)
            {}

        /**  Copy constructor.  */
        Lab(const _Self & rhs_)
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
                component_type x_contribution = source_.x / 95.047;
                component_type y_contribution = source_.y / 100.0;
                component_type z_contribution = source_.z / 108.883;

                if (x_contribution > 0.008856) {
                    x_contribution = std::pow(x_contribution, 1.0 / 3.0);
                } else {
                    x_contribution = (7.787 * x_contribution) + (16.0 / 116.0);
                }
                if (y_contribution > 0.008856) {
                    y_contribution = std::pow(y_contribution, 1.0 / 3.0);
                } else {
                    y_contribution = (7.787 * y_contribution) + (16.0 / 116.0);
                }
                if (z_contribution > 0.008856) {
                    z_contribution = std::pow(z_contribution, 1.0 / 3.0);
                } else {
                    z_contribution = (7.787 * z_contribution) + (16.0 / 116.0);
                }

                this->l = (116.0 * y_contribution) - 16.0;
                this->a = 500.0 * (x_contribution - y_contribution);
                this->b = 200.0 * (y_contribution - z_contribution);
                return *this;
            }

        //@}

        /**  Red colour component.  */
        component_type l;
        /**  Green colour component.  */
        component_type a;
        /**  Blue colour component.  */
        component_type b;

    }; /* class Lab */

} /* namespace gtl */

#endif /* GTL_COLOR_LAB_INCL_ */
