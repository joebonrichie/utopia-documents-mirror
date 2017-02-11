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

#ifndef GTL_COLOR_CMY_INCL_
#define GTL_COLOR_CMY_INCL_

#include <gtl/config.h>
#include <cstring>
#include <cmath>

namespace gtl
{

    // Forward declaration (for use by other colour spaces)
    template< typename _ComponentType >
    struct CMY;

} /* namespace gtl */

// Include all other colour spaces
#include <gtl/color/spaces.h>

namespace gtl
{

    /**
     *  \class  CMY
     *  \brief  The CMY colour space.
     *  \ingroup  gl_color
     *
     *  Holds red, green and blue colour components.
     */
    template< typename _ComponentType >
    struct CMY
    {
        // Convenience typedef
        typedef CMY< _ComponentType > _Self;

    public:
        // Convenience typedefs
        typedef _ComponentType component_type;

        // Inspection static member
        static const char * name() { return "CMY"; }

        /**  \name  Construction, destruction and assignment  */
        //@{

        /**  Default constructor.  */
        CMY()
            : c(0), m(0), y(0)
            {}

        /**  Ecplict constructor.  */
        CMY(const component_type & c_,
            const component_type & m_,
            const component_type & y_)
            : c(c_), m(m_), y(y_)
            {}

        /**  Copy constructor.  */
        CMY(const _Self & rhs_)
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
                this->c = 1.0 - source_.r;
                this->m = 1.0 - source_.g;
                this->y = 1.0 - source_.b;
                return *this;
            }

        //@}

        /**  Red colour component.  */
        component_type c;
        /**  Green colour component.  */
        component_type m;
        /**  Blue colour component.  */
        component_type y;

    }; /* class CMY */

} /* namespace gtl */

#endif /* GTL_COLOR_CMY_INCL_ */
