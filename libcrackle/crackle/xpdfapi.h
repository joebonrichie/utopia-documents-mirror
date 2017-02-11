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

#ifndef XPDFAPI_INCL_
#define XPDFAPI_INCL_

/*****************************************************************************
 *
 * xpdfapi.h
 *
 * Copyright 2009 Advanced Interfaces Group
 *
 ****************************************************************************/

#include "aconf.h"
#include "GString.h"
#include "PDFDocEncoding.h"
#include <string>
#include <utf8/unicode.h>

namespace Crackle {

    inline std::string gstring2UnicodeString(GString *str_)
    {
        uint16_t u;
        std::string result;

        // this could be sped up by making a utf16 iterator over the gstring
        std::vector<utf8::uint16_t> data;

        if(str_) {

            if (str_->getLength() >=4 &&
                (str_->getChar(0) & 0xff) == 0xfe && (str_->getChar(1) & 0xff) == 0xff) {

                // Unicode utf-16characters
                for(int i = 2; i < str_->getLength(); i+=2) {
                    u= ((str_->getChar(i) & 0xff) << 8) | (str_->getChar(i+1) & 0xff);
                    data.push_back(u);
                }
            } else {
                // single byte characters
                for(int i = 0; i < str_->getLength(); ++i) {
                    size_t ch(str_->getChar(i) & 0xff);
                    if(ch==0 || ch==9 || ch==10 || ch==12 || ch==13) {
                        u=ch;
                    } else {
                        u = pdfDocEncoding[ch];
                        if(u==0) {
                            // map undefined characters to Unicode replacement character
                            u=0xfffd;
                        }
                    }

                    data.push_back(u);
                }
            }

            try {

                std::string tmp;
                utf8::utf16to8(data.begin(), data.end(), back_inserter(tmp));
                utf8::normalize_utf8(tmp.begin(), tmp.end(), back_inserter(result));

            } catch (utf8::exception e) {
                // error decoding string
            }

        }

        return result;
    }

    inline std::string unicode2UnicodeString(Unicode *str_, int len_)
    {
        std::string result;

        try {
            std::string tmp;
            utf8::utf32to8(str_, str_+len_, back_inserter(tmp));
            utf8::normalize_utf8(tmp.begin(), tmp.end(), back_inserter(result));

        } catch (utf8::exception e) {
            // error decoding string
        }

        return result;
    }

}
#endif /* XPDFAPI_INCL_ */
