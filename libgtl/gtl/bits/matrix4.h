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

#ifndef GTL_MATRIX4_INCL_
#define GTL_MATRIX4_INCL_

// Cannot be included before <gtl/matrix.h>
#ifndef GTL_MATRIX_INCL_
#error Do not include <gtl/bits/matrix4.h> directly; include <gtl/matrix.h> instead.
#endif

#include <gtl/vector.h>
#include <cassert>
#include <cmath>
#include <cstring>
#include <iostream>
#include <stdexcept>

namespace gtl
{

    /**  \ingroup  math_matrix  */
    template< typename _ComponentType >
    class matrix< _ComponentType, 4 >
    {
        // Convenience typedef
        typedef matrix< _ComponentType, 4 > _Self;

    public:
        // Convenience typedefs
        typedef _ComponentType component_type;
        static const size_t degree = 4;

        /**  \name Construction and destruction  */
        //@{

        /**  Default constructor for matrix data. Sets matrix to be a zero matrix  */
        matrix()
            { std::memset(this->_data, 0, sizeof(component_type) * 4 * 4); }

        /**
         *  \brief  Explicit constructor.
         *
         *  Explicitly sets matrix, using column-major ordering.
         *    ( c1_, c5_, c9_,  c13_ )
         *    ( c2_, c6_, c10_, c14_ )
         *    ( c3_, c7_, c11_, c15_ )
         *    ( c4_, c8_, c12_, c16_ )
         */
        matrix(const component_type & c1_, const component_type & c2_, const component_type & c3_, const component_type & c4_,
               const component_type & c5_, const component_type & c6_, const component_type & c7_, const component_type & c8_,
               const component_type & c9_, const component_type & c10_, const component_type & c11_, const component_type & c12_,
               const component_type & c13_, const component_type & c14_, const component_type & c15_, const component_type & c16_)
            {
                this->_data[0][0] = c1_;
                this->_data[0][1] = c2_;
                this->_data[0][2] = c3_;
                this->_data[0][3] = c4_;
                this->_data[1][0] = c5_;
                this->_data[1][1] = c6_;
                this->_data[1][2] = c7_;
                this->_data[1][3] = c8_;
                this->_data[2][0] = c9_;
                this->_data[2][1] = c10_;
                this->_data[2][2] = c11_;
                this->_data[2][3] = c12_;
                this->_data[3][0] = c13_;
                this->_data[3][1] = c14_;
                this->_data[3][2] = c15_;
                this->_data[3][3] = c16_;
            }

        /**
         *  \brief  Explicit constructor.
         *
         *  Explicitly sets matrix from an array, using column-major ordering.
         */
        matrix(const component_type * array_)
            { std::memcpy(this->_data, array_, sizeof(component_type) * 4 * 4); }

        /**  Copy constructor  */
        matrix(const _Self & rhs_)
            { this->operator=(rhs_); }

        //@}
        /**  \name  Matrix arithmetic methods  */
        //@{

        /**  Matrix determinant  */
        component_type det()
            {
                return ((this->_data[1][1] * this->_data[2][2] * this->_data[3][3] +
                         this->_data[2][1] * this->_data[3][2] * this->_data[1][3] +
                         this->_data[3][1] * this->_data[1][2] * this->_data[2][3]) -
                        (this->_data[3][1] * this->_data[2][2] * this->_data[1][3] +
                         this->_data[2][1] * this->_data[1][2] * this->_data[3][3] +
                         this->_data[1][1] * this->_data[3][2] * this->_data[2][3])) * this->_data[0][0] +
                    ((this->_data[0][1] * this->_data[1][2] * this->_data[3][3] +
                      this->_data[1][1] * this->_data[3][2] * this->_data[0][3] +
                      this->_data[3][1] * this->_data[0][2] * this->_data[1][3]) -
                     (this->_data[3][1] * this->_data[1][2] * this->_data[0][3] +
                      this->_data[1][1] * this->_data[0][2] * this->_data[3][3] +
                      this->_data[0][1] * this->_data[3][2] * this->_data[1][3])) * this->_data[2][0] -
                    (((this->_data[0][1] * this->_data[2][2] * this->_data[3][3] +
                       this->_data[2][1] * this->_data[3][2] * this->_data[0][3] +
                       this->_data[3][1] * this->_data[0][2] * this->_data[2][3]) -
                      (this->_data[3][1] * this->_data[2][2] * this->_data[0][3] +
                       this->_data[2][1] * this->_data[0][2] * this->_data[3][3] +
                       this->_data[0][1] * this->_data[3][2] * this->_data[2][3])) * this->_data[1][0] +
                     ((this->_data[0][1] * this->_data[1][2] * this->_data[2][3] +
                       this->_data[1][1] * this->_data[2][2] * this->_data[0][3] +
                       this->_data[2][1] * this->_data[0][2] * this->_data[1][3]) -
                      (this->_data[2][1] * this->_data[1][2] * this->_data[0][3] +
                       this->_data[1][1] * this->_data[0][2] * this->_data[2][3] +
                       this->_data[0][1] * this->_data[2][2] * this->_data[1][3])) * this->_data[3][0]);
            }

        /**  Matrix trace  */
        component_type trace()
            {
                return this->_data[0][0] + this->_data[1][1] +
                    this->_data[2][2] + this->_data[3][3];
            }

        //@}
        /**  \name  Matrix operators  */
        //@{

        /**  Matrix negation  */
        _Self operator - () const
            {
                return _Self(-this->_data[0][0],
                             -this->_data[0][1],
                             -this->_data[0][2],
                             -this->_data[0][3],
                             -this->_data[1][0],
                             -this->_data[1][1],
                             -this->_data[1][2],
                             -this->_data[1][3],
                             -this->_data[2][0],
                             -this->_data[2][1],
                             -this->_data[2][2],
                             -this->_data[2][3],
                             -this->_data[3][0],
                             -this->_data[3][1],
                             -this->_data[3][2],
                             -this->_data[3][3]);
            }

        /**  Matrix compound addition  */
        _Self & operator += (const _Self & rhs_)
            {
                this->_data[0][0] += rhs_._data[0][0];
                this->_data[0][1] += rhs_._data[0][1];
                this->_data[0][2] += rhs_._data[0][2];
                this->_data[0][3] += rhs_._data[0][3];
                this->_data[1][0] += rhs_._data[1][0];
                this->_data[1][1] += rhs_._data[1][1];
                this->_data[1][2] += rhs_._data[1][2];
                this->_data[1][3] += rhs_._data[1][3];
                this->_data[2][0] += rhs_._data[2][0];
                this->_data[2][1] += rhs_._data[2][1];
                this->_data[2][2] += rhs_._data[2][2];
                this->_data[2][3] += rhs_._data[2][3];
                this->_data[3][0] += rhs_._data[3][0];
                this->_data[3][1] += rhs_._data[3][1];
                this->_data[3][2] += rhs_._data[3][2];
                this->_data[3][3] += rhs_._data[3][3];
                return *this;
            }

        /**  Matrix compound subtraction  */
        _Self & operator -= (const _Self & rhs_)
            {
                this->_data[0][0] -= rhs_._data[0][0];
                this->_data[0][1] -= rhs_._data[0][1];
                this->_data[0][2] -= rhs_._data[0][2];
                this->_data[0][3] -= rhs_._data[0][3];
                this->_data[1][0] -= rhs_._data[1][0];
                this->_data[1][1] -= rhs_._data[1][1];
                this->_data[1][2] -= rhs_._data[1][2];
                this->_data[1][3] -= rhs_._data[1][3];
                this->_data[2][0] -= rhs_._data[2][0];
                this->_data[2][1] -= rhs_._data[2][1];
                this->_data[2][2] -= rhs_._data[2][2];
                this->_data[2][3] -= rhs_._data[2][3];
                this->_data[3][0] -= rhs_._data[3][0];
                this->_data[3][1] -= rhs_._data[3][1];
                this->_data[3][2] -= rhs_._data[3][2];
                this->_data[3][3] -= rhs_._data[3][3];
                return *this;
            }

        /**  Matrix / Scalar compound multiplication  */
        template< typename _ScalarType >
        _Self & operator *= (const _ScalarType & rhs_)
            {
                this->_data[0][0] *= rhs_;
                this->_data[0][1] *= rhs_;
                this->_data[0][2] *= rhs_;
                this->_data[0][3] *= rhs_;
                this->_data[1][0] *= rhs_;
                this->_data[1][1] *= rhs_;
                this->_data[1][2] *= rhs_;
                this->_data[1][3] *= rhs_;
                this->_data[2][0] *= rhs_;
                this->_data[2][1] *= rhs_;
                this->_data[2][2] *= rhs_;
                this->_data[2][3] *= rhs_;
                this->_data[3][0] *= rhs_;
                this->_data[3][1] *= rhs_;
                this->_data[3][2] *= rhs_;
                this->_data[3][3] *= rhs_;
                return *this;
            }

        /**  Matrix / Scalar compound division  */
        template< typename _ScalarType >
        _Self & operator /= (const _ScalarType & rhs_)
            {
                this->_data[0][0] /= rhs_;
                this->_data[0][1] /= rhs_;
                this->_data[0][2] /= rhs_;
                this->_data[0][3] /= rhs_;
                this->_data[1][0] /= rhs_;
                this->_data[1][1] /= rhs_;
                this->_data[1][2] /= rhs_;
                this->_data[1][3] /= rhs_;
                this->_data[2][0] /= rhs_;
                this->_data[2][1] /= rhs_;
                this->_data[2][2] /= rhs_;
                this->_data[2][3] /= rhs_;
                this->_data[3][0] /= rhs_;
                this->_data[3][1] /= rhs_;
                this->_data[3][2] /= rhs_;
                this->_data[3][3] /= rhs_;
                return *this;
            }

        //@}
        /**  \name  Element access and manipulation  */
        //@{

        /**  Matrix assignment  */
        _Self & operator = (const _Self & rhs_)
            {
                std::memcpy(this, &rhs_, sizeof(component_type) * 4 * 4);
                return *this;
            }

        /**  Cast this matrix to a matrix of different element type  */
        template< class _DestinationElementType >
        matrix< _DestinationElementType, 4 > cast() const
            {
                return matrix< _DestinationElementType, 4 >(
                    static_cast< _DestinationElementType >(this->_data[0][0]),
                    static_cast< _DestinationElementType >(this->_data[0][1]),
                    static_cast< _DestinationElementType >(this->_data[0][2]),
                    static_cast< _DestinationElementType >(this->_data[0][3]),
                    static_cast< _DestinationElementType >(this->_data[1][0]),
                    static_cast< _DestinationElementType >(this->_data[1][1]),
                    static_cast< _DestinationElementType >(this->_data[1][2]),
                    static_cast< _DestinationElementType >(this->_data[1][3]),
                    static_cast< _DestinationElementType >(this->_data[2][0]),
                    static_cast< _DestinationElementType >(this->_data[2][1]),
                    static_cast< _DestinationElementType >(this->_data[2][2]),
                    static_cast< _DestinationElementType >(this->_data[2][3]),
                    static_cast< _DestinationElementType >(this->_data[3][0]),
                    static_cast< _DestinationElementType >(this->_data[3][1]),
                    static_cast< _DestinationElementType >(this->_data[3][2]),
                    static_cast< _DestinationElementType >(this->_data[3][3]));
            }

        /**  Implicitly cast this matrix to a matrix of different element type  */
        template< class _DestinationElementType >
        operator matrix< _DestinationElementType, 4 >() const
            { return this->cast< _DestinationElementType >(); }

        /**
         *  \brief  Provides access to row data contained in the %Matrix.
         *  \param  index_  The index of the row for which data should be accessed.
         *  \return  Copy of the requested row.
         *  \throw  std::out_of_range  If \a index_ is an invalid index.
         *
         *  This function provides for safer data access.  The parameter is first
         *  checked that it is in the bounds of the matrix.  The function throws
         *  out_of_range if the check fails.
         */
        vector< component_type, 4 > row(const size_t & index_)
            {
                // Ensure a sane row
                if (index_ >= 4) {
                    std::cout << "matrix4.h: Requested matrix<4> row index (" << index_ << ") out of bounds" << std::endl;
                    throw std::out_of_range("matrix<4> range check");
                }

                return vector< component_type, 4 >(this->_data[0][index_], this->_data[1][index_], this->_data[2][index_], this->_data[3][index_]);
            }

        /**
         *  \brief  Sets row data contained in the %Matrix.
         *  \param  index_  The index of the row for which data should be set.
         *  \param  x_  Column 1 data.
         *  \param  y_  Column 2 data.
         *  \param  z_  Column 3 data.
         *  \param  w_  Column 4 data.
         *  \throw  std::out_of_range  If \a index_ is an invalid index.
         *
         *  This function provides for safer data access.  The parameter is first
         *  checked that it is in the bounds of the matrix.  The function throws
         *  out_of_range if the check fails.
         */
        void row(const size_t & index_, const component_type & x_, const component_type & y_, const component_type & z_, const component_type & w_)
            {
                // Ensure a sane row
                if (index_ >= 4) {
                    std::cout << "matrix4.h: Requested matrix<4> row index (" << index_ << ") out of bounds" << std::endl;
                    throw std::out_of_range("matrix<4> range check");
                }

                this->_data[0][index_] = x_;
                this->_data[1][index_] = y_;
                this->_data[2][index_] = z_;
                this->_data[3][index_] = w_;
            }

        /**
         *  \brief  Sets row data contained in the %Matrix.
         *  \param  index_  The index of the row for which data should be set.
         *  \param  row_  Row data.
         *  \throw  std::out_of_range  If \a index_ is an invalid index.
         *
         *  This function provides for safer data access.  The parameter is first
         *  checked that it is in the bounds of the matrix.  The function throws
         *  out_of_range if the check fails.
         */
        void row(const size_t & index_, const vector< component_type, 4 > & row_)
            {
                // Ensure a sane row
                if (index_ >= 4) {
                    std::cout << "matrix4.h: Requested matrix<4> row index (" << index_ << ") out of bounds" << std::endl;
                    throw std::out_of_range("matrix<4> range check");
                }

                this->_data[0][index_] = row_.x();
                this->_data[1][index_] = row_.y();
                this->_data[2][index_] = row_.z();
                this->_data[3][index_] = row_.w();
            }

        /**
         *  \brief  Provides access to column data contained in the %Matrix.
         *  \param  index_  The index of the column for which data should be accessed.
         *  \return  Copy of the requested column.
         *  \throw  std::out_of_range  If \a index_ is an invalid index.
         *
         *  This function provides for safer data access.  The parameter is first
         *  checked that it is in the bounds of the matrix.  The function throws
         *  out_of_range if the check fails.
         */
        vector< component_type, 4 > col(const size_t & index_)
            {
                // Ensure a sane column
                if (index_ >= 4) {
                    std::cout << "matrix4.h: Requested matrix<4> column index (" << index_ << ") out of bounds" << std::endl;
                    throw std::out_of_range("matrix<4> range check");
                }

                return vector< component_type, 4 >(this->_data[index_][0], this->_data[index_][1], this->_data[index_][2], this->_data[index_][3]);
            }

        /**
         *  \brief  Sets column data contained in the %Matrix.
         *  \param  index_  The index of the column for which data should be set.
         *  \param  x_  Row 1 data.
         *  \param  y_  Row 2 data.
         *  \param  z_  Row 3 data.
         *  \param  w_  Row 4 data.
         *  \throw  std::out_of_range  If \a index_ is an invalid index.
         *
         *  This function provides for safer data access.  The parameter is first
         *  checked that it is in the bounds of the matrix.  The function throws
         *  out_of_range if the check fails.
         */
        void col(const size_t & index_, const component_type & x_, const component_type & y_, const component_type & z_, const component_type & w_)
            {
                // Ensure a sane column
                if (index_ >= 4) {
                    std::cout << "matrix4.h: Requested matrix<4> column index (" << index_ << ") out of bounds" << std::endl;
                    throw std::out_of_range("matrix<4> range check");
                }

                this->_data[index_][0] = x_;
                this->_data[index_][1] = y_;
                this->_data[index_][2] = z_;
                this->_data[index_][3] = w_;
            }

        /**
         *  \brief  Sets column data contained in the %Matrix.
         *  \param  index_  The index of the column for which data should be set.
         *  \param  col_  Column data.
         *  \throw  std::out_of_range  If \a index_ is an invalid index.
         *
         *  This function provides for safer data access.  The parameter is first
         *  checked that it is in the bounds of the matrix.  The function throws
         *  out_of_range if the check fails.
         */
        void col(const size_t & index_, const vector< component_type, 4 > & col_)
            {
                // Ensure a sane column
                if (index_ >= 4) {
                    std::cout << "matrix4.h: Requested matrix<4> column index (" << index_ << ") out of bounds" << std::endl;
                    throw std::out_of_range("matrix<4> range check");
                }

                this->_data[index_][0] = col_.x();
                this->_data[index_][1] = col_.y();
                this->_data[index_][2] = col_.z();
                this->_data[index_][3] = col_.w();
            }

        /**
         *  \brief  Provides access to cell data contained in the %Matrix.
         *  \param  row_  The index of the row for which data should be accessed.
         *  \param  col_  The index of the column for which data should be accessed.
         *  \return  Read/write reference to data.
         *  \throw  std::out_of_range  If \a row_ or \a col_ are invalid indices.
         *
         *  This function provides for safer data access.  The parameters are first
         *  checked that it is in the bounds of the matrix.  The function throws
         *  out_of_range if the check fails.
         */
        component_type & at(const size_t & col_, const size_t & row_)
            {
                // Ensure a sane column / row
                if (row_ >= 4 || col_ >= 4) {
                    std::cout << "matrix4.h: Requested matrix<4> cell index (" << col_ << ", " << row_ << ") out of bounds" << std::endl;
                    throw std::out_of_range("matrix<4> range check");
                }

                return this->_data[col_][row_];
            }

        /**
         *  \brief  Provides access to cell data contained in the %Matrix.
         *  \param  row_  The index of the row for which data should be accessed.
         *  \param  col_  The index of the column for which data should be accessed.
         *  \return  Read-only reference to data.
         *  \throw  std::out_of_range  If \a row_ or \a col_ are invalid indices.
         *
         *  This function provides for safer data access.  The parameters are first
         *  checked that it is in the bounds of the matrix.  The function throws
         *  out_of_range if the check fails.
         */
        const component_type & at(const size_t & col_, const size_t & row_) const
            {
                // Ensure a sane column / row
                if (row_ >= 4 || col_ >= 4) {
                    std::cout << "matrix4.h: Requested matrix<4> cell index (" << col_ << ", " << row_ << ") out of bounds" << std::endl;
                    throw std::out_of_range("matrix<4> range check");
                }

                return this->_data[col_][row_];
            }

        /**
         *  \brief  Sets column data contained in the %Matrix.
         *  \param  row_  The index of the row for which data should be set.
         *  \param  col_  The index of the column for which data should be set.
         *  \param  val_  Cell data.
         *  \throw  std::out_of_range  If \a row_ or \a col_ are invalid indices.
         *
         *  This function provides for safer data access.  The parameters are first
         *  checked that it is in the bounds of the matrix.  The function throws
         *  out_of_range if the check fails.
         */
        void set(const size_t & col_, const size_t & row_, const component_type & val_)
            {
                // Ensure a sane column / row
                if (row_ >= 4 || col_ >= 4) {
                    std::cout << "matrix4.h: Requested matrix<4> cell index (" << col_ << ", " << row_ << ") out of bounds" << std::endl;
                    throw std::out_of_range("matrix<4> range check");
                }

                this->_data[col_][row_] = val_;
            }

        /**
         *  \brief  Subscript access to the data contained in the %Vector.
         *  \param  col_  The index of the column to be accessed.
         *  \return  Read/write reference to data.
         *
         *  This operator allows for easy, array-style, data access.
         *  Note that data access with this operator is unchecked and
         *  out_of_range lookups are not defined.
         */
        component_type * operator [] (const size_t & col_)
            { return this->_data[col_]; }

        /**
         *  \brief  Subscript access to the data contained in the %Vector.
         *  \param  col_  The index of the column to be accessed.
         *  \return  Read-only reference to data.
         *
         *  This operator allows for easy, array-style, data access.
         *  Note that data access with this operator is unchecked and
         *  out_of_range lookups are not defined.
         */
        const component_type * operator [] (const size_t & col_) const
            { return this->_data[col_]; }

        //@}
        /**  \name  Static factory methods  */
        //@{

        static _Self identity()
            {
                return _Self(1, 0, 0, 0,
                             0, 1, 0, 0,
                             0, 0, 1, 0,
                             0, 0, 0, 1);
            }

        static _Self zero()
            {
                return _Self(0, 0, 0, 0,
                             0, 0, 0, 0,
                             0, 0, 0, 0,
                             0, 0, 0, 0);
            }

        //@}

    private:
        // Actual matrix data.
        component_type _data[4][4];

    }; /* class Matrix */

} /* namespace gtl */

#endif /* GTL_MATRIX4_INCL_ */
