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

# Find QJSON - JSON handling library for Qt
#
# This module defines
#  QJSON_FOUND - whether the qsjon library was found
#  QJSON_LIBRARIES - the qjson library
#  QJSON_INCLUDE_DIR - the include path of the qjson library
#

if (QJSON_INCLUDE_DIR AND QJSON_LIBRARIES)

  # Already in cache
  set (QJSON_FOUND TRUE)

else (QJSON_INCLUDE_DIR AND QJSON_LIBRARIES)

  find_library (QJSON_LIBRARIES
    NAMES
    qjson
    PATHS
    ${QJSON_LIBRARY_DIRS}
    ${LIB_INSTALL_DIR}
    ${KDE4_LIB_DIR}
  )

  find_path (QJSON_INCLUDE_DIR
    NAMES
    parser.h
    PATH_SUFFIXES
    qjson
    PATHS
    ${QJSON_INCLUDE_DIRS}
    ${INCLUDE_INSTALL_DIR}
    ${KDE4_INCLUDE_DIR}
  )

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(QJSON DEFAULT_MSG QJSON_LIBRARIES QJSON_INCLUDE_DIR)

endif (QJSON_INCLUDE_DIR AND QJSON_LIBRARIES)
