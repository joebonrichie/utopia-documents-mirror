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

#ifndef IMAGE_INCL_
#define IMAGE_INCL_


/*****************************************************************************
 *
 * Image.h
 *
 * Copyright 2008 Advanced Interfaces Group
 *
 ****************************************************************************/

#include <spine/BoundingBox.h>

#include <vector>
#include <algorithm>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#if 0
struct DELETER {
    void operator () (char * ptr) {
        //printf("--- IMPTR %p\n", ptr);
        delete [] ptr;
    }
};
#endif

namespace Spine {

    class Image {

        /*************************************************************************
         *
         * Image
         *
         ************************************************************************/
    public:

        enum ImageType {Null, JPEG, RGB, Bitmap};

        Image()
            : _type(Null), _width(0), _height(0), _size(0)
        {
            //printf("+++ IM %p\n", this);
        }

        Image(ImageType type_, int width_, int height_,
              BoundingBox minmax_,
              const char *data_, size_t size_)
            : _type(type_), _width(width_), _height(height_),
              _box(minmax_), _size(size_)
        {
            //printf("+++ IM %p\n", this);
            char * data = new char[size_];
            //printf("+++ IMPTR %p\n", data);
            _data=boost::shared_ptr<char> (data, boost::checked_array_deleter<char>());
            std::copy(data_, data_+size_, _data.get());
        }

        Image(const Image &rhs_)
            : _type(rhs_._type), _width(rhs_._width), _height(rhs_._height),
              _box(rhs_._box), _data(rhs_._data),_size(rhs_._size)
        {
            //printf("+++ IM %p\n", this);
        }

        ~Image() {
            //printf("--- IM %p\n", this);
        }

        Image &operator=(const Image &rhs_)
        {

            if(&rhs_ != this) {
                _type=rhs_._type;
                _width=rhs_._width;
                _height=rhs_._height;
                _box=rhs_._box;
                _data=rhs_._data;
                _size=rhs_._size;
            }

            return *this;
        }

        BoundingBox boundingBox() const
        {
            return _box;
        }

        int rotation() const
        {
            return 0;
        }

        ImageType type() const
        {
            return _type;
        }

        int width() const
        {
            return _width;
        }

        int height() const
        {
            return _height;
        }

        boost::shared_ptr<char> data() const
        {
            return _data;
        }

        size_t size() const
        {
            return _size;
        }

    private:

        ImageType _type;
        int _width;
        int _height;
        BoundingBox _box;
        boost::shared_ptr<char> _data;
        size_t _size;
    };

    typedef boost::shared_ptr< Image > ImageHandle;
    typedef boost::weak_ptr< Image > WeakImageHandle;

}

#endif /* IMAGE_INCL_ */
