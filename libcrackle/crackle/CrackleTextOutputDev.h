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

//========================================================================
//
// CrackleTextOutputDev.h
//
// Copyright 2013 Lost Island Labs
// Copyright 2008 Advanced Interfaces Group
// Copyright 1997-2003 Glyph & Cog, LLC
//
//========================================================================

#ifndef CRACKLETEXTOUTPUTDEV_H
#define CRACKLETEXTOUTPUTDEV_H

#define TEXTOUT_WORD_LIST 1

#include <aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#ifdef UTOPIA_SPINE_BACKEND_POPPLER
#ifndef Link
#define Link AnnotLink
#endif
#ifndef GList
#define Glist GooList
#endif
#include <Annot.h>
#endif

#include <stdio.h>
#include "gtypes.h"
#include <GString.h>
#include <GList.h>
#include "GfxFont.h"
#include "OutputDev.h"
#include "Page.h"
#include "Object.h"

#include <spine/BoundingBox.h>
#include <spine/Image.h>
#include <boost/shared_ptr.hpp>
#include <stdio.h>

#include <map>
#include <vector>
#include <crackle/PDFFont.h>
#include <crackle/PDFFontCollection.h>

class Stream;
class GString;
class GList;
class GfxFont;
class GfxState;
class UnicodeMap;
class Link;

class CrackleTextWord;
class CrackleTextPool;
class CrackleTextLine;
class CrackleTextLineFrag;
class CrackleTextBlock;
class CrackleTextFlow;
class CrackleTextWordList;
class CrackleTextPage;

namespace Crackle
{
    class ImageCollection;
}

//------------------------------------------------------------------------

typedef void (*TextOutputFunc)(void *stream, const char *text, int len);

//------------------------------------------------------------------------
// CrackleTextFontInfo
//------------------------------------------------------------------------

class CrackleTextFontInfo {
public:

    CrackleTextFontInfo(GfxState *state);
    ~CrackleTextFontInfo();

    GBool matches(GfxState *state);

#if TEXTOUT_WORD_LIST
    // Get the font name (which may be NULL).
    GString *getFontName() { return fontName; }

    // Get font descriptor flags.
    GBool isFixedWidth() { return flags & fontFixedWidth; }
    GBool isSerif() { return flags & fontSerif; }
    GBool isSymbolic() { return flags & fontSymbolic; }
    GBool isItalic() { return flags & fontItalic; }
    GBool isBold() { return flags & fontBold; }
#endif

    GfxFont *getFont() { return gfxFont; }
private:

    GfxFont *gfxFont;
#if TEXTOUT_WORD_LIST
    GString *fontName;
    int flags;
#endif

    friend class CrackleTextWord;
    friend class CrackleTextPage;
};

//------------------------------------------------------------------------
// CrackleTextWord
//------------------------------------------------------------------------

class CrackleTextWord {
public:

    // Constructor.
    CrackleTextWord(GfxState *state, int rotA, double x0, double y0,
                    CrackleTextFontInfo *fontA, double fontSize);

    // Destructor.
    ~CrackleTextWord();

    // Add a character to the word.
    void addChar(GfxState *state, double x, double y,
                 double dx, double dy, int charPosA, int charLen,
                 Unicode u);

    // Merge <word> onto the end of <this>.
    void merge(CrackleTextWord *word);

    // Compares <this> to <word>, returning -1 (<), 0 (=), or +1 (>),
    // based on a primary-axis comparison, e.g., x ordering if rot=0.
    int primaryCmp(CrackleTextWord *word);

    // Return the distance along the primary axis between <this> and
    // <word>.
    double primaryDelta(CrackleTextWord *word);

    static int cmpYX(const void *p1, const void *p2);

    // Get the CrackleTextFontInfo object associated with this word.
    CrackleTextFontInfo *getFontInfo() { return font; }

    // Get the next CrackleTextWord on the linked list.
    CrackleTextWord *getNext() { return next; }

    Unicode lastChar() {
        Unicode res=0x20; //space
        if(len>0) {
            res=text[len-1];
        }
        return res;
    }

    void popChar() {
        if(len>0) {
            --len;
        }
    }

#if TEXTOUT_WORD_LIST
    int getLength() { return len; }
    Unicode getChar(int idx) { return text[idx]; }
    GString *getText();
    Unicode *getUnicodeText() { return text;};
    GString *getFontName() { return font->fontName; }
    void getColor(double *r, double *g, double *b)
        { *r = colorR; *g = colorG; *b = colorB; }
    void getBBox(double *xMinA, double *yMinA, double *xMaxA, double *yMaxA)
        { *xMinA = xMin; *yMinA = yMin; *xMaxA = xMax; *yMaxA = yMax; }
    void getCharBBox(int charIdx, double *xMinA, double *yMinA,
                     double *xMaxA, double *yMaxA);
    double getFontSize() { return fontSize; }
    int getRotation() { return rot; }
    int getCharPos() { return charPos[0]; }
    int getCharLen() { return charPos[len] - charPos[0]; }
    GBool getSpaceAfter() { return spaceAfter; }
#endif

    GBool isUnderlined() { return underlined; }
    Link *getLink() { return link; }
    double getEdge(int i) { return edge[i]; }
    double getBaseline () { return base; }
    GBool hasSpaceAfter  () { return spaceAfter; }
    CrackleTextWord* nextWord () { return next; };
private:

    int rot;                      // rotation, multiple of 90 degrees
    //   (0, 1, 2, or 3)
    double xMin, xMax;            // bounding box x coordinates
    double yMin, yMax;            // bounding box y coordinates
    double base;                  // baseline x or y coordinate
    Unicode *text;                // the text
    double *edge;                 // "near" edge x or y coord of each char
    //   (plus one extra entry for the last char)
    int *charPos;                       // character position (within content stream)
    //   of each char (plus one extra entry for
    //   the last char)
    int len;                    // length of text/edge/charPos arrays
    int size;                   // size of text/edge/charPos arrays
    CrackleTextFontInfo *font;            // font information
    double fontSize;              // font size
    GBool spaceAfter;             // set if there is a space between this
    //   word and the next word on the line
    CrackleTextWord *next;                // next word in line

#if TEXTOUT_WORD_LIST
    double colorR,                // word color
        colorG,
        colorB;
#endif

    GBool underlined;
    Link *link;

    friend class CrackleTextPool;
    friend class CrackleTextLine;
    friend class CrackleTextBlock;
    friend class CrackleTextFlow;
    friend class CrackleTextWordList;
    friend class CrackleTextPage;
};

//------------------------------------------------------------------------
// CrackleTextPool
//------------------------------------------------------------------------

class CrackleTextPool {
public:

    CrackleTextPool();
    ~CrackleTextPool();

    CrackleTextWord *getPool(int baseIdx) { return pool[baseIdx - minBaseIdx]; }
    void setPool(int baseIdx, CrackleTextWord *p) { pool[baseIdx - minBaseIdx] = p; }

    int getBaseIdx(double base);

    void addWord(CrackleTextWord *word);

private:

    int minBaseIdx;               // min baseline bucket index
    int maxBaseIdx;               // max baseline bucket index
    CrackleTextWord **pool;               // array of linked lists, one for each
    //   baseline value (multiple of 4 pts)
    CrackleTextWord *cursor;              // pointer to last-accessed word
    int cursorBaseIdx;            // baseline bucket index of last-accessed word

    friend class CrackleTextBlock;
    friend class CrackleTextPage;
};

//------------------------------------------------------------------------
// CrackleTextLine
//------------------------------------------------------------------------

class CrackleTextLine {
public:

    CrackleTextLine(CrackleTextBlock *blkA, int rotA, double baseA);
    ~CrackleTextLine();

    void addWord(CrackleTextWord *word);

    // Return the distance along the primary axis between <this> and
    // <line>.
    double primaryDelta(CrackleTextLine *line);

    // Compares <this> to <line>, returning -1 (<), 0 (=), or +1 (>),
    // based on a primary-axis comparison, e.g., x ordering if rot=0.
    int primaryCmp(CrackleTextLine *line);

    // Compares <this> to <line>, returning -1 (<), 0 (=), or +1 (>),
    // based on a secondary-axis comparison of the baselines, e.g., y
    // ordering if rot=0.
    int secondaryCmp(CrackleTextLine *line);

    int cmpYX(CrackleTextLine *line);

    static int cmpXY(const void *p1, const void *p2);

    void coalesce(UnicodeMap *uMap);

    // Get the head of the linked list of CrackleTextWords.
    CrackleTextWord *getWords() { return words; }

    // Get the next CrackleTextLine on the linked list.
    CrackleTextLine *getNext() { return next; }

    // Returns true if the last char of the line is a hyphen.
    GBool isHyphenated() { return hyphenated; }

    void getBBox(double *xMinA, double *yMinA, double *xMaxA, double *yMaxA)
        { *xMinA = xMin; *yMinA = yMin; *xMaxA = xMax; *yMaxA = yMax; }

    int getRotation() { return rot; }

    Unicode *getText() { return text; }
    int getLength() { return len; }

private:

    CrackleTextBlock *blk;                // parent block
    int rot;                      // text rotation
    double xMin, xMax;            // bounding box x coordinates
    double yMin, yMax;            // bounding box y coordinates
    double base;                  // baseline x or y coordinate
    CrackleTextWord *words;               // words in this line
    CrackleTextWord *lastWord;            // last word in this line
    Unicode *text;                // Unicode text of the line, including
    //   spaces between words
    double *edge;                 // "near" edge x or y coord of each char
    //   (plus one extra entry for the last char)
    int *col;                     // starting column number of each Unicode char
    int len;                      // number of Unicode chars
    int convertedLen;             // total number of converted characters
    GBool hyphenated;             // set if last char is a hyphen
    CrackleTextLine *next;                // next line in block

    friend class CrackleTextLineFrag;
    friend class CrackleTextBlock;
    friend class CrackleTextFlow;
    friend class CrackleTextWordList;
    friend class CrackleTextPage;
};

//------------------------------------------------------------------------
// CrackleTextBlock
//------------------------------------------------------------------------

class CrackleTextBlock {
public:

    CrackleTextBlock(CrackleTextPage *pageA, int rotA);
    ~CrackleTextBlock();

    void addWord(CrackleTextWord *word);

    void coalesce(UnicodeMap *uMap, double fixedPitch);

    // Update this block's priMin and priMax values, looking at <blk>.
    void updatePriMinMax(CrackleTextBlock *blk);

    static int cmpXYPrimaryRot(const void *p1, const void *p2);

    static int cmpYXPrimaryRot(const void *p1, const void *p2);

    int primaryCmp(CrackleTextBlock *blk);

    double secondaryDelta(CrackleTextBlock *blk);

    // Returns true if <this> is below <blk>, relative to the page's
    // primary rotation.
    GBool isBelow(CrackleTextBlock *blk);

    // Get the head of the linked list of CrackleTextLines.
    CrackleTextLine *getLines() { return lines; }

    // Get the next CrackleTextBlock on the linked list.
    CrackleTextBlock *getNext() { return next; }

    void getBBox(double *xMinA, double *yMinA, double *xMaxA, double *yMaxA)
        { *xMinA = xMin; *yMinA = yMin; *xMaxA = xMax; *yMaxA = yMax; }

    int getRotation() { return rot; }

private:

    CrackleTextPage *page;                // the parent page
    int rot;                      // text rotation
    double xMin, xMax;            // bounding box x coordinates
    double yMin, yMax;            // bounding box y coordinates
    double priMin, priMax;        // whitespace bounding box along primary axis

    CrackleTextPool *pool;                // pool of words (used only until lines
    //   are built)
    CrackleTextLine *lines;               // linked list of lines
    CrackleTextLine *curLine;             // most recently added line
    int nLines;                   // number of lines
    int charCount;                // number of characters in the block
    int col;                      // starting column
    int nColumns;                 // number of columns in the block

    CrackleTextBlock *next;
    CrackleTextBlock *stackNext;

    friend class CrackleTextLine;
    friend class CrackleTextLineFrag;
    friend class CrackleTextFlow;
    friend class CrackleTextWordList;
    friend class CrackleTextPage;
};

//------------------------------------------------------------------------
// CrackleTextFlow
//------------------------------------------------------------------------

class CrackleTextFlow {
public:

    CrackleTextFlow(CrackleTextPage *pageA, CrackleTextBlock *blk);
    ~CrackleTextFlow();

    // Add a block to the end of this flow.
    void addBlock(CrackleTextBlock *blk);

    // Returns true if <blk> fits below <prevBlk> in the flow, i.e., (1)
    // it uses a font no larger than the last block added to the flow,
    // and (2) it fits within the flow's [priMin, priMax] along the
    // primary axis.
    GBool blockFits(CrackleTextBlock *blk, CrackleTextBlock *prevBlk);

    // Get the head of the linked list of CrackleTextBlocks.
    CrackleTextBlock *getBlocks() { return blocks; }

    // Get the next CrackleTextFlow on the linked list.
    CrackleTextFlow *getNext() { return next; }

    void getBBox(double *xMinA, double *yMinA, double *xMaxA, double *yMaxA)
        { *xMinA = xMin; *yMinA = yMin; *xMaxA = xMax; *yMaxA = yMax; }

private:

    CrackleTextPage *page;                // the parent page
    double xMin, xMax;            // bounding box x coordinates
    double yMin, yMax;            // bounding box y coordinates
    double priMin, priMax;        // whitespace bounding box along primary axis
    CrackleTextBlock *blocks;             // blocks in flow
    CrackleTextBlock *lastBlk;            // last block in this flow
    CrackleTextFlow *next;

    friend class CrackleTextWordList;
    friend class CrackleTextPage;
};

#if TEXTOUT_WORD_LIST

//------------------------------------------------------------------------
// CrackleTextWordList
//------------------------------------------------------------------------

class CrackleTextWordList {
public:

    // Build a flat word list, in content stream order (if
    // text->rawOrder is true), physical layout order (if <physLayout>
    // is true and text->rawOrder is false), or reading order (if both
    // flags are false).
    CrackleTextWordList(CrackleTextPage *text, GBool physLayout);

    ~CrackleTextWordList();

    // Return the number of words on the list.
    int getLength();

    // Return the <idx>th word from the list.
    CrackleTextWord *get(int idx);

private:

    GList *words;                 // [TextWord]
};

#endif // TEXTOUT_WORD_LIST

//------------------------------------------------------------------------
// CrackleTextPage
//------------------------------------------------------------------------

class CrackleTextPage {
public:

    // Constructor.
    CrackleTextPage(GBool rawOrderA);

    // Destructor.
    ~CrackleTextPage();

    // Start a new page.
    void startPage(GfxState *state);

    // End the current page.
    void endPage();

    // Update the current font.
    void updateFont(GfxState *state);

    // Begin a new word.
    void beginWord(GfxState *state, double x0, double y0);

    // Add a character to the current word.
    void addChar(GfxState *state, double x, double y,
                 double dx, double dy,
                 CharCode c, int nBytes, Unicode *u, int uLen);

    // Add <nChars> invisible characters.
    void incCharCount(int nChars);

    // Begin/end an "ActualText" span, where the char indexes are
    // supplied by a marked content operator rather than the text
    // drawing operators.
    void beginActualText(GfxState *state, Unicode *u, int uLen);
    void endActualText(GfxState *state);

    // End the current word, sorting it into the list of words.
    void endWord();

    // Add a word, sorting it into the list of words.
    void addWord(CrackleTextWord *word);

    // Add a (potential) underline.
    void addUnderline(double x0, double y0, double x1, double y1);

    // Add a hyperlink.
    void addLink(int xMin, int yMin, int xMax, int yMax, Link *link);

    // Coalesce strings that look like parts of the same line.
    void coalesce(GBool physLayout, double fixedPitch, GBool doHTML);

    // Find a string.  If <startAtTop> is true, starts looking at the
    // top of the page; else if <startAtLast> is true, starts looking
    // immediately after the last find result; else starts looking at
    // <xMin>,<yMin>.  If <stopAtBottom> is true, stops looking at the
    // bottom of the page; else if <stopAtLast> is true, stops looking
    // just before the last find result; else stops looking at
    // <xMax>,<yMax>.
    GBool findText(Unicode *s, int len,
                   GBool startAtTop, GBool stopAtBottom,
                   GBool startAtLast, GBool stopAtLast,
                   GBool caseSensitive, GBool backward,
                   GBool wholeWord,
                   double *xMin, double *yMin,
                   double *xMax, double *yMax);

    // Get the text which is inside the specified rectangle.
    GString *getText(double xMin, double yMin,
                     double xMax, double yMax);

    // Find a string by character position and length.  If found, sets
    // the text bounding rectangle and returns true; otherwise returns
    // false.
    GBool findCharRange(int pos, int length,
                        double *xMin, double *yMin,
                        double *xMax, double *yMax);

    // Dump contents of page to a file.
    void dump(void *outputStream, TextOutputFunc outputFunc,
              GBool physLayout);

    // Get the head of the linked list of CrackleTextFlows.
    CrackleTextFlow *getFlows() { return flows; }

#if TEXTOUT_WORD_LIST
    // Build a flat word list, in content stream order (if
    // this->rawOrder is true), physical layout order (if <physLayout>
    // is true and this->rawOrder is false), or reading order (if both
    // flags are false).
    CrackleTextWordList *makeWordList(GBool physLayout);
#endif

    GList * getFontList() { return this->fonts; };
    const Crackle::PDFFontCollection &getFontCollection() { return this->_font_collection; }

private:

    void clear();
    void assignColumns(CrackleTextLineFrag *frags, int nFrags, GBool rot);
    int dumpFragment(Unicode *text, int len, UnicodeMap *uMap, GString *s);

    Crackle::PDFFontCollection _font_collection;

    GBool rawOrder;               // keep text in content stream order

    double pageWidth, pageHeight; // width and height of current page
    CrackleTextWord *curWord;             // currently active string
    int charPos;                  // next character position (within content
    //   stream)
    CrackleTextFontInfo *curFont; // current font
    double curFontSize;           // current font size
    int nest;                     // current nesting level (for Type 3 fonts)
    int nTinyChars;               // number of "tiny" chars seen so far
    GBool lastCharOverlap;        // set if the last added char overlapped the
    //   previous char
    Unicode *actualText;                // current "ActualText" span
    int actualTextLen;
    double actualTextX0,
        actualTextY0,
        actualTextX1,
        actualTextY1;
    int actualTextNBytes;

    CrackleTextPool *pools[4];            // a "pool" of CrackleTextWords for each rotation
    CrackleTextFlow *flows;               // linked list of flows
    CrackleTextBlock **blocks;            // array of blocks, in yx order
    int nBlocks;                  // number of blocks
    int primaryRot;               // primary rotation
    GBool primaryLR;              // primary direction (true means L-to-R,
    //   false means R-to-L)
    CrackleTextWord *rawWords;            // list of words, in raw order (only if
    //   rawOrder is set)
    CrackleTextWord *rawLastWord; // last word on rawWords list

    GList *fonts;                 // all font info objects used on this
    //   page [CrackleTextFontInfo]

    double lastFindXMin,          // coordinates of the last "find" result
        lastFindYMin;
    GBool haveLastFind;

    GList *underlines;            // [TextUnderline]
    GList *links;                 // [TextLink]

    friend class CrackleTextLine;
    friend class CrackleTextLineFrag;
    friend class CrackleTextBlock;
    friend class CrackleTextFlow;
    friend class CrackleTextWordList;
};

//------------------------------------------------------------------------
// CrackleTextOutputDev
//------------------------------------------------------------------------

class CrackleTextOutputDev: public OutputDev {
public:

    // Open a text output file.  If <fileName> is NULL, no file is
    // written (this is useful, e.g., for searching text).  If
    // <physLayoutA> is true, the original physical layout of the text
    // is maintained.  If <rawOrder> is true, the text is kept in
    // content stream order.
    CrackleTextOutputDev(char *fileName, GBool physLayoutA,
                         double fixedPitchA, GBool rawOrderA, GBool append);

    // Create a CrackleTextOutputDev which will write to a generic stream.  If
    // <physLayoutA> is true, the original physical layout of the text
    // is maintained.  If <rawOrder> is true, the text is kept in
    // content stream order.
    CrackleTextOutputDev(TextOutputFunc func, void *stream,
                         GBool physLayoutA, double fixedPitchA,
                         GBool rawOrderA);

    // Destructor.
    virtual ~CrackleTextOutputDev();

    // Check if file was successfully created.
    virtual GBool isOk() { return ok; }

    //---- get info about output device

    // Does this device use upside-down coordinates?
    // (Upside-down means (0,0) is the top left corner of the page.)
    virtual GBool upsideDown() { return gTrue; }

    // Does this device use drawChar() or drawString()?
    virtual GBool useDrawChar() { return gTrue; }

    // Does this device use beginType3Char/endType3Char?  Otherwise,
    // text in Type 3 fonts will be drawn with drawChar/drawString.
    virtual GBool interpretType3Chars() { return gFalse; }

    // Does this device need non-text content?
    virtual GBool needNonText() { return gTrue; }

    // Does this device require incCharCount to be called for text on
    // non-shown layers?
    virtual GBool needCharCount() { return gTrue; }

    //----- initialization and control

    // Start a page.
    virtual void startPage(int pageNum, GfxState *state);
#ifdef UTOPIA_SPINE_BACKEND_POPPLER
    virtual void startPage(int pageNum, GfxState *state, XRef *xref) {
      // for poppler
      startPage(pageNum,state);
    }
#endif

    // End a page.
    virtual void endPage();

    //----- save/restore graphics state
    virtual void restoreState(GfxState *state);

    //----- update text state
    virtual void updateFont(GfxState *state);

    //----- text drawing
    virtual void beginString(GfxState *state, GString *s);
    virtual void endString(GfxState *state);
    virtual void drawChar(GfxState *state, double x, double y,
                          double dx, double dy,
                          double originX, double originY,
                          CharCode c, int nBytes, Unicode *u, int uLen);
    virtual void incCharCount(int nChars);
    using OutputDev::beginActualText;
    virtual void beginActualText(GfxState *state, Unicode *u, int uLen);
    // FIXME:  virtual void beginActualText(GfxState *state, GooString *text);
    virtual void endActualText(GfxState *state);

    //----- path painting
    virtual void stroke(GfxState *state);
    virtual void fill(GfxState *state);
    virtual void eoFill(GfxState *state);

    //----- link borders
    virtual void processLink(Link *link);

    //----- special access

    // Find a string.  If <startAtTop> is true, starts looking at the
    // top of the page; else if <startAtLast> is true, starts looking
    // immediately after the last find result; else starts looking at
    // <xMin>,<yMin>.  If <stopAtBottom> is true, stops looking at the
    // bottom of the page; else if <stopAtLast> is true, stops looking
    // just before the last find result; else stops looking at
    // <xMax>,<yMax>.
    GBool findText(Unicode *s, int len,
                   GBool startAtTop, GBool stopAtBottom,
                   GBool startAtLast, GBool stopAtLast,
                   GBool caseSensitive, GBool backward,
                   GBool wholeWord,
                   double *xMin, double *yMin,
                   double *xMax, double *yMax);

    // Get the text which is inside the specified rectangle.
    GString *getText(double xMin, double yMin,
                     double xMax, double yMax);

    // Find a string by character position and length.  If found, sets
    // the text bounding rectangle and returns true; otherwise returns
    // false.
    GBool findCharRange(int pos, int length,
                        double *xMin, double *yMin,
                        double *xMax, double *yMax);

#if TEXTOUT_WORD_LIST
    // Build a flat word list, in content stream order (if
    // this->rawOrder is true), physical layout order (if
    // this->physLayout is true and this->rawOrder is false), or reading
    // order (if both flags are false).
    CrackleTextWordList *makeWordList();
#endif

    // Returns the CrackleTextPage object for the last rasterized page,
    // transferring ownership to the caller.
    CrackleTextPage *takeText();

    // Turn extra processing for HTML conversion on or off.
    void enableHTMLExtras(GBool doHTMLA) { doHTML = doHTMLA; }

    boost::shared_ptr<Crackle::ImageCollection> pageImages() {
        return _images;
    }


    virtual void drawImage(GfxState *state, Object *ref, Stream *str,
                           int width, int height, GfxImageColorMap *colorMap,
                           int *maskColors, GBool inlineImg, GBool interpolate);

#ifdef UTOPIA_SPINE_BACKEND_POPPLER
    virtual void drawImage(GfxState *state, Object *ref, Stream *str,
			   int width, int height, GfxImageColorMap *colorMap,
			   GBool interpolate, int *maskColors, GBool inlineImg) {
      // Poppler compatibility
      drawImage(state, ref, str, width, height, colorMap, maskColors, inlineImg, interpolate);
    }
#endif

private:
    TextOutputFunc outputFunc;    // output function
    void *outputStream;           // output stream
    GBool needClose;              // need to close the output file?
    //   (only if outputStream is a FILE*)
    CrackleTextPage *text;                // text for the current page
    GBool physLayout;             // maintain original physical layout when
    //   dumping text
    double fixedPitch;          // if physLayout is true and this is non-zero,
                                //   assume fixed-pitch characters with this
                                //   width
    GBool rawOrder;               // keep text in content stream order
    GBool doHTML;                 // extra processing for HTML conversion
    GBool ok;                     // set up ok?

    // store extracted images
    boost::shared_ptr<Crackle::ImageCollection> _images;

};

#endif
