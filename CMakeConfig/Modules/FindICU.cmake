###############################################################################
#   
#    This file is part of the Utopia Documents application.
#        Copyright (c) 2008-2014 Lost Island Labs
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
# Finds the International Components for Unicode (ICU) Library
#
#  ICU_FOUND                    True if ICU found.
#  ICU_INCLUDE_DIRS             Directory to include to get ICU headers
#  ICU_LIBRARIES                Libraries to link against for ICU
#  ICU_USE_STATIC_LIBS          Can be set to ON to force the use of the static
#                               icu libraries. Defaults to OFF.
#

include(FindPackageMessage)

if(ICU_USE_STATIC_LIBS)

  # Look for the header file.
  find_path(
    ICU_INCLUDE_DIR
    NAMES unicode/utypes.h
    DOC "Include directory for the ICU library")
  mark_as_advanced(ICU_INCLUDE_DIR)

  # Look for the mainlibrary.
  find_library(
    ICU_UC_LIBRARY
    NAMES sicuuc
    DOC "Library to link against for the common parts of ICU")
  mark_as_advanced(ICU_UC_LIBRARY)

  # Look for the io library.
  find_library(
    ICU_IO_LIBRARY
    NAMES sicuio
    DOC "Library to link against for the stream IO parts of ICU")
  mark_as_advanced(ICU_IO_LIBRARY)

  # Look for the io library.
  find_library(
    ICU_I18N_LIBRARY
    NAMES sicui18n
    DOC "Library to link against for the internationalisation parts of ICU")
  mark_as_advanced(ICU_I18N_LIBRARY)

  # Look for the data library.
  find_library(
    ICU_DATA_LIBRARY
    NAMES sicudata
    DOC "Library to link against for the data parts of ICU")
  mark_as_advanced(ICU_DATA_LIBRARY)

else()

  # Look for the header file.
  find_path(
    ICU_INCLUDE_DIR
    NAMES unicode/utypes.h
    DOC "Include directory for the ICU library")
  mark_as_advanced(ICU_INCLUDE_DIR)

  # Look for the mainlibrary.
  find_library(
    ICU_UC_LIBRARY
    NAMES icuuc
    DOC "Library to link against for the common parts of ICU")
  mark_as_advanced(ICU_UC_LIBRARY)

  # Look for the io library.
  find_library(
    ICU_IO_LIBRARY
    NAMES icuio
    DOC "Library to link against for the stream IO parts of ICU")
  mark_as_advanced(ICU_IO_LIBRARY)

  # Look for the io library.
  find_library(
    ICU_I18N_LIBRARY
    NAMES icui18n icuin
    DOC "Library to link against for the internationalisation parts of ICU")
  mark_as_advanced(ICU_I18N_LIBRARY)

  # Look for the data library.
  find_library(
    ICU_DATA_LIBRARY
    NAMES icudata icudt
    DOC "Library to link against for the data parts of ICU")
  mark_as_advanced(ICU_DATA_LIBRARY)

endif()

# Copy the results to the output variables.
if(ICU_INCLUDE_DIR AND ICU_UC_LIBRARY AND ICU_IO_LIBRARY AND ICU_DATA_LIBRARY)
  set(ICU_FOUND 1)
  set(ICU_LIBRARIES ${ICU_IO_LIBRARY} ${ICU_I18N_LIBRARY} ${ICU_UC_LIBRARY} ${ICU_DATA_LIBRARY})
  set(ICU_INCLUDE_DIRS ${ICU_INCLUDE_DIR})

  if(ICU_USE_STATIC_LIBS)
    add_definitions(-DU_STATIC_IMPLEMENTATION=1)
  endif()
else()
  set(ICU_FOUND 0)
  set(ICU_LIBRARIES)
  set(ICU_INCLUDE_DIRS)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ICU  DEFAULT_MSG  ICU_LIBRARIES  ICU_INCLUDE_DIRS)
