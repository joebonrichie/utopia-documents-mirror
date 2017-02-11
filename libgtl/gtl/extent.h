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

#ifndef GTL_EXTENT_INCL_
#define GTL_EXTENT_INCL_

#include <cmath>

namespace gtl
{
    /**
     *  \class  extent
     *  \brief
     *
     */
    template < typename _ComponentType >
    class extent
    {
        // Convenience typedef
        typedef extent< _ComponentType > _Self;

    public:
        // Convenience typedef
        typedef _ComponentType component_type;

        /**  \name  Construction, destruction and assignment  */
        //@{

        /**
         *  \brief  Default constructor.
         *
         *  By default, an extent represents zero, i.e. an extent of zero
         *  size. This is achieved by setting both the minimum and maxinum
         *  elements to the same value. Exactly what that value is is
         *  irrelevant.
         */
        extent()
            { this->_max = this->_min; }

        /**
         *  \brief  Explicit constructor.
         *
         *  This constructor takes one value from the extent's space, and
         *  constructs an inialised extent of zero size at that point.
         */
        extent(const _ComponentType & element_)
            : _min(element_), _max(element_)
            {}

        /**
         *  \brief  Explicit constructor.
         *
         *  This constructor takes two values from the extent's space, and
         *  creates an extent that incorporates both values.
         */
        extent(const _ComponentType & element1_, const _ComponentType & element2_)
            {
                using std::min;
                using std::max;

                this->_min = min(element1_, element2_);
                this->_max = max(element1_, element2_);
            }

        /**  Copy constructor.  */
        extent(const _Self & rhs_)
            { this->operator=(rhs_); }

        /**  Assignment operator.  */
        _Self & operator = (const _Self & rhs_)
            {
                this->_min = rhs_._min;
                this->_max = rhs_._max;
                return *this;
            }

        //@}
        /**  \name  Element access and manipulation  */
        //@{

        /**
         *  \brief  Clears the extent.
         *
         *  This resets the extent to be of zero size.
         */
        void clear()
            { this->_max = this->_min; }

        /**
         *  \brief  Compound addition of an extent and a value.
         *
         *  An uninitialised extent will simply be initialised to this
         *  value. An already initialised extent will be extended
         *  appropriately to incorporate the new value.
         */
        _Self & operator += (const _ComponentType & rhs_)
            {
                using std::min;
                using std::max;

                // If already initialised...
                this->_min = min(this->_min, rhs_);
                this->_max = max(this->_max, rhs_);
                return *this;
            }

        /**
         *  \brief  Addition of an extent and a value.
         *
         *  An uninitialised extent will simply be initialised to this
         *  value. An already initialised extent will be extended
         *  appropriately to incorporate the new value.
         */
        _Self operator + (const _ComponentType & rhs_) const
            {
                _Self result(*this);
                result += rhs_;
                return result;
            }

        /**
         *  \brief  Compound addition of extents.
         *
         *  An uninitialised extent will simply be initialised to the given
         *  extent. An already initialised extent will be extended
         *  appropriately to incorporate the new extent.
         */
        _Self & operator += (const _Self & rhs_)
            {
                using std::min;
                using std::max;

                // If both extents are initialised...
                this->_min = min(this->_min, rhs_._min);
                this->_max = max(this->_max, rhs_._max);
                return *this;
            }

        /**
         *  \brief  Addition of extents.
         *
         *  An uninitialised extent will simply be initialised to the given
         *  extent. An already initialised extent will be extended
         *  appropriately to incorporate the new extent.
         */
        _Self operator + (const _Self & rhs_) const
            {
                _Self result(*this);
                result += rhs_;
                return result;
            }

        /**  Get the minimum value.  */
        const _ComponentType & min() const
            { return this->_min; }

        /**  Sets the minimum value.  */
        void min(const _ComponentType & min_)
            { this->_min = min_; }

        /**  Get the maximum value.  */
        const _ComponentType & max() const
            { return this->_max; }

        /**  Sets the maximum value.  */
        void max(const _ComponentType & max_)
            { this->_max = max_; }

        /**  Tests for inclusion.  */
        bool includes(const _ComponentType & value_) const
            {
                using std::min;
                using std::max;

                return (min(value_, this->_min) == this->_min &&
                        max(value_, this->_max) == this->_max);
            }

        /**
         *  \brief  Boolean operator.
         *
         *  This operator return the "truth" of this extent. Returns true
         *  if this extent is initialised and has a greater span than zero.
         */
        operator bool ()
            {
                using std::min;
                using std::max;

                return this->_min != this->_max && min(this->_min, this->_max) == this->_min;
            }

        /**
         *  \brief  Explicit casting.
         */
        template< typename _DestinationComponentType >
        extent< _DestinationComponentType > castTo()
            {
                return extent< _DestinationComponentType >(
                    (_DestinationComponentType) this->_min,
                    (_DestinationComponentType) this->_max);
            }

        /**
         *  \brief  Implicit casting.
         */
        template< typename _DestinationComponentType >
        operator extent< _DestinationComponentType > ()
            { return this->castTo< _DestinationComponentType >(); }

        //@}

    private:
        // Mininum and maximum values of extent
        _ComponentType _min;
        _ComponentType _max;

    }; /* class extent */

    /**  Element / Extent addition operator.  */
    template< typename _ComponentType >
    extent< _ComponentType > operator + (const _ComponentType & lhs_, const extent< _ComponentType > & rhs_)
    { return rhs_ + lhs_; }

    typedef extent< float > extent_f;
    typedef extent< vector_2f > extent_2f;
    typedef extent< vector_3f > extent_3f;
    typedef extent< vector_4f > extent_4f;

    typedef extent< double > extent_d;
    typedef extent< vector_2d > extent_2d;
    typedef extent< vector_3d > extent_3d;
    typedef extent< vector_4d > extent_4d;

    typedef extent< unsigned int > extent_ui;
    typedef extent< vector< unsigned int, 2 > > extent_2ui;
    typedef extent< vector< unsigned int, 3 > > extent_3ui;
    typedef extent< vector< unsigned int, 4 > > extent_4ui;

    typedef extent< size_t > extent_size_t;

}

#endif /* GTL_EXTENT_INCL_ */
