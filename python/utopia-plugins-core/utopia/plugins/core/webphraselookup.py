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

import urllib
import utopia.document
import re

_strippable = re.compile(r'\W+', re.UNICODE)

def strip(input):
    return _strippable.sub(' ', input)

class NCBIPhraseLookup(utopia.document.PhraseLookup):
    """7Search NCBI Databases"""
    def lookup(self, phrase):
        return "https://www.ncbi.nlm.nih.gov/gquery/?term=%s" % urllib.quote(strip(phrase).encode('utf-8'))

class QuertlePhraseLookup(utopia.document.PhraseLookup):
    """7Search Quertle"""
    def lookup(self, phrase):
        return "http://www.quertle.info/search?s=r&query=%s" % urllib.quote(strip(phrase).encode('utf-8'))

class WikipediaPhraseLookup(utopia.document.PhraseLookup):
    """8Search Wikipedia"""
    def lookup(self, phrase):
        return "http://en.wikipedia.org/wiki/search.php?search=%s" % urllib.quote(strip(phrase).encode('utf-8'))

class DictionaryLookup(utopia.document.PhraseLookup):
    """9Find in Dictionary"""
    def lookup(self, phrase):
        return "http://dictionary.reference.com/browse/%s" % urllib.quote(strip(phrase).encode('utf-8'))

class ThesaurusLookup(utopia.document.PhraseLookup):
    """9Find in Thesaurus"""
    def lookup(self, phrase):
        return "http://thesaurus.reference.com/browse/%s" % urllib.quote(strip(phrase).encode('utf-8'))
