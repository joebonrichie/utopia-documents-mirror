###############################################################################
#   
#    This file is part of the Utopia Documents application.
#        Copyright (c) 2008-2017 Lost Island Labs
#            <info@utopiadocs.com>
#    
#    Utopia Documents is free software: you can redistribute it and/or modify
#    it under the terms of the GNU GENERAL PUBLIC LICENSE VERSION 3 as
#    published by the Free Software Foundation.
#    
#    Utopia Documents is distributed in the hope that it will be useful, but
#    WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
#    Public License for more details.
#    
#    In addition, as a special exception, the copyright holders give
#    permission to link the code of portions of this program with the OpenSSL
#    library under certain conditions as described in each individual source
#    file, and distribute linked combinations including the two.
#    
#    You must obey the GNU General Public License in all respects for all of
#    the code used other than OpenSSL. If you modify file(s) with this
#    exception, you may extend this exception to your version of the file(s),
#    but you are not obligated to do so. If you do not wish to do so, delete
#    this exception statement from your version.
#    
#    You should have received a copy of the GNU General Public License
#    along with Utopia Documents. If not, see <http://www.gnu.org/licenses/>
#   
###############################################################################

import re
from utopia.log import logger



try:
    import spineapi
    def scrape(document):
        '''Look for a title on the front page'''

        fontSizeFrequency = {}
        allLines = []
        for line in document.newCursor().lines(spineapi.UntilEndOfPage):
            if line.lineArea()[1] == 0:
                allLines.append(line)
                fontSizeFrequencyForLine = {}
                for word in line.words(spineapi.UntilEndOfLine):
                    fontInfo = word.wordFontSize(), word.wordFontName()
                    fontSizeFrequencyForLine.setdefault(fontInfo, 0)
                    fontSizeFrequencyForLine[fontInfo] += 1
                maxFont = (0, '')
                for fontInfo, frequency in fontSizeFrequencyForLine.iteritems():
                    if fontInfo[0] > maxFont[0]:
                        maxFont = fontInfo
                fontSizeFrequency.setdefault(maxFont, [])
                fontSizeFrequency[maxFont].append(line)

        #print fontSizeFrequency

        fonts = []
        for fontInfo, lines in fontSizeFrequency.iteritems():
            fonts.append(fontInfo + (lines,))

        fontsBySize = sorted(fonts, key=lambda fs: fs[0], reverse = True)

        #print 'Sorted by size'
        #print fontsBySize

        candidateLines = None
        for size, font, lines in fontsBySize:
            wordCount = reduce(lambda accum, line: accum + len(tuple(line.words(spineapi.UntilEndOfLine))), lines, 0)
            #print '---', wordCount
            #print u' '.join([l.lineText() for l in lines]).encode('utf8')
            if wordCount > 2:
                candidateLines = lines
                break

        if candidateLines is not None and len(candidateLines) < 10:
            titleString = ''
            started = False
            for line in allLines:
                if line in candidateLines:
                    if titleString != '':
                        titleString = titleString + ' '
                    titleString = titleString + line.lineText()
                    started = True
                elif started:
                    break

            # sanitise
            return re.sub(r'[\s,;:*]+$', '', titleString)

    __all__ = ['scrape']
except ImportError:
    logger.info('spineapi not imported: document scraping of titles will be unavailable')
