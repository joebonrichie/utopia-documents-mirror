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
# $Id$
#
# - Find pcre
# Find the native PCRE includes and library
#
#  PCRE_INCLUDE_DIRS - where to find pcre.h, etc.
#  PCRE_LIBRARIES    - List of libraries when using pcre.
#  PCRE_FOUND        - True if pcre found.


IF (PCRE_INCLUDE_DIRS)
  # Already in cache, be silent
  SET(PCRE_FIND_QUIETLY TRUE)
ENDIF (PCRE_INCLUDE_DIRS)

FIND_PATH(PCRE_INCLUDE_DIR pcrecpp.h)

SET(PCRE_NAMES pcre)
FIND_LIBRARY(PCRE_LIBRARY NAMES ${PCRE_NAMES} )

SET(PCRECPP_NAMES pcrecpp)
FIND_LIBRARY(PCRECPP_LIBRARY NAMES ${PCRECPP_NAMES} )

# handle the QUIETLY and REQUIRED arguments and set PCRE_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PCRE DEFAULT_MSG PCRE_LIBRARY PCRECPP_LIBRARY PCRE_INCLUDE_DIR)

IF(PCRE_FOUND)
  SET( PCRE_LIBRARIES ${PCRECPP_LIBRARY} ${PCRE_LIBRARY} )
  SET( PCRE_INCLUDE_DIRS ${PCRE_INCLUDE_DIR} )
ELSE(PCRE_FOUND)
  SET( PCRE_LIBRARIES )
  SET( PCRE_INCLUDE_DIRS )
ENDIF(PCRE_FOUND)

MARK_AS_ADVANCED( PCRE_LIBRARIES PCRE_INCLUDE_DIRS )
