/*****************************************************************************
 *  
 *   This file is part of the libcrackle library.
 *       Copyright (c) 2008-2017 Lost Island Labs
 *           <info@utopiadocs.com>
 *   
 *   The libcrackle library is free software: you can redistribute it and/or
 *   modify it under the terms of the GNU AFFERO GENERAL PUBLIC LICENSE
 *   VERSION 3 as published by the Free Software Foundation.
 *   
 *   The libcrackle library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero
 *   General Public License for more details.
 *   
 *   You should have received a copy of the GNU Affero General Public License
 *   along with the libcrackle library. If not, see
 *   <http://www.gnu.org/licenses/>
 *  
 *****************************************************************************/

#ifndef PDFCURSOR_INCL_
#define PDFCURSOR_INCL_

/*****************************************************************************
 *
 * PDFCursor.h
 *
 * Copyright 2008 Advanced Interfaces Group
 *
 ****************************************************************************/

#include <spine/Cursor.h>
#include <crackle/PDFDocument.h>
#include <crackle/PDFPage.h>
#include <crackle/PDFTextRegionCollection.h>
#include <crackle/PDFTextBlockCollection.h>
#include <crackle/PDFTextLineCollection.h>
#include <crackle/PDFTextWordCollection.h>
#include <crackle/PDFTextCharacterCollection.h>
#include <crackle/ImageCollection.h>
#include <crackle/PDFFontCollection.h>
#include <iostream>
#include <sstream>

namespace Crackle {

    class PDFCursor : public Spine::Cursor
    {

        typedef boost::shared_ptr< Spine::Cursor > CursorHandle;

    public:

        Spine::Document * document() { return _doc; }
        const char * type() { return "PDF"; }
        bool isValid() { return _doc != 0; }
        inline bool gotoPage(int page_) // 1-based page index
        {
            if (isValid())
            {
                _page = _doc->begin();
                while (page_ > 1 && _page != _doc->end()) { ++_page; --page_; }
                if (_page != _doc->end())
                {
                    toFront(Spine::ElementImage);
                    return true;
                }
            }
            return false;
        }

        /*******************************************************************************************
         *  The accessor methods are required to return the currently pointed at document element,
         *  or NULL if cursor is at end.
         *******************************************************************************************/
        const Spine::Page * page() { return isValidPage() ? &*_page : 0; }
        const Spine::Image * image() { return isValidImage() ? &*_image : 0; }
        const Spine::Region * region() { return isValidRegion() ? &*_region : 0; }
        const Spine::Block * block() { return isValidBlock() ? &*_block : 0; }
        const Spine::Line * line() { return isValidLine() ? &*_line : 0; }
        const Spine::Word * word() { return isValidWord() ? &*_word : 0; }
        const Spine::Character * character() { return isValidCharacter() ? &*_character : 0; }
        boost::tuple<Spine::Document *,
                     const Spine::Page *,
                     const Spine::Image *,
                     const Spine::Region *,
                     const Spine::Block *,
                     const Spine::Line *,
                     const Spine::Word *,
                     const Spine::Character *> fullState()
        {
            Spine::Document * document = 0;
            const Spine::Page * page = 0;
            const Spine::Image * image = 0;
            const Spine::Region * region = 0;
            const Spine::Block * block = 0;
            const Spine::Line * line = 0;
            const Spine::Word * word = 0;
            const Spine::Character * character = 0;

            if ( (document = _doc) )
            {
                if ( (page = _page != _doc->end() ? &*_page : 0 ))
                {
                    image = _image != _page->images().end() ? &*_image : 0;
                    if ( (region = _region != _page->regions().end() ? &*_region : 0) )
                    {
                        if ( (block = _block != _region->blocks().end() ? &*_block : 0) )
                        {
                            if ( (line = _line != _block->lines().end() ? &*_line : 0) )
                            {
                                if ( (word = _word != _line->words().end() ? &*_word : 0) )
                                {
                                    character = _character != _word->characters().end() ? &*_character : 0;
                                }
                            }
                        }
                    }
                }
            }

            return boost::make_tuple(document,
                                     page,
                                     image,
                                     region,
                                     block,
                                     line,
                                     word,
                                     character);
        }

        /************************************************************************/

        typedef Crackle::PDFDocument::const_iterator                  page_iterator;
        typedef Crackle::PDFTextRegionCollection::const_iterator      region_iterator;
        typedef Crackle::PDFTextBlockCollection::const_iterator       block_iterator;
        typedef Crackle::PDFTextLineCollection::const_iterator        line_iterator;
        typedef Crackle::PDFTextWordCollection::const_iterator        word_iterator;
        typedef Crackle::PDFTextCharacterCollection::const_iterator   character_iterator;
        typedef Crackle::ImageCollection::const_iterator              image_iterator;
        typedef Crackle::PDFFontCollection::const_iterator            font_iterator;


        inline PDFCursor() : _doc(0) {}

        inline PDFCursor(const PDFCursor &rhs_)
            : _doc(rhs_._doc),
              _page(rhs_._page),
              _image(rhs_._image),
              _region(rhs_._region),
              _block(rhs_._block),
              _line(rhs_._line),
              _word(rhs_._word),
              _character(rhs_._character),
              _font(rhs_._font)
        {}

    protected:

        inline bool isValidDocument() { return _doc!=0; }
        inline bool isValidPage() { return isValidDocument() && _page!=_doc->end(); }
        inline bool isValidImage(Spine::IterateLimit assumeTrue_ = Spine::WithinDocument)
        {
            return (assumeTrue_ == Spine::WithinPage || isValidPage()) && _image!=_page->images().end();
        }
        inline bool isValidRegion(Spine::IterateLimit assumeTrue_ = Spine::WithinDocument)
        {
            return (assumeTrue_ == Spine::WithinPage || isValidPage()) && _region!=_page->regions().end();
        }
        inline bool isValidBlock(Spine::IterateLimit assumeTrue_ = Spine::WithinDocument)
        {
            return (assumeTrue_ == Spine::WithinRegion || isValidRegion(assumeTrue_)) && _block!=_region->blocks().end();
        }
        inline bool isValidLine(Spine::IterateLimit assumeTrue_ = Spine::WithinDocument)
        {
            return (assumeTrue_ == Spine::WithinBlock || isValidBlock(assumeTrue_)) && _line!=_block->lines().end();
        }
        inline bool isValidWord(Spine::IterateLimit assumeTrue_ = Spine::WithinDocument)
        {
            return (assumeTrue_ == Spine::WithinLine || isValidLine(assumeTrue_)) && _word!=_line->words().end();
        }
        inline bool isValidCharacter(Spine::IterateLimit assumeTrue_ = Spine::WithinDocument)
        {
            return (assumeTrue_ == Spine::WithinWord || isValidWord(assumeTrue_)) && _character!=_word->characters().end();
        }

        /*******************************************************************************************
         *  The skip* methods are required to advance to the beginning of the adjacent element.
         *******************************************************************************************/
        const Spine::Page * nextPage(Spine::IterateLimit limit_ = Spine::WithinDocument)
        {
            if (limit_ < Spine::WithinDocument) return 0;
            if (isValidPage())
            {
                ++_page;
                toFront(Spine::ElementImage);
            }
            return getPage();
        }

        const Spine::Image * nextImage(Spine::IterateLimit limit_ = Spine::WithinPage)
        {
            if (limit_ < Spine::WithinPage) return 0;
            if (isValidImage())
            {
                ++_image;
            }
            if (limit_ > Spine::WithinPage)
            {
                while (isValidPage() && !isValidImage(Spine::WithinPage))
                {
                    nextPage(limit_);
                }
            }
            return getImage();
        }

        const Spine::Region * nextRegion(Spine::IterateLimit limit_ = Spine::WithinPage)
        {
            if (limit_ < Spine::WithinPage) return 0;
            if (isValidRegion())
            {
                ++_region;
                if (_region != _page->regions().end())
                {
                    toFront(Spine::ElementBlock, false);
                }
            }
            if (limit_ > Spine::WithinPage)
            {
                while (isValidPage() && !isValidRegion(Spine::WithinPage))
                {
                    nextPage(limit_);
                }
            }
            return getRegion();
        }

        const Spine::Block * nextBlock(Spine::IterateLimit limit_ = Spine::WithinRegion)
        {
            if (limit_ < Spine::WithinRegion) return 0;
            if (isValidBlock())
            {
                ++_block;
                if (_block != _region->blocks().end())
                {
                    toFront(Spine::ElementLine, false);
                }
            }
            if (limit_ > Spine::WithinRegion)
            {
                while (isValidRegion(limit_) && !isValidBlock(Spine::WithinRegion))
                {
                    nextRegion(limit_);
                }
            }
            return getBlock();
        }

        const Spine::Line * nextLine(Spine::IterateLimit limit_ = Spine::WithinBlock)
        {
            if (limit_ < Spine::WithinBlock) return 0;
            if (isValidLine())
            {
                ++_line;
                if (_line != _block->lines().end())
                {
                    toFront(Spine::ElementWord, false);
                }
            }
            if (limit_ > Spine::WithinBlock)
            {
                while (isValidBlock(limit_) && !isValidLine(Spine::WithinBlock))
                {
                    nextBlock(limit_);
                }
            }
            return getLine();
        }

        const Spine::Word * nextWord(Spine::IterateLimit limit_ = Spine::WithinLine)
        {
            if (limit_ < Spine::WithinLine) return 0;
            if (isValidWord())
            {
                ++_word;
                if (_word != _line->words().end())
                {
                    toFront(Spine::ElementCharacter, false);
                }
            }
            if (limit_ > Spine::WithinLine)
            {
                while (isValidLine(limit_) && !isValidWord(Spine::WithinLine))
                {
                    nextLine(limit_);
                }
            }
            return getWord();
        }

        const Spine::Character * nextCharacter(Spine::IterateLimit limit_ = Spine::WithinWord)
        {
            if (isValidCharacter())
            {
                ++_character;
            }
            if (limit_ > Spine::WithinWord)
            {
                while (isValidWord(limit_) && !isValidCharacter(Spine::WithinWord))
                {
                    nextWord(limit_);
                }
            }
            return getCharacter();
        }

        const Spine::Page * previousPage(Spine::IterateLimit limit_ = Spine::WithinDocument)
        {
            if (limit_ < Spine::WithinDocument) return 0;
            if (isValidDocument() && _page != _doc->begin())
            {
                --_page;
                toFront(Spine::ElementImage, false);
                return &*_page;
            }
            return 0;
        }

        const Spine::Image * previousImage(Spine::IterateLimit limit_ = Spine::WithinPage)
        {
            if (limit_ < Spine::WithinPage) return 0;
            if (isValidPage() && _image != _page->images().begin())
            {
                --_image;
                return &*_image;
            }
            if (limit_ > Spine::WithinPage)
            {
                bool end = false;
                do
                {
                    end = previousPage(limit_) == 0;
                }
                while (!end && isValidDocument() && !isValidImage(Spine::WithinPage));
                if (!end && isValidImage())
                {
                    toBack(Spine::ElementImage, false);
                    --_image;
                    return &*_image;
                }
            }
            return 0;
        }

        const Spine::Region * previousRegion(Spine::IterateLimit limit_ = Spine::WithinPage)
        {
            if (limit_ < Spine::WithinPage) return 0;
            if (isValidPage() && _region != _page->regions().begin())
            {
                --_region;
                toFront(Spine::ElementBlock, false);
                return &*_region;
            }
            if (limit_ > Spine::WithinPage)
            {
                bool end = false;
                do
                {
                    end = previousPage(limit_) == 0;
                }
                while (!end && isValidDocument() && !isValidRegion(Spine::WithinPage));
                if (!end && isValidRegion())
                {
                    toBack(Spine::ElementRegion, false);
                    --_region;
                    toFront(Spine::ElementBlock, false);
                    return &*_region;
                }
            }
            return 0;
        }

        const Spine::Block * previousBlock(Spine::IterateLimit limit_ = Spine::WithinRegion)
        {
            if (limit_ < Spine::WithinRegion) return 0;
            if (isValidRegion() && _block != _region->blocks().begin())
            {
                --_block;
                toFront(Spine::ElementLine, false);
                return &*_block;
            }
            if (limit_ > Spine::WithinRegion)
            {
                bool end = false;
                do
                {
                    end = previousRegion(limit_) == 0;
                }
                while (!end && isValidPage() && !isValidBlock(Spine::WithinRegion));
                if (!end && isValidBlock())
                {
                    toBack(Spine::ElementBlock, false);
                    --_block;
                    toFront(Spine::ElementLine, false);
                    return &*_block;
                }
            }
            return 0;
        }

        const Spine::Line * previousLine(Spine::IterateLimit limit_ = Spine::WithinBlock)
        {
            if (limit_ < Spine::WithinBlock) return 0;
            if (isValidBlock() && _line != _block->lines().begin())
            {
                --_line;
                toFront(Spine::ElementWord, false);
                return &*_line;
            }
            if (limit_ > Spine::WithinBlock)
            {
                bool end = false;
                do
                {
                    end = previousBlock(limit_) == 0;
                }
                while (!end && isValidRegion() && !isValidLine(Spine::WithinBlock));
                if (!end && isValidLine())
                {
                    toBack(Spine::ElementLine, false);
                    --_line;
                    toFront(Spine::ElementWord, false);
                    return &*_line;
                }
            }
            return 0;
        }

        const Spine::Word * previousWord(Spine::IterateLimit limit_ = Spine::WithinLine)
        {
            if (limit_ < Spine::WithinLine) return 0;
            if (isValidLine() && _word != _line->words().begin())
            {
                --_word;
                toFront(Spine::ElementCharacter, false);
                return &*_word;
            }
            if (limit_ > Spine::WithinLine)
            {
                bool end = false;
                do
                {
                    end = previousLine(limit_) == 0;
                }
                while (!end && isValidBlock() && !isValidWord(Spine::WithinLine));
                if (!end && isValidWord())
                {
                    toBack(Spine::ElementWord, false);
                    --_word;
                    toFront(Spine::ElementCharacter, false);
                    return &*_word;
                }
            }
            return 0;
        }

        const Spine::Character * previousCharacter(Spine::IterateLimit limit_ = Spine::WithinWord)
        {
            if (limit_ < Spine::WithinWord) return 0;
            if (isValidWord() && _character != _word->characters().begin())
            {
                --_character;
                return &*_character;
            }
            if (limit_ > Spine::WithinWord)
            {
                bool end = false;
                do
                {
                    end = previousWord(limit_) == 0;
                }
                while (!end && isValidLine() && !isValidCharacter(Spine::WithinWord));
                if (!end && isValidCharacter())
                {
                    toBack(Spine::ElementCharacter, false);
                    --_character;
                    return &*_character;
                }
            }
            return 0;
        }

        /*******************************************************************************************
         *  The to* methods are required to advance to the front or back of an element list.
         *******************************************************************************************/
        inline void toBack(Spine::DocumentElement element_)
        {
            toBack(element_, true);
        }

        void toBack(Spine::DocumentElement element_, bool validate_)
        {
            switch (element_)
            {
            case Spine::ElementCharacter: if (!validate_ || isValidWord()) _character=_word->characters().end(); break;
            case Spine::ElementWord: if (!validate_ || isValidLine()) _word=_line->words().end(); break;
            case Spine::ElementLine: if (!validate_ || isValidBlock()) _line=_block->lines().end(); break;
            case Spine::ElementBlock: if (!validate_ || isValidRegion()) _block=_region->blocks().end(); break;
            case Spine::ElementRegion: if (!validate_ || isValidPage()) _region=_page->regions().end(); break;
            case Spine::ElementImage: if (!validate_ || isValidPage()) _image=_page->images().end(); break;
            case Spine::ElementPage: if (!validate_ || isValidDocument()) _page=_doc->end(); break;
            }
        }

        inline void toFront(Spine::DocumentElement element_)
        {
            toFront(element_, true);
        }

        void toFront(Spine::DocumentElement element_, bool validate_)
        {
            if (validate_)
            {
                // Sanitise
                switch (element_)
                {
                case Spine::ElementCharacter: if (!isValidWord()) return; break;
                case Spine::ElementWord: if (!isValidLine()) return; break;
                case Spine::ElementLine: if (!isValidBlock()) return; break;
                case Spine::ElementBlock: if (!isValidRegion()) return; break;
                case Spine::ElementRegion: if (!isValidPage()) return; break;
                case Spine::ElementImage: if (!isValidPage()) return; break;
                case Spine::ElementPage: if (!isValidDocument()) return; break;
                }
            }

            // Reset
            switch (element_)
            {
            case Spine::ElementPage: _page = _doc->begin();
            case Spine::ElementImage: _image = _page->images().begin();
            case Spine::ElementRegion:
                if (_page == _doc->end()) break;
                _region = _page->regions().begin();
            case Spine::ElementBlock:
                if (_region == _page->regions().end()) break;
                _block = _region->blocks().begin();
            case Spine::ElementLine:
                if (_block == _region->blocks().end()) break;
                _line = _block->lines().begin();
            case Spine::ElementWord:
                if (_line == _block->lines().end()) break;
                _word = _line->words().begin();
            case Spine::ElementCharacter:
                if (_word == _line->words().end()) break;
                _character = _word->characters().begin();
            }
        }

        bool operator == (Cursor &rhs_)
        {
            // if tags don't match then cannot be equal
            if(std::string(type()) != rhs_.type()) {
                return false;
            }

            PDFCursor * other = reinterpret_cast<PDFCursor *>(&rhs_);

            // Valid Document
            if (_doc != 0 && _doc == other->_doc)
            {
                bool equal = (_page == other->_page);
                if (_page != _doc->end())
                {
                    equal &= (_image == other->_image);
                    equal &= (_region == other->_region);
                    if (_region != _page->regions().end())
                    {
                        equal &= (_block == other->_block);
                        if (_block != _region->blocks().end())
                        {
                            equal &= (_line == other->_line);
                            if (_line != _block->lines().end())
                            {
                                equal &= (_word == other->_word);
                                if (_word != _line->words().end())
                                {
                                    equal &= (_character == other->_character);
                                }
                            }
                        }
                    }
                }
                return equal;
            }
            else
            {
                return false;
            }
        }

        bool operator < (Cursor &rhs_)
        {
            // if tags don't match then cannot be equal
            if(std::string(type()) != rhs_.type()) {
                return false;
            }

            PDFCursor * other = reinterpret_cast<PDFCursor *>(&rhs_);

            // Valid Document
            if (_doc != 0 && _doc == other->_doc)
            {
                if (_page < other->_page) return true;
                if (_page == other->_page && _page != _doc->end())
                {
                    if (_region < other->_region) return true;
                    if (_region == other->_region && _region != _page->regions().end())
                    {
                        if (_block < other->_block) return true;
                        if (_block == other->_block && _block != _region->blocks().end())
                        {
                            if (_line < other->_line) return true;
                            if (_line == other->_line && _line != _block->lines().end())
                            {
                                if (_word < other->_word) return true;
                                if (_word == other->_word && _word != _line->words().end())
                                {
                                    return _character < other->_character;
                                }
                            }
                        }
                    }
                }
            }

            return false;
        }

        /************************************************************************/

        CursorHandle clone()
        {
            return CursorHandle(new PDFCursor(*this));
        }

        virtual std::string repr()
        {
            std::stringstream str;

            str << "PDFCursor(";
            str << _doc;
            if (_doc)
            {
                str << " p" << (_page - _doc->begin());
                if (_page != _doc->end())
                {
                    str << " i" << (_image - _page->images().begin());
                    str << " r" << (_region - _page->regions().begin());
                    if (_region != _page->regions().end())
                    {
                        str << " b" << (_block - _region->blocks().begin());
                        if (_block != _region->blocks().end())
                        {
                            str << " l" << (_line - _block->lines().begin());
                            if (_line != _block->lines().end())
                            {
                                str << " w" << (_word - _line->words().begin());
                                if (_word != _line->words().end())
                                {
                                    str << " c" << (_character - _word->characters().begin());
                                }
                                else
                                {
                                    str << " c-";
                                }
                            }
                            else
                            {
                                str << " w- c-";
                            }
                        }
                        else
                        {
                            str << " l- w- c-";
                        }
                    }
                    else
                    {
                        str << " b- l- w- c-";
                    }
                }
                else
                {
                    str << " r- b- l- w- c-";
                }
            }
            else
            {
                str << " p- r- b- l- w- c-";
            }
            str << ")";
            return str.str();
        }

        /************************************************************************/

    protected:

        friend class Crackle::PDFDocument;

        inline PDFCursor(Crackle::PDFDocument * doc_, int page_ = 1)
            : _doc(doc_)
        {
            this->gotoPage(page_);
        }

        Crackle::PDFDocument * _doc;
        page_iterator _page;
        image_iterator _image;
        region_iterator _region;
        block_iterator _block;
        line_iterator _line;
        word_iterator _word;
        character_iterator _character;
        font_iterator _font;
    };

}

#endif /* PDFCURSOR_INCL_ */
