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

#ifndef SPINEAPI_INCL_
#define SPINEAPI_INCL_

#include <stdint.h>
#include <string.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef enum {
        Spine_DefaultSearchOptions            = 0x0,
        Spine_IgnoreCase                      = 0x1,
        Spine_WholeWordsOnly                  = 0x2,
        Spine_RegExp                          = 0x4,
    } Spine_RegexOption;

    typedef enum {
        Spine_NullImage,
        Spine_RGBImage,
        Spine_JPEGImage,
        Spine_BitmapImage
    } Spine_ImageType;

    typedef enum {
        SpineError_NoError,
        SpineError_IO,
        SpineError_InvalidType,
        SpineError_InvalidArgument,
        SpineError_InvalidRegex,
        SpineError_Unknown,
    } SpineError;

    int SpineError_ok(SpineError status);
    int SpineError_failed(SpineError status);

    typedef struct SpineBufferImpl {
        char * data;
        size_t length;
    } *SpineBuffer;

    typedef struct SpineStringImpl {
        char * utf8;
        size_t length;
    } *SpineString;

    typedef struct SpineMapImpl {
        SpineString * keys;
        SpineString * values;
        size_t length;
    } *SpineMap;

    typedef struct SpineSetImpl {
        SpineString * values;
        size_t length;
    } *SpineSet;

    typedef struct SpineAnnotationImpl *SpineAnnotation;
    typedef struct SpineDocumentImpl *SpineDocument;
    typedef struct SpineCursorImpl *SpineCursor;
    typedef struct SpineTextExtentImpl *SpineTextExtent;
    typedef struct SpineImageImpl *SpineImage;

    typedef struct SpineTextExtentListImpl {
        SpineTextExtent * extents;
        size_t count;
    } *SpineTextExtentList;

    typedef struct {
        int page;
        int rotation;
        double x1;
        double y1;
        double x2;
        double y2;
    } SpineArea;

    typedef struct SpineAreaListImpl {
        SpineArea * areas;
        size_t count;
    } *SpineAreaList;

    typedef struct SpineAnnotationListImpl {
        SpineAnnotation * annotations;
        size_t count;
    } *SpineAnnotationList;

    typedef struct
    {
        double r;
        double g;
        double b;
    } SpineColor;

    /* SpineString */
    void delete_SpineString(SpineString *str, SpineError *error);
    char *SpineString_asUTF8(SpineString str, SpineError *error);
    SpineString new_SpineStringFromUTF8(const char *utf8, size_t length, SpineError *error);
    SpineString SpineString_copy(SpineString str, SpineError *error);

    /* SpineBuffer */
    void delete_SpineBuffer(SpineBuffer * buf, SpineError *error);
    SpineBuffer new_SpineBuffer(const char * data, size_t length, SpineError *error);
    SpineBuffer SpineBuffer_copy(SpineBuffer buf, SpineError *error);

    /* SpineImage */
    SpineImage new_SpineImage(SpineError *error);
    void delete_SpineImage(SpineImage *img, SpineError *error);
    int SpineImage_width(SpineImage img, SpineError *error);
    int SpineImage_height(SpineImage img, SpineError *error);
    int SpineImage_size(SpineImage img, SpineError *error);
    Spine_ImageType SpineImage_type(SpineImage img, SpineError *error);
    char *SpineImage_data(SpineImage img, SpineError *error);

    /* SpineMap */
    SpineMap new_SpineMap(size_t entries, SpineError *error);
    void delete_SpineMap(SpineMap *str, SpineError *error);

    /* SpineSet */
    void delete_SpineSet(SpineSet *array, SpineError *error);
    SpineSet new_SpineSet(size_t entries, SpineError *error);

    /* SpineTextExtentList */
    SpineTextExtentList new_SpineTextExtentList(size_t entries, SpineError *error);
    void delete_SpineTextExtentList(SpineTextExtentList *list, SpineError *error);

    /* SpineAreaList */
    SpineAreaList new_SpineAreaList(size_t entries, SpineError *error);
    void delete_SpineAreaList(SpineAreaList *list, SpineError *error);

    /* SpineAnnotationList */
    SpineAnnotationList new_SpineAnnotationList(size_t entries, SpineError *error);
    void delete_SpineAnnotationList(SpineAnnotationList *list, SpineError *error);

    /* Document */
    typedef enum {
        SpineDocument_ViewDefault,
        SpineDocument_ViewOutlines,
        SpineDocument_ViewThumbs,
        SpineDocument_ViewFullScreen,
        SpineDocument_ViewOC,
        SpineDocument_ViewAttach
    } SpineDocument_ViewMode;

    typedef enum {
        SpineDocument_LayoutDefault,
        SpineDocument_LayoutSinglePage,
        SpineDocument_LayoutOneColumn,
        SpineDocument_LayoutTwoColumnLeft,
        SpineDocument_LayoutTwoColumnRight,
        SpineDocument_LayoutTwoPageLeft,
        SpineDocument_LayoutTwoPageRight
    } SpineDocument_PageLayout;

    SpineDocument new_SpineDocument(SpineError *error);
    void delete_SpineDocument(SpineDocument *doc, SpineError *error);
    int SpineDocument_valid(SpineDocument doc, SpineError *error);
    const char *SpineDocument_errorString(SpineDocument doc, SpineError *error);

    SpineDocument_ViewMode SpineDocument_viewMode(SpineDocument doc, SpineError *error);
    SpineDocument_PageLayout SpineDocument_pageLayout(SpineDocument doc, SpineError *error);
    SpineCursor SpineDocument_newCursor(SpineDocument doc, int page, SpineError *error);

    SpineBuffer SpineDocument_data(SpineDocument doc, SpineError *error);

    SpineString SpineDocument_pmid(SpineDocument doc, SpineError *error);
    SpineString SpineDocument_doi(SpineDocument doc, SpineError *error);
    SpineString SpineDocument_pii(SpineDocument doc, SpineError *error);
    SpineString SpineDocument_uniqueID(SpineDocument doc, SpineError *error);
    SpineString SpineDocument_filehash(SpineDocument doc, SpineError *error);
    SpineString SpineDocument_iri(SpineDocument doc, SpineError *error);

    SpineSet SpineDocument_fingerprints(SpineDocument doc, SpineError *error);

    size_t SpineDocument_numberOfPages(SpineDocument doc, SpineError *error);
    size_t SpineDocument_wordCount(SpineDocument doc, SpineError *error);

    SpineString SpineDocument_title(SpineDocument doc, SpineError *error);
    SpineString SpineDocument_subject(SpineDocument doc, SpineError *error);
    SpineString SpineDocument_keywords(SpineDocument doc, SpineError *error);
    SpineString SpineDocument_author(SpineDocument doc, SpineError *error);
    SpineString SpineDocument_creator(SpineDocument doc, SpineError *error);
    SpineString SpineDocument_producer(SpineDocument doc, SpineError *error);

    SpineString SpineDocument_newScratchId(SpineDocument doc, SpineError *error);
    SpineString SpineDocument_deletedItemsScratchId(SpineDocument doc, SpineError *error);

    void * SpineDocument_userdef(SpineDocument doc, SpineError *error);
    void SpineDocument_userdefSet(SpineDocument doc, void *userdef, SpineError *error);

    time_t SpineDocument_creationDate(SpineDocument doc, SpineError *error);
    time_t SpineDocument_modificationDate(SpineDocument doc, SpineError *error);

    int SpineDocument_imageBased(SpineDocument doc, SpineError *error);

    SpineImage SpineDocument_render(SpineDocument doc, int pageNumber, float resolution, SpineError *error);
    SpineImage SpineDocument_renderArea(SpineDocument doc, SpineArea pb, float resolution, SpineError *error);

    SpineAreaList SpineDocument_areaSelection(SpineDocument doc, SpineError *error);
    SpineTextExtentList SpineDocument_textSelection(SpineDocument doc, SpineError *error);

    SpineTextExtentList SpineDocument_search(SpineDocument doc, SpineString regex, int options, SpineError *error);
    SpineTextExtentList SpineDocument_searchFrom(SpineDocument doc, SpineCursor start, SpineString regex, int options, SpineError *error);
    SpineString SpineDocument_text(SpineDocument doc, SpineError *error);
    SpineTextExtent SpineDocument_substr(SpineDocument doc, int start, int len, SpineError *error);

    SpineAnnotationList SpineDocument_annotations(SpineDocument doc, SpineError *error);
    SpineAnnotationList SpineDocument_scratchAnnotations(SpineDocument doc, SpineString list, SpineError *error);
    void SpineDocument_addAnnotation(SpineDocument doc, SpineAnnotation sa, SpineError *error);
    void SpineDocument_addAnnotations(SpineDocument doc, SpineAnnotationList sas, SpineError *error);
    void SpineDocument_addScratchAnnotation(SpineDocument doc, SpineAnnotation sa, SpineString list, SpineError *error);
    void SpineDocument_addScratchAnnotations(SpineDocument doc, SpineAnnotationList sas, SpineString list, SpineError *error);
    void SpineDocument_removeAnnotation(SpineDocument doc, SpineAnnotation sa, SpineError *error);
    void SpineDocument_removeScratchAnnotation(SpineDocument doc, SpineAnnotation sa, SpineString list, SpineError *error);
    SpineTextExtent SpineDocument_resolveExtent(SpineDocument doc, int page1, double x1, double y1, int page2, double x2, double y2, SpineError *error);

    /* Cursor */
    typedef enum { SpineCursor_DoNotIterate=0,
                   SpineCursor_WithinWord=1,
                   SpineCursor_WithinLine=2,
                   SpineCursor_WithinBlock=3,
                   SpineCursor_WithinRegion=4,
                   SpineCursor_WithinPage=5,
                   SpineCursor_WithinDocument=6,
                   SpineCursor_WithinAll=99999 } SpineCursor_IterateLimit;

    void delete_SpineCursor(SpineCursor *cur, SpineError *error);

    SpineCursor SpineCursor_copy(SpineCursor cur, SpineError *error);

    int SpineCursor_advancePage(SpineCursor cur, SpineCursor_IterateLimit limit, SpineError *error);
    int SpineCursor_retreatPage(SpineCursor cur, SpineCursor_IterateLimit limit, SpineError *error);

    void SpineCursor_gotoPage(SpineCursor cur, int page, SpineError *error);

    SpineString SpineCursor_pageText(SpineCursor cur, SpineError *error);
    int SpineCursor_pageValid(SpineCursor cur, SpineError *error);
    SpineArea SpineCursor_pageArea(SpineCursor cur, SpineError *error);

    int SpineCursor_advanceRegion(SpineCursor cur, SpineCursor_IterateLimit limit, SpineError *error);
    int SpineCursor_retreatRegion(SpineCursor cur, SpineCursor_IterateLimit limit, SpineError *error);

    SpineString SpineCursor_regionText(SpineCursor cur, SpineError *error);
    int SpineCursor_regionValid(SpineCursor cur, SpineError *error);
    SpineArea SpineCursor_regionArea(SpineCursor cur, SpineError *error);

    int SpineCursor_advanceBlock(SpineCursor cur, SpineCursor_IterateLimit limit, SpineError *error);
    int SpineCursor_retreatBlock(SpineCursor cur, SpineCursor_IterateLimit limit, SpineError *error);

    SpineString SpineCursor_blockText(SpineCursor cur, SpineError *error);
    int SpineCursor_blockValid(SpineCursor cur, SpineError *error);
    SpineArea SpineCursor_blockArea(SpineCursor cur, SpineError *error);

    int SpineCursor_advanceLine(SpineCursor cur, SpineCursor_IterateLimit limit, SpineError *error);
    int SpineCursor_retreatLine(SpineCursor cur, SpineCursor_IterateLimit limit, SpineError *error);

    SpineString SpineCursor_lineText(SpineCursor cur, SpineError *error);
    int SpineCursor_lineValid(SpineCursor cur, SpineError *error);
    SpineArea SpineCursor_lineArea(SpineCursor cur, SpineError *error);

    int SpineCursor_advanceWord(SpineCursor cur, SpineCursor_IterateLimit limit, SpineError *error);
    int SpineCursor_retreatWord(SpineCursor cur, SpineCursor_IterateLimit limit, SpineError *error);

    SpineString SpineCursor_wordText(SpineCursor cur, SpineError *error);
    int SpineCursor_wordValid(SpineCursor cur, SpineError *error);
    SpineArea SpineCursor_wordArea(SpineCursor cur, SpineError *error);
    SpineString SpineCursor_wordFontName(SpineCursor cur, SpineError *error);
    double SpineCursor_wordFontSize(SpineCursor cur, SpineError *error);
    int SpineCursor_wordSpaceAfter(SpineCursor cur, SpineError *error);
    int SpineCursor_wordUnderlined(SpineCursor cur, SpineError *error);
    double SpineCursor_wordBaseline(SpineCursor cur, SpineError *error);
    SpineColor SpineCursor_wordColor(SpineCursor cur, SpineError *error);

    int SpineCursor_advanceCharacter(SpineCursor cur, SpineCursor_IterateLimit limit, SpineError *error);
    int SpineCursor_retreatCharacter(SpineCursor cur, SpineCursor_IterateLimit limit, SpineError *error);

    SpineString SpineCursor_characterText(SpineCursor cur, SpineError *error);
    int SpineCursor_characterValid(SpineCursor cur, SpineError *error);
    SpineArea SpineCursor_characterArea(SpineCursor cur, SpineError *error);
    SpineString SpineCursor_characterFontName(SpineCursor cur, SpineError *error);
    double SpineCursor_characterFontSize(SpineCursor cur, SpineError *error);
    int SpineCursor_characterSpaceAfter(SpineCursor cur, SpineError *error);
    int SpineCursor_characterUnderlined(SpineCursor cur, SpineError *error);
    double SpineCursor_characterBaseline(SpineCursor cur, SpineError *error);
    SpineColor SpineCursor_characterColor(SpineCursor cur, SpineError *error);

    int SpineCursor_advanceImage(SpineCursor cur, SpineCursor_IterateLimit limit, SpineError *error);
    int SpineCursor_retreatImage(SpineCursor cur, SpineCursor_IterateLimit limit, SpineError *error);

    SpineImage SpineCursor_image(SpineCursor cur, SpineError *error);
    int SpineCursor_imageValid(SpineCursor cur, SpineError *error);
    SpineArea SpineCursor_imageArea(SpineCursor cur, SpineError *error);

    /* Annotation */
    SpineAnnotation new_SpineAnnotation(SpineError *error);
    void delete_SpineAnnotation(SpineAnnotation *sa, SpineError *error);
    void SpineAnnotation_insertProperty(SpineAnnotation sa, const SpineString key, const SpineString val, SpineError *error);
    void SpineAnnotation_setProperty(SpineAnnotation sa, const SpineString key, const SpineString val, SpineError *error);
    void SpineAnnotation_removePropertyAll(SpineAnnotation sa, const SpineString key, SpineError *error);
    void SpineAnnotation_removeProperty(SpineAnnotation sa, const SpineString key, const SpineString value, SpineError *error);
    SpineSet SpineAnnotation_getProperty(SpineAnnotation sa, const SpineString key, SpineError *error);
    SpineString SpineAnnotation_getFirstProperty(SpineAnnotation sa, const SpineString key, SpineError *error);
    void SpineAnnotation_clearProperties(SpineAnnotation sa, SpineError *error);
    int SpineAnnotation_hasProperty(SpineAnnotation sa, const SpineString key, SpineError *error);
    int SpineAnnotation_equal(SpineAnnotation sa1, SpineAnnotation sa2, SpineError *error);
    int SpineAnnotation_equalRegions(SpineAnnotation sa1, SpineAnnotation sa2, SpineError *error);
    int SpineAnnotation_isPublic(SpineAnnotation sa, SpineError *error);
    void SpineAnnotation_setPublic(SpineAnnotation sa, int isPublic, SpineError *error);

    SpineMap SpineAnnotation_properties(SpineAnnotation sa, SpineError *error);
    SpineTextExtentList SpineAnnotation_extents(SpineAnnotation sa, SpineError *error);
    SpineAreaList SpineAnnotation_areas(SpineAnnotation sa, SpineError *error);
    void SpineAnnotation_addArea(SpineAnnotation sa, SpineArea pb, SpineError *error);
    void SpineAnnotation_addExtent(SpineAnnotation sa, SpineTextExtent se, SpineError *error);
    void SpineAnnotation_removeArea(SpineAnnotation sa, SpineArea pb, SpineError *error);
    void SpineAnnotation_removeExtent(SpineAnnotation sa, SpineTextExtent se, SpineError *error);

    /* TextExtent */
    SpineTextExtent new_SpineTextExtent(SpineCursor start, SpineCursor to, SpineError *error);
    void delete_SpineTextExtent(SpineTextExtent *se, SpineError *error);
    SpineCursor SpineTextExtent_from(SpineTextExtent se, SpineError *error);
    SpineCursor SpineTextExtent_to(SpineTextExtent se, SpineError *error);
    SpineString SpineTextExtent_text(SpineTextExtent se, SpineError *error);
    SpineAreaList SpineTextExtent_areas(SpineTextExtent se, SpineError *error);
    SpineTextExtentList SpineTextExtent_search(SpineTextExtent se, SpineString regex, int options, SpineError *error);
    // FIXME Annotation?

#ifdef __cplusplus
}

#endif

#endif /* SPINEAPI_INCL_ */
