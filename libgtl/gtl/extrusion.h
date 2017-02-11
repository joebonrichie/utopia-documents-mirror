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

#ifndef GTL_EXTRUSION_INCL_
#define GTL_EXTRUSION_INCL_

#include <gtl/config.h>
#include <gtl/interpolation.h>
#include <gtl/functional.h>
#include <gtl/extent.h>
#include <vector>
#include <cmath>

namespace gtl
{
    /**
     *  \class  SimpleUpVector
     */
    class SimpleUpVector
    {
    public:
        /**  Calculate rotation twine of a path's up vector.  */
        template< typename _ExtrusionType >
        void operator () (const _ExtrusionType & extrusion_,
                          interpolation< typename _ExtrusionType::path_type::argument_type, typename _ExtrusionType::path_type::result_type::component_type, CardinalSpline< typename _ExtrusionType::path_type::argument_type, typename _ExtrusionType::path_type::result_type::component_type > > & rotation_) const
            {
                // Clear current up vector rotations
                rotation_.clear();

                // Just fill the rotation with zeros
                typename _ExtrusionType::path_type::const_iterator iter = extrusion_.path().begin();
                typename _ExtrusionType::path_type::const_iterator end = extrusion_.path().end();
                for (; iter != end; ++iter) {
                    rotation_[iter->first] = 0;
                }
            }

    }; /* class SimpleUpVector */

    /**
     *  \class  CentripetalUpVector
     */
    class CentripetalUpVector
    {
    public:
        /**  Calculate rotation twine of a path's up vector.  */
        template< typename _ExtrusionType >
        void operator () (const _ExtrusionType & extrusion_,
                          interpolation< typename _ExtrusionType::path_type::argument_type, typename _ExtrusionType::path_type::result_type::component_type > & rotation_) const
            {
                // Clear current up vector rotations
                rotation_.clear();

                // Set up variables for later use
                typename _ExtrusionType::path_type::result_type::component_type offset = 0;

                // Just fill the rotation with zeros
                typename _ExtrusionType::path_type::const_iterator iter = extrusion_.path().begin();
                typename _ExtrusionType::path_type::const_iterator end = extrusion_.path().end();
                for (; iter != end; ++iter) {
                    // If this is the first or last, skip
                    typename _ExtrusionType::path_type::const_iterator next(iter);
                    ++next;
                    if (iter == extrusion_.path().begin() || next == extrusion_.path().end()) {
                        continue;
                    }

                    // Find actual up vector
                    typename _ExtrusionType::path_type::result_type centre, localx; // Ignored
                    typename _ExtrusionType::path_type::result_type localy;
                    extrusion_.extrapolate(iter->first, centre, localx, localy);

                    // Find the centripetal vector
                    typename _ExtrusionType::path_type::result_type centripetal = normalise(extrusion_.path()(iter->first - 0.01) - extrusion_.path()(iter->first)) + normalise(extrusion_.path()(iter->first + 0.01) - extrusion_.path()(iter->first));
                    if (norm(centripetal) == 0) {
                        // Skip if no bend
                        if (!rotation_.empty()) {
                            rotation_[iter->first] = offset;
                        }
                        continue;
                    } else {
                        centripetal = normalise(centripetal);
                    }

                    // Find the minimum rotation around the tangent required to
                    // transform localy into centripetal vector and insert
                    // it into rotation interpolator
                    using std::acos;
                    typename _ExtrusionType::path_type::result_type::component_type prev_offset = offset;
                    typename _ExtrusionType::path_type::result_type::component_type dot_product = dot(centripetal, localy);
                    if (dot_product < -1) {
                        dot_product = -1;
                    } else if (dot_product > 1) {
                        dot_product = 1;
                    }
                    offset = acos(dot_product);
                    if (iter != extrusion_.path().begin()) {
                        // Make sure difference is minimal
                        while (prev_offset < offset - M_PI) {
                            offset -= M_PI * 2.0;
                        }
                        while (prev_offset > offset + M_PI) {
                            offset += M_PI * 2.0;
                        }
                    }
                    rotation_[iter->first] = offset;
                }

                // If the rotation is now empty, make sure there is at least a zero in it
                if (rotation_.empty()) {
                    rotation_[0] = 0;
                }
            }

    }; /* class CentripetalUpVector */

    /**
     *  \class  PartialCentripetalUpVector
     */
    class PartialCentripetalUpVector
    {
    public:
        /**  Calculate rotation twine of a path's up vector.  */
        template< typename _ExtrusionType, typename _InterpolatorType >
        void operator () (const _ExtrusionType & extrusion_,
                          interpolation< typename _ExtrusionType::path_type::argument_type, typename _ExtrusionType::path_type::result_type::component_type, _InterpolatorType > & rotation_) const
            {
                // Clear current up vector rotations
                rotation_.clear();

                // Set up variables for later use
                typename _ExtrusionType::path_type::result_type::component_type offset = 0;

                // Just fill the rotation with zeros
                typename _ExtrusionType::path_type::const_iterator iter = extrusion_.path().begin();
                typename _ExtrusionType::path_type::const_iterator end = extrusion_.path().end();
                for (; iter != end; ++iter) {
                    // If this is the first or last, skip
                    typename _ExtrusionType::path_type::const_iterator next(iter);
                    ++next;
                    if (iter == extrusion_.path().begin() || next == extrusion_.path().end()) {
                        continue;
                    }

                    // Find actual up vector
                    typename _ExtrusionType::path_type::result_type centre, localx; // Ignored
                    typename _ExtrusionType::path_type::result_type localy;
                    extrusion_.extrapolate(iter->first, centre, localx, localy);

                    // Find the centripetal vector
                    typename _ExtrusionType::path_type::result_type centripetal = normalise(extrusion_.path()(iter->first - 0.01) - extrusion_.path()(iter->first)) + normalise(extrusion_.path()(iter->first + 0.01) - extrusion_.path()(iter->first));
                    if (norm(centripetal) == 0) {
                        // Skip if no bend
                        if (!rotation_.empty()) {
                            rotation_[iter->first] = offset;
                        }
                        continue;
                    } else {
                        centripetal = normalise(centripetal);
                    }

                    // Find the minimum rotation around the tangent required to
                    // transform localy into centripetal vector and insert
                    // it into rotation interpolator
                    using std::acos;
                    typename _ExtrusionType::path_type::result_type::component_type prev_offset = offset;
                    typename _ExtrusionType::path_type::result_type::component_type dot_product = dot(centripetal, localy);
                    if (dot_product < -1) {
                        dot_product = -1;
                    } else if (dot_product > 1) {
                        dot_product = 1;
                    }
                    offset = acos(dot_product);
                    if (iter != extrusion_.path().begin()) {
                        // Make sure difference is minimal
                        while (prev_offset < offset - M_PI) {
                            offset -= M_PI * 2.0;
                        }
                        while (prev_offset > offset + M_PI) {
                            offset += M_PI * 2.0;
                        }
                        if (prev_offset < offset - M_PI / 2.0) {
                            offset -= M_PI;
                        } else if (prev_offset > offset + M_PI / 2.0) {
                            offset += M_PI;
                        }
                    }
                    rotation_[iter->first] = offset;
                }

                // If the rotation is now empty, make sure there is at least a zero in it
                if (rotation_.empty()) {
                    rotation_[0] = 0;
                }
            }

    }; /* class PartialCentripetalUpVector */

    /**
     *  \class  extrusion
     *  \brief  Class for extruding 2D shapes along 3D paths.
     *
     *
     */
    template< typename _PathType, class _UpVector = SimpleUpVector >
    class extrusion
    {
        // Convencience typedef
        typedef extrusion< _PathType, _UpVector > _Self;

    public:
        // Convencience typedef
        typedef _PathType path_type;

        typedef typename path_type::result_type vector3_type;
        typedef vector< typename vector3_type::component_type, 2 > vector2_type;

        typedef typename path_type::argument_type argument_type;
        typedef typename vector3_type::component_type component_type;

        typedef typename std::vector< vector2_type > xsection2_type;
        typedef typename std::vector< vector3_type > xsection3_type;

        /**  \name  Construction and destruction  */
        //@{

        /**  Default constructor.  */
        extrusion()
            {
                this->_rotation.tension(1.0);
                this->_rotation.constant(true);
            }

        /**  Explicit constructor.  */
        extrusion(const path_type & path_)
            : _path(path_), _range(0, 1), _frequency(1)
            {
                this->_calculate();
                this->_rotation.tension(1.0);
//                this->_rotation.constant(true);
            }

        /**  Explicit constructor.  */
        extrusion(const path_type & path_,
                  extent< component_type > range_,
                  component_type frequency_)
            : _path(path_), _range(range_), _frequency(frequency_)
            {
                this->_calculate();
                this->_rotation.tension(1.0);
//                this->_rotation.constant(true);
            }

        //@}
        /**  \name  Access and manipulation  */
        //@{

        /**  Get extrusion path.  */
        const path_type & path() const
            { return this->_path; }

        /**  Set extrusion path.  */
        void path(const path_type & path_)
            {
                this->_path = path_;
                this->_calculate();
            }

        /**  Set extrusion path.  */
        void path(const path_type & path_,
                  extent< component_type > range_,
                  component_type frequency_)
            {
                this->_range = range_;
                this->_frequency = frequency_;
                this->path(path_);
            }

        /**  Get range.  */
        const extent< component_type > & range() const
            { return this->_range; }

        /**  Set range.  */
        void range(const extent< component_type > & range_)
            {
                this->_range = range_;
                this->_calculate();
            }

        /**  Get frequency.  */
        const component_type & frequency() const
            { return this->_frequency; }

        /**  Set frequency.  */
        void frequency(const component_type & frequency_)
            {
                this->_frequency = frequency_;
                this->_calculate();
            }

        void vertices(const xsection2_type & vertices_)
            { this->_vertices = vertices_; }

        xsection3_type extrapolate_vertices(const argument_type & parameter_) const
            { return this->extrapolate_vertices(this->_vertices, parameter_, Constant< argument_type, vector2_type >()); }

        template< typename _Scaling >
        xsection3_type extrapolate_vertices(const argument_type & parameter_,
                                            const _Scaling & scaling_) const
            { return this->extrapolate_vertices(this->_vertices, parameter_, scaling_); }

        xsection3_type extrapolate_vertices(const xsection2_type & vertices_,
                                            const argument_type & parameter_) const
            { return this->extrapolate_vertices(vertices_, parameter_, Constant< argument_type, vector2_type >()); }

        template< typename _Scaling >
        xsection3_type extrapolate_vertices(const xsection2_type & vertices_,
                                            const argument_type & parameter_,
                                            const _Scaling & scaling_) const
            {
                // Resultant vertices
                xsection3_type result;

                // Find extrusion values
                vector3_type centre;
                vector3_type localx;
                vector3_type localy;
                this->extrapolate(parameter_, centre, localx, localy);

                // Perturb up-vector
                vector3_type tangent = normalise(cross(localy, localx));
                vector3_type rotate_axis = tangent;
                component_type rotate_angle = this->_rotation(parameter_);
                orientation< component_type, 3 > rotate(rotate_angle, rotate_axis);
                localy = normalise(rotate * localy);
                localx = normalise(cross(tangent, localy));

                // Extrude each normal
                typename xsection2_type::const_iterator iter = vertices_.begin();
                typename xsection2_type::const_iterator end = vertices_.end();
                for (; iter != end; ++iter) {
                    typename _Scaling::result_type scale = scaling_(parameter_);
                    result.push_back(centre + scale.x() * iter->x() * localx + scale.y() * iter->y() * localy);
                }

                return result;
            }

        void normals(const xsection2_type & normals_)
            { this->_normals = normals_; }

        xsection3_type extrapolate_normals(const argument_type & parameter_) const
            { return this->extrapolate_normals(this->_normals, parameter_, Constant< argument_type, vector2_type >()); }

        template< typename _Scaling >
        xsection3_type extrapolate_normals(const argument_type & parameter_,
                                           const _Scaling & scaling_) const
            { return this->extrapolate_normals(this->_normals, parameter_, scaling_); }

        xsection3_type extrapolate_normals(const xsection2_type & normals_,
                                           const argument_type & parameter_) const
            { return this->extrapolate_normals(normals_, parameter_, Constant< argument_type, vector2_type >()); }

        template< typename _Scaling >
        xsection3_type extrapolate_normals(const xsection2_type & normals_,
                                           const argument_type & parameter_,
                                           const _Scaling & scaling_) const
            {
                // Resultant normals
                xsection3_type result;

                // Find extrusion values
                vector3_type centre;
                vector3_type localx;
                vector3_type localy;
                this->extrapolate(parameter_, centre, localx, localy);

                // Perturb up-vector
                vector3_type tangent = normalise(cross(localy, localx));
                vector3_type rotate_axis = tangent;
                component_type rotate_angle = this->_rotation(parameter_);
                orientation< component_type, 3 > rotate(rotate_angle, rotate_axis);
                localy = normalise(rotate * localy);
                localx = normalise(rotate * localx);

                // Extrude each normal
                typename xsection2_type::const_iterator iter = normals_.begin();
                typename xsection2_type::const_iterator end = normals_.end();
                for (; iter != end; ++iter) {
                    typename _Scaling::result_type scale = scaling_(parameter_);
                    result.push_back(normalise(scale.y() * iter->x() * localx + scale.x() * iter->y() * localy));
                }

                return result;
            }

        void extrapolate(const argument_type & parameter_,
                         vector3_type & centre_,
                         vector3_type & localx_,
                         vector3_type & localy_) const
            {
                // Utilise cache
                if (parameter_ == this->_cache.parameter) {
                    centre_ = this->_cache.centre;
                    localx_ = this->_cache.localx;
                    localy_ = this->_cache.localy;
                    return;
                }

                // Find this point on the path
                centre_ = this->_path(parameter_);

                if (this->_localx_map.find(parameter_) == this->_localx_map.end()) {
                    // Tangential direction vector of this point
                    vector3_type from = this->_path(parameter_ - 0.01);
                    vector3_type to = this->_path(parameter_ + 0.01);
                    vector3_type tangent = normalise(to - from);

                    // Tangential direction vector of previous control point
                    // FIXME: Deal with points before the start of the range
                    typename path_type::const_iterator control_iter = this->_localx_map.upper_bound(parameter_);
                    --control_iter;
                    from = this->_path(control_iter->first - 0.01);
                    to = this->_path(control_iter->first + 0.01);
                    vector3_type prev_tangent = normalise(to - from);

                    // Local x and y of previous control point
                    typename std::map< argument_type, vector3_type >::const_iterator localx_iter = this->_localx_map.upper_bound(parameter_);
                    --localx_iter;
                    typename std::map< argument_type, vector3_type >::const_iterator localy_iter = this->_localy_map.upper_bound(parameter_);
                    --localy_iter;

                    // Calculate axis of rotation
                    component_type dot_product = dot(prev_tangent, tangent);
                    if (dot_product <= 1.0 && norm(cross(prev_tangent, tangent)) > 0) {
                        vector3_type rotate_axis = normalise(cross(prev_tangent, tangent));
                        component_type rotate_angle = std::acos(dot_product);

                        // Rotate the previous localy_ and localx_
                        orientation< component_type, 3 > rotate(rotate_angle, rotate_axis);
                        localy_ = normalise(rotate * localy_iter->second);
                        localx_ = normalise(cross(tangent, localy_));
                    } else {
                        localx_ = localx_iter->second;
                        localy_ = localy_iter->second;
                    }
                } else {
                    localx_ = this->_localx_map.find(parameter_)->second;
                    localy_ = this->_localy_map.find(parameter_)->second;
                }

                // Populate cache
                this->_cache.parameter = parameter_;
                this->_cache.centre = centre_;
                this->_cache.localx = localx_;
                this->_cache.localy = localy_;
            }

        //@}

    private:
        // Extrusion path
        path_type _path;
        // Extrapolation
        std::map< argument_type, vector3_type > _localx_map;
        std::map< argument_type, vector3_type > _localy_map;
        // Range and frequency
        extent< argument_type > _range;
        argument_type _frequency;
        // Cache
        struct {
            argument_type parameter;
            vector3_type centre;
            vector3_type localx;
            vector3_type localy;
        } mutable _cache;

        // x-section vertices
        xsection2_type _vertices;
        // x-section normals
        xsection2_type _normals;

        // Up vector rotation interpolation
        interpolation< argument_type, component_type, CardinalSpline< argument_type, component_type > > _rotation;

        // Up vector algorithm
        _UpVector _up_vector_algorithm;

        // Calculate extrusion
        void _calculate()
            {
                // This method will extrude the x-section along the desired
                // path, recording at specific intervals the exptrapolated
                // local coordinate system of the x-section in 3D space.

                // Clear previous extrpolations
                this->_localx_map.clear();
                this->_localy_map.clear();

                // Local x/y axes
                vector3_type localy, localx;

                // Previous tangent
                vector3_type prev_tangent;

                // Previous parameter
                argument_type parameter;

                // For each control point, calculate the coords of the xsection
                bool first = true;
                for (component_type i = this->_range.min(); i < this->_range.max(); i += this->_frequency) {
                    // If this is the first control point
                    if (first) {
                        // Record parameter
                        parameter = i;

                        // Calculate localx/y...

                        // Calculate initial tangential direction vector
                        vector3_type from = this->_path(i - 0.01);
                        vector3_type to = this->_path(i + 0.01);
                        vector3_type tangent = normalise(to - from);

                        // Remember tangent
                        prev_tangent = tangent;

                        // Choose arbitrary up point
                        vector3_type up;
                        if (tangent == vector3_type(0, 1, 0) || tangent == vector3_type(0, -1, 0)) {
                            up = vector3_type(1, 0, 0);
                        } else {
                            up = vector3_type(0, 1, 0);
                        }

                        // Use these to calculate localy and then localx
                        localy = normalise(cross(tangent, up));
                        localx = normalise(cross(tangent, localy));

                        // Record localx/y
                        this->_localx_map[parameter] = localx;
                        this->_localy_map[parameter] = localy;

                        // Populate cache
                        this->_cache.parameter = parameter;
                        this->_cache.centre = from;
                        this->_cache.localx = localx;
                        this->_cache.localy = localy;

                        first = false;
                    } else {
                        // For subsequent control points, extrapolate localx/y
                        // by subdividing the segment
                        for (size_t sub = 1; sub < 5; ++sub) {
                            // This subdivision's parameter offset
                            argument_type offset = ((i - parameter) / 5.0) * (argument_type) sub;

                            // Calculate current subdivision's tangent
                            vector3_type from = this->_path(parameter + offset - 0.01);
                            vector3_type to = this->_path(parameter + offset + 0.01);
                            vector3_type tangent = normalise(to - from);

                            // Calculate axis of rotation
                            if (dot(prev_tangent, tangent) <= 1.0 && norm(cross(prev_tangent, tangent)) > 0) {
                                vector3_type rotate_axis = normalise(cross(prev_tangent, tangent));
                                component_type rotate_angle = std::acos(dot(prev_tangent, tangent));

                                // Rotate the previous localy and infer localx
                                orientation< component_type, 3 > rotate(rotate_angle, rotate_axis);
                                localy = normalise(rotate * localy);
                                localx = normalise(cross(tangent, localy));
                            }

                            // Record localx/y
                            this->_localx_map[parameter + offset] = localx;
                            this->_localy_map[parameter + offset] = localy;

                            // Remember previous tangent
                            prev_tangent = tangent;
                        }

                        // Calculate current control point's tangent
                        vector3_type from = this->_path(i - 0.01);
                        vector3_type to = this->_path(i + 0.01);
                        vector3_type tangent = normalise(to - from);

                        // Calculate axis of rotation
                        if (dot(prev_tangent, tangent) <= 1.0 && norm(cross(prev_tangent, tangent)) > 0) {
                            vector3_type rotate_axis = normalise(cross(prev_tangent, tangent));
                            component_type rotate_angle = std::acos(dot(prev_tangent, tangent));

                            // Rotate the previous localy and infer localx
                            orientation< component_type, 3 > rotate(rotate_angle, rotate_axis);
                            localy = normalise(rotate * localy);
                            localx = normalise(cross(tangent, localy));
                        }
                        // Remember tangent
                        prev_tangent = tangent;

                        // Record localx/y
                        this->_localx_map[i] = localx;
                        this->_localy_map[i] = localy;

                        // Remember parameter
                        parameter = i;
                    }
                }

                // Calculate up-vector perturbations
                this->_up_vector_algorithm.template operator()< _Self >(*this, this->_rotation);
            }

    }; /* class extrusion */

} /* namespace gtl */

#endif /* GTL_EXTRUSION_INCL_ */
