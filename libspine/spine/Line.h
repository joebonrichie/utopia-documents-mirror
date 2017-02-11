/*****************************************************************************
 *  
 *   This file is part of the libspine library.
 *       Copyright (c) 2008-2017 Lost Island Labs
 *           <info@utopiadocs.com>
 *   
 *   The libspine library is free software: you can redistribute it and/or
 *   modify it under the terms of the GNU AFFERO GENERAL PUBLIC LICENSE
 *   VERSION 3 as published by the Free Software Foundation.
 *   
 *   The libspine library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero
 *   General Public License for more details.
 *   
 *   You should have received a copy of the GNU Affero General Public License
 *   along with the libspine library. If not, see
 *   <http://www.gnu.org/licenses/>
 *  
 *****************************************************************************/

#ifndef LINE_INCL_
#define LINE_INCL_

/*****************************************************************************
 *
 * Line.h
 *
 * Copyright 2008 Advanced Interfaces Group
 *
 ****************************************************************************/

#include <spine/BoundingBox.h>
#include <string>

namespace Spine
{

    class Line
    {
    public:

        virtual ~Line() {}
        virtual BoundingBox boundingBox() const=0;
        virtual bool hyphenated() const=0;
        virtual int rotation() const=0;
        virtual std::string text() const=0;

    };

}

#endif /* LINE_INCL_ */
