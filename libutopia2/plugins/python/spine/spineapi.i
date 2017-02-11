/*****************************************************************************
 *  
 *   This file is part of the Utopia Documents application.
 *       Copyright (c) 2008-2017 Lost Island Labs
 *           <info@utopiadocs.com>
 *   
 *   Utopia Documents is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU GENERAL PUBLIC LICENSE VERSION 3 as
 *   published by the Free Software Foundation.
 *   
 *   Utopia Documents is distributed in the hope that it will be useful, but
 *   WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 *   Public License for more details.
 *   
 *   In addition, as a special exception, the copyright holders give
 *   permission to link the code of portions of this program with the OpenSSL
 *   library under certain conditions as described in each individual source
 *   file, and distribute linked combinations including the two.
 *   
 *   You must obey the GNU General Public License in all respects for all of
 *   the code used other than OpenSSL. If you modify file(s) with this
 *   exception, you may extend this exception to your version of the file(s),
 *   but you are not obligated to do so. If you do not wish to do so, delete
 *   this exception statement from your version.
 *   
 *   You should have received a copy of the GNU General Public License
 *   along with Utopia Documents. If not, see <http://www.gnu.org/licenses/>
 *  
 *****************************************************************************/

/* Swig Python interface to spine documents */
%feature("autodoc", "0");
%module (docstring="Utopia Documents Spine API") spineapi

%{

#include <spine/spineapi.h>
#include "pyspineapi.h"

    static int check_exception(SpineError e) {
        switch (e) {
        case SpineError_NoError:
            break;
        case SpineError_IO:
            PyErr_SetString(PyExc_IOError,"Document cannot be loaded");
            return 1;
        case SpineError_InvalidType:
            PyErr_SetString(PyExc_TypeError,"Invalid object or object in invalid state");
            return 1;
        case SpineError_InvalidArgument:
            PyErr_SetString(PyExc_ValueError,"Invalid argument");
            return 1;
        case SpineError_InvalidRegex:
            PyErr_SetString(PyExc_ValueError,"Invalid regular expression");
            return 1;
        case SpineError_Unknown:
            PyErr_SetString(PyExc_RuntimeError,"Unknown error accessing document");
            return 1;
        }
        return 0;
    }

    SpineCursor_IterateLimit spinelimit(int limit, SpineError *error) {
        SpineCursor_IterateLimit result=SpineCursor_DoNotIterate;
        switch(limit) {
        case SpineCursor_DoNotIterate:
            result=SpineCursor_DoNotIterate;
            break;
        case SpineCursor_WithinWord:
            result=SpineCursor_WithinWord;
            break;
        case SpineCursor_WithinLine:
            result=SpineCursor_WithinLine;
            break;
        case SpineCursor_WithinBlock:
            result=SpineCursor_WithinBlock;
            break;
        case SpineCursor_WithinRegion:
            result=SpineCursor_WithinRegion;
            break;
        case SpineCursor_WithinPage:
            result=SpineCursor_WithinPage;
            break;
        case SpineCursor_WithinDocument:
            result=SpineCursor_WithinDocument;
            break;
        default:
            *error=SpineError_InvalidArgument;
            break;
        }
        return result;
    }

%}




%constant int DefaultSearchOptions=Spine_DefaultSearchOptions;
%constant int RegExp=Spine_RegExp;
%constant int IgnoreCase=Spine_IgnoreCase;
%constant int WholeWordsOnly=Spine_WholeWordsOnly;

%constant int DefaultView=SpineDocument_ViewDefault;
%constant int OutlineView=SpineDocument_ViewOutlines;
%constant int ThumbnailView=SpineDocument_ViewThumbs;
%constant int FullScreen=SpineDocument_ViewFullScreen;
%constant int OCView=SpineDocument_ViewOC;
%constant int AttachView=SpineDocument_ViewAttach;

%constant int DefaultLayout=SpineDocument_LayoutDefault;
%constant int SinglePageLayout=SpineDocument_LayoutSinglePage;
%constant int OneColumnLayout=SpineDocument_LayoutOneColumn;
%constant int TwoColumnLeftLayout=SpineDocument_LayoutTwoColumnLeft;
%constant int TwoColumnRightLayout=SpineDocument_LayoutTwoColumnRight;
%constant int TwoPageLeftLayout=SpineDocument_LayoutTwoPageLeft;
%constant int TwoPageRightLayout=SpineDocument_LayoutTwoPageRight;

%constant int DoNotIterate= SpineCursor_DoNotIterate;
%constant int UntilEndOfWord= SpineCursor_WithinWord;
%constant int UntilEndOfLine= SpineCursor_WithinLine;
%constant int UntilEndOfBlock= SpineCursor_WithinBlock;
%constant int UntilEndOfRegion= SpineCursor_WithinRegion;
%constant int UntilEndOfPage= SpineCursor_WithinPage;
%constant int UntilEndOfDocument= SpineCursor_WithinDocument;

%constant int NullImage=Spine_NullImage;
%constant int RGBImage=Spine_RGBImage;
%constant int JPEGImage=Spine_JPEGImage;
%constant int BitmapImage=Spine_BitmapImage;




%exception
{
    Py_BEGIN_ALLOW_THREADS
    $action
    Py_END_ALLOW_THREADS
}




%typemap(out) SpineString
{
    if($1 && $1->utf8) {
        $result = PyUnicode_DecodeUTF8($1->utf8, $1->length, 0);
    } else {
        Py_INCREF(Py_None);
        $result=Py_None;
    }
}

%typemap(newfree) SpineString
{
    delete_SpineString(&$1, 0);
}

%typemap(in) SpineString
{
    if(PyUnicode_Check($input)) {
        PyObject *tempstring=PyUnicode_AsUTF8String($input);
        const char *utf8= PyString_AsString(tempstring);
        size_t length=PyString_Size(tempstring);
        $1=new_SpineStringFromUTF8(utf8, length, 0);
        Py_DECREF(tempstring);
    } else if(PyString_Check($input)) {
        const char *utf8= PyString_AsString($input);
        size_t length=PyString_Size($input);
        $1=new_SpineStringFromUTF8(utf8, length, 0);
    } else {
        PyErr_SetString(PyExc_ValueError,"Need a string or unicode argument");
        SWIG_fail;
    }
}

%typemap(arginit) SpineString
{
    $1 = 0;
}

%typemap(freearg) SpineString
{
    delete_SpineString(&$1, 0);
}




%typemap(out) SpineBuffer
{
    if($1 && $1->data) {
        $result = PyString_FromStringAndSize($1->data, $1->length);
    } else {
        Py_INCREF(Py_None);
        $result=Py_None;
    }
}

%typemap(newfree) SpineBuffer
{
    delete_SpineBuffer(&$1, 0);
}

%typemap(in) SpineBuffer
{
    if(PyString_Check($input)) {
        const char *data= PyString_AsString($input);
        size_t length=PyString_Size($input);
        $1=new_SpineBuffer(data, length, 0);
    } else {
        PyErr_SetString(PyExc_ValueError,"Need a string argument");
        SWIG_fail;
    }
}

%typemap(arginit) SpineBuffer
{
    $1 = 0;
}

%typemap(freearg) SpineBuffer
{
    delete_SpineBuffer(&$1, 0);
}




%typemap(out) SpineColor
{
    $result=PyTuple_New(3);
    PyTuple_SetItem($result, 0, PyFloat_FromDouble($1.r));
    PyTuple_SetItem($result, 1, PyFloat_FromDouble($1.g));
    PyTuple_SetItem($result, 2, PyFloat_FromDouble($1.b));
}




%typemap(out) SpineArea
{
    PyObject *top_left=PyTuple_New(2);
    PyTuple_SetItem(top_left, 0, PyFloat_FromDouble($1.x1));
    PyTuple_SetItem(top_left, 1, PyFloat_FromDouble($1.y1));

    PyObject *bottom_right=PyTuple_New(2);
    PyTuple_SetItem(bottom_right, 0, PyFloat_FromDouble($1.x2));
    PyTuple_SetItem(bottom_right, 1, PyFloat_FromDouble($1.y2));

    $result=PyTuple_New(4);
    PyTuple_SetItem($result, 0, PyInt_FromLong($1.page));
    PyTuple_SetItem($result, 1, PyInt_FromLong($1.rotation));
    PyTuple_SetItem($result, 2, top_left);
    PyTuple_SetItem($result, 3, bottom_right);
}

%typemap(in) SpineArea
{
    if(PyTuple_Check($input) && PySequence_Size($input) == 4) {
        int page = (int) PyInt_AsLong(PyTuple_GetItem($input, 0));
        int rotation = (int) PyInt_AsLong(PyTuple_GetItem($input, 1));
        PyObject * start = PyTuple_GetItem($input, 2);
        PyObject * to = PyTuple_GetItem($input, 3);
        if (PyTuple_Check(start) && PySequence_Size(start) == 2 &&
            PyTuple_Check(to) && PySequence_Size(to) == 2)
        {
            double x1 = PyFloat_AsDouble(PyTuple_GetItem(start, 0));
            double y1 = PyFloat_AsDouble(PyTuple_GetItem(start, 1));
            double x2 = PyFloat_AsDouble(PyTuple_GetItem(to, 0));
            double y2 = PyFloat_AsDouble(PyTuple_GetItem(to, 1));
            SpineArea box = { page, rotation, x1, y1, x2, y2 };
            $1 = box;
        }
        else
        {
            PyErr_SetString(PyExc_ValueError,"Need a tuple argument (page, rotation, (x1, y1), (x2, y2))");
            SWIG_fail;
        }
    } else {
        PyErr_SetString(PyExc_ValueError,"Need a tuple argument (page, rotation, (x1, y1), (x2, y2))");
        SWIG_fail;
    }
}




%typemap(out) SpineMap
{
    int i;
    if($1) {
        $result=PyDict_New();
        for(i=0; i < $1->length; ++i) {
            PyObject *key=PyUnicode_DecodeUTF8(($1->keys[i])->utf8, ($1->keys[i])->length, 0);
            PyObject *val=PyUnicode_DecodeUTF8(($1->values[i])->utf8, ($1->values[i])->length, 0);
            PyObject *s;

            if(!PyMapping_HasKey ($result, key)) {
                s=PyList_New(0);
                PyDict_SetItem($result, key, s);
            } else {
                s=PyDict_GetItem ($result, key);
                // Borrowed reference.
                Py_INCREF(s);
            }

            PyList_Append(s, val);

            Py_DECREF(s);
            Py_DECREF(key);
            Py_DECREF(val);
        }
    }
}

%typemap(arginit) SpineMap
{
    $1 = 0;
}

%typemap(newfree) SpineMap
{
    delete_SpineMap(&$1, 0);
}




%typemap(out) SpineSet
{
    int i;
    if($1) {
        $result=PyList_New(0);
        for(i=0; i < $1->length; ++i) {
            PyObject *val=PyUnicode_DecodeUTF8(($1->values[i])->utf8, ($1->values[i])->length, 0);
            PyList_Append($result, val);
            Py_XDECREF(val);
        }
    }
}

%typemap(arginit) SpineSet
{
    $1 = 0;
}

%typemap(newfree) SpineSet
{
    delete_SpineSet(&$1, 0);
}




%typemap(out) SpineTextExtentList
{
    size_t i;
    if($1) {
        PyObject *list=PyList_New($1->count);
        for (i = 0; i < $1->count; ++i)
        {
            struct TextExtent *ext = (struct TextExtent *)(malloc(sizeof(struct TextExtent)));
            ext->_extent = $1->extents[i];
            ext->_err = SpineError_NoError;
            PyList_SetItem(list,
                           i,
                           SWIG_NewPointerObj((void *)(ext),
                                              SWIG_TypeQuery("_p_TextExtent"),
                                              SWIG_POINTER_OWN));
        }
        $result = list;
    } else {
        Py_INCREF(Py_None);
        $result=Py_None;
    }
}


%typemap(arginit) SpineTextExtentList
{
    $1 = 0;
}

%typemap(newfree) SpineTextExtentList
{
    delete_SpineTextExtentList(&$1, 0);
}




%typemap(out) SpineAnnotationList
{
    size_t i;
    if($1) {
        PyObject *list=PyList_New($1->count);
        for (i = 0; i < $1->count; ++i)
        {
            struct Annotation *ann = (struct Annotation *)(malloc(sizeof(struct Annotation)));
            ann->_ann = $1->annotations[i];
            ann->_err = SpineError_NoError;
            PyList_SetItem(list,
                           i,
                           SWIG_NewPointerObj((void *)(ann),
                                              SWIG_TypeQuery("_p_Annotation"),
                                              SWIG_POINTER_OWN));
        }
        $result = list;
    } else {
        Py_INCREF(Py_None);
        $result=Py_None;
    }
}

%typemap(newfree) SpineAnnotationList
{
    delete_SpineAnnotationList(&$1, 0);
}

%typemap(in) SpineAnnotationList
{
    $1=0;
    if(PySequence_Check($input)) {
        Py_ssize_t size = PySequence_Size($input);
        SpineAnnotationList anns = new_SpineAnnotationList(size, 0);
        int i;
        for (i = 0; i < size; ++i)
        {
            PyObject * item = PySequence_GetItem($input, i);
            struct Annotation * ann = 0;
            if (SWIG_ConvertPtr(item, (void **) &ann,
                                SWIG_TypeQuery("_p_Annotation"), SWIG_POINTER_EXCEPTION) == 0)
            {
                anns->annotations[i] = ann->_ann;
            }
        }
        $1 = anns;
    } else {
        PyErr_SetString(PyExc_ValueError,"Need a sequence argument");
        SWIG_fail;
    }
}

%typemap(arginit) SpineAnnotationList
{
    $1 = 0;
}

%typemap(freearg) SpineAnnotationList
{
    delete_SpineAnnotationList(&$1, 0);
}




%typemap(out) SpineAreaList
{
    size_t i;
    if($1) {
        PyObject *list=PyList_New($1->count);
        for (i = 0; i < $1->count; ++i)
        {
            PyObject *tuple=PyTuple_New(4);
            PyTuple_SetItem(tuple, 0, PyInt_FromLong($1->areas[i].page));
            PyTuple_SetItem(tuple, 1, PyInt_FromLong($1->areas[i].rotation));
            PyObject *tuple2=PyTuple_New(2);
            PyTuple_SetItem(tuple2, 0, PyFloat_FromDouble($1->areas[i].x1));
            PyTuple_SetItem(tuple2, 1, PyFloat_FromDouble($1->areas[i].y1));
            PyTuple_SetItem(tuple, 2, tuple2);
            PyObject *tuple3=PyTuple_New(2);
            PyTuple_SetItem(tuple3, 0, PyFloat_FromDouble($1->areas[i].x2));
            PyTuple_SetItem(tuple3, 1, PyFloat_FromDouble($1->areas[i].y2));
            PyTuple_SetItem(tuple, 3, tuple3);

            PyList_SetItem(list, i, tuple);
        }
        $result = list;
    } else {
        Py_INCREF(Py_None);
        $result=Py_None;
    }
}

%typemap(arginit) SpineAreaList
{
    $1 = 0;
}

%typemap(newfree) SpineAreaList
{
    delete_SpineAreaList(&$1, 0);
}




%constant int DefaultSearchOptions=Spine_DefaultSearchOptions;
%constant int RegExp=Spine_RegExp;
%constant int IgnoreCase=Spine_IgnoreCase;
%constant int WholeWordsOnly=Spine_WholeWordsOnly;

%constant int DefaultView=SpineDocument_ViewDefault;
%constant int OutlineView=SpineDocument_ViewOutlines;
%constant int ThumbnailView=SpineDocument_ViewThumbs;
%constant int FullScreen=SpineDocument_ViewFullScreen;
%constant int OCView=SpineDocument_ViewOC;
%constant int AttachView=SpineDocument_ViewAttach;

%constant int DefaultLayout=SpineDocument_LayoutDefault;
%constant int SinglePageLayout=SpineDocument_LayoutSinglePage;
%constant int OneColumnLayout=SpineDocument_LayoutOneColumn;
%constant int TwoColumnLeftLayout=SpineDocument_LayoutTwoColumnLeft;
%constant int TwoColumnRightLayout=SpineDocument_LayoutTwoColumnRight;
%constant int TwoPageLeftLayout=SpineDocument_LayoutTwoPageLeft;
%constant int TwoPageRightLayout=SpineDocument_LayoutTwoPageRight;

%constant int DoNotIterate= SpineCursor_DoNotIterate;
%constant int UntilEndOfWord= SpineCursor_WithinWord;
%constant int UntilEndOfLine= SpineCursor_WithinLine;
%constant int UntilEndOfBlock= SpineCursor_WithinBlock;
%constant int UntilEndOfRegion= SpineCursor_WithinRegion;
%constant int UntilEndOfPage= SpineCursor_WithinPage;
%constant int UntilEndOfDocument= SpineCursor_WithinDocument;

%constant int NullImage=Spine_NullImage;
%constant int RGBImage=Spine_RGBImage;
%constant int JPEGImage=Spine_JPEGImage;
%constant int BitmapImage=Spine_BitmapImage;




%include <pyspineapi.h>

%extend Annotation {

    Annotation() {
        struct Annotation *a=(struct Annotation *) malloc(sizeof(struct Annotation));
        a->_err=SpineError_NoError;
        a->_ann=new_SpineAnnotation(&a->_err);
        check_exception(a->_err);
        return a;
    }

    ~Annotation() {
        $self->_err=SpineError_NoError;
        delete_SpineAnnotation(&$self->_ann, &$self->_err);
    }

}

%extend Document {

    Document() {
        struct Document *d=(struct Document *) malloc(sizeof(struct Document));
        d->_err=SpineError_NoError;
        d->_doc=new_SpineDocument(&d->_err);
        return d;
    }

    ~Document() {
        $self->_err=SpineError_NoError;
        delete_SpineDocument(&$self->_doc, &$self->_err);
    }

}

%extend Image {
    Image() {
        struct Image *img=(struct Image *) malloc(sizeof(struct Image));
        img->_err=SpineError_NoError;
        img->_image=new_SpineImage(&img->_err);
        check_exception(img->_err);
        return img;
    }

    ~Image() {
        $self->_err=SpineError_NoError;
        delete_SpineImage(&$self->_image, &$self->_err);
    }

}

%extend Cursor {
    Cursor() {
        struct Cursor *c=(struct Cursor *) malloc(sizeof(struct Cursor));
        c->_err=SpineError_InvalidType;
        c->_cursor=0;
        check_exception(c->_err);
        return c;
    }

    ~Cursor() {
        $self->_err=SpineError_NoError;
        delete_SpineCursor(&$self->_cursor, &$self->_err);
    }

}

%extend TextExtent {

    TextExtent(const struct Cursor start, const struct Cursor to) {
        struct TextExtent *e=(struct TextExtent *) malloc(sizeof(struct TextExtent));
        e->_err=SpineError_NoError;
        e->_extent=new_SpineTextExtent(start._cursor, to._cursor, &e->_err);
        check_exception(e->_err);
        return e;
    }

    ~TextExtent() {
        $self->_err=SpineError_NoError;
        delete_SpineTextExtent(&$self->_extent, &$self->_err);
    }

}


/****************************************************************************
 *
 * Standard exception handler for all methods except constuctors
 *
 ****************************************************************************/

%exception {
    $action
    if(check_exception(arg1->_err)) SWIG_fail;
}

%extend Document {

    int valid() {
        int result= SpineDocument_valid($self->_doc, &$self->_err);
        return result;
    }

    const char *errorString() {
        const char *result=SpineDocument_errorString($self->_doc, &$self->_err);
        return result;
    }

    int viewMode() {
        int result=(int) SpineDocument_viewMode($self->_doc, &$self->_err);
        return result;
    }

    int pageLayout() {
        int result=(int) SpineDocument_pageLayout($self->_doc, &$self->_err);
        return result;
    }

    struct Cursor newCursor(int page=1) {
        struct Cursor c;
        c._err=SpineError_NoError;
        c._cursor=SpineDocument_newCursor($self->_doc, page,  &$self->_err);
        return c;
    }

    %newobject data;
    SpineBuffer data() {
       return SpineDocument_data($self->_doc, &$self->_err);
    }

    %newobject newScratchId;
    SpineString newScratchId() {
        SpineString result=SpineDocument_newScratchId($self->_doc, &$self->_err);
        return result;
    }

    %newobject deletedItemsScratchId;
    SpineString deletedItemsScratchId() {
        SpineString result=SpineDocument_deletedItemsScratchId($self->_doc, &$self->_err);
        return result;
    }

    %newobject pmid;
    SpineString pmid() {
        SpineString result=SpineDocument_pmid($self->_doc, &$self->_err);
        return result;
    }

    %newobject doi;
    SpineString doi() {
        SpineString result=SpineDocument_doi($self->_doc, &$self->_err);
        return result;
    }

    %newobject pii;
    SpineString pii() {
        SpineString result=SpineDocument_pii($self->_doc, &$self->_err);
        return result;
    }

    %newobject uniqueID;
    SpineString uniqueID() {
        SpineString result=SpineDocument_uniqueID($self->_doc, &$self->_err);
        return result;
    }

    %newobject filehash;
    SpineString filehash() {
        SpineString result=SpineDocument_filehash($self->_doc, &$self->_err);
        return result;
    }

    %newobject fingerprints;
    SpineSet fingerprints() {
        $self->_err=SpineError_NoError;
        SpineSet result=SpineDocument_fingerprints($self->_doc, &$self->_err);
        return result;
    }

    %newobject iri;
    SpineString iri() {
        SpineString result=SpineDocument_iri($self->_doc, &$self->_err);
        return result;
    }

    size_t numberOfPages() {
        size_t result=SpineDocument_numberOfPages($self->_doc, &$self->_err);
        return result;
    }

    size_t wordCount() {
        size_t result=SpineDocument_wordCount($self->_doc, &$self->_err);
        return result;
    }

    %newobject title;
    SpineString title() {
        SpineString result=SpineDocument_title($self->_doc, &$self->_err);
        return result;
    }

    %newobject subject;
    SpineString subject() {
        SpineString result=SpineDocument_subject($self->_doc, &$self->_err);
        return result;
    }

    %newobject keywords;
    SpineString keywords() {
        SpineString result=SpineDocument_keywords($self->_doc, &$self->_err);
        return result;
    }

    %newobject author;
    SpineString author() {
        SpineString result=SpineDocument_author($self->_doc, &$self->_err);
        return result;
    }

    %newobject creator;
    SpineString creator() {
        SpineString result=SpineDocument_creator($self->_doc, &$self->_err);
        return result;
    }

    %newobject producer;
    SpineString producer() {
        SpineString result=SpineDocument_producer($self->_doc, &$self->_err);
        return result;
    }

    time_t creationDate() {
        time_t result=SpineDocument_creationDate($self->_doc, &$self->_err);
        return result;
    }

    time_t modificationDate() {
        time_t result=SpineDocument_modificationDate($self->_doc, &$self->_err);
        return result;
    }

    int imageBased() {
        int result=SpineDocument_imageBased($self->_doc, &$self->_err);
        return result;
    }

    struct Image _render(int page, float resolution)
    {
        struct Image img;
        img._err=SpineError_NoError;
        img._image=SpineDocument_render($self->_doc, page, resolution, &$self->_err);
        return img;
    }

    struct Image _renderArea(const SpineArea pb, float resolution)
    {
        struct Image img;
        img._err=SpineError_NoError;
        img._image=SpineDocument_renderArea($self->_doc, pb, resolution, &$self->_err);
        return img;
    }

    %newobject textSelection;
    SpineTextExtentList textSelection()
    {
        $self->_err=SpineError_NoError;
        return SpineDocument_textSelection($self->_doc, &$self->_err);
    }

    %newobject areaSelection;
    SpineAreaList areaSelection()
    {
        $self->_err=SpineError_NoError;
        return SpineDocument_areaSelection($self->_doc, &$self->_err);
    }

    %newobject _search;
    SpineTextExtentList _search(const SpineString regex, int options)
    {
        SpineTextExtentList result=SpineDocument_search($self->_doc, regex, options, &$self->_err);
        return result;
    }

    %newobject _searchFrom;
    SpineTextExtentList _searchFrom(const struct Cursor start, const SpineString regex, int options)
    {
        SpineTextExtentList result=SpineDocument_searchFrom($self->_doc, start._cursor, regex, options, &$self->_err);
        return result;
    }

    %newobject text;
    SpineString text() {
        SpineString result=SpineDocument_text($self->_doc, &$self->_err);
        return result;
    }

    struct TextExtent substr(int start, int len) {
        struct TextExtent e;
        e._err=SpineError_NoError;
        e._extent=SpineDocument_substr($self->_doc, start, len, &$self->_err);
        return e;
    }

    %newobject _annotations;
    SpineAnnotationList _annotations()
    {
        return SpineDocument_annotations($self->_doc, &$self->_err);
    }

    %newobject _scratchAnnotations;
    SpineAnnotationList _scratchAnnotations(const SpineString listName)
    {
        return SpineDocument_scratchAnnotations($self->_doc, listName, &$self->_err);
    }

    void _addAnnotation(const struct Annotation ann)
    {
        SpineDocument_addAnnotation($self->_doc, ann._ann, &$self->_err);
    }

    void _addAnnotations(const SpineAnnotationList ann)
    {
        SpineDocument_addAnnotations($self->_doc, ann, &$self->_err);
    }

    void _addScratchAnnotation(const struct Annotation ann, const SpineString listName)
    {
        SpineDocument_addScratchAnnotation($self->_doc, ann._ann, listName, &$self->_err);
    }

    void _addScratchAnnotations(const SpineAnnotationList anns, const SpineString listName)
    {
        SpineDocument_addScratchAnnotations($self->_doc, anns, listName, &$self->_err);
    }

    void _removeAnnotation(const struct Annotation ann)
    {
        SpineDocument_removeAnnotation($self->_doc, ann._ann, &$self->_err);
    }

    void _removeScratchAnnotation(const struct Annotation ann, const SpineString listName)
    {
        SpineDocument_removeScratchAnnotation($self->_doc, ann._ann, listName, &$self->_err);
    }

    struct TextExtent resolveExtent(int page1, double x1, double y1, int page2, double x2, double y2)
    {
        struct TextExtent e;
        e._err=SpineError_NoError;
        e._extent=SpineDocument_resolveExtent($self->_doc, page1, x1, y1, page2, x2, y2, &$self->_err);
        return e;
    }

}

%extend Cursor {

    struct Cursor copy() {
        struct Cursor c;
        c._cursor=SpineCursor_copy($self->_cursor, &$self->_err);
        c._err=$self->_err;
        return c;
    }

    int advancePage(int limit=SpineCursor_WithinDocument) {
        SpineCursor_IterateLimit lim=spinelimit(limit, &$self->_err);
        if(SpineError_ok($self->_err)) {
            return SpineCursor_advancePage($self->_cursor, lim, &$self->_err);
        }
        return 0;
    }

    int retreatPage(int limit=SpineCursor_WithinDocument) {
        SpineCursor_IterateLimit lim=spinelimit(limit, &$self->_err);
        if(SpineError_ok($self->_err)) {
            return SpineCursor_retreatPage($self->_cursor, lim, &$self->_err);
        }
        return 0;
    }

    void gotoPage(int page) {
        SpineCursor_gotoPage($self->_cursor, page, &$self->_err);
    }

    %newobject pageText;
    SpineString pageText() {
        SpineString result=SpineCursor_pageText($self->_cursor, &$self->_err);
        return result;
    }

    int pageValid() {
        int result=SpineCursor_pageValid($self->_cursor, &$self->_err);
        return result;
    }

    SpineArea pageArea() {
        SpineArea result=SpineCursor_pageArea($self->_cursor, &$self->_err);
        return result;
    }

    int advanceRegion(int limit=SpineCursor_WithinPage) {
        SpineCursor_IterateLimit lim=spinelimit(limit, &$self->_err);
        if(SpineError_ok($self->_err)) {
            return SpineCursor_advanceRegion($self->_cursor, lim, &$self->_err);
        }
        return 0;
    }

    int retreatRegion(int limit=SpineCursor_WithinPage) {
        SpineCursor_IterateLimit lim=spinelimit(limit, &$self->_err);
        if(SpineError_ok($self->_err)) {
            return SpineCursor_retreatRegion($self->_cursor, lim, &$self->_err);
        }
        return 0;
    }
    %newobject regionText;
    SpineString regionText() {
        SpineString result=SpineCursor_regionText($self->_cursor, &$self->_err);
        return result;
    }
    int regionValid() {
        int result=SpineCursor_regionValid($self->_cursor, &$self->_err);
        return result;
    }
    SpineArea regionArea() {
        SpineArea result=SpineCursor_regionArea($self->_cursor, &$self->_err);
        return result;
    }
    int advanceBlock(int limit=SpineCursor_WithinRegion) {
        SpineCursor_IterateLimit lim=spinelimit(limit, &$self->_err);
        if(SpineError_ok($self->_err)) {
            return SpineCursor_advanceBlock($self->_cursor, lim, &$self->_err);
        }
        return 0;
    }
    int retreatBlock(int limit=SpineCursor_WithinRegion) {
        SpineCursor_IterateLimit lim=spinelimit(limit, &$self->_err);
        if(SpineError_ok($self->_err)) {
            return SpineCursor_retreatBlock($self->_cursor, lim, &$self->_err);
        }
        return 0;
    }

    %newobject blockText;
    SpineString blockText() {
        SpineString result=SpineCursor_blockText($self->_cursor, &$self->_err);
        return result;
    }

    int blockValid() {
        int result=SpineCursor_blockValid($self->_cursor, &$self->_err);
        return result;
    }

    SpineArea blockArea() {
        SpineArea result=SpineCursor_blockArea($self->_cursor, &$self->_err);
        return result;
    }

    int advanceLine(int limit=SpineCursor_WithinBlock) {
        SpineCursor_IterateLimit lim=spinelimit(limit, &$self->_err);
        if(SpineError_ok($self->_err)) {
            return SpineCursor_advanceLine($self->_cursor, lim, &$self->_err);
        }
        return 0;
    }

    int retreatLine(int limit=SpineCursor_WithinBlock) {
        SpineCursor_IterateLimit lim=spinelimit(limit, &$self->_err);
        if(SpineError_ok($self->_err)) {
            return SpineCursor_retreatLine($self->_cursor, lim, &$self->_err);
        }
        return 0;
    }

    %newobject lineText;
    SpineString lineText() {
        SpineString result=SpineCursor_lineText($self->_cursor, &$self->_err);
        return result;
    }

    int lineValid() {
        int result=SpineCursor_lineValid($self->_cursor, &$self->_err);
        return result;
    }

    SpineArea lineArea() {
        SpineArea result=SpineCursor_lineArea($self->_cursor, &$self->_err);
        return result;
    }

    int advanceWord(int limit=SpineCursor_WithinLine) {
        SpineCursor_IterateLimit lim=spinelimit(limit, &$self->_err);
        if(SpineError_ok($self->_err)) {
            return SpineCursor_advanceWord($self->_cursor, lim, &$self->_err);
        }
        return 0;
    }

    int retreatWord(int limit=SpineCursor_WithinLine) {
        SpineCursor_IterateLimit lim=spinelimit(limit, &$self->_err);
        if(SpineError_ok($self->_err)) {
            return SpineCursor_retreatWord($self->_cursor, lim, &$self->_err);
        }
        return 0;
    }

    %newobject wordText;
    SpineString wordText() {
        SpineString result=SpineCursor_wordText($self->_cursor, &$self->_err);
        return result;
    }

    int wordValid() {
        int result=SpineCursor_wordValid($self->_cursor, &$self->_err);
        return result;
    }

    SpineArea wordArea() {
        SpineArea result=SpineCursor_wordArea($self->_cursor, &$self->_err);
        return result;
    }

    %newobject wordFontName;
    SpineString wordFontName() {
        SpineString result=SpineCursor_wordFontName($self->_cursor, &$self->_err);
        return result;
    }

    double wordFontSize() {
        double result=SpineCursor_wordFontSize($self->_cursor, &$self->_err);
        return result;
    }

    int wordSpaceAfter() {
        int result=SpineCursor_wordSpaceAfter($self->_cursor, &$self->_err);
        return result;
    }

    int wordUnderlined() {
        int result=SpineCursor_wordUnderlined($self->_cursor, &$self->_err);
        return result;
    }

    double wordBaseline() {
        double result=SpineCursor_wordBaseline($self->_cursor, &$self->_err);
        return result;
    }

    SpineColor wordColor() {
        SpineColor result=SpineCursor_wordColor($self->_cursor, &$self->_err);
        return result;
    }

    int advanceCharacter(int limit=SpineCursor_WithinWord) {
        SpineCursor_IterateLimit lim=spinelimit(limit, &$self->_err);
        if(SpineError_ok($self->_err)) {
            return SpineCursor_advanceCharacter($self->_cursor, lim, &$self->_err);
        }
        return 0;
    }

    int retreatCharacter(int limit=SpineCursor_WithinWord) {
        SpineCursor_IterateLimit lim=spinelimit(limit, &$self->_err);
        if(SpineError_ok($self->_err)) {
            return SpineCursor_retreatCharacter($self->_cursor, lim, &$self->_err);
        }
        return 0;
    }

    %newobject characterText;
    SpineString characterText() {
        SpineString result=SpineCursor_characterText($self->_cursor, &$self->_err);
        return result;
    }

    int characterValid() {
        int result=SpineCursor_characterValid($self->_cursor, &$self->_err);
        return result;
    }

    SpineArea characterArea() {
        SpineArea result=SpineCursor_characterArea($self->_cursor, &$self->_err);
        return result;
    }

    %newobject characterFontName;
    SpineString characterFontName() {
        SpineString result=SpineCursor_characterFontName($self->_cursor, &$self->_err);
        return result;
    }

    double characterFontSize() {
        double result=SpineCursor_characterFontSize($self->_cursor, &$self->_err);
        return result;
    }

    int characterSpaceAfter() {
        int result=SpineCursor_characterSpaceAfter($self->_cursor, &$self->_err);
        return result;
    }

    int characterUnderlined() {
        int result=SpineCursor_characterUnderlined($self->_cursor, &$self->_err);
        return result;
    }

    double characterBaseline() {
        double result=SpineCursor_characterBaseline($self->_cursor, &$self->_err);
        return result;
    }

    SpineColor characterColor() {
        SpineColor result=SpineCursor_characterColor($self->_cursor, &$self->_err);

        return result;
    }

    int advanceImage(int limit=SpineCursor_WithinPage) {
        SpineCursor_IterateLimit lim=spinelimit(limit, &$self->_err);
        if(SpineError_ok($self->_err)) {
            return SpineCursor_advanceImage($self->_cursor, lim, &$self->_err);
        }
        return 0;
    }

    int retreatImage(int limit=SpineCursor_WithinPage) {
        SpineCursor_IterateLimit lim=spinelimit(limit, &$self->_err);
        if(SpineError_ok($self->_err)) {
            return SpineCursor_retreatImage($self->_cursor, lim, &$self->_err);
        }
        return 0;
    }


    struct Image _image() {
        struct Image result;
        result._err=SpineError_NoError;
        result._image=SpineCursor_image($self->_cursor, &$self->_err);
        return result;
    }

    int imageValid() {
        int result=SpineCursor_imageValid($self->_cursor, &$self->_err);
        return result;
    }

    SpineArea imageArea() {
        SpineArea result=SpineCursor_imageArea($self->_cursor, &$self->_err);
        return result;
    }

}

%extend TextExtent
{

    struct Cursor begin()
    {
        struct Cursor result;
        result._err=SpineError_NoError;
        result._cursor=SpineTextExtent_from($self->_extent, &$self->_err);
        return result;
    }

    struct Cursor end()
    {
        struct Cursor result;
        result._err=SpineError_NoError;
        result._cursor=SpineTextExtent_to($self->_extent, &$self->_err);
        return result;
    }

    %newobject text;
    SpineString text()
    {
        $self->_err=SpineError_NoError;
        return SpineTextExtent_text($self->_extent, &$self->_err);
    }

    %newobject areas;
    SpineAreaList areas()
    {
        $self->_err=SpineError_NoError;
        return SpineTextExtent_areas($self->_extent, &$self->_err);
    }

    %newobject _search;
    SpineTextExtentList _search(const SpineString regex, int options) {
        SpineTextExtentList result=SpineTextExtent_search($self->_extent, regex, options, &$self->_err);
        return result;
    }

}

%extend Annotation {

    void setProperty(const SpineString key, const SpineString val)
    {
        $self->_err=SpineError_NoError;
        SpineAnnotation_setProperty($self->_ann, key, val, &$self->_err);
    }

    void removePropertyAll(const SpineString key)
    {
        $self->_err=SpineError_NoError;
        SpineAnnotation_removePropertyAll($self->_ann, key, &$self->_err);
    }

    void removeProperty(const SpineString key, const SpineString val)
    {
        $self->_err=SpineError_NoError;
        SpineAnnotation_removeProperty($self->_ann, key, val, &$self->_err);
    }

    %newobject getFirstProperty;
    SpineString getFirstProperty(const SpineString key)
    {
        $self->_err=SpineError_NoError;
        return SpineAnnotation_getFirstProperty($self->_ann, key, &$self->_err);
    }

    PyObject * hasProperty(const SpineString key)
    {
        $self->_err=SpineError_NoError;
        if (SpineAnnotation_hasProperty($self->_ann, key, &$self->_err)) {
            Py_RETURN_TRUE;
        } else {
            Py_RETURN_FALSE;
        }
    }

    void insertProperty(const SpineString key, const SpineString val)
    {
        $self->_err=SpineError_NoError;
        SpineAnnotation_insertProperty($self->_ann, key, val, &$self->_err);
    }

    %newobject getAllProperties;
    SpineSet getAllProperties(const SpineString key)
    {
        $self->_err=SpineError_NoError;
        return SpineAnnotation_getProperty($self->_ann, key, &$self->_err);
    }

    %newobject properties;
    SpineMap properties()
    {
        $self->_err=SpineError_NoError;
        return SpineAnnotation_properties($self->_ann, &$self->_err);
    }

    void clearProperties()
    {
        $self->_err=SpineError_NoError;
        SpineAnnotation_clearProperties($self->_ann, &$self->_err);
    }

    %newobject extents;
    SpineTextExtentList extents()
    {
        $self->_err=SpineError_NoError;
        return SpineAnnotation_extents($self->_ann, &$self->_err);
    }

    %newobject areas;
    SpineAreaList areas()
    {
        $self->_err=SpineError_NoError;
        return SpineAnnotation_areas($self->_ann, &$self->_err);
    }

    void addExtent(const struct TextExtent extent)
    {
        return SpineAnnotation_addExtent($self->_ann, extent._extent, &$self->_err);
    }

    void removeExtent(const struct TextExtent extent)
    {
        return SpineAnnotation_removeExtent($self->_ann, extent._extent, &$self->_err);
    }

    void addArea(const SpineArea pb)
    {
        return SpineAnnotation_addArea($self->_ann, pb, &$self->_err);
    }

    void removeArea(const SpineArea pb)
    {
        return SpineAnnotation_removeArea($self->_ann, pb, &$self->_err);
    }

    PyObject * __eq__(const struct Annotation ann)
    {
        if(SpineAnnotation_equal($self->_ann, ann._ann, &$self->_err)) {
            Py_RETURN_TRUE;
        } else {
            Py_RETURN_FALSE;
        }
    }

    PyObject * equalRegions(const struct Annotation ann)
    {
        if(SpineAnnotation_equalRegions($self->_ann, ann._ann, &$self->_err)) {
            Py_RETURN_TRUE;
        } else {
            Py_RETURN_FALSE;
        }
    }

    PyObject * isPublic()
    {
        if(SpineAnnotation_isPublic($self->_ann, &$self->_err)) {
            Py_RETURN_TRUE;
        } else {
            Py_RETURN_FALSE;
        }
    }

    void setPublic(int isPublic)
    {
        SpineAnnotation_setPublic($self->_ann, isPublic, &$self->_err);
    }

}

%extend Image
{

    PyObject * size()
    {
        PyObject * size = PyTuple_New(2);
        PyTuple_SetItem(size, 0, PyInt_FromLong(SpineImage_width($self->_image, &$self->_err)));
        PyTuple_SetItem(size, 1, PyInt_FromLong(SpineImage_height($self->_image, &$self->_err)));
        return size;
    }

    int __len__()
    {
        return SpineImage_size($self->_image, &$self->_err);
    }

    int type()
    {
        return SpineImage_type($self->_image, &$self->_err);
    }

    PyObject * data()
    {
        return PyString_FromStringAndSize(SpineImage_data($self->_image, &$self->_err), SpineImage_size($self->_image, &$self->_err));
    }

}

%include "spineapi_python.py"
