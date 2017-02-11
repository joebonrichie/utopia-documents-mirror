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

#ifndef GTL_COLOR_INCL_
#define GTL_COLOR_INCL_

#include <gtl/config.h>
#include <cstring>
#include <iostream>

// Include color spaces
#include <gtl/color/spaces.h>

namespace gtl
{

    /**
     *  \class  _color
     *  \brief  A class representing the concept of a color.
     *  \ingroup  gl_color
     *
     *  This class comprises a vector into a color space, and an alpha
     *  transparency value. Conversions between color spaces are implicit.
     */
    template< typename _ColourSpace = struct RGB< float > >
    struct _color : public _ColourSpace
    {
        // Convenience typedefs
        typedef _color< _ColourSpace > _Self;

    public:
        // Convenience typedef
        typedef typename _ColourSpace::component_type component_type;

        /**  \name  Construction, destruction and assignment  */
        //@{

        /**
         *  \brief  Default constructor.
         *
         *  Construct a color with the default values for the _color space
         *  (usually black) and an alpha value of 1 (no transparency).
         */
        _color()
            : _ColourSpace(), alpha(1)
            {}

        /**
         *  \brief  Explicit constructor.
         *  \param  c1_  First _color component value.
         *  \param  c2_  Second _color component value.
         *  \param  c3_  Third _color component value.
         *  \param  alpha_  Alpha transparenct value.
         *
         *  Construct a color using the three components given, and the
         *  given alpha value. Alpha defaults to 1 (no transparency).
         */
        _color(const component_type & c1_,
               const component_type & c2_,
               const component_type & c3_,
               const component_type & alpha_ = 1)
            : _ColourSpace(c1_, c2_, c3_), alpha(alpha_)
            {}

        /**
         *  \brief  Copy constructor.
         *
         *  Construct a _color using the components of another color of
         *  the same color space.
         */
        _color(const _Self & rhs_)
            { this->operator=(rhs_); }

        /**
         *  \brief  Conversion copy constructor.
         *
         *  Construct a color using the converted components of a color
         *  of a different color space.
         */
        template< typename _SourceColourSpace >
        explicit _color(const _color< _SourceColourSpace > & rhs_)
            { this->from(rhs_); }

        /**
         *  \brief  Assignment operator.
         *
         *  Assign to this color the components of another color of the
         *  same color space.
         */
        _Self & operator = (const _Self & rhs_)
            {
                std::memcpy(this, &rhs_, sizeof(_Self));
                return *this;
            }

        /**
         *  \brief  Conversion assignment operator.
         *
         *  Assign to this color the converted components of a color of
         *  a different color space.
         */
        template< typename _SourceColourSpace >
        _Self & operator = (const _color< _SourceColourSpace > & rhs_)
            { this->from(rhs_); }

        //@}
        /**  \name  Explicit conversion methods  */
        //@{

        // Expose conversion methods from this color space
        using _ColourSpace::from;

        /**
         *  \brief  Set this color from another.
         */
        template< typename _SourceColourSpace >
        void from(const _color< _SourceColourSpace > & rhs_)
            {
//                std::cout << "conversion from " << _SourceColourSpace::name() << " to " << _ColourSpace::name() << std::endl;
                this->from(static_cast< _SourceColourSpace >(rhs_));
                this->alpha = rhs_.alpha;
            }

        /**  Default catch-all conversion method.  */
        template< typename _DestinationColourSpace >
        void to(_color< _DestinationColourSpace > & rhs_) const
            { rhs_.from(*this); }

        /**  Default catch-all conversion method.  */
        template< typename _SourceColourSpace >
        void from(const _SourceColourSpace & rhs_)
            {
//                std::cout << " (via XYZ)" << std::endl;
                XYZ< component_type > intermediary;
                intermediary.from(rhs_);
                this->_ColourSpace::from(intermediary);
            }

        /**  Implicit cast operator.  */
        template< typename _DestinationColourSpace >
        operator _color< _DestinationColourSpace > () const
            {
                _color< _DestinationColourSpace > destination;
                this->to(destination);
                return destination;
            }

        /**  Implicit cast operator for casting to arrays.  */
        operator component_type * ()
            { return reinterpret_cast< component_type * >(this); }

        /**  Implicit cast operator for casting to const arrays.  */
        operator const component_type * () const
            { return reinterpret_cast< const component_type * const >(this); }

        //@}

        /**  Alpha transparency component.  */
        component_type alpha;

    }; /* class _color */

    /**  \addtogroup  ser  */
    //@{

    /**  Output stream serialiser.  */
    template< typename _ColourSpace >
    std::ostream & operator << (std::ostream & os, const _color< _ColourSpace > & rhs_)
    {
        os << "Color[" << _ColourSpace::name() << "](" << rhs_[0] << ", " << rhs_[1] << ", " << rhs_[2] << ", " << rhs_[3] << ")";
        return os;
    }

    //@}

    typedef _color< RGB< float > > rgba;

} /* namespace gtl */

#endif /* GTL_COLOR_INCL_ */
