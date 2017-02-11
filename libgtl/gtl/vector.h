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

#ifndef GTL_VECTOR_INCL_
#define GTL_VECTOR_INCL_

#include <gtl/config.h>
#include <gtl/common.h>
#include <cstring>
#include <cmath>

namespace gtl
{
    /**
     *  \class vector
     *  \brief Encapsulation of vectorial data.
     *  \ingroup  math_vector
     *
     *  This graphics oriented class encapsulates vectorial data of any element
     *  type, so long as the normal rules of arithmetic apply to it (addition,
     *  subtraction, multiplication and division). Specialisations exist for
     *  vectors of sizes 2, 3 and 4, with all the standard vector arithmetic
     *  being available through either operators, methods or global functions.
     */
    template< typename _ComponentType, size_t _size >
    class vector
    {};

    /**  \addtogroup  math_vector  */
    //@{

    /**  Vector equality.  */
    template< typename _ComponentType, size_t _size >
    bool operator == (const vector< _ComponentType, _size > & lhs_, const vector< _ComponentType, _size > & rhs_)
    { return std::memcmp(&lhs_, &rhs_, sizeof(_ComponentType) * _size) == 0; }

    /**  Vector addition.  */
    template< typename _ComponentType, size_t _size >
    vector< _ComponentType, _size > operator + (const vector< _ComponentType, _size > & lhs_, const vector< _ComponentType, _size > & rhs_)
    {
        vector< _ComponentType, _size > result(lhs_);
        result += rhs_;
        return result;
    }

    /**  Vector subtraction.  */
    template< typename _ComponentType, size_t _size >
    vector< _ComponentType, _size > operator - (const vector< _ComponentType, _size > & lhs_, const vector< _ComponentType, _size > & rhs_)
    {
        vector< _ComponentType, _size > result(lhs_);
        result -= rhs_;
        return result;
    }

    /**  Vector / Scalar multiplication.  */
    template< typename _ComponentType, size_t _size, typename Scalar >
    vector< _ComponentType, _size > operator * (const vector< _ComponentType, _size > & lhs_, const Scalar & rhs_)
    {
        vector< _ComponentType, _size > result(lhs_);
        result *= rhs_;
        return result;
    }

    /**  Scalar / Vector multiplication.  */
    template< typename _ComponentType, size_t _size, typename Scalar >
    vector< _ComponentType, _size > operator * (const Scalar & lhs_, const vector< _ComponentType, _size > & rhs_)
    { return rhs_ * lhs_; }

    /**  Vector / Scalar division.  */
    template< typename _ComponentType, size_t _size, typename Scalar >
    vector< _ComponentType, _size > operator / (const vector< _ComponentType, _size > & lhs_, const Scalar & rhs_)
    {
        vector< _ComponentType, _size > result(lhs_);
        result /= rhs_;
        return result;
    }

    typedef vector< float, 4 > vector_4f;
    typedef vector< float, 3 > vector_3f;
    typedef vector< float, 2 > vector_2f;
    typedef vector< double, 4 > vector_4d;
    typedef vector< double, 3 > vector_3d;
    typedef vector< double, 2 > vector_2d;

    //@}

} /* namespace gtl */

// Include partial specialisations
#include <gtl/bits/vector2.h>
#include <gtl/bits/vector3.h>
#include <gtl/bits/vector4.h>

#endif /* GTL_VECTOR_INCL_ */
