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
# Finds the GLEW Library
#
#  GLEW_FOUND                    True if GLEW found.
#  GLEW_INCLUDE_DIRS             Directory to include to get GLEW headers
#  GLEW_LIBRARIES                Libraries to link against for GLEW
#

include(FindPackageMessage)

# Look for the header file.
find_path(
  GLEW_INCLUDE_DIR
  NAMES "GL/glew.h"
  DOC "Include directory for the GLEW library")
mark_as_advanced(GLEW_INCLUDE_DIR)

# Look for the GLEW library.
find_library(
  GLEW_LIBRARY
  NAMES GLEW GLEW32
  DOC "Library to link against for GLEW")
mark_as_advanced(GLEW_LIBRARY)

# Copy the results to the output variables.
if(GLEW_INCLUDE_DIR AND GLEW_LIBRARY)
  set(GLEW_FOUND 1)
  set(GLEW_LIBRARIES ${GLEW_LIBRARY})
  set(GLEW_INCLUDE_DIRS ${GLEW_INCLUDE_DIR})
else()
  set(GLEW_FOUND 0)
  set(GLEW_LIBRARIES)
  set(GLEW_INCLUDE_DIRS)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLEW  DEFAULT_MSG  GLEW_LIBRARIES  GLEW_INCLUDE_DIRS)
