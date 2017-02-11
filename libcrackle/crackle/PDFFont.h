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

#ifndef PDFFONT_INCL_
#define PDFFONT_INCL_

/*****************************************************************************
 *
 * PDFFont.h
 *
 * Copyright 2008 Advanced Interfaces Group
 *
 ****************************************************************************/
#include <string>
#include <map>

class GfxFont;
class CrackleTextPage;

namespace Crackle
{

    class PDFPage;
    class PDFTextWord;
    class PDFTextCharacter;
    class PDFDocument;
    class PDFFontCollection;

    class PDFFont
    {
    public:

        typedef std::map<double, int> FontSizes;

        // two fonts are equivalent if their tags match
        bool operator==(const PDFFont &rhs_) const;
        bool operator!=(const PDFFont &rhs_) const;
        int operator <(const PDFFont &rhs_) const;
        std::string name() const;
        std::string tag() const;

        // note these are not reliable
        bool isFixedWidth() const;
        bool isSerif() const;
        bool isSymbolic() const;
        bool isItalic() const;
        bool isBold() const;

        // occurences and sizes are helper methods, not
        // part of the font definition itself and hence
        // not considered for equality
        int occurences() const;
        const FontSizes &sizes() const;

    private:

        PDFFont (GfxFont *gfxfont_, const FontSizes &sizes_=FontSizes());
        void updateSizes(float size_, int increase_=1);
        void updateSizes(const FontSizes &sizes_);

        friend class Crackle::PDFPage;
        friend class Crackle::PDFDocument;
        friend class Crackle::PDFTextWord;
        friend class Crackle::PDFTextCharacter;
        friend class Crackle::PDFFontCollection;
        friend class ::CrackleTextPage;

        bool _isFixedWidth;
        bool _isSerif;
        bool _isSymbolic;
        bool _isItalic;
        bool _isBold;
        std::string _name;
        std::string _tag;
        int _count;
        FontSizes _sizes;
    };

}

#endif /* PDFFONT_INCL_ */
