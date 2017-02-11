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

#ifndef PDFDOCUMENT_INCL_
#define PDFDOCUMENT_INCL_

/*****************************************************************************
 *
 * PDFDocument.h
 *
 * Copyright 2008 Advanced Interfaces Group
 *
 ****************************************************************************/

#include <spine/Document.h>
#include <spine/Cursor.h>
#include <crackle/PDFPage.h>

#include <cstddef>
#include <cstdio>
#include <iterator>
#include <map>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/thread.hpp>

#ifdef UTOPIA_SPINE_BACKEND_POPPLER
#ifndef GList
#define GList GooList
#endif
#endif

class BaseStream;
class Object;
class PDFDoc;
class CrackleTextOutputDev;
class SplashOutputDev;
class GList;
class UnicodeMap;
class LinkDest;

namespace Crackle
{

    class PDFPageSequence;
    class PDFCursor;

    /***************************************************************************
     *
     * PDFDocument
     *
     * The root of the PDF tree. Can be constructed from memory or a file.
     * An individual document is not reentrant but distinct documents are.
     *
     * Represents a collection of pages from a PDF.
     * PDFPage instances are created lazily.
     *
     **************************************************************************/

    class PDFDocument : public Spine::Document
    {

    public:

        typedef Crackle::PDFCursor cursor;

        class const_iterator
            : public std::iterator<std::random_access_iterator_tag, Crackle::PDFPage>
        {
            /**********************************************************************
             *
             * PDFDocument::const_iterator
             *
             **********************************************************************/

        public:

            typedef const Crackle::PDFPage value_type;
            typedef std::ptrdiff_t difference_type;
            typedef const Crackle::PDFPage& reference;
            typedef const Crackle::PDFPage* pointer;

            // forward iterator methods

            const_iterator()
                : _sequence(0), _index(0) {}

            const_iterator(const const_iterator& rhs)
                : _sequence(0), _index(0) {
                *this=rhs;
            }

            virtual ~const_iterator() {}

            const_iterator& operator=(const const_iterator &rhs_) {
                if(&rhs_!=this) {
                    _sequence=rhs_._sequence;
                    _index=rhs_._index;
                }
                return *this;
            }

            bool operator==(const const_iterator &rhs_) {
                return (rhs_._sequence==this->_sequence && rhs_._index==this->_index);
            }

            bool operator!=(const const_iterator &rhs_) {
                return (! (*this==rhs_));
            }

            const PDFPage& operator*() {
                return (*_sequence)[_index];
            }

            const PDFPage* operator->() {
                return(&*(*this));
            }

            const_iterator& operator++() {
                ++_index;
                return (*this);
            }

            const_iterator operator++(int) {
                const_iterator tmp(*this);
                ++(*this);
                return (tmp);
            }

            // bidirectional iterator methods

            const_iterator& operator--() {
                --_index;
                return (*this);
            }

            const_iterator operator--(int) {
                const_iterator tmp(*this);
                --(*this);
                return (tmp);
            }

            // random access iterator methods

            const_iterator operator+ (int v_) {
                const_iterator tmp(*this);
                tmp+=v_;
                return (tmp);
            }

            const_iterator& operator+= (int v_) {
                this->_index+=v_;
                return *this;
            }

            const_iterator operator- (int v_) {
                return ((*this) + (-v_));
            }

            const_iterator& operator-= (int v_) {
                return ((*this) += (-v_));
            }

            difference_type operator- (const const_iterator &rhs) {
                return _index-rhs._index;
            }

            bool operator < (const const_iterator &rhs) {
                return _index<rhs._index;
            }

            bool operator > (const const_iterator &rhs) {
                return _index>rhs._index;
            }

            bool operator >= (const const_iterator &rhs) {
                return _index<=rhs._index;
            }

            bool operator <= (const const_iterator &rhs) {
                return _index>=rhs._index;
            }

        private:

            // this constructor is private - see operator [];
            friend class PDFDocument;

            const_iterator(PDFDocument& sequence_, unsigned int page_=0)
                : _sequence(&sequence_), _index(page_) {}

            PDFDocument *_sequence;
            unsigned int _index;
        };

        typedef Crackle::PDFPage value_type;
        typedef std::ptrdiff_t difference_type;
        typedef std::size_t size_type;

        PDFDocument();
        virtual ~PDFDocument();
        PDFDocument(const char * filename_);
        PDFDocument(boost::shared_array<char> buffer_, std::size_t length_);

        bool isOK();
        const char *errorString();

        void readFile(const char * filename_);
        void readBuffer(boost::shared_array<char> data_, size_t length_);
        void close();
        std::string data();

        ViewMode viewMode();
        PageLayout pageLayout();
        std::string metadata();
        std::string uniqueID();
        std::string pdfFileID();
        std::string filehash();
        virtual Spine::Document::FingerprintSet fingerprints();
        size_type size();
        size_t numberOfPages();
        const_iterator begin();
        const_iterator end();

        const Crackle::PDFPage& operator[](int idx_);

        std::string title();
        std::string subject();
        std::string keywords();
        std::string author();
        std::string creator();
        std::string producer();
        std::time_t creationDate();
        std::time_t modificationDate();

        boost::shared_ptr<Spine::Cursor> newCursor(int page_=1);

        Spine::DocumentHandle clone();

    private:

        // Do not copy or assign
        PDFDocument(const PDFDocument& rhs_);
        const PDFDocument& operator=(const PDFDocument& rhs_);

        void _initialise();
        void _updateAnnotations();
        void _open(BaseStream *stream_);

        std::string _addAnchor(Object *obj, std::string name="");
        std::string _addAnchor(LinkDest *dest, std::string name="");
        void _updateNameTree(Object *tree);
        void _extractOutline(GList *items, std::string label_prefix, UnicodeMap *uMap);
        void _extractLinks();

        boost::shared_ptr<PDFDoc> _doc;
        boost::shared_ptr<Object> _dict;

        // pages are created on demand, but conceptually the document
        // remains const
        mutable std::map<int, Crackle::PDFPage *> _pageMap;
        mutable boost::mutex _mutexPageMap;

        mutable boost::mutex _mutexDocument;
        static boost::mutex _globalMutexDocument;

        boost::shared_ptr<PDFDoc> xpdfDoc() { return _doc; }

        friend class PDFPage;

        boost::shared_ptr<CrackleTextOutputDev>  _textDevice;
        boost::shared_ptr<SplashOutputDev> _renderDevice;
        boost::shared_ptr<SplashOutputDev> _printDevice;

        int _crackle_errorcode;
        mutable bool _fonts_counted;

        mutable std::string _uuid;
        mutable std::string _docid;
        mutable std::string _filehash;

        boost::shared_array<char> _data;
        long _datalen;

        int _generated_anchors;
    };

    /**************************************************************************/

}

#include <crackle/PDFCursor.h>

#endif /* PDFDOCUMENT_INCL_ */
