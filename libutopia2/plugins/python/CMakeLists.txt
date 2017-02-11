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


add_subdirectory( scripts )
add_subdirectory( spine )
add_subdirectory( utopia )
add_subdirectory( utopiabridge )
add_subdirectory( utopialib )

project(utopia2-python)

set(SOURCES
    conversion.cpp
    interpreter.cpp
    python.cpp
    )

include_directories(
  ${PROJECT_BINARY_DIR}
  ${athenaeum_INCLUDE_DIR}
  ${papyro_INCLUDE_DIR}
  ${spine_INCLUDE_DIR}
  ${PYTHON_INCLUDE_PATH}
  ${utopia2_qt_INCLUDE_DIR}
  )

if(NOT USE_SYSTEM_Python OR WIN32)
  add_definitions(-DREMAP_BUNDLED_PYTHON=1)
endif()
add_definitions("-DPYTHON_VERSION=\"${PYTHON_VERSION}\"")

if(UTOPIA_BUILD_DOCUMENTS)
  add_definitions(-DUTOPIA_BUILD_DOCUMENTS=1)
endif()
if(UTOPIA_BUILD_LIBRARY)
  add_definitions(-DUTOPIA_BUILD_LIBRARY=1)
endif()

SET(SWIG_RUNTIME "${PROJECT_BINARY_DIR}/swigruntime.h")
ADD_CUSTOM_COMMAND(
    OUTPUT ${SWIG_RUNTIME}
    COMMAND ${SWIG_EXECUTABLE} -python -external-runtime ${SWIG_RUNTIME}
    )

file(GLOB_RECURSE KEND_SCRIPTS RELATIVE "${CMAKE_SOURCE_DIR}/server/kend/0.7/kend/" "${CMAKE_SOURCE_DIR}/server/kend/0.7/kend/kend/*.py")
foreach(SCRIPT ${KEND_SCRIPTS})
  configure_file(${CMAKE_SOURCE_DIR}/server/kend/0.7/kend/${SCRIPT} "${CMAKE_BINARY_DIR}/${PYTHON_INSTALL_DIR}/lib/python${PYTHON_VERSION}/site-packages/${SCRIPT}" COPYONLY)
endforeach()
file(GLOB_RECURSE AUTHD_SCRIPTS RELATIVE "${CMAKE_SOURCE_DIR}/server/authd/0.3/authd/" "${CMAKE_SOURCE_DIR}/server/authd/0.3/authd/authd/*.py")
foreach(SCRIPT ${AUTHD_SCRIPTS})
  configure_file(${CMAKE_SOURCE_DIR}/server/authd/0.3/authd/${SCRIPT} "${CMAKE_BINARY_DIR}/${PYTHON_INSTALL_DIR}/lib/python${PYTHON_VERSION}/site-packages/${SCRIPT}" COPYONLY)
endforeach()
file(GLOB_RECURSE CODA_SCRIPTS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/" "${CMAKE_CURRENT_SOURCE_DIR}/coda_network/*.py")
foreach(SCRIPT ${CODA_SCRIPTS})
  configure_file(${SCRIPT} "${CMAKE_BINARY_DIR}/${PYTHON_INSTALL_DIR}/lib/python${PYTHON_VERSION}/site-packages/${SCRIPT}" COPYONLY)
endforeach()
install(DIRECTORY "${CMAKE_BINARY_DIR}/${PYTHON_INSTALL_DIR}/lib/python${PYTHON_VERSION}/site-packages/kend"
        DESTINATION "${PYTHON_INSTALL_DIR}/lib/python${PYTHON_VERSION}/site-packages/"
        COMPONENT "${COMPONENT}"
        PATTERN "*.pyc" EXCLUDE
        PATTERN "*.pyo" EXCLUDE)
install(DIRECTORY "${CMAKE_BINARY_DIR}/${PYTHON_INSTALL_DIR}/lib/python${PYTHON_VERSION}/site-packages/coda_network"
        DESTINATION "${PYTHON_INSTALL_DIR}/lib/python${PYTHON_VERSION}/site-packages/"
        COMPONENT "${COMPONENT}"
        PATTERN "*.pyc" EXCLUDE
        PATTERN "*.pyo" EXCLUDE)

add_utopia_plugin(${PROJECT_NAME} MODULE ${SWIG_RUNTIME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} utopia2 utopia2_qt ${PYTHON_LIBRARY})

if(UTOPIA_BUILD_DOCUMENTS)
  target_link_libraries(${PROJECT_NAME} papyro spine ${Boost_LIBRARIES} utf8)
endif()

install_utopia_plugin(${PROJECT_NAME} ${COMPONENT})
