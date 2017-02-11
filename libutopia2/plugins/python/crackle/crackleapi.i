/*****************************************************************************
 *  
 *   This file is part of the Utopia Documents application.
 *       Copyright (c) 2008-2017 Lost Island Labs
 *           <info@utopiadocs.com>
 *   
 *   Utopia Documents is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU GENERAL PUBLIC LICENSE VERSION 3 as
 *   published by the Free Software Foundation.
 *   
 *   Utopia Documents is distributed in the hope that it will be useful, but
 *   WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 *   Public License for more details.
 *   
 *   In addition, as a special exception, the copyright holders give
 *   permission to link the code of portions of this program with the OpenSSL
 *   library under certain conditions as described in each individual source
 *   file, and distribute linked combinations including the two.
 *   
 *   You must obey the GNU General Public License in all respects for all of
 *   the code used other than OpenSSL. If you modify file(s) with this
 *   exception, you may extend this exception to your version of the file(s),
 *   but you are not obligated to do so. If you do not wish to do so, delete
 *   this exception statement from your version.
 *   
 *   You should have received a copy of the GNU General Public License
 *   along with Utopia Documents. If not, see <http://www.gnu.org/licenses/>
 *  
 *****************************************************************************/

/* Swig Python interface to spine documents */
%feature("autodoc", "0");
%module (docstring="Utopia Documents Crackle API") crackleapi

%{

    //#include <spine/spineapi.h>
#include <crackle/crackleapi.h>
#include <spine/pyspineapi.h>

  static int check_exception(SpineError e) {
    switch (e) {
    case SpineError_NoError:
      break;
    case SpineError_IO:
      PyErr_SetString(PyExc_IOError,"Document cannot be loaded");
      return 1;
    case SpineError_InvalidType:
      PyErr_SetString(PyExc_TypeError,"Invalid object or object in invalid state");
      return 1;
    case SpineError_InvalidArgument:
      PyErr_SetString(PyExc_ValueError,"Invalid argument");
      return 1;
    case SpineError_InvalidRegex:
      PyErr_SetString(PyExc_TypeError,"Invalid regular expression");
      return 1;
    case SpineError_Unknown:
      PyErr_SetString(PyExc_RuntimeError,"Unknown error accessing document");
      return 1;
    }
    return 0;
  }

struct Document loadPDF(const char *filename_) {
    struct Document d;
    d._err=SpineError_NoError;
    d._doc=new_CrackleDocument(filename_, &d._err);
    return d;
 }

struct Document loadPDFFromBuffer(const char *buffer_, size_t size_) {
    struct Document d;
    d._err=SpineError_NoError;
    d._doc=new_CrackleDocumentFromBuffer(buffer_, size_, &d._err);
    return d;
 }

%}


%exception loadPDF {
  $action
    if(check_exception(result._err)) {
        return NULL;
    }
 }

struct Document loadPDF(const char *filename_);
struct Document loadPDFFromBuffer(const char *buffer_, size_t size_);

%include "crackleapi_python.py"
