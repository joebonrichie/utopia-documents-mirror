/*****************************************************************************
 *  
 *   This file is part of the Utopia Documents application.
 *       Copyright (c) 2008-2017 Lost Island Labs
 *           <info@utopiadocs.com>
 *   
 *   Utopia Documents is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU GENERAL PUBLIC LICENSE VERSION 3 as
 *   published by the Free Software Foundation.
 *   
 *   Utopia Documents is distributed in the hope that it will be useful, but
 *   WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 *   Public License for more details.
 *   
 *   In addition, as a special exception, the copyright holders give
 *   permission to link the code of portions of this program with the OpenSSL
 *   library under certain conditions as described in each individual source
 *   file, and distribute linked combinations including the two.
 *   
 *   You must obey the GNU General Public License in all respects for all of
 *   the code used other than OpenSSL. If you modify file(s) with this
 *   exception, you may extend this exception to your version of the file(s),
 *   but you are not obligated to do so. If you do not wish to do so, delete
 *   this exception statement from your version.
 *   
 *   You should have received a copy of the GNU General Public License
 *   along with Utopia Documents. If not, see <http://www.gnu.org/licenses/>
 *  
 *****************************************************************************/

#ifndef UNICODE_INCL_
#define UNICODE_INCL_

#include <cstdlib>
#include <vector>
#include <exception>
#include <stdint.h>

#include "utf8proc/utf8proc.h"

// import c++ iterators (already in utf8 namespace)
#include "utf8cpp/source/utf8.h"

namespace utf8 {

    class invalid_normalization : public exception {
    public:
        virtual const char* what() const throw() { return "Invalid normalization"; }
    };

    enum unicode_decomposition { NFD, NFC, NFKD, NFKC };

    template <typename octet_iterator, typename output_iterator>
    output_iterator normalize_utf8(octet_iterator start, octet_iterator end,
                                   output_iterator out,
                                   unicode_decomposition option=NFKC)
    {
        // default flags
        unsigned int opt= UTF8PROC_STABLE | UTF8PROC_IGNORE | UTF8PROC_STRIPCC;

        // set composition and compatibility flags
        if(option==NFD || option==NFKD) {
            opt |= UTF8PROC_DECOMPOSE;
        } else {
            opt |= UTF8PROC_COMPOSE;
        }

        if (option==NFKD || option==NFKC) {
            opt |= UTF8PROC_COMPAT;
        }

        std::vector<uint8_t> src(start, end);
        uint8_t *tmp, *i;

        ssize_t length(utf8proc_map(&src[0], src.size(), &tmp, utf8proc_option_t(opt)));

        if(length < 0) {
            switch (length) {
            case UTF8PROC_ERROR_NOMEM:
                throw utf8::not_enough_room();
                break;
            default:
                throw utf8::invalid_normalization();
                break;
            }
        } else {
            // copy result to output iterator
            for(i=tmp; (*i); ++i) {
                *out++=*i;
            }
            std::free(tmp);
        }
        return out;
    }

    typedef uint32_t UChar32;

    ssize_t utf8_advance_char(const uint8_t ** bytes);

}

#endif /* UNICODE_INCL_ */
