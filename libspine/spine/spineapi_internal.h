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

#ifndef SPINEAPI_INTERNAL_INCL_
#define SPINEAPI_INTERNAL_INCL_

/*****************************************************************************
 *
 * spineapi_internal.h
 *
 * Copyright 2009 Advanced Interfaces Group
 *
 ****************************************************************************/

struct SpineDocumentImpl {
    Spine::DocumentHandle _handle;
};

struct SpineAnnotationImpl {
    Spine::AnnotationHandle _handle;
};

struct SpineCursorImpl {
    Spine::CursorHandle _handle;
};

struct SpineTextExtentImpl {
    Spine::TextExtentHandle _handle;
};

struct SpineImageImpl {
    Spine::Image _handle;
};

#endif /* SPINEAPI_INTERNAL_INCL_ */
