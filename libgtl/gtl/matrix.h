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

#ifndef GTL_MATRIX_INCL_
#define GTL_MATRIX_INCL_

#include <gtl/config.h>
#include <gtl/vector.h>
#include <cstring>
#include <iostream>

namespace gtl
{

    /**
     *  \class matrix
     *  \brief Encapsulation of matrix data.
     *  \ingroup  math_matrix
     *
     *  This graphics oriented class encapsulates matrix data of any element
     *  type, so long as the normal rules of arithmetic apply to it (addition,
     *  subtraction, multiplication and division). Specialisations exist for
     *  matrices of sizes 3 and 4, with all the standard vector arithmetic
     *  being available through either operators, methods or global functions.
     */
    template< typename _ComponentType, size_t _size >
    class matrix
    {};

    /**  \addtogroup  math_matrix  */
    //@{

    /**  Matrix / Matrix multiplication.  */
    template< typename _ComponentType, size_t _size >
    matrix< _ComponentType, _size > transpose(const matrix< _ComponentType, _size > & rhs_)
    {
        matrix< _ComponentType, _size > result;

        for (size_t x = 0; x < _size; ++x) {
            for (size_t y = 0; y < _size; ++y) {
                result[y][x] = rhs_[x][y];
            }
        }

        return result;
    }

    /**  Matrix inverse.  */
    template< typename _ComponentType, size_t _size >
    _ComponentType inverse(const matrix< _ComponentType, _size > & rhs_)
    { return rhs_.inverse(); }

    /**  Matrix determinant.  */
    template< typename _ComponentType, size_t _size >
    _ComponentType det(const matrix< _ComponentType, _size > & rhs_)
    { return rhs_.det(); }

    /**  Matrix trace.  */
    template< typename _ComponentType, size_t _size >
    _ComponentType trace(const matrix< _ComponentType, _size > & rhs_)
    { return rhs_.trace(); }

    /**  Matrix equality.  */
    template< typename _ComponentType, size_t _size >
    bool operator == (const matrix< _ComponentType, _size > & lhs_, const matrix< _ComponentType, _size > & rhs_)
    { return std::memcmp(&lhs_, &rhs_, sizeof(_ComponentType) * _size * _size) == 0; }

    /**  Matrix addition.  */
    template< typename _ComponentType, size_t _size >
    matrix< _ComponentType, _size > operator + (const matrix< _ComponentType, _size > & lhs_, const matrix< _ComponentType, _size > & rhs_)
    {
        matrix< _ComponentType, _size > result(lhs_);
        result += rhs_;
        return result;
    }

    /**  Matrix subtraction.  */
    template< typename _ComponentType, size_t _size >
    matrix< _ComponentType, _size > operator - (const matrix< _ComponentType, _size > & lhs_, const matrix< _ComponentType, _size > & rhs_)
    {
        matrix< _ComponentType, _size > result(lhs_);
        result -= rhs_;
        return result;
    }

    /**  Matrix / Scalar multiplication.  */
    template< typename _ComponentType, size_t _size, typename _ScalarType >
    matrix< _ComponentType, _size > operator * (const matrix< _ComponentType, _size > & lhs_, const _ScalarType & rhs_)
    {
        matrix< _ComponentType, _size > result(lhs_);
        result *= rhs_;
        return result;
    }

    /**  Scalar / Matrix multiplication.  */
    template< typename _ComponentType, size_t _size, typename _ScalarType >
    matrix< _ComponentType, _size > operator * (const _ScalarType & lhs_, const matrix< _ComponentType, _size > & rhs_)
    { return rhs_ * lhs_; }

    /**  Matrix / Matrix multiplication.  */
    template< typename _ComponentType, size_t _size >
    matrix< _ComponentType, _size > operator * (const matrix< _ComponentType, _size > & lhs_, const matrix< _ComponentType, _size > & rhs_)
    {
        matrix< _ComponentType, _size > result;

        for (size_t x = 0; x < _size; ++x) {
            for (size_t y = 0; y < _size; ++y) {
                _ComponentType cell = 0;
                for (size_t index = 0; index < _size; ++index) {
                    cell += lhs_[index][y] * rhs_[x][index];
                }
                result[x][y] = cell;
            }
        }

        return result;
    }

    /**  Matrix / Vector multiplication.  */
    template< typename _ComponentType, size_t _size >
    vector< _ComponentType, _size > operator * (const matrix< _ComponentType, _size > & lhs_, const vector< _ComponentType, _size > & rhs_)
    {
        vector< _ComponentType, _size > result;

        for (size_t y = 0; y < _size; ++y) {
            _ComponentType cell = 0;
            for (size_t index = 0; index < _size; ++index) {
                cell += lhs_[index][y] * rhs_[index];
            }
            result[y] = cell;
        }

        return result;
    }

    /**  Matrix / Scalar division.  */
    template< typename _ComponentType, size_t _size, typename _ScalarType >
    matrix< _ComponentType, _size > operator / (const matrix< _ComponentType, _size > & lhs_, const _ScalarType & rhs_)
    {
        matrix< _ComponentType, _size > result(lhs_);
        result /= rhs_;
        return result;
    }

    typedef matrix< float, 2 > matrix_2f;
    typedef matrix< float, 3 > matrix_3f;
    typedef matrix< float, 4 > matrix_4f;
    typedef matrix< double, 2 > matrix_2d;
    typedef matrix< double, 3 > matrix_3d;
    typedef matrix< double, 4 > matrix_4d;

    //@}
    /**  \addtogroup  ser  */
    //@{

    /**  Output stream serialiser.  */
    template< typename _ComponentType, size_t _size >
    std::ostream & operator << (std::ostream & os, const matrix< _ComponentType, _size > & rhs_)
    {
        os << "Matrix" << _size << "x" << _size << "( ";
        for (size_t x = 0; x < _size; ++x) {
            if (x != 0) os << ", ";
            os << "(";
            for (size_t y = 0; y < _size; ++y) {
                if (y != 0) os << ", ";
                os << rhs_[x][y];
            }
            os << ")";
        }
        os << " )";
        return os;
    }

    //@}

} /* namespace gtl */

// Include partial specialisations
#include <gtl/bits/matrix2.h>
#include <gtl/bits/matrix3.h>
#include <gtl/bits/matrix4.h>

#endif /* GTL_MATRIX_INCL_ */
