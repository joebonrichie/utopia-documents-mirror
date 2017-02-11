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

#ifndef GTL_SCALE2_INCL_
#define GTL_SCALE2_INCL_

// Cannot be included before <gtl/scale.h>
#ifndef GTL_SCALE_INCL_
#error Do not include <gtl/bits/scale2.h> directly; include <gtl/scale.h> instead.
#endif

#include <gtl/config.h>
#include <gtl/vector.h>
#include <gtl/matrix.h>
#include <cmath>
#include <cstring>
#include <iostream>

namespace gtl
{

    template< typename _ComponentType >
    class scale< _ComponentType, 2 >
    {
        // Convenience typedef
        typedef scale< _ComponentType, 2 > _Self;

    public:
        // Convenience typedefs
        typedef _ComponentType component_type;
        typedef vector< component_type, 2 > vector2_type;
        typedef vector< component_type, 3 > vector3_type;
        typedef matrix< component_type, 2 > matrix2_type;
        typedef matrix< component_type, 3 > matrix3_type;

        /**
         *  Convenience static variable for access to variable template
         *  parameter.
         */
        static const size_t degree = 2;

        /**  \name  Construction and destruction  */
        //@{

        /**
         *  \brief  Default constructor.
         *
         *  Creates a scaling equal to the identity scale.
         */
        scale()
            : _vector(1, 1)
            {}

        /**
         *  \brief  Explicit constructor.
         *
         *  Create a scaling from a vector.
         */
        scale(const vector2_type & vector_)
            { this->from_vector(vector_); }

        /**
         *  \brief  Explicit constructor.
         *
         *  Create a scaling from a scale matrix.
         */
        template< class _MatrixType >
        scale(const _MatrixType & matrix_)
            { this->_from_matrix(matrix_); }

        /**  Copy constructor  */
        scale(const _Self & rhs_)
            { this->operator=(rhs_); }

        //@}
        /**  \name  Element access and manipulation  */
        //@{

        /**
         *  \brief  Scale assignment.
         *  \param  rhs_  The new scale.
         *
         *  Makes this scale equal to that of \a rhs_.
         */
        _Self & operator = (const _Self & rhs_)
            {
                std::memcpy(this, &rhs_, sizeof(_Self));
                return *this;
            }

        /**  Static cast this scale to one of different element type  */
        template< class _DestinationElementType >
        scale< _DestinationElementType, 2 > cast() const
            {
                return scale< _DestinationElementType, 2 >(
                    this->_vector.template cast< _DestinationElementType >());
            }

        /**  Implicitly cast this scale to a scale of different element type  */
        template< class _DestinationElementType >
        operator scale< _DestinationElementType, 2 >() const
            { return this->cast< _DestinationElementType >(); }

        /**  Set scale using a vector  */
        void from_vector(const vector2_type & vector_)
            { this->_vector = vector_; }

        /**  Get scale as a vector  */
        void to_vector(vector2_type & vector_) const
            { vector_ = this->_vector; }

        /**  Set scale using a 2x2 scale matrix  */
        void from_matrix(const matrix2_type & matrix_)
            { this->_from_matrix(matrix_); }

        /**  Set scale using a 3x3 scale matrix  */
        void from_matrix(const matrix3_type & matrix_)
            { this->_from_matrix(matrix_); }

        /**  Get scale as a 2x2 scale matrix  */
        void to_matrix(matrix2_type & matrix_) const
            { this->_to_matrix(matrix_); }

        /**  Get scale as a 3x3 scale matrix  */
        void to_matrix(matrix3_type & matrix_) const
            { this->_to_matrix(matrix_); }

        /**  Get scale of x axis.  */
        component_type x() const
            { return this->_vector.x(); }

        /**  Set scale of x axis.  */
        void x(const component_type & x_)
            { this->_vector.x(x_); }

        /**  Get scale of y ayis.  */
        component_type y() const
            { return this->_vector.y(); }

        /**  Set scale of y ayis.  */
        void y(const component_type & y_)
            { this->_vector.y(y_); }

        //@}
        /**  \name  Scale arithmetic operators  */
        //@{

        /**  Scale equality  */
        bool operator == (const _Self & rhs_) const
            { return std::memcmp(this, &rhs_, sizeof(_Self)) == 0; }

        /**  Scale inequality  */
        bool operator != (const _Self & rhs_) const
            { return !(this->operator==(rhs_)); }

        /**
         *  \brief  Scale negation.
         *
         *  Reverses this scale.
         */
        _Self operator - () const
            { return _Self(-this->_vector); }

        /**
         *  \brief  Scale / Scalar multiplication.
         *
         *  Multiplying a scaling by a scalar value effectively
         *  multiplies the scale coefficients.
         */
        template< typename _ScalarType >
        _Self operator * (const _ScalarType & rhs_) const
            { return _Self(this->_vector * static_cast< component_type >(rhs_)); }

        /**
         *  \brief  Scale / Scalar compound multiplication.
         *
         *  Multiplying a scaling by a scalar value effectively
         *  multiplies the scale coefficients.
         */
        template< typename _ScalarType >
        _Self & operator *= (const _ScalarType & rhs_)
            {
                this->_vector *= static_cast< component_type >(rhs_);
                return *this;
            }

        /**
         *  \brief  Scale / Scalar division.
         *
         *  Dividing a scaling by a scalar value effectively
         *  reduces the scale coefficients.
         */
        template< typename _ScalarType >
        _Self operator / (const _ScalarType & rhs_) const
            { return _Self(this->_vector / static_cast< component_type >(rhs_)); }

        /**
         *  \brief  Scale / Scalar compound division.
         *
         *  Dividing a scaling by a scalar value effectively
         *  reduces the scale coefficients.
         */
        template< typename _ScalarType >
        _Self & operator /= (const _ScalarType & rhs_)
            {
                this->_vector /= static_cast< component_type >(rhs_);
                return *this;
            }

        /**
         *  \brief  Scale multiplication.
         *
         *  Multiplying two scales together yeilds the scaleal
         *  sum of the scales.
         */
        _Self operator * (const _Self & rhs_) const
            {
                return _Self(this->_vector.x() * rhs_._vector.x(),
                             this->_vector.y() * rhs_._vector.y());
            }

        /**
         *  \brief  Scale compound multiplication.
         *
         *  Multiplying two scales together yeilds the scaleal
         *  sum of the scales.
         */
        _Self & operator *= (const _Self & rhs_)
            {
                this->operator=(this->operator*(rhs_));
                return *this;
            }

        /**
         *  \brief  Scale / Vector multiplication.
         */
        vector2_type operator * (const vector2_type & rhs_) const
            {
                return vector2_type(this->_vector.x() * rhs_.x(),
                                    this->_vector.y() * rhs_.y());
            }

        /**
         *  \brief  Scale / Vector multiplication.
         */
        vector3_type operator * (const vector3_type & rhs_) const
            {
                return vector2_type(this->_vector.x() * rhs_.x(),
                                    this->_vector.y() * rhs_.y(),
                                    rhs_.z());
            }

        /**
         *  \brief  Scale / Matrix multiplication.
         */
        matrix2_type operator * (const matrix2_type & rhs_) const
            {
                // Convert to matrix and multiply
                matrix2_type matrix;
                this->to_matrix(matrix);
                return matrix * rhs_;
            }

        /**
         *  \brief  Scale / Matrix multiplication.
         */
        matrix3_type operator * (const matrix3_type & rhs_) const
            {
                // Convert to matrix and multiply
                matrix3_type matrix;
                this->to_matrix(matrix);
                return matrix * rhs_;
            }

        //@}
        /**  \name  Static factory methods  */
        //@{

        static _Self identity()
            { return _Self(1, 1); }

        static _Self zero()
            { return _Self(0, 0); }

        //@}

    private:
        // Vector representation for efficiency
        vector2_type _vector;

        // Set scale using a scale matrix. Called by specific public method names
        template< class _MatrixType >
        void _from_matrix(const _MatrixType & matrix_)
            { this->_vector = vector2_type(matrix_[0][0], matrix_[1][1]); }

        // Get scale as a scale matrix. Called by specific public method names
        template< class _MatrixType >
        void _to_matrix(_MatrixType & matrix_) const
            {
                // Reset to identity (to catch homogenous coordinates)
                matrix_ = _MatrixType::identity();

                // Compose matrix
                matrix_[0][0] = this->_vector.x();
                matrix_[1][1] = this->_vector.y();
            }

    }; /* class Scale */

    /**  \name  Scale functions  */
    //@{

    /**
     *  \brief Output stream serialiser.
     */
    template< typename _ComponentType >
    std::ostream & operator << (std::ostream & os, const scale< _ComponentType, 2 > & rhs_)
    {
        _ComponentType angle;
        rhs_.to_angle(angle);
        os << "scale2D(" << rhs_.x() << ", " << rhs_.y() << ")";
        return os;
    }

    //@}

} /* namespace gtl */

#endif /* GTL_SCALE3_INCL_ */
