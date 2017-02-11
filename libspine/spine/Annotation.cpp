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

/*****************************************************************************
 *
 * Annotation.cpp
 *
 * Copyright 2009 Advanced Interfaces Group
 *
 ****************************************************************************/

#include <spine/spineapi.h>

#include <spine/Annotation.h>
#include <spine/Page.h>
#include <spine/Region.h>
#include <spine/Block.h>
#include <spine/Line.h>
#include <spine/Word.h>
#include <spine/Character.h>
#include <spine/Image.h>
#include <spine/BoundingBox.h>
#include <spine/Area.h>
#include <spine/Cursor.h>
#include <spine/Document.h>
#include <spine/TextSelection.h>
#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include "spineapi_internal.h"

namespace Spine
{

    typedef std::multimap< std::string, std::string >::iterator property_iterator;
    typedef std::multimap< std::string, std::string >::const_iterator property_const_iterator;

    class AnnotationPrivate
    {
    public:
        AnnotationPrivate()
            : isPublic(false)
        {}

        std::multimap< std::string, std::string > properties;

        struct
        {
            TextExtentSet extents;
            std::multiset< Area > areas;
        } text;

        struct
        {
            AreaSet areas;
        } area;

        AreaSet uniqueAreas;
        std::set< int > uniquePages;

        boost::recursive_mutex mutex;
        bool isPublic;

        std::list< CapabilityHandle > capabilities;

        bool equalRegions(const AnnotationPrivate &rhs_) const
        {
            return area.areas == rhs_.area.areas &&
                text.areas == rhs_.text.areas;
        }

        bool operator==(const AnnotationPrivate &rhs_) const
        {
            return properties == rhs_.properties &&
                equalRegions(rhs_);
        }

        void recache()
        {
            boost::lock_guard< boost::recursive_mutex > guard(mutex);
            uniqueAreas = AreaSet(text.areas.begin(), text.areas.end());
            uniqueAreas.insert(area.areas.begin(), area.areas.end());
            uniquePages.clear();
            BOOST_FOREACH(const Area & area, uniqueAreas)
            {
                uniquePages.insert(area.page);
            }
        }
    };

    Annotation::Annotation()
        : d(new AnnotationPrivate)
    {}

    Annotation::Annotation(const Annotation &rhs_)
        : d(new AnnotationPrivate)
    {
        boost::lock_guard< boost::recursive_mutex > guard(rhs_.d->mutex);

        d->properties = rhs_.d->properties;
        d->area.areas = rhs_.d->area.areas;
        BOOST_FOREACH(Spine::TextExtentHandle extent, rhs_.d->text.extents)
        {
            d->text.extents.insert(Spine::TextExtentHandle(new Spine::TextExtent(*extent)));
        }
        d->text.areas = rhs_.d->text.areas;
        d->uniqueAreas = rhs_.d->uniqueAreas;
        d->uniquePages = rhs_.d->uniquePages;
    }

    Annotation::~Annotation()
    {}

    bool Annotation::addArea(const Area & area)
    {
        boost::lock_guard< boost::recursive_mutex > guard(d->mutex);
        bool exists = d->area.areas.count(area);
        if (!exists)
        {
            d->area.areas.insert(area);
        }
        d->recache();
        return !exists;
    }

    AreaSet Annotation::areas() const
    {
        boost::lock_guard< boost::recursive_mutex > guard(d->mutex);
        return d->area.areas;
    }

    void Annotation::addCapability(CapabilityHandle capability)
    {
        if (capability) {
            boost::lock_guard< boost::recursive_mutex > guard(d->mutex);
            bool found = false;
            BOOST_FOREACH(CapabilityHandle next, d->capabilities) {
                if (next == capability) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                d->capabilities.push_back(capability);
            }
        }
    }

    bool Annotation::addExtent(TextExtentHandle extent)
    {
        if (extent)
        {
            boost::lock_guard< boost::recursive_mutex > guard(d->mutex);
            bool exists = d->text.extents.find(extent) != d->text.extents.end();
            if (!exists)
            {
                d->text.extents.insert(extent);
                std::list< Area > boxes(extent->areas());
                d->text.areas.insert(boxes.begin(), boxes.end());
            }
            d->recache();
            return !exists;
        }
        else
        {
            return false;
        }
    }

    Annotation::iterator Annotation::begin()
    {
        boost::lock_guard< boost::recursive_mutex > guard(d->mutex);
        return d->uniqueAreas.begin();
    }

    Annotation::const_iterator Annotation::begin() const
    {
        boost::lock_guard< boost::recursive_mutex > guard(d->mutex);
        return d->uniqueAreas.begin();
    }

    Annotation::iterator Annotation::begin(int page)
    {
        boost::lock_guard< boost::recursive_mutex > guard(d->mutex);
        return d->uniqueAreas.lower_bound(Area(page));
    }

    std::list< CapabilityHandle > Annotation::capabilities() const
    {
        boost::lock_guard< boost::recursive_mutex > guard(d->mutex);
        return d->capabilities;
    }

    void Annotation::clearProperties()
    {
        boost::lock_guard< boost::recursive_mutex > guard(d->mutex);
        d->properties.clear();
    }

    bool Annotation::contains(int page)
    {
        boost::lock_guard< boost::recursive_mutex > guard(d->mutex);
        return d->uniquePages.find(page) != d->uniquePages.end();
    }

    bool Annotation::contains(int page, double x, double y)
    {
        boost::lock_guard< boost::recursive_mutex > guard(d->mutex);
        if (d->uniquePages.find(page) != d->uniquePages.end())
        {
            iterator i(begin(page));
            iterator i_end(end(page));
            for (; i != i_end; ++i)
            {
                if (i->boundingBox.contains(x, y))
                {
                    return true;
                }
            }
        }
        iterator i(d->area.areas.begin());
        iterator i_end(d->area.areas.end());
        for (; i != i_end; ++i)
        {
            if (i->page == page && i->boundingBox.contains(x, y))
            {
                return true;
            }
        }
        return false;
    }

    Annotation::iterator Annotation::end()
    {
        boost::lock_guard< boost::recursive_mutex > guard(d->mutex);
        return d->uniqueAreas.end();
    }

    Annotation::const_iterator Annotation::end() const
    {
        boost::lock_guard< boost::recursive_mutex > guard(d->mutex);
        return d->uniqueAreas.end();
    }

    Annotation::iterator Annotation::end(int page)
    {
        boost::lock_guard< boost::recursive_mutex > guard(d->mutex);
        return d->uniqueAreas.lower_bound(Area(page+1));
    }

    TextExtentSet Annotation::extents () const
    {
        boost::lock_guard< boost::recursive_mutex > guard(d->mutex);
        return d->text.extents;
    }

    std::string Annotation::getFirstProperty(const std::string & key) const
    {
        boost::lock_guard< boost::recursive_mutex > guard(d->mutex);
        property_const_iterator found = d->properties.find(key);
        if (found != d->properties.end()) {
            return found->second;
        } else {
            return "";
        }
    }

    std::vector< std::string > Annotation::getProperty(const std::string & key) const
    {
        boost::lock_guard< boost::recursive_mutex > guard(d->mutex);
        std::vector< std::string > properties;
        std::pair< property_const_iterator, property_const_iterator > bounds(d->properties.equal_range(key));
        for (property_const_iterator i = bounds.first; i != bounds.second; ++i) {
            properties.push_back(i->second);
        }
        return properties;
    }

    bool Annotation::hasProperty(const std::string & key) const
    {
        boost::lock_guard< boost::recursive_mutex > guard(d->mutex);
        return d->properties.find(key) != d->properties.end();
    }

    bool Annotation::hasProperty(const std::string & key_, const std::string & value_) const
    {
        boost::lock_guard< boost::recursive_mutex > guard(d->mutex);
        bool found = false;
        std::pair< property_const_iterator, property_const_iterator > range(d->properties.equal_range(key_));
        while (!found && range.first != range.second) {
            found = found || (range.first->second == value_);
            ++range.first;
        }
        return found;
    }

    bool Annotation::isPublic() const
    {
        boost::lock_guard< boost::recursive_mutex > guard(d->mutex);
        return d->isPublic;
    }

    std::multimap< std::string, std::string > Annotation::properties() const
    {
        boost::lock_guard< boost::recursive_mutex > guard(d->mutex);
        return d->properties;
    }

    bool Annotation::removeArea(const Area & area)
    {
        boost::lock_guard< boost::recursive_mutex > guard(d->mutex);
        bool ret = d->area.areas.erase(area) > 0;
        d->recache();
        return ret;
    }

    void Annotation::removeCapability(CapabilityHandle capability)
    {
        if (capability) {
            boost::lock_guard< boost::recursive_mutex > guard(d->mutex);
            d->capabilities.remove(capability);
        }
    }

    bool Annotation::removeExtent(TextExtentHandle extent)
    {
        if (extent)
        {
            boost::lock_guard< boost::recursive_mutex > guard(d->mutex);
            bool exists = d->text.extents.find(extent) != d->text.extents.end();
            if (exists)
            {
                d->text.extents.erase(extent);
                BOOST_FOREACH(const Area & area, extent->areas())
                {
                    std::multiset< Area >::const_iterator found = d->text.areas.find(area);
                    if (found != d->text.areas.end())
                    {
                        d->text.areas.erase(found);
                    }
                }
            }
            d->recache();
            return exists;
        }
        else
        {
            return false;
        }
    }

    bool Annotation::removeProperty(const std::string & key)
    {
        boost::lock_guard< boost::recursive_mutex > guard(d->mutex);
        return d->properties.erase(key) > 0;
    }

    bool Annotation::removeProperty(const std::string & key, const std::string & value)
    {
        boost::lock_guard< boost::recursive_mutex > guard(d->mutex);
        if (value.empty())
        {
            return d->properties.erase(key) > 0;
        }
        else
        {
            std::pair< property_iterator, property_iterator > bounds(d->properties.equal_range(key));
            for (property_iterator i = bounds.first; i != bounds.second; ++i)
            {
                if (i->second == value)
                {
                    d->properties.erase(i);
                    return true;
                }
            }
            return false;
        }
    }

    bool Annotation::setProperty(const std::string & key, const std::string & value)
    {
        boost::lock_guard< boost::recursive_mutex > guard(d->mutex);
        bool result(false);
        if (!value.empty())
        {
            d->properties.insert(std::make_pair(key, value));
        }
        return result;
    }

    void Annotation::setPublic(bool isPublic)
    {
        boost::lock_guard< boost::recursive_mutex > guard(d->mutex);
        d->isPublic = isPublic;
    }

    std::string Annotation::text(const std::string & joiner) const
    {
        boost::lock_guard< boost::recursive_mutex > guard(d->mutex);
        std::string str;
        bool first = true;
        BOOST_FOREACH(const TextExtentHandle & extent, d->text.extents)
        {
            if (first)
            {
                first = false;
            }
            else
            {
                str += joiner;
            }

            str += extent->text();
        }
        return str;
    }

    bool Annotation::operator==(const Annotation &rhs_) const
    {
        return *d==*(rhs_.d);
    }

    bool Annotation::equalRegions(const Annotation &rhs_) const
    {
        return d->equalRegions(*(rhs_.d));
    }

    // Note: this shares the handle
    SpineAnnotation share_SpineAnnotation(AnnotationHandle handle_, SpineError *error_)
    {
        SpineAnnotation result=new SpineAnnotationImpl;
        result->_handle=handle_;
        return result;
    }

    AnnotationHandle SpineAnnotation_handle(SpineAnnotation ann_, SpineError *error_)
    {
        return ann_->_handle;
    }

}
