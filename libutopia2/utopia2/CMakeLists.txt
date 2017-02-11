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

set(SOURCES
  aminoacid.cpp
  bus.cpp
  busagent.cpp
  certificateerrordialog.cpp
  configurable.cpp
  configuration.cpp
  element.cpp
  encryption.cpp
  extensionlibrary.cpp
  fileformat.cpp
  functional.cpp
  global.cpp
  initializer.cpp
  #invocation.cpp
  #invocationset.cpp
  library.cpp
  list.cpp
  localsocketbusagent.cpp
  networkaccessmanager.cpp
  node.cpp
  nucleotide.cpp
  ontology.cpp
  pacproxyfactory.cpp
  pacscript.cpp
  parser.cpp
  plugin.cpp
  pluginmanager.cpp
  property.cpp
  propertylist.cpp
  serializer.cpp
  threads.cpp
  variant.cpp
  )

include_directories(
  ${Boost_INCLUDE_DIR}
  ${utf8_INCLUDE_DIR}
  ${OPENSSL_INCLUDE_DIR}
  ${CMAKE_BINARY_DIR}
  )

if(WIN32)
  add_definitions(-DBUILD_LIBUTOPIA=1)
endif()

get_git_head_revision(UTOPIA_REVISION_BRANCH UTOPIA_REVISION_HASH)

string(REGEX REPLACE "^.*/([^/]+\\s*$).*" "\\1" UTOPIA_REVISION_ABBREV_BRANCH "${UTOPIA_REVISION_BRANCH}" )
string(SUBSTRING "${UTOPIA_REVISION_HASH}" 0 8 UTOPIA_REVISION_ABBREV_HASH)
configure_file("version_p.h.in" "${CMAKE_BINARY_DIR}/version_p.h" @ONLY IMMEDIATE)

add_utopia_library(${PROJECT_NAME} SHARED ${SOURCES})
target_link_libraries(${PROJECT_NAME} utf8 ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES})
qt5_use_modules(${PROJECT_NAME} Network Widgets WebKitWidgets Script)

if(UNIX AND NOT APPLE)
  target_link_libraries(utopia2 dl)
elseif(APPLE)
  find_library(SYSTEMCONFIGURATION_LIBRARY SystemConfiguration)
  find_library(COREFOUNDATION_LIBRARY CoreFoundation)
  target_link_libraries(utopia2 ${SYSTEMCONFIGURATION_LIBRARY} ${COREFOUNDATION_LIBRARY})
endif()

install_utopia_library(${PROJECT_NAME} "${COMPONENT}")

