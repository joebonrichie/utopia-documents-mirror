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

#ifndef PDFTEXTREGION_INCL_
#define PDFTEXTREGION_INCL_

/*****************************************************************************
 *
 * PDFTextRegion.h
 *
 * Copyright 2008 Advanced Interfaces Group
 *
 ****************************************************************************/

#include <crackle/SimpleCollection.h>
#include <crackle/PDFTextBlock.h>
#include <spine/BoundingBox.h>
#include <spine/Region.h>
#include <string>

class CrackleTextFlow;

namespace Crackle
{

    class PDFTextRegion : public Spine::Region
    {
    public:

        PDFTextRegion(const PDFTextRegion &rhs_);
        PDFTextRegion& operator=(const PDFTextRegion &rhs_);
        virtual ~PDFTextRegion();
        bool operator==(const PDFTextRegion&rhs_) const;

        const SimpleCollection<PDFTextBlock> &blocks() const;
        Spine::BoundingBox boundingBox() const;
        std::string text() const;
        int rotation() const;

    private:

        typedef CrackleTextFlow wrapped_class;

        PDFTextRegion(wrapped_class *flow_);
        friend class SimpleCollection<PDFTextRegion>;

        void advance();

        mutable wrapped_class *_flow;
        mutable SimpleCollection<PDFTextBlock> *_blocks;
    };

}


#endif /* PDFTEXTREGION_INCL_ */
