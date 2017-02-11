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

#ifndef CHARACTER_INCL_
#define CHARACTER_INCL_

/*****************************************************************************
 *
 * Character.h
 *
 * Copyright 2008 Advanced Interfaces Group
 *
 ****************************************************************************/

#include <spine/BoundingBox.h>
#include <spine/Color.h>
#include <string>
#include <utf8/unicode.h>

namespace Spine
{
    class Character
    {
    public:

        virtual ~Character() {}
        virtual BoundingBox boundingBox() const=0;
        virtual std::string fontName() const=0;
        virtual double fontSize() const=0;
        virtual int rotation() const=0;
        virtual bool spaceAfter() const=0;
        virtual bool underlined() const=0;
        virtual double baseline() const=0;
        virtual Color color() const=0;
        virtual utf8::uint32_t charcode() const=0;
        virtual std::string text() const;
    };

}

#endif /* CHARACTER_INCL_ */
