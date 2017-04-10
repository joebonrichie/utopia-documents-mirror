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

#include <spine/Cursor.h>
#include <spine/Document.h>
#include <spine/TextSelection.h>
#include <spine/Line.h>
#include <spine/Page.h>
#include <spine/Word.h>
#include <spine/Character.h>
#include <pcre.h>
#include <pcrecpp.h>
#include <string>
#include <utf8/unicode.h>
#include <algorithm>

using namespace std;
using namespace utf8;
using namespace pcrecpp;

namespace Spine
{

    void TextExtent::_cacheText() const
    {
        // append utf8 representation of each character in turn onto
        // cached text string. An entry is inserted into the skiplist
        // every 100 characters or if the utf8 representation of the
        // character > 1 byte

        _cached_text.clear();
        _skiplist_utf8.clear();
        _skiplist_utf32.clear();

        TextIterator chr(first);
        back_insert_iterator<string> ins(back_inserter(_cached_text));

        size_t offset_utf8(0); // count of utf8 characters into this cache
        size_t offset_utf32(0); // count of utf32 characters into this cache

        while (chr < second) {

            // insert skip list entry every 100 chars
            if(offset_utf32 % 100 == 0) {
                _skiplist_utf8.insert(make_pair(offset_utf8, chr));
                _skiplist_utf32.insert(make_pair(offset_utf32, chr));
            }

            // append utf8 string representing current char and advance iterator
            append(*chr, ins);
            ++chr;

            // insert utf8 skip list entry for offset_utf8 following character if
            // utf32 char decomposed to more than 1 utf8 char
            if(_cached_text.length() > offset_utf8+1) {
                _skiplist_utf8.insert(make_pair(_cached_text.length(), chr));
            }
            offset_utf8=_cached_text.length();
            ++offset_utf32;
        }
        // ensure final skip list entry for end of sequence
        _skiplist_utf8.insert(make_pair(offset_utf8, chr));
        _skiplist_utf32.insert(make_pair(offset_utf32, chr));
    }

    AreaList TextExtent::areas() const
    {
        AreaList areas;

        // Set sentinels
        CursorHandle start = first.cursor()->clone();
        CursorHandle end = second.cursor()->clone();

        // Iterate over lines
        CursorHandle line = start->clone();
        while (line->line() && *line <= *end)
        {
            // If this is the first or last line (or both)
            if (line->line() == start->line() || line->line() == end->line())
            {
                // Iterate over words
                CursorHandle word = line->clone();
                BoundingBox prevWordRect;
                while (word->word() && *word <= *end)
                {
                    // Get bounds of word
                    BoundingBox bb = word->word()->boundingBox();
                    BoundingBox wordRect(bb.x1, bb.y1, bb.x2, bb.y2);

                    // If this is the first or last word (or both)
                    if (word->word() == start->word() || word->word() == end->word())
                    {
                        // Iterate over characters
                        CursorHandle character = word->clone();
                        while (*character < *end)
                        {
                            if (const Character * ch = character->character())
                            {
                                // Get bounds of character
                                BoundingBox bb = ch->boundingBox();
                                BoundingBox characterRect(bb.x1, bb.y1, bb.x2, bb.y2);

                                if (prevWordRect.isValid() && *character == *word)
                                {
                                    characterRect.x1 = prevWordRect.x2;
                                }

                                if (!areas.empty() && areas.back().boundingBox.x2>=characterRect.x1
                                    && areas.back().boundingBox.y1==characterRect.y1
                                    && areas.back().boundingBox.y2==characterRect.y2)
                                {
                                    characterRect.x1=areas.back().boundingBox.x1;
                                    areas.pop_back();
                                }
                                int pageNumber = character->page()->pageNumber();
                                int rotation = character->word()->rotation();
                                areas.push_back(Area(pageNumber, rotation, characterRect));

                                // Move on to the next character in the selection
                                character->nextCharacter();
                            }
                            else
                            {
                                // Deal with extraneous spaces
                                CursorHandle nextWord(character->clone()); nextWord->nextWord();
                                if (word->word()->spaceAfter() && nextWord->word())
                                {
                                    BoundingBox pre_bb = word->word()->boundingBox();
                                    BoundingBox post_bb = nextWord->word()->boundingBox();
                                    BoundingBox spaceRect(pre_bb.x2, pre_bb.y1, post_bb.x1, pre_bb.y2);

                                    if (!areas.empty() && areas.back().boundingBox.x2>=spaceRect.x1
                                        && areas.back().boundingBox.y1==spaceRect.y1
                                        && areas.back().boundingBox.y2==spaceRect.y2)
                                    {
                                        spaceRect.x1=areas.back().boundingBox.x1;
                                        areas.pop_back();
                                    }
                                    int pageNumber = word->page()->pageNumber();
                                    int rotation = word->word()->rotation();
                                    areas.push_back(Area(pageNumber, rotation, spaceRect));
                                }
                                break;
                            }
                        }
                    }
                    else
                    {
                        // Otherwise add the whole word
                        // Deal with extraneous spaces
                        CursorHandle nextWord(word->clone()); nextWord->nextWord();
                        if (word->word()->spaceAfter() && nextWord->word())
                        {
                            wordRect.x2 = nextWord->word()->boundingBox().x1;
                        }

                        if (!areas.empty() && areas.back().boundingBox.x2>=wordRect.x1
                            && areas.back().boundingBox.y1==wordRect.y1
                            && areas.back().boundingBox.y2==wordRect.y2)
                        {
                            wordRect.x1=areas.back().boundingBox.x1;
                            areas.pop_back();
                        }
                        int pageNumber = word->page()->pageNumber();
                        int rotation = word->word()->rotation();
                        areas.push_back(Area(pageNumber, rotation, wordRect));
                    }

                    // Move on to the next word in the selection
                    prevWordRect = wordRect;
                    word->nextWord();
                }
            }
            else
            {
                int pageNumber = line->page()->pageNumber();
                int rotation = line->word()->rotation();
                areas.push_back(Area(pageNumber, rotation, line->line()->boundingBox()));
            }

            // Move on to the next line in the selection
            line->nextLine(WithinDocument);
        }

        return areas;
    }

    Spine::TextExtentSet TextExtent::search(const string &regexp_, int options) const
    {
        Spine::TextExtentSet matches;

        //std::cerr << "=== SEARCHING ===" << std::endl;
        if (!regexp_.empty()) {
            //std::cerr << "   orig regex: " << regexp_ << std::endl;

            // initialise regex string
            string regex;
            if(options & RegExp) {
                regex=regexp_;
            } else {
                regex=RE::QuoteMeta(regexp_);
            }

            //std::cerr << "escaped regex: " << regex << std::endl;

            if(options & WholeWordsOnly) {
                // this was converted from previous boost using code but
                // doesn't really make sense for regexs since the regex
                // may match a non word pattern such as a particular type
                // of punctuation
                regex = "\\b" + regex + "\\b";
            }

            //std::cerr << "  final regex: " << regex << std::endl;

            // set options for regex
            int opt = PCRE_UTF8;
            if(options & IgnoreCase) {
                opt |= PCRE_CASELESS;
            }

            if (!regex.empty()) {
                //std::cerr << "REGEX SEARCH " << regex << std::endl;

                // Compile the regular expression
                const char * errptr = 0;
                int erroffset = 0;
                pcre * re = pcre_compile(regex.c_str(), opt, &errptr, &erroffset, NULL);

                // check regex was created OK
                if(!re) {
                    throw TextExtent::regex_exception(regex, string(errptr));
                }

                // cache text if not already
                if(_cached_text.empty()) {
                    _cacheText();
                }

                // Find number of sub-string matches
                int substring_count = 0;
                pcre_fullinfo(re, NULL, PCRE_INFO_CAPTURECOUNT, &substring_count);

                //std::cerr << "    regex info: " << substring_count << std::endl;

                // Set up output variables and dynamic offset
                int ovector_length = (substring_count + 1) * 3;
                int ovector[ovector_length];
                int offset = 0;

                // Continue searching until complete
                while (true) {
                    int rc = pcre_exec(re,                      /* the compiled pattern */
                                   NULL,                    /* no extra data - we didn't study the pattern */
                                   _cached_text.c_str(),    /* the subject string */
                                   _cached_text.length(),   /* the length of the subject in bytes */
                                   offset,                  /* offset in the subject */
                                   0,                       /* default options */
                                   ovector,                 /* output vector for substring information */
                                   ovector_length);          /* number of elements in the output vector */

                    //std::cerr << " -> " << rc << std::endl;
                    if (rc < 0) { // Error
                        break;
                    }

                    for (int i = 0; i < substring_count + 1; ++i) {
                        int match_offset = ovector[i * 2];
                        int match_length = ovector[i * 2 + 1] - match_offset;
                        if (i == 0) {
                            offset = match_offset + match_length;
                        }
                        if (match_length > 0) {
                            //std::cerr << "match " << i << " from " << match_offset << " length " << match_length << "       " << TextExtentHandle(subExtentUtf8(match_offset, match_length))->text() << std::endl;
                            matches.insert(TextExtentHandle(subExtentUtf8(match_offset, match_length)));
                        } else if (i == 0) {
                            break;
                        }
                    }
                }

                // Delete regular expression
                pcre_free(re);
            }
        }

        return matches;
    }

    Spine::TextIterator TextExtent::iteratorFromOffset(size_t start_) const {
        return this->_iteratorFromOffset(start_, this->_skiplist_utf32);
    }

    Spine::TextIterator TextExtent::iteratorFromOffsetUtf8(size_t start_) const {
        return this->_iteratorFromOffset(start_, this->_skiplist_utf8);
    }

    Spine::TextIterator TextExtent::_iteratorFromOffset(size_t start_,
                                                       const std::map<size_t,
                                                       TextIterator> &skiplist_) const {

        if(_cached_text.empty()) {
            _cacheText();
        }

        // This returns an iterator pointing to the first element in
        // the container whose key compares greater than x
        std::map<size_t, TextIterator>::const_iterator i(skiplist_.upper_bound(start_));

        // decrement to get key preceding offset
        --i;

        size_t ti_pos(i->first);
        TextIterator ti(i->second);

        if(ti_pos > start_) {
            // shouldn't happen!
//            printf("Skiplist: ti_pos > start_\n");
            ti=first;
            ti_pos=0;
        }
        while(ti_pos < start_ && ti != second) {
            ++ti_pos;
            ++ti;
        }
        if(ti==second) {
            // shouldn't happen!
//            printf("Skiplist: ti==second\n");
        }

        return ti;
    }

    Spine::TextExtentHandle TextExtent::subExtent(size_t start_, size_t length_) const {
        return this->_cachedSubExtent(start_, length_, this->_skiplist_utf32);
    }

    Spine::TextExtentHandle TextExtent::subExtentUtf8(size_t start_, size_t length_) const {
        return this->_cachedSubExtent(start_, length_, this->_skiplist_utf8);
    }

    Spine::TextExtentHandle TextExtent::_cachedSubExtent(size_t start_, size_t length_,
                                                         const std::map<size_t,
                                                         TextIterator> &skiplist_) const {

        TextIterator match_begin(this->_iteratorFromOffset(start_, skiplist_));
        TextIterator match_end(this->_iteratorFromOffset(start_+length_, skiplist_));

        return TextExtentHandle(new TextExtent(match_begin, match_end));
    }

    Spine::TextExtentHandle TextExtent::clone()
    {
        return Spine::TextExtentHandle(new TextExtent(*this));
    }

}

bool operator < (const Spine::TextExtentHandle & lhs, const Spine::TextExtentHandle & rhs)
{
    if (lhs->first == rhs->first) {
        return rhs->second < lhs->second;
    } else {
        return lhs->first < rhs->first;
    }
    //return *lhs < *rhs;
}

#include <spine/Annotation.h>
#include "spineapi.h"
#include "spineapi_internal.h"

SpineTextExtent copy_SpineTextExtent(Spine::TextExtentHandle extent_, SpineError *error_)
{
    SpineTextExtent extent=new struct SpineTextExtentImpl;
    extent->_handle=extent_;
    return extent;
}

Spine::TextExtentHandle SpineTextExtent_extent(SpineTextExtent extent_, SpineError *error_)
{
    return extent_->_handle;
}
