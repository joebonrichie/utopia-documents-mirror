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

INCLUDE_DIRECTORIES(
    ${PYTHON_INCLUDE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_BINARY_DIR}
    ${utopia2_INCLUDE_DIR}
    ${Boost_INCLUDE_DIR}
    ${utf8_INCLUDE_DIR}
    ${CMAKE_BINARY_DIR}
    )

file(GLOB SCRIPTS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.py)

foreach(SCRIPT ${SCRIPTS})
  configure_file(${SCRIPT} "${CMAKE_BINARY_DIR}/${PYTHON_INSTALL_DIR}/lib/python${PYTHON_VERSION}/site-packages/utopia/${SCRIPT}" COPYONLY)
  install(FILES "${CMAKE_BINARY_DIR}/${PYTHON_INSTALL_DIR}/lib/python${PYTHON_VERSION}/site-packages/utopia/${SCRIPT}" DESTINATION "${PYTHON_INSTALL_DIR}/lib/python${PYTHON_VERSION}/site-packages/utopia" COMPONENT "${COMPONENT}")
endforeach()

if(FALSE)
  set(PYTHON_BIN "${PYTHON_BIN_ROOT}/python.exe")
  configure_file("${PYTHON_BIN}" "${CMAKE_BINARY_DIR}/${BIN_INSTALL_DIR}/python.exe" COPYONLY)

  file(GLOB_RECURSE PY_SCRIPTS RELATIVE "${PYTHON_LIB_ROOT}" "${PYTHON_LIB_ROOT}/*.py")
  file(GLOB_RECURSE PYD_SCRIPTS RELATIVE "${PYTHON_LIB_ROOT}" "${PYTHON_LIB_ROOT}/*.pyd")
  foreach(SCRIPT ${PY_SCRIPTS} ${PYD_SCRIPTS})
    if(NOT "${SCRIPT}" MATCHES ".*ctypes.*")
      configure_file("${PYTHON_LIB_ROOT}/${SCRIPT}" "${CMAKE_BINARY_DIR}/${PYTHON_INSTALL_DIR}/lib/python${PYTHON_VERSION}/${SCRIPT}" COPYONLY)
    endif()
  endforeach()

  set(PYTHON_DLL_ROOT "c:/mingw64/lib/python${PYTHON_VERSION}/DLLs")
  file(GLOB_RECURSE DLL_SCRIPTS RELATIVE "${PYTHON_DLL_ROOT}" "${PYTHON_DLL_ROOT}/*.dll")
  file(GLOB_RECURSE PYD_SCRIPTS RELATIVE "${PYTHON_DLL_ROOT}" "${PYTHON_DLL_ROOT}/*.pyd")
  foreach(SCRIPT ${PYD_SCRIPTS} ${DLL_SCRIPTS})
    if(NOT "${SCRIPT}" MATCHES ".*ctypes.*")
      configure_file("${PYTHON_DLL_ROOT}/${SCRIPT}" "${CMAKE_BINARY_DIR}/${PYTHON_INSTALL_DIR}/dlls/${SCRIPT}" COPYONLY)
    endif()
  endforeach()

  install(DIRECTORY "${CMAKE_BINARY_DIR}/${PYTHON_INSTALL_DIR}/lib" DESTINATION "${PYTHON_INSTALL_DIR}" COMPONENT "${COMPONENT}"
          PATTERN "*.pyc" EXCLUDE
          PATTERN "*.pyo" EXCLUDE)
  install(DIRECTORY "${CMAKE_BINARY_DIR}/${PYTHON_INSTALL_DIR}/dlls" DESTINATION "${PYTHON_INSTALL_DIR}" COMPONENT "${COMPONENT}"
          PATTERN "*.pyc" EXCLUDE
          PATTERN "*.pyo" EXCLUDE)

elseif(NOT USE_SYSTEM_Python OR WIN32)
  if(WIN32)
    #configure_file("${PYTHON_BIN_ROOT}/bin/python.exe" "${CMAKE_BINARY_DIR}/${BIN_INSTALL_DIR}/python.exe" COPYONLY)
  else()
    #configure_file("${PYTHON_BIN_ROOT}/bin/python" "${CMAKE_BINARY_DIR}/${BIN_INSTALL_DIR}/python" COPYONLY)
  endif()

  file(GLOB_RECURSE PY_SCRIPTS RELATIVE "${PYTHON_LIB_ROOT}/" "${PYTHON_LIB_ROOT}/*.py")
  file(GLOB_RECURSE SO_SCRIPTS RELATIVE "${PYTHON_LIB_ROOT}/" "${PYTHON_LIB_ROOT}/*.so")
  file(GLOB_RECURSE DLL_SCRIPTS RELATIVE "${PYTHON_LIB_ROOT}" "${PYTHON_LIB_ROOT}/*.dll")
  file(GLOB_RECURSE PYD_SCRIPTS RELATIVE "${PYTHON_LIB_ROOT}" "${PYTHON_LIB_ROOT}/*.pyd")
  file(GLOB_RECURSE MAKEFILE RELATIVE "${PYTHON_LIB_ROOT}/" "${PYTHON_LIB_ROOT}/Makefile")

  foreach(SCRIPT ${PY_SCRIPTS} ${SO_SCRIPTS} ${MAKEFILE} ${PYD_SCRIPTS} ${DLL_SCRIPTS})
    configure_file("${PYTHON_LIB_ROOT}/${SCRIPT}" "${CMAKE_BINARY_DIR}/${PYTHON_INSTALL_DIR}/lib/python${PYTHON_VERSION}/${SCRIPT}" COPYONLY)
  endforeach()

  install(DIRECTORY "${CMAKE_BINARY_DIR}/${PYTHON_INSTALL_DIR}/lib/python${PYTHON_VERSION}/" DESTINATION "${PYTHON_INSTALL_DIR}/lib/python${PYTHON_VERSION}" COMPONENT "${COMPONENT}"
        PATTERN "*.pyc" EXCLUDE
        PATTERN "*.pyo" EXCLUDE)

  #file(GLOB_RECURSE PYCONFIG RELATIVE "${PYTHON_INCLUDE_ROOT}" "${PYTHON_INCLUDE_ROOT}/pyconfig.h")
  #foreach(SCRIPT ${PYCONFIG})
  #  configure_file("${PYTHON_INCLUDE_ROOT}/${SCRIPT}" "${CMAKE_BINARY_DIR}/${PYTHON_INSTALL_DIR}/include/python${PYTHON_VERSION}/${SCRIPT}" COPYONLY)
  #endforeach()
  #install(DIRECTORY "${CMAKE_BINARY_DIR}/${PYTHON_INSTALL_DIR}/include/python${PYTHON_VERSION}" DESTINATION "${PYTHON_INSTALL_DIR}/include/python${PYTHON_VERSION}" COMPONENT "${COMPONENT}")

endif()



