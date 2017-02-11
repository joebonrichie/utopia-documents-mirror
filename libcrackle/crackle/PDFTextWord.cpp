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
 * PDFTextWord.cpp
 *
 * Copyright 2008 Advanced Interfaces Group
 *
 ****************************************************************************/

#include <crackle/PDFTextWord.h>
#include <crackle/PDFTextCharacterCollection.h>
#include <crackle/CrackleTextOutputDev.h>
#include <crackle/PDFFont.h>
#include <crackle/xpdfapi.h>

using namespace Spine;
using namespace Crackle;
using namespace std;

Crackle::PDFTextWord::PDFTextWord(const PDFTextWord &rhs_)
    : _word(rhs_._word), _characters(0)
{}

Crackle::PDFTextWord& Crackle::PDFTextWord::operator=(const Crackle::PDFTextWord &rhs_)
{
    if(&rhs_!=this) {
        delete _characters;
        _characters=0;
        _word=rhs_._word;
    }
    return *this;
}

Crackle::PDFTextWord::~PDFTextWord()
{
    delete _characters;
}

BoundingBox Crackle::PDFTextWord::boundingBox() const
{
    BoundingBox b;
    _word->getBBox(&b.x1,&b.y1,&b.x2,&b.y2);
    return b;
}

Crackle::PDFTextWord::PDFTextWord(wrapped_class *word_)
    : _word(word_), _characters(0)
{}

bool Crackle::PDFTextWord::operator==(const PDFTextWord &rhs_) const
{
    return _word==rhs_._word;
}

const PDFTextCharacterCollection &Crackle::PDFTextWord::characters() const
{
    if(!_characters) {
        std::vector< PDFTextCharacter > chars;
        for (int i = 0; i < _word->getLength(); ++i) { chars.push_back(PDFTextCharacter(_word, i)); }
        _characters=new PDFTextCharacterCollection(chars);
    }
    return *_characters;
}

void Crackle::PDFTextWord::advance()
{
    delete _characters;
    _characters=0;
    _word=_word->nextWord();
}

double Crackle::PDFTextWord::fontSize() const
{
    return _word->getFontSize();
}

string Crackle::PDFTextWord::fontName() const
{
    return gstring2UnicodeString(_word->getFontInfo()->getFontName());
}

int Crackle::PDFTextWord::rotation() const
{
    return _word->getRotation();
}

bool Crackle::PDFTextWord::spaceAfter() const
{
    return _word->getSpaceAfter()==gTrue;
}

bool Crackle::PDFTextWord::underlined() const
{
    return _word->isUnderlined()==gTrue;
}

double Crackle::PDFTextWord::baseline() const
{
    return _word->getBaseline();
}

string Crackle::PDFTextWord::text() const
{
    return unicode2UnicodeString(_word->getUnicodeText(), _word->getLength());
}

Color Crackle::PDFTextWord::color() const
{
    Color result;
    _word->getColor(&result.r,&result.g,&result.b);
    return result;
}
