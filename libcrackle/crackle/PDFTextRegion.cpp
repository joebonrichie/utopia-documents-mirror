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
 * PDFTextRegion.cpp
 *
 * Copyright 2008 Advanced Interfaces Group
 *
 ****************************************************************************/

#include <crackle/PDFTextRegion.h>
#include <crackle/CrackleTextOutputDev.h>
#include <crackle/PDFTextBlockCollection.h>

using namespace std;
using namespace Spine;
using namespace Crackle;

Crackle::PDFTextRegion::PDFTextRegion(wrapped_class *flow_)
    : _flow(flow_), _blocks(0)
{}

Crackle::PDFTextRegion::PDFTextRegion(const PDFTextRegion &rhs_)
    : _flow(rhs_._flow), _blocks(0)
{}

Crackle::PDFTextRegion& Crackle::PDFTextRegion::operator=(const Crackle::PDFTextRegion &rhs_)
{
    if(&rhs_!=this) {
        delete _blocks;
        _blocks=0;
        _flow=rhs_._flow;
    }
    return *this;
}

Crackle::PDFTextRegion::~PDFTextRegion()
{
    delete _blocks;
}

const PDFTextBlockCollection &Crackle::PDFTextRegion::blocks() const
{
    if(!_blocks) {
        _blocks=new PDFTextBlockCollection(_flow->getBlocks());
    }
    return *_blocks;
}

BoundingBox Crackle::PDFTextRegion::boundingBox() const
{
    BoundingBox b;
    _flow->getBBox(&b.x1,&b.y1,&b.x2,&b.y2);
    return b;
}

bool Crackle::PDFTextRegion::operator==(const PDFTextRegion &rhs_) const
{
    return _flow==rhs_._flow;
}

void Crackle::PDFTextRegion::advance()
{
    delete _blocks;
    _blocks=0;
    _flow=_flow->getNext();
}

string Crackle::PDFTextRegion::text() const
{
    string result;
    PDFTextBlockCollection::const_iterator i;
    for (i=blocks().begin(); i!=blocks().end(); ++i) {
        result+=i->text() + "\n";
    }
    return result;
}

int Crackle::PDFTextRegion::rotation() const
{
    // region is not of uniform rotation
    return 0;
}
