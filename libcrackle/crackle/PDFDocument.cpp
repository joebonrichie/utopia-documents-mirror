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
 * PDFDocument.cpp
 *
 * Copyright 2008 Advanced Interfaces Group
 *
 ****************************************************************************/

#include <crackle/PDFDocument.h>
#include <crackle/PDFPage.h>
#include <crackle/PDFTextRegionCollection.h>
#include <crackle/PDFTextBlockCollection.h>
#include <crackle/PDFTextLineCollection.h>
#include <crackle/PDFTextWordCollection.h>
#include <crackle/PDFTextCharacterCollection.h>
#include <crackle/CrackleTextOutputDev.h>
#include <crackle/ImageCollection.h>
#include <crackle/xpdfapi.h>
#include <spine/fingerprint.h>
#include <spine/Annotation.h>

#include "aconf.h"
#include "GlobalParams.h"
#include "Object.h"
#include "Stream.h"
#include "Catalog.h"
#include "PDFDoc.h"
#include "ErrorCodes.h"
#include "UnicodeMap.h"
#include "PDFDocEncoding.h"
#include "SplashOutputDev.h"
#include "Outline.h"
#include "GList.h"
#include "Link.h"

#include <cwctype>
#include <algorithm>
#include <locale>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <sstream>
#include <iostream>
#include <sstream>

#include <string>
#include <utf8/unicode.h>
#include <pcrecpp.h>

using namespace std;
using namespace boost;
using namespace Crackle;
using namespace Spine;
using namespace pcrecpp;

namespace {

    // allocate a static instance of GlobalParams so it cleanly gets deleted
    // when program exits
    GlobalParams CrackleGlobalParams(0);

    string getPDFInfo(boost::shared_ptr<PDFDoc> doc_, const char *key_)
    {
        Object obj, val;
        string data;

        doc_->getDocInfo(&obj);
        if (obj.isDict() && obj.getDict()->lookup(const_cast<char *>(key_), &val)->isString()) {
            data=gstring2UnicodeString(val.getString());
        }

        obj.free();
        val.free();

        return data;
    }

    time_t getPDFInfoDate(boost::shared_ptr<PDFDoc> doc_, const char *key_)
    {
        Object obj,val;
        char *s;
        int year, mon, day, hour, min, sec, n;
        struct tm tmStruct;
        time_t res(0);

        doc_->getDocInfo(&obj);
        if (obj.isDict() && obj.getDict()->lookup(const_cast<char *>(key_), &val)->isString()) {

            s = val.getString()->getCString();

            if (s[0] == 'D' && s[1] == ':') {
                s += 2;
            }
            if ((n = sscanf(s, "%4d%2d%2d%2d%2d%2d",
                            &year, &mon, &day, &hour, &min, &sec)) >= 1) {
                switch (n) {
                case 1: mon = 1;
                case 2: day = 1;
                case 3: hour = 0;
                case 4: min = 0;
                case 5: sec = 0;
                }
                tmStruct.tm_year = year - 1900;
                tmStruct.tm_mon = mon - 1;
                tmStruct.tm_mday = day;
                tmStruct.tm_hour = hour;
                tmStruct.tm_min = min;
                tmStruct.tm_sec = sec;
                tmStruct.tm_wday = -1;
                tmStruct.tm_yday = -1;
                tmStruct.tm_isdst = -1;

                res=mktime(&tmStruct);
            }
        }
        obj.free();
        val.free();
        return res;
    }

}

/****************************************************************************/

boost::mutex Crackle::PDFDocument::_globalMutexDocument;


Crackle::PDFDocument::PDFDocument()
    : Spine::Document(), _crackle_errorcode(errNone), _fonts_counted(false), _datalen(0), _generated_anchors(0)

{
    //std::cerr << "+++ DOC " << this << std::endl;
    _initialise();
}

/****************************************************************************/

Crackle::PDFDocument::PDFDocument(const char *filename_)
    : Spine::Document(), _crackle_errorcode(errNone), _fonts_counted(false), _datalen(0), _generated_anchors(0)
{
    //std::cerr << "+++ DOC " << this << std::endl;
    _initialise();
    this->readFile(filename_);
}

/****************************************************************************/

Crackle::PDFDocument::PDFDocument(boost::shared_array<char> buffer_, std::size_t length_)
    : Spine::Document(), _crackle_errorcode(errNone), _fonts_counted(false), _datalen(0), _generated_anchors(0)
{
    //std::cerr << "+++ DOC " << this << std::endl;
    _initialise();
    this->readBuffer(buffer_, length_);
}

/****************************************************************************/

Crackle::PDFDocument::~PDFDocument()
{
    //std::cerr << "--- DOC " << this << " " << _textDevice.use_count() << std::endl;
    this->close();
}

/****************************************************************************/

void Crackle::PDFDocument::close() {

    boost::lock_guard<boost::mutex> g(_mutexPageMap);
    _crackle_errorcode=errNone;

    for(std::map<int,PDFPage *>::iterator i(_pageMap.begin());
        i!=_pageMap.end(); ++i) {
        delete i->second;
    }

    _textDevice.reset();
    _renderDevice.reset();
    _printDevice.reset();

    _doc.reset();
    _dict.reset();
    _data.reset();
    _datalen=0;
}

/****************************************************************************/

bool
Crackle::PDFDocument::isOK()
{
    bool res(false);
    if(_doc) {
        res=(_doc->isOk()==gTrue);
    }
    return res;
}

/****************************************************************************/

const char *
Crackle::PDFDocument::errorString()
{

    const char *res("");
    int err(_crackle_errorcode);

    if(err==0 && _doc) {
        err= _doc->getErrorCode();
    }

    switch(err)
    {
    case errNone:
        res="no error";
        break;
    case errOpenFile:
        res="couldn't open the PDF file";
        break;
    case errBadCatalog:
        res="couldn't read the page catalog";
        break;
    case errDamaged:
        res="PDF file was damaged and couldn't be repaired";
        break;
    case errEncrypted:
        res="file was encrypted and password was incorrect or not supplied";
        break;
    case errHighlightFile:
        res="nonexistent or invalid highlight file";
        break;
    case errBadPrinter:
        res="invalid printer";
        break;
    case errPrinting:
        res="error during printing";
        break;
    case errPermission:
        res="PDF file doesn't allow that operation";
        break;
    case errBadPageNum:
        res="invalid page number";
        break;
    case errFileIO:
        res="file I/O error";
        break;
    default:
        res="undefined error";
        break;
    }

    return res;
}

/****************************************************************************/

void Crackle::PDFDocument::readFile(const char * filename_) // MUST be utf-8
{
    FILE *file=fopen(filename_, "rb");
    if(file) {
        fseek(file, 0, SEEK_END);
        size_t len=ftell(file);
        shared_array<char> data(new char[len]);
        fseek(file, 0, SEEK_SET);
        size_t rd=fread(static_cast<void *> (data.get()), 1, len, file);
        if(rd < len)
        {
            _crackle_errorcode= errFileIO;
        } else {
            this->readBuffer(data, len);
        }
        fclose(file);
    } else {
        _crackle_errorcode= errFileIO;
    }
}

/****************************************************************************/

void Crackle::PDFDocument::readBuffer(shared_array<char> data_, size_t length_)
{
    this->close();

    // stream and file ownership is passed to PDFDoc
    _dict=boost::shared_ptr<Object>(new Object);
    _dict->initNull();

    _data=data_;
    _datalen=length_;

    MemStream *stream=new MemStream(_data.get(), 0, _datalen, _dict.get());
    _open(stream);

    Spine::Sha256 hash;
    hash.update(reinterpret_cast< unsigned char * > (data_.get()), length_);
    _filehash=Spine::Fingerprint::binaryFingerprintIri(hash.calculateHash());
    if(this->isOK()) {
        _updateAnnotations();
    }
}

/****************************************************************************/

std::string Crackle::PDFDocument::_addAnchor( Object *obj, std::string name)
{
    Object obj2;
    LinkDest *dest(0);
    std::string result;

    if (obj->isArray()) {
        dest = new LinkDest(obj->getArray());
    } else if (obj->isDict()) {
        if (obj->dictLookup("D", &obj2)->isArray()) {
            dest = new LinkDest(obj2.getArray());
        }
        obj2.free();
    }

    if (dest && dest->isOk()) {
        result=_addAnchor(dest, name);
    }

    delete dest;
    return result;
}

static Spine::BoundingBox rotateRect(Spine::BoundingBox rect, int rotation, const Spine::BoundingBox & pageBox)
{
    Spine::BoundingBox rotated;

    // translate according to the page box offset
    rect.x1 -= pageBox.x1;
    rect.x2 -= pageBox.x1;
    rect.y1 -= pageBox.y1;
    rect.y2 -= pageBox.y1;

    switch (rotation) {
    case 0:
        rotated.x1 = rect.x1;
        rotated.y1 = pageBox.height() - rect.y2;
        rotated.x2 = rect.x2;
        rotated.y2 = pageBox.height() - rect.y1;
        break;
    case 90:
        rotated.x1 = rect.y1;
        rotated.y1 = rect.x1;
        rotated.x2 = rect.y2;
        rotated.y2 = rect.x2;
        break;
    case 180:
        rotated.x1 = pageBox.width() - rect.x2;
        rotated.y1 = rect.y1;
        rotated.x2 = pageBox.width() - rect.x1;
        rotated.y2 = rect.y2;
        break;
    case 270:
        rotated.x1 = pageBox.width() - rect.y2;
        rotated.y1 = pageBox.height() - rect.x2;
        rotated.x2 = pageBox.width() - rect.y1;
        rotated.y2 = pageBox.height() - rect.x1;
        break;
    default:
        break;
    }

    return rotated;
}

std::string Crackle::PDFDocument::_addAnchor(LinkDest * dest, std::string name)
{
    size_t page;
    ostringstream anchorname;

    if (dest && dest->isOk()) {

        Ref pageRef;

        if (name.size()>0) {
            if(name[0]!='#') {
                anchorname << "#";
            }
            anchorname << name;
        } else {
            anchorname << "#com.utopiadocs.anchor" << _generated_anchors++;
        }

        if (dest->isPageRef()) {
            pageRef = dest->getPageRef();
            page = _doc->findPage(pageRef.num, pageRef.gen);
        } else {
            page = dest->getPageNum();
        }

        if (page <= 0 || page > this->size()) {
            // error - page number incorrect
            page = 1;
        }

        const PDFPage &pdfpage((*this)[page-1]);
        Spine::BoundingBox pageArea(pdfpage.boundingBox());
        Spine::BoundingBox destArea(Spine::BoundingBox(dest->getLeft(),
                                                       dest->getTop(),
                                                       dest->getRight(),
                                                       dest->getBottom()));
        destArea = rotateRect(destArea, pdfpage.rotation(), pageArea);

        // be sure to convert y coordinate!
        Spine::BoundingBox area(pageArea);
        switch(dest->getKind()) {
        case destXYZ:
            area.x1=destArea.x1;
            area.y1=destArea.y1;
            break;
        case destFit:
        case destFitB:
            // show whole page
            break;
        case destFitH:
        case destFitBH:
            area.y1=destArea.y1;
            break;
        case destFitV:
        case destFitBV:
            area.x1=destArea.x1;
            break;
        case destFitR:
            area.x1=destArea.x1;
            area.y1=destArea.y1;
            area.x2=destArea.x2;
            area.y2=destArea.y2;
        default:
            // uh - fit page by default
            break;
        }

        AnnotationHandle anchor(new Annotation());
        anchor->setProperty("concept", "Anchor");
        anchor->setProperty("property:anchor", anchorname.str());
        anchor->addArea(Area(page, 0, area));

        // TODO: add text extent?

        this->addAnnotation(anchor);
    }
    return anchorname.str();
}

/****************************************************************************/

void Crackle::PDFDocument::_updateNameTree(Object *tree)
{
    if (tree->isDict()) {
        Object names, name;
        Object kids, kid;
        Object obj, obj2;

        // leaf node
        if (tree->dictLookup("Names", &names)->isArray()) {
            for (int i = 0; i < names.arrayGetLength(); i += 2) {
                if (names.arrayGet(i, &name)->isString()) {
                    string namestring(gstring2UnicodeString(name.getString()));
                    names.arrayGet(i+1, &obj);
                    _addAnchor(&obj, namestring);
                    obj.free();
                }
                name.free();
            }
        }
        names.free();

        // root or intermediate node - process children
        if (tree->dictLookup("Kids", &kids)->isArray()) {
            for (int i = 0; i < kids.arrayGetLength(); ++i) {
                if (kids.arrayGet(i, &kid)->isDict()) {
                    _updateNameTree(&kid);
                }
                kid.free();
            }
        }
        kids.free();
    }
}

/****************************************************************************/

void Crackle::PDFDocument::_extractOutline(GList *items, string prefix, UnicodeMap *uMap)
{
    char buf[8];

    for (int i = 0; i < items->getLength(); ++i) {
        OutlineItem *item = (OutlineItem *)items->get(i);
        string title;
        for (int j = 0; j < item->getTitleLength(); ++j) {
            int n = uMap->mapUnicode(item->getTitle()[j], buf, sizeof(buf));
            title.append(buf, buf+n);
        }
        size_t divider_pos;
        // replace one of the spaces in symbol we'll use for hierarchy with a nbsp
        while( (divider_pos = title.find(" / ") ) != std::string::npos) {
            title.replace(divider_pos, 5, "\xc2\xa0/ ");
        }

        ostringstream position;
        if(prefix.size()) {
            position << prefix << ".";
        }
        position << i+1;

        if (item->getAction()->getKind()==actionGoTo || item->getAction()->getKind()==actionGoToR )
        {
            LinkDest *dest(0);
            GString *namedDest(0);
            if (item->getAction()->getKind()==actionGoTo) {
                string anchor;

                // go to destination in this file
                if ((dest = ((LinkGoTo *)item->getAction())->getDest())) {
                    anchor=_addAnchor(dest);
                } else if ((namedDest = ((LinkGoTo *)item->getAction())->getNamedDest())) {
                    anchor=gstring2UnicodeString(namedDest);
                }
                if (anchor.size()>0) {
                    if(anchor[0]!='#') {
                        anchor=string("#") + anchor;
                    }
                    AnnotationHandle ann(new Annotation());
                    ann->setProperty("concept", "OutlineItem");
                    ann->setProperty("property:destinationAnchorName", anchor);
                    ann->setProperty("property:outlineTitle", title);
                    ann->setProperty("property:outlinePosition", position.str());
                    ann->setProperty("property:name", "Outline");
                    ann->setProperty("property:description", "Document Outline");
                    ann->setProperty("property:sourceDatabase",  "pdf");
                    ann->setProperty("property:sourceDescription",  "<p>Embedded PDF outline</p>");

                    this->addAnnotation(ann);
                }
            } else {
                // Action GoToR ???
            }
        }

        item->open();

        GList * kids;
        if ((kids = item->getKids())) {
            _extractOutline(kids, position.str(), uMap);
        }
    }
}

/****************************************************************************/

void Crackle::PDFDocument::_extractLinks()
{
    Catalog *catalog= xpdfDoc()->getCatalog();

    for (size_t page=0; page<size(); ++page) {

        const PDFPage &pdfpage((*this)[page]);

        // get annotations
#ifdef UTOPIA_SPINE_BACKEND_POPPLER
        Links *links=new Links(catalog->getPage(page+1)->getAnnots());
#else // XPDF
        Object annotsObj;
        Links *links=new Links(catalog->getPage(page+1)->getAnnots(&annotsObj), catalog->getBaseURI());
        annotsObj.free();
#endif

        for (int i(0); i<links->getNumLinks(); ++i) {

            Link *link=links->getLink(i);

            Spine::BoundingBox pageArea(pdfpage.boundingBox());

            double x1, y1, x2, y2;
            link->getRect(&x1, &y1, &x2, &y2);
            Spine::BoundingBox linkArea(rotateRect(Spine::BoundingBox(x1, y1, x2, y2),
                                                   pdfpage.rotation(),
                                                   pageArea));

            LinkAction *action(link->getAction());

            std::string uri;
            std::string anchor;

            if (action->getKind()==actionGoTo || action->getKind()==actionGoToR ) {

                LinkDest *dest(0);
                GString *namedDest(0);
                if (action->getKind()==actionGoTo) {
                    string anchor;

                    // go to destination in this file
                    if ((dest = ((LinkGoTo *)action)->getDest())) {
                        anchor=_addAnchor(dest);
                    } else if ((namedDest = ((LinkGoTo *)action)->getNamedDest())) {
                        anchor=gstring2UnicodeString(namedDest);
                    }
                    if (anchor.size()>0) {
                        if(anchor[0]!='#') {
                            anchor=string("#") + anchor;
                        }
                        AnnotationHandle ann(new Annotation());
                        ann->setProperty("concept", "Hyperlink");
                        ann->setProperty("property:webpageUrl", "#");
                        ann->setProperty("property:destinationAnchorName", anchor);
                        ann->addArea(Area(page+1, 0, linkArea));

                        this->addAnnotation(ann);
                    }
                } else {
                    // Action GoToR ???
                }

            }

            if (action->getKind()==actionURI ) {
                GString *uri;
                if ((uri = ((LinkURI *)action)->getURI())) {
                    AnnotationHandle ann(new Annotation());
                    ann->setProperty("concept", "Hyperlink");
                    ann->setProperty("property:webpageUrl", gstring2UnicodeString(uri));
                    ann->addArea(Area(page+1, 0, linkArea));

                    this->addAnnotation(ann);
                }
            }
        }
        delete links;


    }
}

void Crackle::PDFDocument::_updateAnnotations()
{
    Catalog *catalog(_doc->getCatalog());

#ifdef UTOPIA_SPINE_BACKEND_POPPLER

    // extract anchors from name tree
    Object catDict;
    _doc->getXRef()->getCatalog(&catDict);
    if (catDict.isDict()) {
      Object obj;
      if (catDict.dictLookup("Names", &obj)->isDict()) {
        Object nameTree;
        obj.dictLookup("Dests", &nameTree);
        _updateNameTree(&nameTree);
        nameTree.free();
      }
      obj.free();
    }
    catDict.free();

#else // XPDF

    // extract anchors from name tree
    Object *nameTree=catalog->getNameTree();
    if (nameTree) {
        _updateNameTree(nameTree);
    }

#endif

    // extract anchors from named destination dict
    Object *dests=catalog->getDests();
    if (dests && dests->isDict()) {
        for (int i=0; i< dests->dictGetLength(); ++i) {
            string namestring(dests->dictGetKey(i));
            Object obj;
            dests->dictGetVal(i, &obj);
            _addAnchor(&obj, namestring);
            obj.free();
        }
    }

    // extract contents outline
    Outline *outline(_doc->getOutline());
    if(outline) {
        GList *items(outline->getItems());
        if (items && items->getLength() > 0) {
            GString *enc = new GString("Latin1");
            UnicodeMap *uMap = globalParams->getUnicodeMap(enc);
            delete enc;
            _extractOutline(items, "", uMap);
            uMap->decRefCnt();
        }
    }

    // Extract link annotations
    _extractLinks();
}

/****************************************************************************/

std::string Crackle::PDFDocument::data()
{
    return std::string(_data.get(), _datalen);
}

/****************************************************************************/

string Crackle::PDFDocument::title()
{
    return getPDFInfo(_doc, "Title");
}

/****************************************************************************/

string Crackle::PDFDocument::subject()
{
    return getPDFInfo(_doc, "Subject");
}

/****************************************************************************/

string Crackle::PDFDocument::keywords()
{
    return getPDFInfo(_doc, "Keywords");
}

/****************************************************************************/

string Crackle::PDFDocument::author()
{
    return getPDFInfo(_doc, "Author");
}

/****************************************************************************/

string Crackle::PDFDocument::creator()
{
    return getPDFInfo(_doc, "Creator");
}

/****************************************************************************/

string Crackle::PDFDocument::producer()
{
    return getPDFInfo(_doc, "Producer");
}

/****************************************************************************/

time_t Crackle::PDFDocument::creationDate()
{

    return getPDFInfoDate(_doc, "CreationDate");
}

/****************************************************************************/

time_t Crackle::PDFDocument::modificationDate()
{
    return getPDFInfoDate(_doc, "ModDate");
}

/****************************************************************************/

// generate SHA-256 hash of file
string Crackle::PDFDocument::filehash()
{
    return _filehash;
}


/****************************************************************************/

void Crackle::PDFDocument::_initialise()
{
    if(!::globalParams) {

        ::globalParams = &::CrackleGlobalParams; // new GlobalParams();
        //::globalParams->setTextEncoding("UTF-8");
        ::globalParams->setTextEncoding((char *) "UTF-16");
        ::globalParams->setupBaseFonts(0);

        const char *verbose=getenv("PDF_VERBOSE");
        if(verbose && strcmp(verbose,"0")!=0) {
            ::globalParams->setErrQuiet(gFalse);
        } else {
            ::globalParams->setErrQuiet(gTrue);
        }
    }
}

/****************************************************************************/

void Crackle::PDFDocument::_open(BaseStream *stream_)
{
    _doc      = boost::shared_ptr<PDFDoc>(new PDFDoc(stream_));

    if (_doc->isOk()) {
        _textDevice=boost::shared_ptr<CrackleTextOutputDev>(new CrackleTextOutputDev ((char *)0, gFalse, 0.0, gFalse, gFalse));

        SplashColor paperColour;
        paperColour[0] = 255;
        paperColour[1] = 255;
        paperColour[2] = 255;

#ifdef UTOPIA_SPINE_BACKEND_POPPLER
        // defaults setup anti aliasing for screen
        _renderDevice=boost::shared_ptr<SplashOutputDev>(new SplashOutputDev(splashModeRGB8, 3, gFalse, paperColour, gTrue));

  #ifdef HAVE_POPPLER_SPLASH_SET_FONT_ANTIALIAS
        // newer versions of poppler no longer sets font anti-aliasing in constructor
        _printDevice=boost::shared_ptr<SplashOutputDev>(new SplashOutputDev(splashModeRGB8, 3, gFalse, paperColour, gTrue));
        _printDevice->setFontAntialias(gFalse);
  #else
        _printDevice=boost::shared_ptr<SplashOutputDev>(new SplashOutputDev(splashModeRGB8, 3, gFalse, paperColour, gTrue, gFalse));
        // original
  #endif

  #ifdef HAVE_POPPLER_SPLASH_SET_VECTOR_ANTIALIAS
        _printDevice->setVectorAntialias(gFalse);
  #endif

#else // XPDF
        _renderDevice=boost::shared_ptr<SplashOutputDev>(new SplashOutputDev(splashModeRGB8, 3, gFalse, paperColour, gTrue, gTrue));
        _printDevice=boost::shared_ptr<SplashOutputDev>(new SplashOutputDev(splashModeRGB8, 3, gFalse, paperColour, gTrue, gFalse));
#endif

#ifdef UTOPIA_SPINE_BACKEND_POPPLER
        _renderDevice->startDoc(_doc.get());
        _printDevice->startDoc(_doc.get());
#else // XPDF
        _renderDevice->startDoc(_doc->getXRef());
        _printDevice->startDoc(_doc->getXRef());
#endif

    } else {
        _crackle_errorcode=errOpenFile;
    }
}

/****************************************************************************/

Crackle::PDFDocument::ViewMode Crackle::PDFDocument::viewMode()
{
    ViewMode res=ViewNone;

    XRef *xref(_doc->getXRef());
    Object catDict;

    xref->getCatalog(&catDict);
    if (catDict.isDict()) {
        Object obj;

        if (catDict.dictLookup("PageMode", &obj)->isName()) {
            if (obj.isName("UseNone"))
                // Neither document outline nor thumbnail images visible.
                res=ViewNone;
            else if (obj.isName("UseOutlines"))
                // Document outline visible.
                res=ViewOutlines;
            else if (obj.isName("UseThumbs"))
                // Thumbnail images visible.
                res=ViewThumbs;
            else if (obj.isName("FullScreen"))
                // Full-screen mode, with no menu bar, window controls, or any other window visible.
                res=ViewFullScreen;
            else if (obj.isName("UseOC"))
                // Optional content group panel visible.
                res=ViewOC;
            else if (obj.isName("UseAttachments"))
                // Attachments panel visible.
                res=ViewAttach;
        }
        obj.free();
    }
    catDict.free();

    return res;
}

/****************************************************************************/

Crackle::PDFDocument::PageLayout Crackle::PDFDocument::pageLayout()
{
    PageLayout res=LayoutNone;

    XRef *xref(_doc->getXRef());
    Object catDict;

    xref->getCatalog(&catDict);
    if (catDict.isDict()) {
        Object obj;
        if (catDict.dictLookup("PageLayout", &obj)->isName()) {
            if (obj.isName("SinglePage"))
                res=LayoutSinglePage;
            if (obj.isName("OneColumn"))
                res=LayoutOneColumn;
            if (obj.isName("TwoColumnLeft"))
                res=LayoutTwoColumnLeft;
            if (obj.isName("TwoColumnRight"))
                res=LayoutTwoColumnRight;
            if (obj.isName("TwoPageLeft"))
                res=LayoutTwoPageLeft;
            if (obj.isName("TwoPageRight"))
                res=LayoutTwoPageRight;
        }
        obj.free();
    }
    catDict.free();

    return res;
}

/****************************************************************************/

string Crackle::PDFDocument::metadata()
{
    string result;
    GString *md(_doc->readMetadata());
    if(md) {
        result=gstring2UnicodeString(md);
    }
    delete md;
    return result;
}

/****************************************************************************/

size_t Crackle::PDFDocument::size()
{
    size_t sz(0);
    if(this->isOK()) {
        sz=_doc->getNumPages();
    }
    return sz;
}

/****************************************************************************/

size_t Crackle::PDFDocument::numberOfPages()
{
    return this->size();
}

/****************************************************************************/

Crackle::PDFDocument::const_iterator
Crackle::PDFDocument::begin()
{
    return(const_iterator(*this,0));
}

/****************************************************************************/

Crackle::PDFDocument::const_iterator
Crackle::PDFDocument::end()
{
    return(const_iterator(*this,this->size()));
}

/****************************************************************************/

const PDFPage&
Crackle::PDFDocument::operator[](int idx_)
{
    boost::lock_guard<boost::mutex> g(_mutexPageMap);
    std::map<int,PDFPage *>::const_iterator i=_pageMap.find(idx_);
    if(i==_pageMap.end()) {
      _pageMap[idx_]= new PDFPage(this, idx_+1, _textDevice,
                                  _renderDevice, _printDevice);
    }

    return(*(_pageMap[idx_]));
}

/****************************************************************************/

boost::shared_ptr<Spine::Cursor> Crackle::PDFDocument::newCursor(int page_)
{
    return boost::shared_ptr<Spine::Cursor>(new PDFCursor(this, page_));
}

/****************************************************************************/

Spine::DocumentHandle Crackle::PDFDocument::clone()
{
    return Spine::DocumentHandle(new PDFDocument(_data,_datalen));
}

/****************************************************************************/

string Crackle::PDFDocument::uniqueID()
{
    // An XMP InstanceID should be a GUID/UUID-style ID, which is a
    // large integer that is guaranteed to be globally unique (in
    // practical terms, the probability of a collision is so remote as
    // to be effectively impossible). Typically 128- or 144-bit integers
    // are used, encoded as 22 or 24 base-64 characters.

    if(!_uuid.empty()) {
        return _uuid;
    }

    RE re("InstanceID>([^<]*)", UTF8());
    string meta_data(this->metadata());

    string uuid;
    if (re.PartialMatch(meta_data, &uuid)) {

        // hex encode uuid
        ostringstream s;
        s.setf (ios::hex, std::ios::basefield);
        s.fill('0');

        for(string::const_iterator i(uuid.begin()); i != uuid.end(); ++i) {
            s.width(2);
            s << (int) *i;
        }

        _uuid=Spine::Fingerprint::xmpFingerprintIri(s.str());
    } else {
        _uuid.clear();
    }

    return _uuid;
}

/****************************************************************************/

string Crackle::PDFDocument::pdfFileID()
{

    if(!_docid.empty()) {
        return _docid;
    }

    _docid.clear();

    Object fileIDArray;
    _doc->getXRef()->getTrailerDict()->dictLookup("ID", &fileIDArray);

    if (fileIDArray.isArray()) {
        Object fileIDObj0;
        if (fileIDArray.arrayGet(0, &fileIDObj0)->isString()) {

            GString *str(fileIDObj0.getString());

            ostringstream s;
            s.setf (ios::hex, std::ios::basefield);
            s.fill('0');

            for(int i = 0; i < str->getLength(); ++i) {
                s.width(2);
                s << (static_cast<int>(str->getChar(i)) & 0xff);
            }
            _docid=Spine::Fingerprint::pdfFileIDFingerprintIri(s.str());;
        }
        fileIDObj0.free();
    }
    fileIDArray.free();

    return _docid;
}

/****************************************************************************/

Spine::Document::FingerprintSet Crackle::PDFDocument::fingerprints() {
    Spine::Document::FingerprintSet result(Spine::Document::fingerprints());
    string s1(Crackle::PDFDocument::uniqueID());
    if(!s1.empty()) {
        result.insert(s1);
    }
    string s2(Crackle::PDFDocument::pdfFileID());
    if(!s2.empty()) {
        result.insert(s2);
    }

    return result;
}

/****************************************************************************/
