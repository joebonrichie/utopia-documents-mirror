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

cmake_minimum_required (VERSION 2.6)

project ( DEPENDENCIES )

set(DEFINES "")
set(LOCALS "")
set(ENVS "")

if(APPLE)
    if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        set(CMAKE_OSX_ARCHITECTURES "i386;x86_64"
            CACHE STRING "Build architectures for OSX" FORCE)
        set(CMAKE_OSX_SYSROOT /Developer/SDKs/MacOSX10.6.sdk
            CACHE STRING "isysroot used for universal binary support" FORCE)
        set(CMAKE_OSX_DEPLOYMENT_TARGET 10.6
            CACHE STRING "-mmacosx-version-min flag used for universal binary support" FORCE)
    endif()
endif()

foreach(CMAKE_OSX_ARCHITECTURE ${CMAKE_OSX_ARCHITECTURES})
  set(CMAKE_OSX_FLAGS "${CMAKE_OSX_ARCHITECTURE}" "${CMAKE_OSX_FLAGS}")
endforeach()

file(GLOB DEPS RELATIVE "${PROJECT_SOURCE_DIR}" "build_*[^~]")

foreach(SCRIPT ${DEPS})

  string(REGEX REPLACE "build_"  "" PKG "${SCRIPT}")

  OPTION(USE_SYSTEM_${PKG} "Use system version of ${PKG}" OFF)

  set(LOCALS "${LOCALS}set(USE_SYSTEM_${PKG} ${USE_SYSTEM_${PKG}})\n")

  set(ENVS "${ENVS}USE_SYSTEM_${PKG}=\"${USE_SYSTEM_${PKG}}\"\n")

  if(USE_SYSTEM_${PKG})
    message(STATUS "Using system version of dependency script: ${PKG}")
  else()
    add_custom_command(OUTPUT ".${PKG}-BUILT"
      DEPENDS "build_${PKG}"
      COMMAND "${PROJECT_SOURCE_DIR}/build_${PKG}"
              "${CMAKE_OSX_DEPLOYMENT_TARGET}"
              "${CMAKE_OSX_SYSROOT}"
              ${CMAKE_OSX_FLAGS}
      COMMENT "Building ${PKG}"
      )
    add_custom_target("${PKG}" ALL DEPENDS ".${PKG}-BUILT")

    message(STATUS "Found dependency script: ${PKG}")

    set(DEFINES "${DEFINES}#define LOCAL_${PKG} 1\n")

    list(APPEND PKGS "${PKG}")
  endif()
endforeach(SCRIPT ${DEPS})

set(ENVS "${ENVS}CMAKE_OSX_ARCHITECTURES=\"${CMAKE_OSX_ARCHITECTURES}\"\n")
set(ENVS "${ENVS}CMAKE_OSX_SYSROOT=\"${CMAKE_OSX_SYSROOT}\"\n")
set(ENVS "${ENVS}CMAKE_OSX_DEPLOYMENT_TARGET=\"${CMAKE_OSX_DEPLOYMENT_TARGET}\"\n")

configure_file(dependencies.h.in ../libutopia2/utopia2/dependencies.h @ONLY)
configure_file(dependencies.cmake.in dependencies.cmake @ONLY)
configure_file(dependencies.env.in dependencies.env @ONLY)

string(REGEX REPLACE ";" "|" TARGETS "${PKGS}")
message(STATUS "")
message(STATUS "To build with a local copy of a specific dependency type:")
message(STATUS "")
message(STATUS "    make [${TARGETS}]")
message(STATUS "")
message(STATUS "To build with all local dependencies just type:")
message(STATUS "")
message(STATUS "    make")
message(STATUS "")
