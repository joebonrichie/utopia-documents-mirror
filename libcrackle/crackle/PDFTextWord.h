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

#ifndef PDFTEXTWORD_INCL_
#define PDFTEXTWORD_INCL_

/*****************************************************************************
 *
 * PDFTextWord.h
 *
 * Copyright 2008 Advanced Interfaces Group
 *
 ****************************************************************************/

#include <crackle/SimpleCollection.h>
#include <crackle/PDFTextCharacter.h>
#include <crackle/PDFFont.h>
#include <spine/Color.h>
#include <spine/Word.h>
#include <spine/BoundingBox.h>
#include <string>

class CrackleTextWord;

namespace Crackle
{

    class PDFTextWord : public Spine::Word
    {
    public:

        PDFTextWord(const PDFTextWord &rhs_);
        PDFTextWord& operator=(const PDFTextWord &rhs_);
        virtual ~PDFTextWord();
        bool operator==(const PDFTextWord&rhs_) const;

        const SimpleCollection<PDFTextCharacter> &characters() const;
        Spine::BoundingBox boundingBox() const;
        std::string fontName() const;
        double fontSize() const;
        int rotation() const;
        bool spaceAfter() const;
        bool underlined() const;
        double baseline() const;
        Spine::Color color() const;
        std::string text() const;

    private:

        typedef CrackleTextWord wrapped_class;

        PDFTextWord(wrapped_class *word_);
        friend class SimpleCollection<PDFTextCharacter>;
        friend class SimpleCollection<PDFTextWord>;

        void advance();

        mutable wrapped_class *_word;
        mutable SimpleCollection<PDFTextCharacter> *_characters;
    };

}


#endif /* PDFTEXTWORD_INCL_ */
