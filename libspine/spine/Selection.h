/*****************************************************************************
 *  
 *   This file is part of the libspine library.
 *       Copyright (c) 2008-2017 Lost Island Labs
 *           <info@utopiadocs.com>
 *   
 *   The libspine library is free software: you can redistribute it and/or
 *   modify it under the terms of the GNU AFFERO GENERAL PUBLIC LICENSE
 *   VERSION 3 as published by the Free Software Foundation.
 *   
 *   The libspine library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero
 *   General Public License for more details.
 *   
 *   You should have received a copy of the GNU Affero General Public License
 *   along with the libspine library. If not, see
 *   <http://www.gnu.org/licenses/>
 *  
 *****************************************************************************/

#ifndef LIBSPINE_SELECTION_INCL_
#define LIBSPINE_SELECTION_INCL_

#include <boost/foreach.hpp>
#include <set>
#include <utility>

/*****************************************************************************
 *
 * Selection.h
 *
 * Copyright 2009 Advanced Interfaces Group
 *
 ****************************************************************************/

// std::pair accessors
template< typename Limit >
inline const Limit & extentFrom(const std::pair< Limit, Limit > & extent)
{
    return extent.first;
}

template< typename Limit >
inline const Limit & extentTo(const std::pair< Limit, Limit > & extent)
{
    return extent.second;
}

namespace Spine
{

    namespace detail
    {

        template< typename Selection >
        bool intersects(const typename Selection::extent_handle_type & extent,
                        const typename Selection::limit_type & limit)
        {
            return extentFrom(*extent) <= limit && limit < extentTo(*extent);
        }

        template< typename Selection >
        bool intersects(const typename Selection::extent_handle_type & extent1,
                        const typename Selection::extent_handle_type & extent2)
        {
            return extentFrom(*extent1) < extentTo(*extent2) && extentFrom(*extent2) < extentTo(*extent1);
        }

        template< typename Selection >
        bool within(const typename Selection::extent_handle_type & extent1,
                    const typename Selection::extent_handle_type & extent2)
        {
            return extentFrom(*extent2) < extentFrom(*extent1) && extentTo(*extent1) < extentTo(*extent2);
        }

        template< typename Selection >
        bool endsWithStartOf(const typename Selection::extent_handle_type & extent1,
                             const typename Selection::extent_handle_type & extent2)
        {
            return extentFrom(*extent1) < extentFrom(*extent2) && extentTo(*extent1) <= extentTo(*extent2);
        }

        template< typename Selection >
        bool startsWithEndOf(const typename Selection::extent_handle_type & extent1,
                             const typename Selection::extent_handle_type & extent2)
        {
            return extentFrom(*extent2) <= extentFrom(*extent1) && extentTo(*extent2) < extentTo(*extent1);
        }

        template< typename Selection >
        typename Selection::extent_handle_type intersectionOf(const typename Selection::extent_handle_type & extent1,
                                                              const typename Selection::extent_handle_type & extent2)
        {
            return typename Selection::extent_handle_type(
                new typename Selection::extent_type(
                    extentFrom(*extent1) < extentFrom(*extent2) ? extentFrom(*extent2) : extentFrom(*extent1),
                    extentTo(*extent2) < extentTo(*extent1) ? extentTo(*extent2) : extentTo(*extent1)
                    )
                );
        }

        template< typename Selection >
        typename Selection::extent_handle_type unionOf(const typename Selection::extent_handle_type & extent1,
                                                       const typename Selection::extent_handle_type & extent2)
        {
            return typename Selection::extent_handle_type(
                new typename Selection::extent_type(
                    extentFrom(*extent1) < extentFrom(*extent2) ? extentFrom(*extent1) : extentFrom(*extent2),
                    extentTo(*extent2) < extentTo(*extent1) ? extentTo(*extent1) : extentTo(*extent2)
                    )
                );
        }

    }


    template< typename Extent >
    class ExtentCompare
    {
    public:
        bool operator () (const boost::shared_ptr< Extent > & lhs, const boost::shared_ptr< Extent > & rhs) const
        {
            if (lhs->first == rhs->first) {
                return rhs->second < lhs->second;
            } else {
                return lhs->first < rhs->first;
            }
            //return *lhs < *rhs;
        }
    };


    /***************************************************************************
     *
     * Selection
     *
     **************************************************************************/

    template< typename Limit, typename Extent = std::pair< Limit, Limit > >
    class Selection : public std::set< boost::shared_ptr< Extent >, ExtentCompare< Extent > >
    {
        typedef boost::shared_ptr< Extent > ExtentHandle;
        typedef std::set< ExtentHandle, ExtentCompare< Extent > > _Base;

    protected:
        ExtentHandle wrap(const Limit & from, const Limit & to)
            {
                return ExtentHandle(new Extent(from, to));
            }

    public:
        typedef Limit limit_type;
        typedef Extent extent_type;
        typedef boost::shared_ptr< Extent > extent_handle_type;
        typedef std::set< ExtentHandle, ExtentCompare< Extent > > set_type;

        Selection()
            : _Base() {}

        template< class InputIterator >
        Selection(InputIterator f, InputIterator l)
            : _Base(f, l) {}

        Selection(const Selection< Limit, Extent > & rhs)
            : _Base(rhs) {}

        Selection(const extent_handle_type & rhs)
            : _Base() { this->insert(extent_handle_type(new extent_type(*rhs))); }

        Selection(const extent_type & rhs)
            : _Base() { this->insert(extent_handle_type(new extent_type(rhs))); }

        Selection(const _Base & other)
            : _Base(other) {}


        bool contains(const Limit & rhs) const
            {
                // FIXME
                return false;
            }


        /********************************************************************************************/
        /** Intersection ****************************************************************************/
        /********************************************************************************************/

        Selection< Limit, Extent > & intersect(const Selection< Limit, Extent > & rhs)
            {
                return *this &= rhs;
            }

        Selection< Limit, Extent > & intersect(const extent_handle_type & rhs)
            {
                return *this &= rhs;
            }

        inline Selection< Limit, Extent > & intersect(const Limit & from, const Limit & to)
            {
                return *this &= wrap(from, to);
            }

        Selection< Limit, Extent > intersected(const Selection< Limit, Extent > & rhs) const
            {
                Selection< Limit, Extent > intersection;
                BOOST_FOREACH(const extent_handle_type & extent, rhs) { intersection += intersected(extent); }
                return intersection;
            }

        Selection< Limit, Extent > intersected(const extent_handle_type & rhs) const
            {
                Selection< Limit, Extent > intersection;
                BOOST_FOREACH(const extent_handle_type & extent, *this)
                {
                    if (::Spine::detail::intersects< Selection< Limit, Extent > >(extent, rhs))
                        intersection += ::Spine::detail::intersectionOf< Selection< Limit, Extent > >(extent, rhs);
                }
                return intersection;
            }

        inline Selection< Limit, Extent > intersected(const Limit & from, const Limit & to) const
            {
                return intersected(wrap(from, to));
            }

        bool intersects(const Selection< Limit, Extent > & rhs) const
            {
                BOOST_FOREACH(const extent_handle_type & extent, rhs) { if (intersects(extent)) return true; }
                return false;
            }

        bool intersects(const extent_handle_type & rhs) const
            {
                BOOST_FOREACH(const extent_handle_type & extent, *this)
                {
                    if (::Spine::detail::intersects< Selection< Limit, Extent > >(extent, rhs))
                        return true;
                }
                return false;
            }

        inline bool intersects(const Limit & from, const Limit & to) const
            {
                return intersects(wrap(from, to));
            }


        /********************************************************************************************/
        /** Union ***********************************************************************************/
        /********************************************************************************************/

        Selection< Limit, Extent > & merge(const Selection< Limit, Extent > & rhs)
            {
                return *this += rhs;
            }

        Selection< Limit, Extent > & merge(const extent_handle_type & rhs)
            {
                return *this += rhs;
            }

        Selection< Limit, Extent > & merge(const Limit & from, const Limit & to)
            {
                return *this += wrap(from, to);
            }

        Selection< Limit, Extent > merged(const Selection< Limit, Extent > & rhs) const
            {
                return Selection< Limit, Extent >(*this) += rhs;
            }

        Selection< Limit, Extent > merged(const extent_handle_type & rhs) const
            {
                return Selection< Limit, Extent >(*this) += rhs;
            }

        inline Selection< Limit, Extent > merged(const Limit & from, const Limit & to) const
            {
                return united(wrap(from, to));
            }


        /********************************************************************************************/
        /** Difference ******************************************************************************/
        /********************************************************************************************/

        Selection< Limit, Extent > & subtract(const Selection< Limit, Extent > & rhs)
            {
                return *this -= rhs;
            }

        Selection< Limit, Extent > & subtract(const extent_handle_type & rhs)
            {
                return *this -= rhs;
            }

        Selection< Limit, Extent > & subtract(const Limit & from, const Limit & to)
            {
                return *this -= wrap(from, to);
            }

        Selection< Limit, Extent > subtracted(const Selection< Limit, Extent > & rhs) const
            {
                return Selection< Limit, Extent >(*this) -= rhs;
            }

        Selection< Limit, Extent > subtracted(const extent_handle_type & rhs) const
            {
                return Selection< Limit, Extent >(*this) -= rhs;
            }

        inline Selection< Limit, Extent > subtracted(const Limit & from, const Limit & to) const
            {
                return subtracted(wrap(from, to));
            }


        /********************************************************************************************/
        /** XOR *************************************************************************************/
        /********************************************************************************************/

        Selection< Limit, Extent > & punch(const Selection< Limit, Extent > & rhs)
            {
                return *this ^= rhs;
            }

        Selection< Limit, Extent > & punch(const extent_handle_type & rhs)
            {
                return *this ^= rhs;
            }

        Selection< Limit, Extent > & punch(const Limit & from, const Limit & to)
            {
                return *this ^= wrap(from, to);
            }

        Selection< Limit, Extent > punched(const Selection< Limit, Extent > & rhs) const
            {
                return Selection< Limit, Extent >(*this) ^= rhs;
            }

        Selection< Limit, Extent > punched(const extent_handle_type & rhs) const
            {
                return Selection< Limit, Extent >(*this) ^= rhs;
            }

        inline Selection< Limit, Extent > punched(const Limit & from, const Limit & to) const
            {
                return punched(wrap(from, to));
            }


        /********************************************************************************************/
        /** Normalisation ***************************************************************************/
        /********************************************************************************************/

        bool isNormalised() const
            {
                typename _Base::const_iterator iter_prev;
                typename _Base::const_iterator iter;
                typename _Base::const_iterator iter_begin;
                typename _Base::const_iterator iter_end;
                do
                {
                    iter_begin = iter = _Base::begin();
                    iter_end = _Base::end();
                    for (; iter != iter_end; ++iter)
                    {
                        if (iter != iter_begin)
                        {
                            if (extentFrom(**iter_prev) == extentFrom(**iter))
                            {
                                return false;
                            }
                        }
                        iter_prev = iter;
                    }
                } while (iter != iter_end);

                return true;
            }

        Selection< Limit, Extent > & normalise()
            {
                bool hit;
                typename _Base::const_iterator iter_prev;
                typename _Base::const_iterator iter;
                typename _Base::const_iterator iter_begin;
                typename _Base::const_iterator iter_end;
                do
                {
                    hit = false;
                    iter_begin = iter = _Base::begin();
                    iter_end = _Base::end();
                    for (; iter != iter_end; ++iter)
                    {
                        if ((*iter)->isSingular())
                        {
                            _Base::erase(iter);
                            hit = true;
                            break;
                        }
                        else if (iter != iter_begin)
                        {
                            if (extentFrom(**iter_prev) == extentTo(**iter))
                            {
                                ExtentHandle merged(new extent_type(extentFrom(**iter_prev), extentTo(**iter)));
                                ExtentHandle prev(*iter_prev);
                                _Base::erase(iter);
                                _Base::erase(prev);
                                _Base::insert(merged);
                                hit = true;
                                break;
                            }
                        }
                        iter_prev = iter;
                    }
                } while (hit || iter != iter_end);

                return *this;
            }

        Selection< Limit, Extent > normalised() const
            { return Selection< Limit, Extent >(*this).normalise(); }


        /********************************************************************************************/
        /** Operators *******************************************************************************/
        /********************************************************************************************/

        Selection< Limit, Extent > & operator += (const Selection< Limit, Extent > & rhs)
            {
                return operator|=(rhs);
            }

        Selection< Limit, Extent > & operator += (const extent_handle_type & rhs)
            {
                return operator|=(rhs);
            }

        Selection< Limit, Extent > & operator |= (const Selection< Limit, Extent > & rhs)
            {
                BOOST_FOREACH(const extent_handle_type & extent, rhs) { *this += extent; }
                return *this;
            }

        Selection< Limit, Extent > & operator |= (const extent_handle_type & rhs)
            {
                ExtentHandle incoming(new extent_type(*rhs));

                bool hit;
                typename _Base::const_iterator iter;
                typename _Base::const_iterator iter_end;
                do
                {
                    hit = false;
                    iter = _Base::begin();
                    iter_end = _Base::end();
                    for (; iter != iter_end; ++iter)
                    {
                        if (::Spine::detail::intersects< Selection< Limit, Extent > >(*iter, incoming))
                        {
                            incoming = ::Spine::detail::unionOf< Selection< Limit, Extent > >(*iter, incoming);
                            _Base::erase(iter);
                            hit = true;
                            break;
                        }
                    }
                } while (hit || iter != iter_end);

                _Base::insert(incoming);

                return *this;
            }

        Selection< Limit, Extent > & operator -= (const Selection< Limit, Extent > & rhs)
            {
                BOOST_FOREACH(const extent_handle_type & extent, rhs) { *this -= extent; }
                return *this;
            }

        Selection< Limit, Extent > & operator -= (const extent_handle_type & rhs)
            {
                bool hit;
                typename _Base::iterator iter;
                typename _Base::iterator iter_end;
                do
                {
                    hit = false;
                    iter = _Base::begin();
                    iter_end = _Base::end();
                    for (; iter != iter_end; ++iter)
                    {
                        if (::Spine::detail::intersects< Selection< Limit, Extent > >(*iter, rhs))
                        {
                            if (::Spine::detail::within< Selection< Limit, Extent > >(*iter, rhs))
                            {
                                _Base::erase(iter);
                                hit = true;
                                break;
                            }
                            if (::Spine::detail::within< Selection< Limit, Extent > >(rhs, *iter))
                            {
                                ExtentHandle left(new extent_type(extentFrom(**iter), extentFrom(*rhs)));
                                ExtentHandle right(new extent_type(extentTo(*rhs), extentTo(**iter)));
                                _Base::erase(iter);
                                _Base::insert(left);
                                _Base::insert(right);
                                hit = true;
                                break;
                            }
                            if (::Spine::detail::startsWithEndOf< Selection< Limit, Extent > >(*iter, rhs))
                            {
                                ExtentHandle right(new extent_type(extentTo(*rhs), extentTo(**iter)));
                                _Base::erase(iter);
                                _Base::insert(right);
                                hit = true;
                                break;
                            }
                            if (::Spine::detail::endsWithStartOf< Selection< Limit, Extent > >(*iter, rhs))
                            {
                                ExtentHandle left(new extent_type(extentFrom(**iter), extentFrom(*rhs)));
                                _Base::erase(iter);
                                _Base::insert(left);
                                hit = true;
                                break;
                            }
                        }
                    }
                } while (hit || iter != iter_end);

                return *this;
            }

        Selection< Limit, Extent > & operator &= (const Selection< Limit, Extent > & rhs)
            {
                return *this = intersected(*this, rhs);
            }

        Selection< Limit, Extent > & operator &= (const extent_handle_type & rhs)
            {
                return *this = intersected(*this, rhs);
            }

        Selection< Limit, Extent > & operator ^= (const Selection< Limit, Extent > & rhs)
            {
                Selection< Limit, Extent > overlap = intersected(rhs);
                *this += rhs;
                *this -= overlap;
                return *this;
            }

        Selection< Limit, Extent > & operator ^= (const extent_handle_type & rhs)
            {
                Selection< Limit, Extent > overlap = intersected(rhs);
                *this += rhs;
                *this -= overlap;
                return *this;
            }

    };

    template< typename Limit, typename Extent >
    Selection< Limit, Extent > operator + (const Selection< Limit, Extent > & lhs,
                                           const Selection< Limit, Extent > & rhs)
    {
        return lhs | rhs;
    }

    template< typename Limit, typename Extent >
    Selection< Limit, Extent > operator + (const Selection< Limit, Extent > & lhs,
                                           const typename Selection< Limit, Extent >::extent_handle_type & rhs)
    {
        return lhs | rhs;
    }

    template< typename Limit, typename Extent >
    Selection< Limit, Extent > operator + (const typename Selection< Limit, Extent >::extent_handle_type & lhs,
                                           const Selection< Limit, Extent > & rhs)
    {
        return lhs | rhs;
    }

    template< typename Limit, typename Extent >
    Selection< Limit, Extent > operator | (const Selection< Limit, Extent > & lhs,
                                           const Selection< Limit, Extent > & rhs)
    {
        return Selection< Limit, Extent >(lhs) |= rhs;
    }

    template< typename Limit, typename Extent >
    Selection< Limit, Extent > operator | (const Selection< Limit, Extent > & lhs,
                                           const typename Selection< Limit, Extent >::extent_handle_type & rhs)
    {
        return Selection< Limit, Extent >(lhs) |= rhs;
    }

    template< typename Limit, typename Extent >
    Selection< Limit, Extent > operator | (const typename Selection< Limit, Extent >::extent_handle_type & lhs,
                                           const Selection< Limit, Extent > & rhs)
    {
        return rhs | lhs;
    }

    template< typename Limit, typename Extent >
    Selection< Limit, Extent > operator - (const Selection< Limit, Extent > & lhs,
                                           const Selection< Limit, Extent > & rhs)
    {
        return Selection< Limit, Extent >(lhs) -= rhs;
    }

    template< typename Limit, typename Extent >
    Selection< Limit, Extent > operator - (const Selection< Limit, Extent > & lhs,
                                           const typename Selection< Limit, Extent >::extent_handle_type & rhs)
    {
        return Selection< Limit, Extent >(lhs) -= rhs;
    }

    template< typename Limit, typename Extent >
    Selection< Limit, Extent > operator & (const Selection< Limit, Extent > & lhs,
                                           const Selection< Limit, Extent > & rhs)
    {
        return Selection< Limit, Extent >(lhs) &= rhs;
    }

    template< typename Limit, typename Extent >
    Selection< Limit, Extent > operator & (const Selection< Limit, Extent > & lhs,
                                           const typename Selection< Limit, Extent >::extent_handle_type & rhs)
    {
        return Selection< Limit, Extent >(lhs) &= rhs;
    }

    template< typename Limit, typename Extent >
    Selection< Limit, Extent > operator & (const typename Selection< Limit, Extent >::extent_handle_type & lhs,
                                           const Selection< Limit, Extent > & rhs)
    {
        return rhs & lhs;
    }

    template< typename Limit, typename Extent >
    Selection< Limit, Extent > operator ^ (const Selection< Limit, Extent > & lhs,
                                           const Selection< Limit, Extent > & rhs)
    {
        return Selection< Limit, Extent >(lhs) ^= rhs;
    }

    template< typename Limit, typename Extent >
    Selection< Limit, Extent > operator ^ (const Selection< Limit, Extent > & lhs,
                                           const typename Selection< Limit, Extent >::extent_handle_type & rhs)
    {
        return Selection< Limit, Extent >(lhs) ^= rhs;
    }

    template< typename Limit, typename Extent >
    Selection< Limit, Extent > operator ^ (const typename Selection< Limit, Extent >::extent_handle_type & lhs,
                                           const Selection< Limit, Extent > & rhs)
    {
        return rhs ^ lhs;
    }

}

#endif /* LIBSPINE_SELECTION_INCL_ */
