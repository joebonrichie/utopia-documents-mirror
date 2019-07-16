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

/*****************************************************************************
 *
 * PDFFont.cpp
 *
 * Copyright 2008 Advanced Interfaces Group
 *
 ****************************************************************************/

#include <crackle/PDFFont.h>
#include <crackle/xpdfapi.h>

#include "aconf.h"
#include "goo/GString.h"
#include "GfxFont.h"

#include <cstring>

using namespace Crackle;
using namespace std;

bool PDFFont::operator==(const PDFFont &rhs_) const
{
    return this->name()==rhs_.name();
}

bool PDFFont::operator!=(const PDFFont &rhs_) const
{
    return !(*this == rhs_);
}

int PDFFont::operator <(const PDFFont &rhs_) const
{
    return this->occurences() < rhs_.occurences();
}

PDFFont::PDFFont (GfxFont *gfxfont_, const FontSizes &sizes_)
    :_sizes(sizes_)
{
    // name
    const GString *nm(gfxfont_->getName());

    if(!nm) {
        nm=gfxfont_->getEmbeddedFontName();
    }

    if(!nm) {
        nm=gfxfont_->getTag();
    }

    if(nm) {
        _name=gstring2UnicodeString(nm);
    }

    // tag

    const GString *tag(gfxfont_->getTag());
    if(tag) {
        _tag=gstring2UnicodeString(tag);
    }

    // flags
    _isFixedWidth=gfxfont_->isFixedWidth()!=false;
    _isSerif=gfxfont_->isSerif()!=false;
    _isSymbolic=gfxfont_->isSymbolic()!=false;
    _isItalic=gfxfont_->isItalic()!=false;
    _isBold=gfxfont_->isBold()!=false;


}

std::string PDFFont::name() const
{
    return _name;
}

std::string PDFFont::tag() const
{
    return _tag;
}

bool PDFFont::isFixedWidth() const
{
    return _isFixedWidth;
}

bool PDFFont::isSerif() const
{
    return _isSerif;
}

bool PDFFont::isSymbolic() const
{
    return _isSymbolic;
}

bool PDFFont::isItalic() const
{
    return _isItalic;
}

bool PDFFont::isBold() const
{
    return _isBold;
}

const PDFFont::FontSizes &PDFFont::sizes() const
{
    return _sizes;
}

int PDFFont::occurences() const
{
    int result(0);

    for(FontSizes::const_iterator i(_sizes.begin()); i!=_sizes.end(); ++i) {
        result+=i->second;
    }
    return result;
}

void PDFFont::updateSizes(float size_, int increase_)
{
    _sizes[size_]+=increase_;
}

void PDFFont::updateSizes(const FontSizes &sizes_)
{
    for(FontSizes::const_iterator i(sizes_.begin()); i!=sizes_.end(); ++i) {
        _sizes[i->first]+=i->second;
    }
}
