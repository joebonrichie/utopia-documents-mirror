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
 * PDFPage.cpp
 *
 * Copyright 2008 Advanced Interfaces Group
 *
 ****************************************************************************/
#include <crackle/PDFPage.h>
#include <crackle/PDFDocument.h>
#include <spine/Image.h>
#include <crackle/ImageCollection.h>
#include <crackle/CrackleTextOutputDev.h>
#include <crackle/PDFTextRegionCollection.h>
#include <crackle/PDFFontCollection.h>
#include <crackle/xpdfapi.h>

#include "aconf.h"
#include "PDFDoc.h"
#include "Page.h"
#include "SplashOutputDev.h"
#include "splash/SplashBitmap.h"
#include "goo/GList.h"

#include <iostream>
#include <algorithm>

#include <string>

#include <boost/thread/locks.hpp>

using namespace std;
using namespace boost;
using namespace Spine;
using namespace Crackle;

Crackle::PDFPage::PDFPage (PDFDocument * doc_, unsigned int page_,
                           boost::shared_ptr<CrackleTextOutputDev> textDevice_,
                           boost::shared_ptr<SplashOutputDev> renderDevice_,
                           boost::shared_ptr<SplashOutputDev> printDevice_)
  : _doc(doc_), _page(page_), _textDevice(textDevice_),
    _renderDevice(renderDevice_),
    _printDevice(printDevice_),
    _sharedData(boost::shared_ptr<SharedData>(new SharedData))
{
    //std::cerr << "+++ PAG " << this << std::endl;
}

Crackle::PDFPage::PDFPage (const PDFPage &rhs_)
    : _doc(rhs_._doc), _page(rhs_._page), _textDevice(rhs_._textDevice),
      _renderDevice(rhs_._renderDevice),
      _printDevice(rhs_._printDevice),
      _sharedData(rhs_._sharedData)
{
    //std::cerr << "+++ PAG " << this << std::endl;
}

Crackle::PDFPage::~PDFPage ()
{
    //std::cerr << "--- PAG " << this << " " << _textDevice.use_count() << std::endl;
}

PDFPage &Crackle::PDFPage::operator= (const PDFPage &rhs_)
{

    if(&rhs_!=this) {
        _sharedData=rhs_._sharedData;
        _doc=rhs_._doc;
        _page=rhs_._page;
        _textDevice=rhs_._textDevice;
        _renderDevice=rhs_._renderDevice;
        _printDevice=rhs_._printDevice;
    }

    return *this;
}

int Crackle::PDFPage::pageNumber() const
{
    return _page;
}

BoundingBox Crackle::PDFPage::boundingBox() const
{
    int rotate = _doc->xpdfDoc()->getCatalog()->getPage(_page)->getRotate();
    PDFRectangle *rect=_doc->xpdfDoc()->getCatalog()->getPage(_page)->getCropBox();
    //PDFRectangle *rect=_doc->xpdfDoc()->getCatalog()->getPage(_page)->getTrimBox();
    if (rotate % 180 == 0) { // No change to rectangle
        return BoundingBox(rect->x1, rect->y1, rect->x2, rect->y2);
    } else { // Swap height and width
        return BoundingBox(rect->x1, rect->y1, rect->x1+(rect->y2-rect->y1), rect->y1+(rect->x2-rect->x1));
    }
}

/*
  Spine::BoundingBox Crackle::PDFPage::artBox() const
  {
  PDFRectangle *rect=_doc->xpdfDoc()->getCatalog()->getPage(_page)->getArtBox();
  return BoundingBox(rect->x1, rect->y1, rect->x2, rect->y2);
  }

  Spine::BoundingBox Crackle::PDFPage::bleedBox() const
  {
  PDFRectangle *rect=_doc->xpdfDoc()->getCatalog()->getPage(_page)->getBleedBox();
  return BoundingBox(rect->x1, rect->y1, rect->x2, rect->y2);
  }

  Spine::BoundingBox Crackle::PDFPage::cropBox() const
  {
  PDFRectangle *rect=_doc->xpdfDoc()->getCatalog()->getPage(_page)->getCropBox();
  return BoundingBox(rect->x1, rect->y1, rect->x2, rect->y2);
  }

  Spine::BoundingBox Crackle::PDFPage::trimBox() const
  {
  PDFRectangle *rect=_doc->xpdfDoc()->getCatalog()->getPage(_page)->getTrimBox();
  return BoundingBox(rect->x1, rect->y1, rect->x2, rect->y2);
  }
*/

Spine::BoundingBox Crackle::PDFPage::mediaBox() const
{
    PDFRectangle *rect=_doc->xpdfDoc()->getCatalog()->getPage(_page)->getMediaBox();
    return BoundingBox(rect->x1, rect->y1, rect->x2, rect->y2);
}

int Crackle::PDFPage::rotation() const
{
    return _doc->xpdfDoc()->getPageRotate(_page);
}

const Crackle::ImageCollection &Crackle::PDFPage::images() const
{
    _mutexSharedData.lock();
    bool alreadyExtracted = (bool) _sharedData->_images;
    _mutexSharedData.unlock();

    if (!alreadyExtracted) {
        _extractTextAndImages();
    }

    boost::lock_guard<boost::mutex> g(_mutexSharedData);
    return *_sharedData->_images;
}

Spine::Image Crackle::PDFPage::render(size_t width_,
                                      size_t height_,
                                      bool antialias_) const
{
    Crackle::PDFDocument::_globalMutexDocument.lock();

    double w(_doc->xpdfDoc()->getPageCropWidth(_page));
    double h(_doc->xpdfDoc()->getPageCropHeight(_page));
    /*
      double w(_doc->xpdfDoc()->getCatalog()->getPage(_page)->getTrimBox()->x2 - _doc->xpdfDoc()->getCatalog()->getPage(_page)->getTrimBox()->x1);
      double h(_doc->xpdfDoc()->getCatalog()->getPage(_page)->getTrimBox()->y2 - _doc->xpdfDoc()->getCatalog()->getPage(_page)->getTrimBox()->y1);
    */
    if (_doc->xpdfDoc()->getPageRotate(_page) % 180) // Swap if rotated by 90 / 270 degrees
    {
        double tmp(w); w=h; h=tmp;
    }
    Crackle::PDFDocument::_globalMutexDocument.unlock();


    double fit_resolution_w = (72.0 * width_) / w;
    double fit_resolution_h = (72.0 * height_) / h;
    double resolution = std::min(fit_resolution_w, fit_resolution_h);

    return this->render(resolution);
}

Spine::Image Crackle::PDFPage::render(double resolution_, bool antialias_) const
{
    boost::lock_guard<boost::mutex> g(Crackle::PDFDocument::_globalMutexDocument);
    _doc->xpdfDoc()->displayPage(_renderDevice.get(), _page, resolution_,
                                 resolution_, 0, gFalse, gFalse, gFalse);

    SplashBitmap *bitmap(_renderDevice->getBitmap());

    size_t length= bitmap->getWidth() * 3 * bitmap->getHeight();
    char *data=reinterpret_cast<char *>(bitmap->getDataPtr());

    // updside down image?
    if (bitmap->getRowSize()<0) {
        data += (bitmap->getHeight() - 1) * bitmap->getRowSize();
    }

    return Image(Image::RGB, bitmap->getWidth(), bitmap->getHeight(),
                 this->boundingBox(),data,length);
}

Spine::Image Crackle::PDFPage::renderArea(const Spine::BoundingBox & slice,
                                          size_t width_,
                                          size_t height_,
                                          bool antialias_) const
{
    double fit_resolution_w = (72.0 * width_) / slice.width();
    double fit_resolution_h = (72.0 * height_) / slice.height();
    double resolution = std::min(fit_resolution_w, fit_resolution_h);

    return this->renderArea(slice, resolution, antialias_);
}

Spine::Image Crackle::PDFPage::renderArea(const Spine::BoundingBox & slice,
                                          double resolution_,
                                          bool antialias_) const
{
  return renderArea(slice, resolution_, resolution_, antialias_);
}

Spine::Image Crackle::PDFPage::renderArea(const Spine::BoundingBox & slice,
                                          double resolutionX_,
                                          double resolutionY_,
                                          bool antialias_) const
{
    boost::lock_guard<boost::mutex> g(Crackle::PDFDocument::_globalMutexDocument);
    double resolutionScaleX(72.0 / resolutionX_);
    double resolutionScaleY(72.0 / resolutionY_);
    Spine::BoundingBox scaledSlice(slice.x1 / resolutionScaleX, slice.y1 / resolutionScaleX,
                                   slice.x2 / resolutionScaleY, slice.y2 / resolutionScaleY);

    boost::shared_ptr<SplashOutputDev> dev;
    if(antialias_) {
      dev = _renderDevice;
    } else {
      dev = _printDevice;
    }

    _doc->xpdfDoc()->displayPageSlice(dev.get(), _page, resolutionX_,
                                      resolutionY_, 0, gFalse, gFalse, gFalse,
                                      (int) scaledSlice.x1, (int) scaledSlice.y1,
                                      (int) (scaledSlice.x2-scaledSlice.x1),
                                      (int) (scaledSlice.y2-scaledSlice.y1));

    SplashBitmap *bitmap(dev->getBitmap());

    size_t length= bitmap->getWidth() * 3 * bitmap->getHeight();
    char *data=reinterpret_cast<char *>(bitmap->getDataPtr());

    // updside down image?
    if (bitmap->getRowSize()<0) {
        data += (bitmap->getHeight() - 1) * bitmap->getRowSize();
    }

    return Image(Image::RGB, bitmap->getWidth(), bitmap->getHeight(),
                 slice, data, length);
}

void Crackle::PDFPage::_extractTextAndImages() const
{
    {
        boost::lock_guard<boost::mutex> g(Crackle::PDFDocument::_globalMutexDocument);

        double w(_doc->xpdfDoc()->getPageMediaWidth(_page));
        double h(_doc->xpdfDoc()->getPageMediaHeight(_page));

        PDFRectangle *rect=_doc->xpdfDoc()->getCatalog()->getPage(_page)->getMediaBox();

        double resolution_w = (72.0 * (rect->x2-rect->x1)) / w;
        double resolution_h = (72.0 * (rect->y2-rect->y1)) / h;

        _doc->xpdfDoc()->displayPage(_textDevice.get(), _page, resolution_w, resolution_h,
                                     0, gFalse, gFalse, gFalse);
    }

    boost::lock_guard<boost::mutex> g(_mutexSharedData);
    _sharedData->_textpage=boost::shared_ptr<CrackleTextPage> (_textDevice->takeText());
    _sharedData->_text= boost::shared_ptr<PDFTextRegionCollection> (new PDFTextRegionCollection(_sharedData->_textpage->getFlows()));
    _sharedData->_images=boost::shared_ptr<ImageCollection>(_textDevice->pageImages());
}

const Crackle::PDFTextRegionCollection &Crackle::PDFPage::regions() const
{
    _mutexSharedData.lock();
    bool alreadyExtracted = (bool) _sharedData->_text;
    _mutexSharedData.unlock();

    if (!alreadyExtracted) {
        _extractTextAndImages();
    }

    boost::lock_guard<boost::mutex> g(_mutexSharedData);
    return *_sharedData->_text;
}

const PDFFontCollection &PDFPage::fonts() const
{
    this->regions();

    boost::lock_guard<boost::mutex> g(_mutexSharedData);
    return _sharedData->_textpage->getFontCollection();
}

string Crackle::PDFPage::text() const
{
    string result;

    PDFTextRegionCollection::const_iterator i;
    for (i=regions().begin(); i!=regions().end(); ++i) {
        result+=i->text() + "\n";
    }
    return result;
}
