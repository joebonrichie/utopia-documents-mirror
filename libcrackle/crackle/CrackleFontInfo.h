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

#ifndef CRACKLEFONTINFO_INCL_
#define CRACKLEFONTINFO_INCL_

#include <map>

namespace Crackle
{

    class FontInfo {
    public:
        enum Type {
            unknown,
            Type1,
            Type1C,
            Type1COT,
            Type3,
            TrueType,
            TrueTypeOT,
            CIDType0,
            CIDType0C,
            CIDType0COT,
            CIDTrueType,
            CIDTrueTypeOT
        };

        // Constructor.
        FontInfo(GfxFont *fontA, PDFDoc *doc);
        // Copy constructor
        FontInfo(FontInfo& f);
        // Destructor.
        ~FontInfo();

        const char *getName()      { return name; };
        const char *getFile()      { return file; };
        Type       getType()      { return type; };
        bool      isEmbedded()  { return emb; };
        bool      isSubset()    { return subset; };

    private:
        shared_ptr<const char *> name;
        GooString *file;
        Type type;
        GBool emb;
        GBool subset;
        GBool hasToUnicode;
        Ref fontRef;
    };

    class FontInfoScanner {
    public:

        // Constructor.
        FontInfoScanner(PDFDoc *doc);
        // Destructor.
        ~FontInfoScanner();

        GooList *scan(int nPages);

    private:

        PDFDoc *doc;
        int currentPage;
        Ref *fonts;
        int fontsLen;
        int fontsSize;

        Ref *visitedXObjects;
        int visitedXObjectsLen;
        int visitedXObjectsSize;

        void scanFonts(Dict *resDict, GooList *fontsList);
    };

#endif /* CRACKLEFONTINFO_INCL_ */
