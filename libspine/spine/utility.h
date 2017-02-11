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

#ifndef UTILITY_INCL_
#define UTILITY_INCL_

/*****************************************************************************
 *
 * utility.h
 *
 * Copyright 2008 Advanced Interfaces Group
 *
 ****************************************************************************/

#include <spine/Cursor.h>
#include <spine/Line.h>
#include <spine/Page.h>

#include <algorithm>
#include <locale>
#include <iostream>

namespace Spine {

    struct title_case
    {
        title_case() : _seenSpace(true)
        {}

        wchar_t operator()(const wchar_t& char_)
        {
            wchar_t result;

            if(_seenSpace) {
                result=std::towupper(char_);
            } else {
                result=std::towlower(char_);
            }

            if(std::iswspace(char_)) {
                _seenSpace=true;
            } else {
                _seenSpace=false;
            }

            return result;
        }

        bool _seenSpace;
    };

    // header size - to be cropped when considering body
    const double vmargin(40);

    inline bool advance_to_body(CursorHandle cursor, IterateLimit limit_=WithinPage) {
        while(cursor->line()) {
            const Page * page = cursor->page();
            const Line * line = cursor->line();
            if((line->boundingBox().y1 > page->boundingBox().y1 + vmargin ) &&
               (line->boundingBox().y2 < page->boundingBox().y2-vmargin)){
                return true;
            }
            cursor->nextLine(limit_);
        }
        return false;
    }

}

#endif /* UTILITY_INCL_ */
