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

import logging
import tempfile
import shutil

from pylons import request, response, session, tmpl_context as c
from pylons.controllers.util import abort, redirect_to

from pdftextextract.lib.base import BaseController, render

from crackleapi import loadPDF

def doc2text(doc):
    result=""
    for page in doc.pages():

        _ ,_ ,(_, top_edge),(_, bottom_edge)= page.pageArea()

        # skip text overlapping margins of 50pt
        top_edge+=45
        bottom_edge-=45
        new_line=False
        new_block=False

        pre_bottom=-1

        # store bottom of last output line
        for region in page.regions():

            for block in region.blocks():

                _,_,(block_left,_),(_,_)= block.blockArea();

                for line in block.lines():

                    text = line.lineText().strip()

                    pg,angle,(left,top),(right,bottom)= line.lineArea();

                    # skip if text enters margins
                    if bottom>bottom_edge or top < top_edge:
                        continue

                    # Check if we might be starting a new sentence
                    if len(text) and text[0]==text[0].upper():

                        if new_block:
                            new_line=True
                            new_block=False

                        # ensure blank line between indented paragraphs
                        if left > block_left + 8:
                            new_line=True

                        # check for blank line
                        if pre_bottom != -1 and top > pre_bottom + (bottom - top):
                            new_line=True

                    # print just one line separating blocks
                    if new_line:
                        result += "\n"
                        new_line=False

                    # store line
                    result+=text + "\n"

                    pre_bottom=bottom

                # separate blocks with blank line
                new_block=True

    return result

log = logging.getLogger(__name__)

class PdftextController(BaseController):

    def index(self):
        return render ('/pdfupload.mako')

    def upload(self):

        try:
            myfile = request.POST['pdfdoc']
            pdfdoc=tempfile.NamedTemporaryFile()
            shutil.copyfileobj(myfile.file, pdfdoc)
            pdfdoc.flush()

            doc=loadPDF(pdfdoc.name)
            text = doc2text(doc)
            response.headers['content-type'] = 'text/plain; charset=utf-8'
            return text.encode('utf-8')

        except:
            return ("Could not read pdf")
