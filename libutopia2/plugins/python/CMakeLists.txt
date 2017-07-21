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

# Ensure cmake-controlled Python packages are compiled
add_subdirectory(spine)
add_subdirectory(utopiabridge)

project(utopia2-python)

set(SOURCES
    conversion.cpp
    interpreter.cpp
    python.cpp)

include_directories(${PROJECT_BINARY_DIR}
                    ${athenaeum_INCLUDE_DIR}
                    ${papyro_INCLUDE_DIR}
                    ${spine_INCLUDE_DIR}
                    ${PYTHON_INCLUDE_PATH}
                    ${utopia2_qt_INCLUDE_DIR})

add_definitions("-DPYTHON_VERSION=\"${PYTHON_VERSION}\"")

if(UTOPIA_BUILD_DOCUMENTS)
  add_definitions(-DUTOPIA_BUILD_DOCUMENTS=1)
endif()
if(UTOPIA_BUILD_LIBRARY)
  add_definitions(-DUTOPIA_BUILD_LIBRARY=1)
endif()

SET(SWIG_RUNTIME "${PROJECT_BINARY_DIR}/swigruntime.h")
ADD_CUSTOM_COMMAND(OUTPUT ${SWIG_RUNTIME}
                   COMMAND ${SWIG_EXECUTABLE} -python -external-runtime ${SWIG_RUNTIME})

add_utopia_plugin(${PROJECT_NAME} MODULE ${SWIG_RUNTIME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} utopia2 utopia2_qt ${PYTHON_LIBRARY})

if(UTOPIA_BUILD_DOCUMENTS)
  target_link_libraries(${PROJECT_NAME} papyro spine ${Boost_LIBRARIES} utf8)
endif()

install_utopia_plugin(${PROJECT_NAME} ${COMPONENT})


###############################################################################
## Make sure that the coda_network replacement of urllib2 is correctly copied
## from the source directory into the build directory

file(GLOB_RECURSE CODA_PTH RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/" "${CMAKE_CURRENT_SOURCE_DIR}/coda_network.pth")
file(GLOB_RECURSE CODA_SCRIPTS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/" "${CMAKE_CURRENT_SOURCE_DIR}/coda_network/*.py")
foreach(SCRIPT ${CODA_PTH} ${CODA_SCRIPTS})
  configure_file(${SCRIPT} "${CMAKE_BINARY_DIR}/${PYTHON_INSTALL_DIR}/lib/python${PYTHON_VERSION}/site-packages/${SCRIPT}" COPYONLY)
endforeach()


###############################################################################
## Make sure that Python dependencies are correctly copied from the build
## environment into the build directory. On Linux, this isn't necessary, as
## those dependencies should already be install.

if(NOT OS_LINUX)
  file(GLOB_RECURSE PTH_SCRIPTS RELATIVE "${PYTHON_LIB_ROOT}/" "${PYTHON_LIB_ROOT}/*.pth")
  file(GLOB_RECURSE EGG_SCRIPTS RELATIVE "${PYTHON_LIB_ROOT}/" "${PYTHON_LIB_ROOT}/*.egg")
  file(GLOB_RECURSE PY_SCRIPTS RELATIVE "${PYTHON_LIB_ROOT}/" "${PYTHON_LIB_ROOT}/*.py")
  file(GLOB_RECURSE SO_SCRIPTS RELATIVE "${PYTHON_LIB_ROOT}/" "${PYTHON_LIB_ROOT}/*.so")
  file(GLOB_RECURSE DLL_SCRIPTS RELATIVE "${PYTHON_LIB_ROOT}" "${PYTHON_LIB_ROOT}/*.dll")
  file(GLOB_RECURSE PYD_SCRIPTS RELATIVE "${PYTHON_LIB_ROOT}" "${PYTHON_LIB_ROOT}/*.pyd")
  file(GLOB_RECURSE MAKEFILE RELATIVE "${PYTHON_LIB_ROOT}/" "${PYTHON_LIB_ROOT}/Makefile")

  foreach(SCRIPT ${EGG_SCRIPTS} ${PTH_SCRIPTS} ${PY_SCRIPTS} ${SO_SCRIPTS} ${MAKEFILE} ${PYD_SCRIPTS} ${DLL_SCRIPTS})
    configure_file("${PYTHON_LIB_ROOT}/${SCRIPT}" "${CMAKE_BINARY_DIR}/${PYTHON_INSTALL_DIR}/lib/python${PYTHON_VERSION}/${SCRIPT}" COPYONLY)
  endforeach()
endif()


###############################################################################
## Compile/install our Python packages

set(PYTHON_SITEPACKAGES_PATH "${CMAKE_BINARY_DIR}/${PYTHON_INSTALL_DIR}/lib/python${PYTHON_VERSION}/site-packages")
file(MAKE_DIRECTORY "${PYTHON_SITEPACKAGES_PATH}")
set(ENV{PYTHONPATH} "${PYTHON_SITEPACKAGES_PATH}")

set(PYPACKAGES
    authd
    utopia-core
    utopia-plugins-core)

if(NOT UTOPIA_SKIP_PUBLIC_PYTHON_PACKAGES)
  list(APPEND PYPACKAGES
    kend
    utopia
    utopia-plugins-common
    utopia-tools)
endif(NOT UTOPIA_SKIP_PUBLIC_PYTHON_PACKAGES)

if(NOT USE_SYSTEM_Python)
  set(EASY_INSTALL "${PYTHON_BIN_ROOT}/bin/easy_install")
  set(EASY_INSTALL_ARGS "")
elseif(WIN32)
  set(EASY_INSTALL "${PYTHON_BIN_ROOT}/easy_install.exe")
  set(EASY_INSTALL_ARGS "-N")
else()
  set(EASY_INSTALL "/usr/bin/easy_install")
  set(EASY_INSTALL_ARGS "-N")
endif()

foreach(PYPACKAGE ${PYPACKAGES})
  execute_process(COMMAND ${EASY_INSTALL} ${EASY_INSTALL_ARGS} --install-dir "${PYTHON_SITEPACKAGES_PATH}" . WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/python/${PYPACKAGE}/)
endforeach()

###############################################################################
## On Linux, make sure collected site-packages are installed. On Windows and
## Apple, the entire Python library needs to be installed.

if(OS_APPLE OR OS_WINDOWS)
  set(PYTHON_INSTALL_TARGET "lib/python${PYTHON_VERSION}")
elseif(OS_LINUX)
  set(PYTHON_INSTALL_TARGET "lib/python${PYTHON_VERSION}/site-packages")
endif()

install(DIRECTORY "${CMAKE_BINARY_DIR}/${PYTHON_INSTALL_DIR}/${PYTHON_INSTALL_TARGET}/"
        DESTINATION "${PYTHON_INSTALL_DIR}/${PYTHON_INSTALL_TARGET}"
        COMPONENT "${COMPONENT}"
        PATTERN "*.pyc" EXCLUDE
        PATTERN "*.pyo" EXCLUDE)
