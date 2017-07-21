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

import random
import string
import utopia.extension



class PhraseLookup(utopia.extension.Extension):
	pass

class Visualiser(utopia.extension.Extension):
    def cssId(self, size = 6, chars = string.ascii_lowercase + string.digits, initial_chars = string.ascii_lowercase):
        return random.choice(initial_chars) + ''.join(random.choice(chars) for x in range(size - 1))

class Decorator(utopia.extension.Extension):
	pass

class LinkFinder(utopia.extension.Extension):
	pass

class OverlayRendererMapper(utopia.extension.Extension):
    pass

class Annotator(utopia.extension.Extension):
	pass

class TextAnnotator(Annotator):
	pass

class AreaAnnotator(Annotator):
	pass

class MixedAnnotator(Annotator):
	pass




class _BufferedDocumentWrapper:
    def __init__(self, document):
        self.__dict__['_document'] = document
        self.__dict__['_annotations'] = []
    def flush(self, test = False):
        if not test or reduce(lambda t, l: t + len(l[1]), self._annotations, 0) > 50:
            for scratch, annotations in self._annotations:
                self._document.addAnnotations(annotations, scratch)
            self.__dict__['_annotations'] = []
    def addAnnotation(self, annotation, scratch = None):
        if len(self._annotations) > 0 and self._annotations[-1][0] == scratch:
            self._annotations[-1][1].append(annotation)
        else:
            self._annotations.append((scratch, [annotation]))
        self.flush(test=True)
    def addAnnotations(self, annotations, scratch = None):
        if len(self._annotations) > 0 and self._annotations[-1][0] == scratch:
            self._annotations[-1][1] += annotations
        else:
            self._annotations.append((scratch, annotations))
        self.flush(test=True)
    def __getattr__(self, key):
        return getattr(self._document, key)
    def __setattr__(self, key, value):
        setattr(self._document, key, value)

def buffer(fn):
    def wrapper(self, document, *args, **kwargs):
        document = _BufferedDocumentWrapper(document)
        try:
            result = fn(self, document, *args, **kwargs)
        finally:
            document.flush()
        return result
    return wrapper



