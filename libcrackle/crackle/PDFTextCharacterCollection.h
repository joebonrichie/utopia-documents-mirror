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

#ifndef PDFTEXTCHARACTERCOLLECTION_INCL_
#define PDFTEXTCHARACTERCOLLECTION_INCL_

/*****************************************************************************
 *
 * PDFTextCharacterCollection.h
 *
 * Copyright 2008 Advanced Interfaces Group
 *
 ****************************************************************************/

#include <crackle/PDFTextCharacter.h>
#include <crackle/PDFTextWord.h>
#include <crackle/SimpleCollection.h>

namespace Crackle
{

    template<>
    class SimpleCollection< PDFTextCharacter > : public std::vector< PDFTextCharacter >
    {
        typedef std::vector< PDFTextCharacter > _Base;

    public:
        SimpleCollection(const std::vector< PDFTextCharacter > & characters)
            : _Base(characters)
            {}

        friend class PDFTextWord;
    };

    typedef SimpleCollection< PDFTextCharacter > PDFTextCharacterCollection;

}

#endif /* PDFTEXTCHARACTERCOLLECTION_INCL_ */
