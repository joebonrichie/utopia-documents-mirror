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

#ifndef CRACKLEAPI_INCL_
#define CRACKLEAPI_INCL_

#include <spine/spineapi.h>

#ifdef __cplusplus
extern "C" {
#endif

    SpineDocument new_CrackleDocument(const char *filename, SpineError *error);
    SpineDocument new_CrackleDocumentFromBuffer(const char *buffer, size_t size, SpineError *error);

#ifdef __cplusplus
}

/* TODO write clone code! */

#endif

#endif /* CRACKLEAPI_INCL_ */
