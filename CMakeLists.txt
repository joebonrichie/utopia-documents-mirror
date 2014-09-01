###############################################################################
#   
#    This file is part of the Utopia Documents application.
#        Copyright (c) 2008-2014 Lost Island Labs
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

### Versioning and general setup ##################################################################

cmake_minimum_required(VERSION 2.6)
project( Utopia )

#cmake_policy(SET CMP0017 OLD)
cmake_policy(VERSION 2.6)

# Utopia versioning
set(VERSION_MAJOR "2")
set(VERSION_MINOR "4")
set(VERSION_PATCH "4")

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
	add_definitions(-DUTOPIA_BUILD_DEBUG=1)
else()
	add_definitions(-DQT_NO_DEBUG_OUTPUT=1 -DQT_NO_WARNING_OUTPUT=1)
endif()

if(WIN32)
	add_definitions(-D_UNICODE=1)
endif()


### Dependency information ########################################################################

# search for local cmake find modules
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/CMakeConfig/Modules)

include(GetGitRevisionDescription)
include(GetYear)

get_current_year(UTOPIA_CURRENT_YEAR)

IF(WIN32)
  SET(PYTHON_VERSION "2.6")
ELSE()
  SET(PYTHON_VERSION "2.7")
ENDIF()
STRING(REPLACE "." "" PYTHON_VERSION_UNDOTTED "${PYTHON_VERSION}")

# prefer our static libraries
set(UTOPIA_DEPENDENCIES_DIR "${CMAKE_SOURCE_DIR}/dependencies" CACHE STRING "Library dependencies location")
list(INSERT CMAKE_MODULE_PATH 0 ${UTOPIA_DEPENDENCIES_DIR}/share/apps/cmake/modules)
include(${UTOPIA_DEPENDENCIES_DIR}/dependencies.cmake)
list(INSERT CMAKE_PREFIX_PATH 0 "${UTOPIA_DEPENDENCIES_DIR}")
if(WIN32)
    list(INSERT CMAKE_PREFIX_PATH 0 ${UTOPIA_DEPENDENCIES_DIR}/Qt)
    list(INSERT CMAKE_PREFIX_PATH 0 ${UTOPIA_DEPENDENCIES_DIR}/bin)
    list(INSERT CMAKE_PREFIX_PATH 0 "c:/Python${PYTHON_VERSION_UNDOTTED}/")
    # ensure explicit template instantiations are exported on Windows!
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--enable-auto-import")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--enable-auto-import")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--export-all-symbols -Wl,--enable-auto-import")
endif()

### Find dependencies #############################################################################

# PCRE
find_package( PCRE REQUIRED )

# QJSON
find_package( QJSON REQUIRED )

# OpenGL
find_package( OpenGL REQUIRED )

# OpenSSL
find_package( OpenSSL REQUIRED )

# GLEW
find_package( GLEW REQUIRED )

# QGLViewer
find_package( QGLViewer REQUIRED )

# FIXME: Do we need this?
if(WIN32)
    set(QT_USE_QTMAIN TRUE)
endif(WIN32)

# Qt4
find_package( Qt4 REQUIRED )
    set(QT_USE_QTNETWORK 1)
    set(QT_USE_QTOPENGL 1)
    set(QT_USE_QTXML 1)
    set(QT_USE_QTSVG 1)
    set(QT_USE_QTWEBKIT 1)
    set(QT_USE_QTSCRIPT 1)
    set(QT_USE_QTDESIGNER 0)
    set(QT_USE_QTUITOOLS 0)
    set(QT_USE_QTHELP 0)
include(${QT_USE_FILE})


if(APPLE)
    if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        set(CMAKE_OSX_ARCHITECTURES "x86_64"
            CACHE STRING "Build architectures for OSX" FORCE)
        set(CMAKE_OSX_SYSROOT /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.8.sdk
            CACHE STRING "isysroot used for universal binary support" FORCE)
        set(CMAKE_OSX_DEPLOYMENT_TARGET 10.8
            CACHE STRING "-mmacosx-version-min flag used for universal binary support" FORCE)
    endif()
endif()


# Set GUI_TYPE for each platform
if(WIN32)
    # Only set GUI_TYPE to WIN32 in release mode (keeps console up for debug mode)
    if(NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        set(GUI_TYPE WIN32)
    endif()
endif()



# Boost
set(BOOST_VERSION_REQUIRED "1.45")
set(BOOST_ROOT "${UTOPIA_DEPENDENCIES_DIR}")
#set(Boost_DEBUG 1)
set(Boost_ADDITIONAL_VERSIONS "${BOOST_VERSION_REQUIRED}")
if (UNIX)
  set(Boost_USE_STATIC_LIBS OFF)
else()
  set(Boost_USE_STATIC_LIBS OFF)
endif()
find_package(Boost ${BOOST_VERSION_REQUIRED} COMPONENTS thread system python)
if(NOT Boost_FOUND)
  FIND_PACKAGE(Boost COMPONENTS thread system python REQUIRED)
endif()
if(UNIX)
  list(APPEND Boost_THREAD_LIBRARY "pthread")
endif()

# ZLib
find_package(ZLIB REQUIRED)

# Freetype
FIND_PACKAGE(Freetype REQUIRED)
list(APPEND FREETYPE_LIBRARIES ${ZLIB_LIBRARIES})

# Graphviz
###FIND_PACKAGE(Graphviz REQUIRED)

# Expat
FIND_PACKAGE(EXPAT REQUIRED)

# Raptor
FIND_PACKAGE(Raptor REQUIRED)

# Python
SET(_CMAKE_FIND_FRAMEWORK ${CMAKE_FIND_FRAMEWORK})
SET(CMAKE_FIND_FRAMEWORK "NEVER")
SET(Python_ADDITIONAL_VERSIONS "${PYTHON_VERSION}")
FIND_PACKAGE(PythonLibs REQUIRED)
SET(CMAKE_FIND_FRAMEWORK ${_CMAKE_FIND_FRAMEWORK})
if(UNIX AND NOT APPLE)
  SET(USE_SYSTEM_Python "1")
  set(Boost_LIBRARIES ${Boost_LIBRARIES} ${PYTHON_LIBRARIES})
endif()


# SWIG
find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

# Handle Building ############################################################

function(install_utopia_library name comp)
  install(TARGETS ${name} LIBRARY DESTINATION ${LIB_INSTALL_DIR} RUNTIME DESTINATION ${BIN_INSTALL_DIR} ARCHIVE DESTINATION ${LIB_INSTALL_DIR} COMPONENT "${comp}")
endfunction()

function(install_utopia_plugin name comp)
  install(TARGETS ${name} LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR} COMPONENT "${comp}")
endfunction()

function(add_utopia_executable target)
 add_executable(${ARGV})
 ADD_DEPENDENCIES(build ${target})
endfunction()

function(add_utopia_plugin target)
 add_library(${ARGV})
 ADD_DEPENDENCIES(build ${target})
endfunction()

function(add_utopia_library target)
if(UNIX AND NOT APPLE)
  add_library(${ARGV})
  set(SOURCES "${ARGN}")
  list(REMOVE_AT SOURCES 0)
  set_source_files_properties(${SOURCES} PROPERTIES COMPILE_FLAGS "-fPIC")

#  set_target_properties( ${target} PROPERTIES
#    VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}"
#    SOVERSION "1"
#    )
else()
  add_library(${ARGV})
endif()
 ADD_DEPENDENCIES(build ${target})
endfunction()

add_custom_target(build)
add_custom_target(post-build ALL)

ADD_DEPENDENCIES(post-build build)

# General config #############################################################

set(CMAKE_SKIP_RPATH NO)

if(APPLE)
    set(BUNDLE_CONTENTS_DIR "Utopia.app/Contents")
    set(BIN_INSTALL_DIR "${BUNDLE_CONTENTS_DIR}/MacOS")
    set(LIB_INSTALL_DIR "${BUNDLE_CONTENTS_DIR}/Libraries")
    set(PLUGIN_INSTALL_DIR "${BUNDLE_CONTENTS_DIR}/PlugIns")
    set(RESOURCE_INSTALL_DIR "${BUNDLE_CONTENTS_DIR}/Resources")
    set(PYTHON_INSTALL_DIR "${RESOURCE_INSTALL_DIR}/python")
elseif(UNIX)
    # see http://www.cmake.org/Wiki/CMake_RPATH_handling

    set(CMAKE_SKIP_BUILD_RPATH  FALSE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib/utopia-documents/lib")
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    set(LIB_INSTALL_DIR "lib/utopia-documents/lib")
    set(BIN_INSTALL_DIR "bin")
    set(PLUGIN_INSTALL_DIR "lib/utopia-documents/plugins")
    set(RESOURCE_INSTALL_DIR "share/utopia-documents")
    set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "Install path prefix, prepended onto install directories.")
    set(PYTHON_INSTALL_DIR "lib/utopia-documents/python")
elseif(WIN32)
    set(BUNDLE_CONTENTS_DIR ".")
    set(LIB_INSTALL_DIR "bin")
    set(BIN_INSTALL_DIR "bin")
    set(PLUGIN_INSTALL_DIR "plugins")
    set(RESOURCE_INSTALL_DIR "resources")
    set(PYTHON_INSTALL_DIR "${RESOURCE_INSTALL_DIR}/python")
endif()

set(PYTHON_INCLUDE_ROOT)
set(PYTHON_LIB_ROOT)
set(PYTHON_DLL_ROOT)
if(WIN32)
  set(PYTHON_INCLUDE_ROOT "c:/Python${PYTHON_VERSION_UNDOTTED}/Include")
  set(PYTHON_BIN_ROOT "c:/Python${PYTHON_VERSION_UNDOTTED}")
  set(PYTHON_LIB_ROOT "c:/Python${PYTHON_VERSION_UNDOTTED}/Lib")
  set(PYTHON_DLL_ROOT "c:/Python${PYTHON_VERSION_UNDOTTED}/DLLs")
elseif(NOT USE_SYSTEM_Python)
  set(PYTHON_INCLUDE_ROOT "${CMAKE_SOURCE_DIR}/dependencies/include/python${PYTHON_VERSION}")
  set(PYTHON_LIB_ROOT "${CMAKE_SOURCE_DIR}/dependencies/lib/python${PYTHON_VERSION}")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${BIN_INSTALL_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${LIB_INSTALL_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${LIB_INSTALL_DIR})

# Project include dirs #######################################################

set(utf8_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/libutf8")
set(gtl_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/libgtl")
set(spine_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/libspine")
set(crackle_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/libcrackle")
set(papyro_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/libpapyro")
set(utopia2_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/libutopia2")
set(utopia2_qt_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/libutopia2_qt")
set(utopia2_auth_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/libutopia2_auth")
set(utopia2_auth_qt_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/libutopia2_auth_qt")
set(ambrosia_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/libambrosia")
set(cinema6_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/libcinema6")
set(graffiti_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/libgraffiti")
set(athenaeum_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/libathenaeum")

##############################################################################

if(APPLE)
    OPTION(CHANGE_PERMISSIONS "Change ownership to root after building - REQUIRED FOR PACKAGING" OFF)
endif()

set(COMPONENT "Core")
add_subdirectory( libutf8 )
add_subdirectory( libgtl )
add_subdirectory( libutopia2 )
add_subdirectory( libutopia2_qt )
add_subdirectory( data )
add_subdirectory( libspine )
add_subdirectory( libcrackle )

set(COMPONENT "Auth")
add_subdirectory( libutopia2_auth )
add_subdirectory( libutopia2_auth_qt )

include(CMakeDependentOption)

OPTION(UTOPIA_BUILD_DOCUMENTS "Build Utopia Documents" ON)
CMAKE_DEPENDENT_OPTION(UTOPIA_BUILD_DOCVIEW "Build Document Viewer" ON "UTOPIA_BUILD_DOCUMENTS" OFF)
if(UTOPIA_BUILD_DOCUMENTS)
  set(COMPONENT "Documents")
  add_subdirectory( libpapyro )
  #add_subdirectory( libannotator )
  if(UTOPIA_BUILD_DOCVIEW)
    add_subdirectory( papyro )
  endif()
endif()

OPTION(UTOPIA_BUILD_LIBRARY "Build Utopia Library" ON)
if(UTOPIA_BUILD_LIBRARY)
  set(COMPONENT "Library")
  add_subdirectory( libathenaeum )
endif()

OPTION(UTOPIA_BUILD_MOLECULES "Build Utopia Molecules" ON)
CMAKE_DEPENDENT_OPTION(UTOPIA_BUILD_MOLVIEW "Build Molecule Viewer" OFF "UTOPIA_BUILD_MOLECULES" OFF)
if(UTOPIA_BUILD_MOLECULES)
  set(COMPONENT "Molecules")
  add_subdirectory( libambrosia )
  if(UTOPIA_BUILD_MOLVIEW)
    add_subdirectory( ambrosia )
  endif()
endif()

OPTION(UTOPIA_BUILD_SEQUENCES "Build Utopia Sequences" ON)
CMAKE_DEPENDENT_OPTION(UTOPIA_BUILD_SEQVIEW "Build Sequence Viewer" OFF "UTOPIA_BUILD_SEQUENCES" OFF)
if(UTOPIA_BUILD_SEQUENCES)
  set(COMPONENT "Sequences")
  add_subdirectory( libcinema6 )
  if(UTOPIA_BUILD_SEQVIEW)
    add_subdirectory( cinema6 )
  endif()
endif()

OPTION(UTOPIA_BUILD_GRAPHS "Build Utopia Graphs" ON)
if(UTOPIA_BUILD_GRAPHS)
  set(COMPONENT "Graphs")
  add_subdirectory( libgraffiti )
endif()

OPTION(UTOPIA_BUILD_PERSISTENCE "Build Persistence Components" ON)


##############################################################################
# Include Qt plugins #########################################################
##############################################################################

set(QT_PLUGINS_DEBUG "")
if(NOT USE_SYSTEM_Qt)
  # Install needed Qt plugins by copying directories from the qt installation
  if(WIN32)
    set(QT_PLUGINS_DEBUG_WILDCARD "*[a-ce-z]d4")
  elseif(APPLE)
    set(QT_PLUGINS_DEBUG_WILDCARD "*_debug")
  endif()

  file(GLOB_RECURSE QT_PLUGINS
    RELATIVE "${QT_PLUGINS_DIR}"
    "${QT_PLUGINS_DIR}/imageformats/*${CMAKE_SHARED_LIBRARY_SUFFIX}"
    "${QT_PLUGINS_DIR}/iconengines/*${CMAKE_SHARED_LIBRARY_SUFFIX}")
  file(GLOB_RECURSE QT_PLUGINS_DEBUG
    RELATIVE "${QT_PLUGINS_DIR}"
    "${QT_PLUGINS_DIR}/imageformats/${QT_PLUGINS_DEBUG_WILDCARD}${CMAKE_SHARED_LIBRARY_SUFFIX}"
    "${QT_PLUGINS_DIR}/iconengines/${QT_PLUGINS_DEBUG_WILDCARD}${CMAKE_SHARED_LIBRARY_SUFFIX}")
  if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    set(QT_PLUGINS ${QT_PLUGINS_DEBUG})
  else()
    list(REMOVE_ITEM QT_PLUGINS ${QT_PLUGINS_DEBUG})
  endif()
  foreach(QT_PLUGIN ${QT_PLUGINS})
    configure_file("${QT_PLUGINS_DIR}/${QT_PLUGIN}" "${CMAKE_BINARY_DIR}/${PLUGIN_INSTALL_DIR}/${QT_PLUGIN}" COPYONLY)
  endforeach()
  install(DIRECTORY "${CMAKE_BINARY_DIR}/${PLUGIN_INSTALL_DIR}/imageformats" "${CMAKE_BINARY_DIR}/${PLUGIN_INSTALL_DIR}/iconengines"
    DESTINATION "${PLUGIN_INSTALL_DIR}"
    COMPONENT Core)
endif()
##############################################################################
# Include required Win32 DLLs ################################################
##############################################################################

if(WIN32)
#  install(FILES "${UTOPIA_DEPENDENCIES_DIR}/bin/libt1-5.dll" DESTINATION ${LIB_INSTALL_DIR} COMPONENT "Core")
#  install(FILES "${UTOPIA_DEPENDENCIES_DIR}/lib/QGLViewer2.dll" DESTINATION ${LIB_INSTALL_DIR} COMPONENT "Core")
#  install(FILES "c:/MinGW/bin/mingwm10.dll" DESTINATION ${LIB_INSTALL_DIR} COMPONENT "Core")
#  foreach(QT_LIB Core Gui Xml Network OpenGL Svg)
#    install(FILES "${QT_LIBRARY_DIR}/Qt${QT_LIB}4.dll" DESTINATION ${LIB_INSTALL_DIR} COMPONENT "Core")
#  endforeach()

  install(FILES "c:/Windows/System32/msvcrt.dll" DESTINATION ${LIB_INSTALL_DIR} COMPONENT "Core")
  install(FILES "c:/Windows/System32/Python${PYTHON_VERSION_UNDOTTED}.dll" DESTINATION ${LIB_INSTALL_DIR} COMPONENT "Core")
  install(FILES "${CMAKE_SOURCE_DIR}/packaging/win/vcredist_x86.exe" DESTINATION ${LIB_INSTALL_DIR} COMPONENT "Core")
  foreach(SSL_LIB libeay32 ssleay32)
    install(FILES "${UTOPIA_DEPENDENCIES_DIR}/bin/${SSL_LIB}.dll" DESTINATION ${LIB_INSTALL_DIR} COMPONENT "Core")
  endforeach()

endif()

##############################################################################
# Configure package building #################################################
##############################################################################

INCLUDE(InstallRequiredSystemLibraries)

if(APPLE)
  SET(CPACK_SYSTEM_NAME "Mac")
elseif(WIN32)
  SET(CPACK_SYSTEM_NAME "Win32")
elseif(UNIX)
  SET(CPACK_SYSTEM_NAME "Unix")
endif()

SET(CPACK_PACKAGE_NAME "Utopia")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Utopia:Documents - Bringing your documents to life.")
SET(CPACK_PACKAGE_VENDOR "Lost Island Labs")
SET(CPACK_PACKAGE_CONTACT "info@utopiadocs.com")

SET(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
SET(CPACK_PACKAGE_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

SET(PACKAGE_VERSION ${CPACK_PACKAGE_VERSION})
SET(PACKAGE_STRING "Utopia ${CPACK_PACKAGE_VERSION}")

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    set(CPACK_PACKAGE_VERSION_SUFFIX "d")
endif()

if(CPACK_PACKAGE_VARIANT)
  set(CPACK_PACKAGE_VARIANT "-${CPACK_PACKAGE_VARIANT}")
endif()

SET(CPACK_PACKAGE_FILE_NAME "Utopia-${CPACK_PACKAGE_VERSION}${CPACK_PACKAGE_VERSION_SUFFIX}-${CPACK_SYSTEM_NAME}${CPACK_PACKAGE_VARIANT}")
if(WIN32)
  SET(CPACK_PACKAGE_EXECUTABLES "Documents" "Utopia Documents")
else()
  SET(CPACK_PACKAGE_EXECUTABLES "Documents" "Documents")
endif()
#SET(CPACK_PACKAGE_EXECUTABLES "Library" "Library")
SET(CPACK_STRIP_FILES 0)

SET(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/doc/LICENSE.txt")
SET(CPACK_RESOURCE_FILE_README "${PROJECT_SOURCE_DIR}/doc/README.txt")
SET(CPACK_RESOURCE_FILE_WELCOME "${PROJECT_SOURCE_DIR}/CMakeConfig/welcome.txt")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "Utopia")

if(APPLE)
  SET(CPACK_GENERATOR "PackageMaker")
  SET(CPACK_PACKAGE_ICON ${CMAKE_SOURCE_DIR}/papyro/documents.icns)
  SET(CPACK_OSX_PACKAGE_VERSION 10.6)
  SET(CPACK_PACKAGING_INSTALL_PREFIX "/")
  SET(CPACK_PACKAGE_DEFAULT_LOCATION "/Applications")
  # Replace CMAKE_INSTALL_PREFIX to Currently used one,
  # If it wasn't overridden from command line / cache.
  if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/Applications/Utopia" CACHE PATH "Install path prefix, prepended onto install directories." FORCE)
  endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
elseif(WIN32)
  SET(CPACK_PACKAGE_INSTALL_REGISTRY_KEY, "Utopia ${CPACK_PACKAGE_VERSION}")
  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backlasshes.
  SET(CPACK_NSIS_DISPLAY_NAME "Utopia")
#  SET(CPACK_PACKAGE_ICON "${CMake_SOURCE_DIR}/Utilities/Release\\\\InstallIcon.bmp")
  SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\www.utopiadocs.com\\\\")
  SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.utopiadocs.com\\\\")
  SET(CPACK_NSIS_CONTACT "info@utopiadocs.com")
  SET(CPACK_NSIS_MODIFY_PATH OFF)
  SET(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
      ExecWait '\\\"$INSTDIR\\\\bin\\\\vcredist_x86.exe\\\" /q:a'
      Delete '\\\"$INSTDIR\\\\bin\\\\vcredist_x86.exe\\\"'
      ")
elseif(UNIX)
  SET(CPACK_GENERATOR "DEB")
  SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6, libfreetype6, libgcc1, libgl1-mesa-glx, libglu1-mesa, libqt4-network, libqt4-opengl, libqt4-xml, libqtcore4, libqtgui4, libstdc++6, libt1-5, libx11-6, libxext6, zlib1g")
  SET(CPACK_DEBIAN_PACKAGE_SECTION "Science")
endif()

# Don't package source
set(CPACK_SOURCE_TBZ2 OFF)
set(CPACK_SOURCE_TGZ OFF)
set(CPACK_SOURCE_TZ OFF)
set(CPACK_SOURCE_ZIP OFF)

set(CPACK_SOURCE_IGNORE_FILES "/debian/cache" "/debian/packages" "/build")


# This must always be last!
INCLUDE(CPack)

cpack_add_component("Core"
    "Utopia Core"
    DESCRIPTION "The core support libraries of Utopia."
    REQUIRED
    )

cpack_add_component("Auth"
    "Utopia Auth"
    DESCRIPTION "The authentication support libraries of Utopia."
    REQUIRED
    )

if(UTOPIA_BUILD_DOCUMENTS)
cpack_add_component("Documents"
    "Utopia Documents"
    DESCRIPTION "The Utopia document viewer and annotator plus related plugins."
    REQUIRED
    DEPENDS "Core"
    )
endif()

if(UTOPIA_BUILD_LIBRARY)
cpack_add_component("Library"
    "Utopia Library"
    DESCRIPTION "The Utopia library viewer."
    REQUIRED
    DEPENDS "Documents"
    )
endif()

if(UTOPIA_BUILD_MOLECULES)
cpack_add_component("Molecules"
    "Utopia Molecules"
    DESCRIPTION "The Utopia molecule viewer plus related plugins."
    DEPENDS "Core"
    )
endif()

if(UTOPIA_BUILD_SEQUENCES)
cpack_add_component("Sequences"
    "Utopia Sequences"
    DESCRIPTION "The Utopia sequence viewer plus related plugins."
    DEPENDS "Core"
    )
endif()

if(UTOPIA_BUILD_GRAPHS)
cpack_add_component("Graphs"
    "Utopia Graphs"
    DESCRIPTION "The Utopia graph viewer plus related plugins."
    DEPENDS "Core"
    )
endif()

if(APPLE)
  if(EXISTS "${CMAKE_SOURCE_DIR}/packaging/mac/package-mac.sh.in")
    CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/packaging/mac/package-mac.sh.in" "${PROJECT_BINARY_DIR}/package-mac.sh" @ONLY IMMEDIATE)
  endif()
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/packaging/src/package-src.sh.in")
  CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/packaging/src/package-src.sh.in" "${PROJECT_BINARY_DIR}/package-src.sh" @ONLY IMMEDIATE)
endif()

if(WIN32 OR APPLE)
  set(UTOPIA_SUPPORT_LIBRARY_DIR BundleSupportLibraries)
  file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/${UTOPIA_SUPPORT_LIBRARY_DIR}")

  set(OSX_MAKE_STANDALONE_BUNDLE_CMAKE_SCRIPT "${CMAKE_BINARY_DIR}/CMakeFiles/CompleteBundle.cmake")
  set(OSX_SET_PERMISSIONS_SCRIPT "${CMAKE_BINARY_DIR}/CMakeFiles/SetPermissions.sh")

  CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/CMakeConfig/CompleteBundle.cmake.in" "${OSX_MAKE_STANDALONE_BUNDLE_CMAKE_SCRIPT}" @ONLY IMMEDIATE)
  CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/CMakeConfig/SetPermissions.sh.in" "${OSX_SET_PERMISSIONS_SCRIPT}" @ONLY IMMEDIATE)

  set(CPACK_INSTALL_COMMANDS "${OSX_SET_PERMISSIONS_SCRIPT}")

  install(DIRECTORY "${PROJECT_BINARY_DIR}/${UTOPIA_SUPPORT_LIBRARY_DIR}/" DESTINATION "${LIB_INSTALL_DIR}" COMPONENT "Core")

  add_custom_command(TARGET post-build COMMAND ${CMAKE_COMMAND} -P ${OSX_MAKE_STANDALONE_BUNDLE_CMAKE_SCRIPT})
  add_dependencies(post-build preinstall)
endif()
