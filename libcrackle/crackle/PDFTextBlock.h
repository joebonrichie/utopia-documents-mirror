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

#ifndef PDFTEXTBLOCK_INCL_
#define PDFTEXTBLOCK_INCL_

/*****************************************************************************
 *
 * PDFTextBlock.h
 *
 * Copyright 2008 Advanced Interfaces Group
 *
 ****************************************************************************/

#include <crackle/SimpleCollection.h>
#include <crackle/PDFTextLine.h>
#include <spine/BoundingBox.h>
#include <spine/Block.h>
#include <string>

class CrackleTextBlock;

namespace Crackle
{

    class PDFTextBlock : public Spine::Block
    {
    public:

        PDFTextBlock(const PDFTextBlock &rhs_);
        PDFTextBlock& operator=(const PDFTextBlock &rhs_);
        virtual ~PDFTextBlock();
        bool operator==(const PDFTextBlock&rhs_) const;

        const SimpleCollection<PDFTextLine> &lines() const;
        Spine::BoundingBox boundingBox() const;
        std::string text() const;
        int rotation() const;

    private:

        typedef CrackleTextBlock wrapped_class;

        PDFTextBlock(wrapped_class *block_);
        friend class SimpleCollection<PDFTextBlock>;

        void advance();

        mutable wrapped_class *_block;
        mutable SimpleCollection<PDFTextLine> *_lines;
    };

}


#endif /* PDFTEXTBLOCK_INCL_ */
