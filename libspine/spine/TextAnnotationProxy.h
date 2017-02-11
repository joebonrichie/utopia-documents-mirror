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

#ifndef TEXTANNOTATIONPROXY_INCL_
#define TEXTANNOTATIONPROXY_INCL_

/*****************************************************************************
 *
 * TextAnnotationProxy.h
 *
 * Copyright 2009 Advanced Interfaces Group
 *
 ****************************************************************************/

#include <boost/shared_array.hpp>

namespace Spine {

    class text_property_accessor : public property_accessor {
    public:

        text_property_accessor(AnnotationInterface *annotation_)
            : property_accessor(annotation_)
            { }

        virtual UnicodeString getProperty(const UnicodeString key_) const
            {
                UnicodeString result;

                if(key_=="spine:annotation:text:content") {
                    result=this->annotation()->getProperty("spine:annotation:text:richcontent");
                    result.findAndReplace("_", " ");
                    result.findAndReplace(0x2017, "_");
                } else {
                    result.setToBogus();
                }

                return result;
            }

        virtual bool setProperty(UnicodeString &key_, const UnicodeString &val_)
            {
                bool result(false);

                if(key_=="spine:annotation:text:content") {
                    // store the content in subsequent property_accessor in annotation using spine:annotation:text:richcontent
                    key_="spine:annotation:text:richcontent";
                    // note: we return false to continue processing
                }

                // continue processing chain
                return result;
            }

        virtual void insertProperties(std::map<UnicodeString,UnicodeString> *explicit_, std::map<UnicodeString,UnicodeString> *calculated_) const
            {
                if(calculated_) {
                    calculated_->insert(std::make_pair<UnicodeString, UnicodeString> ("spine:annotation:text:content",this->getProperty("spine:annotation:text:content")));
                }
            }
    };

    class TextAnnotationProxy : public AnnotationInterface
    {
    public:

        TextAnnotationProxy(AnnotationHandle annotation_)
            : _annotation(annotation_), _format("{0,number,integer}@{1,number}:{2,number}:{3,number}:{4,number};")
            {
                UErrorCode status=U_ZERO_ERROR;
                MessageFormat pageBoxesFormat(_format, Locale::createFromName("en_US_POSIX"), status);
                this->addAccessor("TextAnnotation", boost::shared_ptr<property_accessor> (new text_property_accessor(annotation_.get())));
                UnicodeString boxes(annotation_->getProperty("spine:annotation:region:pageboxes"));
                ParsePosition p(0);

                while(true) {
                    int32_t count(0);
                    boost::shared_array<Formattable> arguments(pageBoxesFormat.parse(boxes, p, count));

                    if (count!=5) {
                        break;
                    }

                    _boxes.push_back(PageBox(arguments[0].getLong(), BoundingBox(arguments[1].getDouble(),arguments[2].getDouble(),arguments[3].getDouble(),arguments[4].getDouble())));
                }
            }

        void clear() {
            _boxes.clear();
            _last_font="";
            _rich_content="";
            this->store();
        }

        virtual UnicodeString store() {
            UnicodeString regiontxt;
            FieldPosition fpos = 0;
            UErrorCode status=U_ZERO_ERROR;
            MessageFormat pageBoxesFormat(_format, Locale::createFromName("en_US_POSIX"), status);

            std::vector<Spine::PageBox>::iterator i;
            for(i=_boxes.begin(); i!=_boxes.end(); ++i) {

                Formattable arguments[5];
                arguments[0]=static_cast<int32_t> (i->page());
                arguments[1]=i->boundingBox().x1;
                arguments[2]=i->boundingBox().y1;
                arguments[3]=i->boundingBox().x2;
                arguments[4]=i->boundingBox().y2;

                pageBoxesFormat.format(arguments, 5, regiontxt, fpos, status);
            }

            _annotation->setProperty("spine:annotation:text:richcontent", _rich_content);
            _annotation->setProperty("spine:annotation:region:pageboxes", regiontxt);
            _annotation->setProperty("spine:annotation:type:text", "1");

            return _rich_content;
        }

        void appendPage(CursorHandle cursor_)
            {
                CursorHandle cur(cursor_->clone());
                _boxes.push_back(PageBox(cur->page()->pageNumber(), cur->page()->boundingBox()));
                const Word *w;
                while( (w=cur->word()) ) {
                    _updateContentByWord(w);
                    cur->nextWord(WithinPage);
                }
            }

        void appendRegion(CursorHandle cursor_)
            {
                CursorHandle cur(cursor_->clone());
                _boxes.push_back(PageBox(cur->page()->pageNumber(), cur->region()->boundingBox()));
                const Word *w;
                while( (w=cur->word()) ) {
                    _updateContentByWord(w);
                    cur->nextWord(WithinRegion);
                }
            }

        void appendBlock(CursorHandle cursor_)
            {
                CursorHandle cur(cursor_->clone());
                _boxes.push_back(PageBox(cur->page()->pageNumber(), cur->block()->boundingBox()));
                const Word *w;
                while( (w=cur->word()) ) {
                    _updateContentByWord(w);
                    cur->nextWord(WithinBlock);
                }
            }

        void appendLine(CursorHandle cursor_)
            {
                CursorHandle cur(cursor_->clone());
                _boxes.push_back(PageBox(cur->page()->pageNumber(), cur->line()->boundingBox()));
                const Word *w;
                while( (w=cur->word()) ) {
                    _updateContentByWord(w);
                    cur->nextWord(WithinLine);
                }
            }

        void appendWord(CursorHandle cursor_) {
            CursorHandle cur(cursor_->clone());
            _boxes.push_back(PageBox(cur->page()->pageNumber(), cur->word()->boundingBox()));
            _updateContentByWord(cur->word());
        }

        UnicodeString findAndReplace(UnicodeString find_, UnicodeString replace_) {
            return this->_rich_content.findAndReplace(find_, replace_);
        }

        bool setProperty(UnicodeString key_, UnicodeString value_)
            {
                return _annotation->setProperty(key_, value_);
            }

        UnicodeString getProperty(UnicodeString key_) const
            {
                return _annotation->getProperty(key_);
            }

        void insertProperties(std::map<UnicodeString,UnicodeString> *explicit_, std::map<UnicodeString,UnicodeString> *calculated_) const
            {
                _annotation->insertProperties(explicit_, calculated_);
            }

        void addAccessor(UnicodeString name_, boost::shared_ptr<property_accessor> accessor_)
            {
                _annotation->addAccessor(name_, accessor_);
            }

        boost::shared_ptr<property_accessor> getAccessor(UnicodeString name_)
            {
                return _annotation->getAccessor(name_);
            }

        AnnotationHandle annotation() {
            return _annotation;
        }

        virtual bool dirty() {
            return !_boxes.empty();
        }

        const std::vector<Spine::PageBox> &boxes() const
            {
                return _boxes;
            }

    private:

        void _updateContentByWord(const Word *word_)
            {
                if (word_) {

                    UnicodeString text(word_->text().trim());
                    // replace existing underscores with double underscores
                    // we will of course fail to preserve existing double underscores
                    text.findAndReplace("_", 0x2017);

                    // add a space or font change character if neccessary
                    if(!_rich_content.isEmpty()) {

                        // deal with hyphenated lines
                        // (hyphenated words should already include the hyphen)
                        bool need_space(true);
                        if(boost::u32regex_match(_rich_content,
                                                 boost::make_u32regex(".*[^\\d][[:Pd:]]"))) {
                            _rich_content.truncate(_rich_content.length()-1);
                            need_space=false;
                        }

                        // Mark font changes
                        if(UnicodeString(word_->fontName()) != _last_font) {
                            _rich_content+="_";
                            _last_font=word_->fontName();
                        } else if (need_space) {
                            _rich_content+=" ";
                        }

                    } else {
                        _last_font=word_->fontName();
                    }

                    // add text
                    _rich_content+=text;
                }
            }

        UnicodeString _last_font;
        UnicodeString _rich_content;
        std::vector<Spine::PageBox> _boxes;
        AnnotationHandle _annotation;
        const char * _format;
    };

}
#endif /* TEXTANNOTATIONPROXY_INCL_ */
