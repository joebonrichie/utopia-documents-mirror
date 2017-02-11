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

#ifndef TEXTITERATOR_INCL_
#define TEXTITERATOR_INCL_

#include <boost/shared_ptr.hpp>
#include <string>
#include <iterator>
#include <vector>

#include <utf8/unicode.h>

#include <spine/Block.h>
#include <spine/Character.h>
#include <spine/Cursor.h>
#include <spine/TextIterator.h>
#include <spine/Line.h>
#include <spine/Page.h>
#include <spine/Region.h>
#include <spine/Word.h>

/*****************************************************************************
 *
 * TextIterator.h
 *
 * Copyright 2009 Advanced Interfaces Group
 *
 ****************************************************************************/

namespace Spine
{

    class Cursor;

    // FIXME - check and sanitise cursors in constructors!

    /***************************************************************************
     *
     * TextIterator
     *
     **************************************************************************/

    class TextIterator
    {
    public:
        typedef const utf8::uint32_t                   value_type;
        typedef size_t                                 difference_type;
        typedef struct std::bidirectional_iterator_tag iterator_category;
        typedef value_type *                           pointer;
        typedef value_type &                           reference;

        // Default Constructible
        TextIterator() {} // Trivial

        // Assignable
        TextIterator(const TextIterator & rhs)
        {
            _cursor = rhs._cursor.get() ? rhs._cursor->clone() : boost::shared_ptr< Cursor >((Cursor*) 0);
            _ligature = rhs._ligature;
            _subLigature = rhs._subLigature;
        }

        TextIterator & operator = (const TextIterator & rhs)
        {
            _cursor = rhs._cursor.get() ? rhs._cursor->clone() : boost::shared_ptr< Cursor >((Cursor*) 0);
            _ligature = rhs._ligature;
            _subLigature = rhs._subLigature;

            return *this;
        }

        // Equality Comparable
        bool operator == (const TextIterator & rhs) const {
            // Either both singular or both equal
            return (_cursor == 0 && rhs._cursor == 0) ||
                (_subLigature == rhs._subLigature && *_cursor == *rhs._cursor);
        }

        inline bool operator != (const TextIterator & rhs) const { return !(*this == rhs); } // Trivial

        // Trivial Iterator
        value_type operator * () const
        {
            if (_cursor->character()) {
                return _ligature[_subLigature];
            } else {
                return ' ';
            }
        }

        // Forward Iterator
        // Pre
        TextIterator & operator ++ ()
        {
            const Character * character = _cursor->character();
            if (character == 0) {
                character = _cursor->nextCharacter(WithinDocument);
            }

            // Deal with ligatures
            else if (_subLigature < _ligature.size() - 1) {
                ++_subLigature;
                return *this;
            }
            else {
                character = _cursor->nextCharacter(_cursor->word()->spaceAfter() || !_cursor->hasNextWord(WithinLine) ? WithinWord : WithinDocument);

                // If now at end of hyphenated line, skip to start of next line
                if (character)
                {
                    if (_cursor->line()->hyphenated() && !_cursor->hasNextCharacter(WithinLine))
                    {
                        // Skip hyphen and subsequent space
                        character = _cursor->nextCharacter(WithinDocument);
                    }
                }
            }

            if (character)
            {
                _compileLigature();
            }
            else if (_cursor->page() == 0)
            {
                _cursor->previousCharacter(WithinDocument);
                _cursor->nextCharacter();
            }

            return *this;
        }

        TextIterator operator ++ (int) { TextIterator v(*this); ++*this; return v; } // Trivial (Post)

        // Bidirectional Iterator
        // Pre
        TextIterator & operator -- ()
        {
            if (_subLigature > 0)
            {
                --_subLigature;
            }
            else
            {
                const Character * character = _cursor->previousCharacter();
                if (character)
                {
                    _compileLigature();
                    _subLigature = _ligature.size() - 1;
                }
                else
                {
                    character = _cursor->previousCharacter(WithinDocument);

                    // Deal with hyphenated lines
                    if (_cursor->line()->hyphenated() && !_cursor->hasNextCharacter(WithinLine))
                    {
                        // Skip hyphen
                        character = _cursor->previousCharacter();
                    }
                    // Deal with spaceAfter()
                    else if (_cursor->word()->spaceAfter() || !_cursor->hasNextWord(WithinLine))
                    {
                        character = _cursor->nextCharacter();
                    }

                    if (character)
                    {
                        _compileLigature();
                        _subLigature = _ligature.size() - 1;
                    }
                }
            }

            return *this;
        }

        TextIterator operator -- (int) { TextIterator v(*this); --*this; return v; } // Trivial (Post)

        // Document Text Iterator
        TextIterator(boost::shared_ptr< Spine::Cursor > cursor)
            : _cursor(cursor->clone()), _subLigature(0)
        {
            bool advance = false;

            // resolve spaceAfter/hyphen stuff
            if (_cursor->character() == 0)
            {
                // Move to next character if:
                // 1. this is the end of a line with no space after it, or
                // 2. word() == 0
                if (_cursor->word() == 0 || !_cursor->word()->spaceAfter())
                {
                    advance = true;
                }
            }
            else if (_cursor->line()->hyphenated() && !_cursor->hasNextCharacter(WithinLine))
            {
                // Skip hyphen and subsequent space
                advance = true;
            }
            if (advance && _cursor->hasNextCharacter(WithinDocument))
            {
                _cursor->nextCharacter(WithinDocument);
            }

            _compileLigature();
        }


        boost::shared_ptr< Spine::Cursor > cursor() const {
            return _cursor->clone();
        }

        bool isSingular() const {
            return _cursor.get() == 0;
        }


        // Ordered
        bool operator < (const TextIterator & rhs) const {
            // Either both singular or both equal
            return *_cursor < *rhs._cursor || (*_cursor == *rhs._cursor && _subLigature < rhs._subLigature);
        }

        bool inline operator <= (const TextIterator & rhs) const { return !operator>(rhs); }
        bool inline operator > (const TextIterator & rhs) const { return rhs < *this; }
        bool inline operator >= (const TextIterator & rhs) const { return !operator<(rhs); }

    private:
        boost::shared_ptr< Spine::Cursor > _cursor;
        std::vector<utf8::uint32_t> _ligature;
        size_t _subLigature;

        void _compileLigature()
        {
            // decompose character
            _ligature.clear();
            _subLigature = 0;

            const Character * c(_cursor->character());
            if (c) {
                try {
                    std::string text(c->text());
                    utf8::utf8to32(text.begin(), text.end(), std::back_inserter(_ligature));
                }
                catch (utf8::exception e) {
                    // invalid utf8 char so insert Unicode replacement character
                    _ligature.push_back(0xFFFD);
                }
            }

            // FIXME - what happens to state of iterator if _cursor->character() is 0?
            // I've caused it be the replacement character - JM

            // FIXME - code in above methods assumes _ligature is not empty
            if(_ligature.size()==0) {
                _ligature.push_back(0xFFFD);
            }
        }

    };

    /***************************************************************************
     *
     * TextIterator
     *
     **************************************************************************/

    class FontEncodedTextIterator : public TextIterator
    {
    public:
        // Default Constructible
        FontEncodedTextIterator() : TextIterator() {} // Trivial

        // Assignable
        FontEncodedTextIterator(const FontEncodedTextIterator & rhs);

        // Trivial Iterator
        value_type operator * () const;

        // Forward Iterator
        FontEncodedTextIterator & operator ++ (); // Pre
        FontEncodedTextIterator operator ++ (int) { FontEncodedTextIterator v(*this); ++*this; return v; } // Trivial (Post)

        // Bidirectional Iterator
        FontEncodedTextIterator & operator -- (); // Pre
        FontEncodedTextIterator operator -- (int) { FontEncodedTextIterator v(*this); --*this; return v; } // Trivial (Post)

        // Document Text Iterator
        FontEncodedTextIterator(boost::shared_ptr< Spine::Cursor > cursor);
        FontEncodedTextIterator(const TextIterator & rhs);

    private:
        std::string _prefix;
    };

}

#endif /* TEXTITERATOR_INCL_ */
