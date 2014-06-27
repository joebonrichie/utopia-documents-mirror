/*****************************************************************************
 *  
 *   This file is part of the libcrackle library.
 *       Copyright (c) 2008-2014 Lost Island Labs
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
 * crackleapi.cpp
 *
 * Copyright 2009 Advanced Interfaces Group
 *
 ****************************************************************************/

#include <crackle/crackleapi.h>
#include <crackle/crackle.h>

#include <spine/spineapi_internal.h>

using namespace Spine;
using namespace Crackle;

SpineDocument new_CrackleDocument(const char *filename, SpineError *error)
{
    SpineDocument result=new SpineDocumentImpl;
    result->_handle=Spine::DocumentHandle(new PDFDocument (filename));

    if(!SpineDocument_valid(result, error)) {
        *error=SpineError_IO;
    }
    return result;
}
