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

#ifndef GTL_QUATERNION_INCL_
#define GTL_QUATERNION_INCL_

#include <gtl/config.h>
#include <gtl/common.h>
#include <gtl/vector.h>
#include <cmath>
#include <cstring>
#include <iostream>

namespace gtl
{

    /**
     *  \class quaternion
     *  \brief Encapsulation for quaternion data.
     *  \ingroup  math_quaternion
     */
    template< typename _ComponentType >
    class quaternion
    {
        // Convenience typedef
        typedef quaternion< _ComponentType > _Self;

    public:
        // Convenience typedefs
        typedef _ComponentType component_type;
        typedef vector< _ComponentType, 3 > vector_type;

        /**  \name  Construction and destruction  */
        //@{

        /**
         *  \brief  Default constructor.
         *
         *  Initially sets the quaternion to zero.
         */
        quaternion()
            : _s(0), _v(0, 0, 0)
            {}

        /**  Explicit constructor of quaternion data.  */
        quaternion(const component_type & s_, const component_type & x_, const component_type & y_, const component_type & z_)
            : _s(s_), _v(x_, y_, z_)
            {}

        /**  Explicit constructor of quaternion data.  */
        quaternion(const component_type & s_, const vector_type & vector_ = vector_type(0, 0, 0))
            : _s(s_), _v(vector_)
            {}

        /**  Copy constructor of quaternion data.  */
        quaternion(const _Self & rhs_)
            { *this = rhs_; }

        //@}
        /**  \name  Quaternion arithmetic methods  */
        //@{

        /**  Quaternion norm. ||q||  */
        component_type norm() const
            { return static_cast<component_type>(std::sqrt((double)(this->_s * this->_s + this->_v.x() * this->_v.x() + this->_v.y() * this->_v.y() + this->_v.z() * this->_v.z()))); }

        //@}
        /**  \name  Element access and manipulation  */
        //@{

        /**  Quaternion assignment operator.  */
        _Self & operator = (const _Self & rhs_)
            {
                std::memcpy(this, &rhs_, sizeof(component_type) * 4);
                return *this;
            }

        /**  Quaternion / Scalar assignment operator.  */
        template< typename _ScalarType >
        _Self & operator = (const _ScalarType & rhs_)
            {
                this->_s = static_cast< component_type >(rhs_);
                this->_v = vector_type(0, 0, 0);
                return *this;
            }

        /**  Cast this quaternion to a quaternion of different element type.  */
        template< typename _DestinationElementType >
        quaternion< _DestinationElementType > cast()
            {
                return quaternion< _DestinationElementType >(
                    static_cast< _DestinationElementType >(this->_s),
                    this->_v.template cast< _DestinationElementType >());
            }

        /**  Implicitly cast this quaternion to a quaternion of different element type.  */
        template< class _DestinationElementType >
        operator quaternion< _DestinationElementType >()
            { return this->cast< _DestinationElementType >(); }

        /**  Return the const scalar value from this const quaternion.  */
        inline component_type s() const
            { return this->_s; }

        /**  Set the scalar value for this quaternion.  */
        inline void s(const component_type & s_)
            { this->_s = s_; }

        /**  Return the const vector value from this const quaternion.  */
        inline vector_type v() const
            { return this->_v; }

        /**  Set the vector value for this quaternion.  */
        inline void v(const vector_type & v_)
            { this->_v = v_; }

        /**  Return the const x value from this const quaternion's vector.  */
        inline component_type x() const
            { return this->_v.x(); }

        /**  Set the x value for this quaternion's vector.  */
        inline void x(const component_type & x_)
            { this->_v.x(x_); }

        /**  Return the const y value from this const quaternion's vector.  */
        inline component_type y() const
            { return this->_v.y(); }

        /**  Set the y value for this quaternion's vector.  */
        inline void y(const component_type & y_)
            { this->_v.y(y_); }

        /**  Return the const z value from this const quaternion's vector.  */
        inline component_type z() const
            { return this->_v.z(); }

        /**  Set the z value for this quaternion's vector.  */
        inline void z(const component_type & z_)
            { this->_v.z(z_); }

        //@}
        /**  \name  Quaternion arithmetic operators  */
        //@{

        /**  Quaternion equality.  */
        bool operator == (const _Self & rhs_) const
            { return std::memcmp(this, &rhs_, sizeof(_Self)) == 0; }

        /**  Quaternion inequality.  */
        bool operator != (const _Self & rhs_) const
            { return !(this->operator==(rhs_)); }

        /**  Quaternion / Scalar equality.  */
        template< typename _ScalarType >
        bool operator == (const _ScalarType & rhs_) const
            { return this->_s == static_cast< component_type >(rhs_) && this->_v == vector_type(0, 0, 0); }

        /**  Quaternion / Scalar inequality.  */
        template< typename _ScalarType >
        bool operator != (const _ScalarType & rhs_) const
            { return !(this->operator==(static_cast< component_type >(rhs_))); }

        /**  Quaternion negation.  */
        _Self operator - () const
            { return _Self(-this->_s, -this->_v); }

        /**  Quaternion addition.  */
        _Self operator + (const _Self & rhs_) const
            { return _Self(this->_s + rhs_._s, this->_v + rhs_._v); }

        /**  Quaternion compound addition.  */
        _Self & operator += (const _Self & rhs_)
            {
                this->_s += rhs_._s;
                this->_v += rhs_._v;
                return *this;
            }

        /**  Quaternion / Scalar addition.  */
        template< typename _ScalarType >
        _Self operator + (const _ScalarType & rhs_) const
            {
                return _Self(this->_s + static_cast< component_type >(rhs_),
                             this->_v);
            }

        /**  Quaternion / Scalar compound addition.  */
        template< typename _ScalarType >
        _Self & operator += (const _ScalarType & rhs_)
            {
                this->_s += static_cast< component_type >(rhs_);
                return *this;
            }

        /**  Quaternion subtraction.  */
        _Self operator - (const _Self & rhs_) const
            { return _Self(this->_s - rhs_._s, this->_v - rhs_._v); }

        /**  Quaternion compound subtraction.  */
        _Self & operator -= (const _Self & rhs_)
            {
                this->_s -= rhs_._s;
                this->_v -= rhs_._v;
                return *this;
            }

        /**  Quaternion / Scalar subtraction.  */
        template< typename _ScalarType >
        _Self operator - (const _ScalarType & rhs_) const
            {
                return _Self(this->_s - static_cast< component_type >(rhs_),
                             this->_v);
            }

        /**  Quaternion / Scalar compound subtraction.  */
        template< typename _ScalarType >
        _Self & operator -= (const _ScalarType & rhs_)
            {
                this->_s -= static_cast< component_type >(rhs_);
                return *this;
            }

        /**  Quaternion / Scalar multiplication.  */
        template< typename _ScalarType >
        _Self operator * (const _ScalarType & rhs_) const
            {
                return _Self(this->_s * static_cast< component_type >(rhs_),
                             this->_v * static_cast< component_type >(rhs_));
            }

        /**  Quaternion / Scalar compound multiplication.  */
        template< typename _ScalarType >
        _Self & operator *= (const _ScalarType & rhs_)
            {
                this->_s *= static_cast< component_type >(rhs_);
                this->_v *= static_cast< component_type >(rhs_);
                return *this;
            }

        /**  Quaternion / Scalar division.  */
        template< typename _ScalarType >
        _Self operator / (const _ScalarType & rhs_) const
            {
                return _Self(this->_s / static_cast< component_type >(rhs_),
                             this->_v / static_cast< component_type >(rhs_));
            }

        /**  Quaternion / Scalar compound division.  */
        template< typename _ScalarType >
        _Self & operator /= (const _ScalarType & rhs_)
            {
                this->_s /= static_cast< component_type >(rhs_);
                this->_v /= static_cast< component_type >(rhs_);
                return *this;
            }

        /**  Quaternion multiplication.  */
        _Self operator * (const _Self & rhs_) const
            {
                return _Self(this->_s * rhs_._s - dot(this->_v, rhs_._v),
                             this->_s * rhs_._v + rhs_._s * this->_v + cross(this->_v, rhs_._v));
            }

        /**  Quaternion division. p/q = p.inverse(q)  */
        _Self operator / (const _Self & rhs_) const
            { return this->operator*(inverse(rhs_)); }

        //@}
        /**  \name  Static factory methods.  */
        //@{

        static _Self identity()
            { return _Self(1, 0, 0, 0); }

        static _Self zero()
            { return _Self(0, 0, 0, 0); }

        //@}

    private:
        // Actual quaternion data.
        component_type _s;
        vector_type _v;

    }; /* class Quaternion */

    /**  \addtogroup  math_quaternion  */
    //@{

    /**  Scalar / Quaternion addition.  */
    template< typename _ComponentType, typename _ScalarType >
    quaternion< _ComponentType > operator + (const _ScalarType & lhs_, const quaternion< _ComponentType > & rhs_)
    { return rhs_ + lhs_; }

    /**  Scalar / Quaternion subtraction.  */
    template< typename _ComponentType, typename _ScalarType >
    quaternion< _ComponentType > operator - (const _ScalarType & lhs_, const quaternion< _ComponentType > & rhs_)
    { return (-rhs_) + lhs_; }

    /**  Scalar / Quaternion multiplication.  */
    template< typename _ComponentType, typename _ScalarType >
    quaternion< _ComponentType > operator * (const _ScalarType & lhs_, const quaternion< _ComponentType > & rhs_)
    { return rhs_ * lhs_; }

    /**  Scalar / Quaternion division.  */
    template< typename _ComponentType, typename _ScalarType >
    quaternion< _ComponentType > operator / (const _ScalarType & lhs_, const quaternion< _ComponentType > & rhs_)
    { return quaternion< _ComponentType >(lhs_) / rhs_; }

    /**  Quaternion norm.  */
    template< typename _ComponentType >
    _ComponentType norm(const quaternion< _ComponentType > & q_)
    { return q_.norm(); }

    /**  Quaternion conjugation.  */
    template< typename _ComponentType >
    quaternion< _ComponentType > conjugate(const quaternion< _ComponentType > & q_)
    { return quaternion< _ComponentType >(q_.s(), -q_.v()); }

    /**  Quaternion inverse.  */
    template< typename _ComponentType >
    quaternion< _ComponentType > inverse(const quaternion< _ComponentType > & q_)
    { return conjugate(q_) / (q_.s() * q_.s() + dot(q_.v(), q_.v())); }

    /**  Quaternion dot product.  */
    template< typename _ComponentType >
    quaternion< _ComponentType > dot(const quaternion< _ComponentType > & lhs_, const quaternion< _ComponentType > & rhs_)
    { return lhs_.s() * rhs_.s() + dot(lhs_.v(), rhs_.v()); }

    /**  Quaternion outer product.  */
    template< typename _ComponentType >
    quaternion< _ComponentType > outer(const quaternion< _ComponentType > & lhs_, const quaternion< _ComponentType > & rhs_)
    { return quaternion< _ComponentType >(0, cross(lhs_.v(), rhs_.v())); }

    /**  Quaternion cross product (odd product).  */
    template< typename _ComponentType >
    quaternion< _ComponentType > cross(const quaternion< _ComponentType > & lhs_, const quaternion< _ComponentType > & rhs_)
    { return quaternion< _ComponentType >(0, lhs_.s() * rhs_.v() - rhs_.s() * lhs_.v() - cross(lhs_.v(), rhs_.v())); }

    /**  Quaternion even product.  */
    template< typename _ComponentType >
    quaternion< _ComponentType > even(const quaternion< _ComponentType > & lhs_, const quaternion< _ComponentType > & rhs_)
    { return quaternion< _ComponentType >(lhs_.s() * rhs_.s() - dot(lhs_.v(), rhs_.v()), lhs_.s() * rhs_.v() + rhs_.s() * lhs_.v()); }

    /**  Quaternion argument.  */
    template< typename _ComponentType >
    _ComponentType arg(const quaternion< _ComponentType > & q_)
    {
        // arccos(s/|q|)
        return std::acos(q_.s() / q_.norm());
    }

    /**  Quaternion natural exponential.  */
    template< typename _ComponentType >
    quaternion< _ComponentType > exp(const quaternion< _ComponentType > & q_)
    {
        // exp(s)(cos(|v|) + sgn(v)sin(|v|))
        _ComponentType length = q_.v().norm();
        return std::exp(q_.s()) * quaternion< _ComponentType >(std::cos(length), std::sin(length) * q_.v() / length);
    }

    /**  Quaternion natural logarithm.  */
    template< typename _ComponentType >
    quaternion< _ComponentType > log(const quaternion< _ComponentType > & q_)
    {
        // ln(|q|) + sgn(v)arg(q)
        return quaternion< _ComponentType >(std::log(q_.norm()), normalise(q_.v()) * arg(q_));
    }

    /**  Quaternion power.  */
    template< typename _ComponentType, typename _ScalarType >
    quaternion< _ComponentType > pow(const quaternion< _ComponentType > & q_, const _ScalarType & n_)
    {
        // exp(ln(q)n)
        return exp(log(q_) * n_);
    }

    /**  Quaternion sin.  */
    template< typename _ComponentType >
    quaternion< _ComponentType > sin(const quaternion< _ComponentType > & q_)
    {
        // sin(s)cosh(|v|) + cos(s)sgn(v)sinh(|v|)
        _ComponentType length = q_.v().norm();
        return quaternion< _ComponentType >(std::sin(q_.s()) * std::cosh(length),
                                            std::cos(q_.s()) * std::sinh(length) * q_.v() / length);
    }

    /**  Quaternion cos.  */
    template< typename _ComponentType >
    quaternion< _ComponentType > cos(const quaternion< _ComponentType > & q_)
    {
        // cos(s)cosh(|v|) - sin(s)sgn(v)sinh(|v|)
        _ComponentType length = q_.v().norm();
        return quaternion< _ComponentType >(std::cos(q_.s()) * std::cosh(length),
                                            - std::sin(q_.s()) * std::sinh(length) * q_.v() / length);
    }

    /**  Quaternion tan.  */
    template< typename _ComponentType >
    quaternion< _ComponentType > tan(const quaternion< _ComponentType > & q_)
    {
        // sin(q)/cos(q)
        return sin(q_) / cos(q_);
    }

    /**  Quaternion arcsin.  */
    template< typename _ComponentType >
    quaternion< _ComponentType > asin(const quaternion< _ComponentType > & q_)
    {
        // -sgn(v)arcsinh(q sgn(v))
        quaternion< _ComponentType > normal = quaternion< _ComponentType >(0, normalise(q_.v()));
        return -normal * asinh(q_ * normal);
    }

    /**  Quaternion arccos.  */
    template< typename _ComponentType >
    quaternion< _ComponentType > acos(const quaternion< _ComponentType > & q_)
    {
        // -sgn(v)arccosh(q)
        quaternion< _ComponentType > normal = quaternion< _ComponentType >(0, normalise(q_.v()));
        return -normal * acosh(q_);
    }

    /**  Quaternion arctan.  */
    template< typename _ComponentType >
    quaternion< _ComponentType > atan(const quaternion< _ComponentType > & q_)
    {
        // -sgn(v)arctanh(q sin(v))
        quaternion< _ComponentType > normal = quaternion< _ComponentType >(0, normalise(q_.v()));
        return -normal * atanh(q_ * normal);
    }

    /**  Quaternion hyperbolic sin.  */
    template< typename _ComponentType >
    quaternion< _ComponentType > sinh(const quaternion< _ComponentType > & q_)
    {
        // (exp(q) - exp(-q))/2
        return (exp(q_) - exp(-q_)) / static_cast< _ComponentType >(2);
    }

    /**  Quaternion hyperbolic cos.  */
    template< typename _ComponentType >
    quaternion< _ComponentType > cosh(const quaternion< _ComponentType > & q_)
    {
        // (exp(q) + exp(-q))/2
        return (exp(q_) + exp(-q_)) / static_cast< _ComponentType >(2);
    }

    /**  Quaternion hyperbolic tan.  */
    template< typename _ComponentType >
    quaternion< _ComponentType > tanh(const quaternion< _ComponentType > & q_)
    {
        // sinh(q)/cosh(q)
        return sinh(q_) / cosh(q_);
    }

    /**  Quaternion hyperbolic arcsin.  */
    template< typename _ComponentType >
    quaternion< _ComponentType > asinh(const quaternion< _ComponentType > & q_)
    { return log(q_ + pow(pow(q_, static_cast< _ComponentType >(2)) + quaternion< _ComponentType >(1, 0, 0, 0), static_cast< _ComponentType >(0.5))); }

    /**  Quaternion hyperbolic arccos.  */
    template< typename _ComponentType >
    quaternion< _ComponentType > acosh(const quaternion< _ComponentType > & q_)
    { return log(q_ + pow(pow(q_, static_cast< _ComponentType >(2)) - quaternion< _ComponentType >(1, 0, 0, 0), static_cast< _ComponentType >(0.5))); }

    /**  Quaternion hyperbolic arctan.  */
    template< typename _ComponentType >
    quaternion< _ComponentType > atanh(const quaternion< _ComponentType > & q_)
    { return (log(q_ + quaternion< _ComponentType >(1, 0, 0, 0)) - log(q_ - quaternion< _ComponentType >(1, 0, 0, 0))) / static_cast< _ComponentType >(2); }

    typedef quaternion< float > quaternion_f;
    typedef quaternion< double > quaternion_d;

    //@}
    /**  \addtogroup  ser  */
    //@{

    /**  Output stream serialiser.  */
    template< typename _ComponentType >
    std::ostream & operator << (std::ostream & os, const quaternion< _ComponentType > & rhs_)
    {
        os << "Quaternion[" << rhs_.s() << ", (" << rhs_.x() << ", " << rhs_.y() << ", " << rhs_.z() << ")]";
        return os;
    }

    //@}

} /* namespace gtl */

#endif /* GTL_QUATERNION_INCL_ */
