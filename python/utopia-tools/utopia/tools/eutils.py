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

#? name: EUtils (NCBI)
#? www: http://www.ncbi.nlm.nih.gov/
#? urls: http://eutils.ncbi.nlm.nih.gov/ http://www.ncbi.nlm.nih.gov/


import socket
import time
import urllib
import urllib2


# pubmed stopwords
STOPWORDS = ('a', 'about', 'again', 'all', 'almost', 'also', 'although', 'always', 'among',
             'an', 'and', 'another', 'any', 'are', 'as', 'at', 'be', 'because', 'been',
             'before', 'being', 'between', 'both', 'but', 'by', 'can', 'could', 'did', 'do',
             'does', 'done', 'due', 'during', 'each', 'either', 'enough', 'especially', 'etc',
             'for', 'found', 'from', 'further', 'had', 'has', 'have', 'having', 'here', 'how',
             'however', 'i', 'if', 'in', 'into', 'is', 'it', 'its', 'itself', 'just', 'kg',
             'km', 'made', 'mainly', 'make', 'may', 'mg', 'might', 'ml', 'mm', 'most', 'mostly',
             'must', 'nearly', 'neither', 'no', 'nor', 'obtained', 'of', 'often', 'on', 'our',
             'overall', 'perhaps', 'pmid', 'quite', 'rather', 'really', 'regarding', 'seem',
             'seen', 'several', 'should', 'show', 'showed', 'shown', 'shows', 'significantly',
             'since', 'so', 'some', 'such', 'than', 'that', 'the', 'their', 'theirs', 'them',
             'then', 'there', 'therefore', 'these', 'they', 'this', 'those', 'through', 'thus',
             'to', 'upon', 'use', 'used', 'using', 'various', 'very', 'was', 'we', 'were',
             'what', 'when', 'which', 'while', 'with', 'within', 'without', 'would')


def eutils(utility, **defaultparams):
    defaultparams.update({
        'db': 'pubmed',
        'tool': 'UtopiaDocuments',
        'email': 'utopia@cs.man.ac.uk',
    })
    def _eutils(**userparams):
        """Execute an eutils utility"""
        params = defaultparams.copy()
        params.update(userparams)
        for k, v in params.items():
            if v is None:
                del params[k]
            else:
                params[k] = unicode(v).encode('utf8')
        url = 'https://eutils.ncbi.nlm.nih.gov/entrez/eutils/{0}.fcgi?{1}'
        url = url.format(utility, urllib.urlencode(params))
        response = None
        for wait in (1, 2, 3, 4, 5):
            try:
                response = urllib2.urlopen(url, timeout = 5 + wait)
                break
            except socket.timeout:
                # Wait and retry if it timed out
                if wait != 5:
                    time.sleep(wait)
                else:
                    raise
        return response and response.read()
    return _eutils

esearch = eutils('esearch', usehistory = 'y', retmax = 100)
espell = eutils('espell', retmode = 'xml')
efetch = eutils('efetch', retmode = 'xml')
egquery = eutils('egquery')
