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

#ifndef GTL_COLOR_HSV_INCL_
#define GTL_COLOR_HSV_INCL_

#include <gtl/config.h>
#include <cstring>
#include <cmath>

namespace gtl
{

    // Forward declaration (for use by other colour spaces)
    template< typename _ComponentType >
    struct HSV;

} /* namespace gtl */

// Include all other colour spaces
#include <gtl/color/spaces.h>

namespace gtl
{

    /**
     *  \class  HSV
     *  \brief  The HSV colour space.
     *  \ingroup  gl_color
     *
     *  Holds red, green and blue colour components.
     */
    template< typename _ComponentType >
    struct HSV
    {
        // Convenience typedef
        typedef HSV< _ComponentType > _Self;

    public:
        // Convenience typedefs
        typedef _ComponentType component_type;

        // Inspection static member
        static const char * name() { return "HSV"; }

        /**  \name  Construction, destruction and assignment  */
        //@{

        /**  Default constructor.  */
        HSV()
            : h(0), s(0), v(0)
            {}

        /**  Ecplict constructor.  */
        HSV(const component_type & h_,
            const component_type & s_,
            const component_type & v_)
            : h(h_), s(s_), v(v_)
            {}

        /**  Copy constructor.  */
        HSV(const _Self & rhs_)
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
                if (source_.r > source_.g && source_.r > source_.b) {
                    if (source_.g >= source_.b) {
                        this->h = 60.0 * (source_.g - source_.b) / (source_.r - source_.b);
                        this->s = (source_.r - source_.b) / source_.r;
                    } else {
                        this->h = 60.0 * (source_.g - source_.b) / (source_.r - source_.g) + 360.0;
                        this->s = (source_.r - source_.g) / source_.r;
                    }
                    this->v = source_.r;
                } else if (source_.g > source_.r && source_.g > source_.b) {
                    if (source_.r >= source_.b) {
                        this->h = 60.0 * (source_.b - source_.r) / (source_.g - source_.b) + 120.0;
                        this->s = (source_.g - source_.b) / source_.g;
                    } else {
                        this->h = 60.0 * (source_.b - source_.r) / (source_.g - source_.r) + 120.0;
                        this->s = (source_.g - source_.r) / source_.g;
                    }
                    this->v = source_.g;
                } else {
                    if (source_.r >= source_.g) {
                        this->h = 60.0 * (source_.r - source_.g) / (source_.b - source_.g) + 240.0;
                        this->s = (source_.b - source_.g) / source_.b;
                    } else {
                        this->h = 60.0 * (source_.r - source_.g) / (source_.b - source_.r) + 240.0;
                        this->s = (source_.b - source_.r) / source_.b;
                    }
                    this->v = source_.b;
                }
                return *this;
            }

        //@}

        /**  Red colour component.  */
        component_type h;
        /**  Green colour component.  */
        component_type s;
        /**  Blue colour component.  */
        component_type v;

    }; /* class HSV */

} /* namespace gtl */

#endif /* GTL_COLOR_HSV_INCL_ */
