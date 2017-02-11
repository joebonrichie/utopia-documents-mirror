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

/*****************************************************************************
 *
 * PDFFontCollection.cpp
 *
 *     based on pdffonts.cc
 *
 * Copyright 2001-2007 Glyph & Cog, LLC
 * Copyright 2008 Advanced Interfaces Group
 *
 ****************************************************************************/

#include <crackle/PDFFontCollection.h>

#include "aconf.h"
#include "goo/GString.h"
#include "goo/gmem.h"
#include "Object.h"
#include "Dict.h"
#include "GfxFont.h"
#include "Annot.h"
#include "PDFDoc.h"

#include <string>
#include <map>
#include <sstream>

using namespace Crackle;
using namespace std;

PDFFontCollection::PDFFontCollection(PDFDoc *doc_)
{
    Page *page;
    Dict *resDict;
    Annots *annots;
    Object obj1, obj2;

    for (int pg = 1; pg <= doc_->getNumPages(); ++pg) {
        page = doc_->getCatalog()->getPage(pg);
        if ((resDict = page->getResourceDict())) {
            this->_scanFonts(resDict, doc_);
        }
        annots = new Annots(doc_, page->getAnnots(&obj1));
        obj1.free();

        for (int i = 0; i < annots->getNumAnnots(); ++i) {
            if (annots->getAnnot(i)->getAppearance(&obj1)->isStream()) {
                obj1.streamGetDict()->lookup("Resources", &obj2);
                if (obj2.isDict()) {
                    this->_scanFonts(obj2.getDict(), doc_);
                }
                obj2.free();
            }
            obj1.free();
        }
        delete annots;
    }
}

void PDFFontCollection::_scanFonts(Dict *resDict, PDFDoc *doc)
{
    Object obj1, obj2, xObjDict, xObj, resObj;
    Ref r;
    GfxFontDict *gfxFontDict;
    GfxFont *font;
    int i;

    // scan the fonts in this resource dictionary
    gfxFontDict = NULL;
    resDict->lookupNF("Font", &obj1);
    if (obj1.isRef()) {
        obj1.fetch(doc->getXRef(), &obj2);
        if (obj2.isDict()) {
            r = obj1.getRef();
            gfxFontDict = new GfxFontDict(doc->getXRef(), &r, obj2.getDict());
        }
        obj2.free();
    } else if (obj1.isDict()) {
        gfxFontDict = new GfxFontDict(doc->getXRef(), NULL, obj1.getDict());
    }
    if (gfxFontDict) {
        for (i = 0; i < gfxFontDict->getNumFonts(); ++i) {
            if ((font = gfxFontDict->getFont(i))) {
                if(font->isOk() && font->getTag()) {
                    this->insert(std::make_pair(font->getTag()->getCString(), PDFFont(font)));
                }
            }
        }
        delete gfxFontDict;
    }
    obj1.free();

    // recursively scan any resource dictionaries in objects in this
    // resource dictionary
    resDict->lookup("XObject", &xObjDict);
    if (xObjDict.isDict()) {
        for (i = 0; i < xObjDict.dictGetLength(); ++i) {
            xObjDict.dictGetVal(i, &xObj);
            if (xObj.isStream()) {
                xObj.streamGetDict()->lookup("Resources", &resObj);
                if (resObj.isDict()) {
                    this->_scanFonts(resObj.getDict(), doc);
                }
                resObj.free();
            }
            xObj.free();
        }
    }
    xObjDict.free();
}
