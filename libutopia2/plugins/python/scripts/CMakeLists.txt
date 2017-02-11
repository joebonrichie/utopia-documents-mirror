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

project(utopia2-python)

file(GLOB SCRIPTS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} core/*.py)
file(GLOB SCRIPTS_3RDPARTY RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 3rdparty/*.py)

file(GLOB SCRIPTZIPS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} core/*.zip)
file(GLOB SCRIPTZIPS_3RDPARTY RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 3rdparty/*.zip)

foreach(SCRIPT ${SCRIPTS} ${SCRIPTS_3RDPARTY})
  configure_file(${SCRIPT} "${CMAKE_BINARY_DIR}/${PLUGIN_INSTALL_DIR}/python/${SCRIPT}" COPYONLY)
  string(REGEX REPLACE "/[^/]*" "" SCRIPT_DIR ${SCRIPT})
  install(FILES "${CMAKE_BINARY_DIR}/${PLUGIN_INSTALL_DIR}/python/${SCRIPT}" DESTINATION "${PLUGIN_INSTALL_DIR}/python/${SCRIPT_DIR}" COMPONENT "${COMPONENT}")
endforeach()

foreach(SCRIPTZIP ${SCRIPTZIPS} ${SCRIPTZIPS_3RDPARTY})
  string(REGEX REPLACE "/[^/]*" "" SCRIPTZIP_DIR ${SCRIPTZIP})

  execute_process(
    COMMAND mkdir -p "${CMAKE_CURRENT_BINARY_DIR}/${SCRIPTZIP_DIR}"
  )
  execute_process(
    COMMAND zip -vr "${CMAKE_CURRENT_BINARY_DIR}/${SCRIPTZIP}" .
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${SCRIPTZIP}"
    OUTPUT_QUIET
  )
  configure_file("${CMAKE_CURRENT_BINARY_DIR}/${SCRIPTZIP}" "${CMAKE_BINARY_DIR}/${PLUGIN_INSTALL_DIR}/python/${SCRIPTZIP}" COPYONLY)

  install(FILES "${CMAKE_BINARY_DIR}/${PLUGIN_INSTALL_DIR}/python/${SCRIPTZIP}" DESTINATION "${PLUGIN_INSTALL_DIR}/python/${SCRIPTZIP_DIR}" COMPONENT "${COMPONENT}")
endforeach()

