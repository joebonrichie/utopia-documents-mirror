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

#ifndef LIBSPINE_TEXTSELECTION_INCL_
#define LIBSPINE_TEXTSELECTION_INCL_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/shared_array.hpp>
#include <spine/spineapi.h>
#include <spine/Area.h>
#include <spine/BoundingBox.h>
#include <spine/Cursor.h>
#include <spine/Selection.h>
#include <spine/TextIterator.h>
#include <string>

#include <list>
#include <sstream>
#include <iostream>
#include <vector>
#include <exception>
/*****************************************************************************
 *
 * TextSelection.h
 *
 * Copyright 2009 Advanced Interfaces Group
 *
 ****************************************************************************/

namespace Spine
{

    class Document;

    typedef enum
    {
        DefaultSearchOptions            = 0x0,
        IgnoreCase                      = 0x1,
        WholeWordsOnly                  = 0x2,
        RegExp                          = 0x4,

        RegularExpression               = RegExp
    } SearchOptions;

    /***************************************************************************
     *
     * TextExtent
     *
     **************************************************************************/

    class TextExtent : public std::pair< TextIterator, TextIterator >
    {
        typedef std::pair< TextIterator, TextIterator > _Base;

    public:

        class regex_exception : public std::exception {
        public:
            regex_exception(const std::string &pattern_, const std::string &error_)
                : _pattern(pattern_), _error(error_), _msg("Invalid regular expression [" +
                                                           pattern_ + "] :" + error_)
            {}

            virtual ~regex_exception() throw()
            {}

            virtual const char* what() const throw() {
                return _msg.c_str();
            }

            virtual const char *pattern() const throw() {
                return _pattern.c_str();
            }

            virtual const char *error() const throw() {
                return _error.c_str();
            }

        private:
            std::string _pattern;
            std::string _error;
            std::string _msg;
        };

#if 0
        TextExtent()
            : _Base()
        {}
#endif
        TextExtent(const TextIterator & first, const TextIterator & second)
            : _Base(first, second)
        {}
#if 0
        TextExtent(const _Base & other)
            : _Base(other)
        {}
#endif
        bool isSingular() const
        {
            return first == second;
        }

        template< class TextIteratorAdaptor >
        std::string text() const
        {
            std::string str;
            TextIteratorAdaptor from(first);
            TextIteratorAdaptor to(second);
//        std::cerr << "------ Testing first >= second ------ " << (from <= to) << std::endl;
//        std::cerr << "                                from: " << from.cursor()->repr() << std::endl;
//        std::cerr << "                                  to: " << to.cursor()->repr() << std::endl;
            while (from != to)
            {
                str += *from;
//          std::cerr << "====== " << *from << std::endl;
                ++from;

//          std::cerr << "                                from: " << from.cursor()->repr() << std::endl;
//          std::cerr << "                                  to: " << to.cursor()->repr() << std::endl;
            }
            return str;
        }

        std::string text() const
        {
            if(_cached_text.empty()) {
                _cacheText();
            }
            return _cached_text;
        }

        Spine::TextIterator iteratorFromOffset(size_t start_codepoints_) const;
        Spine::TextIterator iteratorFromOffsetUtf8(size_t start_octets_) const;

        boost::shared_ptr< TextExtent > subExtent(size_t start_codepoints_, size_t length_codepoints_) const;
        boost::shared_ptr< TextExtent > subExtentUtf8(size_t start_octets_, size_t length_octets_) const;

        AreaList areas() const;
        std::set< boost::shared_ptr< TextExtent >, ExtentCompare< TextExtent > >
            search(const std::string &regexp_, int options = DefaultSearchOptions) const;
        boost::shared_ptr< TextExtent > clone();

    private:

        void _cacheText() const;
        boost::shared_ptr< TextExtent > _cachedSubExtent(size_t start_, size_t length_,
                                                         const std::map<size_t,
                                                         TextIterator> &skiplist_) const;
        Spine::TextIterator _iteratorFromOffset(size_t start_,
                                                const std::map<size_t,
                                                TextIterator> &skiplist_) const;

        mutable std::string _cached_text;
        mutable std::map<size_t, TextIterator> _skiplist_utf8;
        mutable std::map<size_t, TextIterator> _skiplist_utf32;
    };

    typedef boost::shared_ptr< TextExtent > TextExtentHandle;
    typedef boost::weak_ptr< TextExtent > WeakTextExtentHandle;

    /***************************************************************************
     *
     * TextSelection
     *
     **************************************************************************/

    class TextSelection : public Selection< TextIterator, TextExtent >
    {
        typedef Selection< TextIterator, TextExtent > _Base;

    public:

        TextSelection()
            : _Base() {}

        template< class InputIterator >
        TextSelection(InputIterator f, InputIterator l)
            : _Base(f, l) {}

        TextSelection(const TextSelection & rhs)
            : _Base(rhs) {}

        TextSelection(const extent_handle_type & rhs)
            : _Base(rhs) {}

        TextSelection(const extent_type & rhs)
            : _Base(rhs) {}

        TextSelection(const _Base & other)
            : _Base(other) {}

        bool isSingular() const
        {
            bool ret = true;

            if (!empty())
            {
                BOOST_FOREACH(TextExtentHandle e, *this)
                {
                    if (!(ret = e->isSingular())) break;
                }
            }

            return ret;
        }

        std::string text() const
        {
            std::string str;

            bool first = true;
            BOOST_FOREACH(TextExtentHandle e, *this)
            {
                if (!first) { str += "\n"; }
                str += e->text();
                first = false;
            }

            return str;
        }

    };

    typedef TextSelection::set_type TextExtentSet;

    template< typename ValueType >
    class TextExtentMap : public std::map< boost::shared_ptr< TextExtent >, ValueType, ExtentCompare< TextExtent > >
    {
        typedef std::map< boost::shared_ptr< TextExtent >, ValueType, ExtentCompare< TextExtent > > _Base;

    public:
        TextExtentMap() : _Base() {}
        TextExtentMap(const TextExtentMap< ValueType > & rhs) : _Base(rhs) {}
        template <typename InputIterator>
        TextExtentMap(InputIterator first, InputIterator last) : _Base(first, last) {}

    };

}

bool operator < (const Spine::TextExtentHandle & lhs, const Spine::TextExtentHandle & rhs);

SpineTextExtent copy_SpineTextExtent(Spine::TextExtentHandle extent_, SpineError *error_);
Spine::TextExtentHandle SpineAnnotation_extent(SpineTextExtent extent_, SpineError *error_);

#endif /* LIBSPINE_TEXTSELECTION_INCL_ */
