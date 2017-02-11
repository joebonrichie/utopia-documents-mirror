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

#ifndef FONTCOLLECTION_INCL_
#define FONTCOLLECTION_INCL_

/*****************************************************************************
 *
 * FontCollection.h
 *
 * Copyright 2008 Advanced Interfaces Group
 *
 ****************************************************************************/

#include <crackle/PDFFont.h>
#include <string>
#include <map>

class GfxFont;
class PDFDoc;
class Dict;

namespace Crackle
{

    class PDFDocument;

    class PDFFontCollection
        : public std::map<std::string, Crackle::PDFFont>
    {

    public:

        PDFFontCollection() {}

    private:

        PDFFontCollection(PDFDoc *doc_);
        void _scanFonts(Dict *resDict, PDFDoc *doc);

        friend class PDFDocument;
    };

}

#endif /* FONTCOLLECTION_INCL_ */
