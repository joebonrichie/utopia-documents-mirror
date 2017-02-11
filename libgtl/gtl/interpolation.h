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

#ifndef GTL_INTERPOLATION_INCL_
#define GTL_INTERPOLATION_INCL_

/***
 *
 *  Interpolators are intended to provide a variety of interpolation algorithms
 *  that operate on Unique Sorted Associative Containers
 *  <http://www.sgi.com/tech/stl/UniqueSortedAssociativeContainer.html>
 *  (e.g. set, map)
 *
 */

#include <gtl/config.h>
#include <gtl/vector.h>
#include <gtl/orientation.h>
#include <map>
#include <cmath>

namespace gtl
{

    template< typename _ArgType, typename _ResultType >
    _ResultType interpolate_linear(const _ArgType & fraction_,
                                   const _ResultType & prev_,
                                   const _ResultType & next_)
    {
        // linearly interpolate between two points.
        return prev_ + (next_ - prev_) * fraction_;
    }

    /**
     *  \class  lerp
     *  \brief  Linear interpolation class
     */
    template< typename _ArgType, typename _ResultType >
    class lerp : public std::binary_function< std::map< _ArgType, _ResultType >, _ArgType, _ResultType >
    {
        // Convenience typedefs
        typedef lerp< _ArgType, _ResultType > _Self;
        typedef std::binary_function< std::map< _ArgType, _ResultType >, _ArgType, _ResultType > _Base;

    public:
        // Convenience typedefs
        typedef typename _Base::first_argument_type container_type;
        typedef typename _Base::second_argument_type parameter_type;

        /**  Interpolation operator.  */
        _ResultType operator () (const container_type & container_,
                                 const parameter_type & parameter_,
                                 const bool & constant_ = false) const
            {
                // Resultant value
                _ResultType result;

                // Find neighbouring 2 points
                typename container_type::const_iterator next(container_.lower_bound(parameter_));
                typename container_type::const_iterator prev(next);

                // Shortcut if the parameter exists in the container
                if (next != container_.end() && next->first == parameter_)
                {
                    return next->second;
                }

                // If before start of container
                if (next == container_.begin())
                {
                    result = next->second;
                }
                else
                {
                    // Expand to include previous
                    --prev;

                    // If after end of container
                    if (next == container_.end())
                    {
                        result = prev->second;
                    }
                    else
                    {
                        // normal segment - linear interpolation
                        result = interpolate_linear((parameter_ - prev->first) / (next->first - prev->first),
                                                    prev->second, next->second);
                    }
                }

                return result;
            }

    }; /* class lerp */


    /**
     *  \brief  Perform spherical linear interpolation.
     */
    template< typename _ArgType, typename _ResultType >
    class interpolate_spherical_linear
    {}; /* class interpolate_spherical_linear */
    template< typename _ArgType, typename _ComponentType >
    class interpolate_spherical_linear< _ArgType, vector< _ComponentType, 2 > >
    {
        // Convenience typedef
        typedef vector< _ComponentType, 2 > _ResultType;

    public:
        _ResultType operator () (const _ArgType & fraction_,
                                 const _ResultType & prev_,
                                 const _ResultType & next_)
            {
                _ComponentType prev_theta = std::atan2(prev_.y(), prev_.x());
                _ComponentType next_theta = std::atan2(next_.y(), next_.x());
                if (next_theta - prev_theta > M_PI)
                {
                    prev_theta += M_PI * 2.0;
                }
                else if (prev_theta - next_theta > M_PI)
                {
                    next_theta += M_PI * 2.0;
                }

                // Make use of a 2-vector to simultaneously interpolate both the length and angle
                _ResultType prev_spherical = vector< _ComponentType, 2 >(norm(prev_), prev_theta);
                _ResultType next_spherical = vector< _ComponentType, 2 >(norm(next_), next_theta);
                _ResultType interpolated_spherical = interpolate_linear(fraction_, prev_spherical, next_spherical);
                return _ResultType(interpolated_spherical.x() * std::cos(interpolated_spherical.y()),
                                   interpolated_spherical.x() * std::sin(interpolated_spherical.y()));
            }
    };
    template< typename _ArgType, typename _ComponentType >
    class interpolate_spherical_linear< _ArgType, vector< _ComponentType, 3 > >
    {
        // Convenience typedef
        typedef vector< _ComponentType, 3 > _ResultType;

    public:
        _ResultType operator () (const _ArgType & fraction_,
                                 _ResultType prev_, _ResultType next_)
            {
                _ComponentType theta = std::acos(dot(prev_, next_) / (norm(prev_) * norm(next_)));
                _ResultType axis = normalise(cross(normalise(prev_), normalise(next_)));
                orientation< _ComponentType, 3 > rotation = orientation< _ArgType, 3 >(theta * fraction_, axis);
                return rotation * prev_ * ((norm(prev_) + fraction_ * (norm(next_) - norm(prev_))) / norm(prev_));
            }
    };

    /**
     *  \class  slerp
     *  \brief  Spherical linear interpolation meta-class
     */
    template< typename _ArgType, typename _ResultType >
    class slerp : public std::binary_function< std::map< _ArgType, _ResultType >, _ArgType, _ResultType >
    {
        // Convenience typedefs
        typedef slerp< _ArgType, _ResultType > _Self;
        typedef std::binary_function< std::map< _ArgType, _ResultType >, _ArgType, _ResultType > _Base;

    public:
        // Convenience typedefs
        typedef typename _Base::first_argument_type container_type;
        typedef typename _Base::second_argument_type parameter_type;

        /**  Interpolation operator.  */
        _ResultType operator () (const container_type & container_,
                                 const parameter_type & parameter_,
                                 const bool & constant_ = false) const
            {
                // Resultant value
                _ResultType result;

                // Find neighbouring 2 points
                typename container_type::const_iterator next(container_.lower_bound(parameter_));
                typename container_type::const_iterator prev(next);

                // Shortcut if the parameter exists in the container
                if (next != container_.end() && next->first == parameter_)
                {
                    return next->second;
                }

                // If before start of container
                if (next == container_.begin())
                {
                    result = next->second;
                }
                else
                {
                    // Expand to include previous
                    --prev;

                    // If after end of container
                    if (next == container_.end())
                    {
                        result = prev->second;
                    }
                    else
                    {
                        // normal segment - spherical linear interpolation
                        interpolate_spherical_linear< _ArgType, _ResultType > interpolator;
                        result = interpolator((parameter_ - prev->first) / (next->first - prev->first),
                                              prev->second, next->second);
                    }
                }

                return result;
            }

    }; /* class slerp */



    template<typename _ArgType, typename _ResultType>
    _ResultType interpolate_hermite(const _ArgType & parameter_,
                                    const _ResultType & p1, const _ResultType & p2,
                                    const _ResultType & t1, const _ResultType & t2)
    {
        // convert the interval to the appropriate internal format
        const double i(parameter_);

        // calculate hermite basis functions
        const double h1(static_cast<double>(2)*i*i*i - static_cast<double>(3)*i*i + 1);
        const double h2(static_cast<double>(3)*i*i - static_cast<double>(2)*i*i*i);
        const double h3(i*i*i - static_cast<double>(2)*i*i + i);
        const double h4(i*i*i - i*i);

        return static_cast< _ResultType >(h1*p1 + h2*p2 + h3*t1 + h4*t2);
    }

    /**
     *  Kochanek Bartels interpolation
     */
    template< typename _ArgType, typename _ResultType >
    class KochanekBartelsSpline : public std::binary_function< std::map< _ArgType, _ResultType >, _ArgType, _ResultType >
    {
        // Convenience typedefs
        typedef KochanekBartelsSpline< _ArgType, _ResultType > _Self;
        typedef std::binary_function< std::map< _ArgType, _ResultType >, _ArgType, _ResultType > _Base;

    public:
        // Convenience typedefs
        typedef typename _Base::first_argument_type container_type;
        typedef typename _Base::second_argument_type parameter_type;

        /**  Default Constructor.  */
        KochanekBartelsSpline()
            : _tension(0), _bias(0), _continuity(0), _start_gradientDefined(false), _end_gradientDefined(false)
            {}

        /**  Explicit constructor.  */
        KochanekBartelsSpline(const double & tension_, const double & bias_, const double & continuity_)
            : _tension(tension_), _bias(bias_), _continuity(continuity_), _start_gradientDefined(false), _end_gradientDefined(false)
            {}

        /**
         *  Set the tension.
         */
        void tension(const double & tension_)
            { this->_tension = tension_; }

        /**
         *  Get the tension.
         */
        double tension() const
            { return this->_tension; }

        /**
         *  Set the bias.
         */
        void bias(const double & bias_)
            { this->_bias = bias_; }

        /**
         *  Get the bias.
         */
        double bias() const
            { return this->_bias; }

        /**
         *  Set the continuity.
         */
        void continuity(const double & continuity_)
            { this->_continuity = continuity_; }

        /**
         *  Get the continuity.
         */
        double continuity() const
            { return this->_continuity; }

        /**
         *  Set the initial gadient.
         */
        void start_gradient(const _ResultType & gradient_)
            {
                this->_start_gradient = gradient_;
                this->_start_gradientDefined = true;
            }

        /**
         *  Get the initial gadient.
         */
        _ResultType start_gradient() const
            { return this->_start_gradient; }

        /**
         *  Set the terminal gadient.
         */
        void end_gradient(const _ResultType & gradient_)
            {
                this->_end_gradient = gradient_;
                this->_end_gradientDefined = true;
            }

        /**
         *  Get the terminal gadient.
         */
        _ResultType end_gradient() const
            { return this->_end_gradient; }

        /**
         *  Interpolate a value from a given parameter.
         */
        _ResultType operator () (const container_type & values_,
                                 const parameter_type & parameter_,
                                 const bool & constant_ = false) const
            {
                // Resultant value
                _ResultType result;

                // Find neighbouring 4 points
                typename container_type::const_iterator c2(values_.upper_bound(parameter_));

                typename container_type::const_iterator c3(c2);
                // Is there more than one subsequent control point?
                if (c2 != values_.end()) {
                    ++c3;
                }

                typename container_type::const_iterator c1(c2);
                // Are there no prior control points?
                if (c1 == values_.begin()) {
                    c1 = values_.end();
                } else {
                    --c1;
                }

                typename container_type::const_iterator c0(c1);
                // Is there more than one prior control point?
                if (c1 != values_.end() && c1 != values_.begin()) {
                    --c0;
                } else {
                    c0 = values_.end();
                }

                // Calculate gradients of start and end of segment
                _ResultType t1 = this->_start_gradient;
                _ResultType t2 = this->_end_gradient;
                // Is the start gradient defined by neighbouring control points
                if (c2 != values_.end()) {
                    if (c0 != values_.end()) {
                        t1 = (1 - this->_tension) * (1 + this->_bias) * (1 + this->_continuity) * (c1->second - c0->second);
                        t1 += (1 - this->_tension) * (1 - this->_bias) * (1 - this->_continuity) * (c2->second - c1->second);
                        t1 /= 2.0;
                    } else if (values_.size() > 1) {
                        if (c1 != values_.end()) {
                            t1 = (c2->second - c1->second);
                        } else {
                            t1 = (c3->second - c2->second);
                        }
                    }
                }
                // Is the end gradient defined by neighbouring control points
                if (c1 != values_.end()) {
                    if (c3 != values_.end()) {
                        t2 = (1 - this->_tension) * (1 + this->_bias) * (1 - this->_continuity) * (c2->second - c1->second);
                        t2 += (1 - this->_tension) * (1 - this->_bias) * (1 + this->_continuity) * (c3->second - c2->second);
                        t2 /= 2.0;
                    } else if (values_.size() > 1) {
                        if (c2 != values_.end()) {
                            t2 = (c2->second - c1->second);
                        } else {
                            t2 = (c1->second - c0->second);
                        }
                    }
                }

                // Calculate resultant value
                if (c1 == values_.end() && c2 != values_.end()) {
                    // before start
                    // if only one value, use that
                    if (values_.size() == 1 || constant_) {
                        result = c2->second;
                    } else {
                        result = c2->second - ((t1 * (c2->first - parameter_)) / (c3->first - c2->first));
                    }
                } else if (c1 != values_.end() && c2 == values_.end()) {
                    // after end
                    // if only one value, use that
                    if (values_.size() == 1 || constant_) {
                        result = c1->second;
                    } else {
                        result = c1->second + (t2 * (parameter_ - c1->first)) / (c1->first - c0->first);
                    }
                } else if (c1 != values_.end() && c2 != values_.end()) {
                    // normal segment
                    result = interpolate_hermite< parameter_type, _ResultType >((parameter_ - c1->first) / (c2->first - c1->first), c1->second, c2->second, t1, t2);
                }

                return result;
            }

    protected:
        // Tension of the kochanek-bartels spline
        double _tension;
        // Bias of the kochanek-bartels spline
        double _bias;
        // Continuity of the kochanek-bartels spline
        double _continuity;

        // Initial gradient of the kochanek-bartels spline
        _ResultType _start_gradient;
        // Terminal gradient of the kochanek-bartels spline
        _ResultType _end_gradient;

        // Are initial and terminal gradients undefined?
        bool _start_gradientDefined;
        bool _end_gradientDefined;

    }; /* class KochanekBartelsSpline */



    /**
     *  Cardinal Spline interpolation
     */
    template< typename _ArgType, typename _ResultType >
    class CardinalSpline : public KochanekBartelsSpline< _ArgType, _ResultType >
    {
        // Convenience typedefs
        typedef CardinalSpline< _ArgType, _ResultType > _Self;
        typedef KochanekBartelsSpline< _ArgType, _ResultType > _Base;

    public:
        // Convenience typedefs
        typedef typename _Base::first_argument_type first_argument_type;
        typedef typename _Base::second_argument_type second_argument_type;
        typedef typename _Base::result_type result_type;

        /**  Default constructor.  */
        CardinalSpline()
            : _Base()
            {}

        /**  Explicit constructor.  */
        CardinalSpline(const double & tension_)
            : _Base()
            { this->tension(tension_); }

        /**  Set the tension.  */
        void tension(const double & tension_)
            { this->_tension = tension_ * 2.0 - 1; }

        /**  Get the tension.  */
        double tension() const
            { return (this->_tension + 1) / 2.0; }

        using _Base::operator();

    private:
        // Hide bias and continuity
        using _Base::bias;
        using _Base::continuity;

    }; /* class CardinalSpline */



    /**
     *  A Catmull-Rom is just a special case of the Cardinal Spline with tension=0.5
     */
    template< typename _ArgType, typename _ResultType >
    class CatmullRomSpline : public CardinalSpline< _ArgType, _ResultType >
    {
        // Convenience typedefs
        typedef CatmullRomSpline< _ArgType, _ResultType > _Self;
        typedef CardinalSpline< _ArgType, _ResultType > _Base;

    public:
        // Convenience typedefs
        typedef typename _Base::first_argument_type first_argument_type;
        typedef typename _Base::second_argument_type second_argument_type;
        typedef typename _Base::result_type result_type;

        /**  Default constructor.  */
        CatmullRomSpline()
            : _Base(0.5)
            {}

        using _Base::operator();

    private:
        // Hide tension
        using _Base::tension;

    }; /* class CatmullRomSpline */



    /**
     *  Self contained interpolation class, defaultly implementing the Catmull-Rom algorithm.
     */
    template< typename _ArgType, typename _ResultType, class _InterpolatorType = CatmullRomSpline< _ArgType, _ResultType > >
    class interpolation : public std::map< _ArgType, _ResultType >, public std::unary_function< _ArgType, _ResultType >, public _InterpolatorType
    {
        // Convenience typedefs
        typedef interpolation< _ArgType, _ResultType, _InterpolatorType > _Self;
        typedef std::unary_function< _ArgType, _ResultType > _Base;

    public:
        // Convenience typedefs
        typedef typename _Base::argument_type argument_type;
        typedef typename _Base::result_type result_type;

        /**  Default constructor.  */
        interpolation()
            : _constant(false)
            {}

        /**  Set constant interpolation for pre/post parameters  */
        void constant(const bool & constant_)
            { this->_constant = constant_; }

        /**  Get constant interpolation for pre/post parameters  */
        bool constant()
            { return this->_constant; }

        /**  Interpolation operator.  */
        result_type operator () (const argument_type & parameter_) const
            { return this->_InterpolatorType::operator()(*this, parameter_, this->_constant); }

    private:
        // Is the value constant before first control point and after last?
        bool _constant;

    }; /* class interpolation */

    typedef interpolation< float, vector_2f > twine_2f;
    typedef interpolation< float, vector_3f > twine_3f;

    typedef interpolation< double, vector_2d > twine_2d;
    typedef interpolation< double, vector_3d > twine_3d;

} /* namespace gtl */

#endif /* GTL_INTERPOLATION_INCL_ */
