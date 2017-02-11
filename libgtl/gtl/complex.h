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

#ifndef GTL_COMPLEX_INCL_
#define GTL_COMPLEX_INCL_

#include <gtl/config.h>
#include <gtl/common.h>
#include <gtl/vector.h>
#include <cmath>
#include <cstring>
#include <iostream>

namespace gtl
{

    /**
     *  \class  complex
     *  \brief  Encapsulation for complex data.
     *  \ingroup  math_complex
     */
    template< typename _ComponentType >
    class complex
    {
        // Convenience typedef
        typedef complex< _ComponentType > _Self;

    public:
        // Convenience typedefs
        typedef _ComponentType component_type;

        /**  \name  Construction and destruction  */
        //@{

        /**
         *  \brief  Default constructor.
         *
         *  Initially sets the complex to the identity.
         */
        complex()
            : _r(0), _i(0)
            {}

        /**  Explicit constructor of complex data.  */
        complex(const component_type & r_, const component_type & i_ = 0)
            : _r(r_), _i(i_)
            {}

        /**  Copy constructor of complex data.  */
        complex(const _Self & rhs_)
            { *this = rhs_; }

        //@}
        /**  \name  Complex arithmetic methods  */
        //@{

        /**  Complex norm. ||q||  */
        component_type norm() const
            { return static_cast<component_type>(std::sqrt(this->_r * this->_r + this->_i * this->_i)); }

        //@}
        /**  \name  Element access and manipulation  */
        //@{

        /**  Complex assignment operator.  */
        _Self & operator = (const _Self & rhs_)
            {
                std::memcpy(this, &rhs_, sizeof(component_type) * 2);
                return *this;
            }

        /**  Complex / Scalar assignment operator.  */
        template< typename _ScalarType >
        _Self & operator = (const _ScalarType & rhs_)
            {
                this->_r = static_cast< component_type >(rhs_);
                this->_i = 0;
                return *this;
            }

        /**  Cast this complex to a complex of different element type.  */
        template< typename _DestinationElementType >
        complex< _DestinationElementType > cast()
            {
                return complex< _DestinationElementType >(
                    static_cast< _DestinationElementType >(this->_r),
                    static_cast< _DestinationElementType >(this->_i));
            }

        /**  Implicitly cast this complex to a complex of different element type.  */
        template< class _DestinationElementType >
        operator complex< _DestinationElementType >()
            { return this->cast< _DestinationElementType >(); }

        /**  Return the const scalar value from this const complex.  */
        inline component_type r() const
            { return this->_r; }

        /**  Set the scalar value for this complex.  */
        inline void r(const component_type & r_)
            { this->_r = r_; }

        /**  Return the const vector value from this const complex.  */
        inline component_type i() const
            { return this->_i; }

        /**  Set the vector value for this complex.  */
        inline void i(const component_type & i_)
            { this->_i = i_; }

        //@}
        /**  \name  Complex arithmetic operators  */
        //@{

        /**  Complex equality.  */
        bool operator == (const _Self & rhs_) const
            { return std::memcmp(this, &rhs_, sizeof(_Self)) == 0; }

        /**  Complex inequality.  */
        bool operator != (const _Self & rhs_) const
            { return !(this->operator==(rhs_)); }

        /**  Complex / Scalar equality.  */
        template< typename _ScalarType >
        bool operator == (const _ScalarType & rhs_) const
            { return this->_r == static_cast< component_type >(rhs_) && this->_i == 0; }

        /**  Complex / Scalar inequality.  */
        template< typename _ScalarType >
        bool operator != (const _ScalarType & rhs_) const
            { return !(this->operator==(static_cast< component_type >(rhs_))); }

        /**  Complex negation.  */
        _Self operator - () const
            { return _Self(-this->_r, -this->_i); }

        /**  Complex addition.  */
        _Self operator + (const _Self & rhs_) const
            { return _Self(this->_r + rhs_.r(), this->_i + rhs_.i()); }

        /**  Complex compound addition.  */
        _Self & operator += (const _Self & rhs_)
            {
                this->_r += rhs_._r;
                this->_i += rhs_._i;
                return *this;
            }

        /**  Complex / Scalar addition.  */
        template< typename _ScalarType >
        _Self operator + (const _ScalarType & rhs_) const
            {
                return _Self(this->_r + static_cast< component_type >(rhs_),
                             this->_i);
            }

        /**  Complex / Scalar compound addition.  */
        template< typename _ScalarType >
        _Self & operator += (const _ScalarType & rhs_)
            {
                this->_r += static_cast< component_type >(rhs_);
                return *this;
            }

        /**  Complex subtraction.  */
        _Self operator - (const _Self & rhs_) const
            { return _Self(this->_r - rhs_._r, this->_i - rhs_._i); }

        /**  Complex compound subtraction.  */
        _Self & operator -= (const _Self & rhs_)
            {
                this->_r -= rhs_._r;
                this->_i -= rhs_._i;
                return *this;
            }

        /**  Complex / Scalar subtraction.  */
        template< typename _ScalarType >
        _Self operator - (const _ScalarType & rhs_) const
            {
                return _Self(this->_r - static_cast< component_type >(rhs_),
                             this->_i);
            }

        /**  Complex / Scalar compound subtraction.  */
        template< typename _ScalarType >
        _Self & operator -= (const _ScalarType & rhs_)
            {
                this->_r -= static_cast< component_type >(rhs_);
                return *this;
            }

        /**  Complex / Scalar multiplication.  */
        template< typename _ScalarType >
        _Self operator * (const _ScalarType & rhs_) const
            {
                return _Self(this->_r * static_cast< component_type >(rhs_),
                             this->_i * static_cast< component_type >(rhs_));
            }

        /**  Complex / Scalar compound multiplication.  */
        template< typename _ScalarType >
        _Self & operator *= (const _ScalarType & rhs_)
            {
                this->_r *= static_cast< component_type >(rhs_);
                this->_i *= static_cast< component_type >(rhs_);
                return *this;
            }

        /**  Complex / Scalar division.  */
        template< typename _ScalarType >
        _Self operator / (const _ScalarType & rhs_) const
            {
                return _Self(this->_r / static_cast< component_type >(rhs_),
                             this->_i / static_cast< component_type >(rhs_));
            }

        /**  Complex / Scalar compound division.  */
        template< typename _ScalarType >
        _Self & operator /= (const _ScalarType & rhs_)
            {
                this->_r /= static_cast< component_type >(rhs_);
                this->_i /= static_cast< component_type >(rhs_);
                return *this;
            }

        /**  Complex multiplication.  */
        _Self operator * (const _Self & rhs_) const
            {
                return _Self(this->_r * rhs_._r - this->_i * rhs_._i,
                             this->_i * rhs_._r + this->_r * rhs_._i);
            }

        /**  Complex division. p/q = p.inverse(q)  */
        _Self operator / (const _Self & rhs_) const
            { return this->operator*(inverse(rhs_)); }

        //@}
        /**  \name  Static factory methods.  */
        //@{

        static _Self identity()
            { return _Self(1, 0); }

        static _Self zero()
            { return _Self(0, 0); }

        //@}

    private:
        // Actual complex data.
        component_type _r;
        component_type _i;

    }; /* class Complex */

    /**  \addtogroup  math_complex  */
    //@{

    /**  Scalar / Complex addition.  */
    template< typename _ComponentType, typename _ScalarType >
    complex< _ComponentType > operator + (const _ScalarType & lhs_, const complex< _ComponentType > & rhs_)
    { return rhs_ + lhs_; }

    /**  Scalar / Complex subtraction.  */
    template< typename _ComponentType, typename _ScalarType >
    complex< _ComponentType > operator - (const _ScalarType & lhs_, const complex< _ComponentType > & rhs_)
    { return (-rhs_) + lhs_; }

    /**  Scalar / Complex multiplication.  */
    template< typename _ComponentType, typename _ScalarType >
    complex< _ComponentType > operator * (const _ScalarType & lhs_, const complex< _ComponentType > & rhs_)
    { return rhs_ * lhs_; }

    /**  Scalar / Complex division.  */
    template< typename _ComponentType, typename _ScalarType >
    complex< _ComponentType > operator / (const _ScalarType & lhs_, const complex< _ComponentType > & rhs_)
    { return complex< _ComponentType >(lhs_) / rhs_; }

    /**  Complex norm.  */
    template< typename _ComponentType >
    _ComponentType norm(const complex< _ComponentType > & c_)
    { return c_.norm(); }

    /**  Complex conjugation.  */
    template< typename _ComponentType >
    complex< _ComponentType > conjugate(const complex< _ComponentType > & c_)
    { return complex< _ComponentType >(c_.r(), -c_.i()); }

    /**  Complex inverse.  */
    template< typename _ComponentType >
    complex< _ComponentType > inverse(const complex< _ComponentType > & c_)
    { return conjugate(c_) / (c_.r() * c_.r() + c_.i() * c_.i()); }

    /**  Complex dot product.  */
    template< typename _ComponentType >
    complex< _ComponentType > dot(const complex< _ComponentType > & lhr_, const complex< _ComponentType > & rhs_)
    { return lhr_.r() * rhs_.r() + dot(lhr_.i() * rhs_.i()); }

    /**  Complex outer product.  */
    template< typename _ComponentType >
    complex< _ComponentType > outer(const complex< _ComponentType > & lhr_, const complex< _ComponentType > & rhs_)
    { return complex< _ComponentType >(0, cross(lhr_.i() * rhs_.i())); }

    /**  Complex cross product (odd product).  */
    template< typename _ComponentType >
    complex< _ComponentType > cross(const complex< _ComponentType > & lhr_, const complex< _ComponentType > & rhs_)
    { return complex< _ComponentType >(0, lhr_.r() * rhs_.i() - rhs_.r() * lhr_.i() - cross(lhr_.i() * rhs_.i())); }

    /**  Complex even product.  */
    template< typename _ComponentType >
    complex< _ComponentType > even(const complex< _ComponentType > & lhr_, const complex< _ComponentType > & rhs_)
    { return complex< _ComponentType >(lhr_.r() * rhs_.r() - dot(lhr_.i() * rhs_.i()), lhr_.r() * rhs_.i() + rhs_.r() * lhr_.i()); }

    /**  Complex argument.  */
    template< typename _ComponentType >
    _ComponentType arg(const complex< _ComponentType > & c_)
    {
        // arctan(Im(q)/Re(q))
        return std::atan2(c_.i(), c_.r());
    }

    /**  Complex natural exponential.  */
    template< typename _ComponentType >
    complex< _ComponentType > exp(const complex< _ComponentType > & c_)
    {
        // exp(s)(cos(|v|) + sgn(v)sin(|v|))
        _ComponentType length = c_.i().norm();
        return std::exp(c_.r()) * complex< _ComponentType >(std::cos(length), std::sin(length) * c_.i() / length);
    }

    /**  Complex natural logarithm.  */
    template< typename _ComponentType >
    complex< _ComponentType > log(const complex< _ComponentType > & c_)
    {
        // ln(|q|) + sgn(v)arg(q)
        return complex< _ComponentType >(std::log(c_.norm()), normalise(c_.i()) * arg(c_));
    }

    /**  Complex power.  */
    template< typename _ComponentType, typename _ScalarType >
    complex< _ComponentType > pow(const complex< _ComponentType > & c_, const _ScalarType & n_)
    {
        // exp(ln(q)n)
        return exp(log(c_) * n_);
    }

    /**  Complex sin.  */
    template< typename _ComponentType >
    complex< _ComponentType > sin(const complex< _ComponentType > & c_)
    {
        // sin(s)cosh(|v|) + cos(s)sgn(v)sinh(|v|)
        _ComponentType length = c_.i().norm();
        return complex< _ComponentType >(std::sin(c_.r()) * std::cosh(length),
                                         std::cos(c_.r()) * std::sinh(length) * c_.i() / length);
    }

    /**  Complex cos.  */
    template< typename _ComponentType >
    complex< _ComponentType > cos(const complex< _ComponentType > & c_)
    {
        // cos(s)cosh(|v|) - sin(s)sgn(v)sinh(|v|)
        _ComponentType length = c_.i().norm();
        return complex< _ComponentType >(std::cos(c_.r()) * std::cosh(length),
                                         - std::sin(c_.r()) * std::sinh(length) * c_.i() / length);
    }

    /**  Complex tan.  */
    template< typename _ComponentType >
    complex< _ComponentType > tan(const complex< _ComponentType > & c_)
    {
        // sin(q)/cos(q)
        return sin(c_) / cos(c_);
    }

    /**  Complex arcsin.  */
    template< typename _ComponentType >
    complex< _ComponentType > asin(const complex< _ComponentType > & c_)
    {
        // -sgn(v)arcsinh(q sgn(v))
        complex< _ComponentType > normal = complex< _ComponentType >(0, normalise(c_.i()));
        return -normal * asinh(c_ * normal);
    }

    /**  Complex arccos.  */
    template< typename _ComponentType >
    complex< _ComponentType > acos(const complex< _ComponentType > & c_)
    {
        // -sgn(v)arccosh(q)
        complex< _ComponentType > normal = complex< _ComponentType >(0, normalise(c_.i()));
        return -normal * acosh(c_);
    }

    /**  Complex arctan.  */
    template< typename _ComponentType >
    complex< _ComponentType > atan(const complex< _ComponentType > & c_)
    {
        // -sgn(v)arctanh(q sin(v))
        complex< _ComponentType > normal = complex< _ComponentType >(0, normalise(c_.i()));
        return -normal * atanh(c_ * normal);
    }

    /**  Complex hyperbolic sin.  */
    template< typename _ComponentType >
    complex< _ComponentType > sinh(const complex< _ComponentType > & c_)
    {
        // (exp(q) - exp(-q))/2
        return (exp(c_) - exp(-c_)) / static_cast< _ComponentType >(2);
    }

    /**  Complex hyperbolic cos.  */
    template< typename _ComponentType >
    complex< _ComponentType > cosh(const complex< _ComponentType > & c_)
    {
        // (exp(q) + exp(-q))/2
        return (exp(c_) + exp(-c_)) / static_cast< _ComponentType >(2);
    }

    /**  Complex hyperbolic tan.  */
    template< typename _ComponentType >
    complex< _ComponentType > tanh(const complex< _ComponentType > & c_)
    {
        // sinh(q)/cosh(q)
        return sinh(c_) / cosh(c_);
    }

    /**  Complex hyperbolic arcsin.  */
    template< typename _ComponentType >
    complex< _ComponentType > asinh(const complex< _ComponentType > & c_)
    { return log(c_ + pow(pow(c_, static_cast< _ComponentType >(2)) + complex< _ComponentType >(1, 0, 0, 0), static_cast< _ComponentType >(0.5))); }

    /**  Complex hyperbolic arccos.  */
    template< typename _ComponentType >
    complex< _ComponentType > acosh(const complex< _ComponentType > & c_)
    { return log(c_ + pow(pow(c_, static_cast< _ComponentType >(2)) - complex< _ComponentType >(1, 0, 0, 0), static_cast< _ComponentType >(0.5))); }

    /**  Complex hyperbolic arctan.  */
    template< typename _ComponentType >
    complex< _ComponentType > atanh(const complex< _ComponentType > & c_)
    { return (log(c_ + complex< _ComponentType >(1, 0, 0, 0)) - log(c_ - complex< _ComponentType >(1, 0, 0, 0))) / static_cast< _ComponentType >(2); }

    typedef complex< float > complex_f;
    typedef complex< double > complex_d;

    //@}
    /**  \addtogroup  ser  */
    //@{

    /**  Output stream serialiser.  */
    template< typename _ComponentType >
    std::ostream & operator << (std::ostream & os, const complex< _ComponentType > & rhs_)
    {
        os << "Complex(" << rhs_.r() << " + " << rhs_.i() << "i)";
        return os;
    }

    //@}

} /* namespace gtl */

#endif /* GTL_COMPLEX_INCL_ */
