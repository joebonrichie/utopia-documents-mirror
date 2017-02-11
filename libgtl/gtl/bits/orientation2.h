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

#ifndef GTL_ORIENTATION2_INCL_
#define GTL_ORIENTATION2_INCL_

// Cannot be included before <gtl/orientation.h>
#ifndef GTL_ORIENTATION_INCL_
#error Do not include <gtl/bits/orientation2.h> directly; include <gtl/orientation.h> instead.
#endif

#include <gtl/config.h>
#include <gtl/vector.h>
#include <gtl/matrix.h>
#include <gtl/complex.h>
#include <cmath>
#include <cstring>
#include <iostream>

namespace gtl
{

    template< typename _ComponentType >
    class orientation< _ComponentType, 2 >
    {
        // Convenience typedef
        typedef orientation< _ComponentType, 2 > _Self;

    public:
        // Convenience typedefs
        typedef _ComponentType component_type;
        typedef complex< component_type > complex_type;
        typedef vector< component_type, 2 > vector2_type;
        typedef vector< component_type, 3 > vector3_type;
        typedef matrix< component_type, 2 > matrix2_type;
        typedef matrix< component_type, 3 > matrix3_type;

        /**
         *  Convenience static variable for access to variable template
         *  parameter.
         */
        static const size_t degree = 2;

        /**  \name  Construction and destruction.  */
        //@{

        /**
         *  \brief  Default constructor.
         *
         *  Creates an orientation equal to the identity rotation.
         */
        orientation()
            : _complex(1, 0)
            {}

        /**
         *  \brief  Explicit constructor.
         *
         *  Create an orientation from a complex number.
         */
        orientation(const complex_type & complex_)
            { this->from_complex(complex_); }

        /**
         *  \brief  Explicit constructor.
         *
         *  Create an orientation from a rotation matrix.
         */
        template< class _MatrixType >
        orientation(const _MatrixType & matrix_)
            { this->_from_matrix(matrix_); }

        /**
         *  \brief  Explicit constructor.
         *
         *  Create an orientation from an angle.
         */
        orientation(const component_type & angle_)
            { this->from_angle(angle_); }

        /**  Copy constructor.  */
        orientation(const _Self & rhs_)
            { this->operator=(rhs_); }

        //@}
        /**  \name  Element access and manipulation  */
        //@{

        /**
         *  \brief  Orientation assignment.
         *  \param  rhs_  The new orientation.
         *
         *  Makes this orientation equal to that of \a rhs_.
         */
        _Self & operator = (const _Self & rhs_)
            {
                std::memcpy(this, &rhs_, sizeof(_Self));
                return *this;
            }

        /**  Static cast this orientation to one of different element type.  */
        template< class _DestinationElementType >
        orientation< _DestinationElementType, 2 > cast() const
            {
                return orientation< _DestinationElementType, 2 >(
                    this->_complex.template cast< _DestinationElementType >());
            }

        /**  Implicitly cast this orientation to a orientation of different element type.  */
        template< class _DestinationElementType >
        operator orientation< _DestinationElementType, 2 >() const
            { return this->cast< _DestinationElementType >(); }

        /**  Set orientation using a complex number.  */
        void from_complex(const complex_type & complex_)
            { this->_complex = normalise(complex_); }

        /**  Get orientation as a complex number.  */
        void to_complex(complex_type & complex_) const
            { complex_ = this->_complex; }

        /**  Set orientation using an angle.  */
        void from_angle(const component_type & angle_)
            {
                this->_complex.r(std::cos(angle_));
                this->_complex.i(std::sin(angle_));
            }

        /**  Get orientation as an angle.  */
        void to_angle(component_type & angle_) const
            { angle_ = arg(this->_complex); }

        /**  Set orientation using a 2x2 rotation matrix.  */
        void from_matrix(const matrix2_type & matrix_)
            { this->_from_matrix(matrix_); }

        /**  Set orientation using a 3x3 rotation matrix.  */
        void from_matrix(const matrix3_type & matrix_)
            { this->_from_matrix(matrix_); }

        /**  Get orientation as a 2x2 rotation matrix.  */
        void to_matrix(matrix2_type & matrix_) const
            { this->_to_matrix(matrix_); }

        /**  Get orientation as a 3x3 rotation matrix.  */
        void to_matrix(matrix3_type & matrix_) const
            { this->_to_matrix(matrix_); }

        //@}
        /**  \name  Orientation arithmetic operators.  */
        //@{

        /**  Orientation equality.  */
        bool operator == (const _Self & rhs_) const
            { return std::memcmp(this, &rhs_, sizeof(_Self)) == 0; }

        /**  Orientation inequality.  */
        bool operator != (const _Self & rhs_) const
            { return !(this->operator==(rhs_)); }

        /**
         *  \brief  Orientation negation.
         *
         *  Reverses this orientation.
         */
        _Self operator - () const
            { return _Self(conjugate(this->_complex)); }

        /**
         *  \brief  Orientation / Scalar multiplication.
         *
         *  Multiplying an orientation by a scalar value effectively
         *  multiplies the orientation angle.
         */
        template< typename _ScalarType >
        _Self operator * (const _ScalarType & rhs_) const
            {
                component_type angle;
                this->to_angle(angle);
                return _Self(angle * static_cast< component_type >(rhs_));
            }

        /**
         *  \brief  Orientation / Scalar compound multiplication.
         *
         *  Multiplying an orientation by a scalar value effectively
         *  multiplies the orientation angle.
         */
        template< typename _ScalarType >
        _Self & operator *= (const _ScalarType & rhs_)
            {
                component_type angle;
                this->to_angle(angle);
                this->from_angle(angle * static_cast< component_type >(rhs_));
                return *this;
            }

        /**
         *  \brief  Orientation / Scalar division.
         *
         *  Dividing an orientation by a scalar value effectively
         *  reduces the orientation angle.
         */
        template< typename _ScalarType >
        _Self operator / (const _ScalarType & rhs_) const
            {
                component_type angle;
                this->to_angle(angle);
                return _Self(angle / static_cast< component_type >(rhs_));
            }

        /**
         *  \brief  Orientation / Scalar compound division.
         *
         *  Dividing an orientation by a scalar value effectively
         *  reduces the orientation angle.
         */
        template< typename _ScalarType >
        _Self & operator /= (const _ScalarType & rhs_)
            {
                component_type angle;
                this->to_angle(angle);
                this->from_angle(angle * static_cast< component_type >(rhs_));
                return *this;
            }

        /**
         *  \brief  Orientation multiplication.
         *
         *  Multiplying two orientations together yeilds the rotational
         *  sum of the orientations.
         */
        _Self operator * (const _Self & rhs_) const
            { return this->_complex * rhs_._complex; }

        /**
         *  \brief  Orientation compound multiplication.
         *
         *  Multiplying two orientations together yeilds the rotational
         *  sum of the orientations.
         */
        _Self & operator *= (const _Self & rhs_)
            {
                this->operator=(this->operator*(rhs_));
                return *this;
            }

        /**
         *  \brief  Orientation / Vector multiplication.
         */
        vector2_type operator * (const vector2_type & rhs_) const
            {
                complex_type result = this->_complex * complex_type(rhs_.x(), rhs_.y());
                return vector2_type(result.r(), result.i());
            }

        /**
         *  \brief  Orientation / Vector multiplication.
         */
        vector3_type operator * (const vector3_type & rhs_) const
            {
                complex_type result = this->_complex * complex_type(rhs_.x(), rhs_.y());
                return vector3_type(result.r(), result.i(), rhs_.z());
            }

        /**
         *  \brief  Orientation / Matrix multiplication.
         */
        matrix2_type operator * (const matrix2_type & rhs_) const
            {
                // Convert to matrix and multiply
                matrix2_type matrix;
                this->to_matrix(matrix);
                return matrix * rhs_;
            }

        /**
         *  \brief  Orientation / Matrix multiplication.
         */
        matrix3_type operator * (const matrix3_type & rhs_) const
            {
                // Convert to matrix and multiply
                matrix3_type matrix;
                this->to_matrix(matrix);
                return matrix * rhs_;
            }

        //@}
        /**  \name  Static factory methods.  */
        //@{

        static _Self identity()
            { return _Self(1, 0); }

        //@}

    private:
        // Quaternion representation for efficiency
        complex_type _complex;

        // Set orientation using a rotation matrix. Called by specific public method names
        template< class _MatrixType >
        void _from_matrix(const _MatrixType & matrix_)
            { this->_complex = normalise(complex_type(matrix_[0][0], matrix_[0][1])); }

        // Get orientation as a rotation matrix. Called by specific public method names
        template< class _MatrixType >
        void _to_matrix(_MatrixType & matrix_) const
            {
                // Reset to identity (to catch homogenous coordinates)
                matrix_ = _MatrixType::identity();

                // Compose matrix
                matrix_[0][0] = this->_complex.r();
                matrix_[0][1] = this->_complex.i();
                matrix_[1][0] = -this->_complex.i();
                matrix_[1][1] = this->_complex.r();
            }

    }; /* class Orientation */

    /**  \name  Orientation functions  */
    //@{

    /**
     *  \brief Output stream serialiser.
     */
    template< typename _ComponentType >
    std::ostream & operator << (std::ostream & os, const orientation< _ComponentType, 2 > & rhs_)
    {
        _ComponentType angle;
        rhs_.to_angle(angle);
        os << "orientation(" << angle << ")";
        return os;
    }

    //@}

} /* namespace gtl */

#endif /* GTL_ORIENTATION3_INCL_ */
