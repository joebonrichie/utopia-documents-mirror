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
# Finds the FontConfig Library
#
#  FontConfig_FOUND                    True if FontConfig found.
#  FontConfig_INCLUDE_DIRS             Directory to include to get FontConfig headers
#  FontConfig_LIBRARIES                Libraries to link against for FontConfig
#

include(FindPackageMessage)

# Look for the header file.
find_path(
  FontConfig_INCLUDE_DIR
  NAMES "fontconfig/fontconfig.h"
  DOC "Include directory for the FontConfig library")
mark_as_advanced(FontConfig_INCLUDE_DIR)

# Look for the FontConfig library.
find_library(
  FontConfig_LIBRARY
  NAMES fontconfig
  DOC "Library to link against for FontConfig")
mark_as_advanced(FontConfig_LIBRARY)

# Copy the results to the output variables.
if(FontConfig_INCLUDE_DIR AND FontConfig_LIBRARY)
  set(FontConfig_FOUND 1)
  set(FontConfig_LIBRARIES ${FontConfig_LIBRARY})
  set(FontConfig_INCLUDE_DIRS ${FontConfig_INCLUDE_DIR})
else()
  set(FontConfig_FOUND 0)
  set(FontConfig_LIBRARIES)
  set(FontConfig_INCLUDE_DIRS)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FontConfig  DEFAULT_MSG  FontConfig_LIBRARIES  FontConfig_INCLUDE_DIRS)
