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

#ifndef GTL_FUNCTIONAL_INCL_
#define GTL_FUNCTIONAL_INCL_

#include <gtl/config.h>
#include <functional>

namespace gtl {

    template< typename _FirstFunctionType, typename _SecondFunctionType = _FirstFunctionType >
    class bind_unary_vector2 : public std::unary_function< typename _FirstFunctionType::argument_type, vector< typename _FirstFunctionType::result_type, 2 > >
    {
        // Convenience typedefs
        typedef bind_unary_vector2< _FirstFunctionType, _SecondFunctionType > _Self;
        typedef std::unary_function< typename _FirstFunctionType::argument_type, vector< typename _FirstFunctionType::result_type, 2 > > _Base;

    public:
        // Convenience typedefs
        typedef _FirstFunctionType first_function_type;
        typedef _SecondFunctionType second_function_type;
        typedef typename _Base::argument_type argument_type;
        typedef typename _Base::result_type result_type;

        /**  Default constructor.  */
        bind_unary_vector2()
            {}

        /**  Explicit constructor.  */
        bind_unary_vector2(const first_function_type & first_)
            : _first(first_), _second(first_)
            {}

        /**  Explicit constructor.  */
        bind_unary_vector2(const first_function_type & first_, const second_function_type & second_)
            : _first(first_), _second(second_)
            {}

        /**  Function operator.  */
        result_type operator () (const argument_type & argument_) const
            { return result_type(this->_first(argument_), this->_second(argument_)); }

    private:
        // Constituent functors
        first_function_type _first;
        second_function_type _second;

    }; /* class bind_unary_vector2 */



    /**
     *  \class  Constant
     *  \brief  Returns a constant value regardless of the argument.
     */
    template< typename _ArgType, typename _ResultType = _ArgType >
    class Constant : public std::unary_function< _ArgType, _ResultType >
    {
        // Convenience typedefs
        typedef Constant< _ArgType, _ResultType > _Self;
        typedef std::unary_function< _ArgType, _ResultType > _Base;

    public:
        // Convenience typedefs
        typedef typename _Base::argument_type argument_type;
        typedef typename _Base::result_type result_type;

        /**  Default constructor.  */
        Constant(const result_type & constant_ = 1)
            : _constant(constant_)
            {}

        /**  Function operator.  */
        result_type operator () (const argument_type &) const
            { return this->_constant; }

    private:
        // The constant value
        result_type _constant;

    }; /* class Constant */

    /**
     *  \class  Linear
     *  \brief  Returns a linear scaling.
     *
     *  Uses the equation y=ax+b.
     */
    template< typename _ArgType, typename _ResultType = _ArgType >
    class Linear : public std::unary_function< _ArgType, _ResultType >
    {
        // Convenience typedefs
        typedef Linear< _ArgType, _ResultType > _Self;
        typedef std::unary_function< _ArgType, _ResultType > _Base;

    public:
        // Convenience typedefs
        typedef typename _Base::argument_type argument_type;
        typedef typename _Base::result_type result_type;

        /**  Default constructor.  */
        Linear(const argument_type & a_ = 1, const argument_type & b_ = 0)
            : _a(a_), _b(b_)
            {}

        /**  Function operator.  */
        result_type operator () (const argument_type & x_) const
            { return result_type(this->_a * x_ + this->_b); }

    private:
        // The linear co-efficients
        argument_type _a;
        argument_type _b;

    }; /* class Linear */

    typedef bind_unary_vector2< Linear< float > > linear_scaling_2f;
    typedef bind_unary_vector2< Linear< double > > linear_scaling_2d;

    /**
     *  \class  Quadratic
     *  \brief  Returns a quadratic scaling.
     *
     *  Uses the equation y=ax^2+bx+c.
     */
    template< typename _ArgType, typename _ResultType = _ArgType >
    class Quadratic : public std::unary_function< _ArgType, _ResultType >
    {
        // Convenience typedefs
        typedef Quadratic< _ArgType, _ResultType > _Self;
        typedef std::unary_function< _ArgType, _ResultType > _Base;

    public:
        // Convenience typedefs
        typedef typename _Base::argument_type argument_type;
        typedef typename _Base::result_type result_type;

        /**  Default constructor.  */
        Quadratic(const argument_type & a_ = 1, const argument_type & b_ = 0, const argument_type & c_ = 0)
            : _a(a_), _b(b_)
            {}

        /**  Function operator.  */
        result_type operator () (const argument_type & x_) const
            { return result_type(this->_a * x_ * x_ + this->_b * x_ + this->_c); }

    private:
        // The quadratic co-efficients
        argument_type _a;
        argument_type _b;
        argument_type _c;

    }; /* class Quadratic */

    /**
     *  \class  Sin
     *  \brief  Returns a sinusoidal scaling.
     *
     *  Uses the equation y=sin(x).
     */
    template< typename _ArgType, typename _ResultType = _ArgType >
    class Sin : public std::unary_function< _ArgType, _ResultType >
    {
        // Convenience typedefs
        typedef Sin< _ArgType, _ResultType > _Self;
        typedef std::unary_function< _ArgType, _ResultType > _Base;

    public:
        // Convenience typedefs
        typedef typename _Base::argument_type argument_type;
        typedef typename _Base::result_type result_type;

        /**  Default constructor.  */
        Sin()
            {}

        /**  Function operator.  */
        result_type operator () (const argument_type & x_) const
            {
                using std::sin;
                return result_type(sin(x_));
            }

    }; /* class Sin */

}

#endif /* GTL_FUNCTIONAL_INCL_ */
