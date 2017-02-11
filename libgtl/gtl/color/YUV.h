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

#ifndef GTL_COLOR_YUV_INCL_
#define GTL_COLOR_YUV_INCL_

#include <gtl/config.h>
#include <cstring>
#include <cmath>

namespace gtl
{

    // Forward declaration (for use by other colour spaces)
    template< typename _ComponentType >
    struct YUV;

} /* namespace gtl */

// Include all other colour spaces
#include <gtl/color/spaces.h>
#include <gtl/vector.h>
#include <gtl/matrix.h>

namespace gtl
{

    /**
     *  \class  YUV
     *  \brief  The YUV colour space.
     *  \ingroup  gl_color
     *
     *  Holds colour components.
     */
    template< typename _ComponentType >
    struct YUV
    {
        // Convenience typedef
        typedef YUV< _ComponentType > _Self;

    public:
        // Convenience typedefs
        typedef _ComponentType component_type;

        // Inspection static member
        static const char * name() { return "YUV"; }

        /**  \name  Construction, destruction and assignment  */
        //@{

        /**  Default constructor.  */
        YUV()
            : y(0), u(0), v(0)
            {}

        /**  Explict constructor.  */
        YUV(const component_type & y_,
            const component_type & u_,
            const component_type & v_)
            : y(y_), u(u_), v(v_)
            {}

        /**  Copy constructor.  */
        YUV(const _Self & rhs_)
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
                this->y = 0.299 * source_.r + 0.587 * source_.g + 0.114 * source_.b;
                this->u = 0.492 * (source_.b - this->y);
                this->v = 0.877 * (source_.r - this->y);
                return *this;
            }

        //@}

        /**  Luminance colour component.  */
        component_type y;
        /**  U colour component.  */
        component_type u;
        /**  V colour component.  */
        component_type v;

    }; /* class YUV */

} /* namespace gtl */

#endif /* GTL_COLOR_YUV_INCL_ */
