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

#ifndef PDFTEXTCHARACTER_INCL_
#define PDFTEXTCHARACTER_INCL_

/*****************************************************************************
 *
 * PDFTextCharacter.h
 *
 * Copyright 2008 Advanced Interfaces Group
 *
 ****************************************************************************/

#include <spine/BoundingBox.h>
#include <spine/Character.h>
#include <spine/Color.h>
#include <crackle/SimpleCollection.h>
#include <crackle/PDFFont.h>
#include <string>
#include <utf8/unicode.h>

class CrackleTextWord;


namespace Crackle
{

    class PDFTextCharacter : public Spine::Character
    {
    public:

        PDFTextCharacter(const PDFTextCharacter &rhs_);
        PDFTextCharacter& operator=(const PDFTextCharacter &rhs_);
        virtual ~PDFTextCharacter();
        bool operator==(const PDFTextCharacter&rhs_) const;

        Spine::BoundingBox boundingBox() const;
        PDFFont font() const;
        std::string fontName() const;
        double fontSize() const;
        int rotation() const;
        bool spaceAfter() const;
        bool underlined() const;
        double baseline() const;
        Spine::Color color() const;
        utf8::uint32_t charcode() const;

    private:

        typedef CrackleTextWord wrapped_class;

        PDFTextCharacter(wrapped_class *word_, int idx_ = 0);
        friend class SimpleCollection<PDFTextCharacter>;
        friend class PDFTextWord;

        void advance();

        mutable wrapped_class *_word;
        mutable int _idx;
    };

}

#endif /* PDFTEXTCHARACTER_INCL_ */

#include <crackle/SimpleCollection.h>
