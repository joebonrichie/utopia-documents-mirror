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

#
# Finds the Poppler Library
#
#  Poppler_FOUND                    True if Poppler found.
#  Poppler_INCLUDE_DIRS             Directory to include to get Poppler headers
#  Poppler_LIBRARIES                Libraries to link against for Poppler
#

include(FindPackageMessage)
include(CheckCXXSourceCompiles) 

# Look for the header file.
find_path(
  Poppler_INCLUDE_DIR
  NAMES "poppler/SplashOutputDev.h"
  DOC "Include directory for the Poppler library")
mark_as_advanced(Poppler_INCLUDE_DIR)

# Look for the Poppler library.
find_library(
  Poppler_LIBRARY
  NAMES poppler
  DOC "Library to link against for Poppler")
mark_as_advanced(Poppler_LIBRARY)

# Copy the results to the output variables.
if(Poppler_INCLUDE_DIR AND Poppler_LIBRARY)
  set(Poppler_FOUND 1)
  set(Poppler_LIBRARIES ${Poppler_LIBRARY})
  set(Poppler_INCLUDE_DIRS ${Poppler_INCLUDE_DIR})

#   CMAKE_REQUIRED_FLAGS = string of compile command line flags
#   CMAKE_REQUIRED_DEFINITIONS = list of macros to define (-DFOO=
  set(CMAKE_REQUIRED_INCLUDES  ${Poppler_INCLUDE_DIR} ${Poppler_INCLUDE_DIR}/poppler)
  set(CMAKE_REQUIRED_LIBRARIES ${Poppler_LIBRARY})

  check_cxx_source_compiles(
"
#include <poppler/SplashOutputDev.h>

typedef void (SplashOutputDev::*setVectorAntialiasFn)(GBool aa);

int main(int argc, char** argv)
{
  setVectorAntialiasFn fn= &SplashOutputDev::setVectorAntialias;
  return 1;
}
" HAVE_POPPLER_SPLASH_SET_VECTOR_ANTIALIAS)

  check_cxx_source_compiles(
"
#include <poppler/SplashOutputDev.h>

typedef void (SplashOutputDev::*setFontAntialiasFn)(GBool aa);

int main(int argc, char** argv)
{
  setFontAntialiasFn fn= &SplashOutputDev::setFontAntialias;
  return 1;
}
" HAVE_POPPLER_SPLASH_SET_FONT_ANTIALIAS)

else()
  set(Poppler_FOUND 0)
  set(Poppler_LIBRARIES)
  set(Poppler_INCLUDE_DIRS)
endif()



include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Poppler  DEFAULT_MSG  Poppler_LIBRARIES  Poppler_INCLUDE_DIRS)
