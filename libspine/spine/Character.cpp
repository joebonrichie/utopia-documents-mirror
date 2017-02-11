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

/*****************************************************************************
 *
 * Character.cpp
 *
 * Copyright 2012 Advanced Interfaces Group
 *
 ****************************************************************************/

#include "Character.h"

using namespace std;

namespace Spine {

    std::string Character::text() const
    {
        try {
            string tmp;
            utf8::append(this->charcode(), std::back_inserter(tmp));

            string result;
            normalize_utf8(tmp.begin(), tmp.end(), std::back_inserter(result), utf8::NFKC);

            return result;
        }
        catch (utf8::exception e) {
            return "";
        }
    }

}
