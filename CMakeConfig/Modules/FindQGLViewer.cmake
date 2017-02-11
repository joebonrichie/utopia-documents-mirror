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
# Finds the QGLViewer Library
#
#  QGLViewer_FOUND                    True if QGLViewer found.
#  QGLViewer_INCLUDE_DIRS             Directory to include to get QGLViewer headers
#  QGLViewer_LIBRARIES                Libraries to link against for QGLViewer
#

include(FindPackageMessage)

# Look for the header file.
find_path(
  QGLViewer_INCLUDE_DIR
  NAMES qglviewer.h
  PATH_SUFFIXES QGLViewer
  DOC "Include directory for the QGLViewer library")
mark_as_advanced(QGLViewer_INCLUDE_DIR)

# Look for the library.
find_library(
  QGLViewer_LIBRARY_RELEASE
  NAMES QGLViewer QGLViewer2 qglviewer-qt4
  DOC "Library to link against for QGLViewer")
mark_as_advanced(QGLViewer_LIBRARY_RELEASE)
if(WIN32)
  set(QGLViewer "QGLViewerd2")
elseif(APPLE)
  set(QGLViewer "QGLViewer_debug")
else()
  SET(QGLViewer "qglviewer-qt4")
endif()
find_library(
  QGLViewer_LIBRARY_DEBUG
  NAMES "${QGLViewer}"
  DOC "Library to link against for QGLViewer")
mark_as_advanced(QGLViewer_LIBRARY_DEBUG)

if (QGLViewer_LIBRARY_RELEASE AND NOT QGLViewer_LIBRARY_DEBUG)
  set(QGLViewer_LIBRARY_DEBUG ${QGLViewer_LIBRARY_RELEASE})
  set(QGLViewer_LIBRARY ${QGLViewer_LIBRARY_RELEASE})
  set(QGLViewer_LIBRARIES ${QGLViewer_LIBRARY_RELEASE})
endif()

if (QGLViewer_LIBRARY_DEBUG AND NOT QGLViewer_LIBRARY_RELEASE)
  set(QGLViewer_LIBRARY_RELEASE ${QGLViewer_LIBRARY_DEBUG})
  set(QGLViewer_LIBRARY ${QGLViewer_LIBRARY_DEBUG})
  set(QGLViewer_LIBRARIES ${QGLViewer_LIBRARY_DEBUG})
endif()

if (QGLViewer_INCLUDE_DIR AND QGLViewer_LIBRARY_DEBUG AND QGLViewer_LIBRARY_RELEASE)
  set(QGLViewer_FOUND 1)
  if(CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)
    set(QGLViewer_LIBRARY optimized ${QGLViewer_LIBRARY_RELEASE} debug ${QGLViewer_LIBRARY_DEBUG})
  else(CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)
    set(QGLViewer_LIBRARY ${QGLViewer_LIBRARY_RELEASE})
  endif(CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)
  set(QGLViewer_LIBRARIES optimized ${QGLViewer_LIBRARY_RELEASE} debug ${QGLViewer_LIBRARY_DEBUG})
  set(QGLViewer_INCLUDE_DIRS ${QGLViewer_INCLUDE_DIR})
else()
  set(QGLViewer_FOUND 0)
  set(QGLViewer_LIBRARY)
  set(QGLViewer_LIBRARIES)
  set(QGLViewer_INCLUDE_DIRS)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(QGLViewer  DEFAULT_MSG  QGLViewer_LIBRARIES  QGLViewer_INCLUDE_DIRS)
