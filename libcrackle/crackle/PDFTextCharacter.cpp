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
 * PDFTextCharacter.cpp
 *
 * Copyright 2008 Advanced Interfaces Group
 *
 ****************************************************************************/

#include <crackle/PDFTextCharacter.h>
#include <crackle/CrackleTextOutputDev.h>

using namespace Spine;
using namespace Crackle;
using namespace std;

Crackle::PDFTextCharacter::PDFTextCharacter(const PDFTextCharacter &rhs_)
    : _word(rhs_._word), _idx(rhs_._idx)
{}

Crackle::PDFTextCharacter& Crackle::PDFTextCharacter::operator=(const Crackle::PDFTextCharacter &rhs_)
{
    if(&rhs_!=this) {
        _word=rhs_._word;
        _idx=rhs_._idx;
    }
    return *this;
}

Crackle::PDFTextCharacter::~PDFTextCharacter()
{
}

BoundingBox Crackle::PDFTextCharacter::boundingBox() const
{
    BoundingBox b;
    _word->getCharBBox(_idx, &b.x1,&b.y1,&b.x2,&b.y2);
    return b;
}

Crackle::PDFTextCharacter::PDFTextCharacter(wrapped_class *word_, int idx_)
    : _word(word_), _idx(idx_)
{}

bool Crackle::PDFTextCharacter::operator==(const PDFTextCharacter &rhs_) const
{
    return _word==rhs_._word && _idx==rhs_._idx;
}

void Crackle::PDFTextCharacter::advance()
{
    ++_idx;
    if(_idx >= _word->getLength()) {
        *this=PDFTextCharacter(0); // so the iterator == end()
    }
}

PDFFont Crackle::PDFTextCharacter::font() const
{
    return PDFFont(_word->getFontInfo()->getFont());
}

double Crackle::PDFTextCharacter::fontSize() const
{
    return _word->getFontSize();
}

string Crackle::PDFTextCharacter::fontName() const
{
    return this->font().name();
}

int Crackle::PDFTextCharacter::rotation() const
{
    return _word->getRotation();
}

bool Crackle::PDFTextCharacter::spaceAfter() const
{
    return (_idx==_word->getLength()-1)
        && (_word->getSpaceAfter()==gTrue);
}

bool Crackle::PDFTextCharacter::underlined() const
{
    return _word->isUnderlined()==gTrue;
}

double Crackle::PDFTextCharacter::baseline() const
{
    return _word->getBaseline();
}

Color Crackle::PDFTextCharacter::color() const
{
    Color result;
    _word->getColor(&result.r,&result.g,&result.b);
    return result;
}

utf8::uint32_t Crackle::PDFTextCharacter::charcode() const
{
    return _word->getChar(_idx);
}
