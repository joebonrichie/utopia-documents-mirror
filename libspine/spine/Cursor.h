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

#ifndef CURSOR_INCL_
#define CURSOR_INCL_

/*****************************************************************************
 *
 * Cursor.h
 *
 * Copyright 2008 Advanced Interfaces Group
 *
 ****************************************************************************/

#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <string>
#include <iostream>

#define PARENT_OF_Page Document
#define PARENT_OF_Image Page
#define PARENT_OF_Region Page
#define PARENT_OF_Block Region
#define PARENT_OF_Line Block
#define PARENT_OF_Word Line
#define PARENT_OF_Character Word

namespace Spine {

    class Document;
    class Page;
    class Image;
    class Region;
    class Block;
    class Line;
    class Word;
    class Character;

    typedef enum { DoNotIterate=0,
                   WithinWord=1,
                   WithinLine=2,
                   WithinBlock=3,
                   WithinRegion=4,
                   WithinPage=5,
                   WithinDocument=6,
                   UntilEnd=99999 } IterateLimit;

    typedef enum { ElementCharacter,
                   ElementWord,
                   ElementLine,
                   ElementBlock,
                   ElementRegion,
                   ElementImage,
                   //                 ElementFont,
                   ElementPage } DocumentElement;

    class Cursor
    {
        typedef boost::shared_ptr<Cursor> CursorHandle;

    public:
        virtual ~Cursor() {}

        virtual Document * document() = 0;
        virtual bool isValid() = 0;
        virtual const char * type() = 0;

        inline operator bool() { return isValid(); }

        virtual inline bool gotoPage(int page_) // 1-based page index
            {
                for (toFrontPage(); --page_ > 0 && hasNextPage(); nextPage()) {}
                return getPage() != 0;
            }


        /*******************************************************************************************
         *  Cursors have the following methods available on them:
         *    hasNext* - are there any more after this?
         *    hasPrevious* - are there any more before this?
         *    next* - advance to next element
         *    previous* - advance to previous element
         *    peekNext* - peek at the element after this one?
         *    peekPrevious* - peek at the element before this?
         *    toBack, toFront - go straight to a particular end of list
         *
         *    document, page, image, font, region, block, line, word, character - accessors
         *******************************************************************************************/

#define SPINE_CURSOR_NEXT_METHOD(ELEMENT) SPINE_CURSOR_NEXT_METHOD2(ELEMENT, PARENT_OF_ ## ELEMENT)
#define SPINE_CURSOR_NEXT_METHOD2(ELEMENT, PARENT) SPINE_CURSOR_NEXT_METHOD3(ELEMENT, PARENT)
#define SPINE_CURSOR_NEXT_METHOD3(ELEMENT, PARENT)                      \
        virtual const ELEMENT * next ## ELEMENT(IterateLimit limit_ = Within ## PARENT) = 0;

        SPINE_CURSOR_NEXT_METHOD(Page)
        SPINE_CURSOR_NEXT_METHOD(Image)
        SPINE_CURSOR_NEXT_METHOD(Region)
        SPINE_CURSOR_NEXT_METHOD(Block)
        SPINE_CURSOR_NEXT_METHOD(Line)
        SPINE_CURSOR_NEXT_METHOD(Word)
        SPINE_CURSOR_NEXT_METHOD(Character)
#undef SPINE_CURSOR_NEXT_METHOD
#undef SPINE_CURSOR_NEXT_METHOD2
#undef SPINE_CURSOR_NEXT_METHOD3

#define SPINE_CURSOR_PREVIOUS_METHOD(ELEMENT) SPINE_CURSOR_PREVIOUS_METHOD2(ELEMENT, PARENT_OF_ ## ELEMENT)
#define SPINE_CURSOR_PREVIOUS_METHOD2(ELEMENT, PARENT) SPINE_CURSOR_PREVIOUS_METHOD3(ELEMENT, PARENT)
#define SPINE_CURSOR_PREVIOUS_METHOD3(ELEMENT, PARENT)                  \
        virtual const ELEMENT * previous ## ELEMENT(IterateLimit limit_ = Within ## PARENT) = 0;

        SPINE_CURSOR_PREVIOUS_METHOD(Page)
        SPINE_CURSOR_PREVIOUS_METHOD(Image)
        SPINE_CURSOR_PREVIOUS_METHOD(Region)
        SPINE_CURSOR_PREVIOUS_METHOD(Block)
        SPINE_CURSOR_PREVIOUS_METHOD(Line)
        SPINE_CURSOR_PREVIOUS_METHOD(Word)
        SPINE_CURSOR_PREVIOUS_METHOD(Character)
#undef SPINE_CURSOR_PREVIOUS_METHOD
#undef SPINE_CURSOR_PREVIOUS_METHOD2
#undef SPINE_CURSOR_PREVIOUS_METHOD3

#define SPINE_CURSOR_PEEK_NEXT_METHOD(ELEMENT) SPINE_CURSOR_PEEK_NEXT_METHOD2(ELEMENT, PARENT_OF_ ## ELEMENT)
#define SPINE_CURSOR_PEEK_NEXT_METHOD2(ELEMENT, PARENT) SPINE_CURSOR_PEEK_NEXT_METHOD3(ELEMENT, PARENT)
#define SPINE_CURSOR_PEEK_NEXT_METHOD3(ELEMENT, PARENT)                 \
        inline const ELEMENT * peekNext ## ELEMENT(IterateLimit limit_ = Within ## PARENT) \
            {                                                           \
                return clone()->next ## ELEMENT(limit_);                \
            }

        SPINE_CURSOR_PEEK_NEXT_METHOD(Page)
        SPINE_CURSOR_PEEK_NEXT_METHOD(Image)
        SPINE_CURSOR_PEEK_NEXT_METHOD(Region)
        SPINE_CURSOR_PEEK_NEXT_METHOD(Block)
        SPINE_CURSOR_PEEK_NEXT_METHOD(Line)
        SPINE_CURSOR_PEEK_NEXT_METHOD(Word)
        SPINE_CURSOR_PEEK_NEXT_METHOD(Character)
#undef SPINE_CURSOR_PEEK_NEXT_METHOD
#undef SPINE_CURSOR_PEEK_NEXT_METHOD2
#undef SPINE_CURSOR_PEEK_NEXT_METHOD3

#define SPINE_CURSOR_PEEK_PREVIOUS_METHOD(ELEMENT) SPINE_CURSOR_PEEK_PREVIOUS_METHOD2(ELEMENT, PARENT_OF_ ## ELEMENT)
#define SPINE_CURSOR_PEEK_PREVIOUS_METHOD2(ELEMENT, PARENT) SPINE_CURSOR_PEEK_PREVIOUS_METHOD3(ELEMENT, PARENT)
#define SPINE_CURSOR_PEEK_PREVIOUS_METHOD3(ELEMENT, PARENT)             \
        inline const ELEMENT * peekPrevious ## ELEMENT(IterateLimit limit_ = Within ## PARENT) \
            {                                                           \
                return clone()->previous ## ELEMENT(limit_);            \
            }

        SPINE_CURSOR_PEEK_PREVIOUS_METHOD(Page)
        SPINE_CURSOR_PEEK_PREVIOUS_METHOD(Image)
        SPINE_CURSOR_PEEK_PREVIOUS_METHOD(Region)
        SPINE_CURSOR_PEEK_PREVIOUS_METHOD(Block)
        SPINE_CURSOR_PEEK_PREVIOUS_METHOD(Line)
        SPINE_CURSOR_PEEK_PREVIOUS_METHOD(Word)
        SPINE_CURSOR_PEEK_PREVIOUS_METHOD(Character)
#undef SPINE_CURSOR_PEEK_PREVIOUS_METHOD
#undef SPINE_CURSOR_PEEK_PREVIOUS_METHOD2
#undef SPINE_CURSOR_PEEK_PREVIOUS_METHOD3

#define SPINE_CURSOR_HAS_NEXT_METHOD(ELEMENT) SPINE_CURSOR_HAS_NEXT_METHOD2(ELEMENT, PARENT_OF_ ## ELEMENT)
#define SPINE_CURSOR_HAS_NEXT_METHOD2(ELEMENT, PARENT) SPINE_CURSOR_HAS_NEXT_METHOD3(ELEMENT, PARENT)
#define SPINE_CURSOR_HAS_NEXT_METHOD3(ELEMENT, PARENT)                  \
        inline bool hasNext ## ELEMENT(IterateLimit limit_ = Within ## PARENT) \
            {                                                           \
                return peekNext ## ELEMENT(limit_) != 0;                \
            }

        SPINE_CURSOR_HAS_NEXT_METHOD(Page)
        SPINE_CURSOR_HAS_NEXT_METHOD(Image)
        SPINE_CURSOR_HAS_NEXT_METHOD(Region)
        SPINE_CURSOR_HAS_NEXT_METHOD(Block)
        SPINE_CURSOR_HAS_NEXT_METHOD(Line)
        SPINE_CURSOR_HAS_NEXT_METHOD(Word)
        SPINE_CURSOR_HAS_NEXT_METHOD(Character)
#undef SPINE_CURSOR_HAS_NEXT_METHOD
#undef SPINE_CURSOR_HAS_NEXT_METHOD2
#undef SPINE_CURSOR_HAS_NEXT_METHOD3

#define SPINE_CURSOR_HAS_PREVIOUS_METHOD(ELEMENT) SPINE_CURSOR_HAS_PREVIOUS_METHOD2(ELEMENT, PARENT_OF_ ## ELEMENT)
#define SPINE_CURSOR_HAS_PREVIOUS_METHOD2(ELEMENT, PARENT) SPINE_CURSOR_HAS_PREVIOUS_METHOD3(ELEMENT, PARENT)
#define SPINE_CURSOR_HAS_PREVIOUS_METHOD3(ELEMENT, PARENT)              \
        inline bool hasPrevious ## ELEMENT(IterateLimit limit_ = Within ## PARENT) \
            {                                                           \
                return peekPrevious ## ELEMENT(limit_) != 0;            \
            }

        SPINE_CURSOR_HAS_PREVIOUS_METHOD(Page)
        SPINE_CURSOR_HAS_PREVIOUS_METHOD(Image)
        SPINE_CURSOR_HAS_PREVIOUS_METHOD(Region)
        SPINE_CURSOR_HAS_PREVIOUS_METHOD(Block)
        SPINE_CURSOR_HAS_PREVIOUS_METHOD(Line)
        SPINE_CURSOR_HAS_PREVIOUS_METHOD(Word)
        SPINE_CURSOR_HAS_PREVIOUS_METHOD(Character)
#undef SPINE_CURSOR_HAS_PREVIOUS_METHOD
#undef SPINE_CURSOR_HAS_PREVIOUS_METHOD2
#undef SPINE_CURSOR_HAS_PREVIOUS_METHOD3

#define SPINE_CURSOR_TO_BACK_METHOD(ELEMENT)    \
        inline void toBack ## ELEMENT()         \
            {                                   \
                toBack(Element ## ELEMENT);     \
            }

        SPINE_CURSOR_TO_BACK_METHOD(Page)
        SPINE_CURSOR_TO_BACK_METHOD(Image)
        SPINE_CURSOR_TO_BACK_METHOD(Region)
        SPINE_CURSOR_TO_BACK_METHOD(Block)
        SPINE_CURSOR_TO_BACK_METHOD(Line)
        SPINE_CURSOR_TO_BACK_METHOD(Word)
        SPINE_CURSOR_TO_BACK_METHOD(Character)
#undef SPINE_CURSOR_TO_BACK_METHOD

#define SPINE_CURSOR_TO_FRONT_METHOD(ELEMENT)   \
        inline void toFront ## ELEMENT()        \
            {                                   \
                toFront(Element ## ELEMENT);    \
            }

        SPINE_CURSOR_TO_FRONT_METHOD(Page)
        SPINE_CURSOR_TO_FRONT_METHOD(Image)
        SPINE_CURSOR_TO_FRONT_METHOD(Region)
        SPINE_CURSOR_TO_FRONT_METHOD(Block)
        SPINE_CURSOR_TO_FRONT_METHOD(Line)
        SPINE_CURSOR_TO_FRONT_METHOD(Word)
        SPINE_CURSOR_TO_FRONT_METHOD(Character)
#undef SPINE_CURSOR_TO_FRONT_METHOD

        /*******************************************************************************************
         *  The accessor methods are required to return the currently pointed at document element,
         *  or NULL if cursor is at end.
         *******************************************************************************************/
        virtual const Page * page() = 0;
        virtual const Image * image() = 0;
        virtual const Region * region() = 0;
        virtual const Block * block() = 0;
        virtual const Line * line() = 0;
        virtual const Word * word() = 0;
        virtual const Character * character() = 0;
        virtual boost::tuple<Document *,
                             const Page *,
                             const Image *,
                             const Region *,
                             const Block *,
                             const Line *,
                             const Word *,
                             const Character *> fullState()
            {
                return boost::make_tuple(document(),
                                         page(),
                                         image(),
                                         region(),
                                         block(),
                                         line(),
                                         word(),
                                         character());
            }

    protected:
        /*******************************************************************************************
         *  The to* methods are required to advance to the front or back of an element list.
         *******************************************************************************************/
        virtual void toBack(DocumentElement element_) = 0;
        virtual void toFront(DocumentElement element_) = 0;

        virtual const Page * getPage() { return page(); }
        virtual const Image * getImage() { return image(); }
        virtual const Region * getRegion() { return region(); }
        virtual const Block * getBlock() { return block(); }
        virtual const Line * getLine() { return line(); }
        virtual const Word * getWord() { return word(); }
        virtual const Character * getCharacter() { return character(); }

    public:

        /************************************************************************/

        virtual bool operator == (Cursor &rhs_) = 0;
        virtual bool operator != (Cursor &rhs_) { return !operator==(rhs_); }
        virtual bool operator < (Cursor &rhs_) = 0;
        virtual bool operator <= (Cursor &rhs_) { return !operator>(rhs_); }
        virtual bool operator > (Cursor &rhs_) { return rhs_ < *this; }
        virtual bool operator >= (Cursor &rhs_) { return !operator<(rhs_); }

        /************************************************************************/

        virtual boost::shared_ptr<Cursor> clone() = 0;
        virtual std::string repr() = 0;

    };

    typedef boost::shared_ptr<Cursor> CursorHandle;
    typedef boost::weak_ptr<Cursor> WeakCursorHandle;

    inline bool order(CursorHandle & first, CursorHandle & second)
    {
        if (*first > *second)
        {
            CursorHandle tmp(first); first = second; second = tmp;
            return true;
        }
        else
            return false;
    }

}

#undef PARENT_OF_Page
#undef PARENT_OF_Image
#undef PARENT_OF_Region
#undef PARENT_OF_Block
#undef PARENT_OF_Line
#undef PARENT_OF_Word
#undef PARENT_OF_Character

#endif /* CURSOR_INCL_ */
