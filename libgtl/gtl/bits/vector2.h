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

#ifndef GTL_VECTOR2_INCL_
#define GTL_VECTOR2_INCL_

// Cannot be included before <gtl/vector.h>
#ifndef GTL_VECTOR_INCL_
#error Do not include <gtl/bits/vector2.h> directly; include <gtl/vector.h> instead.
#endif

#include <cmath>
#include <cstring>
#include <iostream>
#include <stdexcept>

namespace gtl
{

    template< typename _ComponentType >
    class vector< _ComponentType, 2 >
    {
        // Convenience typedef
        typedef vector< _ComponentType, 2 > _Self;

    public:
        // Convenience typedef
        typedef _ComponentType component_type;

        /**  \name Construction and destruction  */
        //@{

        /**  Default constructor. Sets vector to be equal to [0, 0].  */
        vector()
            : _x(0), _y(0)
            {}

        /**
         *  \brief  Explicit constructor.
         *  \param  x_  Value of the x component.
         *  \param  y_  Value of the y component.
         *
         *  Explicitly sets vector to be equal to [\a x_, \a y_].
         */
        vector(const component_type & x_, const component_type & y_)
            : _x(x_), _y(y_)
            {}

        /**
         *  \brief  Explicit constructor.
         *  \param  c_  Value of all components.
         *
         *  Explicitly sets vector to be equal to [\a c_, \a c_].
         */
        vector(const component_type & c_)
            { this->operator=(c_); }

        /**
         *  \brief  Explicit constructor.
         *  \param  a_  Value of all components in array form.
         *
         *  Explicitly sets vector to be equal to [\a a_[0], \a a_[1]].
         */
        vector(const component_type * a_)
            { this->operator=(a_); }

        /**  Copy constructor.  */
        vector(const _Self & rhs_)
            { this->operator=(rhs_); }

        //@}
        /**  \name  Vector arithmetic methods  */
        //@{

        /**  Euclidean length of vector. ||v||  */
        component_type norm() const
            { return static_cast<component_type>(std::sqrt(this->_x * this->_x + this->_y * this->_y)); }

        //@}
        /**  \name  Vector operators  */
        //@{

        /**  Vector negation.  */
        _Self operator - () const
            { return _Self(-this->_x, -this->_y); }

        /**  Vector compound addition.  */
        _Self & operator += (const _Self & rhs_)
            {
                this->_x += rhs_._x;
                this->_y += rhs_._y;
                return *this;
            }

        /**  Vector compound subtraction.  */
        _Self & operator -= (const _Self & rhs_)
            {
                this->_x -= rhs_._x;
                this->_y -= rhs_._y;
                return *this;
            }

        /**  Vector / Scalar compound multiplication.  */
        template< typename _ScalarType >
        _Self & operator *= (const _ScalarType & rhs_)
            {
                this->_x *= rhs_;
                this->_y *= rhs_;
                return *this;
            }

        /**  Vector / Scalar compound division operator.  */
        template< typename _ScalarType >
        _Self & operator /= (const _ScalarType & rhs_)
            {
                this->_x /= rhs_;
                this->_y /= rhs_;
                return *this;
            }

        //@}
        /**  \name  Element access and manipulation  */
        //@{

        /**  Vector assignment.  */
        _Self & operator = (const _Self & rhs_)
            {
                std::memcpy(this, &rhs_, sizeof(component_type) * 2);
                return *this;
            }

        /**  Array assignment.  */
        _Self & operator = (const component_type * rhs_)
            {
                std::memcpy(this, rhs_, sizeof(component_type) * 2);
                return *this;
            }

        /**  Scalar assignment.  */
        _Self & operator = (const component_type & rhs_)
            {
                this->_x = rhs_;
                this->_y = rhs_;
                return *this;
            }

        /**  Cast this vector to a vector of different element type.  */
        template< class _DestinationElementType >
        vector< _DestinationElementType, 2 > cast() const
            {
                return vector< _DestinationElementType, 2 >(
                    static_cast< _DestinationElementType >(this->_x),
                    static_cast< _DestinationElementType >(this->_y));
            }

        /**  Implicitly cast this vector to a vector of different element type.  */
        template< class _DestinationElementType >
        operator vector< _DestinationElementType, 2 >() const
            { return this->cast< _DestinationElementType >(); }

        /**  Implicitly cast this vector to an array.  */
        operator component_type * ()
            { return &this->_x; }

        /**  Implicitly cast this vector to an array.  */
        operator component_type * const () const
            { return &this->_x; }

        /**
         *  \brief  Provides access to the data contained in the %Vector.
         *  \param  index_  The index of the element for which data should be accessed.
         *  \return  Read/write reference to data.
         *  \throw  std::out_of_range  If \a index_ is an invalid index.
         *
         *  This function provides for safer data access.  The parameter is first
         *  checked that it is in the range of the vector.  The function throws
         *  out_of_range if the check fails.
         */
        component_type & at(const size_t & index_)
            {
                // Ensure valid index
                if (index_ >= 2) {
                    std::cout << "vector2.h: Requested vector<2> index (" << index_ << ") out of bounds" << std::endl;
                    throw std::out_of_range("vector<2> range check");
                }

                return ((component_type *) this)[index_];
            }

        /**
         *  \brief  Provides access to the data contained in the %Vector.
         *  \param  index_  The index of the element for which data should be accessed.
         *  \return  Read-only reference to data.
         *  \throw  std::out_of_range  If \a index_ is an invalid index.
         *
         *  This function provides for safer data access.  The parameter is first
         *  checked that it is in the range of the vector.  The function throws
         *  out_of_range if the check fails.
         */
        const component_type & at(const size_t & index_) const
            {
                // Ensure valid index
                if (index_ >= 2) {
                    std::cout << "vector2.h: Requested vector<2> index (" << index_ << ") out of bounds" << std::endl;
                    throw std::out_of_range("vector<2> range check");
                }

                return ((component_type *) this)[index_];
            }

#include <gtl/bits/xy_swizzles.h>

        //@}

    private:
        // Actual vectorial data.
        component_type _x;
        component_type _y;

    }; /* class Vector */

    /**  \addtogroup  math_vector  */
    //@{

    using std::min;
    using std::max;

    /**  Dot product of two vectors.  */
    template< typename _ComponentType >
    _ComponentType dot(const vector< _ComponentType, 2 > & lhs_, const vector< _ComponentType, 2 > & rhs_)
    { return lhs_.x() * rhs_.x() + lhs_.y() * rhs_.y(); }

    /**  Finds the minimum vector that is greater than both vectors.  */
    template< typename _ComponentType >
    vector< _ComponentType, 2 > max(const vector< _ComponentType, 2 > & lhs_, const vector< _ComponentType, 2 > & rhs_)
    {
        return vector< _ComponentType, 2 >(gtl::max(lhs_.x(), rhs_.x()),
                                           gtl::max(lhs_.y(), rhs_.y()));
    }

    /**  Finds the maximum vector that is smaller than both vectors.  */
    template< typename _ComponentType >
    vector< _ComponentType, 2 > min(const vector< _ComponentType, 2 > & lhs_, const vector< _ComponentType, 2 > & rhs_)
    {
        return vector< _ComponentType, 2 >(gtl::min(lhs_.x(), rhs_.x()),
                                           gtl::min(lhs_.y(), rhs_.y()));
    }

    //@}
    /**  \addtogroup  ser  */
    //@{

    /**  Output stream serialiser.  */
    template< typename _ComponentType >
    std::ostream & operator << (std::ostream & os, const vector< _ComponentType, 2 > & vec_)
    {
        os << "Vector2(" << vec_.x() << ", " << vec_.y() << ")";
        return os;
    }

    //@}

} /* namespace gtl */

#endif /* GTL_VECTOR2_INCL_ */
