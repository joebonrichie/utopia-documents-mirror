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
// CrackleTextOutputDev.cpp
//
// Copyright 2013 Lost Island Labs
// Copyright 2008 Advanced Interfaces Group
// Copyright 1997-2003 Glyph & Cog, LLC
//
//========================================================================

#include <aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <math.h>
#include <ctype.h>
#ifdef WIN32
#include <fcntl.h> // for O_BINARY
#include <io.h>    // for setmode
#endif
#include "gmem.h"
#include "GString.h"
#include "GList.h"
#include "Error.h"
#include "GlobalParams.h"
#include "UnicodeMap.h"
#include "UnicodeTypeTable.h"
#include "GfxState.h"
#include "Link.h"
#include "Object.h"
#include "Stream.h"
#include "glib/poppler-features.h"

#include "CrackleTextOutputDev.h"
#include <crackle/ImageCollection.h>
#include <utf8/unicode.h>
#include <pcrecpp.h>

extern bool unicodeTypeAlphaNum(Unicode c);

#include <vector>
#include <algorithm>

using namespace std;
using namespace Spine;
using namespace Crackle;
using namespace boost;
using namespace utf8;

//------------------------------------------------------------------------
// parameters
//------------------------------------------------------------------------

// Each bucket in a text pool includes baselines within a range of
// this many points.
#define textPoolStep 4

// Inter-character space width which will cause addChar to start a new
// word.
#define minWordBreakSpace 0.1

// Negative inter-character space width, i.e., overlap, which will
// cause addChar to start a new word.
#define minDupBreakOverlap 0.2

// Max distance between baselines of two lines within a block, as a
// fraction of the font size.
#define maxLineSpacingDelta 1.5

// Max difference in primary font sizes on two lines in the same
// block.  Delta1 is used when examining new lines above and below the
// current block; delta2 is used when examining text that overlaps the
// current block; delta3 is used when examining text to the left and
// right of the current block.
#define maxBlockFontSizeDelta1 0.05
#define maxBlockFontSizeDelta2 0.6
#define maxBlockFontSizeDelta3 0.2

// Max difference in font sizes inside a word.
#define maxWordFontSizeDelta 0.05

// Maximum distance between baselines of two words on the same line,
// e.g., distance between subscript or superscript and the primary
// baseline, as a fraction of the font size.
#define maxIntraLineDelta 0.5

// Minimum inter-word spacing, as a fraction of the font size.  (Only
// used for raw ordering.)
#define minWordSpacing 0.15

// Maximum inter-word spacing, as a fraction of the font size.
#define maxWordSpacing 1.5

// Maximum inter-word spacing, as a fraction of the font size for
// words in same block. Slightly more lax since words have already
// found their way into the same block and this catches large spaces
// between single digits and words in numbered lists for example
#define maxWordSpacing2 3.0

// Maximum horizontal spacing which will allow a word to be pulled
// into a block.
#define minColSpacing1 0.3

// Minimum spacing between columns, as a fraction of the font size.
#define minColSpacing2 1.0

// Maximum vertical spacing between blocks within a flow, as a
// multiple of the font size.
#define maxBlockSpacing 2.5

// Minimum spacing between characters within a word, as a fraction of
// the font size.
#define minCharSpacing -0.2

// Maximum spacing between characters within a word, as a fraction of
// the font size, when there is no obvious extra-wide character
// spacing.
#define maxCharSpacing 0.03

// When extra-wide character spacing is detected, the inter-character
// space threshold is set to the minimum inter-character space
// multiplied by this constant.
#define maxWideCharSpacingMul 1.3

// Upper limit on spacing between characters in a word.
#define maxWideCharSpacing 0.4

// Max difference in primary,secondary coordinates (as a fraction of
// the font size) allowed for duplicated text (fake boldface, drop
// shadows) which is to be discarded.
#define dupMaxPriDelta 0.1
#define dupMaxSecDelta 0.2

// Max width of underlines (in points).
#define maxUnderlineWidth 3

// Min distance between baseline and underline (in points).
//~ this should be font-size-dependent
#define minUnderlineGap -2

// Max distance between baseline and underline (in points).
//~ this should be font-size-dependent
#define maxUnderlineGap 4

// Max horizontal distance between edge of word and start of underline
// (in points).
//~ this should be font-size-dependent
#define underlineSlack 1

// Max distance between edge of text and edge of link border
#define hyperlinkSlack 2

// max height of a flow to skip when splitting into columns
#define columnSegmentationSkipHeight 40

//------------------------------------------------------------------------
// TextUnderline
//------------------------------------------------------------------------

class TextUnderline {
public:

    TextUnderline(double x0A, double y0A, double x1A, double y1A)
        { x0 = x0A; y0 = y0A; x1 = x1A; y1 = y1A; horiz = y0 == y1; }
    ~TextUnderline() {}

    double x0, y0, x1, y1;
    bool horiz;
};

//------------------------------------------------------------------------
// TextLink
//------------------------------------------------------------------------

class TextLink {
public:

    TextLink(int xMinA, int yMinA, int xMaxA, int yMaxA, Link *linkA)
        { xMin = xMinA; yMin = yMinA; xMax = xMaxA; yMax = yMaxA; link = linkA; }
    ~TextLink() {}

    int xMin, yMin, xMax, yMax;
    Link *link;
};

//------------------------------------------------------------------------
// CrackleTextFontInfo
//------------------------------------------------------------------------

CrackleTextFontInfo::CrackleTextFontInfo(GfxState *state) {
    gfxFont = state->getFont();
#if TEXTOUT_WORD_LIST
    fontName = (gfxFont && gfxFont->getName()) ? gfxFont->getName()->copy()
        : (GString *)NULL;
    flags = gfxFont ? gfxFont->getFlags() : 0;
#endif
}

CrackleTextFontInfo::~CrackleTextFontInfo() {
#if TEXTOUT_WORD_LIST
    if (fontName) {
        delete fontName;
    }
#endif
}

bool CrackleTextFontInfo::matches(GfxState *state) {
    return state->getFont() == gfxFont;
}

//------------------------------------------------------------------------
// CrackleTextWord
//------------------------------------------------------------------------

CrackleTextWord::CrackleTextWord(GfxState *state, int rotA, double x0, double y0,
                                 CrackleTextFontInfo *fontA, double fontSizeA) {
    GfxFont *gfxFont;
    double x, y, ascent, descent;
    int wMode;

    rot = rotA;
    font = fontA;
    fontSize = fontSizeA;
    state->transform(x0, y0, &x, &y);
    if ((gfxFont = font->gfxFont)) {
        ascent = gfxFont->getAscent() * fontSize;
        descent = gfxFont->getDescent() * fontSize;
        wMode = gfxFont->getWMode();
    } else {
        // this means that the PDF file draws text without a current font,
        // which should never happen
        ascent = 0.95 * fontSize;
        descent = -0.35 * fontSize;
        wMode = 0;
    }
    if (wMode) { // vertical writing mode
        // NB: the rotation value has been incremented by 1 (in
        // TextPage::beginWord()) for vertical writing mode
        switch (rot) {
        case 0:
            yMin = y - fontSize;
            yMax = y;
            base = y;
            break;
        case 1:
            xMin = x;
            xMax = x + fontSize;
            base = x;
            break;
        case 2:
            yMin = y;
            yMax = y + fontSize;
            base = y;
            break;
        case 3:
            xMin = x - fontSize;
            xMax = x;
            base = x;
            break;
        }
    } else { // horizontal writing mode
        switch (rot) {
        case 0:
            yMin = y - ascent;
            yMax = y - descent;
            if (yMin == yMax) {
                // this is a sanity check for a case that shouldn't happen -- but
                // if it does happen, we want to avoid dividing by zero later
                yMin = y;
                yMax = y + 1;
            }
            base = y;
            break;
        case 1:
            xMin = x + descent;
            xMax = x + ascent;
            if (xMin == xMax) {
                // this is a sanity check for a case that shouldn't happen -- but
                // if it does happen, we want to avoid dividing by zero later
                xMin = x;
                xMax = x + 1;
            }
            base = x;
            break;
        case 2:
            yMin = y + descent;
            yMax = y + ascent;
            if (yMin == yMax) {
                // this is a sanity check for a case that shouldn't happen -- but
                // if it does happen, we want to avoid dividing by zero later
                yMin = y;
                yMax = y + 1;
            }
            base = y;
            break;
        case 3:
            xMin = x - ascent;
            xMax = x - descent;
            if (xMin == xMax) {
                // this is a sanity check for a case that shouldn't happen -- but
                // if it does happen, we want to avoid dividing by zero later
                xMin = x;
                xMax = x + 1;
            }
            base = x;
            break;
        }
    }
    text = NULL;
    edge = NULL;
    charPos = NULL;
    len = size = 0;
    spaceAfter = false;
    next = NULL;

#if TEXTOUT_WORD_LIST
    GfxRGB rgb;

    if ((state->getRender() & 3) == 1) {
        state->getStrokeRGB(&rgb);
    } else {
        state->getFillRGB(&rgb);
    }
    colorR = colToDbl(rgb.r);
    colorG = colToDbl(rgb.g);
    colorB = colToDbl(rgb.b);
#endif

    underlined = false;
    link = NULL;
}

CrackleTextWord::~CrackleTextWord() {
    gfree(text);
    gfree(edge);
    gfree(charPos);
}

void CrackleTextWord::addChar(GfxState *state, double x, double y,
                              double dx, double dy, int charPosA, int charLen,
                              Unicode u) {
    int wMode;

    if (len == size) {
        size += 16;
        text = (Unicode *)greallocn(text, size, sizeof(Unicode));
        edge = (double *)greallocn(edge, size + 1, sizeof(double));
        charPos = (int *)greallocn(charPos, size + 1, sizeof(int));
    }
    text[len] = u;
    charPos[len] = charPosA;
    charPos[len + 1] = charPosA + charLen;
    wMode = font->gfxFont ? font->gfxFont->getWMode() : 0;
    if (wMode) { // vertical writing mode
        // NB: the rotation value has been incremented by 1 (in
        // TextPage::beginWord()) for vertical writing mode
        switch (rot) {
        case 0:
            if (len == 0) {
                xMin = x - fontSize;
            }
            edge[len] = x - fontSize;
            xMax = edge[len+1] = x;
            break;
        case 1:
            if (len == 0) {
                yMin = y - fontSize;
            }
            edge[len] = y - fontSize;
            yMax = edge[len+1] = y;
            break;
        case 2:
            if (len == 0) {
                xMax = x + fontSize;
            }
            edge[len] = x + fontSize;
            xMin = edge[len+1] = x;
            break;
        case 3:
            if (len == 0) {
                yMax = y + fontSize;
            }
            edge[len] = y + fontSize;
            yMin = edge[len+1] = y;
            break;
        }
    } else { // horizontal writing mode
        switch (rot) {
        case 0:
            if (len == 0) {
                xMin = x;
            }
            edge[len] = x;
            xMax = edge[len+1] = x + dx;
            break;
        case 1:
            if (len == 0) {
                yMin = y;
            }
            edge[len] = y;
            yMax = edge[len+1] = y + dy;
            break;
        case 2:
            if (len == 0) {
                xMax = x;
            }
            edge[len] = x;
            xMin = edge[len+1] = x + dx;
            break;
        case 3:
            if (len == 0) {
                yMax = y;
            }
            edge[len] = y;
            yMin = edge[len+1] = y + dy;
            break;
        }
    }
    ++len;
}

void CrackleTextWord::merge(CrackleTextWord *word) {
    int i;

    if (word->xMin < xMin) {
        xMin = word->xMin;
    }
    if (word->yMin < yMin) {
        yMin = word->yMin;
    }
    if (word->xMax > xMax) {
        xMax = word->xMax;
    }
    if (word->yMax > yMax) {
        yMax = word->yMax;
    }
    if (len + word->len > size) {
        size = len + word->len;
        text = (Unicode *)greallocn(text, size, sizeof(Unicode));
        edge = (double *)greallocn(edge, size + 1, sizeof(double));
        charPos = (int *)greallocn(charPos, size + 1, sizeof(int));
    }
    for (i = 0; i < word->len; ++i) {
        text[len + i] = word->text[i];
        edge[len + i] = word->edge[i];
        charPos[len + i] = word->charPos[i];
    }
    edge[len + word->len] = word->edge[word->len];
    charPos[len + word->len] = word->charPos[word->len];
    len += word->len;
}

inline int CrackleTextWord::primaryCmp(CrackleTextWord *word) {
    double cmp;

    cmp = 0; // make gcc happy
    switch (rot) {
    case 0:
        cmp = xMin - word->xMin;
        break;
    case 1:
        cmp = yMin - word->yMin;
        break;
    case 2:
        cmp = word->xMax - xMax;
        break;
    case 3:
        cmp = word->yMax - yMax;
        break;
    }
    return cmp < 0 ? -1 : cmp > 0 ? 1 : 0;
}

double CrackleTextWord::primaryDelta(CrackleTextWord *word) {
    double delta;

    delta = 0; // make gcc happy
    switch (rot) {
    case 0:
        delta = word->xMin - xMax;
        break;
    case 1:
        delta = word->yMin - yMax;
        break;
    case 2:
        delta = xMin - word->xMax;
        break;
    case 3:
        delta = yMin - word->yMax;
        break;
    }
    return delta;
}

int CrackleTextWord::cmpYX(const void *p1, const void *p2) {
    CrackleTextWord *word1 = *(CrackleTextWord **)p1;
    CrackleTextWord *word2 = *(CrackleTextWord **)p2;
    double cmp;

    cmp = word1->yMin - word2->yMin;
    if (cmp == 0) {
        cmp = word1->xMin - word2->xMin;
    }
    return cmp < 0 ? -1 : cmp > 0 ? 1 : 0;
}

#if TEXTOUT_WORD_LIST

GString *CrackleTextWord::getText() {
    GString *s;
    UnicodeMap *uMap;
    char buf[8];
    int n, i;

    s = new GString();
    if (!(uMap = globalParams->getTextEncoding())) {
        return s;
    }
    for (i = 0; i < len; ++i) {
        n = uMap->mapUnicode(text[i], buf, sizeof(buf));
        s->append(buf, n);
    }
    uMap->decRefCnt();
    return s;
}

void CrackleTextWord::getCharBBox(int charIdx, double *xMinA, double *yMinA,
                                  double *xMaxA, double *yMaxA) {
    if (charIdx < 0 || charIdx >= len) {
        return;
    }
    switch (rot) {
    case 0:
        *xMinA = edge[charIdx];
        *xMaxA = edge[charIdx + 1];
        *yMinA = yMin;
        *yMaxA = yMax;
        break;
    case 1:
        *xMinA = xMin;
        *xMaxA = xMax;
        *yMinA = edge[charIdx];
        *yMaxA = edge[charIdx + 1];
        break;
    case 2:
        *xMinA = edge[charIdx + 1];
        *xMaxA = edge[charIdx];
        *yMinA = yMin;
        *yMaxA = yMax;
        break;
    case 3:
        *xMinA = xMin;
        *xMaxA = xMax;
        *yMinA = edge[charIdx + 1];
        *yMaxA = edge[charIdx];
        break;
    }
}

#endif // TEXTOUT_WORD_LIST

//------------------------------------------------------------------------
// CrackleTextPool
//------------------------------------------------------------------------

CrackleTextPool::CrackleTextPool() {
    minBaseIdx = 0;
    maxBaseIdx = -1;
    pool = NULL;
    cursor = NULL;
    cursorBaseIdx = -1;
}

CrackleTextPool::~CrackleTextPool() {
    int baseIdx;
    CrackleTextWord *word, *word2;

    for (baseIdx = minBaseIdx; baseIdx <= maxBaseIdx; ++baseIdx) {
        for (word = pool[baseIdx - minBaseIdx]; word; word = word2) {
            word2 = word->next;
            delete word;
        }
    }
    gfree(pool);
}

int CrackleTextPool::getBaseIdx(double base) {
    int baseIdx;

    baseIdx = (int)(base / textPoolStep);
    if (baseIdx < minBaseIdx) {
        return minBaseIdx;
    }
    if (baseIdx > maxBaseIdx) {
        return maxBaseIdx;
    }
    return baseIdx;
}

void CrackleTextPool::addWord(CrackleTextWord *word) {
    CrackleTextWord **newPool;
    int wordBaseIdx, newMinBaseIdx, newMaxBaseIdx, baseIdx;
    CrackleTextWord *w0, *w1;

    // expand the array if needed
    wordBaseIdx = (int)(word->base / textPoolStep);
    if (minBaseIdx > maxBaseIdx) {
        minBaseIdx = wordBaseIdx - 128;
        maxBaseIdx = wordBaseIdx + 128;
        pool = (CrackleTextWord **)gmallocn(maxBaseIdx - minBaseIdx + 1,
                                            sizeof(CrackleTextWord *));
        for (baseIdx = minBaseIdx; baseIdx <= maxBaseIdx; ++baseIdx) {
            pool[baseIdx - minBaseIdx] = NULL;
        }
    } else if (wordBaseIdx < minBaseIdx) {
        newMinBaseIdx = wordBaseIdx - 128;
        newPool = (CrackleTextWord **)gmallocn(maxBaseIdx - newMinBaseIdx + 1,
                                               sizeof(CrackleTextWord *));
        for (baseIdx = newMinBaseIdx; baseIdx < minBaseIdx; ++baseIdx) {
            newPool[baseIdx - newMinBaseIdx] = NULL;
        }
        memcpy(&newPool[minBaseIdx - newMinBaseIdx], pool,
               (maxBaseIdx - minBaseIdx + 1) * sizeof(CrackleTextWord *));
        gfree(pool);
        pool = newPool;
        minBaseIdx = newMinBaseIdx;
    } else if (wordBaseIdx > maxBaseIdx) {
        newMaxBaseIdx = wordBaseIdx + 128;
        pool = (CrackleTextWord **)greallocn(pool, newMaxBaseIdx - minBaseIdx + 1,
                                             sizeof(CrackleTextWord *));
        for (baseIdx = maxBaseIdx + 1; baseIdx <= newMaxBaseIdx; ++baseIdx) {
            pool[baseIdx - minBaseIdx] = NULL;
        }
        maxBaseIdx = newMaxBaseIdx;
    }

    // insert the new word
    if (cursor && wordBaseIdx == cursorBaseIdx &&
        word->primaryCmp(cursor) >= 0) {
        w0 = cursor;
        w1 = cursor->next;
    } else {
        w0 = NULL;
        w1 = pool[wordBaseIdx - minBaseIdx];
    }
    for (; w1 && word->primaryCmp(w1) > 0; w0 = w1, w1 = w1->next) ;
    word->next = w1;
    if (w0) {
        w0->next = word;
    } else {
        pool[wordBaseIdx - minBaseIdx] = word;
    }
    cursor = word;
    cursorBaseIdx = wordBaseIdx;
}

//------------------------------------------------------------------------
// CrackleTextLine
//------------------------------------------------------------------------

CrackleTextLine::CrackleTextLine(CrackleTextBlock *blkA, int rotA, double baseA) {
    blk = blkA;
    rot = rotA;
    xMin = yMin = 0;
    xMax = yMax = -1;
    base = baseA;
    words = lastWord = NULL;
    text = NULL;
    edge = NULL;
    col = NULL;
    len = 0;
    convertedLen = 0;
    hyphenated = false;
    next = NULL;
}

CrackleTextLine::~CrackleTextLine() {
    CrackleTextWord *word;

    while (words) {
        word = words;
        words = words->next;
        delete word;
    }
    gfree(text);
    gfree(edge);
    gfree(col);
}

void CrackleTextLine::addWord(CrackleTextWord *word) {
    if (lastWord) {
        lastWord->next = word;
    } else {
        words = word;
    }
    lastWord = word;

    if (xMin > xMax) {
        xMin = word->xMin;
        xMax = word->xMax;
        yMin = word->yMin;
        yMax = word->yMax;
    } else {
        if (word->xMin < xMin) {
            xMin = word->xMin;
        }
        if (word->xMax > xMax) {
            xMax = word->xMax;
        }
        if (word->yMin < yMin) {
            yMin = word->yMin;
        }
        if (word->yMax > yMax) {
            yMax = word->yMax;
        }
    }
}

double CrackleTextLine::primaryDelta(CrackleTextLine *line) {
    double delta;

    delta = 0; // make gcc happy
    switch (rot) {
    case 0:
        delta = line->xMin - xMax;
        break;
    case 1:
        delta = line->yMin - yMax;
        break;
    case 2:
        delta = xMin - line->xMax;
        break;
    case 3:
        delta = yMin - line->yMax;
        break;
    }
    return delta;
}

int CrackleTextLine::primaryCmp(CrackleTextLine *line) {
    double cmp;

    cmp = 0; // make gcc happy
    switch (rot) {
    case 0:
        cmp = xMin - line->xMin;
        break;
    case 1:
        cmp = yMin - line->yMin;
        break;
    case 2:
        cmp = line->xMax - xMax;
        break;
    case 3:
        cmp = line->yMax - yMax;
        break;
    }
    return cmp < 0 ? -1 : cmp > 0 ? 1 : 0;
}

int CrackleTextLine::secondaryCmp(CrackleTextLine *line) {
    double cmp;

    cmp = (rot == 0 || rot == 3) ? base - line->base : line->base - base;
    return cmp < 0 ? -1 : cmp > 0 ? 1 : 0;
}

int CrackleTextLine::cmpYX(CrackleTextLine *line) {
    int cmp;

    if ((cmp = secondaryCmp(line))) {
        return cmp;
    }
    return primaryCmp(line);
}

int CrackleTextLine::cmpXY(const void *p1, const void *p2) {
    CrackleTextLine *line1 = *(CrackleTextLine **)p1;
    CrackleTextLine *line2 = *(CrackleTextLine **)p2;
    int cmp;

    if ((cmp = line1->primaryCmp(line2))) {
        return cmp;
    }
    return line1->secondaryCmp(line2);
}

void CrackleTextLine::coalesce(UnicodeMap *uMap) {
    CrackleTextWord *word0, *word1;
    double space, delta, minSpace;
    bool isUnicode;
    char buf[8];
    int i, j;

    if (words->next) {

        // compute the inter-word space threshold
        if (words->len > 1 || words->next->len > 1) {
            minSpace = 0;
        } else {
            minSpace = words->primaryDelta(words->next);
            for (word0 = words->next, word1 = word0->next;
                 word1 && minSpace > 0;
                 word0 = word1, word1 = word0->next) {
                if (word1->len > 1) {
                    minSpace = 0;
                }
                delta = word0->primaryDelta(word1);
                if (delta < minSpace) {
                    minSpace = delta;
                }
            }
        }
        if (minSpace <= 0) {
            space = maxCharSpacing * words->fontSize;
        } else {
            space = maxWideCharSpacingMul * minSpace;
            if (space > maxWideCharSpacing * words->fontSize) {
                space = maxWideCharSpacing * words->fontSize;
            }
        }

        // merge words
        word0 = words;
        word1 = words->next;
        while (word1) {
            if (word0->primaryDelta(word1) >= space) {
                word0->spaceAfter = true;
                word0 = word1;
                word1 = word1->next;
            } else if (word0->font == word1->font &&
                       word0->underlined == word1->underlined &&
                       fabs(word0->fontSize - word1->fontSize) <
                       maxWordFontSizeDelta * words->fontSize &&
                       word1->charPos[0] == word0->charPos[word0->len]) {
                word0->merge(word1);
                word0->next = word1->next;
                delete word1;
                word1 = word0->next;
            } else {
                word0 = word1;
                word1 = word1->next;
            }
        }
    }

    // build the line text
    isUnicode = uMap ? uMap->isUnicode() : false;
    len = 0;
    for (word1 = words; word1; word1 = word1->next) {
        len += word1->len;
        if (word1->spaceAfter) {
            ++len;
        }
    }
    text = (Unicode *)gmallocn(len, sizeof(Unicode));
    edge = (double *)gmallocn(len + 1, sizeof(double));
    i = 0;
    for (word1 = words; word1; word1 = word1->next) {
        for (j = 0; j < word1->len; ++j) {
            text[i] = word1->text[j];
            edge[i] = word1->edge[j];
            ++i;
        }
        edge[i] = word1->edge[word1->len];
        if (word1->spaceAfter) {
            text[i] = (Unicode)0x0020;
            ++i;
        }
    }

    // compute convertedLen and set up the col array
    col = (int *)gmallocn(len + 1, sizeof(int));
    convertedLen = 0;
    for (i = 0; i < len; ++i) {
        col[i] = convertedLen;
        if (isUnicode) {
            ++convertedLen;
        } else if (uMap) {
            convertedLen += uMap->mapUnicode(text[i], buf, sizeof(buf));
        }
    }
    col[len] = convertedLen;

    // check for hyphen at end of line
    //~ need to check for other chars used as hyphens
    if(text[len-1] == (Unicode)'-' ||
       text[len-1] == (Unicode)0x2010 ||
       text[len-1] == (Unicode)0x2011) {
        // Non breaking hyphen is 2011 in unicode
        // Hyphen is 2010 in unicode
        hyphenated = true;
    } else {
        hyphenated = false;
    }
}

//------------------------------------------------------------------------
// CrackleTextLineFrag
//------------------------------------------------------------------------

class CrackleTextLineFrag {
public:

    CrackleTextLine *line;                // the line object
    int start, len;               // offset and length of this fragment
    //   (in Unicode chars)
    double xMin, xMax;            // bounding box coordinates
    double yMin, yMax;
    double base;                  // baseline virtual coordinate
    int col;                      // first column

    void init(CrackleTextLine *lineA, int startA, int lenA);
    void computeCoords(bool oneRot);

    static int cmpYXPrimaryRot(const void *p1, const void *p2);
    static int cmpYXLineRot(const void *p1, const void *p2);
    static int cmpXYLineRot(const void *p1, const void *p2);
    static int cmpXYColumnPrimaryRot(const void *p1, const void *p2);
    static int cmpXYColumnLineRot(const void *p1, const void *p2);
};

void CrackleTextLineFrag::init(CrackleTextLine *lineA, int startA, int lenA) {
    line = lineA;
    start = startA;
    len = lenA;
    col = line->col[start];
}

void CrackleTextLineFrag::computeCoords(bool oneRot) {
    CrackleTextBlock *blk;
    double d0, d1, d2, d3, d4;

    if (oneRot) {

        switch (line->rot) {
        case 0:
            xMin = line->edge[start];
            xMax = line->edge[start + len];
            yMin = line->yMin;
            yMax = line->yMax;
            break;
        case 1:
            xMin = line->xMin;
            xMax = line->xMax;
            yMin = line->edge[start];
            yMax = line->edge[start + len];
            break;
        case 2:
            xMin = line->edge[start + len];
            xMax = line->edge[start];
            yMin = line->yMin;
            yMax = line->yMax;
            break;
        case 3:
            xMin = line->xMin;
            xMax = line->xMax;
            yMin = line->edge[start + len];
            yMax = line->edge[start];
            break;
        }
        base = line->base;

    } else {

        if (line->rot == 0 && line->blk->page->primaryRot == 0) {

            xMin = line->edge[start];
            xMax = line->edge[start + len];
            yMin = line->yMin;
            yMax = line->yMax;
            base = line->base;

        } else {

            blk = line->blk;
            d0 = line->edge[start];
            d1 = line->edge[start + len];
            d2 = d3 = d4 = 0; // make gcc happy

            switch (line->rot) {
            case 0:
                d2 = line->yMin;
                d3 = line->yMax;
                d4 = line->base;
                d0 = (d0 - blk->xMin) / (blk->xMax - blk->xMin);
                d1 = (d1 - blk->xMin) / (blk->xMax - blk->xMin);
                d2 = (d2 - blk->yMin) / (blk->yMax - blk->yMin);
                d3 = (d3 - blk->yMin) / (blk->yMax - blk->yMin);
                d4 = (d4 - blk->yMin) / (blk->yMax - blk->yMin);
                break;
            case 1:
                d2 = line->xMax;
                d3 = line->xMin;
                d4 = line->base;
                d0 = (d0 - blk->yMin) / (blk->yMax - blk->yMin);
                d1 = (d1 - blk->yMin) / (blk->yMax - blk->yMin);
                d2 = (blk->xMax - d2) / (blk->xMax - blk->xMin);
                d3 = (blk->xMax - d3) / (blk->xMax - blk->xMin);
                d4 = (blk->xMax - d4) / (blk->xMax - blk->xMin);
                break;
            case 2:
                d2 = line->yMax;
                d3 = line->yMin;
                d4 = line->base;
                d0 = (blk->xMax - d0) / (blk->xMax - blk->xMin);
                d1 = (blk->xMax - d1) / (blk->xMax - blk->xMin);
                d2 = (blk->yMax - d2) / (blk->yMax - blk->yMin);
                d3 = (blk->yMax - d3) / (blk->yMax - blk->yMin);
                d4 = (blk->yMax - d4) / (blk->yMax - blk->yMin);
                break;
            case 3:
                d2 = line->xMin;
                d3 = line->xMax;
                d4 = line->base;
                d0 = (blk->yMax - d0) / (blk->yMax - blk->yMin);
                d1 = (blk->yMax - d1) / (blk->yMax - blk->yMin);
                d2 = (d2 - blk->xMin) / (blk->xMax - blk->xMin);
                d3 = (d3 - blk->xMin) / (blk->xMax - blk->xMin);
                d4 = (d4 - blk->xMin) / (blk->xMax - blk->xMin);
                break;
            }

            switch (line->blk->page->primaryRot) {
            case 0:
                xMin = blk->xMin + d0 * (blk->xMax - blk->xMin);
                xMax = blk->xMin + d1 * (blk->xMax - blk->xMin);
                yMin = blk->yMin + d2 * (blk->yMax - blk->yMin);
                yMax = blk->yMin + d3 * (blk->yMax - blk->yMin);
                base = blk->yMin + d4 * (blk->yMax - blk->yMin);
                break;
            case 1:
                xMin = blk->xMax - d3 * (blk->xMax - blk->xMin);
                xMax = blk->xMax - d2 * (blk->xMax - blk->xMin);
                yMin = blk->yMin + d0 * (blk->yMax - blk->yMin);
                yMax = blk->yMin + d1 * (blk->yMax - blk->yMin);
                base = blk->xMax - d4 * (blk->xMax - blk->xMin);
                break;
            case 2:
                xMin = blk->xMax - d1 * (blk->xMax - blk->xMin);
                xMax = blk->xMax - d0 * (blk->xMax - blk->xMin);
                yMin = blk->yMax - d3 * (blk->yMax - blk->yMin);
                yMax = blk->yMax - d2 * (blk->yMax - blk->yMin);
                base = blk->yMax - d4 * (blk->yMax - blk->yMin);
                break;
            case 3:
                xMin = blk->xMin + d2 * (blk->xMax - blk->xMin);
                xMax = blk->xMin + d3 * (blk->xMax - blk->xMin);
                yMin = blk->yMax - d1 * (blk->yMax - blk->yMin);
                yMax = blk->yMax - d0 * (blk->yMax - blk->yMin);
                base = blk->xMin + d4 * (blk->xMax - blk->xMin);
                break;
            }

        }
    }
}

int CrackleTextLineFrag::cmpYXPrimaryRot(const void *p1, const void *p2) {
    CrackleTextLineFrag *frag1 = (CrackleTextLineFrag *)p1;
    CrackleTextLineFrag *frag2 = (CrackleTextLineFrag *)p2;
    double cmp;

    cmp = 0; // make gcc happy
    switch (frag1->line->blk->page->primaryRot) {
    case 0:
        if (fabs(cmp = frag1->yMin - frag2->yMin) < 0.01) {
            cmp = frag1->xMin - frag2->xMin;
        }
        break;
    case 1:
        if (fabs(cmp = frag2->xMax - frag1->xMax) < 0.01) {
            cmp = frag1->yMin - frag2->yMin;
        }
        break;
    case 2:
        if (fabs(cmp = frag2->yMin - frag1->yMin) < 0.01) {
            cmp = frag2->xMax - frag1->xMax;
        }
        break;
    case 3:
        if (fabs(cmp = frag1->xMax - frag2->xMax) < 0.01) {
            cmp = frag2->yMax - frag1->yMax;
        }
        break;
    }
    return cmp < 0 ? -1 : cmp > 0 ? 1 : 0;
}

int CrackleTextLineFrag::cmpYXLineRot(const void *p1, const void *p2) {
    CrackleTextLineFrag *frag1 = (CrackleTextLineFrag *)p1;
    CrackleTextLineFrag *frag2 = (CrackleTextLineFrag *)p2;
    double cmp;

    cmp = 0; // make gcc happy
    switch (frag1->line->rot) {
    case 0:
        if ((cmp = frag1->yMin - frag2->yMin) == 0) {
            cmp = frag1->xMin - frag2->xMin;
        }
        break;
    case 1:
        if ((cmp = frag2->xMax - frag1->xMax) == 0) {
            cmp = frag1->yMin - frag2->yMin;
        }
        break;
    case 2:
        if ((cmp = frag2->yMin - frag1->yMin) == 0) {
            cmp = frag2->xMax - frag1->xMax;
        }
        break;
    case 3:
        if ((cmp = frag1->xMax - frag2->xMax) == 0) {
            cmp = frag2->yMax - frag1->yMax;
        }
        break;
    }
    return cmp < 0 ? -1 : cmp > 0 ? 1 : 0;
}

int CrackleTextLineFrag::cmpXYLineRot(const void *p1, const void *p2) {
    CrackleTextLineFrag *frag1 = (CrackleTextLineFrag *)p1;
    CrackleTextLineFrag *frag2 = (CrackleTextLineFrag *)p2;
    double cmp;

    cmp = 0; // make gcc happy
    switch (frag1->line->rot) {
    case 0:
        if ((cmp = frag1->xMin - frag2->xMin) == 0) {
            cmp = frag1->yMin - frag2->yMin;
        }
        break;
    case 1:
        if ((cmp = frag1->yMin - frag2->yMin) == 0) {
            cmp = frag2->xMax - frag1->xMax;
        }
        break;
    case 2:
        if ((cmp = frag2->xMax - frag1->xMax) == 0) {
            cmp = frag2->yMin - frag1->yMin;
        }
        break;
    case 3:
        if ((cmp = frag2->yMax - frag1->yMax) == 0) {
            cmp = frag1->xMax - frag2->xMax;
        }
        break;
    }
    return cmp < 0 ? -1 : cmp > 0 ? 1 : 0;
}

int CrackleTextLineFrag::cmpXYColumnPrimaryRot(const void *p1, const void *p2) {
    CrackleTextLineFrag *frag1 = (CrackleTextLineFrag *)p1;
    CrackleTextLineFrag *frag2 = (CrackleTextLineFrag *)p2;
    double cmp;

    // if columns overlap, compare y values
    if (frag1->col < frag2->col + (frag2->line->col[frag2->start + frag2->len] -
                                   frag2->line->col[frag2->start]) &&
        frag2->col < frag1->col + (frag1->line->col[frag1->start + frag1->len] -
                                   frag1->line->col[frag1->start])) {
        cmp = 0; // make gcc happy
        switch (frag1->line->blk->page->primaryRot) {
        case 0: cmp = frag1->yMin - frag2->yMin; break;
        case 1: cmp = frag2->xMax - frag1->xMax; break;
        case 2: cmp = frag2->yMin - frag1->yMin; break;
        case 3: cmp = frag1->xMax - frag2->xMax; break;
        }
        return cmp < 0 ? -1 : cmp > 0 ? 1 : 0;
    }

    // otherwise, compare starting column
    return frag1->col - frag2->col;
}

int CrackleTextLineFrag::cmpXYColumnLineRot(const void *p1, const void *p2) {
    CrackleTextLineFrag *frag1 = (CrackleTextLineFrag *)p1;
    CrackleTextLineFrag *frag2 = (CrackleTextLineFrag *)p2;
    double cmp;

    // if columns overlap, compare y values
    if (frag1->col < frag2->col + (frag2->line->col[frag2->start + frag2->len] -
                                   frag2->line->col[frag2->start]) &&
        frag2->col < frag1->col + (frag1->line->col[frag1->start + frag1->len] -
                                   frag1->line->col[frag1->start])) {
        cmp = 0; // make gcc happy
        switch (frag1->line->rot) {
        case 0: cmp = frag1->yMin - frag2->yMin; break;
        case 1: cmp = frag2->xMax - frag1->xMax; break;
        case 2: cmp = frag2->yMin - frag1->yMin; break;
        case 3: cmp = frag1->xMax - frag2->xMax; break;
        }
        return cmp < 0 ? -1 : cmp > 0 ? 1 : 0;
    }

    // otherwise, compare starting column
    return frag1->col - frag2->col;
}

//------------------------------------------------------------------------
// CrackleTextBlock
//------------------------------------------------------------------------

CrackleTextBlock::CrackleTextBlock(CrackleTextPage *pageA, int rotA) {
    page = pageA;
    rot = rotA;
    xMin = yMin = 0;
    xMax = yMax = -1;
    priMin = 0;
    priMax = page->pageWidth;
    pool = new CrackleTextPool();
    lines = NULL;
    curLine = NULL;
    next = NULL;
    stackNext = NULL;
}

CrackleTextBlock::~CrackleTextBlock() {
    CrackleTextLine *line;

    delete pool;
    while (lines) {
        line = lines;
        lines = lines->next;
        delete line;
    }
}

void CrackleTextBlock::addWord(CrackleTextWord *word) {
    pool->addWord(word);
    if (xMin > xMax) {
        xMin = word->xMin;
        xMax = word->xMax;
        yMin = word->yMin;
        yMax = word->yMax;
    } else {
        if (word->xMin < xMin) {
            xMin = word->xMin;
        }
        if (word->xMax > xMax) {
            xMax = word->xMax;
        }
        if (word->yMin < yMin) {
            yMin = word->yMin;
        }
        if (word->yMax > yMax) {
            yMax = word->yMax;
        }
    }
}

void CrackleTextBlock::coalesce(UnicodeMap *uMap, double fixedPitch) {
    CrackleTextWord *word0, *word1, *word2, *bestWord0, *bestWord1, *lastWord;
    CrackleTextLine *line, *line0, *line1;
    int poolMinBaseIdx, startBaseIdx, minBaseIdx, maxBaseIdx;
    int baseIdx, bestWordBaseIdx, idx0, idx1;
    double minBase, maxBase;
    double fontSize, wordSpacing, delta, priDelta, secDelta;
    CrackleTextLine **lineArray;
    bool found, overlap;
    int col1, col2;
    int i, j, k;

    // discard duplicated text (fake boldface, drop shadows)
    for (idx0 = pool->minBaseIdx; idx0 <= pool->maxBaseIdx; ++idx0) {
        word0 = pool->getPool(idx0);
        while (word0) {
            priDelta = dupMaxPriDelta * word0->fontSize;
            secDelta = dupMaxSecDelta * word0->fontSize;
            maxBaseIdx = pool->getBaseIdx(word0->base + secDelta);
            found = false;
            word1 = word2 = NULL; // make gcc happy
            for (idx1 = idx0; idx1 <= maxBaseIdx; ++idx1) {
                if (idx1 == idx0) {
                    word1 = word0;
                    word2 = word0->next;
                } else {
                    word1 = NULL;
                    word2 = pool->getPool(idx1);
                }
                for (; word2; word1 = word2, word2 = word2->next) {
                    if (word2->len == word0->len &&
                        !memcmp(word2->text, word0->text,
                                word0->len * sizeof(Unicode))) {
                        switch (rot) {
                        case 0:
                        case 2:
                            found = fabs(word0->xMin - word2->xMin) < priDelta &&
                                                                      fabs(word0->xMax - word2->xMax) < priDelta &&
                                                                                                        fabs(word0->yMin - word2->yMin) < secDelta &&
                                                                                                                                          fabs(word0->yMax - word2->yMax) < secDelta;
                            break;
                        case 1:
                        case 3:
                            found = fabs(word0->xMin - word2->xMin) < secDelta &&
                                                                      fabs(word0->xMax - word2->xMax) < secDelta &&
                                                                                                        fabs(word0->yMin - word2->yMin) < priDelta &&
                                                                                                                                          fabs(word0->yMax - word2->yMax) < priDelta;
                            break;
                        }
                    }
                    if (found) {
                        break;
                    }
                }
                if (found) {
                    break;
                }
            }
            if (found) {
                if (word1) {
                    word1->next = word2->next;
                } else {
                    pool->setPool(idx1, word2->next);
                }
                delete word2;
            } else {
                word0 = word0->next;
            }
        }
    }

    // build the lines
    curLine = NULL;
    poolMinBaseIdx = pool->minBaseIdx;
    charCount = 0;
    nLines = 0;
    while (1) {

        // find the first non-empty line in the pool
        for (;
             poolMinBaseIdx <= pool->maxBaseIdx && !pool->getPool(poolMinBaseIdx);
             ++poolMinBaseIdx) ;
        if (poolMinBaseIdx > pool->maxBaseIdx) {
            break;
        }

        // look for the left-most word in the first four lines of the
        // pool -- this avoids starting with a superscript word
        startBaseIdx = poolMinBaseIdx;
        for (baseIdx = poolMinBaseIdx + 1;
             baseIdx < poolMinBaseIdx + 4 && baseIdx <= pool->maxBaseIdx;
             ++baseIdx) {
            if (!pool->getPool(baseIdx)) {
                continue;
            }
            if (pool->getPool(baseIdx)->primaryCmp(pool->getPool(startBaseIdx))
                < 0) {
                startBaseIdx = baseIdx;
            }
        }

        // create a new line
        word0 = pool->getPool(startBaseIdx);
        pool->setPool(startBaseIdx, word0->next);
        word0->next = NULL;
        line = new CrackleTextLine(this, word0->rot, word0->base);
        line->addWord(word0);
        lastWord = word0;

        // compute the search range
        fontSize = word0->fontSize;
        minBase = word0->base - maxIntraLineDelta * fontSize;
        maxBase = word0->base + maxIntraLineDelta * fontSize;
        minBaseIdx = pool->getBaseIdx(minBase);
        maxBaseIdx = pool->getBaseIdx(maxBase);
        wordSpacing = fixedPitch ? fixedPitch : maxWordSpacing * fontSize;

        // find the rest of the words in this line
        while (1) {

            // find the left-most word whose baseline is in the range for
            // this line
            bestWordBaseIdx = 0;
            bestWord0 = bestWord1 = NULL;
            overlap = false;
            for (baseIdx = minBaseIdx;
                 !overlap && baseIdx <= maxBaseIdx;
                 ++baseIdx) {
                for (word0 = NULL, word1 = pool->getPool(baseIdx);
                     word1;
                     word0 = word1, word1 = word1->next) {
                    if (word1->base >= minBase &&
                        word1->base <= maxBase) {
                        delta = lastWord->primaryDelta(word1);
                        if (delta < minCharSpacing * fontSize) {
                            overlap = true;
                            break;
                        } else {
                            if (delta < wordSpacing &&
                                (!bestWord1 || word1->primaryCmp(bestWord1) < 0)) {
                                bestWordBaseIdx = baseIdx;
                                bestWord0 = word0;
                                bestWord1 = word1;
                            }
                            break;
                        }
                    }
                }
            }
            if (overlap || !bestWord1) {
                break;
            }

            // remove it from the pool, and add it to the line
            if (bestWord0) {
                bestWord0->next = bestWord1->next;
            } else {
                pool->setPool(bestWordBaseIdx, bestWord1->next);
            }
            bestWord1->next = NULL;
            line->addWord(bestWord1);
            lastWord = bestWord1;
        }

        // add the line
        if (curLine && line->cmpYX(curLine) > 0) {
            line0 = curLine;
            line1 = curLine->next;
        } else {
            line0 = NULL;
            line1 = lines;
        }
        for (;
             line1 && line->cmpYX(line1) > 0;
             line0 = line1, line1 = line1->next) ;
        if (line0) {
            line0->next = line;
        } else {
            lines = line;
        }
        line->next = line1;
        curLine = line;
        line->coalesce(uMap);
        charCount += line->len;
        ++nLines;
    }

    // sort lines into xy order for column assignment
    lineArray = (CrackleTextLine **)gmallocn(nLines, sizeof(CrackleTextLine *));
    for (line = lines, i = 0; line; line = line->next, ++i) {
        lineArray[i] = line;
    }
    qsort(lineArray, nLines, sizeof(CrackleTextLine *), &CrackleTextLine::cmpXY);

    // column assignment
    nColumns = 0;
    if (fixedPitch) {
        for (i = 0; i < nLines; ++i) {
            line0 = lineArray[i];
            col1 = 0; // make gcc happy
            switch (rot) {
            case 0:
                col1 = (int)((line0->xMin - xMin) / fixedPitch + 0.5);
                break;
            case 1:
                col1 = (int)((line0->yMin - yMin) / fixedPitch + 0.5);
                break;
            case 2:
                col1 = (int)((xMax - line0->xMax) / fixedPitch + 0.5);
                break;
            case 3:
                col1 = (int)((yMax - line0->yMax) / fixedPitch + 0.5);
                break;
            }
            for (k = 0; k <= line0->len; ++k) {
                line0->col[k] += col1;
            }
            if (line0->col[line0->len] > nColumns) {
                nColumns = line0->col[line0->len];
            }
        }
    } else {
        for (i = 0; i < nLines; ++i) {
            line0 = lineArray[i];
            col1 = 0;
            for (j = 0; j < i; ++j) {
                line1 = lineArray[j];
                if (line1->primaryDelta(line0) >= 0) {
                    col2 = line1->col[line1->len] + 1;
                } else {
                    k = 0; // make gcc happy
                    switch (rot) {
                    case 0:
                        for (k = 0;
                             k < line1->len &&
                                 line0->xMin >= 0.5 * (line1->edge[k] + line1->edge[k+1]);
                             ++k) ;
                        break;
                    case 1:
                        for (k = 0;
                             k < line1->len &&
                                 line0->yMin >= 0.5 * (line1->edge[k] + line1->edge[k+1]);
                             ++k) ;
                        break;
                    case 2:
                        for (k = 0;
                             k < line1->len &&
                                 line0->xMax <= 0.5 * (line1->edge[k] + line1->edge[k+1]);
                             ++k) ;
                        break;
                    case 3:
                        for (k = 0;
                             k < line1->len &&
                                 line0->yMax <= 0.5 * (line1->edge[k] + line1->edge[k+1]);
                             ++k) ;
                        break;
                    }
                    col2 = line1->col[k];
                }
                if (col2 > col1) {
                    col1 = col2;
                }
            }
            for (k = 0; k <= line0->len; ++k) {
                line0->col[k] += col1;
            }
            if (line0->col[line0->len] > nColumns) {
                nColumns = line0->col[line0->len];
            }
        }
    }
    gfree(lineArray);
}

void CrackleTextBlock::updatePriMinMax(CrackleTextBlock *blk) {
    double newPriMin, newPriMax;
    bool gotPriMin, gotPriMax;

    gotPriMin = gotPriMax = false;
    newPriMin = newPriMax = 0; // make gcc happy
    switch (page->primaryRot) {
    case 0:
    case 2:
        if (blk->yMin < yMax && blk->yMax > yMin) {
            if (blk->xMin < xMin) {
                newPriMin = blk->xMax;
                gotPriMin = true;
            }
            if (blk->xMax > xMax) {
                newPriMax = blk->xMin;
                gotPriMax = true;
            }
        }
        break;
    case 1:
    case 3:
        if (blk->xMin < xMax && blk->xMax > xMin) {
            if (blk->yMin < yMin) {
                newPriMin = blk->yMax;
                gotPriMin = true;
            }
            if (blk->yMax > yMax) {
                newPriMax = blk->yMin;
                gotPriMax = true;
            }
        }
        break;
    }
    if (gotPriMin) {
        if (newPriMin > xMin) {
            newPriMin = xMin;
        }
        if (newPriMin > priMin) {
            priMin = newPriMin;
        }
    }
    if (gotPriMax) {
        if (newPriMax < xMax) {
            newPriMax = xMax;
        }
        if (newPriMax < priMax) {
            priMax = newPriMax;
        }
    }
}

int CrackleTextBlock::cmpXYPrimaryRot(const void *p1, const void *p2) {
    CrackleTextBlock *blk1 = *(CrackleTextBlock **)p1;
    CrackleTextBlock *blk2 = *(CrackleTextBlock **)p2;
    double cmp;

    cmp = 0; // make gcc happy
    switch (blk1->page->primaryRot) {
    case 0:
        if ((cmp = blk1->xMin - blk2->xMin) == 0) {
            cmp = blk1->yMin - blk2->yMin;
        }
        break;
    case 1:
        if ((cmp = blk1->yMin - blk2->yMin) == 0) {
            cmp = blk2->xMax - blk1->xMax;
        }
        break;
    case 2:
        if ((cmp = blk2->xMax - blk1->xMax) == 0) {
            cmp = blk2->yMin - blk1->yMin;
        }
        break;
    case 3:
        if ((cmp = blk2->yMax - blk1->yMax) == 0) {
            cmp = blk1->xMax - blk2->xMax;
        }
        break;
    }
    return cmp < 0 ? -1 : cmp > 0 ? 1 : 0;
}

int CrackleTextBlock::cmpYXPrimaryRot(const void *p1, const void *p2) {
    CrackleTextBlock *blk1 = *(CrackleTextBlock **)p1;
    CrackleTextBlock *blk2 = *(CrackleTextBlock **)p2;
    double cmp;

    cmp = 0; // make gcc happy
    switch (blk1->page->primaryRot) {
    case 0:
        if ((cmp = blk1->yMin - blk2->yMin) == 0) {
            cmp = blk1->xMin - blk2->xMin;
        }
        break;
    case 1:
        if ((cmp = blk2->xMax - blk1->xMax) == 0) {
            cmp = blk1->yMin - blk2->yMin;
        }
        break;
    case 2:
        if ((cmp = blk2->yMin - blk1->yMin) == 0) {
            cmp = blk2->xMax - blk1->xMax;
        }
        break;
    case 3:
        if ((cmp = blk1->xMax - blk2->xMax) == 0) {
            cmp = blk2->yMax - blk1->yMax;
        }
        break;
    }
    return cmp < 0 ? -1 : cmp > 0 ? 1 : 0;
}

int CrackleTextBlock::primaryCmp(CrackleTextBlock *blk) {
    double cmp;

    cmp = 0; // make gcc happy
    switch (rot) {
    case 0:
        cmp = xMin - blk->xMin;
        break;
    case 1:
        cmp = yMin - blk->yMin;
        break;
    case 2:
        cmp = blk->xMax - xMax;
        break;
    case 3:
        cmp = blk->yMax - yMax;
        break;
    }
    return cmp < 0 ? -1 : cmp > 0 ? 1 : 0;
}

double CrackleTextBlock::secondaryDelta(CrackleTextBlock *blk) {
    double delta;

    delta = 0; // make gcc happy
    switch (rot) {
    case 0:
        delta = blk->yMin - yMax;
        break;
    case 1:
        delta = xMin - blk->xMax;
        break;
    case 2:
        delta = yMin - blk->yMax;
        break;
    case 3:
        delta = blk->xMin - xMax;
        break;
    }
    return delta;
}

bool CrackleTextBlock::isBelow(CrackleTextBlock *blk) {
    bool below;

    below = false; // make gcc happy
    switch (page->primaryRot) {
    case 0:
        below = xMin >= blk->priMin && xMax <= blk->priMax &&
            yMin > blk->yMin;
        break;
    case 1:
        below = yMin >= blk->priMin && yMax <= blk->priMax &&
            xMax < blk->xMax;
        break;
    case 2:
        below = xMin >= blk->priMin && xMax <= blk->priMax &&
            yMax < blk->yMax;
        break;
    case 3:
        below = yMin >= blk->priMin && yMax <= blk->priMax &&
            xMin > blk->xMin;
        break;
    }

    return below;
}

//------------------------------------------------------------------------
// CrackleTextFlow
//------------------------------------------------------------------------

CrackleTextFlow::CrackleTextFlow(CrackleTextPage *pageA, CrackleTextBlock *blk) {
    page = pageA;
    xMin = blk->xMin;
    xMax = blk->xMax;
    yMin = blk->yMin;
    yMax = blk->yMax;
    priMin = blk->priMin;
    priMax = blk->priMax;
    blocks = lastBlk = blk;
    next = NULL;
}

CrackleTextFlow::~CrackleTextFlow() {
    CrackleTextBlock *blk;

    while (blocks) {
        blk = blocks;
        blocks = blocks->next;
        delete blk;
    }
}

void CrackleTextFlow::addBlock(CrackleTextBlock *blk) {
    if (lastBlk) {
        lastBlk->next = blk;
    } else {
        blocks = blk;
    }
    lastBlk = blk;
    if (blk->xMin < xMin) {
        xMin = blk->xMin;
    }
    if (blk->xMax > xMax) {
        xMax = blk->xMax;
    }
    if (blk->yMin < yMin) {
        yMin = blk->yMin;
    }
    if (blk->yMax > yMax) {
        yMax = blk->yMax;
    }
}

bool CrackleTextFlow::blockFits(CrackleTextBlock *blk, CrackleTextBlock *prevBlk) {
    bool fits;
    // lower blocks must use similar sized font - James mod
    if (fabs(blk->lines->words->fontSize - lastBlk->lines->words->fontSize) >
        maxBlockFontSizeDelta1) {
        return false;
    }

    fits = false; // make gcc happy
    switch (page->primaryRot) {
    case 0:
        fits = blk->xMin >= priMin && blk->xMax <= priMax;
        break;
    case 1:
        fits = blk->yMin >= priMin && blk->yMax <= priMax;
        break;
    case 2:
        fits = blk->xMin >= priMin && blk->xMax <= priMax;
        break;
    case 3:
        fits = blk->yMin >= priMin && blk->yMax <= priMax;
        break;
    }
    return fits;
}

#if TEXTOUT_WORD_LIST

//------------------------------------------------------------------------
// CrackleTextWordList
//------------------------------------------------------------------------

CrackleTextWordList::CrackleTextWordList(CrackleTextPage *text, bool physLayout) {
    CrackleTextFlow *flow;
    CrackleTextBlock *blk;
    CrackleTextLine *line;
    CrackleTextWord *word;
    CrackleTextWord **wordArray;
    int nWords, i;

    words = new GList();

    if (text->rawOrder) {
        for (word = text->rawWords; word; word = word->next) {
#if POPPLER_CHECK_VERSION(0, 70, 0)
            words->push_back(word);
#else
            words->append(word);
#endif
        }

    } else if (physLayout) {
        // this is inefficient, but it's also the least useful of these
        // three cases
        nWords = 0;
        for (flow = text->flows; flow; flow = flow->next) {
            for (blk = flow->blocks; blk; blk = blk->next) {
                for (line = blk->lines; line; line = line->next) {
                    for (word = line->words; word; word = word->next) {
                        ++nWords;
                    }
                }
            }
        }
        wordArray = (CrackleTextWord **)gmallocn(nWords, sizeof(CrackleTextWord *));
        i = 0;
        for (flow = text->flows; flow; flow = flow->next) {
            for (blk = flow->blocks; blk; blk = blk->next) {
                for (line = blk->lines; line; line = line->next) {
                    for (word = line->words; word; word = word->next) {
                        wordArray[i++] = word;
                    }
                }
            }
        }
        qsort(wordArray, nWords, sizeof(CrackleTextWord *), &CrackleTextWord::cmpYX);
        for (i = 0; i < nWords; ++i) {
#if POPPLER_CHECK_VERSION(0, 70, 0)
            words->push_back(wordArray[i]);
#else
            words->append(wordArray[i]);
#endif
        }
        gfree(wordArray);

    } else {
        for (flow = text->flows; flow; flow = flow->next) {
            for (blk = flow->blocks; blk; blk = blk->next) {
                for (line = blk->lines; line; line = line->next) {
                    for (word = line->words; word; word = word->next) {
#if POPPLER_CHECK_VERSION(0, 70, 0)
                        words->push_back(word);
#else
                        words->append(word);
#endif
                    }
                }
            }
        }
    }
}

CrackleTextWordList::~CrackleTextWordList() {
    delete words;
}

int CrackleTextWordList::getLength() {
    return words->getLength();
}

CrackleTextWord *CrackleTextWordList::get(int idx) {
    if (idx < 0 || idx >= words->getLength()) {
        return NULL;
    }
    return (CrackleTextWord *)words->get(idx);
}

#endif // TEXTOUT_WORD_LIST

//------------------------------------------------------------------------
// CrackleTextPage
//------------------------------------------------------------------------

CrackleTextPage::CrackleTextPage(bool rawOrderA) {
    int rot;

    rawOrder = rawOrderA;
    curWord = NULL;
    charPos = 0;
    curFont = NULL;
    curFontSize = 0;
    nest = 0;
    nTinyChars = 0;
    lastCharOverlap = false;
    actualText = NULL;
    actualTextLen = 0;
    actualTextNBytes = 0;
    if (!rawOrder) {
        for (rot = 0; rot < 4; ++rot) {
            pools[rot] = new CrackleTextPool();
        }
    }
    flows = NULL;
    blocks = NULL;
    rawWords = NULL;
    rawLastWord = NULL;
    fonts = new GList();
    lastFindXMin = lastFindYMin = 0;
    haveLastFind = false;
    underlines = new GList();
    links = new GList();
}

CrackleTextPage::~CrackleTextPage() {
    int rot;

    clear();
    if (!rawOrder) {
        for (rot = 0; rot < 4; ++rot) {
            delete pools[rot];
        }
    }
    delete fonts; // just delete list, do not delete the objects themselves
#if POPPLER_CHECK_VERSION(0, 70, 0)
    deleteGList<TextUnderline>(underlines);
    deleteGList<TextLink>(links);
#else
    deleteGList(underlines, TextUnderline);
    deleteGList(links, TextLink);
#endif
}

void CrackleTextPage::startPage(GfxState *state) {
    clear();
    if (state) {
        pageWidth = state->getPageWidth();
        pageHeight = state->getPageHeight();
    } else {
        pageWidth = pageHeight = 0;
    }
}

void CrackleTextPage::endPage() {
    if (curWord) {
        endWord();
    }
}

void CrackleTextPage::clear() {
    int rot;
    CrackleTextFlow *flow;
    CrackleTextWord *word;

    if (curWord) {
        delete curWord;
        curWord = NULL;
    }
    gfree(actualText);
    if (rawOrder) {
        while (rawWords) {
            word = rawWords;
            rawWords = rawWords->next;
            delete word;
        }
    } else {
        for (rot = 0; rot < 4; ++rot) {
            delete pools[rot];
        }
        while (flows) {
            flow = flows;
            flows = flows->next;
            delete flow;
        }
        gfree(blocks);
    }
#if POPPLER_CHECK_VERSION(0, 70, 0)
    deleteGList<CrackleTextFontInfo>(fonts);
    deleteGList<TextUnderline>(underlines);
    deleteGList<TextLink>(links);
#else
    deleteGList(fonts, CrackleTextFontInfo);
    deleteGList(underlines, TextUnderline);
    deleteGList(links, TextLink);
#endif

    curWord = NULL;
    charPos = 0;
    curFont = NULL;
    curFontSize = 0;
    nest = 0;
    nTinyChars = 0;
    actualText = NULL;
    actualTextLen = 0;
    actualTextNBytes = 0;
    if (!rawOrder) {
        for (rot = 0; rot < 4; ++rot) {
            pools[rot] = new CrackleTextPool();
        }
    }
    flows = NULL;
    blocks = NULL;
    rawWords = NULL;
    rawLastWord = NULL;
    fonts = new GList();
    underlines = new GList();
    links = new GList();
}

void CrackleTextPage::updateFont(GfxState *state) {
    GfxFont *gfxFont;
    const double *fm;
    const char *name;
    int code, mCode, letterCode, anyCode;
    double w;
    int i;

    // get the font info object
    curFont = NULL;
    for (i = 0; i < fonts->getLength(); ++i) {
        curFont = (CrackleTextFontInfo *)fonts->get(i);
        if (curFont->matches(state)) {
            break;
        }
        curFont = NULL;
    }
    if (!curFont) {
        curFont = new CrackleTextFontInfo(state);
#if POPPLER_CHECK_VERSION(0, 70, 0)
        fonts->push_back(curFont);
#else
        fonts->append(curFont);
#endif

        if (state->getFont()) {
            // Store new font info for crackle use here since
            // embedded pdf documents will be partially freed
            // by the end of the page
            Crackle::PDFFont font(state->getFont());
            string nm=font.name();
            if(!nm.empty()) {
                _font_collection.insert(std::make_pair(nm, font));
            }
        }
    }

    // adjust the font size
    gfxFont = state->getFont();
    curFontSize = state->getTransformedFontSize();
    if (gfxFont && gfxFont->getType() == fontType3) {
        // This is a hack which makes it possible to deal with some Type 3
        // fonts.  The problem is that it's impossible to know what the
        // base coordinate system used in the font is without actually
        // rendering the font.  This code tries to guess by looking at the
        // width of the character 'm' (which breaks if the font is a
        // subset that doesn't contain 'm').
        mCode = letterCode = anyCode = -1;
        for (code = 0; code < 256; ++code) {
            name = ((Gfx8BitFont *)gfxFont)->getCharName(code);
            if (name && name[0] == 'm' && name[1] == '\0') {
                mCode = code;
            }
            if (letterCode < 0 && name && name[1] == '\0' &&
                ((name[0] >= 'A' && name[0] <= 'Z') ||
                 (name[0] >= 'a' && name[0] <= 'z'))) {
                letterCode = code;
            }
            if (anyCode < 0 && name &&
                ((Gfx8BitFont *)gfxFont)->getWidth(code) > 0) {
                anyCode = code;
            }
        }
        if (mCode >= 0 &&
            (w = ((Gfx8BitFont *)gfxFont)->getWidth(mCode)) > 0) {
            // 0.6 is a generic average 'm' width -- yes, this is a hack
            curFontSize *= w / 0.6;
        } else if (letterCode >= 0 &&
                   (w = ((Gfx8BitFont *)gfxFont)->getWidth(letterCode)) > 0) {
            // even more of a hack: 0.5 is a generic letter width
            curFontSize *= w / 0.5;
        } else if (anyCode >= 0 &&
                   (w = ((Gfx8BitFont *)gfxFont)->getWidth(anyCode)) > 0) {
            // better than nothing: 0.5 is a generic character width
            curFontSize *= w / 0.5;
        }
        fm = gfxFont->getFontMatrix();
        if (fm[0] != 0) {
            curFontSize *= fabs(fm[3] / fm[0]);
        }
    }
}

void CrackleTextPage::beginWord(GfxState *state, double x0, double y0) {
    const double *fontm;
    double m[4], m2[4];
    int rot;

    // This check is needed because Type 3 characters can contain
    // text-drawing operations (when CrackleTextPage is being used via
    // {X,Win}SplashOutputDev rather than CrackleTextOutputDev).
    if (curWord) {
        ++nest;
        return;
    }

    // compute the rotation
    state->getFontTransMat(&m[0], &m[1], &m[2], &m[3]);
    if (state->getFont()->getType() == fontType3) {
        fontm = state->getFont()->getFontMatrix();
        m2[0] = fontm[0] * m[0] + fontm[1] * m[2];
        m2[1] = fontm[0] * m[1] + fontm[1] * m[3];
        m2[2] = fontm[2] * m[0] + fontm[3] * m[2];
        m2[3] = fontm[2] * m[1] + fontm[3] * m[3];
        m[0] = m2[0];
        m[1] = m2[1];
        m[2] = m2[2];
        m[3] = m2[3];
    }
    if (fabs(m[0] * m[3]) > fabs(m[1] * m[2])) {
        rot = (m[0] > 0 || m[3] < 0) ? 0 : 2;
    } else {
        rot = (m[2] > 0) ? 1 : 3;
    }

    // for vertical writing mode, the lines are effectively rotated 90
    // degrees
    if (state->getFont()->getWMode()) {
        rot = (rot + 1) & 3;
    }

    curWord = new CrackleTextWord(state, rot, x0, y0, curFont, curFontSize);
}

void CrackleTextPage::addChar(GfxState *state, double x, double y,
                              double dx, double dy,
                              CharCode c, int nBytes, Unicode *u, int uLen) {
    double x1, y1, w1, h1, dx2, dy2, base, sp, delta;
    bool overlap;
    int i;

    // if we're in an ActualText span, save the position info (the
    // ActualText chars will be added by TextPage::endActualText()).
    if (actualText) {
        if (!actualTextNBytes) {
            actualTextX0 = x;
            actualTextY0 = y;
        }
        actualTextX1 = x + dx;
        actualTextY1 = y + dy;
        actualTextNBytes += nBytes;
        return;
    }

    // subtract char and word spacing from the dx,dy values
    sp = state->getCharSpace();
    if (c == (CharCode)0x20) {
        sp += state->getWordSpace();
    }
    state->textTransformDelta(sp * state->getHorizScaling(), 0, &dx2, &dy2);
    dx -= dx2;
    dy -= dy2;
    state->transformDelta(dx, dy, &w1, &h1);

    // throw away chars that aren't inside the page bounds
    // (and also do a sanity check on the character size)
    state->transform(x, y, &x1, &y1);
    if (x1 + w1 < 0 || x1 > pageWidth ||
        y1 + h1 < 0 || y1 > pageHeight ||
        w1 > pageWidth || h1 > pageHeight) {
        charPos += nBytes;
        return;
    }

    // check the tiny chars limit
    if (fabs(w1) < 3 && fabs(h1) < 3) {
        if (++nTinyChars > 50000) {
            charPos += nBytes;
            return;
        }
    }

    // break words at space character
    if (uLen == 1 && u[0] == (Unicode)0x20) {
        charPos += nBytes;
        endWord();
        return;
    }

    Unicode combining_character=0; // holds composition characters when encountering fake accents

    // start a new word if:
    // (1) this character doesn't fall in the right place relative to
    //     the end of the previous word (this places upper and lower
    //     constraints on the position deltas along both the primary
    //     and secondary axes), or
    // (2) this character overlaps the previous one (duplicated text), or
    // (3) the previous character was an overlap (we want each duplicated
    //     character to be in a word by itself at this stage),
    // (4) the font or font size has changed
    if (curWord && curWord->len > 0) {
        base = sp = delta = 0; // make gcc happy
        switch (curWord->rot) {
        case 0:
            base = y1;
            sp = x1 - curWord->xMax;
            delta = x1 - curWord->edge[curWord->len - 1];
            break;
        case 1:
            base = x1;
            sp = y1 - curWord->yMax;
            delta = y1 - curWord->edge[curWord->len - 1];
            break;
        case 2:
            base = y1;
            sp = curWord->xMin - x1;
            delta = curWord->edge[curWord->len - 1] - x1;
            break;
        case 3:
            base = x1;
            sp = curWord->yMin - y1;
            delta = curWord->edge[curWord->len - 1] - y1;
            break;
        }
        overlap = fabs(delta) < dupMaxPriDelta * curWord->fontSize &&
            fabs(base - curWord->base) < dupMaxSecDelta * curWord->fontSize;

        if(overlap) {
            // mostly for the benefit of latex, convert floating accents into combining accents
            Unicode replace(curWord->lastChar());

            if(replace==0x60) { // GRAVE
                combining_character=0x0300;
            } else if(replace==0x27 || replace==0xb4) { // ACUTE
                combining_character=0x0301;
            } else if(replace=='^') { // CIRCUMFLEX
                combining_character=0x0302;
            } else if(replace=='~') { // TILDE
                combining_character=0x0303;
            } else if(replace==0xa8) { // DIAERESIS/UMLAUT
                combining_character=0x0308;
            } else {
                overlap=0;
                curWord->popChar();
                combining_character=0; // not currently supported as accent
            }

            // remove floating accent
            if(combining_character) {
                curWord->popChar();
            }
        }

        // if we didn't substitute an accent, continue old behaviour of splitting word
        if (!combining_character && (overlap || lastCharOverlap ||
                                     sp < -minDupBreakOverlap * curWord->fontSize ||
                                     sp > minWordBreakSpace * curWord->fontSize ||
                                     fabs(base - curWord->base) > 0.5 ||
                                     curFont != curWord->font ||
                                     curFontSize != curWord->fontSize)) {
            endWord();
        }
        lastCharOverlap = overlap;
    } else {
        lastCharOverlap = false;
    }

    if (uLen != 0) {
        // start a new word if needed
        if (!curWord) {
            beginWord(state, x, y);
        }

        // page rotation and/or transform matrices can cause text to be
        // drawn in reverse order -- in this case, swap the begin/end
        // coordinates and break text into individual chars
        if ((curWord->rot == 0 && w1 < 0) ||
            (curWord->rot == 1 && h1 < 0) ||
            (curWord->rot == 2 && w1 > 0) ||
            (curWord->rot == 3 && h1 > 0)) {
            endWord();
            beginWord(state, x + dx, y + dy);
            x1 += w1;
            y1 += h1;
            w1 = -w1;
            h1 = -h1;
        }

        // add the characters to the current word
        w1 /= uLen;
        h1 /= uLen;
        for (i = 0; i < uLen; ++i) {

            if (u[i] >= 0xd800 && u[i] < 0xdc00) { /* surrogate pair */
                if (i + 1 < uLen && u[i+1] >= 0xdc00 && u[i+1] < 0xe000) {
                    /* next code is a low surrogate */
                    Unicode uu = (((u[i] & 0x3ff) << 10) | (u[i+1] & 0x3ff)) + 0x10000;
                    i++;
                    curWord->addChar(state, x1 + i*w1, y1 + i*h1, w1, h1, charPos, nBytes, uu);
                } else {
                    /* missing low surrogate
                       replace it with REPLACEMENT CHARACTER (U+FFFD) */
                    curWord->addChar(state, x1 + i*w1, y1 + i*h1, w1, h1, charPos, nBytes, 0xfffd);
                }
            } else if (u[i] >= 0xdc00 && u[i] < 0xe000) {
                /* invalid low surrogate
                   replace it with REPLACEMENT CHARACTER (U+FFFD) */
                curWord->addChar(state, x1 + i*w1, y1 + i*h1, w1, h1, charPos, nBytes, 0xfffd);
            } else if(combining_character) {
                /* fake accent - add combining version */
                try {
                    string charstring;
                    string composed_charstring;
                    append(u[i], back_inserter(charstring));
                    append(combining_character, back_inserter(charstring));
                    normalize_utf8(charstring.begin(), charstring.end(),
                                   back_inserter(composed_charstring), NFC);
                    std::vector<utf8::uint32_t> utf32char;
                    utf8to32(composed_charstring.begin(), composed_charstring.end(),
                             back_inserter(utf32char));

                    curWord->addChar(state, x1 + i*w1, y1 + i*h1, w1, h1, charPos, nBytes, utf32char[0]);

                } catch (utf8::exception e) {
                    // skip invalid char
                }
            } else {
                curWord->addChar(state, x1 + i*w1, y1 + i*h1, w1, h1, charPos, nBytes, u[i]);
            }

        }
    }
    charPos += nBytes;
}

void CrackleTextPage::incCharCount(int nChars) {
    charPos += nChars;
}

void CrackleTextPage::beginActualText(GfxState *state, Unicode *u, int uLen) {
    if (actualText) {
        gfree(actualText);
    }
    actualText = (Unicode *)gmallocn(uLen, sizeof(Unicode));
    memcpy(actualText, u, uLen * sizeof(Unicode));
    actualTextLen = uLen;
    actualTextNBytes = 0;
}

void CrackleTextPage::endActualText(GfxState *state) {
    Unicode *u;

    u = actualText;
    actualText = NULL;  // so we can call TextPage::addChar()
    if (actualTextNBytes) {
        // now that we have the position info for all of the text inside
        // the marked content span, we feed the "ActualText" back through
        // addChar()
        addChar(state, actualTextX0, actualTextY0,
                actualTextX1 - actualTextX0, actualTextY1 - actualTextY0,
                0, actualTextNBytes, u, actualTextLen);
    }
    gfree(u);
    actualText = NULL;
    actualTextLen = 0;
    actualTextNBytes = false;
}

void CrackleTextPage::endWord() {
    // This check is needed because Type 3 characters can contain
    // text-drawing operations (when CrackleTextPage is being used via
    // {X,Win}SplashOutputDev rather than CrackleTextOutputDev).
    if (nest > 0) {
        --nest;
        return;
    }

    if (curWord) {
        addWord(curWord);
        curWord = NULL;
    }
}

void CrackleTextPage::addWord(CrackleTextWord *word) {
    // throw away zero-length words -- they don't have valid xMin/xMax
    // values, and they're useless anyway
    if (word->len == 0) {
        delete word;
        return;
    }

    if (rawOrder) {
        if (rawLastWord) {
            rawLastWord->next = word;
        } else {
            rawWords = word;
        }
        rawLastWord = word;
    } else {
        pools[word->rot]->addWord(word);
    }
}

void CrackleTextPage::addUnderline(double x0, double y0, double x1, double y1) {
#if POPPLER_CHECK_VERSION(0, 70, 0)
    underlines->push_back(new TextUnderline(x0, y0, x1, y1));
#else
    underlines->append(new TextUnderline(x0, y0, x1, y1));
#endif
}

void CrackleTextPage::addLink(int xMin, int yMin, int xMax, int yMax, Link *link) {
#if POPPLER_CHECK_VERSION(0, 70, 0)
    links->push_back(new TextLink(xMin, yMin, xMax, yMax, link));
#else
    links->append(new TextLink(xMin, yMin, xMax, yMax, link));
#endif
}

void CrackleTextPage::coalesce(bool physLayout, double fixedPitch, bool doHTML) {
    UnicodeMap *uMap;
    CrackleTextPool *pool;
    CrackleTextWord *word0, *word1, *word2;
    CrackleTextLine *line;
    CrackleTextBlock *blkList, *blkStack, *blk, *lastBlk, *blk0, *blk1, *tmpblk;
    CrackleTextBlock **blkArray;
    CrackleTextFlow *flow, *lastFlow;
    TextUnderline *underline;
    TextLink *link;
    int rot, poolMinBaseIdx, baseIdx, startBaseIdx, endBaseIdx;
    double minBase, maxBase, newMinBase, newMaxBase;
    double fontSize, colSpace1, colSpace2, lineSpace, intraLineSpace, blkSpace;
    bool found;
    int count[4];
    int lrCount;
    int firstBlkIdx, nBlocksLeft;
    int col1, col2;
    int i, j, n;

    if (rawOrder) {
        primaryRot = 0;
        primaryLR = true;
        return;
    }

    uMap = globalParams->getTextEncoding();
    blkList = NULL;
    lastBlk = NULL;
    nBlocks = 0;
    primaryRot = 0;

#if 0 // for debugging
    printf("*** initial words ***\n");
    for (rot = 0; rot < 4; ++rot) {
        pool = pools[rot];
        for (baseIdx = pool->minBaseIdx; baseIdx <= pool->maxBaseIdx; ++baseIdx) {
            for (word0 = pool->getPool(baseIdx); word0; word0 = word0->next) {
                printf("    word: x=%.2f..%.2f y=%.2f..%.2f base=%.2f fontSize=%.2f rot=%d link=%p '",
                       word0->xMin, word0->xMax, word0->yMin, word0->yMax,
                       word0->base, word0->fontSize, rot*90, word0->link);
                for (i = 0; i < word0->len; ++i) {
                    fputc(word0->text[i] & 0xff, stdout);
                }
                printf("'\n");
            }
        }
    }
    printf("\n");
#endif

#if 0 //~ for debugging
    for (i = 0; i < underlines->getLength(); ++i) {
        underline = (TextUnderline *)underlines->get(i);
        printf("underline: x=%g..%g y=%g..%g horiz=%d\n",
               underline->x0, underline->x1, underline->y0, underline->y1,
               underline->horiz);
    }
#endif

    if (doHTML) {

        //----- handle underlining
        for (i = 0; i < underlines->getLength(); ++i) {
            underline = (TextUnderline *)underlines->get(i);
            if (underline->horiz) {
                // rot = 0
                if (pools[0]->minBaseIdx <= pools[0]->maxBaseIdx) {
                    startBaseIdx = pools[0]->getBaseIdx(underline->y0 + minUnderlineGap);
                    endBaseIdx = pools[0]->getBaseIdx(underline->y0 + maxUnderlineGap);
                    for (j = startBaseIdx; j <= endBaseIdx; ++j) {
                        for (word0 = pools[0]->getPool(j); word0; word0 = word0->next) {
                            //~ need to check the y value against the word baseline
                            if (underline->x0 < word0->xMin + underlineSlack &&
                                word0->xMax - underlineSlack < underline->x1) {
                                word0->underlined = true;
                            }
                        }
                    }
                }

                // rot = 2
                if (pools[2]->minBaseIdx <= pools[2]->maxBaseIdx) {
                    startBaseIdx = pools[2]->getBaseIdx(underline->y0 - maxUnderlineGap);
                    endBaseIdx = pools[2]->getBaseIdx(underline->y0 - minUnderlineGap);
                    for (j = startBaseIdx; j <= endBaseIdx; ++j) {
                        for (word0 = pools[2]->getPool(j); word0; word0 = word0->next) {
                            if (underline->x0 < word0->xMin + underlineSlack &&
                                word0->xMax - underlineSlack < underline->x1) {
                                word0->underlined = true;
                            }
                        }
                    }
                }
            } else {
                // rot = 1
                if (pools[1]->minBaseIdx <= pools[1]->maxBaseIdx) {
                    startBaseIdx = pools[1]->getBaseIdx(underline->x0 - maxUnderlineGap);
                    endBaseIdx = pools[1]->getBaseIdx(underline->x0 - minUnderlineGap);
                    for (j = startBaseIdx; j <= endBaseIdx; ++j) {
                        for (word0 = pools[1]->getPool(j); word0; word0 = word0->next) {
                            if (underline->y0 < word0->yMin + underlineSlack &&
                                word0->yMax - underlineSlack < underline->y1) {
                                word0->underlined = true;
                            }
                        }
                    }
                }

                // rot = 3
                if (pools[3]->minBaseIdx <= pools[3]->maxBaseIdx) {
                    startBaseIdx = pools[3]->getBaseIdx(underline->x0 + minUnderlineGap);
                    endBaseIdx = pools[3]->getBaseIdx(underline->x0 + maxUnderlineGap);
                    for (j = startBaseIdx; j <= endBaseIdx; ++j) {
                        for (word0 = pools[3]->getPool(j); word0; word0 = word0->next) {
                            if (underline->y0 < word0->yMin + underlineSlack &&
                                word0->yMax - underlineSlack < underline->y1) {
                                word0->underlined = true;
                            }
                        }
                    }
                }
            }
        }

        //----- handle links
        for (i = 0; i < links->getLength(); ++i) {
            link = (TextLink *)links->get(i);

            // rot = 0
            if (pools[0]->minBaseIdx <= pools[0]->maxBaseIdx) {
                startBaseIdx = pools[0]->getBaseIdx(link->yMin);
                endBaseIdx = pools[0]->getBaseIdx(link->yMax);
                for (j = startBaseIdx; j <= endBaseIdx; ++j) {
                    for (word0 = pools[0]->getPool(j); word0; word0 = word0->next) {
                        if (link->xMin < word0->xMin + hyperlinkSlack &&
                            word0->xMax - hyperlinkSlack < link->xMax &&
                            link->yMin < word0->yMin + hyperlinkSlack &&
                            word0->yMax - hyperlinkSlack < link->yMax) {
                            word0->link = link->link;
                        }
                    }
                }
            }

            // rot = 2
            if (pools[2]->minBaseIdx <= pools[2]->maxBaseIdx) {
                startBaseIdx = pools[2]->getBaseIdx(link->yMin);
                endBaseIdx = pools[2]->getBaseIdx(link->yMax);
                for (j = startBaseIdx; j <= endBaseIdx; ++j) {
                    for (word0 = pools[2]->getPool(j); word0; word0 = word0->next) {
                        if (link->xMin < word0->xMin + hyperlinkSlack &&
                            word0->xMax - hyperlinkSlack < link->xMax &&
                            link->yMin < word0->yMin + hyperlinkSlack &&
                            word0->yMax - hyperlinkSlack < link->yMax) {
                            word0->link = link->link;
                        }
                    }
                }
            }

            // rot = 1
            if (pools[1]->minBaseIdx <= pools[1]->maxBaseIdx) {
                startBaseIdx = pools[1]->getBaseIdx(link->xMin);
                endBaseIdx = pools[1]->getBaseIdx(link->xMax);
                for (j = startBaseIdx; j <= endBaseIdx; ++j) {
                    for (word0 = pools[1]->getPool(j); word0; word0 = word0->next) {
                        if (link->yMin < word0->yMin + hyperlinkSlack &&
                            word0->yMax - hyperlinkSlack < link->yMax &&
                            link->xMin < word0->xMin + hyperlinkSlack &&
                            word0->xMax - hyperlinkSlack < link->xMax) {
                            word0->link = link->link;
                        }
                    }
                }
            }

            // rot = 3
            if (pools[3]->minBaseIdx <= pools[3]->maxBaseIdx) {
                startBaseIdx = pools[3]->getBaseIdx(link->xMin);
                endBaseIdx = pools[3]->getBaseIdx(link->xMax);
                for (j = startBaseIdx; j <= endBaseIdx; ++j) {
                    for (word0 = pools[3]->getPool(j); word0; word0 = word0->next) {
                        if (link->yMin < word0->yMin + hyperlinkSlack &&
                            word0->yMax - hyperlinkSlack < link->yMax &&
                            link->xMin < word0->xMin + hyperlinkSlack &&
                            word0->xMax - hyperlinkSlack < link->xMax) {
                            word0->link = link->link;
                        }
                    }
                }
            }
        }
    }

    //----- assemble the blocks

    //~ add an outer loop for writing mode (vertical text)

    // build blocks for each rotation value
    for (rot = 0; rot < 4; ++rot) {
        pool = pools[rot];
        poolMinBaseIdx = pool->minBaseIdx;
        count[rot] = 0;

        // add blocks until no more words are left
        while (1) {

            // find the first non-empty line in the pool
            for (;
                 poolMinBaseIdx <= pool->maxBaseIdx &&
                     !pool->getPool(poolMinBaseIdx);
                 ++poolMinBaseIdx) ;
            if (poolMinBaseIdx > pool->maxBaseIdx) {
                break;
            }

            // look for the left-most word in the first four lines of the
            // pool -- this avoids starting with a superscript word
            startBaseIdx = poolMinBaseIdx;
            for (baseIdx = poolMinBaseIdx + 1;
                 baseIdx < poolMinBaseIdx + 4 && baseIdx <= pool->maxBaseIdx;
                 ++baseIdx) {
                if (!pool->getPool(baseIdx)) {
                    continue;
                }
                if (pool->getPool(baseIdx)->primaryCmp(pool->getPool(startBaseIdx))
                    < 0) {
                    startBaseIdx = baseIdx;
                }
            }

            // create a new block
            word0 = pool->getPool(startBaseIdx);
            pool->setPool(startBaseIdx, word0->next);
            word0->next = NULL;
            blk = new CrackleTextBlock(this, rot);
            blk->addWord(word0);

            fontSize = word0->fontSize;
            minBase = maxBase = word0->base;
            colSpace1 = minColSpacing1 * fontSize;
            colSpace2 = minColSpacing2 * fontSize;
            lineSpace = maxLineSpacingDelta * fontSize;
            intraLineSpace = maxIntraLineDelta * fontSize;

            // add words to the block
            do {
                found = false;

                // look for words on the line above the current top edge of
                // the block
                newMinBase = minBase;
                for (baseIdx = pool->getBaseIdx(minBase);
                     baseIdx >= pool->getBaseIdx(minBase - lineSpace);
                     --baseIdx) {
                    word0 = NULL;
                    word1 = pool->getPool(baseIdx);
                    while (word1) {
                        if (word1->base < minBase &&
                            word1->base >= minBase - lineSpace &&
                            ((rot == 0 || rot == 2)
                             ? (word1->xMin < blk->xMax && word1->xMax > blk->xMin)
                             : (word1->yMin < blk->yMax && word1->yMax > blk->yMin)) &&
                            fabs(word1->fontSize - fontSize) <
                            maxBlockFontSizeDelta1 * fontSize) {
                            word2 = word1;
                            if (word0) {
                                word0->next = word1->next;
                            } else {
                                pool->setPool(baseIdx, word1->next);
                            }
                            word1 = word1->next;
                            word2->next = NULL;
                            blk->addWord(word2);
                            found = true;
                            newMinBase = word2->base;
                        } else {
                            word0 = word1;
                            word1 = word1->next;
                        }
                    }
                }
                minBase = newMinBase;

                // look for words on the line below the current bottom edge of
                // the block
                newMaxBase = maxBase;
                for (baseIdx = pool->getBaseIdx(maxBase);
                     baseIdx <= pool->getBaseIdx(maxBase + lineSpace);
                     ++baseIdx) {
                    word0 = NULL;
                    word1 = pool->getPool(baseIdx);
                    while (word1) {
                        if (word1->base > maxBase &&
                            word1->base <= maxBase + lineSpace &&
                            ((rot == 0 || rot == 2)
                             ? (word1->xMin < blk->xMax && word1->xMax > blk->xMin)
                             : (word1->yMin < blk->yMax && word1->yMax > blk->yMin)) &&
                            fabs(word1->fontSize - fontSize) <
                            maxBlockFontSizeDelta1 * fontSize) {
                            word2 = word1;
                            if (word0) {
                                word0->next = word1->next;
                            } else {
                                pool->setPool(baseIdx, word1->next);
                            }
                            word1 = word1->next;
                            word2->next = NULL;
                            blk->addWord(word2);
                            found = true;
                            newMaxBase = word2->base;
                        } else {
                            word0 = word1;
                            word1 = word1->next;
                        }
                    }
                }
                maxBase = newMaxBase;

                // look for words that are on lines already in the block, and
                // that overlap the block horizontally
                for (baseIdx = pool->getBaseIdx(minBase - intraLineSpace);
                     baseIdx <= pool->getBaseIdx(maxBase + intraLineSpace);
                     ++baseIdx) {
                    word0 = NULL;
                    word1 = pool->getPool(baseIdx);
                    while (word1) {
                        if (word1->base >= minBase - intraLineSpace &&
                            word1->base <= maxBase + intraLineSpace &&
                            ((rot == 0 || rot == 2)
                             ? (word1->xMin < blk->xMax + colSpace1 &&
                                word1->xMax > blk->xMin - colSpace1)
                             : (word1->yMin < blk->yMax + colSpace1 &&
                                word1->yMax > blk->yMin - colSpace1)) &&
                            fabs(word1->fontSize - fontSize) <
                            maxBlockFontSizeDelta2 * fontSize) {
                            word2 = word1;
                            if (word0) {
                                word0->next = word1->next;
                            } else {
                                pool->setPool(baseIdx, word1->next);
                            }
                            word1 = word1->next;
                            word2->next = NULL;
                            blk->addWord(word2);
                            found = true;
                        } else {
                            word0 = word1;
                            word1 = word1->next;
                        }
                    }
                }

                // only check for outlying words (the next two chunks of code)
                // if we didn't find anything else
                if (found) {
                    continue;
                }

                // scan down the left side of the block, looking for words
                // that are near (but not overlapping) the block
                n = 0;
                for (baseIdx = pool->getBaseIdx(minBase - intraLineSpace);
                     baseIdx <= pool->getBaseIdx(maxBase + intraLineSpace);
                     ++baseIdx) {
                    word1 = pool->getPool(baseIdx);
                    while (word1) {
                        if (word1->base >= minBase - intraLineSpace &&
                            word1->base <= maxBase + intraLineSpace &&
                            ((rot == 0 || rot == 2)
                             ? (word1->xMax <= blk->xMin &&
                                word1->xMax > blk->xMin - colSpace2)
                             : (word1->yMax <= blk->yMin &&
                                word1->yMax > blk->yMin - colSpace2)) &&
                            fabs(word1->fontSize - fontSize) <
                            maxBlockFontSizeDelta3 * fontSize) {
                            ++n;
                            break;
                        }
                        word1 = word1->next;
                    }
                }
                if (n > 0) {
                    for (baseIdx = pool->getBaseIdx(minBase - intraLineSpace);
                         baseIdx <= pool->getBaseIdx(maxBase + intraLineSpace);
                         ++baseIdx) {
                        word0 = NULL;
                        word1 = pool->getPool(baseIdx);
                        while (word1) {
                            if (word1->base >= minBase - intraLineSpace &&
                                word1->base <= maxBase + intraLineSpace &&
                                ((rot == 0 || rot == 2)
                                 ? (word1->xMax <= blk->xMin &&
                                    word1->xMax > blk->xMin - colSpace2)
                                 : (word1->yMax <= blk->yMin &&
                                    word1->yMax > blk->yMin - colSpace2)) &&
                                fabs(word1->fontSize - fontSize) <
                                maxBlockFontSizeDelta3 * fontSize) {
                                word2 = word1;
                                if (word0) {
                                    word0->next = word1->next;
                                } else {
                                    pool->setPool(baseIdx, word1->next);
                                }
                                word1 = word1->next;
                                word2->next = NULL;
                                blk->addWord(word2);
                                if (word2->base < minBase) {
                                    minBase = word2->base;
                                } else if (word2->base > maxBase) {
                                    maxBase = word2->base;
                                }
                                found = true;
                                break;
                            } else {
                                word0 = word1;
                                word1 = word1->next;
                            }
                        }
                    }
                }

                // scan down the right side of the block, looking for words
                // that are near (but not overlapping) the block
                n = 0;
                for (baseIdx = pool->getBaseIdx(minBase - intraLineSpace);
                     baseIdx <= pool->getBaseIdx(maxBase + intraLineSpace);
                     ++baseIdx) {
                    word1 = pool->getPool(baseIdx);
                    while (word1) {
                        if (word1->base >= minBase - intraLineSpace &&
                            word1->base <= maxBase + intraLineSpace &&
                            ((rot == 0 || rot == 2)
                             ? (word1->xMin >= blk->xMax &&
                                word1->xMin < blk->xMax + colSpace2)
                             : (word1->yMin >= blk->yMax &&
                                word1->yMin < blk->yMax + colSpace2)) &&
                            fabs(word1->fontSize - fontSize) <
                            maxBlockFontSizeDelta3 * fontSize) {
                            ++n;
                            break;
                        }
                        word1 = word1->next;
                    }
                }
                if (n > 0) {
                    for (baseIdx = pool->getBaseIdx(minBase - intraLineSpace);
                         baseIdx <= pool->getBaseIdx(maxBase + intraLineSpace);
                         ++baseIdx) {
                        word0 = NULL;
                        word1 = pool->getPool(baseIdx);
                        while (word1) {
                            if (word1->base >= minBase - intraLineSpace &&
                                word1->base <= maxBase + intraLineSpace &&
                                ((rot == 0 || rot == 2)
                                 ? (word1->xMin >= blk->xMax &&
                                    word1->xMin < blk->xMax + colSpace2)
                                 : (word1->yMin >= blk->yMax &&
                                    word1->yMin < blk->yMax + colSpace2)) &&
                                fabs(word1->fontSize - fontSize) <
                                maxBlockFontSizeDelta3 * fontSize) {
                                word2 = word1;
                                if (word0) {
                                    word0->next = word1->next;
                                } else {
                                    pool->setPool(baseIdx, word1->next);
                                }
                                word1 = word1->next;
                                word2->next = NULL;
                                blk->addWord(word2);
                                if (word2->base < minBase) {
                                    minBase = word2->base;
                                } else if (word2->base > maxBase) {
                                    maxBase = word2->base;
                                }
                                found = true;
                                break;
                            } else {
                                word0 = word1;
                                word1 = word1->next;
                            }
                        }
                    }
                }

            } while (found);

            //~ need to compute the primary writing mode (horiz/vert) in
            //~ addition to primary rotation

            // add block to the list
            if (lastBlk) {
                lastBlk->next = blk;
            } else {
                blkList = blk;
            }
            lastBlk = blk;
        }
    }

    // merge any blocks intersecting others of a similar font size
    // useful for e.g. numbered lists where the single digits lie
    // more than a column width away from the text but double digits don't
    // (such as in some bibliographies)
    for (blk = blkList; blk; blk = blk->next) {
        if (blk->pool->cursor) {
            int fontSize0=blk->pool->cursor->fontSize;

            blk0 = 0;
            blk1 = blkList;
            while(blk1) {
                if (!blk1->pool->cursor) {
                    // not meant to happen - perhaps an empty block?
                    blk1=0;
                } else {

                    int fontSize1=blk1->pool->cursor->fontSize;

                    // check if blocks are eligible to merge
                    if (blk != blk1 &&
                        blk->rot == blk1->rot &&
                        (abs(fontSize1 - fontSize0) < maxBlockFontSizeDelta2) &&
                        ((blk1->xMin >= blk->xMin &&
                          blk1->xMin <= blk->xMax) ||
                         (blk1->xMax >= blk->xMin &&
                          blk1->xMax <= blk->xMax)) &&
                        ((blk1->yMin >= blk->yMin &&
                          blk1->yMin <= blk->yMax) ||
                         (blk1->yMax >= blk->yMin &&
                          blk1->yMax <= blk->yMax))) {

                        // merge blocks
                        for (int idx1 = blk1->pool->minBaseIdx; idx1 <= blk1->pool->maxBaseIdx; ++idx1) {
                            word1 = blk1->pool->getPool(idx1);
                            while(word1) {
                                word0=word1;
                                word1=word1->next;
                                word0->next=0;
                                blk->addWord(word0);
                            }
                            blk1->pool->setPool(idx1, 0);
                        }

                        // remove old block
                        tmpblk= blk1;
                        blk1=blk1->next;
                        delete tmpblk;
                        if(blk0) {
                            blk0->next=blk1;
                        } else {
                            blkList=blk1;
                        }

                    } else {

                        // didn't need to merge
                        blk0 = blk1;
                        blk1 = blk1->next;
                    }
                }
            }
        }
    }

    // coalesce and count the blocks, determining the primary rotation
    for (blk = blkList; blk; blk = blk->next) {
        blk->coalesce(uMap, fixedPitch);
        count[blk->rot] += blk->charCount;
        if (count[blk->rot] > count[primaryRot]) {
            primaryRot = blk->rot;
        }
        ++nBlocks;
    }

#if 0 // for debugging
    printf("*** rotation ***\n");
    for (rot = 0; rot < 4; ++rot) {
        printf("  %d: %6d\n", rot, count[rot]);
    }
    printf("  primary rot = %d\n", primaryRot);
    printf("\n");
#endif

#if 0 // for debugging
    printf("*** blocks ***\n");
    for (blk = blkList; blk; blk = blk->next) {
        printf("block: rot=%d x=%.2f..%.2f y=%.2f..%.2f\n",
               blk->rot, blk->xMin, blk->xMax, blk->yMin, blk->yMax);
        for (line = blk->lines; line; line = line->next) {
            printf("  line: x=%.2f..%.2f y=%.2f..%.2f base=%.2f\n",
                   line->xMin, line->xMax, line->yMin, line->yMax, line->base);
            for (word0 = line->words; word0; word0 = word0->next) {
                printf("    word: x=%.2f..%.2f y=%.2f..%.2f base=%.2f fontSize=%.2f space=%d: '",
                       word0->xMin, word0->xMax, word0->yMin, word0->yMax,
                       word0->base, word0->fontSize, word0->spaceAfter);
                for (i = 0; i < word0->len; ++i) {
                    fputc(word0->text[i] & 0xff, stdout);
                }
                printf("'\n");
            }
        }
    }
    printf("\n");
#endif

    // determine the primary direction
    lrCount = 0;
    for (blk = blkList; blk; blk = blk->next) {
        for (line = blk->lines; line; line = line->next) {
            for (word0 = line->words; word0; word0 = word0->next) {
                for (i = 0; i < word0->len; ++i) {
                    if (unicodeTypeL(word0->text[i])) {
                        ++lrCount;
                    } else if (unicodeTypeR(word0->text[i])) {
                        --lrCount;
                    }
                }
            }
        }
    }
    primaryLR = lrCount >= 0;

#if 0 // for debugging
    printf("*** direction ***\n");
    printf("lrCount = %d\n", lrCount);
    printf("primaryLR = %d\n", primaryLR);
#endif

    //----- column assignment

    if (physLayout && fixedPitch) {

        if (blocks)
            gfree (blocks);
        blocks = (CrackleTextBlock **)gmallocn(nBlocks, sizeof(CrackleTextBlock *));
        for (blk = blkList, i = 0; blk; blk = blk->next, ++i) {
            blocks[i] = blk;
            col1 = 0; // make gcc happy
            switch (primaryRot) {
            case 0:
                col1 = (int)(blk->xMin / fixedPitch + 0.5);
                break;
            case 1:
                col1 = (int)(blk->yMin / fixedPitch + 0.5);
                break;
            case 2:
                col1 = (int)((pageWidth - blk->xMax) / fixedPitch + 0.5);
                break;
            case 3:
                col1 = (int)((pageHeight - blk->yMax) / fixedPitch + 0.5);
                break;
            }
            blk->col = col1;
            for (line = blk->lines; line; line = line->next) {
                for (j = 0; j <= line->len; ++j) {
                    line->col[j] += col1;
                }
            }
        }

    } else {

        // sort blocks into xy order for column assignment
        if (blocks)
            gfree (blocks);
        blocks = (CrackleTextBlock **)gmallocn(nBlocks, sizeof(CrackleTextBlock *));
        for (blk = blkList, i = 0; blk; blk = blk->next, ++i) {
            blocks[i] = blk;
        }
        qsort(blocks, nBlocks, sizeof(CrackleTextBlock *), &CrackleTextBlock::cmpXYPrimaryRot);

        // column assignment
        for (i = 0; i < nBlocks; ++i) {
            blk0 = blocks[i];
            col1 = 0;
            for (j = 0; j < i; ++j) {
                blk1 = blocks[j];
                col2 = 0; // make gcc happy
                switch (primaryRot) {
                case 0:
                    if (blk0->xMin > blk1->xMax) {
                        col2 = blk1->col + blk1->nColumns + 3;
                    } else if (blk1->xMax == blk1->xMin) {
                        col2 = blk1->col;
                    } else {
                        col2 = blk1->col + (int)(((blk0->xMin - blk1->xMin) /
                                                  (blk1->xMax - blk1->xMin)) *
                                                 blk1->nColumns);
                    }
                    break;
                case 1:
                    if (blk0->yMin > blk1->yMax) {
                        col2 = blk1->col + blk1->nColumns + 3;
                    } else if (blk1->yMax == blk1->yMin) {
                        col2 = blk1->col;
                    } else {
                        col2 = blk1->col + (int)(((blk0->yMin - blk1->yMin) /
                                                  (blk1->yMax - blk1->yMin)) *
                                                 blk1->nColumns);
                    }
                    break;
                case 2:
                    if (blk0->xMax < blk1->xMin) {
                        col2 = blk1->col + blk1->nColumns + 3;
                    } else if (blk1->xMin == blk1->xMax) {
                        col2 = blk1->col;
                    } else {
                        col2 = blk1->col + (int)(((blk0->xMax - blk1->xMax) /
                                                  (blk1->xMin - blk1->xMax)) *
                                                 blk1->nColumns);
                    }
                    break;
                case 3:
                    if (blk0->yMax < blk1->yMin) {
                        col2 = blk1->col + blk1->nColumns + 3;
                    } else if (blk1->yMin == blk1->yMax) {
                        col2 = blk1->col;
                    } else {
                        col2 = blk1->col + (int)(((blk0->yMax - blk1->yMax) /
                                                  (blk1->yMin - blk1->yMax)) *
                                                 blk1->nColumns);
                    }
                    break;
                }
                if (col2 > col1) {
                    col1 = col2;
                }
            }
            blk0->col = col1;
            for (line = blk0->lines; line; line = line->next) {
                for (j = 0; j <= line->len; ++j) {
                    line->col[j] += col1;
                }
            }
        }

    }

#if 0 // for debugging
    printf("*** blocks, after column assignment ***\n");
    for (blk = blkList; blk; blk = blk->next) {
        printf("block: rot=%d x=%.2f..%.2f y=%.2f..%.2f col=%d nCols=%d\n",
               blk->rot, blk->xMin, blk->xMax, blk->yMin, blk->yMax, blk->col,
               blk->nColumns);
        for (line = blk->lines; line; line = line->next) {
            printf("  line: col[0]=%d\n", line->col[0]);
            for (word0 = line->words; word0; word0 = word0->next) {
                printf("    word: x=%.2f..%.2f y=%.2f..%.2f base=%.2f fontSize=%.2f space=%d: '",
                       word0->xMin, word0->xMax, word0->yMin, word0->yMax,
                       word0->base, word0->fontSize, word0->spaceAfter);
                for (i = 0; i < word0->len; ++i) {
                    fputc(word0->text[i] & 0xff, stdout);
                }
                printf("'\n");
            }
        }
    }
    printf("\n");
#endif

    //----- reading order sort

    // sort blocks into yx order (in preparation for reading order sort)
    qsort(blocks, nBlocks, sizeof(CrackleTextBlock *), &CrackleTextBlock::cmpYXPrimaryRot);

    // compute space on left and right sides of each block
    for (i = 0; i < nBlocks; ++i) {
        blk0 = blocks[i];
        for (j = 0; j < nBlocks; ++j) {
            blk1 = blocks[j];
            if (blk1 != blk0) {
                blk0->updatePriMinMax(blk1);
            }
        }
    }

#if 0 // for debugging
    printf("*** blocks, after yx sort ***\n");
    for (i = 0; i < nBlocks; ++i) {
        blk = blocks[i];
        printf("block: rot=%d x=%.2f..%.2f y=%.2f..%.2f space=%.2f..%.2f\n",
               blk->rot, blk->xMin, blk->xMax, blk->yMin, blk->yMax,
               blk->priMin, blk->priMax);
        for (line = blk->lines; line; line = line->next) {
            printf("  line:\n");
            for (word0 = line->words; word0; word0 = word0->next) {
                printf("    word: x=%.2f..%.2f y=%.2f..%.2f base=%.2f fontSize=%.2f space=%d: '",
                       word0->xMin, word0->xMax, word0->yMin, word0->yMax,
                       word0->base, word0->fontSize, word0->spaceAfter);
                for (j = 0; j < word0->len; ++j) {
                    fputc(word0->text[j] & 0xff, stdout);
                }
                printf("'\n");
            }
        }
    }
    printf("\n");
#endif

    // build the flows
    //~ this needs to be adjusted for writing mode (vertical text)
    //~ this also needs to account for right-to-left column ordering
    blkArray = (CrackleTextBlock **)gmallocn(nBlocks, sizeof(CrackleTextBlock *));
    memcpy(blkArray, blocks, nBlocks * sizeof(CrackleTextBlock *));
    while (flows) {
        flow = flows;
        flows = flows->next;
        delete flow;
    }
    flows = lastFlow = NULL;
    firstBlkIdx = 0;
    nBlocksLeft = nBlocks;
    while (nBlocksLeft > 0) {

        // find the upper-left-most block
        for (; !blkArray[firstBlkIdx]; ++firstBlkIdx) ;
        i = firstBlkIdx;
        blk = blkArray[i];
        for (j = firstBlkIdx + 1; j < nBlocks; ++j) {
            blk1 = blkArray[j];
            if (blk1) {
                if (blk && blk->secondaryDelta(blk1) > 0) {
                    break;
                }
                if (blk1->primaryCmp(blk) < 0) {
                    i = j;
                    blk = blk1;
                }
            }
        }
        blkArray[i] = NULL;
        --nBlocksLeft;
        blk->next = NULL;

        // create a new flow, starting with the upper-left-most block
        flow = new CrackleTextFlow(this, blk);
        if (lastFlow) {
            lastFlow->next = flow;
        } else {
            flows = flow;
        }
        lastFlow = flow;
        fontSize = blk->lines->words->fontSize;

        // push the upper-left-most block on the stack
        blk->stackNext = NULL;
        blkStack = blk;

        // find the other blocks in this flow
        while (blkStack) {

            // find the upper-left-most block under (but within
            // maxBlockSpacing of) the top block on the stack
            blkSpace = maxBlockSpacing * blkStack->lines->words->fontSize;
            blk = NULL;
            i = -1;
            for (j = firstBlkIdx; j < nBlocks; ++j) {
                blk1 = blkArray[j];
                if (blk1) {
                    if (blkStack->secondaryDelta(blk1) > blkSpace) {
                        break;
                    }
                    if (blk && blk->secondaryDelta(blk1) > 0) {
                        break;
                    }
                    if (blk1->isBelow(blkStack) &&
                        (!blk || blk1->primaryCmp(blk) < 0)) {
                        i = j;
                        blk = blk1;
                    }
                }
            }

            // if a suitable block was found, add it to the flow and push it
            // onto the stack
            if (blk && flow->blockFits(blk, blkStack)) {
                blkArray[i] = NULL;
                --nBlocksLeft;
                blk->next = NULL;
                flow->addBlock(blk);
                fontSize = blk->lines->words->fontSize;
                blk->stackNext = blkStack;
                blkStack = blk;

                // otherwise (if there is no block under the top block or the
                // block is not suitable), pop the stack
            } else {
                blkStack = blkStack->stackNext;
            }
        }
    }
    gfree(blkArray);

#if 0 // for debugging
    printf("*** flows ***\n");
    for (flow = flows; flow; flow = flow->next) {
        printf("flow: x=%.2f..%.2f y=%.2f..%.2f pri:%.2f..%.2f\n",
               flow->xMin, flow->xMax, flow->yMin, flow->yMax,
               flow->priMin, flow->priMax);
        for (blk = flow->blocks; blk; blk = blk->next) {
            printf("  block: rot=%d x=%.2f..%.2f y=%.2f..%.2f pri=%.2f..%.2f\n",
                   blk->rot, blk->xMin, blk->xMax, blk->yMin, blk->yMax,
                   blk->priMin, blk->priMax);
            for (line = blk->lines; line; line = line->next) {
                printf("    line:\n");
                for (word0 = line->words; word0; word0 = word0->next) {
                    printf("      word: x=%.2f..%.2f y=%.2f..%.2f base=%.2f fontSize=%.2f space=%d: '",
                           word0->xMin, word0->xMax, word0->yMin, word0->yMax,
                           word0->base, word0->fontSize, word0->spaceAfter);
                    for (i = 0; i < word0->len; ++i) {
                        fputc(word0->text[i] & 0xff, stdout);
                    }
                    printf("'\n");
                }
            }
        }
    }
    printf("\n");
#endif

    // reorder flows to better manage two column papers with multiple flows on
    // a page due to e.g. scrappy sections before references but without breaking
    // two column papers with 3 column references such as "creative" layouts used
    // by Nature

    double left=0;
    double right=0;

    // calculate width of used page
    flow=flows;

    if (flow) {
        left=flow->xMin;
        right=flow->xMax;
        flow=flow->getNext();
    }

    while (flow) {

        // skip small flows such as headers/footers/titles that might span the page
        if(flow->yMax-flow->yMin > 40) {
            if(flow->xMin < left) {
                left=flow->xMin;
            }
            if(flow->xMax > right) {
                right=flow->xMax;
            }
        }
        flow=flow->getNext();
    }

    double middle=0.5 * (right-left) + left;

    bool centre_spanned(false);

    flow=flows;
    while (flow && !centre_spanned) {

        // skip small flows that might span the page
        if(flow->yMax-flow->yMin > columnSegmentationSkipHeight) {
            if(flow->xMin < middle && flow->xMax > middle) {
                centre_spanned=true;
            }
        }
        flow=flow->getNext();
    }

    if(!centre_spanned) {
        CrackleTextFlow *l=0, *left_flows=0;
        CrackleTextFlow *r=0, *right_flows=0;

        flow=flows;
        while (flow) {

            if(flow->xMax < middle) {
                if(!l) {
                    left_flows=flow;
                    l=left_flows;
                } else {
                    l->next=flow;
                    l=l->getNext();
                }
                flow=flow->getNext();
                l->next=0;
            } else {
                if(!r) {
                    right_flows=flow;
                    r=right_flows;
                } else {
                    r->next=flow;
                    r=r->getNext();
                }
                flow=flow->getNext();
                r->next=0;
            }
        }

        if(l) {
            l->next=right_flows;
            flows=left_flows;
        } else {
            flows=right_flows;
        }

    }

    if (uMap) {
        uMap->decRefCnt();
    }
}

bool CrackleTextPage::findText(Unicode *s, int len,
                                bool startAtTop, bool stopAtBottom,
                                bool startAtLast, bool stopAtLast,
                                bool caseSensitive, bool backward,
                                bool wholeWord,
                                double *xMin, double *yMin,
                                double *xMax, double *yMax) {
    CrackleTextBlock *blk;
    CrackleTextLine *line;
    Unicode *s2, *txt;
    Unicode *p;
    int txtSize, m, i, j, k;
    double xStart, yStart, xStop, yStop;
    double xMin0, yMin0, xMax0, yMax0;
    double xMin1, yMin1, xMax1, yMax1;
    bool found;

    //~ needs to handle right-to-left text

    if (rawOrder) {
        return false;
    }

    // convert the search string to uppercase
    if (!caseSensitive) {
        s2 = (Unicode *)gmallocn(len, sizeof(Unicode));
        for (i = 0; i < len; ++i) {
            s2[i] = unicodeToUpper(s[i]);
        }
    } else {
        s2 = s;
    }

    txt = NULL;
    txtSize = 0;

    xStart = yStart = xStop = yStop = 0;
    if (startAtLast && haveLastFind) {
        xStart = lastFindXMin;
        yStart = lastFindYMin;
    } else if (!startAtTop) {
        xStart = *xMin;
        yStart = *yMin;
    }
    if (stopAtLast && haveLastFind) {
        xStop = lastFindXMin;
        yStop = lastFindYMin;
    } else if (!stopAtBottom) {
        xStop = *xMax;
        yStop = *yMax;
    }

    found = false;
    xMin0 = xMax0 = yMin0 = yMax0 = 0; // make gcc happy
    xMin1 = xMax1 = yMin1 = yMax1 = 0; // make gcc happy

    for (i = backward ? nBlocks - 1 : 0;
         backward ? i >= 0 : i < nBlocks;
         i += backward ? -1 : 1) {
        blk = blocks[i];

        // check: is the block above the top limit?
        // (this only works if the page's primary rotation is zero --
        // otherwise the blocks won't be sorted in the useful order)
        if (!startAtTop && primaryRot == 0 &&
            (backward ? blk->yMin > yStart : blk->yMax < yStart)) {
            continue;
        }

        // check: is the block below the bottom limit?
        // (this only works if the page's primary rotation is zero --
        // otherwise the blocks won't be sorted in the useful order)
        if (!stopAtBottom && primaryRot == 0 &&
            (backward ? blk->yMax < yStop : blk->yMin > yStop)) {
            break;
        }

        for (line = blk->lines; line; line = line->next) {

            // check: is the line above the top limit?
            // (this only works if the page's primary rotation is zero --
            // otherwise the lines won't be sorted in the useful order)
            if (!startAtTop && primaryRot == 0 &&
                (backward ? line->yMin > yStart : line->yMin < yStart)) {
                continue;
            }

            // check: is the line below the bottom limit?
            // (this only works if the page's primary rotation is zero --
            // otherwise the lines won't be sorted in the useful order)
            if (!stopAtBottom && primaryRot == 0 &&
                (backward ? line->yMin < yStop : line->yMin > yStop)) {
                continue;
            }

            // convert the line to uppercase
            m = line->len;
            if (!caseSensitive) {
                if (m > txtSize) {
                    txt = (Unicode *)greallocn(txt, m, sizeof(Unicode));
                    txtSize = m;
                }
                for (k = 0; k < m; ++k) {
                    txt[k] = unicodeToUpper(line->text[k]);
                }
            } else {
                txt = line->text;
            }

            // search each position in this line
            j = backward ? m - len : 0;
            p = txt + j;
            while (backward ? j >= 0 : j <= m - len) {
                if (!wholeWord ||
                    ((j == 0 || !unicodeTypeAlphaNum(txt[j - 1])) &&
                     (j + len == m || !unicodeTypeAlphaNum(txt[j + len])))) {

                    // compare the strings
                    for (k = 0; k < len; ++k) {
                        if (p[k] != s2[k]) {
                            break;
                        }
                    }

                    // found it
                    if (k == len) {
                        switch (line->rot) {
                        case 0:
                            xMin1 = line->edge[j];
                            xMax1 = line->edge[j + len];
                            yMin1 = line->yMin;
                            yMax1 = line->yMax;
                            break;
                        case 1:
                            xMin1 = line->xMin;
                            xMax1 = line->xMax;
                            yMin1 = line->edge[j];
                            yMax1 = line->edge[j + len];
                            break;
                        case 2:
                            xMin1 = line->edge[j + len];
                            xMax1 = line->edge[j];
                            yMin1 = line->yMin;
                            yMax1 = line->yMax;
                            break;
                        case 3:
                            xMin1 = line->xMin;
                            xMax1 = line->xMax;
                            yMin1 = line->edge[j + len];
                            yMax1 = line->edge[j];
                            break;
                        }
                        if (backward) {
                            if ((startAtTop ||
                                 yMin1 < yStart || (yMin1 == yStart && xMin1 < xStart)) &&
                                (stopAtBottom ||
                                 yMin1 > yStop || (yMin1 == yStop && xMin1 > xStop))) {
                                if (!found ||
                                    yMin1 > yMin0 || (yMin1 == yMin0 && xMin1 > xMin0)) {
                                    xMin0 = xMin1;
                                    xMax0 = xMax1;
                                    yMin0 = yMin1;
                                    yMax0 = yMax1;
                                    found = true;
                                }
                            }
                        } else {
                            if ((startAtTop ||
                                 yMin1 > yStart || (yMin1 == yStart && xMin1 > xStart)) &&
                                (stopAtBottom ||
                                 yMin1 < yStop || (yMin1 == yStop && xMin1 < xStop))) {
                                if (!found ||
                                    yMin1 < yMin0 || (yMin1 == yMin0 && xMin1 < xMin0)) {
                                    xMin0 = xMin1;
                                    xMax0 = xMax1;
                                    yMin0 = yMin1;
                                    yMax0 = yMax1;
                                    found = true;
                                }
                            }
                        }
                    }
                }
                if (backward) {
                    --j;
                    --p;
                } else {
                    ++j;
                    ++p;
                }
            }
        }
    }

    if (!caseSensitive) {
        gfree(s2);
        gfree(txt);
    }

    if (found) {
        *xMin = xMin0;
        *xMax = xMax0;
        *yMin = yMin0;
        *yMax = yMax0;
        lastFindXMin = xMin0;
        lastFindYMin = yMin0;
        haveLastFind = true;
        return true;
    }

    return false;
}

GString *CrackleTextPage::getText(double xMin, double yMin,
                                  double xMax, double yMax) {
    GString *s;
    UnicodeMap *uMap;
    //bool isUnicode;
    CrackleTextBlock *blk;
    CrackleTextLine *line;
    CrackleTextLineFrag *frags;
    int nFrags, fragsSize;
    CrackleTextLineFrag *frag;
    char space[8], eol[16];
    int spaceLen, eolLen;
    int lastRot;
    double x, y, delta;
    int col, idx0, idx1, i, j;
    bool multiLine, oneRot;

    s = new GString();

    if (rawOrder) {
        return s;
    }

    // get the output encoding
    if (!(uMap = globalParams->getTextEncoding())) {
        return s;
    }
    //isUnicode = uMap->isUnicode();
    spaceLen = uMap->mapUnicode(0x20, space, sizeof(space));
    eolLen = 0; // make gcc happy
    switch (globalParams->getTextEOL()) {
    case eolUnix:
        eolLen = uMap->mapUnicode(0x0a, eol, sizeof(eol));
        break;
    case eolDOS:
        eolLen = uMap->mapUnicode(0x0d, eol, sizeof(eol));
        eolLen += uMap->mapUnicode(0x0a, eol + eolLen, sizeof(eol) - eolLen);
        break;
    case eolMac:
        eolLen = uMap->mapUnicode(0x0d, eol, sizeof(eol));
        break;
    }

    //~ writing mode (horiz/vert)

    // collect the line fragments that are in the rectangle
    fragsSize = 256;
    frags = (CrackleTextLineFrag *)gmallocn(fragsSize, sizeof(CrackleTextLineFrag));
    nFrags = 0;
    lastRot = -1;
    oneRot = true;
    for (i = 0; i < nBlocks; ++i) {
        blk = blocks[i];
        if (xMin < blk->xMax && blk->xMin < xMax &&
            yMin < blk->yMax && blk->yMin < yMax) {
            for (line = blk->lines; line; line = line->next) {
                if (xMin < line->xMax && line->xMin < xMax &&
                    yMin < line->yMax && line->yMin < yMax) {
                    idx0 = idx1 = -1;
                    switch (line->rot) {
                    case 0:
                        y = 0.5 * (line->yMin + line->yMax);
                        if (yMin < y && y < yMax) {
                            j = 0;
                            while (j < line->len) {
                                if (0.5 * (line->edge[j] + line->edge[j+1]) > xMin) {
                                    idx0 = j;
                                    break;
                                }
                                ++j;
                            }
                            j = line->len - 1;
                            while (j >= 0) {
                                if (0.5 * (line->edge[j] + line->edge[j+1]) < xMax) {
                                    idx1 = j;
                                    break;
                                }
                                --j;
                            }
                        }
                        break;
                    case 1:
                        x = 0.5 * (line->xMin + line->xMax);
                        if (xMin < x && x < xMax) {
                            j = 0;
                            while (j < line->len) {
                                if (0.5 * (line->edge[j] + line->edge[j+1]) > yMin) {
                                    idx0 = j;
                                    break;
                                }
                                ++j;
                            }
                            j = line->len - 1;
                            while (j >= 0) {
                                if (0.5 * (line->edge[j] + line->edge[j+1]) < yMax) {
                                    idx1 = j;
                                    break;
                                }
                                --j;
                            }
                        }
                        break;
                    case 2:
                        y = 0.5 * (line->yMin + line->yMax);
                        if (yMin < y && y < yMax) {
                            j = 0;
                            while (j < line->len) {
                                if (0.5 * (line->edge[j] + line->edge[j+1]) < xMax) {
                                    idx0 = j;
                                    break;
                                }
                                ++j;
                            }
                            j = line->len - 1;
                            while (j >= 0) {
                                if (0.5 * (line->edge[j] + line->edge[j+1]) > xMin) {
                                    idx1 = j;
                                    break;
                                }
                                --j;
                            }
                        }
                        break;
                    case 3:
                        x = 0.5 * (line->xMin + line->xMax);
                        if (xMin < x && x < xMax) {
                            j = 0;
                            while (j < line->len) {
                                if (0.5 * (line->edge[j] + line->edge[j+1]) < yMax) {
                                    idx0 = j;
                                    break;
                                }
                                ++j;
                            }
                            j = line->len - 1;
                            while (j >= 0) {
                                if (0.5 * (line->edge[j] + line->edge[j+1]) > yMin) {
                                    idx1 = j;
                                    break;
                                }
                                --j;
                            }
                        }
                        break;
                    }
                    if (idx0 >= 0 && idx1 >= 0) {
                        if (nFrags == fragsSize) {
                            fragsSize *= 2;
                            frags = (CrackleTextLineFrag *)
                                greallocn(frags, fragsSize, sizeof(CrackleTextLineFrag));
                        }
                        frags[nFrags].init(line, idx0, idx1 - idx0 + 1);
                        ++nFrags;
                        if (lastRot >= 0 && line->rot != lastRot) {
                            oneRot = false;
                        }
                        lastRot = line->rot;
                    }
                }
            }
        }
    }

    // sort the fragments and generate the string
    if (nFrags > 0) {

        for (i = 0; i < nFrags; ++i) {
            frags[i].computeCoords(oneRot);
        }
        assignColumns(frags, nFrags, oneRot);

        // if all lines in the region have the same rotation, use it;
        // otherwise, use the page's primary rotation
        if (oneRot) {
            qsort(frags, nFrags, sizeof(CrackleTextLineFrag),
                  &CrackleTextLineFrag::cmpYXLineRot);
        } else {
            qsort(frags, nFrags, sizeof(CrackleTextLineFrag),
                  &CrackleTextLineFrag::cmpYXPrimaryRot);
        }
        i = 0;
        while (i < nFrags) {
            delta = maxIntraLineDelta * frags[i].line->words->fontSize;
            for (j = i+1;
                 j < nFrags && fabs(frags[j].base - frags[i].base) < delta;
                 ++j) ;
            qsort(frags + i, j - i, sizeof(CrackleTextLineFrag),
                  oneRot ? &CrackleTextLineFrag::cmpXYColumnLineRot
                  : &CrackleTextLineFrag::cmpXYColumnPrimaryRot);
            i = j;
        }

        col = 0;
        multiLine = false;
        for (i = 0; i < nFrags; ++i) {
            frag = &frags[i];

            // insert a return
            if (frag->col < col ||
                (i > 0 && fabs(frag->base - frags[i-1].base) >
                 maxIntraLineDelta * frags[i-1].line->words->fontSize)) {
                s->append(eol, eolLen);
                col = 0;
                multiLine = true;
            }

            // column alignment
            for (; col < frag->col; ++col) {
                s->append(space, spaceLen);
            }

            // get the fragment text
            col += dumpFragment(frag->line->text + frag->start, frag->len, uMap, s);
        }

        if (multiLine) {
            s->append(eol, eolLen);
        }
    }

    gfree(frags);
    uMap->decRefCnt();

    return s;
}

bool CrackleTextPage::findCharRange(int pos, int length,
                                     double *xMin, double *yMin,
                                     double *xMax, double *yMax) {
    CrackleTextBlock *blk;
    CrackleTextLine *line;
    CrackleTextWord *word;
    double xMin0, xMax0, yMin0, yMax0;
    double xMin1, xMax1, yMin1, yMax1;
    bool first;
    int i, j0, j1;

    if (rawOrder) {
        return false;
    }

    //~ this doesn't correctly handle ranges split across multiple lines
    //~ (the highlighted region is the bounding box of all the parts of
    //~ the range)
    first = true;
    xMin0 = xMax0 = yMin0 = yMax0 = 0; // make gcc happy
    xMin1 = xMax1 = yMin1 = yMax1 = 0; // make gcc happy
    for (i = 0; i < nBlocks; ++i) {
        blk = blocks[i];
        for (line = blk->lines; line; line = line->next) {
            for (word = line->words; word; word = word->next) {
                if (pos < word->charPos[word->len] &&
                    pos + length > word->charPos[0]) {
                    for (j0 = 0;
                         j0 < word->len && pos >= word->charPos[j0 + 1];
                         ++j0) ;
                    for (j1 = word->len - 1;
                         j1 > j0 && pos + length <= word->charPos[j1];
                         --j1) ;
                    switch (line->rot) {
                    case 0:
                        xMin1 = word->edge[j0];
                        xMax1 = word->edge[j1 + 1];
                        yMin1 = word->yMin;
                        yMax1 = word->yMax;
                        break;
                    case 1:
                        xMin1 = word->xMin;
                        xMax1 = word->xMax;
                        yMin1 = word->edge[j0];
                        yMax1 = word->edge[j1 + 1];
                        break;
                    case 2:
                        xMin1 = word->edge[j1 + 1];
                        xMax1 = word->edge[j0];
                        yMin1 = word->yMin;
                        yMax1 = word->yMax;
                        break;
                    case 3:
                        xMin1 = word->xMin;
                        xMax1 = word->xMax;
                        yMin1 = word->edge[j1 + 1];
                        yMax1 = word->edge[j0];
                        break;
                    }
                    if (first || xMin1 < xMin0) {
                        xMin0 = xMin1;
                    }
                    if (first || xMax1 > xMax0) {
                        xMax0 = xMax1;
                    }
                    if (first || yMin1 < yMin0) {
                        yMin0 = yMin1;
                    }
                    if (first || yMax1 > yMax0) {
                        yMax0 = yMax1;
                    }
                    first = false;
                }
            }
        }
    }
    if (!first) {
        *xMin = xMin0;
        *xMax = xMax0;
        *yMin = yMin0;
        *yMax = yMax0;
        return true;
    }
    return false;
}

void CrackleTextPage::dump(void *outputStream, TextOutputFunc outputFunc,
                           bool physLayout) {
    UnicodeMap *uMap;
    CrackleTextFlow *flow;
    CrackleTextBlock *blk;
    CrackleTextLine *line;
    CrackleTextLineFrag *frags;
    CrackleTextWord *word;
    int nFrags, fragsSize;
    CrackleTextLineFrag *frag;
    char space[8], eol[16], eop[8];
    int spaceLen, eolLen, eopLen;
    bool pageBreaks;
    GString *s;
    double delta;
    int col, i, j, d, n;

    // get the output encoding
    if (!(uMap = globalParams->getTextEncoding())) {
        return;
    }
    spaceLen = uMap->mapUnicode(0x20, space, sizeof(space));
    eolLen = 0; // make gcc happy
    switch (globalParams->getTextEOL()) {
    case eolUnix:
        eolLen = uMap->mapUnicode(0x0a, eol, sizeof(eol));
        break;
    case eolDOS:
        eolLen = uMap->mapUnicode(0x0d, eol, sizeof(eol));
        eolLen += uMap->mapUnicode(0x0a, eol + eolLen, sizeof(eol) - eolLen);
        break;
    case eolMac:
        eolLen = uMap->mapUnicode(0x0d, eol, sizeof(eol));
        break;
    }
    eopLen = uMap->mapUnicode(0x0c, eop, sizeof(eop));
    pageBreaks = globalParams->getTextPageBreaks();

    //~ writing mode (horiz/vert)

    // output the page in raw (content stream) order
    if (rawOrder) {

        for (word = rawWords; word; word = word->next) {
            s = new GString();
            dumpFragment(word->text, word->len, uMap, s);
            (*outputFunc)(outputStream, s->c_str(), s->getLength());
            delete s;
            if (word->next &&
                fabs(word->next->base - word->base) <
                maxIntraLineDelta * word->fontSize &&
                word->next->xMin >
                word->xMax - minDupBreakOverlap * word->fontSize) {
                if (word->next->xMin > word->xMax + minWordSpacing * word->fontSize) {
                    (*outputFunc)(outputStream, space, spaceLen);
                }
            } else {
                (*outputFunc)(outputStream, eol, eolLen);
            }
        }

        // output the page, maintaining the original physical layout
    } else if (physLayout) {

        // collect the line fragments for the page and sort them
        fragsSize = 256;
        frags = (CrackleTextLineFrag *)gmallocn(fragsSize, sizeof(CrackleTextLineFrag));
        nFrags = 0;
        for (i = 0; i < nBlocks; ++i) {
            blk = blocks[i];
            for (line = blk->lines; line; line = line->next) {
                if (nFrags == fragsSize) {
                    fragsSize *= 2;
                    frags = (CrackleTextLineFrag *)greallocn(frags,
                                                             fragsSize, sizeof(CrackleTextLineFrag));
                }
                frags[nFrags].init(line, 0, line->len);
                frags[nFrags].computeCoords(true);
                ++nFrags;
            }
        }
        qsort(frags, nFrags, sizeof(CrackleTextLineFrag), &CrackleTextLineFrag::cmpYXPrimaryRot);
        i = 0;
        while (i < nFrags) {
            delta = maxIntraLineDelta * frags[i].line->words->fontSize;
            for (j = i+1;
                 j < nFrags && fabs(frags[j].base - frags[i].base) < delta;
                 ++j) ;
            qsort(frags + i, j - i, sizeof(CrackleTextLineFrag),
                  &CrackleTextLineFrag::cmpXYColumnPrimaryRot);
            i = j;
        }

#if 0 // for debugging
        printf("*** line fragments ***\n");
        for (i = 0; i < nFrags; ++i) {
            frag = &frags[i];
            printf("frag: x=%.2f..%.2f y=%.2f..%.2f base=%.2f '",
                   frag->xMin, frag->xMax, frag->yMin, frag->yMax, frag->base);
            for (n = 0; n < frag->len; ++n) {
                fputc(frag->line->text[frag->start + n] & 0xff, stdout);
            }
            printf("'\n");
        }
        printf("\n");
#endif

        // generate output
        col = 0;
        for (i = 0; i < nFrags; ++i) {
            frag = &frags[i];

            // column alignment
            for (; col < frag->col; ++col) {
                (*outputFunc)(outputStream, space, spaceLen);
            }

            // print the line
            s = new GString();
            col += dumpFragment(frag->line->text + frag->start, frag->len, uMap, s);
            (*outputFunc)(outputStream, s->c_str(), s->getLength());
            delete s;

            // print one or more returns if necessary
            if (i == nFrags - 1 ||
                frags[i+1].col < col ||
                fabs(frags[i+1].base - frag->base) >
                maxIntraLineDelta * frag->line->words->fontSize) {
                if (i < nFrags - 1) {
                    d = (int)((frags[i+1].base - frag->base) /
                              frag->line->words->fontSize);
                    if (d < 1) {
                        d = 1;
                    } else if (d > 5) {
                        d = 5;
                    }
                } else {
                    d = 1;
                }
                for (; d > 0; --d) {
                    (*outputFunc)(outputStream, eol, eolLen);
                }
                col = 0;
            }
        }

        gfree(frags);

        // output the page, "undoing" the layout
    } else {
        for (flow = flows; flow; flow = flow->next) {
            for (blk = flow->blocks; blk; blk = blk->next) {
                for (line = blk->lines; line; line = line->next) {
                    n = line->len;
                    if (line->hyphenated && (line->next || blk->next)) {
                        --n;
                    }
                    s = new GString();
                    dumpFragment(line->text, n, uMap, s);
                    (*outputFunc)(outputStream, s->c_str(), s->getLength());
                    delete s;
                    if (!line->hyphenated) {
                        if (line->next) {
                            (*outputFunc)(outputStream, space, spaceLen);
                        } else if (blk->next) {
                            //~ this is a bit of a kludge - we should really do a more
                            //~ intelligent determination of paragraphs
                            if (blk->next->lines->words->fontSize ==
                                blk->lines->words->fontSize) {
                                (*outputFunc)(outputStream, space, spaceLen);
                            } else {
                                (*outputFunc)(outputStream, eol, eolLen);
                            }
                        }
                    }
                }
            }
            (*outputFunc)(outputStream, eol, eolLen);
            (*outputFunc)(outputStream, eol, eolLen);
        }
    }

    // end of page
    if (pageBreaks) {
        (*outputFunc)(outputStream, eop, eopLen);
    }

    uMap->decRefCnt();
}

void CrackleTextPage::assignColumns(CrackleTextLineFrag *frags, int nFrags, bool oneRot) {
    CrackleTextLineFrag *frag0, *frag1;
    int rot, col1, col2, i, j, k;

    // all text in the region has the same rotation -- recompute the
    // column numbers based only on the text in the region
    if (oneRot) {
        qsort(frags, nFrags, sizeof(CrackleTextLineFrag), &CrackleTextLineFrag::cmpXYLineRot);
        rot = frags[0].line->rot;
        for (i = 0; i < nFrags; ++i) {
            frag0 = &frags[i];
            col1 = 0;
            for (j = 0; j < i; ++j) {
                frag1 = &frags[j];
                col2 = 0; // make gcc happy
                switch (rot) {
                case 0:
                    if (frag0->xMin >= frag1->xMax) {
                        col2 = frag1->col + (frag1->line->col[frag1->start + frag1->len] -
                                             frag1->line->col[frag1->start]) + 1;
                    } else {
                        for (k = frag1->start;
                             k < frag1->start + frag1->len &&
                                 frag0->xMin >= 0.5 * (frag1->line->edge[k] +
                                                       frag1->line->edge[k+1]);
                             ++k) ;
                        col2 = frag1->col +
                            frag1->line->col[k] - frag1->line->col[frag1->start];
                    }
                    break;
                case 1:
                    if (frag0->yMin >= frag1->yMax) {
                        col2 = frag1->col + (frag1->line->col[frag1->start + frag1->len] -
                                             frag1->line->col[frag1->start]) + 1;
                    } else {
                        for (k = frag1->start;
                             k < frag1->start + frag1->len &&
                                 frag0->yMin >= 0.5 * (frag1->line->edge[k] +
                                                       frag1->line->edge[k+1]);
                             ++k) ;
                        col2 = frag1->col +
                            frag1->line->col[k] - frag1->line->col[frag1->start];
                    }
                    break;
                case 2:
                    if (frag0->xMax <= frag1->xMin) {
                        col2 = frag1->col + (frag1->line->col[frag1->start + frag1->len] -
                                             frag1->line->col[frag1->start]) + 1;
                    } else {
                        for (k = frag1->start;
                             k < frag1->start + frag1->len &&
                                 frag0->xMax <= 0.5 * (frag1->line->edge[k] +
                                                       frag1->line->edge[k+1]);
                             ++k) ;
                        col2 = frag1->col +
                            frag1->line->col[k] - frag1->line->col[frag1->start];
                    }
                    break;
                case 3:
                    if (frag0->yMax <= frag1->yMin) {
                        col2 = frag1->col + (frag1->line->col[frag1->start + frag1->len] -
                                             frag1->line->col[frag1->start]) + 1;
                    } else {
                        for (k = frag1->start;
                             k < frag1->start + frag1->len &&
                                 frag0->yMax <= 0.5 * (frag1->line->edge[k] +
                                                       frag1->line->edge[k+1]);
                             ++k) ;
                        col2 = frag1->col +
                            frag1->line->col[k] - frag1->line->col[frag1->start];
                    }
                    break;
                }
                if (col2 > col1) {
                    col1 = col2;
                }
            }
            frag0->col = col1;
        }

        // the region includes text at different rotations -- use the
        // globally assigned column numbers, offset by the minimum column
        // number (i.e., shift everything over to column 0)
    } else {
        col1 = frags[0].col;
        for (i = 1; i < nFrags; ++i) {
            if (frags[i].col < col1) {
                col1 = frags[i].col;
            }
        }
        for (i = 0; i < nFrags; ++i) {
            frags[i].col -= col1;
        }
    }
}

int CrackleTextPage::dumpFragment(Unicode *text, int len, UnicodeMap *uMap,
                                  GString *s) {
    char lre[8], rle[8], popdf[8], buf[8];
    int lreLen, rleLen, popdfLen, n;
    int nCols, i, j, k;

    nCols = 0;

    if (uMap->isUnicode()) {

        lreLen = uMap->mapUnicode(0x202a, lre, sizeof(lre));
        rleLen = uMap->mapUnicode(0x202b, rle, sizeof(rle));
        popdfLen = uMap->mapUnicode(0x202c, popdf, sizeof(popdf));

        if (primaryLR) {

            i = 0;
            while (i < len) {
                // output a left-to-right section
                for (j = i; j < len && !unicodeTypeR(text[j]); ++j) ;
                for (k = i; k < j; ++k) {
                    n = uMap->mapUnicode(text[k], buf, sizeof(buf));
                    s->append(buf, n);
                    ++nCols;
                }
                i = j;
                // output a right-to-left section
                for (j = i;
                     j < len && !(unicodeTypeL(text[j]) || unicodeTypeNum(text[j]));
                     ++j) ;
                if (j > i) {
                    s->append(rle, rleLen);
                    for (k = j - 1; k >= i; --k) {
                        n = uMap->mapUnicode(text[k], buf, sizeof(buf));
                        s->append(buf, n);
                        ++nCols;
                    }
                    s->append(popdf, popdfLen);
                    i = j;
                }
            }

        } else {

            // Note: This code treats numeric characters (European and
            // Arabic/Indic) as left-to-right, which isn't strictly correct
            // (incurs extra LRE/POPDF pairs), but does produce correct
            // visual formatting.
            s->append(rle, rleLen);
            i = len - 1;
            while (i >= 0) {
                // output a right-to-left section
                for (j = i;
                     j >= 0 && !(unicodeTypeL(text[j]) || unicodeTypeNum(text[j]));
                     --j) ;
                for (k = i; k > j; --k) {
                    n = uMap->mapUnicode(text[k], buf, sizeof(buf));
                    s->append(buf, n);
                    ++nCols;
                }
                i = j;
                // output a left-to-right section
                for (j = i; j >= 0 && !unicodeTypeR(text[j]); --j) ;
                if (j < i) {
                    s->append(lre, lreLen);
                    for (k = j + 1; k <= i; ++k) {
                        n = uMap->mapUnicode(text[k], buf, sizeof(buf));
                        s->append(buf, n);
                        ++nCols;
                    }
                    s->append(popdf, popdfLen);
                    i = j;
                }
            }
            s->append(popdf, popdfLen);

        }

    } else {
        for (i = 0; i < len; ++i) {
            n = uMap->mapUnicode(text[i], buf, sizeof(buf));
            s->append(buf, n);
            nCols += n;
        }
    }

    return nCols;
}

#if TEXTOUT_WORD_LIST
CrackleTextWordList *CrackleTextPage::makeWordList(bool physLayout) {
    return new CrackleTextWordList(this, physLayout);
}
#endif

//------------------------------------------------------------------------
// CrackleTextOutputDev
//------------------------------------------------------------------------

static void outputToFile(void *stream, const char *text, int len) {
    fwrite(text, 1, len, (FILE *)stream);
}

CrackleTextOutputDev::CrackleTextOutputDev(char *fileName, bool physLayoutA,
                                           double fixedPitchA, bool rawOrderA, bool append)
    : _images(boost::shared_ptr<ImageCollection> (new ImageCollection))
{
    text = NULL;
    physLayout = physLayoutA;
    fixedPitch = physLayout ? fixedPitchA : 0;
    rawOrder = rawOrderA;
    doHTML = false;
    ok = true;

    // open file
    needClose = false;
    if (fileName) {
        if (!strcmp(fileName, "-")) {
            outputStream = stdout;
#ifdef WIN32
            // keep DOS from munging the end-of-line characters
            setmode(fileno(stdout), O_BINARY);
#endif
        } else if ((outputStream = fopen(fileName, append ? "ab" : "wb"))) {
            needClose = true;
        } else {
            error(errIO, -1, "Couldn't open text file '{0:s}'", fileName);
            ok = false;
            return;
        }
        outputFunc = &outputToFile;
    } else {
        outputStream = NULL;
    }

    // set up text object
    text = new CrackleTextPage(rawOrderA);
}

CrackleTextOutputDev::CrackleTextOutputDev(TextOutputFunc func, void *stream,
                                           bool physLayoutA, double fixedPitchA, bool rawOrderA)
    : _images(boost::shared_ptr<ImageCollection> (new ImageCollection))
{
    outputFunc = func;
    outputStream = stream;
    needClose = false;
    physLayout = physLayoutA;
    fixedPitch = physLayout ? fixedPitchA : 0;
    rawOrder = rawOrderA;
    doHTML = false;
    text = new CrackleTextPage(rawOrderA);
    ok = true;
}

CrackleTextOutputDev::~CrackleTextOutputDev() {
    if (needClose) {
#ifdef MACOS
        ICS_MapRefNumAndAssign((short)((FILE *)outputStream)->handle);
#endif
        fclose((FILE *)outputStream);
    }
    if (text) {
        delete text;
    }
}

void CrackleTextOutputDev::startPage(int pageNum, GfxState *state) {
    _images=boost::shared_ptr<ImageCollection> (new ImageCollection);
    text->startPage(state);
}

void CrackleTextOutputDev::endPage() {
    text->endPage();
    text->coalesce(physLayout, fixedPitch, doHTML);
    if (outputStream) {
        text->dump(outputStream, outputFunc, physLayout);
    }
}

void CrackleTextOutputDev::restoreState(GfxState *state) {
    text->updateFont(state);
}

void CrackleTextOutputDev::updateFont(GfxState *state) {
    text->updateFont(state);
}

void CrackleTextOutputDev::beginString(GfxState *state, GString *s) {
}

void CrackleTextOutputDev::endString(GfxState *state) {
}

void CrackleTextOutputDev::drawImage(GfxState *state, Object *ref, Stream *str,
                                     int width, int height, GfxImageColorMap *colorMap,
                                     int *maskColors, bool inlineImg, bool interpolate)
{
    int c;
    size_t size;
    Image::ImageType type;
    char * data;
    const double *ctm;
    double mat[6];
    bool rot;
    double xScale, yScale;

    ctm=state->getCTM();
    mat[0] = ctm[0];
    mat[1] = ctm[1];
    mat[2] = -ctm[2];
    mat[3] = -ctm[3];
    mat[4] = ctm[2] + ctm[4];
    mat[5] = ctm[3] + ctm[5];

    // check for singular matrix
    if (fabs(mat[0] * mat[3] - mat[1] * mat[2]) < 0.000001) {
        return;
    }

    BoundingBox bbox;

    rot = fabs(mat[1]) > fabs(mat[0]);
    if (rot) {
        xScale = -mat[1];
        yScale = mat[2] - (mat[0] * mat[3]) / mat[1];
        bbox = BoundingBox(mat[4], mat[5] - xScale, mat[4] + yScale, mat[5]);
        // FIXME is this right?
    } else {
        xScale = mat[0];
        yScale = mat[3] - (mat[1] * mat[2]) / mat[0];
        bbox = BoundingBox(mat[4], mat[5], mat[4] + xScale, mat[5] + yScale);
    }


    if (colorMap->getNumPixelComps() == 1 && colorMap->getBits() == 1) {

        type=Image::Bitmap;

        str->reset();

        // copy the stream
        size = height * ((width + 7) / 8);
        data=new char[size];
        for (size_t i = 0; i < size; ++i) {
            data[i]=str->getChar();
        }

        str->close();

    } else if (str->getKind() == strDCT && colorMap->getNumPixelComps() == 3
	       && !inlineImg) {

        type=Image::JPEG;

        vector<char> buffer;

#ifdef UTOPIA_SPINE_BACKEND_POPPLER
	str = str->getNextStream();
#else
        str = ((DCTStream *)str)->getRawStream();
#endif
        str->reset();

        while ( (c= str->getChar()) != EOF ) {
            buffer.push_back(static_cast<char>(c));
        }
        str->close();

        size=buffer.size();
        data=new char[size];
        copy(buffer.begin(), buffer.end(), data);
        buffer.clear();

    } else {

        type=Image::RGB;

        size=(height * width * 3);
        data=new char[size];

        // initialize stream
        ImageStream *imgStr = new ImageStream(str, width, colorMap->getNumPixelComps(),
                                              colorMap->getBits());
        imgStr->reset();

        GfxRGB rgb;
        size_t i(0);

        // for each line...
        for (size_t y = 0; y < static_cast<size_t>(height); ++y) {

            // write the line
            unsigned char *p = imgStr->getLine();
            for (size_t x = 0; x < static_cast<size_t>(width); ++x) {
                colorMap->getRGB(p, &rgb);
                data[i++]=colToByte(rgb.r);
                data[i++]=colToByte(rgb.g);
                data[i++]=colToByte(rgb.b);
                p += colorMap->getNumPixelComps();
            }
        }

        delete imgStr;
    }

    this->_images->push_back( Image(type, width, height, bbox, data, size) );
    delete [] data;
}

void CrackleTextOutputDev::drawChar(GfxState *state, double x, double y,
                                    double dx, double dy,
                                    double originX, double originY,
                                    CharCode c, int nBytes, Unicode *u, int uLen) {
    text->addChar(state, x, y, dx, dy, c, nBytes, u, uLen);
}

void CrackleTextOutputDev::incCharCount(int nChars) {
    text->incCharCount(nChars);
}

void CrackleTextOutputDev::beginActualText(GfxState *state, Unicode *u, int uLen) {
    text->beginActualText(state, u, uLen);
}

void CrackleTextOutputDev::endActualText(GfxState *state) {
    text->endActualText(state);
}


void CrackleTextOutputDev::stroke(GfxState *state) {
    GfxPath *path;
    GfxSubpath *subpath;
    double x[2], y[2];

    if (!doHTML) {
        return;
    }
    path = state->getPath();
    if (path->getNumSubpaths() != 1) {
        return;
    }
    subpath = path->getSubpath(0);
    if (subpath->getNumPoints() != 2) {
        return;
    }
    state->transform(subpath->getX(0), subpath->getY(0), &x[0], &y[0]);
    state->transform(subpath->getX(1), subpath->getY(1), &x[1], &y[1]);

    // look for a vertical or horizontal line
    if (x[0] == x[1] || y[0] == y[1]) {
        text->addUnderline(x[0], y[0], x[1], y[1]);
    }
}

void CrackleTextOutputDev::fill(GfxState *state) {
    GfxPath *path;
    GfxSubpath *subpath;
    double x[5], y[5];
    double rx0, ry0, rx1, ry1, t;
    int i;

    if (!doHTML) {
        return;
    }
    path = state->getPath();
    if (path->getNumSubpaths() != 1) {
        return;
    }
    subpath = path->getSubpath(0);
    if (subpath->getNumPoints() != 5) {
        return;
    }
    for (i = 0; i < 5; ++i) {
        if (subpath->getCurve(i)) {
            return;
        }
        state->transform(subpath->getX(i), subpath->getY(i), &x[i], &y[i]);
    }

    // look for a rectangle
    if (x[0] == x[1] && y[1] == y[2] && x[2] == x[3] && y[3] == y[4] &&
        x[0] == x[4] && y[0] == y[4]) {
        rx0 = x[0];
        ry0 = y[0];
        rx1 = x[2];
        ry1 = y[1];
    } else if (y[0] == y[1] && x[1] == x[2] && y[2] == y[3] && x[3] == x[4] &&
               x[0] == x[4] && y[0] == y[4]) {
        rx0 = x[0];
        ry0 = y[0];
        rx1 = x[1];
        ry1 = y[2];
    } else {
        return;
    }
    if (rx1 < rx0) {
        t = rx0;
        rx0 = rx1;
        rx1 = t;
    }
    if (ry1 < ry0) {
        t = ry0;
        ry0 = ry1;
        ry1 = t;
    }

    // skinny horizontal rectangle
    if (ry1 - ry0 < rx1 - rx0) {
        if (ry1 - ry0 < maxUnderlineWidth) {
            ry0 = 0.5 * (ry0 + ry1);
            text->addUnderline(rx0, ry0, rx1, ry0);
        }

        // skinny vertical rectangle
    } else {
        if (rx1 - rx0 < maxUnderlineWidth) {
            rx0 = 0.5 * (rx0 + rx1);
            text->addUnderline(rx0, ry0, rx0, ry1);
        }
    }
}

void CrackleTextOutputDev::eoFill(GfxState *state) {
    if (!doHTML) {
        return;
    }
    fill(state);
}

void CrackleTextOutputDev::processLink(Link *link) {
    double x1, y1, x2, y2;
    int xMin, yMin, xMax, yMax, x, y;

    if (!doHTML) {
        return;
    }
    link->getRect(&x1, &y1, &x2, &y2);
    cvtUserToDev(x1, y1, &x, &y);
    xMin = xMax = x;
    yMin = yMax = y;
    cvtUserToDev(x1, y2, &x, &y);
    if (x < xMin) {
        xMin = x;
    } else if (x > xMax) {
        xMax = x;
    }
    if (y < yMin) {
        yMin = y;
    } else if (y > yMax) {
        yMax = y;
    }
    cvtUserToDev(x2, y1, &x, &y);
    if (x < xMin) {
        xMin = x;
    } else if (x > xMax) {
        xMax = x;
    }
    if (y < yMin) {
        yMin = y;
    } else if (y > yMax) {
        yMax = y;
    }
    cvtUserToDev(x2, y2, &x, &y);
    if (x < xMin) {
        xMin = x;
    } else if (x > xMax) {
        xMax = x;
    }
    if (y < yMin) {
        yMin = y;
    } else if (y > yMax) {
        yMax = y;
    }
    text->addLink(xMin, yMin, xMax, yMax, link);
}

bool CrackleTextOutputDev::findText(Unicode *s, int len,
                                     bool startAtTop, bool stopAtBottom,
                                     bool startAtLast, bool stopAtLast,
                                     bool caseSensitive, bool backward,
                                     bool wholeWord,
                                     double *xMin, double *yMin,
                                     double *xMax, double *yMax) {
    return text->findText(s, len, startAtTop, stopAtBottom,
                          startAtLast, stopAtLast,
                          caseSensitive, backward, wholeWord,
                          xMin, yMin, xMax, yMax);
}

GString *CrackleTextOutputDev::getText(double xMin, double yMin,
                                       double xMax, double yMax) {
    return text->getText(xMin, yMin, xMax, yMax);
}

bool CrackleTextOutputDev::findCharRange(int pos, int length,
                                          double *xMin, double *yMin,
                                          double *xMax, double *yMax) {
    return text->findCharRange(pos, length, xMin, yMin, xMax, yMax);
}

#if TEXTOUT_WORD_LIST
CrackleTextWordList *CrackleTextOutputDev::makeWordList() {
    return text->makeWordList(physLayout);
}
#endif

CrackleTextPage *CrackleTextOutputDev::takeText() {
    CrackleTextPage *ret;

    ret = text;
    text = new CrackleTextPage(rawOrder);
    return ret;
}
