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

#ifndef ANNOTATION_INCL_
#define ANNOTATION_INCL_

/*****************************************************************************
 *
 * Annotation.h
 *
 * Copyright 2009 Advanced Interfaces Group
 *
 ****************************************************************************/

#include <spine/spineapi.h>
#include <spine/Area.h>
#include <spine/Capability.h>
#include <spine/TextSelection.h>
#include <set>
#include <map>
#include <vector>
#include <iostream>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/foreach.hpp>

namespace Spine {

    class AnnotationPrivate;
    class Annotation
    {
    public:
        typedef AreaSet::iterator iterator;
        typedef AreaSet::const_iterator const_iterator;

        Annotation();
        Annotation(const Annotation &rhs_);
        ~Annotation();

        bool addArea(const Area & area);
        void addCapability(CapabilityHandle capability);
        bool addExtent(TextExtentHandle extent);
        AreaSet areas() const;
        iterator begin();
        const_iterator begin() const;
        iterator begin(int page);
        std::list< CapabilityHandle > capabilities() const;
        void clearProperties();
        bool contains(int page);
        bool contains(int page, double x, double y);
        bool equalRegions(const Annotation &rhs_) const;
        iterator end();
        const_iterator end() const;
        iterator end(int page);
        TextExtentSet extents() const;
        std::string getFirstProperty(const std::string & key_) const;
        std::vector< std::string > getProperty(const std::string & key_) const;
        bool hasProperty(const std::string & key_) const;
        bool hasProperty(const std::string & key_, const std::string & value_) const;
        bool isPublic() const;
        std::multimap< std::string, std::string > properties() const;
        bool removeArea(const Area & area);
        void removeCapability(CapabilityHandle capability);
        bool removeExtent(TextExtentHandle extent);
        bool removeProperty(const std::string & key);
        bool removeProperty(const std::string & key, const std::string & value);
        bool setProperty(const std::string & key_, const std::string & value_ = std::string());
        void setPublic(bool isPublic);
        std::string text(const std::string & joiner = " ") const;

        bool operator==(const Annotation &rhs_) const;

        // Are there any capabilities registered for the given annotation
        template< class CapabilityClass >
        bool capable() const
        {
            BOOST_FOREACH(CapabilityHandle capability, capabilities()) {
                if (dynamic_cast< CapabilityClass * >(capability.get())) {
                    return true;
                }
            }
            return false;
        }

        // Get all capabilities registered for the given annotation
        template< class CapabilityClass >
        std::list< boost::shared_ptr< CapabilityClass > > capabilities() const
        {
            std::list< boost::shared_ptr< CapabilityClass > > matches;
            BOOST_FOREACH(CapabilityHandle capability, capabilities()) {
                boost::shared_ptr< CapabilityClass > casted_capability = boost::dynamic_pointer_cast< CapabilityClass, Capability >(capability);
                if (casted_capability) {
                    matches.push_back(casted_capability);
                }
            }
            return matches;
        }


    private:
        boost::scoped_ptr< AnnotationPrivate > d;

        void _recache();
    };

    typedef boost::shared_ptr<Annotation> AnnotationHandle;
    typedef boost::weak_ptr<Annotation> WeakAnnotationHandle;
    typedef std::set< AnnotationHandle > AnnotationSet;

    // Note: this shares the handle
    SpineAnnotation share_SpineAnnotation(AnnotationHandle handle_, SpineError *error_);
    AnnotationHandle SpineAnnotation_handle(SpineAnnotation ann_, SpineError *error_);

}

#endif /* ANNOTATION_INCL_ */
