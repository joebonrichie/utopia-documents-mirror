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
 * PDFTextBlock.cpp
 *
 * Copyright 2008 Advanced Interfaces Group
 *
 ****************************************************************************/

#include <crackle/PDFTextBlock.h>
#include <crackle/PDFTextLineCollection.h>
#include <crackle/PDFTextWordCollection.h>
#include <crackle/CrackleTextOutputDev.h>

using namespace std;
using namespace Spine;
using namespace Crackle;

Crackle::PDFTextBlock::PDFTextBlock(const PDFTextBlock &rhs_)
    : _block(rhs_._block), _lines(0)
{}

Crackle::PDFTextBlock& Crackle::PDFTextBlock::operator=(const Crackle::PDFTextBlock &rhs_)
{
    if(&rhs_!=this) {
        delete _lines;
        _lines=0;
        _block=rhs_._block;
    }
    return *this;
}

Crackle::PDFTextBlock::~PDFTextBlock()
{
    delete _lines;
}

BoundingBox Crackle::PDFTextBlock::boundingBox() const
{
    BoundingBox b;
    _block->getBBox(&b.x1,&b.y1,&b.x2,&b.y2);
    return b;
}

int Crackle::PDFTextBlock::rotation() const
{
    return (_block->getRotation());
}

Crackle::PDFTextBlock::PDFTextBlock(wrapped_class *block_)
    : _block(block_), _lines(0)
{}

bool Crackle::PDFTextBlock::operator==(const PDFTextBlock &rhs_) const
{
    return _block==rhs_._block;
}

const PDFTextLineCollection &Crackle::PDFTextBlock::lines() const
{
    if(!_lines) {
        _lines=new PDFTextLineCollection(_block->getLines());
    }
    return *_lines;
}

void Crackle::PDFTextBlock::advance()
{
    delete _lines;
    _lines=0;
    _block=_block->getNext();
}

string Crackle::PDFTextBlock::text() const
{
    string text;

    PDFTextLineCollection::const_iterator l(this->lines().begin());
    PDFTextWordCollection::const_iterator w;
    if(l!=this->lines().end()) {
        w=l->words().begin();
    }

    while(l!=this->lines().end()) {
        while(w!=l->words().end()) {
            bool sp(w->spaceAfter());
            text+=w->text();
            if(sp) {
                text+= " ";
            }
            ++w;
        }

        bool hyphenated(l->hyphenated());

        ++l;
        if(l!=this->lines().end()) {
            w=l->words().begin();
            if(hyphenated && w!=l->words().end()) {
                text.resize(text.length()-1);
                text+=w->text();
                ++w;
            }
        }
        text+=" "; // lines are joined with spaces
    }
    return text;
}
