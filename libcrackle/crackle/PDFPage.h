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
 * PDFPage.h
 *
 * Copyright 2008 Advanced Interfaces Group
 *
 ****************************************************************************/

#ifndef PDFPAGE_INCL_
#define PDFPAGE_INCL_

#include <spine/Page.h>
#include <spine/BoundingBox.h>
#include <spine/Image.h>
#include <crackle/PDFFontCollection.h>
#include <crackle/PDFTextRegionCollection.h>

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include <string>

class PDFDoc;
class CrackleTextPage;

class CrackleTextOutputDev;
class SplashOutputDev;
class SplashBitmap;

namespace Crackle
{

    class ImageCollection;

    class PDFPage : public Spine::Page
    {
        /********************************************************************
         *
         * PDFPage
         *
         * Note: conceptually a PDFPage is always constant to client
         * code because crackle is only an access library. However the
         * pdf is actually only processed on demand - hence the
         * mutable data members.
         *
         ********************************************************************/

    public:

        PDFPage (const PDFPage& rhs_);
        virtual ~PDFPage();

        int pageNumber() const;
        Spine::BoundingBox boundingBox() const;
        Spine::BoundingBox mediaBox() const;
        /*
          Spine::BoundingBox artBox() const;
          Spine::BoundingBox bleedBox() const;
          Spine::BoundingBox cropBox() const;
          Spine::BoundingBox trimBox() const;
        */
        int rotation() const;

        const ImageCollection &images() const;
        Spine::Image render(size_t width_,
                            size_t height_,
                            bool antialias_=true) const;
        Spine::Image render(double resolution_,
                            bool antialias_=true) const;
        Spine::Image renderArea(const Spine::BoundingBox & slice,
                                size_t width_,
                                size_t height_,
                                bool antialias_=true) const;
        Spine::Image renderArea(const Spine::BoundingBox & slice,
                                double resolution_,
                                bool antialias_=true) const;

        const PDFTextRegionCollection &regions() const;
        const PDFFontCollection &fonts() const;
        virtual std::string text() const;

    private:

        friend class PDFDocument;

        PDFPage (PDFDocument * doc_, unsigned int page_,
                 boost::shared_ptr<CrackleTextOutputDev> textDevice_,
                 boost::shared_ptr<SplashOutputDev> renderDevice_,
                 boost::shared_ptr<SplashOutputDev> printDevice_);

        PDFPage &operator=(const PDFPage& rhs_);

        inline Spine::Image renderArea(const Spine::BoundingBox & slice,
                                       double resolutionX_,
                                       double resolutionY_,
                                       bool antialias_=true) const;

        void _extractTextAndImages() const;

        mutable PDFDocument * _doc;
        unsigned int _page;

        mutable boost::shared_ptr<CrackleTextOutputDev> _textDevice;
        mutable boost::shared_ptr<SplashOutputDev> _renderDevice;
        mutable boost::shared_ptr<SplashOutputDev> _printDevice;

        // This struct is reference counted and shared between all copies
        // of this page. The data contained within is generated lazilly.
        // Creating this struct therefore allows copies to be made before
        // that data is instantiated and yet still get updated across
        // the shared instances.
        struct SharedData {
            boost::shared_ptr<PDFTextRegionCollection> _text;
            boost::shared_ptr<ImageCollection>   _images;
            boost::shared_ptr<CrackleTextPage>      _textpage;
            boost::shared_ptr<PDFFontCollection>    _fonts;
        };

        mutable boost::shared_ptr<SharedData> _sharedData;
        mutable boost::mutex _mutexSharedData;
        mutable boost::mutex _mutexDisplayPage;

    };
}

#endif /* PDFPAGE_INCL_ */
