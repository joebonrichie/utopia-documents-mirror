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
 * PDFTextLine.cpp
 *
 * Copyright 2008 Advanced Interfaces Group
 *
 ****************************************************************************/

#include <crackle/PDFTextLine.h>
#include <crackle/CrackleTextOutputDev.h>
#include <crackle/PDFTextWordCollection.h>
#include <crackle/xpdfapi.h>

using namespace std;
using namespace Spine;
using namespace Crackle;

Crackle::PDFTextLine::PDFTextLine(const PDFTextLine &rhs_)
    : _line(rhs_._line), _words(0)
{}

Crackle::PDFTextLine& Crackle::PDFTextLine::operator=(const Crackle::PDFTextLine &rhs_)
{
    if(&rhs_!=this) {
        delete _words;
        _words=0;
        _line=rhs_._line;
    }
    return *this;
}

Crackle::PDFTextLine::~PDFTextLine()
{
    delete _words;
}

BoundingBox Crackle::PDFTextLine::boundingBox() const
{
    BoundingBox b;
    _line->getBBox(&b.x1,&b.y1,&b.x2,&b.y2);
    return b;
}

bool Crackle::PDFTextLine::hyphenated() const
{
    return (_line->isHyphenated()!=gFalse);
}

int Crackle::PDFTextLine::rotation() const
{
    return (_line->getRotation());
}

Crackle::PDFTextLine::PDFTextLine(wrapped_class *flow_)
    : _line(flow_), _words(0)
{}

bool Crackle::PDFTextLine::operator==(const PDFTextLine &rhs_) const
{
    return _line==rhs_._line;
}

const Crackle::PDFTextWordCollection &Crackle::PDFTextLine::words() const
{
    if(!_words) {
        _words=new PDFTextWordCollection(_line->getWords());
    }
    return *_words;
}

void Crackle::PDFTextLine::advance()
{
    delete _words;
    _words=0;
    _line=_line->getNext();
}

string Crackle::PDFTextLine::text() const
{
    return unicode2UnicodeString(_line->getText(), _line->getLength());
}
