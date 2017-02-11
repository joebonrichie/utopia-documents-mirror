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

#ifndef GTL_COLOR_YIQ_INCL_
#define GTL_COLOR_YIQ_INCL_

#include <gtl/config.h>
#include <cstring>
#include <cmath>

namespace gtl
{

    // Forward declaration (for use by other colour spaces)
    template< typename _ComponentType >
    struct YIQ;

} /* namespace gtl */

// Include all other colour spaces
#include <gtl/color/spaces.h>
#include <gtl/vector.h>
#include <gtl/matrix.h>

namespace gtl
{

    /**
     *  \class  YIQ
     *  \brief  The YIQ colour space.
     *  \ingroup  gl_color
     *
     *  Holds colour components.
     */
    template< typename _ComponentType >
    struct YIQ
    {
        // Convenience typedef
        typedef YIQ< _ComponentType > _Self;

    public:
        // Convenience typedefs
        typedef _ComponentType component_type;

        // Inspection static member
        static const char * name() { return "YIQ"; }

        /**  \name  Construction, destruction and assignment  */
        //@{

        /**  Default constructor.  */
        YIQ()
            : y(0), i(0), q(0)
            {}

        /**  Explict constructor.  */
        YIQ(const component_type & y_,
            const component_type & i_,
            const component_type & q_)
            : y(y_), i(i_), q(q_)
            {}

        /**  Copy constructor.  */
        YIQ(const _Self & rhs_)
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
            { return this->from(static_cast< RGB< component_type > >(source_)); }

        /**  Conversion from the RGB colour space.  */
        _Self & from(const RGB< component_type > & source_)
            {
                matrix< component_type, 3 > transform(
                    0.299, 0.595716, 0.211456,
                    0.587, -0.274453, -0.522591,
                    0.114, -0.321263, 0.311135);
                vector< component_type, 3 > colour(
                    source_.r,
                    source_.g,
                    source_.b);

                colour = transform * colour;

                this->y = colour.r();
                this->i = colour.g();
                this->q = colour.b();
                return *this;
            }

        //@}

        /**  Luminance colour component.  */
        component_type y;
        /**  In-phase colour component.  */
        component_type i;
        /**  Quadrature colour component.  */
        component_type q;

    }; /* class YIQ */

} /* namespace gtl */

#endif /* GTL_COLOR_YIQ_INCL_ */
