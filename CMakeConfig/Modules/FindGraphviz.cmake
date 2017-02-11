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
# Finds the Graphviz Library
#
#  Graphviz_FOUND                    True if Graphviz found.
#  Graphviz_INCLUDE_DIRS             Directory to include to get Graphviz headers
#  Graphviz_LIBRARIES                Libraries to link against for Graphviz
#

include(FindPackageMessage)

# Look for the header file.
find_path(
  Graphviz_INCLUDE_DIR
  NAMES graphviz/gvc.h
  DOC "Include directory for the Graphviz library")
mark_as_advanced(Graphviz_INCLUDE_DIR)

# Look for the dot library.
find_library(
  Graphviz_DOT_LIBRARY
  NAMES gvplugin_dot_layout
  PATH_SUFFIXES graphviz 
  DOC "Library to link against for the dot layout part of Graphviz")
mark_as_advanced(Graphviz_DOT_LIBRARY)

# Look for the graph library.
find_library(
  Graphviz_GRAPH_LIBRARY
  NAMES graph
  DOC "Library to link against for the graph part of Graphviz")
mark_as_advanced(Graphviz_GRAPH_LIBRARY)

# Look for the gvc library.
find_library(
  Graphviz_GVC_LIBRARY
  NAMES gvc
  DOC "Library to link against for the gvc part of Graphviz")
mark_as_advanced(Graphviz_GVC_LIBRARY)

# Look for the cdt library.
find_library(
  Graphviz_CDT_LIBRARY
  NAMES cdt
  DOC "Library to link against for the cdt part of Graphviz")
mark_as_advanced(Graphviz_CDT_LIBRARY)

# Look for the pathplan library.
find_library(
  Graphviz_PATHPLAN_LIBRARY
  NAMES pathplan
  DOC "Library to link against for the pathplan part of Graphviz")
mark_as_advanced(Graphviz_PATHPLAN_LIBRARY)

# Copy the results to the output variables.
if(Graphviz_INCLUDE_DIR
    AND Graphviz_DOT_LIBRARY
    AND Graphviz_GRAPH_LIBRARY
    AND Graphviz_GVC_LIBRARY
    AND Graphviz_CDT_LIBRARY
    AND Graphviz_PATHPLAN_LIBRARY)
  set(Graphviz_FOUND 1)
  set(Graphviz_LIBRARIES ${Graphviz_DOT_LIBRARY} ${Graphviz_GVC_LIBRARY} ${Graphviz_PATHPLAN_LIBRARY} ${Graphviz_GRAPH_LIBRARY} ${Graphviz_CDT_LIBRARY})
  set(Graphviz_INCLUDE_DIRS ${Graphviz_INCLUDE_DIR})
else()
  set(Graphviz_FOUND 0)
  set(Graphviz_LIBRARIES)
  set(Graphviz_INCLUDE_DIRS)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Graphviz  DEFAULT_MSG  Graphviz_LIBRARIES  Graphviz_INCLUDE_DIRS)
