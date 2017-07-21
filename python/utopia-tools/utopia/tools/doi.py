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

_inner_regex = r'(10\.\d+/[^%"\#\s]+)'
_strip_regex = r'[^\d\w]+'



def search(text):
    '''Look for first DOI in some text'''
    match = re.search(_inner_regex, text)
    if match is not None:
        return re.sub(r'(^{0}|{0}$)'.format(_strip_regex), r'', match.group(0))

def findall(text):
    '''Look for all DOIs in some text'''
    matches = re.findall(_inner_regex, text)
    return [re.sub(r'(^{0}|{0}$)'.format(_strip_regex), r'', match) for match in matches]



__all__ = ['search', 'findall']


try:
    import spineapi
    _regex = r'(?:(?:doi|digital\s+object\s+id(?:entifier)?)\s*\S?\s*)?' + _inner_regex
    def scrape(document):
        '''Look for a DOI in the document'''
        margin = 90
        dois = []
        for match in document.search(_regex, spineapi.IgnoreCase + spineapi.RegExp):
            page, _, (_, _), (width, height) = pageArea = match.begin().pageArea()
            _, orientation, (left, top), (right, bottom) = lineArea = match.begin().lineArea()
            #print pageArea, lineArea

            # Ignore any matches past page 2
            if page > 2:
                break

            # Accept front-page DOI if present
            if page == 2 and len(dois) > 0:
                break

            # Add to list according to where on the page it is found
            # If vertical or in margins, prepend:
            if orientation > 0:
                dois[0:0] = [match.text()]
            elif top > (height - margin):
                dois[0:0] = [match.text()]
            else:
                dois.append(match.text())

        if len(dois) > 0:
            doi = dois[0]
            doi = re.search(_inner_regex, doi).group(0)
            doi = re.sub(r'(^{0}|{0}$)'.format(_strip_regex), r'', doi)
            doi = re.sub(ur'[\u002D\u00AD\u2010\u2011\u2013-\u2015\u2E3A\u2E3B]', '-', doi)
            return doi

    __all__[:] = ['scrape']
except ImportError:
    logger.info('spineapi not imported: document scraping of DOIs will be unavailable')
    spineapi = None
