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

#ifndef PAGE_INCL_
#define PAGE_INCL_

/*****************************************************************************
 *
 * Page.h
 *
 * Copyright 2008 Advanced Interfaces Group
 *
 ****************************************************************************/
#include <spine/BoundingBox.h>
#include <spine/Image.h>
#include <boost/shared_ptr.hpp>
#include <string>

namespace Spine
{

    class Page
    {

    public:
        virtual ~Page() {}
        virtual int pageNumber() const=0;
        virtual BoundingBox boundingBox() const=0;
        virtual BoundingBox mediaBox() const { return boundingBox(); };

        /* These four might stay, but will probably go as they may not
           mean anything to any back end other than Crackle. For
           testing at the moment. */
        /*
          virtual BoundingBox artBox() const=0;
          virtual BoundingBox bleedBox() const=0;
          virtual BoundingBox cropBox() const=0;
          virtual BoundingBox trimBox() const=0;
        */

        virtual int rotation() const=0;
        virtual Image render(size_t width_, size_t height_,
                             bool antialias_=true) const=0;
        virtual Image render(double resolution_, bool antialias_=true) const=0;
        virtual Image renderArea(const BoundingBox & slice,
                                 size_t width_, size_t height_,
                                 bool antialias_=true) const=0;
        virtual Image renderArea(const BoundingBox & slice,
                                 double resolution_,
                                 bool antialias_=true) const=0;
        virtual std::string text() const=0;
    };
}

#endif /* PAGE_INCL_ */
