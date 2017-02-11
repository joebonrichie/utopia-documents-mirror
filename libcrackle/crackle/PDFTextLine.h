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

#ifndef PDFTEXTLINE_INCL_
#define PDFTEXTLINE_INCL_

/*****************************************************************************
 *
 * PDFTextLine.h
 *
 * Copyright 2008 Advanced Interfaces Group
 *
 ****************************************************************************/

#include <crackle/SimpleCollection.h>
#include <crackle/PDFTextWord.h>
#include <spine/BoundingBox.h>
#include <spine/Line.h>
#include <string>

class CrackleTextLine;

namespace Crackle
{

    class PDFTextLine : public Spine::Line
    {
    public:

        PDFTextLine(const PDFTextLine &rhs_);
        PDFTextLine& operator=(const PDFTextLine &rhs_);
        virtual ~PDFTextLine();
        bool operator==(const PDFTextLine&rhs_) const;

        const SimpleCollection<PDFTextWord> &words() const;
        Spine::BoundingBox boundingBox() const;
        bool hyphenated() const;
        int rotation() const;
        std::string text() const;

    private:

        typedef CrackleTextLine wrapped_class;

        PDFTextLine(wrapped_class *line_);

        friend class SimpleCollection<PDFTextLine>;
        void advance();

        mutable wrapped_class *_line;
        mutable SimpleCollection<PDFTextWord> *_words;
    };

}

#endif /* PDFTEXTLINE_INCL_ */
