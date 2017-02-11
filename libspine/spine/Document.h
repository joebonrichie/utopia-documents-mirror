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

#ifndef DOCUMENT_INCL_
#define DOCUMENT_INCL_

/*****************************************************************************
 *
 * Document.h
 *
 * Copyright 2008 Advanced Interfaces Group
 *
 ****************************************************************************/

#define BOOST_SIGNALS

#include <spine/Page.h>
#include <spine/Annotation.h>
#include <spine/Image.h>
#include <spine/TextIterator.h>
#include <spine/TextSelection.h>
#include <spine/spineapi.h>

#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/thread.hpp>
#include <string>
#include <cstdlib>
#include <cwctype>
#include <ctime>
#include <vector>
#include <set>


namespace Spine
{

    typedef enum { AnnotationsAdded, AnnotationsChanged, AnnotationsRemoved } AnnotationsChangedAction;

    typedef void (*AnnotationsChangedSignal)(void*, const std::string &, Spine::AnnotationSet, bool);
    typedef void (*AreaSelectionChangedSignal)(void*, const std::string &, Spine::AreaSet, bool);
    typedef void (*TextSelectionChangedSignal)(void*, const std::string &, Spine::TextExtentSet, bool);

    class Cursor;
    class Annotation;

    /***************************************************************************
     *
     * Document
     *
     **************************************************************************/

    class DocumentPrivate;
    class Document
    {

    public:
        typedef Spine::Cursor cursor;
        typedef std::set<std::string> FingerprintSet;
        typedef boost::shared_ptr<Document> DocumentHandle;

        enum ViewMode {
            ViewNone,
            ViewOutlines,
            ViewThumbs,
            ViewFullScreen,
            ViewOC,
            ViewAttach
        };

        enum PageLayout {
            LayoutNone,
            LayoutSinglePage,
            LayoutOneColumn,
            LayoutTwoColumnLeft,
            LayoutTwoColumnRight,
            LayoutTwoPageLeft,
            LayoutTwoPageRight
        };

        Document(void * userdef_ = 0);
        virtual ~Document();
        virtual bool isOK()=0;
        virtual const char *errorString()=0;

        virtual void readFile(const char * filename_)=0;
        virtual void readBuffer(boost::shared_array<char> data_, size_t length_)=0;
        virtual void close()=0;
        virtual std::string data()=0;

        virtual ViewMode viewMode()=0;
        virtual PageLayout pageLayout()=0;

        virtual boost::shared_ptr<Spine::Cursor> newCursor(int page_=1)=0;
        boost::shared_ptr<Spine::Cursor> cursorAt(int page, double x, double y);

        virtual FingerprintSet fingerprints();

        virtual std::string pmid();
        virtual std::string doi();
        virtual std::string pii();
        virtual std::string uniqueID(); // specific to derived class
        virtual std::string filehash() = 0;
        virtual void calculateCharacterFingerprints();
        virtual void calculateImageFingerprints();
        virtual std::string characterFingerprint1();
        virtual std::string characterFingerprint2();
        virtual std::string imageFingerprint1();
        virtual std::string imageFingerprint2();

        virtual std::string iri();

        virtual size_t numberOfPages()=0;
        virtual size_t wordCount();

        virtual std::string title();
        virtual std::string subject();
        virtual std::string keywords();
        virtual std::string author();
        virtual std::string creator();
        virtual std::string producer();

        std::string newScratchId(const std::string & name = std::string()) const;
        std::string deletedItemsScratchId() const;

        void * userdef();
        void setUserdef(void * userdef_);

        virtual std::time_t creationDate();
        virtual std::time_t modificationDate();

        virtual bool imageBased();

        virtual DocumentHandle clone()=0;

        // Render as image
        Image render(int page, double resolution);
        Image renderArea(const Area & area, double resolution);

        // Iterating over / finding text
        TextIterator begin();
        TextIterator end();
        TextExtentSet search(const std::string & term, int options = DefaultSearchOptions);
        TextExtentSet searchFrom(const TextIterator & start, const std::string & term, int options = DefaultSearchOptions);
        TextExtentHandle resolveExtent(int page1, double x1, double y1, int page2, double x2, double y2);
        virtual std::string text();
        TextExtentHandle substr(int start, int len);

        // Annotations
        std::list< std::string > annotationLists() const;
        std::set< AnnotationHandle > annotations(const std::string & list = std::string()) const;
        std::set< AnnotationHandle > annotationsById(const std::string & id, const std::string & list = std::string()) const;
        std::set< AnnotationHandle > annotationsByParentId(const std::string & id, const std::string & list = std::string()) const;
        void addAnnotation(AnnotationHandle ann_, const std::string & list = std::string());
        void addAnnotations(const std::set< AnnotationHandle > & anns_, const std::string & list = std::string());
        void removeAnnotation(AnnotationHandle ann_, const std::string & list = std::string());
        void removeAnnotations(const std::set< AnnotationHandle > & anns_, const std::string & list = std::string());
        std::set< AnnotationHandle > annotationsAt(int page, const std::string & list = std::string()) const;
        std::set< AnnotationHandle > annotationsAt(int page, double x, double y, const std::string & list = std::string()) const;
        std::set< AnnotationHandle > annotationsSelected(const TextSelection & selection, const std::string & list = std::string()) const;

        // Selection
        void clearSelection(const std::string & name = std::string());

        void addToTextSelection(const TextSelection & selection, const std::string & name = std::string());
        void clearTextSelection(const std::string & name = std::string());
        void removeFromTextSelection(const TextSelection & selection, const std::string & name = std::string());
        const TextSelection & textSelection(const std::string & name = std::string()) const;
        void setTextSelection(const TextSelection & selection, const std::string & name = std::string());
        std::string selectionText(const std::string & name = std::string()) const;

        void addToAreaSelection(const Area & area, const std::string & name = std::string());
        void addToAreaSelection(const AreaSet & areas, const std::string & name = std::string());
        void clearAreaSelection(const std::string & name = std::string());
        void removeFromAreaSelection(const AreaSet & areas, const std::string & name = std::string());
        const AreaSet & areaSelection(const std::string & name = std::string()) const;
        void setAreaSelection(const AreaSet & areas, const std::string & name = std::string());
        void setAreaSelection(const Area & area, const std::string & name = std::string());

        // signals
        void connectAnyAnnotationsChanged(AnnotationsChangedSignal subscriber, void * userdef);
        void connectAnnotationsChanged(AnnotationsChangedSignal subscriber, void * userdef, const std::string & name = std::string());
        void disconnectAnyAnnotationsChanged(AnnotationsChangedSignal subscriber, void * userdef);
        void disconnectAnnotationsChanged(AnnotationsChangedSignal subscriber, void * userdef, const std::string & name = std::string());

        void connectAnyAreaSelectionChanged(AreaSelectionChangedSignal subscriber, void * userdef);
        void connectAreaSelectionChanged(AreaSelectionChangedSignal subscriber, void * userdef, const std::string & name = std::string());
        void disconnectAnyAreaSelectionChanged(AreaSelectionChangedSignal subscriber, void * userdef);
        void disconnectAreaSelectionChanged(AreaSelectionChangedSignal subscriber, void * userdef, const std::string & name = std::string());

        void connectAnyTextSelectionChanged(TextSelectionChangedSignal subscriber, void * userdef);
        void connectTextSelectionChanged(TextSelectionChangedSignal subscriber, void * userdef, const std::string & name = std::string());
        void disconnectAnyTextSelectionChanged(TextSelectionChangedSignal subscriber, void * userdef);
        void disconnectTextSelectionChanged(TextSelectionChangedSignal subscriber, void * userdef, const std::string & name = std::string());

    protected:

        TextExtentHandle _cachedExtent(TextIterator begin_, TextIterator end_);
        std::map<std::pair<TextIterator, TextIterator>, TextExtentHandle> _cached_extents;

        std::string get_prefix(const std::string &prefix_, size_t minlength_=1);
        std::string _sha256(unsigned char * data_, size_t length_);
        DocumentPrivate * d;
    };

    typedef boost::shared_ptr<Document> DocumentHandle;
    typedef boost::weak_ptr<Document> WeakDocumentHandle;

    /**************************************************************************/

    SpineDocument new_SpineDocument(DocumentHandle doc, SpineError *error);
    SpineDocument share_SpineDocument(DocumentHandle doc, SpineError *error);

}


#include <spine/Cursor.h>

#endif /* DOCUMENT_INCL_ */
