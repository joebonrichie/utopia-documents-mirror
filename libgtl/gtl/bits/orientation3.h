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

#ifndef GTL_ORIENTATION3_INCL_
#define GTL_ORIENTATION3_INCL_

// Cannot be included before <gtl/orientation.h>
#ifndef GTL_ORIENTATION_INCL_
#error Do not include <gtl/bits/orientation3.h> directly; include <gtl/orientation.h> instead.
#endif

#include <gtl/config.h>
#include <gtl/vector.h>
#include <gtl/matrix.h>
#include <gtl/quaternion.h>
#include <cmath>
#include <cstring>
#include <iostream>

namespace gtl
{

    template< typename _ComponentType >
    class orientation< _ComponentType, 3 >
    {
        // Convenience typedef
        typedef orientation< _ComponentType, 3 > _Self;

    public:
        // Convenience typedefs
        typedef _ComponentType component_type;
        typedef quaternion< component_type > quaternion_type;
        typedef vector< component_type, 3 > vector3_type;
        typedef vector< component_type, 4 > vector4_type;
        typedef matrix< component_type, 3 > matrix3_type;
        typedef matrix< component_type, 4 > matrix4_type;

        /**
         *  Convenience static variable for access to variable template
         *  parameter.
         */
        static const size_t degree = 3;

        /**  \name  Construction and destruction.  */
        //@{

        /**
         *  \brief  Default constructor.
         *
         *  Creates an orientation equal to the identity rotation.
         */
        orientation()
            : _quaternion(1, 0, 0, 0)
            {}

        /**
         *  \brief  Explicit constructor.
         *
         *  Create an orientation from a quaternion.
         */
        orientation(const quaternion_type & quaternion_)
            { this->from_quaternion(quaternion_); }

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
         *  Create an orientation from an angle and rotation axis.
         */
        orientation(const component_type & angle_, const vector3_type & axis_)
            { this->from_axis(angle_, axis_); }

        /**
         *  \brief  Explicit constructor.
         *
         *  Create an orientation from euler angles.
         */
        orientation(const component_type & roll_, const component_type & pitch_, const component_type & yaw_)
            { this->from_euler(roll_, pitch_, yaw_); }

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
        orientation< _DestinationElementType, 3 > cast() const
            {
                return orientation< _DestinationElementType, 3 >(
                    this->_quaternion.template cast< _DestinationElementType >());
            }

        /**  Implicitly cast this orientation to a orientation of different element type.  */
        template< class _DestinationElementType >
        operator orientation< _DestinationElementType, 3 >() const
            { return this->cast< _DestinationElementType >(); }

        /**  Set orientation using a quaternion.  */
        void from_quaternion(const quaternion_type & quaternion_)
            { this->_quaternion = normalise(quaternion_); }

        /**  Get orientation as a quaternion.  */
        void to_quaternion(quaternion_type & quaternion_) const
            { quaternion_ = this->_quaternion; }

        /**  Set orientation using an angle and axis.  */
        void from_axis(const component_type & angle_, const vector3_type & axis_)
            {
                this->_quaternion.s(std::cos(angle_ / static_cast< component_type >(2)));
                if (this->_quaternion.s() == 1) {
                    this->_quaternion.v(vector3_type(0, 0, 0));
                } else {
                    this->_quaternion.v(normalise(axis_) * std::sin(angle_ / static_cast< component_type >(2)));
                }
            }

        /**  Get orientation as an angle and axis.  */
        void to_axis(component_type & angle_, vector3_type & axis_) const
            {
                angle_ = std::acos(this->_quaternion.s()) * static_cast< component_type >(2);
                axis_ = normalise(this->_quaternion.v() * std::sin(angle_ / static_cast< component_type >(2)));
            }

        /**  Set orientation using an angle and spherical coordinates.  */
        void from_spherical(const component_type & angle_, const component_type & latitude_, const component_type & longitude_)
            {
                component_type sin_a = std::sin(angle_ / static_cast< component_type >(2));
                component_type sin_lat = std::sin(latitude_);

                this->_quaternion.s(std::cos(angle_ / static_cast< component_type >(2)));
                this->_quaternion.x(sin_a * std::cos(latitude_) * std::sin(longitude_));
                this->_quaternion.y(sin_a * sin_lat);
                this->_quaternion.z(sin_a * sin_lat * std::cos(longitude_));
                this->_quaternion = normalise(this->_quaternion);
            }

        /**  Get orientation as an angle and spherical coordinates.  */
        void to_spherical(component_type & angle_, component_type & latitude_, component_type & longitude_) const
            {
                longitude_ = 0;

                // If identity...
                if (this->_quaternion.s() == 1) {
                    angle_ = 0;
                    latitude_ = 0;
                } else {
                    // Start with axis / angle
                    vector3_type axis;
                    this->to_axis(angle_, axis);

                    // Compute latitude / longitude from axis
                    latitude_ = std::asin(axis[1]);
                    if (axis[2] != 0) {
                        longitude_ = std::atan2(axis[0], axis[2]);
                    }
                    if (longitude_ < 0) {
                        longitude_ += static_cast< component_type >(360);
                    }
                }
            }

        /**  Set orientation using an angle and euler coordinates.  */
        void from_euler(const component_type & roll_, const component_type & pitch_, const component_type & yaw_)
            {
                // Unit axis vectors
                vector3_type x(1, 0, 0);
                vector3_type y(0, 1, 0);
                vector3_type z(0, 0, 1);

                // Calculate component rotations
                _Self roll(roll_, x);
                _Self pitch(pitch_, y);
                _Self yaw(yaw_, z);

                // Compose rotation
                this->_quaternion = normalise(roll._quaternion * pitch._quaternion * yaw._quaternion);
            }

        /**  Get orientation as an angle and euler coordinates.  */
        void to_euler(component_type & roll_, component_type & pitch_, component_type & yaw_) const
            {
                // Precompute values for brevity
                component_type xx = this->_quaternion.x() * this->_quaternion.x();
                component_type yy = this->_quaternion.y() * this->_quaternion.y();
                component_type zz = this->_quaternion.z() * this->_quaternion.z();
                component_type sx = this->_quaternion.s() * this->_quaternion.x();
                component_type yz = this->_quaternion.y() * this->_quaternion.z();
                component_type xz = this->_quaternion.x() * this->_quaternion.z();
                component_type sy = this->_quaternion.s() * this->_quaternion.y();
                component_type sz = this->_quaternion.s() * this->_quaternion.z();
                component_type xy = this->_quaternion.x() * this->_quaternion.y();

                // Compute Euler angles
                roll_ = std::atan2((static_cast< component_type >(2) * (sx + yz)), static_cast< component_type >(1) - static_cast< component_type >(2) * (xx + yy));
                pitch_ = std::asin((static_cast< component_type >(2) * (xz - sy)));
                yaw_ = std::atan2((static_cast< component_type >(2) * (sz + xy)), static_cast< component_type >(1) - static_cast< component_type >(2) * (yy + zz));
            }

        /**  Set orientation using a 3x3 rotation matrix.  */
        void from_matrix(const matrix3_type & matrix_)
            { this->_from_matrix(matrix_); }

        /**  Set orientation using a 4x4 rotation matrix.  */
        void from_matrix(const matrix4_type & matrix_)
            { this->_from_matrix(matrix_); }

        /**  Get orientation as a 3x3 rotation matrix.  */
        void to_matrix(matrix3_type & matrix_) const
            { this->_to_matrix(matrix_); }

        /**  Get orientation as a 4x4 rotation matrix.  */
        void to_matrix(matrix4_type & matrix_) const
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
            { return _Self(conjugate(this->_quaternion)); }

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
                vector3_type axis;
                this->to_axis(angle, axis);
                return _Self(angle * static_cast< component_type >(rhs_), axis);
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
                vector3_type axis;
                this->to_axis(angle, axis);
                this->from_axis(angle * static_cast< component_type >(rhs_), axis);
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
                vector3_type axis;
                this->to_axis(angle, axis);
                return _Self(angle / static_cast< component_type >(rhs_), axis);
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
                vector3_type axis;
                this->to_axis(angle, axis);
                this->from_axis(angle / static_cast< component_type >(rhs_), axis);
                return *this;
            }

        /**
         *  \brief  Orientation multiplication.
         *
         *  Multiplying two orientations together yeilds the rotational
         *  sum of the orientations.
         */
        _Self operator * (const _Self & rhs_) const
            { return this->_quaternion * rhs_._quaternion; }

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
        vector3_type operator * (const vector3_type & rhs_) const
            { return (this->_quaternion * quaternion_type(0, rhs_) * inverse(this->_quaternion)).v(); }

        /**
         *  \brief  Orientation / Vector multiplication.
         */
        vector4_type operator * (const vector4_type & rhs_) const
            { return vector4_type(this->operator*(rhs_.xyz()), rhs_.w()); }

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

        /**
         *  \brief  Orientation / Matrix multiplication.
         */
        matrix4_type operator * (const matrix4_type & rhs_) const
            {
                // Convert to matrix and multiply
                matrix4_type matrix;
                this->to_matrix(matrix);
                return matrix * rhs_;
            }

        //@}
        /**  \name  Static factory methods.  */
        //@{

        static _Self identity()
            { return _Self(1, 0, 0, 0); }

        //@}

    private:
        // Quaternion representation
        quaternion_type _quaternion;

        // Set orientation using a rotation matrix. Called by specific public method names
        template< class _MatrixType >
        void _from_matrix(const _MatrixType & matrix_)
            {
                component_type trace = matrix_.trace();
                if (trace > 0) {
                    component_type s = static_cast< component_type >(0.5) / std::sqrt(trace);
                    this->_quaternion.s(static_cast< component_type >(0.25) / s);
                    this->_quaternion.x((matrix_[2][1] - matrix_[1][2]) * s);
                    this->_quaternion.y((matrix_[0][2] - matrix_[2][0]) * s);
                    this->_quaternion.z((matrix_[1][0] - matrix_[0][1]) * s);
                } else {
                    // Calculate column with greatest diagonal value
                    size_t maxCol;
                    if (matrix_[0][0] > matrix_[1][1] && matrix_[0][0] > matrix_[2][2]) {
                        maxCol = 0;
                    } else if (matrix_[1][1] > matrix_[0][0] && matrix_[1][1] > matrix_[2][2]) {
                        maxCol = 1;
                    } else {
                        maxCol = 2;
                    }

                    // Depending on this, calculate the quaternion differently
                    switch (maxCol) {
                    case 0:
                    {
                        component_type s = std::sqrt(static_cast< component_type >(1) + matrix_[0][0] - matrix_[1][1] - matrix_[2][2]) * static_cast< component_type >(2);
                        this->quaternion.x(static_cast< component_type >(0.5) / s);
                        this->quaternion.y((matrix_[0][1] + matrix_[1][0]) / s);
                        this->quaternion.z((matrix_[0][2] + matrix_[2][0]) / s);
                        this->quaternion.w((matrix_[1][2] + matrix_[2][1]) / s);
                        break;
                    }
                    case 1:
                    {
                        component_type s = std::sqrt(static_cast< component_type >(1) + matrix_[1][1] - matrix_[0][0] - matrix_[2][2]) * static_cast< component_type >(2);
                        this->quaternion.x((matrix_[0][1] + matrix_[1][0]) / s);
                        this->quaternion.y(static_cast< component_type >(0.5) / s);
                        this->quaternion.z((matrix_[1][2] + matrix_[2][1]) / s);
                        this->quaternion.w((matrix_[0][2] + matrix_[2][0]) / s);
                        break;
                    }
                    default:
                    {
                        component_type s = std::sqrt(static_cast< component_type >(1) + matrix_[2][2] - matrix_[0][0] - matrix_[1][1]) * static_cast< component_type >(2);
                        this->quaternion.x((matrix_[0][2] + matrix_[2][0]) / s);
                        this->quaternion.y((matrix_[1][2] + matrix_[2][1]) / s);
                        this->quaternion.z(static_cast< component_type >(0.5) / s);
                        this->quaternion.w((matrix_[0][1] + matrix_[1][0]) / s);
                        break;
                    }
                    }
                }

                this->_quaternion = normalise(this->_quaternion);
            }

        // Get orientation as a rotation matrix. Called by specific public method names
        template< class _MatrixType >
        void _to_matrix(_MatrixType & matrix_) const
            {
                // Reset to identity (to catch homogenous coordinates)
                matrix_ = _MatrixType::identity();

                // Compose matrix
                component_type xx = this->_quaternion.x() * this->_quaternion.x();
                component_type xy = this->_quaternion.x() * this->_quaternion.y();
                component_type xz = this->_quaternion.x() * this->_quaternion.z();
                component_type xw = this->_quaternion.x() * this->_quaternion.s();
                component_type yy = this->_quaternion.y() * this->_quaternion.y();
                component_type yz = this->_quaternion.y() * this->_quaternion.z();
                component_type yw = this->_quaternion.y() * this->_quaternion.s();
                component_type zz = this->_quaternion.z() * this->_quaternion.z();
                component_type zw = this->_quaternion.z() * this->_quaternion.s();
                matrix_[0][0] = static_cast< component_type >(1) - static_cast< component_type >(2) * (yy + zz);
                matrix_[0][1] = static_cast< component_type >(2) * (xy - zw);
                matrix_[0][2] = static_cast< component_type >(2) * (xz + yw);
                matrix_[1][0] = static_cast< component_type >(2) * (xy + zw);
                matrix_[1][1] = static_cast< component_type >(1) - static_cast< component_type >(2) * (xx + zz);
                matrix_[1][2] = static_cast< component_type >(2) * (yz - xw);
                matrix_[2][0] = static_cast< component_type >(2) * (xz - yw);
                matrix_[2][1] = static_cast< component_type >(2) * (yz + xw);
                matrix_[2][2] = static_cast< component_type >(1) - static_cast< component_type >(2) * (xx + yy);
            }

    }; /* class Orientation */

    /**  \name  Orientation functions  */
    //@{

    /**
     *  \brief Output stream serialiser.
     */
    template< typename _ComponentType >
    std::ostream & operator << (std::ostream & os, const orientation< _ComponentType, 3 > & rhs_)
    {
        typename orientation< _ComponentType, 3 >::quaternion_type quat;
        rhs_.to_quaternion(quat);
        os << "orientation(" << quat.s() << ", [" << quat.x() << ", " << quat.y() << ", " << quat.z() << "])";
        return os;
    }

    //@}

} /* namespace gtl */

#endif /* GTL_ORIENTATION3_INCL_ */
