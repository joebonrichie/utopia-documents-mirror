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

#ifndef AREA_INCL_
#define AREA_INCL_

/*****************************************************************************
 *
 * Area.h
 *
 * Copyright 2009 Advanced Interfaces Group
 *
 ****************************************************************************/

#include <spine/BoundingBox.h>

#include <map>
#include <set>
#include <list>

namespace Spine {

    struct Area {
        Area(int page_ = -1, int orientation_ = 0, const BoundingBox &bb_ = BoundingBox(0, 0, 0 ,0))
            : page(page_), orientation(orientation_), boundingBox(bb_)
            { }

        bool operator < (const Area & other) const
            {
                return page < other.page ||
                    (page == other.page && orientation < other.orientation) ||
                    (page == other.page && orientation == other.orientation && boundingBox < other.boundingBox);
                // Arbitrary ordering for use in data structures such as sets (not intended to be page order)
            }

        bool operator == (const Area & other) const
            {
                return page == other.page && boundingBox == other.boundingBox && orientation == other.orientation;
            }

        int page;
        int orientation;
        BoundingBox boundingBox;
    };

    typedef std::set< Area > AreaSet;
    typedef std::list< Area > AreaList;

    AreaList compile(const AreaList & areas);

    template< typename ValueType >
    class AreaMap : public std::map< Area, ValueType >
    {
        typedef std::map< Area, ValueType > _Base;

    public:
        AreaMap() : _Base() {}
        AreaMap(const AreaMap< ValueType > & rhs) : _Base(rhs) {}
        template <typename InputIterator>
        AreaMap(InputIterator first, InputIterator last) : _Base(first, last) {}

    };

    inline std::ostream & operator << (std::ostream & os, const Area & pb)
    {
        os << "Area(" << pb.page << "/" << pb.orientation << "/" << pb.boundingBox << ")";
        return os;
    }

}

#endif /* AREA_INCL_ */
