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
 * Document.cpp
 *
 * Copyright 2008 Advanced Interfaces Group
 *
 ****************************************************************************/

#include <spine/Document.h>
#include <spine/Cursor.h>
#include <spine/Character.h>
#include <spine/Word.h>
#include <spine/Block.h>
#include <spine/Region.h>
#include <spine/Image.h>
#include <spine/Annotation.h>
#include <spine/utility.h>
#include <spine/fingerprint.h>

#include <algorithm>
#include <locale>
#include <cstdlib>
#include <cwctype>

#include <string>
#include <utf8/unicode.h>
#include <pcrecpp.h>

#include "spineapi_internal.h"

using namespace std;
using namespace utf8;
using namespace Spine;
using namespace boost;
using namespace pcrecpp;

namespace {

    /*
     * scan for either info:<prefix>/<id> or prefix:<id>
     * returns prefix:<id> or empty string
     */
    string scan_for_prefix(const string &str_, const string &prefix_, size_t minlength_)
    {
        string match;
        string result;

        RE r1(prefix_ + ":\\s*([^\\s<]+).*?", UTF8());
        RE r2("info:" + prefix_ + "/\\s*([^\\s<]+).*?", UTF8());

        if(!r1.PartialMatch(str_, &match)) {
            r2.PartialMatch(str_, &match);
        }

        if(match.length() > minlength_) {
            // strip surrounding punctuation
            RE strip(prefix_ + "[\\s[[:P*:]]]*(.*?)[\\s[[:P*:]]]*");

            if(strip.FullMatch(match, &result)) {
                // add prefix in IRI form
                result="info:" + prefix_ + "/" + result;
            }
        }

        return result;
    }
}

/****************************************************************************/

namespace Spine
{

    typedef boost::shared_ptr<Document> DocumentHandle;

    class DocumentPrivate
    {

        // Order URIs by the reverse of their code unit vectors
        // Should mean far fewer comparisons
        // (considering utf8 is not necessary here for stable ordering)
        class compare_uri
        {
        public:
            bool operator () (const string & lhs, const string & rhs) const
            {
                string::const_reverse_iterator l(lhs.rbegin());
                string::const_reverse_iterator r(rhs.rbegin());
                string::const_reverse_iterator l_rend(lhs.rend());
                string::const_reverse_iterator r_rend(rhs.rend());

                while(l!=l_rend && r!=r_rend) {

                    if (*l < *r) {
                        return true;
                    } else if (*l > *r) {
                        return false;
                    }

                    ++l;
                    ++r;
                }

                return lhs.size() > rhs.size();
            }
        };

    public:
        mutable string filehash;
        mutable string charhash1;
        mutable string charhash2;
        mutable string imagehash1;
        mutable string imagehash2;
        mutable string pmid;
        mutable string doi;
        mutable string pii;

        map< string, string > scratchIds;
        string deathRowScratchId;

        map< string, AnnotationSet > annotations;
        map< string, AnnotationSet, compare_uri > annotationsById;
        map< Annotation *, size_t > annotationsByIdRefCount;
        map< string, AnnotationSet, compare_uri > annotationsByParentId;
        map< Annotation *, size_t > annotationsByParentIdRefCount;
        map< string, list< pair< AnnotationsChangedSignal, void * > > > annotationSubscribers;
        mutable boost::recursive_mutex annotationsMutex;

        void emitAnnotationsChanged(const string & name, const AnnotationSet & annotations, bool added)
        {
            string any;
            list< pair< AnnotationsChangedSignal, void * > > subscribers;
            map< string, list< pair< AnnotationsChangedSignal, void * > > >::const_iterator found;

            // Any
            if (name != any) {
                found = annotationSubscribers.find(any);
                if (found != annotationSubscribers.end())
                {
                    subscribers.insert(subscribers.end(), found->second.begin(), found->second.end());
                }
            }
            // Named
            found = annotationSubscribers.find(name);
            if (found != annotationSubscribers.end()) {
                subscribers.insert(subscribers.end(), found->second.begin(), found->second.end());
            }

            typedef pair< AnnotationsChangedSignal, void * > AnnotationsChangedSignalPair;
            BOOST_FOREACH(AnnotationsChangedSignalPair & subscriber, subscribers) {
                (*subscriber.first)(subscriber.second, name, annotations, added);
            }
        }

        map< string, Spine::AreaSet > areaSelections;
        map< string, list< pair< AreaSelectionChangedSignal, void * > > > areaSelectionSubscribers;
        map< string, Spine::TextSelection > textSelections;
        map< string, list< pair< TextSelectionChangedSignal, void * > > > textSelectionSubscribers;
        mutable boost::recursive_mutex selectionsMutex;

        void emitAreaSelectionChanged(const string & name, const AreaSet & areas, bool added)
        {
            string any;
            list< pair< AreaSelectionChangedSignal, void * > > subscribers;
            map< string, list< pair< AreaSelectionChangedSignal, void * > > >::const_iterator found;

            // Any
            found = areaSelectionSubscribers.find(any);
            if (found != areaSelectionSubscribers.end()) {
                subscribers.insert(subscribers.end(), found->second.begin(), found->second.end());
            }
            // Named
            if (name != any) {
                found = areaSelectionSubscribers.find(name);
                if (found != areaSelectionSubscribers.end())
                {
                    subscribers.insert(subscribers.end(), found->second.begin(), found->second.end());
                }
            }

            typedef pair< AreaSelectionChangedSignal, void * > AreaSelectionChangedSignalPair;
            BOOST_FOREACH(AreaSelectionChangedSignalPair & subscriber, subscribers) {
                (*subscriber.first)(subscriber.second, name, areas, added);
            }
        }

        void emitTextSelectionChanged(const string & name, const TextExtentSet & extents, bool added)
        {
            string any;
            list< pair< TextSelectionChangedSignal, void * > > subscribers;
            map< string, list< pair< TextSelectionChangedSignal, void * > > >::const_iterator found;

            // Any
            found = textSelectionSubscribers.find(any);
            if (found != textSelectionSubscribers.end()) {
                subscribers.insert(subscribers.end(), found->second.begin(), found->second.end());
            }
            // Named
            if (name != any) {
                found = textSelectionSubscribers.find(name);
                if (found != textSelectionSubscribers.end())
                {
                    subscribers.insert(subscribers.end(), found->second.begin(), found->second.end());
                }
            }

            typedef pair< TextSelectionChangedSignal, void * > TextSelectionChangedSignalPair;
            BOOST_FOREACH(TextSelectionChangedSignalPair & subscriber, subscribers) {
                (*subscriber.first)(subscriber.second, name, extents, added);
            }
        }

        void * userdef;

        typedef enum { Unknown, ImageBased, TextBased } IsImageBased;
        IsImageBased imageBased;
    };


    /****************************************************************************/


    Document::Document(void * userdef_)
        : d(new DocumentPrivate)
    {
        d->userdef = userdef_;
        d->deathRowScratchId = newScratchId();
        d->imageBased = DocumentPrivate::Unknown;
    }

    Document::~Document()
    {
        delete d;
    }

    string Document::uniqueID() {return "";} // specific to derived class

    string Document::title() {return "";}
    string Document::subject() {return "";}
    string Document::keywords() {return "";}
    string Document::author() {return "";}
    string Document::creator() {return "";}
    string Document::producer() {return "";}

    static void gen_random_string(char *s, const int len) {
        static const char alphanum[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";

        for (int i = 0; i < len; ++i) {
            s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
        }

        s[len] = 0;
    }

    string Document::newScratchId(const std::string & name) const
    {
        string scratchId;
        if (!name.empty()) {
            map< string, string >::const_iterator found(d->scratchIds.find(name));
            if (found != d->scratchIds.end()) {
                scratchId = found->second;
            }
        }
        if (scratchId.empty()) {
            const int len(32);
            char id[len + 1];
            gen_random_string(id, len);
            scratchId = "__" + string(id) + "__";
            if (!name.empty()) {
                d->scratchIds[name] = scratchId;
            }
        }
        return scratchId;
    }

    string Document::deletedItemsScratchId() const
    {
        return d->deathRowScratchId;
    }

    void * Document::userdef() {return d->userdef;}
    void Document::setUserdef(void * userdef_) {d->userdef=userdef_;}

    time_t Document::creationDate() {return 0;}
    time_t Document::modificationDate() {return 0;}

    bool Document::imageBased()
    {
        if (d->imageBased == DocumentPrivate::Unknown)
        {
            // Look for some well known OCR producer keywords
            // or lack of font information
            string prod=this->producer();
            if (prod.find("Apex")!=string::npos ||
                prod.find("Capture")!=string::npos ||
                prod.find("Image")!=string::npos ||
                prod.find("Import")!=string::npos ||
                prod.find("TIF2PDF")!=string::npos) {
                d->imageBased = DocumentPrivate::ImageBased;
            }
            else
            {
                // check for full page images
                int scanned_pages(0);

                // iterate over pages
                for(CursorHandle c(this->newCursor()); c->page(); c->nextPage()) {
                    int pagewidth=static_cast<int>(c->page()->boundingBox().x2 - c->page()->boundingBox().x1);
                    int pageheight=static_cast<int>(c->page()->boundingBox().y2 - c->page()->boundingBox().y1);

                    // iterate over images, but don't cascade to the next page
                    const Image *i=c->image();
                    while( (i=c->image()) ) {
                        int imagewidth=static_cast<int>(i->boundingBox().x2 - i->boundingBox().x1);
                        int imageheight=static_cast<int>(i->boundingBox().y2 - i->boundingBox().y1);
                        if(pagewidth-imagewidth < 50 && pageheight-imageheight < 50) {
                            ++scanned_pages;
                            break;
                        }
                        // don't cascade to the next page
                        c->nextImage();
                    }
                }
                // allow 1 or 2 cover pages
                d->imageBased = (this->numberOfPages()-scanned_pages < 2) ? DocumentPrivate::ImageBased : DocumentPrivate::TextBased;
            }
        }

        return d->imageBased == DocumentPrivate::ImageBased;
    }

    string Document::characterFingerprint1()
    {
        if(d->charhash1.empty()) {
            this->calculateCharacterFingerprints();
        }
        return d->charhash1;
    }

    string Document::characterFingerprint2()
    {
        if(d->charhash2.empty()) {
            this->calculateCharacterFingerprints();
        }
        return d->charhash2;
    }

    // generate SHA-256 hash of characters
    void Document::calculateCharacterFingerprints()
    {
        Sha256 hash1;
        Sha256 hash2;
        unsigned char data[4];

        CursorHandle c(this->newCursor());

        const Character *txtchr;
        const Word *wrd;

        for( ; c->page(); c->nextPage()) {

            int pg(c->page()->pageNumber());

            // iterate over words but not advancing page
            while ( (wrd=c->word()) ) {

                // only hash horizontal text as often watermarks are rotated
                if(wrd->rotation()==0) {
                    while( (txtchr=c->character()) ) {

                        // ignore 1 inch margin
                        if(txtchr->boundingBox().x1 >= 72.0 &&
                           txtchr->boundingBox().x2 <= c->page()->boundingBox().x2-72.0 &&
                           txtchr->boundingBox().y1 >= 72.0 &&
                           txtchr->boundingBox().y2 <= c->page()->boundingBox().y2-72.0)
                        {
                            utf8::uint32_t ch(txtchr->charcode());
                            data[0]=(ch & 0xff000000) >> 24;
                            data[1]=(ch & 0x00ff0000) >> 16;
                            data[2]=(ch & 0x0000ff00) >> 8;
                            data[3]=(ch & 0x000000ff);
                            hash1.update(data, 4);
                            if (pg>1) {
                                hash2.update(data, 4);
                            }
                        }
                        c->nextCharacter();
                    }
                }
                c->nextWord(WithinPage);
            }
        }

        if(hash1.isValid()) {
            d->charhash1=Fingerprint::character1FingerprintIri(hash1.calculateHash());
        } else {
            d->charhash1.clear();
        }

        if(hash2.isValid()) {
            d->charhash2=Fingerprint::character2FingerprintIri(hash2.calculateHash());
        } else {
            d->charhash2.clear();
        }
    }

    string Document::imageFingerprint1()
    {
        if(d->imagehash1.empty()) {
            this->calculateImageFingerprints();
        }
        return d->imagehash1;
    }

    string Document::imageFingerprint2()
    {
        if(d->imagehash2.empty()) {
            this->calculateImageFingerprints();
        }
        return d->imagehash2;
    }

    void Document::calculateImageFingerprints()
    {
        Sha256 hash1;
        Sha256 hash2;

        CursorHandle c(this->newCursor());
        const Image *img;

        for( ; c->page(); c->nextPage()) {

            int pg(c->page()->pageNumber());

            // iterate over images on same page
            while( (img=c->image()) ) {
                if((img->boundingBox().width() * img->boundingBox().height()) > 5000.0) {

                    if(img->boundingBox().x2 > 72.0 &&
                       img->boundingBox().x1 < c->page()->boundingBox().x2-72.0 &&
                       img->boundingBox().y2 > 72.0  &&
                       img->boundingBox().y1 < c->page()->boundingBox().y2-72.0)
                    {
                        unsigned char *data(reinterpret_cast<unsigned char *>(img->data().get()));
                        size_t size(img->size());
                        hash1.update(data, size);
                        if (pg>1) {
                            hash2.update(data, size);
                        }
                    }
                }
                c->nextImage();
            }
        }

        if(hash1.isValid()) {
            d->imagehash1=Fingerprint::image1FingerprintIri(string(hash1.calculateHash()));
        } else {
            d->imagehash1.clear();
        }
        if(hash2.isValid()) {
            d->imagehash2=Fingerprint::image2FingerprintIri(string(hash2.calculateHash()));
        } else {
            d->imagehash2.clear();
        }
    }

    /****************************************************************************/

    Document::FingerprintSet Document::fingerprints() {
        FingerprintSet result;
        string s1(this->filehash());
        string s2(this->characterFingerprint1());
        string s3(this->characterFingerprint2());
        string s4(this->imageFingerprint1());
        string s5(this->imageFingerprint2());

        if(!s1.empty()) {
            result.insert(s1);
        }
        if(!s2.empty()) {
            result.insert(s2);
        }
        if(!s3.empty()) {
            result.insert(s3);
        }
        if(!s4.empty()) {
            result.insert(s4);
        }
        if(!s5.empty()) {
            result.insert(s5);
        }
        return result;
    }

    /****************************************************************************/

    string Document::get_prefix(const string &prefix_, size_t minlength_)
    {
        string result(scan_for_prefix(this->title(), prefix_, minlength_));

        for(CursorHandle c(this->newCursor()); result=="" && c->block(); c->nextBlock(WithinDocument)) {
            result=scan_for_prefix(c->block()->text(), prefix_, minlength_);
        }

        return result;
    }

    /****************************************************************************/

    string Document::pmid()
    {
        if(d->pmid=="") {
            d->pmid=this->get_prefix("pmid");
        }
        return d->pmid;
    }

    /****************************************************************************/

    string Document::doi()
    {
        if(d->doi=="") {
            d->doi=this->get_prefix("doi", 9);
        }
        return d->doi;
    }

    /****************************************************************************/

    string Document::pii()
    {
        if(d->pii=="") {
            d->pii=this->get_prefix("pii", 9);
        }
        return d->pii;
    }

    /****************************************************************************/

    string Document::iri()
    {
        string result(this->doi());

        if (result=="") {
            result=this->pmid();
        }

        if (result=="") {
            result=this->pii();
        }

        if (result=="") {
            result=this->uniqueID();
        }

        return result;
    }

    /****************************************************************************/

    size_t Document::wordCount()
    {
        size_t count(0);

        CursorHandle c(this->newCursor());
        const Word *w;
        while ( (w=c->word()) ) {
            ++count;
            c->nextWord(WithinDocument);
        }
        return count;
    }

    /****************************************************************************/

    Image Document::render(int pageNumber, double resolution)
    {
        CursorHandle c(this->newCursor(pageNumber));
        if (const Page * page = c->page()) {
            return page->render(resolution);
            // FIXME orientation!
        }
        return Image();
    }

    Image Document::renderArea(const Area & area, double resolution)
    {
        CursorHandle c(this->newCursor(area.page));
        if (const Page * page = c->page()) {
            return page->renderArea(area.boundingBox, resolution);
            // FIXME orientation!
        }
        return Image();
    }

    /****************************************************************************/

    TextIterator Document::begin()
    {
        return TextIterator(newCursor());
    }

    TextIterator Document::end()
    {
        CursorHandle cursor(newCursor());
        cursor->toBackPage();
        cursor->previousCharacter(WithinDocument);
        cursor->nextCharacter();
        return TextIterator(cursor);
    }

    TextExtentSet Document::search(const string & regexp, int options)
    {
        return searchFrom(begin(), regexp, options);
    }

    TextExtentSet Document::searchFrom(const TextIterator & start, const string & regexp, int options)
    {
        TextExtentHandle h(_cachedExtent(start, end()));
        return (*h).search(regexp, options);
    }

    TextExtentHandle Document::substr(int start, int len)
    {
        TextExtentHandle h(_cachedExtent(begin(), end()));
        return h->subExtent(start, len);
    }

    /****************************************************************************/

    boost::shared_ptr<Cursor> Document::cursorAt(int page, double x, double y)
    {
        // Drill down from page cursor
        CursorHandle cursor = newCursor(page);

        while (const Image * image = cursor->image())
        {
            if (image->boundingBox().contains(x, y))
            {
                break;
            }
            cursor->nextImage();
        }

        if (cursor->image() == 0)
        {
            while (const Region * region = cursor->region())
            {
                if (region->boundingBox().contains(x, y))
                {
                    while (const Block * block = cursor->block())
                    {
                        if (block->boundingBox().contains(x, y))
                        {
                            while (const Line * line = cursor->line())
                            {
                                if (line->boundingBox().contains(x, y))
                                {
                                    while (const Word * word = cursor->word())
                                    {
                                        if (word->boundingBox().contains(x, y))
                                        {
                                            while (const Character * character = cursor->character())
                                            {
                                                if (character->boundingBox().contains(x, y))
                                                {
                                                    return cursor;
                                                }
                                                cursor->nextCharacter();
                                            }
                                        }
                                        cursor->nextWord();
                                    }
                                }
                                cursor->nextLine();
                            }
                        }
                        cursor->nextBlock();
                    }
                }
                cursor->nextRegion();
            }
        }

        return cursor;
    }

    TextExtentHandle Document::resolveExtent(int page1, double x1, double y1, int page2, double x2, double y2)
    {
        TextExtentHandle extent;
        CursorHandle from(cursorAt(page1, x1, y1));
        CursorHandle to(cursorAt(page2, x2, y2));
        if (from->character() && to->character())
        {
            to->nextCharacter();
            extent = TextExtentHandle(new TextExtent(from, to));
        }
        return extent;
    }

    TextExtentHandle Document::_cachedExtent(TextIterator begin_, TextIterator end_)
    {
        pair<TextIterator, TextIterator> key=make_pair(begin_, end_);
        TextExtentHandle h;

        map<pair<TextIterator, TextIterator>, TextExtentHandle>::iterator i(_cached_extents.find(key));
        if (i==_cached_extents.end()) {
            h=TextExtentHandle(new TextExtent(begin_, end_));
            _cached_extents[key]= h;
        } else {
            h=i->second;
        }
        return h;
    }

    string Document::text()
    {
        TextExtentHandle h(_cachedExtent(begin(), end()));
        return (*h).text();
    }

    /****************************************************************************/

    std::list< std::string > Document::annotationLists() const
    {
        boost::lock_guard<boost::recursive_mutex> g(d->annotationsMutex);
        std::list< std::string > keys;
        map< string, AnnotationSet >::const_iterator iter(d->annotations.begin());
        map< string, AnnotationSet >::const_iterator end(d->annotations.end());
        for (; iter != end; ++iter) {
            keys.push_back(iter->first);
        }
        return keys;
    }

    AnnotationSet Document::annotations(const string & list) const
    {
        boost::lock_guard<boost::recursive_mutex> g(d->annotationsMutex);
        map< string, AnnotationSet >::const_iterator found_list(d->annotations.find(list));
        if (found_list != d->annotations.end()) {
            return found_list->second;
        } else {
            return AnnotationSet();
        }
    }

    AnnotationSet Document::annotationsById(const string & id, const string & list) const
    {
        boost::lock_guard<boost::recursive_mutex> g(d->annotationsMutex);
        map< string, AnnotationSet >::const_iterator found_list(d->annotations.find(list));
        if (found_list != d->annotations.end())
        {
            AnnotationSet anns;
            map< string, AnnotationSet >::const_iterator found_id(d->annotationsById.find(id));
            if (found_id != d->annotationsById.end())
            {
                BOOST_FOREACH(AnnotationHandle annotation, found_id->second)
                {
                    if (found_list->second.find(annotation) != found_list->second.end())
                    {
                        anns.insert(annotation);
                    }
                }
            }
            return anns;
        }
        else
        {
            return AnnotationSet();
        }
    }

    AnnotationSet Document::annotationsByParentId(const string & parentId, const string & list) const
    {
        boost::lock_guard<boost::recursive_mutex> g(d->annotationsMutex);
        map< string, AnnotationSet >::const_iterator found_list(d->annotations.find(list));
        if (found_list != d->annotations.end())
        {
            AnnotationSet anns;
            map< string, AnnotationSet >::const_iterator found_id(d->annotationsByParentId.find(parentId));
            if (found_id != d->annotationsByParentId.end())
            {
                BOOST_FOREACH(AnnotationHandle annotation, found_id->second)
                {
                    if (found_list->second.find(annotation) != found_list->second.end())
                    {
                        anns.insert(annotation);
                    }
                }
            }
            return anns;
        }
        else
        {
            return AnnotationSet();
        }
    }

    void Document::addAnnotation(AnnotationHandle ann_, const string & list)
    {
        AnnotationSet anns;
        anns.insert(ann_);
        addAnnotations(anns, list);
    }

    void Document::addAnnotations(const AnnotationSet & anns_, const string & list)
    {
        {
            boost::lock_guard<boost::recursive_mutex> g(d->annotationsMutex);
            BOOST_FOREACH(AnnotationHandle ann_, anns_)
            {
                // Get the annotation's IDs
                string id = ann_->getFirstProperty("id");
                string parent(ann_->getFirstProperty("parent"));

                // Add the annotation
                if (d->annotations[list].insert(ann_).second) {
                    // If this annotation hasn't been seen before, set its
                    // reference count to 0 and make it concrete
                    if (d->annotationsByIdRefCount.find(ann_.get()) == d->annotationsByIdRefCount.end()) {
                        d->annotationsByIdRefCount[ann_.get()] = 0;
                        ann_->setProperty("concrete", "1");
                    }
                    if (d->annotationsByParentIdRefCount.find(ann_.get()) == d->annotationsByParentIdRefCount.end()) {
                        d->annotationsByParentIdRefCount[ann_.get()] = 0;
                    }

                    d->annotationsById[id].insert(ann_);
                    d->annotationsByIdRefCount[ann_.get()] += 1;

                    //cerr << "+++++ addAnnotation " << list << " - " << parent << endl;
                    if (!parent.empty()) {
                        d->annotationsByParentId[parent].insert(ann_);
                        d->annotationsByParentIdRefCount[ann_.get()] += 1;
                    }
                }
            }
        }
        d->emitAnnotationsChanged(list, anns_, true);
    }

    void Document::removeAnnotation(AnnotationHandle ann_, const string & list)
    {
        AnnotationSet anns;
        anns.insert(ann_);
        removeAnnotations(anns, list);
    }

    void Document::removeAnnotations(const AnnotationSet & anns_, const string & list)
    {
        {
            boost::lock_guard<boost::recursive_mutex> g(d->annotationsMutex);
            BOOST_FOREACH(AnnotationHandle ann_, anns_)
            {
                // Get the annotation's IDs
                string id = ann_->getFirstProperty("id");
                string parent(ann_->getFirstProperty("parent"));

                // Remove the annotation
                if (d->annotations[list].erase(ann_) > 0) {
                    // Remove reference count if no longer needed, and make no longer
                    // concrete
                    d->annotationsByIdRefCount[ann_.get()] -= 1;
                    if (d->annotationsByIdRefCount[ann_.get()] <= 0) {
                        d->annotationsByIdRefCount.erase(ann_.get());
                        d->annotationsById[id].erase(ann_);
                        ann_->setProperty("concrete", "0");
                    }

                    //cerr << "+++++ removeAnnotation " << list << " - " << parent << endl;
                    if (!parent.empty()) {
                        d->annotationsByParentIdRefCount[ann_.get()] -= 1;
                        if (d->annotationsByParentIdRefCount[ann_.get()] <= 0) {
                            d->annotationsByParentIdRefCount.erase(ann_.get());
                            d->annotationsByParentId[parent].erase(ann_);
                        }
                    }
                }
            }
        }
        d->emitAnnotationsChanged(list, anns_, false);
    }

    AnnotationSet Document::annotationsAt(int page, const string & list) const
    {
        boost::lock_guard<boost::recursive_mutex> g(d->annotationsMutex);
        AnnotationSet found;
        map< string, AnnotationSet >::const_iterator found_list(d->annotations.find(list));
        if (found_list != d->annotations.end())
        {
            BOOST_FOREACH(AnnotationHandle annotation, found_list->second)
            {
                if (annotation->contains(page))
                {
                    found.insert(annotation);
                }
            }
        }
        return found;
    }

    AnnotationSet Document::annotationsAt(int page, double x, double y, const string & list) const
    {
        boost::lock_guard<boost::recursive_mutex> g(d->annotationsMutex);
        AnnotationSet found;
        map< string, AnnotationSet >::const_iterator found_list(d->annotations.find(list));
        if (found_list != d->annotations.end())
        {
            BOOST_FOREACH(AnnotationHandle annotation, found_list->second)
            {
                if (annotation->contains(page, x, y))
                {
                    found.insert(annotation);
                }
            }
        }
        return found;
    }

    AnnotationSet Document::annotationsSelected(const TextSelection & selection, const string & list) const
    {
        boost::lock_guard<boost::recursive_mutex> g(d->annotationsMutex);
        AnnotationSet found;
        map< string, AnnotationSet >::const_iterator found_list(d->annotations.find(list));
        if (found_list != d->annotations.end())
        {
            BOOST_FOREACH(AnnotationHandle annotation, found_list->second)
            {
                BOOST_FOREACH(const TextExtentHandle & extent, annotation->extents())
                {
                    if (selection.intersects(*extent))
                    {
                        found.insert(annotation);
                        break;
                    }
                }
            }
        }
        return found;
    }

    /****************************************************************************/

    void Document::clearSelection(const string & name)
    {
        clearAreaSelection(name);
        clearTextSelection(name);
    }

    const TextSelection & Document::textSelection(const string & name) const
    {
        static TextSelection not_found;
        boost::lock_guard<boost::recursive_mutex> g(d->selectionsMutex);
        map< string, TextSelection >::const_iterator found_name(d->textSelections.find(name));
        if (found_name != d->textSelections.end()) {
            return found_name->second;
        } else {
            return not_found;
        }
    }

    void Document::clearTextSelection(const string & name)
    {
        boost::lock_guard<boost::recursive_mutex> g(d->selectionsMutex);
        map< string, TextSelection >::iterator found_name(d->textSelections.find(name));
        if (found_name != d->textSelections.end()) {
            string doomed_name(found_name->first);
            TextSelection doomed(found_name->second);
            d->textSelections.erase(found_name);
            d->emitTextSelectionChanged(doomed_name, doomed, false);
        }
    }

    void Document::setTextSelection(const TextSelection & selection, const string & name)
    {
        boost::lock_guard<boost::recursive_mutex> g(d->selectionsMutex);
        clearTextSelection(name);
        d->textSelections[name] = selection;
        d->emitTextSelectionChanged(name, selection, true);
    }

    void Document::addToTextSelection(const TextSelection & selection, const string & name)
    {
        boost::lock_guard<boost::recursive_mutex> g(d->selectionsMutex);
        d->textSelections[name] += selection;
        d->emitTextSelectionChanged(name, selection, true);
    }

    void Document::removeFromTextSelection(const TextSelection & selection, const string & name)
    {
        boost::lock_guard<boost::recursive_mutex> g(d->selectionsMutex);
        d->textSelections[name] -= selection;
        d->emitTextSelectionChanged(name, selection, false);
    }

    string Document::selectionText(const string & name) const
    {
        boost::lock_guard<boost::recursive_mutex> g(d->selectionsMutex);
        map< string, TextSelection >::const_iterator found_name(d->textSelections.find(name));
        if (found_name != d->textSelections.end())
        {
            return TextSelection(found_name->second.normalised()).text();
        }
        else
        {
            return "";
        }
    }

    void Document::addToAreaSelection(const Area & area, const std::string & name)
    {
        AreaSet areas;
        areas.insert(area);
        addToAreaSelection(areas, name);
    }

    void Document::addToAreaSelection(const AreaSet & areas, const string & name)
    {
        boost::lock_guard<boost::recursive_mutex> g(d->selectionsMutex);
        d->areaSelections[name].insert(areas.begin(), areas.end()); // FIXME deal with duplicates
        d->emitAreaSelectionChanged(name, areas, true);
    }

    void Document::clearAreaSelection(const string & name)
    {
        boost::lock_guard<boost::recursive_mutex> g(d->selectionsMutex);
        map< string, AreaSet >::iterator found_name(d->areaSelections.find(name));
        if (found_name != d->areaSelections.end())
        {
            string doomed_name(found_name->first);
            AreaSet doomed(found_name->second);
            d->areaSelections.erase(found_name);
            d->emitAreaSelectionChanged(doomed_name, doomed, false);
        }
    }

    void Document::removeFromAreaSelection(const AreaSet & areas, const string & name)
    {
        boost::lock_guard<boost::recursive_mutex> g(d->selectionsMutex);
        BOOST_FOREACH(const Area & area, areas)
        {
            AreaSet::const_iterator found(d->areaSelections[name].find(area));
            if (found != d->areaSelections[name].end())
            {
                d->areaSelections[name].erase(found);
            }
        }
        d->emitAreaSelectionChanged(name, areas, false);
    }

    const AreaSet & Document::areaSelection(const string & name) const
    {
        static AreaSet not_found;
        boost::lock_guard<boost::recursive_mutex> g(d->selectionsMutex);
        map< string, AreaSet >::const_iterator found_name(d->areaSelections.find(name));
        if (found_name != d->areaSelections.end())
        {
            return found_name->second;
        }
        else
        {
            return not_found;
        }
    }

    void Document::setAreaSelection(const Area & area, const std::string & name)
    {
        AreaSet areas;
        areas.insert(area);
        setAreaSelection(areas, name);
    }

    void Document::setAreaSelection(const AreaSet & areas, const string & name)
    {
        boost::lock_guard<boost::recursive_mutex> g(d->selectionsMutex);
        clearAreaSelection(name);
        d->areaSelections[name] = areas;
        d->emitAreaSelectionChanged(name, areas, true);
    }

    /****************************************************************************/

    void Document::connectAnyAnnotationsChanged(AnnotationsChangedSignal subscriber, void * userdef)
    {
        string bogus;
        connectAnnotationsChanged(subscriber, userdef, bogus);
    }

    void Document::connectAnnotationsChanged(AnnotationsChangedSignal subscriber, void * userdef, const string & name)
    {
        boost::lock_guard<boost::recursive_mutex> g(d->selectionsMutex);
        d->annotationSubscribers[name].push_back(make_pair(subscriber, userdef));
    }

    void Document::disconnectAnyAnnotationsChanged(AnnotationsChangedSignal subscriber, void * userdef)
    {
        string bogus;
        disconnectAnnotationsChanged(subscriber, userdef, bogus);
    }

    void Document::disconnectAnnotationsChanged(AnnotationsChangedSignal subscriber, void * userdef, const string & name)
    {
        boost::lock_guard<boost::recursive_mutex> g(d->selectionsMutex);
        d->annotationSubscribers[name].remove(make_pair(subscriber, userdef));
    }



    void Document::connectAnyAreaSelectionChanged(AreaSelectionChangedSignal subscriber, void * userdef)
    {
        string bogus;
        connectAreaSelectionChanged(subscriber, userdef, bogus);
    }

    void Document::connectAreaSelectionChanged(AreaSelectionChangedSignal subscriber, void * userdef, const string & name)
    {
        boost::lock_guard<boost::recursive_mutex> g(d->selectionsMutex);
        d->areaSelectionSubscribers[name].push_back(make_pair(subscriber, userdef));
    }

    void Document::disconnectAnyAreaSelectionChanged(AreaSelectionChangedSignal subscriber, void * userdef)
    {
        string bogus;
        disconnectAreaSelectionChanged(subscriber, userdef, bogus);
    }

    void Document::disconnectAreaSelectionChanged(AreaSelectionChangedSignal subscriber, void * userdef, const string & name)
    {
        boost::lock_guard<boost::recursive_mutex> g(d->selectionsMutex);
        d->areaSelectionSubscribers[name].remove(make_pair(subscriber, userdef));
    }



    void Document::connectAnyTextSelectionChanged(TextSelectionChangedSignal subscriber, void * userdef)
    {
        string bogus;
        connectTextSelectionChanged(subscriber, userdef, bogus);
    }

    void Document::connectTextSelectionChanged(TextSelectionChangedSignal subscriber, void * userdef, const string & name)
    {
        boost::lock_guard<boost::recursive_mutex> g(d->selectionsMutex);
        d->textSelectionSubscribers[name].push_back(make_pair(subscriber, userdef));
    }

    void Document::disconnectAnyTextSelectionChanged(TextSelectionChangedSignal subscriber, void * userdef)
    {
        string bogus;
        disconnectTextSelectionChanged(subscriber, userdef, bogus);
    }

    void Document::disconnectTextSelectionChanged(TextSelectionChangedSignal subscriber, void * userdef, const string & name)
    {
        boost::lock_guard<boost::recursive_mutex> g(d->selectionsMutex);
        d->textSelectionSubscribers[name].remove(make_pair(subscriber, userdef));
    }

}

/****************************************************************************/

SpineDocument Spine::new_SpineDocument(DocumentHandle doc, SpineError *error)
{
    SpineDocument d=new_SpineDocument(error);
    d->_handle=doc->clone();
    return d;
}

SpineDocument Spine::share_SpineDocument(DocumentHandle doc, SpineError *error)
{
    SpineDocument d=new_SpineDocument(error);
    d->_handle=doc;
    return d;
}
