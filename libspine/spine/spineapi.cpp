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

/*****************************************************************************
 *
 * spineapi.cpp
 *
 * Copyright 2009 Advanced Interfaces Group
 *
 ****************************************************************************/

#include "spineapi.h"

#include "spine.h"
#include <string>
#include <cstring>
#include "spineapi_internal.h"

using namespace Spine;
using namespace std;

/*****************************************************************************
 *
 * Local preprocessor macros
 *
 * These macros simplify generating repetitve accessor functions and
 * thereby avoid copy/paste/modify typos and errors
 *
 ****************************************************************************/

#define CUR_SCOPE_VALID(SCOPE)                                          \
                                                                        \
    int SpineCursor_##SCOPE##Valid(SpineCursor cur, SpineError *error)  \
    {                                                                   \
        if(cur && cur->_handle) {                                       \
            return cur->_handle->SCOPE() != 0;                          \
        } else {                                                        \
            setError(error, SpineError_InvalidType);                    \
        }                                                               \
        return 0;                                                       \
    }


#define CUR_SCOPE_ADVANCE(SCOPE)                                        \
                                                                        \
    int SpineCursor_advance##SCOPE(SpineCursor cur, SpineCursor_IterateLimit limit, SpineError *error) \
    {                                                                   \
        if(cur && cur->_handle) {                                       \
            return cur->_handle->next##SCOPE(static_cast<Spine::IterateLimit>(limit)) != 0; \
        } else {                                                        \
            setError(error, SpineError_InvalidType);                    \
            return 0;                                                   \
        }                                                               \
    }

#define CUR_SCOPE_RETREAT(SCOPE)                                        \
                                                                        \
    int SpineCursor_retreat##SCOPE(SpineCursor cur, SpineCursor_IterateLimit limit, SpineError *error) \
    {                                                                   \
        if(cur && cur->_handle) {                                       \
            return cur->_handle->previous##SCOPE(static_cast<Spine::IterateLimit>(limit)) != 0; \
        } else {                                                        \
            setError(error, SpineError_InvalidType);                    \
            return 0;                                                   \
        }                                                               \
    }

#define CUR_SCOPE_AREA(SCOPE)                                           \
                                                                        \
    SpineArea SpineCursor_##SCOPE##Area(SpineCursor cur, SpineError *error) \
    {                                                                   \
        SpineArea result;                                               \
        if(cur && cur->_handle && cur->_handle->SCOPE()) {              \
            result.page=cur->_handle->page()->pageNumber();             \
            result.rotation=cur->_handle->SCOPE()->rotation()*90;       \
            BoundingBox b=cur->_handle->SCOPE()->boundingBox();         \
            result.x1=b.x1;                                             \
            result.y1=b.y1;                                             \
            result.x2=b.x2;                                             \
            result.y2=b.y2;                                             \
        }                                                               \
        else {                                                          \
            setError(error, SpineError_InvalidArgument);                \
        }                                                               \
        return result;                                                  \
    }

#define CUR_SCOPE_TEXT(SCOPE)                                           \
                                                                        \
    SpineString SpineCursor_##SCOPE##Text(SpineCursor cur, SpineError *error) \
    {                                                                   \
        if(cur && cur->_handle) {                                       \
            if(cur->_handle->SCOPE()) {                                 \
                return new_SpineStringFromUTF8string(cur->_handle->SCOPE()->text(), error); \
            } else {                                                    \
                return 0;                                               \
            }                                                           \
        } else {                                                        \
            setError(error, SpineError_InvalidType);                    \
        }                                                               \
        return 0;                                                       \
    }

/*****************************************************************************
 *
 * Local utility
 *
 ****************************************************************************/

namespace {

    inline SpineError setError(SpineError *error, SpineError status)
    {
        if(error) {
            *error=status;
        }

        return status;
    }

    inline SpineString new_SpineStringFromUTF8string(const string &str, SpineError *error)
    {
        return new_SpineStringFromUTF8(str.data(), str.length(), error);
    }

    inline string SpineString_asUTF8string(SpineString str, SpineError *error)
    {
        string result;
        if(str) {
            if(str->utf8) {
                result= string(str->utf8, str->length);
            }
        } else {
            setError(error, SpineError_InvalidType);
        }
        return result;
    }

}

/*****************************************************************************
 *
 * SpineError
 *
 ****************************************************************************/

int SpineError_ok(SpineError status) {
    return status==SpineError_NoError;
}

int SpineError_failed(SpineError status) {
    return !SpineError_ok(status);
}

/*****************************************************************************
 *
 * SpineString
 *
 ****************************************************************************/

void delete_SpineString(SpineString *str, SpineError *error)
{
    if(str) {
        if(*str) {
            delete [] (*str)->utf8;
            delete *str;
            *str=0;
        }
    } else {
        setError(error, SpineError_InvalidType);
    }
}

char * SpineString_asUTF8(SpineString str, SpineError *error)
{
    char *result(0);

    if(str) {
        if(str->utf8) {
            result= static_cast<char *>(calloc(sizeof(char), str->length+1));
            copy(str->utf8, str->utf8+str->length, result);
        }
    } else {
        setError(error, SpineError_InvalidType);
    }
    return result;
}

SpineString new_SpineStringFromUTF8(const char *utf8, size_t length, SpineError *error)
{
    SpineString result(0);

    if(utf8) {
        result=new SpineStringImpl;
        result->utf8=new char[length+1];
        std::copy(utf8, utf8+length, result->utf8);
        result->utf8[length]=0;
        result->length= length;
    } else {
        setError(error, SpineError_InvalidArgument);
    }

    return result;
}

SpineString SpineString_copy(SpineString str, SpineError *error)
{
    return new_SpineStringFromUTF8(str->utf8, str->length, error);
}

/*****************************************************************************
 *
 * SpineBuffer
 *
 ****************************************************************************/

void delete_SpineBuffer(SpineBuffer * buf, SpineError *error)
{
    if (buf) {
        if (*buf) {
            delete [] (*buf)->data;
            delete *buf;
            *buf = 0;
        }
    } else {
        setError(error, SpineError_InvalidType);
    }
}

SpineBuffer new_SpineBuffer(const char * data, size_t length, SpineError *error)
{
    SpineBuffer buf(0);

    if (data && length) {
        buf = new SpineBufferImpl;
        buf->data = new char[length + 1];
        buf->data[length] = 0;
        buf->length = length;
        std::copy(data, data + length, buf->data);
    } else {
        setError(error, SpineError_InvalidArgument);
    }

    return buf;
}

SpineBuffer SpineBuffer_copy(SpineBuffer buf, SpineError *error)
{
    return new_SpineBuffer(buf->data, buf->length, error);
}

/*****************************************************************************
 *
 * SpineMap
 *
 ****************************************************************************/

void delete_SpineMap(SpineMap *map, SpineError *error)
{
    if(map) {
        if(*map) {
            for(size_t i(0); i<(*map)->length; ++i) {
                delete_SpineString(&(*map)->keys[i], error);
                delete_SpineString(&(*map)->values[i], error);
            }
            delete [] (*map)->keys;
            delete [] (*map)->values;
            delete *map;
            *map=0;
        }
    } else {
        setError(error, SpineError_InvalidType);
    }
}

SpineMap new_SpineMap(size_t entries, SpineError *error)
{
    SpineMap result=new SpineMapImpl;

    result->length=entries;
    result->keys=new SpineString[entries];
    result->values=new SpineString[entries];
    return result;
}

/*****************************************************************************
 *
 * SpineImage
 *
 ****************************************************************************/

void delete_SpineImage(SpineImage *image, SpineError *error)
{
    if(image) {
        if(*image) {
            delete *image;
            *image=0;
        }
    } else {
        setError(error, SpineError_InvalidType);
    }
}

SpineImage new_SpineImage(SpineError *error)
{
    SpineImage result=new SpineImageImpl;
    result->_handle=Spine::Image();
    return result;
}

int SpineImage_width(SpineImage img, SpineError *error)
{
    return img->_handle.width();
}

int SpineImage_height(SpineImage img, SpineError *error)
{
    return img->_handle.height();
}

int SpineImage_size(SpineImage img, SpineError *error)
{
    return img->_handle.size();
}

Spine_ImageType SpineImage_type(SpineImage img, SpineError *error)
{
    switch(img->_handle.type()) {

    case Spine::Image::RGB:
        return Spine_RGBImage;

    case Spine::Image::JPEG:
        return Spine_JPEGImage;

    case Spine::Image::Bitmap:
        return Spine_BitmapImage;

    default:
        return Spine_NullImage;
    }
}

char *SpineImage_data(SpineImage img, SpineError *error)
{
    return img->_handle.data().get();
}

/*****************************************************************************
 *
 * SpineSet
 *
 ****************************************************************************/

void delete_SpineSet(SpineSet *array, SpineError *error)
{
    if(array) {
        if(*array) {
            for(size_t i(0); i<(*array)->length; ++i) {
                delete_SpineString(&(*array)->values[i], error);
            }
            delete [] (*array)->values;
            delete *array;
            *array=0;
        }
    } else {
        setError(error, SpineError_InvalidType);
    }
}

SpineSet new_SpineSet(size_t entries, SpineError *error)
{
    SpineSet result=new SpineSetImpl;

    result->length=entries;
    result->values=new SpineString[entries];
    return result;
}

/*****************************************************************************
 *
 * SpineTextExtentList
 *
 ****************************************************************************/

SpineTextExtentList new_SpineTextExtentList(size_t entries, SpineError *error)
{
    SpineTextExtentList result = new SpineTextExtentListImpl;

    result->count=entries;
    result->extents=new SpineTextExtent[entries];
    ::memset(result->extents, '\0', entries*sizeof(SpineTextExtent));

    return result;
}

void delete_SpineTextExtentList(SpineTextExtentList *list, SpineError *error)
{
    delete (*list)->extents;
    delete *list;
    *list = 0;
}

/*****************************************************************************
 *
 * SpineAreaList
 *
 ****************************************************************************/

SpineAreaList new_SpineAreaList(size_t entries, SpineError *error)
{
    SpineAreaList result = new SpineAreaListImpl;

    result->count=entries;
    result->areas=new SpineArea[entries];
    ::memset(result->areas, '\0', entries*sizeof(SpineArea));

    return result;
}

void delete_SpineAreaList(SpineAreaList *list, SpineError *error)
{
    delete (*list)->areas;
    delete *list;
    *list = 0;
}

/*****************************************************************************
 *
 * SpineAnnotationList
 *
 ****************************************************************************/

SpineAnnotationList new_SpineAnnotationList(size_t entries, SpineError *error)
{
    SpineAnnotationList result = new SpineAnnotationListImpl;

    result->count=entries;
    result->annotations=new SpineAnnotation[entries];
    ::memset(result->annotations, '\0', entries*sizeof(SpineAnnotation));

    return result;
}

void delete_SpineAnnotationList(SpineAnnotationList *list, SpineError *error)
{
    delete (*list)->annotations;
    delete *list;
    *list = 0;
}

/*****************************************************************************
 *
 * TextExtent
 *
 ****************************************************************************/

SpineTextExtent new_SpineTextExtent(SpineCursor from, SpineCursor to, SpineError *error)
{
    SpineTextExtent result=new SpineTextExtentImpl;
    result->_handle=Spine::TextExtentHandle(new Spine::TextExtent(from->_handle, to->_handle));
    return result;
}

void delete_SpineTextExtent(SpineTextExtent *se, SpineError *error)
{
    if(se) {
        delete *se;
        *se=0;
    } else {
        setError(error, SpineError_InvalidType);
    }
}

SpineCursor SpineTextExtent_from(SpineTextExtent se, SpineError *error)
{
    SpineCursor result=new SpineCursorImpl;
    result->_handle=se->_handle->first.cursor();
    return result;
}

SpineCursor SpineTextExtent_to(SpineTextExtent se, SpineError *error)
{
    SpineCursor result=new SpineCursorImpl;
    result->_handle=se->_handle->second.cursor();
    return result;
}

SpineString SpineTextExtent_text(SpineTextExtent se, SpineError *error)
{
    return new_SpineStringFromUTF8string(se->_handle->text(), error);
}

SpineAreaList SpineTextExtent_areas(SpineTextExtent se, SpineError *error)
{
    Spine::AreaList areas = se->_handle->areas();
    SpineAreaList list = new_SpineAreaList(areas.size(), error);
    size_t j=0;
    Spine::AreaList::const_iterator i(areas.begin());
    Spine::AreaList::const_iterator i_end(areas.end());
    for (; i != i_end; ++i, ++j)
    {
        SpineArea & sbb=list->areas[j];
        sbb.page=i->page;
        sbb.rotation=i->orientation*90;
        sbb.x1=i->boundingBox.x1;
        sbb.y1=i->boundingBox.y1;
        sbb.x2=i->boundingBox.x2;
        sbb.y2=i->boundingBox.y2;
    }
    return list;
}

SpineTextExtentList SpineTextExtent_search(SpineTextExtent se, SpineString regex_, int options, SpineError *error)
{
    SpineTextExtentList list(0);

    if (se) {

        string regex(SpineString_asUTF8string(regex_, error));
        if(SpineError_ok(*error)) {

            try {
                Spine::TextExtentSet extents(se->_handle->search(regex, options));

                list = new_SpineTextExtentList(extents.size(), error);

                if (SpineError_ok(*error)) {
                    Spine::TextExtentSet::const_iterator i(extents.begin());
                    Spine::TextExtentSet::const_iterator i_end(extents.end());
                    size_t j = 0;
                    while(i != i_end && SpineError_ok(*error)) {
                        list->extents[j] = copy_SpineTextExtent(*i, error);
                        ++i; ++j;
                    }
                }

            }
            catch (Spine::TextExtent::regex_exception e) {
                setError(error, SpineError_InvalidRegex);
            }
        }

    } else {
        setError(error, SpineError_InvalidType);
    }

    return list;
}

/*****************************************************************************
 *
 * Annotation
 *
 ****************************************************************************/

SpineAnnotation new_SpineAnnotation(SpineError *error)
{
    SpineAnnotation result=new SpineAnnotationImpl;
    result->_handle=AnnotationHandle(new Annotation);
    return result;
}

SpineAnnotation new_SpineAnnotation(AnnotationHandle ann, SpineError *error)
{
    SpineAnnotation result=new SpineAnnotationImpl;
    result->_handle=ann;
    return result;
}

void delete_SpineAnnotation(SpineAnnotation *sa, SpineError *error)
{
    if(sa) {
        delete *sa;
        *sa=0;
    } else {
        setError(error, SpineError_InvalidType);
    }
}

void SpineAnnotation_insertProperty(SpineAnnotation sa, const SpineString key, const SpineString val, SpineError *error)
{
    if(!sa || !key || !val || !key->utf8 || !val->utf8) {
        setError(error, SpineError_InvalidType);
    } else {
        sa->_handle->setProperty(SpineString_asUTF8string(key, error),
                                 SpineString_asUTF8string(val, error));
    }
}

void SpineAnnotation_setProperty(SpineAnnotation sa, const SpineString key, const SpineString val, SpineError *error)
{
    if(!sa || !key || !val || !key->utf8 || !val->utf8) {
        setError(error, SpineError_InvalidType);
    } else {
        // clear key
        sa->_handle->removeProperty(SpineString_asUTF8string(key, error), "");

        // set key
        sa->_handle->setProperty(SpineString_asUTF8string(key, error),
                                 SpineString_asUTF8string(val, error));
    }
}

SpineString SpineAnnotation_getFirstProperty(SpineAnnotation sa, const SpineString key, SpineError *error)
{
    if(!sa || !key || !key->utf8) {
        setError(error, SpineError_InvalidType);
        return 0;
    }

    return new_SpineStringFromUTF8string(sa->_handle->getFirstProperty(SpineString_asUTF8string(key, error)), error);
}

int SpineAnnotation_hasProperty(SpineAnnotation sa, const SpineString key, SpineError *error)
{
    if(!sa || !key || !key->utf8) {
        setError(error, SpineError_InvalidType);
        return 0;
    }

    return sa->_handle->hasProperty(SpineString_asUTF8string(key, error)) ? 1 : 0;
}

SpineSet SpineAnnotation_getProperty(SpineAnnotation sa, const SpineString key, SpineError *error)
{
    if(!sa || !key || !key->utf8) {
        setError(error, SpineError_InvalidType);
        return 0;
    }

    multimap<string,string> mmap(sa->_handle->properties());
    string ukey(SpineString_asUTF8string(key, error));
    pair<multimap<string,string>::iterator, multimap<string,string>::iterator> values(mmap.equal_range(ukey));

    size_t instances(0);
    multimap<string,string>::iterator i;
    for(i=values.first; i!=values.second; ++i) {
        ++instances;
    }

    SpineSet result=new_SpineSet(instances, error);

    size_t idx(0);
    for (i=values.first; i!=values.second; ++i) {
        result->values[idx++]= new_SpineStringFromUTF8string(i->second, error);
    }

    return result;
}

void SpineAnnotation_removePropertyAll(SpineAnnotation sa, const SpineString key, SpineError *error)
{
    if(!sa || !key || !key->utf8) {
        setError(error, SpineError_InvalidType);
        return;
    }

    sa->_handle->removeProperty(SpineString_asUTF8string(key, error));
}

void SpineAnnotation_removeProperty(SpineAnnotation sa, const SpineString key, const SpineString value, SpineError *error)
{
    if(!sa || !key || !key->utf8) {
        setError(error, SpineError_InvalidType);
        return;
    }

    sa->_handle->removeProperty(SpineString_asUTF8string(key, error),
                                SpineString_asUTF8string(value, error));
}

SpineMap SpineAnnotation_properties(SpineAnnotation sa, SpineError *error)
{
    multimap<string,string> properties(sa->_handle->properties());

    SpineMap result=new_SpineMap(properties.size(), error);
    if(result) {
        size_t idx(0);
        std::multimap<string,string>::iterator i;
        for (i=properties.begin(); i!=properties.end(); ++i) {
            result->keys[idx]= new_SpineStringFromUTF8string(i->first, error);
            result->values[idx]= new_SpineStringFromUTF8string(i->second, error);
            ++idx;
        }
    }
    return result;
}

void SpineAnnotation_clearProperties(SpineAnnotation sa, SpineError *error)
{
    sa->_handle->clearProperties();
}

SpineTextExtentList SpineAnnotation_extents(SpineAnnotation sa, SpineError *error)
{
    if (sa) {
        Spine::TextExtentSet extents = sa->_handle->extents();
        SpineTextExtentList list = new_SpineTextExtentList(extents.size(), error);

        if (SpineError_ok(*error))
        {
            Spine::TextExtentSet::const_iterator i(extents.begin());
            Spine::TextExtentSet::const_iterator i_end(extents.end());
            int j = 0;
            for (; i != i_end && SpineError_ok(*error); ++i, ++j)
            {
                list->extents[j] = copy_SpineTextExtent(*i, error);
            }
            if (SpineError_ok(*error))
            {
                return list;
            }
        }
    } else {
        setError(error, SpineError_InvalidType);
    }
    return 0;
}

SpineAreaList SpineAnnotation_areas(SpineAnnotation sa, SpineError *error)
{
    Spine::AreaSet areas = sa->_handle->areas();
    SpineAreaList list = new_SpineAreaList(areas.size(), error);
    size_t j=0;
    Spine::AreaSet::const_iterator i(areas.begin());
    Spine::AreaSet::const_iterator i_end(areas.end());
    for (; i != i_end; ++i, ++j)
    {
        SpineArea & sbb=list->areas[j];
        sbb.page=i->page;
        sbb.rotation=i->orientation*90;
        sbb.x1=i->boundingBox.x1;
        sbb.y1=i->boundingBox.y1;
        sbb.x2=i->boundingBox.x2;
        sbb.y2=i->boundingBox.y2;
    }
    return list;
}

void SpineAnnotation_addArea(SpineAnnotation sa, SpineArea pb, SpineError *error)
{
    sa->_handle->addArea(Spine::Area(pb.page, pb.rotation/90, Spine::BoundingBox(pb.x1, pb.y1, pb.x2, pb.y2)));
}

void SpineAnnotation_removeArea(SpineAnnotation sa, SpineArea pb, SpineError *error)
{
    sa->_handle->removeArea(Spine::Area(pb.page, pb.rotation/90, Spine::BoundingBox(pb.x1, pb.y1, pb.x2, pb.y2)));
}

void SpineAnnotation_addExtent(SpineAnnotation sa, SpineTextExtent se, SpineError *error)
{
    sa->_handle->addExtent(se->_handle);
}

void SpineAnnotation_removeExtent(SpineAnnotation sa, SpineTextExtent se, SpineError *error)
{
    sa->_handle->removeExtent(se->_handle);
}

int SpineAnnotation_equal(SpineAnnotation sa1, SpineAnnotation sa2, SpineError *error)
{
    return *(sa1->_handle) == *(sa2->_handle);
}
int SpineAnnotation_equalRegions(SpineAnnotation sa1, SpineAnnotation sa2, SpineError *error)
{
    return sa1->_handle->equalRegions(*(sa2->_handle));
}

int SpineAnnotation_isPublic(SpineAnnotation sa, SpineError *error)
{
    return sa->_handle->isPublic() ? 1 : 0;
}

void SpineAnnotation_setPublic(SpineAnnotation sa, int isPublic, SpineError *error)
{
    sa->_handle->setPublic(isPublic != 0);
}

/*****************************************************************************
 *
 * Document
 *
 ****************************************************************************/

void delete_SpineDocument(SpineDocument *doc, SpineError *error)
{
    if(doc) {
        delete *doc;
        *doc=0;
    } else {
        setError(error, SpineError_InvalidType);
    }
}

SpineDocument new_SpineDocument(SpineError *error)
{
    SpineDocument result=new SpineDocumentImpl;
    return result;
}

int SpineDocument_valid(SpineDocument doc, SpineError *error)
{
    if(doc && doc->_handle) {
        return doc->_handle->isOK();
    } else {
        setError(error, SpineError_InvalidType);
    }
    return 0;
}

const char *SpineDocument_errorString(SpineDocument doc, SpineError *error)
{
    if(doc) {
        return doc->_handle->errorString();
    } else {
        setError(error, SpineError_InvalidType);
    }
    return 0;
}

SpineDocument_ViewMode SpineDocument_viewMode(SpineDocument doc, SpineError *error)
{
    SpineDocument_ViewMode result=SpineDocument_ViewDefault;

    if(doc) {
        switch (doc->_handle->viewMode()) {

        case Document::ViewNone:
            result=SpineDocument_ViewDefault;
            break;
        case Document::ViewOutlines:
            result=SpineDocument_ViewOutlines;
            break;
        case Document::ViewThumbs:
            result=SpineDocument_ViewThumbs;
            break;
        case Document::ViewFullScreen:
            result=SpineDocument_ViewFullScreen;
            break;
        case Document::ViewOC:
            result=SpineDocument_ViewOC;
            break;
        case Document::ViewAttach:
            result=SpineDocument_ViewAttach;
            break;
        }
    } else {
        setError(error, SpineError_InvalidType);
    }
    return result;
}

SpineDocument_PageLayout SpineDocument_pageLayout(SpineDocument doc, SpineError *error)
{
    SpineDocument_PageLayout result=SpineDocument_LayoutDefault;
    if(doc) {
        switch (doc->_handle->pageLayout()) {
        case Document::LayoutNone:
            result=SpineDocument_LayoutDefault;
            break;
        case Document::LayoutSinglePage:
            result=SpineDocument_LayoutSinglePage;
            break;
        case Document::LayoutOneColumn:
            result=SpineDocument_LayoutOneColumn;
            break;
        case Document::LayoutTwoColumnLeft:
            result=SpineDocument_LayoutTwoColumnLeft;
            break;
        case Document::LayoutTwoColumnRight:
            result=SpineDocument_LayoutTwoColumnRight;
            break;
        case Document::LayoutTwoPageLeft:
            result=SpineDocument_LayoutTwoPageLeft;
            break;
        case Document::LayoutTwoPageRight:
            result=SpineDocument_LayoutTwoPageRight;
            break;
        }
    } else {
        setError(error, SpineError_InvalidType);
    }
    return result;
}

SpineCursor SpineDocument_newCursor(SpineDocument doc, int page, SpineError *error)
{
    SpineCursor result(0);
    if(doc) {
        result=new SpineCursorImpl;
        result->_handle=doc->_handle->newCursor(page);
    } else {
        setError(error, SpineError_InvalidType);
    }
    return result;
}

SpineBuffer SpineDocument_data(SpineDocument doc, SpineError *error)
{
    if(doc) {
        std::string data(doc->_handle->data());
        return new_SpineBuffer(data.c_str(), data.length(), 0);
    } else {
        setError(error, SpineError_InvalidType);
    }
    return 0;
}

SpineString SpineDocument_pmid(SpineDocument doc, SpineError *error)
{
    if(doc) {
        return new_SpineStringFromUTF8string(doc->_handle->pmid(), error);
    } else {
        setError(error, SpineError_InvalidType);
    }
    return 0;
}

SpineString SpineDocument_doi(SpineDocument doc, SpineError *error)
{
    if(doc) {
        return new_SpineStringFromUTF8string(doc->_handle->doi(), error);
    } else {
        setError(error, SpineError_InvalidType);
    }
    return 0;
}

SpineString SpineDocument_pii(SpineDocument doc, SpineError *error)
{
    if(doc) {
        return new_SpineStringFromUTF8string(doc->_handle->pii(), error);
    } else {
        setError(error, SpineError_InvalidType);
    }
    return 0;
}

SpineString SpineDocument_uniqueID(SpineDocument doc, SpineError *error)
{
    if(doc) {
        return new_SpineStringFromUTF8string(doc->_handle->uniqueID(), error);
    } else {
        setError(error, SpineError_InvalidType);
    }
    return 0;
}

SpineString SpineDocument_filehash(SpineDocument doc, SpineError *error)
{
    if(doc) {
        return new_SpineStringFromUTF8string(doc->_handle->filehash(), error);
    } else {
        setError(error, SpineError_InvalidType);
    }
    return 0;
}

SpineSet SpineDocument_fingerprints(SpineDocument doc, SpineError *error)
{
    if(!doc) {
        setError(error, SpineError_InvalidType);
        return 0;
    }

    Spine::Document::FingerprintSet s(doc->_handle->fingerprints());
    Spine::Document::FingerprintSet::iterator i;

    SpineSet result=new_SpineSet(s.size(), error);

    size_t idx(0);
    for (i=s.begin(); i!=s.end(); ++i) {
        result->values[idx++]= new_SpineStringFromUTF8string(*i, error);
    }

    return result;
}

SpineString SpineDocument_iri(SpineDocument doc, SpineError *error)
{
    if(doc) {
        return new_SpineStringFromUTF8string(doc->_handle->iri(), error);
    } else {
        setError(error, SpineError_InvalidType);
    }
    return 0;
}

size_t SpineDocument_numberOfPages(SpineDocument doc, SpineError *error)
{
    if(doc) {
        return doc->_handle->numberOfPages();
    } else {
        setError(error, SpineError_InvalidType);
    }
    return 0;
}

size_t SpineDocument_wordCount(SpineDocument doc, SpineError *error)
{
    if(doc) {
        return doc->_handle->wordCount();
    } else {
        setError(error, SpineError_InvalidType);
    }
    return 0;
}

SpineString SpineDocument_title(SpineDocument doc, SpineError *error)
{
    if(doc) {
        return new_SpineStringFromUTF8string(doc->_handle->title(), error);
    } else {
        setError(error, SpineError_InvalidType);
    }
    return 0;
}

SpineString SpineDocument_subject(SpineDocument doc, SpineError *error)
{
    if(doc) {
        return new_SpineStringFromUTF8string(doc->_handle->subject(), error);
    } else {
        setError(error, SpineError_InvalidType);
    }
    return 0;
}

SpineString SpineDocument_keywords(SpineDocument doc, SpineError *error)
{
    if(doc) {
        return new_SpineStringFromUTF8string(doc->_handle->keywords(), error);
    } else {
        setError(error, SpineError_InvalidType);
    }
    return 0;
}

SpineString SpineDocument_author(SpineDocument doc, SpineError *error)
{
    if(doc) {
        return new_SpineStringFromUTF8string(doc->_handle->author(), error);
    } else {
        setError(error, SpineError_InvalidType);
    }
    return 0;
}

SpineString SpineDocument_creator(SpineDocument doc, SpineError *error)
{
    if(doc) {
        return new_SpineStringFromUTF8string(doc->_handle->creator(), error);
    } else {
        setError(error, SpineError_InvalidType);
    }
    return 0;
}

SpineString SpineDocument_producer(SpineDocument doc, SpineError *error)
{
    if(doc) {
        return new_SpineStringFromUTF8string(doc->_handle->producer(), error);
    } else {
        setError(error, SpineError_InvalidType);
    }
    return 0;
}

SpineString SpineDocument_newScratchId(SpineDocument doc, SpineError *error)
{
    if(doc) {
        return new_SpineStringFromUTF8string(doc->_handle->newScratchId(), error);
    } else {
        setError(error, SpineError_InvalidType);
    }
    return 0;
}

SpineString SpineDocument_deletedItemsScratchId(SpineDocument doc, SpineError *error)
{
    if(doc) {
        return new_SpineStringFromUTF8string(doc->_handle->deletedItemsScratchId(), error);
    } else {
        setError(error, SpineError_InvalidType);
    }
    return 0;
}

void * SpineDocument_userdef(SpineDocument doc, SpineError *error)
{
    if(doc) {
        return doc->_handle->userdef();
    } else {
        setError(error, SpineError_InvalidType);
    }
    return 0;
}

void SpineDocument_userdefSet(SpineDocument doc, void *userdef, SpineError *error)
{
    if(doc) {
        doc->_handle->setUserdef(userdef);
    } else {
        setError(error, SpineError_InvalidType);
    }
}

time_t SpineDocument_creationDate(SpineDocument doc, SpineError *error)
{
    if(doc) {
        return doc->_handle->creationDate();
    } else {
        setError(error, SpineError_InvalidType);
    }
    return 0;
}

time_t SpineDocument_modificationDate(SpineDocument doc, SpineError *error)
{
    if(doc) {
        return doc->_handle->modificationDate();
    } else {
        setError(error, SpineError_InvalidType);
    }
    return 0;
}

int SpineDocument_imageBased(SpineDocument doc, SpineError *error)
{
    if(doc) {
        return doc->_handle->imageBased();
    } else {
        setError(error, SpineError_InvalidType);
    }
    return 0;
}

SpineImage SpineDocument_render(SpineDocument doc, int pageNumber, float resolution, SpineError *error)
{
    SpineImage result=new SpineImageImpl;
    result->_handle=doc->_handle->render(pageNumber, resolution);
    return result;
}

SpineImage SpineDocument_renderArea(SpineDocument doc, SpineArea pb, float resolution, SpineError *error)
{
    SpineImage result=new SpineImageImpl;
    result->_handle=doc->_handle->renderArea(Spine::Area(pb.page, pb.rotation/90, Spine::BoundingBox(pb.x1, pb.y1, pb.x2, pb.y2)), resolution);
    return result;
}

SpineAreaList SpineDocument_areaSelection(SpineDocument doc, SpineError *error)
{
    Spine::AreaSet areas = doc->_handle->areaSelection();
    SpineAreaList list = new_SpineAreaList(areas.size(), error);
    size_t j=0;
    Spine::AreaSet::const_iterator i(areas.begin());
    Spine::AreaSet::const_iterator i_end(areas.end());
    for (; i != i_end; ++i, ++j)
    {
        SpineArea & sbb=list->areas[j];
        sbb.page=i->page;
        sbb.rotation=i->orientation*90;
        sbb.x1=i->boundingBox.x1;
        sbb.y1=i->boundingBox.y1;
        sbb.x2=i->boundingBox.x2;
        sbb.y2=i->boundingBox.y2;
    }
    return list;
}

SpineTextExtentList SpineDocument_textSelection(SpineDocument doc, SpineError *error)
{
    if (doc) {
        Spine::TextExtentSet extents = doc->_handle->textSelection();
        SpineTextExtentList list = new_SpineTextExtentList(extents.size(), error);

        if (SpineError_ok(*error))
        {
            Spine::TextExtentSet::const_iterator i(extents.begin());
            Spine::TextExtentSet::const_iterator i_end(extents.end());
            int j = 0;
            for (; i != i_end && SpineError_ok(*error); ++i, ++j)
            {
                list->extents[j] = copy_SpineTextExtent(*i, error);
            }
            if (SpineError_ok(*error))
            {
                return list;
            }
        }
    } else {
        setError(error, SpineError_InvalidType);
    }
    return 0;
}

SpineTextExtentList SpineDocument_search(SpineDocument doc, SpineString regex_, int options, SpineError *error)
{
    SpineTextExtentList list(0);

    if (doc) {

        string regex(SpineString_asUTF8string(regex_, error));
        if(SpineError_ok(*error)) {

            try {
                Spine::TextExtentSet extents(doc->_handle->search(
                                                 regex, options));

                list = new_SpineTextExtentList(extents.size(), error);

                if (SpineError_ok(*error)) {
                    Spine::TextExtentSet::const_iterator i(extents.begin());
                    Spine::TextExtentSet::const_iterator i_end(extents.end());

                    size_t j = 0;
                    while(i != i_end && SpineError_ok(*error)) {
                        list->extents[j] = copy_SpineTextExtent(*i, error);
                        ++i; ++j;
                    }
                }
            }
            catch (Spine::TextExtent::regex_exception)
            {
                setError(error, SpineError_InvalidRegex);
            }
        }

    } else {
        setError(error, SpineError_InvalidType);
    }

    return list;
}

SpineTextExtentList SpineDocument_searchFrom(SpineDocument doc, SpineCursor from, SpineString regex_, int options, SpineError *error)
{

    SpineTextExtentList list(0);

    if (doc) {

        string regex(SpineString_asUTF8string(regex_, error));
        if(SpineError_ok(*error)) {

            try {
                Spine::TextExtentSet extents(doc->_handle->searchFrom(
                                                 from->_handle, regex, options));

                list = new_SpineTextExtentList(extents.size(), error);

                if (SpineError_ok(*error)) {
                    Spine::TextExtentSet::const_iterator i(extents.begin());
                    Spine::TextExtentSet::const_iterator i_end(extents.end());

                    size_t j = 0;
                    while(i != i_end && SpineError_ok(*error)) {
                        list->extents[j] = copy_SpineTextExtent(*i, error);
                        ++i; ++j;
                    }
                }
            }
            catch (Spine::TextExtent::regex_exception)
            {
                setError(error, SpineError_InvalidRegex);
            }
        }

    } else {
        setError(error, SpineError_InvalidType);
    }

    return list;
}

SpineString SpineDocument_text(SpineDocument doc, SpineError *error)
{
    if (doc) {
        return new_SpineStringFromUTF8string(doc->_handle->text(),
                                             error);
    } else {
        setError(error, SpineError_InvalidType);
    }

    return 0;
}

SpineTextExtent SpineDocument_substr(SpineDocument doc, int from, int len, SpineError *error)
{
    if (doc) {
        return copy_SpineTextExtent(doc->_handle->substr(from, len),
                                    error);
    } else {
        setError(error, SpineError_InvalidType);
    }

    return 0;
}

static SpineAnnotationList _SpineDocument_annotations(SpineDocument doc, string listName, SpineError *error)
{
    std::set< Spine::AnnotationHandle > annotations = doc->_handle->annotations(listName);
    SpineAnnotationList list = new_SpineAnnotationList(annotations.size(), error);
    size_t j=0;
    std::set< Spine::AnnotationHandle >::const_iterator i(annotations.begin());
    std::set< Spine::AnnotationHandle >::const_iterator i_end(annotations.end());
    for (; i != i_end; ++i, ++j)
    {
        list->annotations[j] = new_SpineAnnotation(*i, error);
    }
    return list;
}

SpineAnnotationList SpineDocument_annotations(SpineDocument doc, SpineError *error)
{
    return _SpineDocument_annotations(doc, "", error);
}

SpineAnnotationList SpineDocument_scratchAnnotations(SpineDocument doc, SpineString listName, SpineError *error)
{
    return _SpineDocument_annotations(doc, SpineString_asUTF8string(listName, error), error);
}

static void _SpineDocument_addAnnotation(SpineDocument doc, SpineAnnotation sa, string listName, SpineError *error)
{
    doc->_handle->addAnnotation(sa->_handle, listName);
}

static void _SpineDocument_addAnnotations(SpineDocument doc, SpineAnnotationList sas, string listName, SpineError *error)
{
    std::set< Spine::AnnotationHandle > anns;
    for (size_t index = 0; index < sas->count; ++index)
    {
        anns.insert(sas->annotations[index]->_handle);
    }
    doc->_handle->addAnnotations(anns, listName);
}

void SpineDocument_addAnnotation(SpineDocument doc, SpineAnnotation sa, SpineError *error)
{
    _SpineDocument_addAnnotation(doc, sa, "", error);
}

void SpineDocument_addAnnotations(SpineDocument doc, SpineAnnotationList sas, SpineError *error)
{
    _SpineDocument_addAnnotations(doc, sas, "", error);
}

void SpineDocument_addScratchAnnotation(SpineDocument doc, SpineAnnotation sa, SpineString listName, SpineError *error)
{
    _SpineDocument_addAnnotation(doc, sa, SpineString_asUTF8string(listName, error), error);
}

void SpineDocument_addScratchAnnotations(SpineDocument doc, SpineAnnotationList sas, SpineString listName, SpineError *error)
{
    _SpineDocument_addAnnotations(doc, sas, SpineString_asUTF8string(listName, error), error);
}

static void _SpineDocument_removeAnnotation(SpineDocument doc, SpineAnnotation sa, string listName, SpineError *error)
{
    doc->_handle->removeAnnotation(sa->_handle, listName);
}

void SpineDocument_removeAnnotation(SpineDocument doc, SpineAnnotation sa, SpineError *error)
{
    _SpineDocument_removeAnnotation(doc, sa, "", error);
}

void SpineDocument_removeScratchAnnotation(SpineDocument doc, SpineAnnotation sa, SpineString listName, SpineError *error)
{
    _SpineDocument_removeAnnotation(doc, sa, SpineString_asUTF8string(listName, error), error);
}

SpineTextExtent SpineDocument_resolveExtent(SpineDocument doc, int page1, double x1, double y1, int page2, double x2, double y2, SpineError *error)
{
    SpineTextExtent result=new SpineTextExtentImpl;
    result->_handle=doc->_handle->resolveExtent(page1, x1, y1, page2, x2, y2);
    return result;
}

/*****************************************************************************
 *
 * Cursor
 *
 ****************************************************************************/


void delete_SpineCursor(SpineCursor *cur, SpineError *error)
{
    if(cur) {
        delete *cur;
        *cur=0;
    } else {
        setError(error, SpineError_InvalidType);
    }
}

SpineCursor SpineCursor_copy(SpineCursor cur, SpineError *error)
{
    SpineCursor result(0);

    if(cur && cur->_handle) {
        result=new SpineCursorImpl;
        result->_handle=cur->_handle->clone();
    } else {
        setError(error, SpineError_InvalidType);
    }
    return result;
}

CUR_SCOPE_TEXT(page)
CUR_SCOPE_VALID(page)
CUR_SCOPE_AREA(page)
CUR_SCOPE_ADVANCE(Page)
CUR_SCOPE_RETREAT(Page)


void SpineCursor_gotoPage(SpineCursor cur, int page, SpineError *error)
{
    if(cur && cur->_handle) {
        cur->_handle->gotoPage(page);
    } else {
        setError(error, SpineError_InvalidType);
    }
}

/*****************************************************************************
 *
 * Cursor/Region
 *
 ****************************************************************************/

CUR_SCOPE_TEXT(region)
CUR_SCOPE_VALID(region)
CUR_SCOPE_AREA(region)
CUR_SCOPE_ADVANCE(Region)
CUR_SCOPE_RETREAT(Region)



/*****************************************************************************
 *
 * Cursor/Block
 *
 ****************************************************************************/

CUR_SCOPE_TEXT(block)
CUR_SCOPE_VALID(block)
CUR_SCOPE_AREA(block)
CUR_SCOPE_ADVANCE(Block)
CUR_SCOPE_RETREAT(Block)


/*****************************************************************************
 *
 * Cursor/Line
 *
 ****************************************************************************/

CUR_SCOPE_TEXT(line)
CUR_SCOPE_VALID(line)
CUR_SCOPE_AREA(line)
CUR_SCOPE_ADVANCE(Line)
CUR_SCOPE_RETREAT(Line)


/*****************************************************************************
 *
 * Cursor/Word
 *
 ****************************************************************************/

CUR_SCOPE_TEXT(word)
CUR_SCOPE_VALID(word)
CUR_SCOPE_AREA(word)
CUR_SCOPE_ADVANCE(Word)
CUR_SCOPE_RETREAT(Word)


SpineString SpineCursor_wordFontName(SpineCursor cur, SpineError *error)
{
    string result;
    if(cur && cur->_handle && cur->_handle->word()) {
        result=cur->_handle->word()->fontName();
    } else {
        setError(error, SpineError_InvalidType);
    }
    return new_SpineStringFromUTF8string(result, error);
}

double SpineCursor_wordFontSize(SpineCursor cur, SpineError *error)
{
    if(cur && cur->_handle && cur->_handle->word()) {
        return cur->_handle->word()->fontSize();
    } else {
        setError(error, SpineError_InvalidType);
    }
    return 0;
}

int SpineCursor_wordSpaceAfter(SpineCursor cur, SpineError *error)
{
    if(cur && cur->_handle && cur->_handle->word()) {
        return cur->_handle->word()->spaceAfter();
    } else {
        setError(error, SpineError_InvalidType);
    }
    return 0;
}

int SpineCursor_wordUnderlined(SpineCursor cur, SpineError *error)
{
    if(cur && cur->_handle && cur->_handle->word()) {
        return cur->_handle->word()->underlined();
    } else {
        setError(error, SpineError_InvalidType);
    }
    return 0;
}

double SpineCursor_wordBaseline(SpineCursor cur, SpineError *error)
{
    if(cur && cur->_handle && cur->_handle->word()) {
        return cur->_handle->word()->baseline();
    } else {
        setError(error, SpineError_InvalidType);
    }
    return 0;
}

SpineColor SpineCursor_wordColor(SpineCursor cur, SpineError *error)
{
    SpineColor result={0,0,0};
    if(cur && cur->_handle && cur->_handle->word()) {
        Spine::Color c=cur->_handle->word()->color();
        result.r=c.r;
        result.g=c.g;
        result.b=c.b;
    } else {
        setError(error, SpineError_InvalidType);
    }
    return result;
}


/*****************************************************************************
 *
 * Cursor/Character
 *
 ****************************************************************************/

CUR_SCOPE_TEXT(character)
CUR_SCOPE_VALID(character)
CUR_SCOPE_AREA(character)
CUR_SCOPE_ADVANCE(Character)
CUR_SCOPE_RETREAT(Character)


SpineString SpineCursor_characterFontName(SpineCursor cur, SpineError *error)
{
    string result;
    if(cur && cur->_handle && cur->_handle->character()) {
        result=cur->_handle->character()->fontName();
    } else {
        setError(error, SpineError_InvalidType);
    }
    return new_SpineStringFromUTF8string(result, error);
}

double SpineCursor_characterFontSize(SpineCursor cur, SpineError *error)
{
    if(cur && cur->_handle && cur->_handle->character()) {
        return cur->_handle->character()->fontSize();
    } else {
        setError(error, SpineError_InvalidType);
    }
    return 0;
}

int SpineCursor_characterSpaceAfter(SpineCursor cur, SpineError *error)
{
    if(cur && cur->_handle && cur->_handle->character()) {
        return cur->_handle->character()->spaceAfter();
    } else {
        setError(error, SpineError_InvalidType);
    }
    return 0;
}

int SpineCursor_characterUnderlined(SpineCursor cur, SpineError *error)
{
    if(cur && cur->_handle && cur->_handle->character()) {
        return cur->_handle->character()->underlined();
    } else {
        setError(error, SpineError_InvalidType);
    }
    return 0;
}

double SpineCursor_characterBaseline(SpineCursor cur, SpineError *error)
{
    if(cur && cur->_handle && cur->_handle->character()) {
        return cur->_handle->character()->baseline();
    } else {
        setError(error, SpineError_InvalidType);
    }
    return 0;
}

SpineColor SpineCursor_characterColor(SpineCursor cur, SpineError *error)
{
    SpineColor result={0,0,0};
    if(cur && cur->_handle && cur->_handle->character()) {
        Spine::Color c=cur->_handle->character()->color();
        result.r=c.r;
        result.g=c.g;
        result.b=c.b;
    } else {
        setError(error, SpineError_InvalidType);
    }
    return result;
}

/*****************************************************************************
 *
 * Cursor/Image
 *
 ****************************************************************************/

CUR_SCOPE_VALID(image)
CUR_SCOPE_AREA(image)
CUR_SCOPE_ADVANCE(Image)
CUR_SCOPE_RETREAT(Image)

SpineImage SpineCursor_image(SpineCursor cur, SpineError *error)
{
    if(cur && cur->_handle) {

        if(cur->_handle->image()) {
            SpineImage result=new SpineImageImpl;
            result->_handle=*(cur->_handle->image());
            return result;
        } else {
            return 0;
        }

    } else {
        setError(error, SpineError_InvalidType);
    }
    return 0;
}
