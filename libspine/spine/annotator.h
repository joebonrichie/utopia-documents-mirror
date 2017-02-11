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

#ifndef REFERENCES_INCL_
#define REFERENCES_INCL_

/*****************************************************************************
 *
 * References.h
 *
 * Copyright 2009 Advanced Interfaces Group
 *
 ****************************************************************************/

#include <spine/utility.h>
#include <spine/Cursor.h>
#include <spine/Annotation.h>
#include <spine/Document.h>
#include <spine/ReferenceAnnotationProxy.h>

#include <string>

#include <vector>

namespace Spine {

    template< class Storage >
    int extract_title(CursorHandle start_, Storage &storage_)
    {
        int result=0;
        double max_fontsize=0;
        CursorHandle best_match;

        for(CursorHandle cursor(start_->clone()); cursor->block() && cursor->word();
            cursor->nextBlock(WithinDocument)) {

            const Word *w=cursor->word();
            // skip rotated text/header/footer and ensure text is mostly black
            if(w && cursor->block()->rotation()==0) {
                if(w->color().r < 0.2 && w->color().g < 0.2 && w->color().b < 0.2 ) {
                    if(w->fontSize() > max_fontsize) {
                        max_fontsize=w->fontSize();
                        best_match=cursor->clone();
                    }
                }
            }
        }

        if(max_fontsize) {
            AnnotationHandle title_ann(new Annotation());
            TextAnnotationProxy current_annotation(title_ann);

            title_ann->setProperty("spine:annotation:type:title", "1");

            current_annotation.appendBlock(best_match);
            current_annotation.store();

            std::string bestTitle = best_match->block()->text();

            title_ann->setProperty("spine:annotation:title", bestTitle);
            storage_(title_ann);
            ++result;
        }
        return result;
    }

    template< class Storage >
    int extract_metadata(CursorHandle start_, Storage &storage_)
    {
        DocumentHandle doc(start_->document());
        AnnotationHandle identifier_ann(new Annotation());

        identifier_ann->setProperty("spine:annotation:type:document", "1");
        identifier_ann->setProperty("spine:document:identifier:hash", doc->hash());
        identifier_ann->setProperty("spine:document:identifier:pmid", doc->pmid());
        identifier_ann->setProperty("spine:document:identifier:doi", doc->doi());
        identifier_ann->setProperty("spine:document:identifier:pii", doc->pii());
        identifier_ann->setProperty("spine:document:identifier:default", doc->iri());

        identifier_ann->setProperty("spine:document:imagebased", doc->imageBased() ? "1" : "");

        UErrorCode status=U_ZERO_ERROR;

        MessageFormat countFormat("{0,number,integer}", Locale::createFromName("en_US_POSIX"), status);
        Formattable arguments[1];
        std::string wordCount;
        std::string pages;
        FieldPosition fpos = 0;

        arguments[0]=static_cast<int32_t>(doc->wordCount());
        countFormat.format(arguments, 1, wordCount, fpos, status);
        identifier_ann->setProperty("spine:document:wordcount", wordCount);

        fpos=0;
        arguments[0]=static_cast<int32_t>(doc->numberOfPages());
        countFormat.format(arguments, 1, pages, fpos, status);
        identifier_ann->setProperty("spine:document:pages", pages);

        identifier_ann->setProperty("spine:document:metadata:title", doc->title());
        identifier_ann->setProperty("spine:document:metadata:subject", doc->subject());
        identifier_ann->setProperty("spine:document:metadata:keywords", doc->keywords());
        identifier_ann->setProperty("spine:document:metadata:author", doc->author());
        identifier_ann->setProperty("spine:document:metadata:creator", doc->creator());
        identifier_ann->setProperty("spine:document:metadata:producer", doc->producer());

        MessageFormat dateFormat("{0,date,long} {0,time,long}", Locale::createFromName("en_US_POSIX"), status);
        if(doc->creationDate()) {
            arguments[0] = Formattable(static_cast<UDate>(1000.0 * doc->creationDate()), Formattable::kIsDate);
            std::string date;
            fpos=0;
            dateFormat.format(arguments, 1, date, fpos, status);
            identifier_ann->setProperty("spine:document:metadata:created", date);
        }
        if(doc->modificationDate()) {
            //      arguments[0] = utmscale_fromInt64(doc->modificationDate(), UDTS_UNIX_TIME, &status);

            arguments[0] = Formattable(static_cast<UDate>(1000.0 * doc->modificationDate()), Formattable::kIsDate);
            std::string date;
            fpos=0;
            dateFormat.format(arguments, 1, date, fpos, status);
            identifier_ann->setProperty("spine:document:metadata:modified", date);
        }

        storage_(identifier_ann);
        return 1;
    }

    template< class Storage >
    int extract_references(CursorHandle start_, Storage &storage_)
    {

        using namespace std;
        using namespace Spine;
        using namespace boost;

        const u32regex regex_numerical_key(make_u32regex("\\b[[:Ps:]]?(\\d+)(?:[[:Pe:]]|\\.)?\\s(.*?)"));
        const u32regex regex_first_numerical_key(make_u32regex("\\b[[:Pe:]]?(0*1)(?:[[:Pe:]]|\\.)?\\s(.*?)"));
        const u32regex regex_textual_key(make_u32regex("[[:Ps:]](\\w+)[[:Pe:]]\\s(.*?)"));
        const u32regex regex_any_key(make_u32regex(".*?"));
        const u32regex regex_year(make_u32regex("(?:/|[[:Ps:]])?([12]\\d{3})(?:[a-z])?(?:[\\s;,\\./]|[[:Pe:]])"));

        // include case variants to avoid needing to be able to
        // handle unicode case on mac
        const u32regex references_start_regex(make_u32regex("^([\\s\\d\\.IXV]+)?("
                                                            "REFERENCES"
                                                            "|BIBLIOGRAPHY"
                                                            "|REFERENCESANDNOTES"
                                                            "|REFERENCESCITED):?\\s*$", u32regex::icase));
        const u32regex references_end_regex(make_u32regex("^([\\s\\d\\.IXV]+)?("
                                                          "ACKNOWLEDGE?MENT"
                                                          "|AUTOBIOGRAPH"
                                                          "|BIOGRAPH"
                                                          "|TABLE"
                                                          "|APPENDIX"
                                                          "|EXHIBIT"
                                                          "|ANNEX"
                                                          "|FIG"
                                                          "|FIGURE"
                                                          "|NOTE"
                                                          "|COMPETING"
                                                          "|FURTHER"
                                                          "|SUPPLEMENTA"
                                                          ").*$", u32regex::icase));


        std::vector<AnnotationHandle> references;

        int num_references(0);
        CursorHandle references_start;
        std::string line;

        CursorHandle cursor(start_->clone());
        int first_page=cursor->page()->pageNumber();

        // iterate through pages in reverse order for a labelled references section
        for(int i=cursor->document()->numberOfPages(); !references_start && i>=first_page; --i) {
            cursor->gotoPage(i);

            // ignore header/footer
            if(!advance_to_body(cursor, WithinDocument)) {
                break;
            }

            while (!references_start && cursor->line()) {

                // strip out spaces that may just be expanded characters in headings
                std::string line(cursor->line()->text());
                line.findAndReplace(" ", "");

                // check if we have found a references heading
                if(u32regex_match(line, references_start_regex)) {
                    // found heading so set start cursor to next line
                    cursor->nextLine(WithinDocument);
                    if(!advance_to_body(cursor, WithinDocument)) {
                        break;
                    }

                    references_start=cursor;
                } else {
                    cursor->nextLine(WithinPage);
                }
            }
        }

        // no labelled references section?
        if(!references_start) {

            // walk backwards through the document looking for a numerical
            // key of 1 at the left margin close to a plausible year

            CursorHandle cursor(start_->clone());
            int first_page=cursor->page()->pageNumber();

            for(int i=cursor->document()->numberOfPages(); !references_start && i>=first_page; --i) {
                cursor->gotoPage(i);

                // ignore header/footer
                if(!advance_to_body(cursor, WithinDocument)) {
                    break;
                }

                int searching_for_year=0;
                CursorHandle possible_start;
                while (!references_start && cursor->line()) {
                    std::string line=cursor->line()->text();

                    // check for key number 1 against left margin
                    // (avoiding lower parts of hanging indents)
                    if((cursor->line()->boundingBox().x1 - cursor->block()->boundingBox().x1 < 5)
                       && u32regex_match(line, regex_first_numerical_key)) {
                        searching_for_year=1;
                        possible_start=cursor->clone();
                    }

                    // check for year
                    if(searching_for_year) {
                        if (u32regex_search(line, regex_year)) {
                            references_start=possible_start;
                        } else if (searching_for_year>=10) {
                            // We've strayed too far
                            searching_for_year=0;
                        } else {
                            ++searching_for_year;
                        }
                    }

                    cursor->nextLine(WithinPage);
                }
            }
        }

        // no labelled references section
        // and no numerical key?
        if(!references_start) {

            // walk foreward through the document looking for a textual
            // key close to a plausible year

            int searching_for_year=0;
            CursorHandle possible_start;

            for(CursorHandle cursor(start_->clone()); cursor->line();
                cursor->nextLine(WithinDocument)) {

                // ignore header/footer
                if(!advance_to_body(cursor, WithinDocument)) {
                    break;
                }

                std::string line=cursor->line()->text();

                // check for textual key against left margin
                // (avoiding lower parts of hanging indents)
                if((cursor->line()->boundingBox().x1 - cursor->block()->boundingBox().x1 < 5)
                   && u32regex_match(line, regex_textual_key)) {
                    searching_for_year=1;
                    possible_start=cursor->clone();
                }

                // check for year
                if(searching_for_year) {
                    if (u32regex_search(line, regex_year)) {
                        references_start=possible_start;
                    } else if (searching_for_year>=10) {
                        // We've strayed too far
                        searching_for_year=0;
                    } else {
                        ++searching_for_year;
                    }
                }

                cursor->nextLine(WithinPage);
            }
        }

        if(references_start) {

            // start a new annotation for each key
            AnnotationHandle annotation(new Annotation());
            ReferenceAnnotationProxy current_annotation(annotation);

            CursorHandle cursor(references_start->clone());
            if(cursor->line()){

                // determine style of key
                u32regex regex_key;
                std::string line=cursor->line()->text();

                if(u32regex_match(line, regex_numerical_key)) {
                    regex_key=regex_numerical_key;
                } else if(u32regex_match(line, regex_textual_key)) {
                    regex_key=regex_textual_key;
                } else {
                    // effectively this will just match hanging indents
                    regex_key=regex_any_key;
                }

                for( ; cursor->line(); cursor->nextLine(WithinDocument)) {

                    // ignore header/footer
                    if(!advance_to_body(cursor, WithinDocument)) {
                        break;
                    }

                    std::string line=cursor->line()->text();
                    // reached end of references?
                    std::string tmptxt(line);
                    tmptxt.findAndReplace(" ", "");
                    if(u32regex_search(tmptxt, references_end_regex)) {
                        break;
                    }

                    // over indented line? - catches IEEE bio images
                    if((cursor->line()->boundingBox().x1 - cursor->block()->boundingBox().x1 > 50)) {
                        break;
                    }

                    // found next key?
                    if((cursor->line()->boundingBox().x1 - cursor->block()->boundingBox().x1 < 5)
                       && u32regex_search(line, regex_key))
                    {
                        if (current_annotation.dirty()) {
                            current_annotation.store();
                            references.push_back(annotation);
                            ++num_references;
                            annotation= AnnotationHandle(new Annotation());
                            current_annotation= annotation;
                        }
                    }

                    current_annotation.appendLine(cursor);
                }
                if (current_annotation.dirty()){
                    current_annotation.store();
                    references.push_back(annotation);
                    ++num_references;
                }
            }

            // this is a hack for where repeated authors are denoted by
            // em-dashes. E.g. in Presence:

            std::string lastref;
            u16match what;

            if(references.size() > 0) {
                vector<AnnotationHandle>::iterator prev=references.begin();
                vector<AnnotationHandle>::iterator cur=prev;

                for (++cur; cur != references.end(); ++cur) {

                    std::string current_text=(*cur)->getProperty("spine:annotation:text:richcontent");

                    // look for dashes at start
                    if(u32regex_match(current_text, what, make_u32regex("([[:Pd:]]+\\.?[[:Zs:]]*)[[:Ps:]].*?"))) {
                        (*cur)->setProperty("spine:reference:authors", (*prev)->getProperty("spine:reference:authors"));
                    }
                    prev= cur;
                }
            }
        }

        vector<AnnotationHandle>::iterator cur;
        for (cur=references.begin(); cur != references.end(); ++cur) {
            // store annotation in functor
            (*cur)->setProperty("spine:annotation:type:reference", "1");
            storage_(*cur);
        }

        return num_references;
    }
}

#endif /* REFERENCES_INCL_ */
