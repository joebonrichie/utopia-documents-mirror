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

### Versioning and general setup ##################################################################

# Get sensible variables for OS
if(APPLE)
  set(OS_APPLE TRUE)
elseif(WIN32)
  set(OS_WINDOWS TRUE)
elseif(UNIX)
  set(OS_LINUX TRUE)
endif()

# Ensure sensible cmake environment
cmake_minimum_required(VERSION 2.8.11)
cmake_policy(VERSION 2.8)

if(POLICY CMP0020)
  cmake_policy(SET CMP0020 OLD)
endif(POLICY CMP0020)
if(POLICY CMP0043)
  cmake_policy(SET CMP0043 OLD)
endif(POLICY CMP0043)
if(POLICY CMP0046)
  cmake_policy(SET CMP0046 OLD)
endif(POLICY CMP0046)

# Main project name
project(Utopia)

# Utopia versioning - read this in from the version file
file(READ version UTOPIA_VERSION)
string(REGEX MATCHALL [a-zA-Z0-9]+ UTOPIA_VERSION_PARTS ${UTOPIA_VERSION})
list(LENGTH UTOPIA_VERSION_PARTS UTOPIA_VERSION_SIZE)
list(GET UTOPIA_VERSION_PARTS 0 UTOPIA_VERSION_MAJOR)
list(GET UTOPIA_VERSION_PARTS 1 UTOPIA_VERSION_MINOR)
list(GET UTOPIA_VERSION_PARTS 2 UTOPIA_VERSION_PATCH)
set(UTOPIA_VERSION_FULL "${UTOPIA_VERSION_MAJOR}.${UTOPIA_VERSION_MINOR}.${UTOPIA_VERSION_PATCH}")
message(STATUS "Configuring Utopia Documents v${UTOPIA_VERSION_FULL}")

# Should we be building debug or release?
string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE)
if(CMAKE_BUILD_TYPE STREQUAL "debug")
  set(BUILD_DEBUG TRUE)
else()
  set(BUILD_RELEASE TRUE)
endif()

# Extra checks and useful variables
set(HARDENED_FLAGS "-fstack-protector -Wformat -Wformat-security -Wall -Wno-unused-local-typedef")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${HARDENED_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${HARDENED_FLAGS}")

# Make sure definitions reflect the build type / platform
if(BUILD_DEBUG)
  add_definitions(-DUTOPIA_BUILD_DEBUG=1)
elseif(BUILD_RELEASE)
  add_definitions(-DQT_NO_DEBUG_OUTPUT=1 -DQT_NO_WARNING_OUTPUT=1)
  add_definitions(-D_FORTIFY_SOURCE=2) # Harden the code
endif()
if(OS_WINDOWS)
  add_definitions(-D_UNICODE=1)
endif()
if(UTOPIA_PACKAGE_MANAGED)
  add_definitions(-DUTOPIA_PACKAGE_MANAGED=1)
endif()

set(CMAKE_MACOSX_RPATH ON)

# select pdf backend
set(UTOPIA_LIBSPINE_PDF_BACKEND "poppler" CACHE STRING "Spine PDF backend")
set_property(CACHE UTOPIA_LIBSPINE_PDF_BACKEND PROPERTY STRINGS "xpdf" "poppler")

### Dependency information ########################################################################

# search for local cmake find modules
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/CMakeConfig/Modules)

include(GetGitRevisionDescription)
include(GetYear)

get_current_year(UTOPIA_CURRENT_YEAR)

SET(PYTHON_VERSION "2.7")
STRING(REPLACE "." "" PYTHON_VERSION_UNDOTTED "${PYTHON_VERSION}")

# prefer our static libraries
set(UTOPIA_DEPENDENCIES_DIR "${CMAKE_SOURCE_DIR}/dependencies" CACHE STRING "Library dependencies location")
list(INSERT CMAKE_MODULE_PATH 0 ${UTOPIA_DEPENDENCIES_DIR}/share/apps/cmake/modules)
include(${UTOPIA_DEPENDENCIES_DIR}/dependencies.cmake)
list(INSERT CMAKE_PREFIX_PATH 0 "${UTOPIA_DEPENDENCIES_DIR}")
include_directories(${UTOPIA_DEPENDENCIES_DIR}/include)
if(OS_WINDOWS)
    list(INSERT CMAKE_PREFIX_PATH 0 ${UTOPIA_DEPENDENCIES_DIR}/bin)
    list(INSERT CMAKE_PREFIX_PATH 0 "/mingw32/lib/python${PYTHON_VERSION}/")
    # ensure explicit template instantiations are exported on Windows!
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--enable-auto-import")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--enable-auto-import")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--export-all-symbols -Wl,--enable-auto-import")
endif()

### Find dependencies #############################################################################

# PCRE
find_package( PCRE REQUIRED )

# OpenGL
find_package( OpenGL REQUIRED )

# OpenSSL
find_package( OpenSSL REQUIRED )

# GLEW
#find_package( GLEW REQUIRED )

# QGLViewer
#find_package( QGLViewer REQUIRED )

# Qt5
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
find_package(Qt5 REQUIRED COMPONENTS Core Widgets Network Gui Concurrent OpenGL Svg WebKitWidgets WebKit Xml PrintSupport Script)

if("${UTOPIA_LIBSPINE_PDF_BACKEND}" STREQUAL "xpdf")
  add_definitions(-DUTOPIA_SPINE_BACKEND_XPDF=1)
elseif("${UTOPIA_LIBSPINE_PDF_BACKEND}" STREQUAL "poppler")
  add_definitions(-DUTOPIA_SPINE_BACKEND_POPPLER=1)
  find_package( Poppler REQUIRED )
else()
  message( FATAL_ERROR "No PDF backend selected." )
endif()

if(OS_APPLE)
    if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        set(CMAKE_OSX_ARCHITECTURES "x86_64"
            CACHE STRING "Build architectures for OSX" FORCE)
        set(CMAKE_OSX_SYSROOT /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk
            CACHE STRING "isysroot used for universal binary support" FORCE)
        set(CMAKE_OSX_DEPLOYMENT_TARGET 10.7
            CACHE STRING "-mmacosx-version-min flag used for universal binary support" FORCE)
    endif()
endif()


# Only set GUI_TYPE to WIN32 in release mode (keeps console up for debug mode)
if(OS_WINDOWS AND BUILD_RELEASE)
  set(GUI_TYPE WIN32)
endif()



# Boost
set(BOOST_VERSION_REQUIRED "1.45")
set(BOOST_ROOT "${UTOPIA_DEPENDENCIES_DIR}")
set(Boost_ADDITIONAL_VERSIONS "${BOOST_VERSION_REQUIRED}")
set(Boost_USE_STATIC_LIBS OFF)
find_package(Boost ${BOOST_VERSION_REQUIRED} COMPONENTS thread system python)
if(NOT Boost_FOUND)
  FIND_PACKAGE(Boost COMPONENTS thread system python REQUIRED)
endif()
if(OS_LINUX OR OS_APPLE)
  list(APPEND Boost_THREAD_LIBRARY "pthread")
endif()

# ZLib
find_package(ZLIB REQUIRED)

# Freetype
FIND_PACKAGE(Freetype REQUIRED)
list(APPEND FREETYPE_LIBRARIES ${ZLIB_LIBRARIES})

if("${UTOPIA_LIBSPINE_PDF_BACKEND}" STREQUAL "poppler")
    FIND_PACKAGE(FontConfig REQUIRED)
    FIND_PACKAGE(Poppler REQUIRED)
endif()

# Graphviz
###FIND_PACKAGE(Graphviz REQUIRED)

# Expat
FIND_PACKAGE(EXPAT REQUIRED)

# Python
SET(_CMAKE_FIND_FRAMEWORK ${CMAKE_FIND_FRAMEWORK})
SET(CMAKE_FIND_FRAMEWORK "NEVER")
SET(Python_ADDITIONAL_VERSIONS "${PYTHON_VERSION}")
FIND_PACKAGE(PythonLibs REQUIRED)
FIND_PROGRAM(Python "python")
SET(CMAKE_FIND_FRAMEWORK ${_CMAKE_FIND_FRAMEWORK})
if(OS_LINUX)
  set(Boost_LIBRARIES ${Boost_LIBRARIES} ${PYTHON_LIBRARIES})
endif()


# SWIG
find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})


# SASS compiled CSS files
find_program(SASS_EXECUTABLE sass)
function(preprocess_sass out_var)
  set(result)
  foreach(in_f ${ARGN})
    file(RELATIVE_PATH out_f ${CMAKE_CURRENT_SOURCE_DIR} ${in_f})
    set(out_f "${CMAKE_CURRENT_SOURCE_DIR}/${out_f}")
    get_filename_component(out_f_dirname "${out_f}" DIRECTORY)
    get_filename_component(out_f_basename "${out_f}" NAME_WE)
    set(out_f "${out_f_dirname}/${out_f_basename}.css")
    file(RELATIVE_PATH out_f_sanitised ${CMAKE_SOURCE_DIR} ${out_f})
    add_custom_command(OUTPUT ${out_f}
      COMMAND ${SASS_EXECUTABLE} ${in_f} ${out_f}
      DEPENDS ${in_f}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMENT "Creating preprocessed sass file ${out_f_sanitised}"
      VERBATIM
      )
    list(APPEND result ${out_f})
  endforeach()
  set(${out_var} "${result}" PARENT_SCOPE)
endfunction()


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
if(OS_LINUX)
  add_library(${ARGV})
  set(SOURCES "${ARGN}")
  list(REMOVE_AT SOURCES 0)
  set_source_files_properties(${SOURCES} PROPERTIES COMPILE_FLAGS "-fPIC")

#  set_target_properties( ${target} PROPERTIES
#    VERSION ${UTOPIA_VERSION_FULL}
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

if(OS_APPLE)
    set(BUNDLE_CONTENTS_DIR "Utopia.app/Contents")
    set(BIN_INSTALL_DIR "${BUNDLE_CONTENTS_DIR}/MacOS")
    set(LIB_INSTALL_DIR "${BUNDLE_CONTENTS_DIR}/Libraries")
    set(PLUGIN_INSTALL_DIR "${BUNDLE_CONTENTS_DIR}/PlugIns")
    set(RESOURCE_INSTALL_DIR "${BUNDLE_CONTENTS_DIR}/Resources")
    set(PYTHON_INSTALL_DIR "${RESOURCE_INSTALL_DIR}/python")
elseif(OS_LINUX)
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
elseif(OS_WINDOWS)
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
if(OS_WINDOWS)
  set(PYTHON_INCLUDE_ROOT "c:/Python${PYTHON_VERSION_UNDOTTED}/Include")
  set(PYTHON_BIN_ROOT "c:/msys32/mingw32/bin")
  set(PYTHON_LIB_ROOT "c:/msys32/mingw32/lib/python${PYTHON_VERSION}")
  set(PYTHON_DLL_ROOT "${PYTHON_BIN_ROOT}/DLLs")
elseif(OS_APPLE)
  set(PYTHON_INCLUDE_ROOT "${UTOPIA_DEPENDENCIES_DIR}/include/python${PYTHON_VERSION}")
  set(PYTHON_LIB_ROOT "${UTOPIA_DEPENDENCIES_DIR}/lib/python${PYTHON_VERSION}")
  set(PYTHON_BIN_ROOT "${UTOPIA_DEPENDENCIES_DIR}")
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

if(OS_APPLE)
    OPTION(CHANGE_PERMISSIONS "Change ownership to root after building - REQUIRED FOR PACKAGING" OFF)
endif()

set(COMPONENT "Core")
add_subdirectory( python )
add_subdirectory( libutf8 )
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

OPTION(UTOPIA_BUILD_MOLECULES "Build Utopia Molecules" OFF)
CMAKE_DEPENDENT_OPTION(UTOPIA_BUILD_MOLVIEW "Build Molecule Viewer" OFF "UTOPIA_BUILD_MOLECULES" OFF)
if(UTOPIA_BUILD_MOLECULES)
  set(COMPONENT "Molecules")
  add_subdirectory( libgtl )
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

if(OS_WINDOWS OR NOT USE_SYSTEM_Qt)
  # Install needed Qt plugins by copying directories from the qt installation
  foreach(QT5_PLUGIN ${Qt5Core_PLUGINS}
                     ${Qt5Widgets_PLUGINS}
                     ${Qt5Network_PLUGINS}
                     ${Qt5Gui_PLUGINS}
                     ${Qt5Concurrent_PLUGINS}
                     ${Qt5OpenGL_PLUGINS}
                     ${Qt5Svg_PLUGINS}
                     ${Qt5WebKitWidgets_PLUGINS}
                     ${Qt5WebKit_PLUGINS}
                     ${Qt5Xml_PLUGINS}
                     ${Qt5PrintSupport_PLUGINS}
                     ${Qt5Script_PLUGINS})
    if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
      get_target_property(_loc ${QT5_PLUGIN} LOCATION_Debug)
    else()
      get_target_property(_loc ${QT5_PLUGIN} LOCATION_Release)
    endif()
    get_filename_component(QT5_PLUGIN_NAME "${_loc}" NAME)
    get_filename_component(_plugin_dir "${_loc}" DIRECTORY)
    get_filename_component(QT5_PLUGIN_REL_DIR "${_plugin_dir}" NAME)
    set(QT5_PLUGIN_REL_PATH "${QT5_PLUGIN_REL_DIR}/${QT5_PLUGIN_NAME}")
    configure_file("${_loc}" "${CMAKE_BINARY_DIR}/${PLUGIN_INSTALL_DIR}/${QT5_PLUGIN_REL_PATH}" COPYONLY)
    install(FILES "${CMAKE_BINARY_DIR}/${PLUGIN_INSTALL_DIR}/${QT5_PLUGIN_REL_PATH}" DESTINATION "${PLUGIN_INSTALL_DIR}/${QT5_PLUGIN_REL_DIR}" COMPONENT Core)
  endforeach()
endif()

##############################################################################
# Configure package building #################################################
##############################################################################

INCLUDE(InstallRequiredSystemLibraries)

if(OS_APPLE)
  SET(CPACK_SYSTEM_NAME "Mac")
elseif(OS_WINDOWS)
  SET(CPACK_SYSTEM_NAME "Win32")
elseif(OS_LINUX)
  SET(CPACK_SYSTEM_NAME "Unix")
endif()

SET(CPACK_PACKAGE_NAME "Utopia")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Utopia:Documents - Bringing your documents to life.")
SET(CPACK_PACKAGE_VENDOR "Lost Island Labs")
SET(CPACK_PACKAGE_CONTACT "info@utopiadocs.com")

SET(CPACK_PACKAGE_VERSION_MAJOR ${UTOPIA_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${UTOPIA_VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${UTOPIA_VERSION_PATCH})
SET(CPACK_PACKAGE_VERSION ${UTOPIA_VERSION_FULL})

SET(PACKAGE_VERSION ${CPACK_PACKAGE_VERSION})
SET(PACKAGE_STRING "Utopia ${CPACK_PACKAGE_VERSION}")

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    set(CPACK_PACKAGE_VERSION_SUFFIX "d")
endif()

if(CPACK_PACKAGE_VARIANT)
  set(CPACK_PACKAGE_VARIANT "-${CPACK_PACKAGE_VARIANT}")
endif()

SET(CPACK_PACKAGE_FILE_NAME "Utopia-${CPACK_PACKAGE_VERSION}${CPACK_PACKAGE_VERSION_SUFFIX}-${CPACK_SYSTEM_NAME}${CPACK_PACKAGE_VARIANT}")
if(OS_WINDOWS)
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

if(OS_APPLE)
  SET(CPACK_GENERATOR "PackageMaker")
  SET(CPACK_PACKAGE_ICON ${CMAKE_SOURCE_DIR}/papyro/documents.icns)
  SET(CPACK_OSX_PACKAGE_VERSION 10.6)
  SET(CPACK_PACKAGING_INSTALL_PREFIX "/")
  SET(CPACK_PACKAGE_DEFAULT_LOCATION "/Applications")
  # Replace CMAKE_INSTALL_PREFIX to Currently used one,
  # If it wasn't overridden from command line / cache.
  if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/Applications" CACHE PATH "Install path prefix, prepended onto install directories." FORCE)
  endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
elseif(OS_WINDOWS)
  SET(CPACK_PACKAGE_INSTALL_REGISTRY_KEY, "Utopia ${CPACK_PACKAGE_VERSION}")
  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backlasshes.
  SET(CPACK_NSIS_DISPLAY_NAME "Utopia")
  SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\www.utopiadocs.com\\\\")
  SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.utopiadocs.com\\\\")
  SET(CPACK_NSIS_CONTACT "info@utopiadocs.com")
  SET(CPACK_NSIS_MODIFY_PATH OFF)
elseif(OS_LINUX)
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

OPTION(UTOPIA_SKIP_PUBLIC_PYTHON_PACKAGES "Skip including public Utopia python packages assuming they will be packaged externally." OFF)

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

if(OS_APPLE)
  if(EXISTS "${CMAKE_SOURCE_DIR}/packaging/mac/package-mac.sh.in")
    CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/packaging/mac/package-mac.sh.in" "${PROJECT_BINARY_DIR}/package-mac.sh" @ONLY IMMEDIATE)
  endif()
endif()

if(OS_WINDOWS)
  if(EXISTS "${CMAKE_SOURCE_DIR}/packaging/win/package-win.sh.in")
    CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/packaging/win/package-win.sh.in" "${PROJECT_BINARY_DIR}/package-win.sh" @ONLY IMMEDIATE)
  endif()
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/packaging/src/package-src.sh.in")
  CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/packaging/src/package-src.sh.in" "${PROJECT_BINARY_DIR}/package-src.sh" @ONLY IMMEDIATE)
endif()

##############################################################################
# Include required Win32 DLLs ################################################
##############################################################################

if(OS_WINDOWS)
#  install(FILES "${UTOPIA_DEPENDENCIES_DIR}/bin/libt1-5.dll" DESTINATION ${LIB_INSTALL_DIR} COMPONENT "Core")
#  install(FILES "${UTOPIA_DEPENDENCIES_DIR}/lib/QGLViewer2.dll" DESTINATION ${LIB_INSTALL_DIR} COMPONENT "Core")
#  install(FILES "c:/MinGW/bin/mingwm10.dll" DESTINATION ${LIB_INSTALL_DIR} COMPONENT "Core")
#  foreach(QT_LIB Core Gui Xml Network OpenGL Svg)
#    install(FILES "${QT_LIBRARY_DIR}/Qt${QT_LIB}4.dll" DESTINATION ${LIB_INSTALL_DIR} COMPONENT "Core")
#  endforeach()

  #install(FILES "c:/Windows/System32/msvcrt.dll" DESTINATION ${LIB_INSTALL_DIR} COMPONENT "Core")
  #install(FILES "c:/Windows/System32/Python${PYTHON_VERSION_UNDOTTED}.dll" DESTINATION ${LIB_INSTALL_DIR} COMPONENT "Core")
  #install(FILES "${CMAKE_SOURCE_DIR}/packaging/win/vcredist_x86.exe" DESTINATION ${LIB_INSTALL_DIR} COMPONENT "Core")
  #file(GLOB files "${UTOPIA_DEPENDENCIES_DIR}/bin/*eay*")
  #foreach(SSL_LIB ${files})
  #  install(FILES "${SSL_LIB}" DESTINATION ${LIB_INSTALL_DIR} COMPONENT "Core")
  #endforeach()

  set(QT_BINARY_DIR "c:/Qt/Qt5.5.1/5.5/mingw492_32/bin")
  install(FILES "${QT_BINARY_DIR}/libEGL.dll" DESTINATION ${LIB_INSTALL_DIR} COMPONENT "Core")
  install(FILES "${QT_BINARY_DIR}/libGLESv2.dll" DESTINATION ${LIB_INSTALL_DIR} COMPONENT "Core")
  install(FILES "${QT_BINARY_DIR}/opengl32sw.dll" DESTINATION ${LIB_INSTALL_DIR} COMPONENT "Core")
endif()


if(OS_WINDOWS OR OS_APPLE)
  set(UTOPIA_SUPPORT_LIBRARY_DIR BundleSupportLibraries)
  file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/${UTOPIA_SUPPORT_LIBRARY_DIR}")

  set(OSX_MAKE_STANDALONE_BUNDLE_CMAKE_SCRIPT "${CMAKE_BINARY_DIR}/CMakeFiles/CompleteBundle.cmake")
  set(OSX_SET_PERMISSIONS_SCRIPT "${CMAKE_BINARY_DIR}/CMakeFiles/SetPermissions.sh")

  CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/CMakeConfig/CompleteBundle.cmake.in" "${OSX_MAKE_STANDALONE_BUNDLE_CMAKE_SCRIPT}" @ONLY IMMEDIATE)
  CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/CMakeConfig/SetPermissions.sh.in" "${OSX_SET_PERMISSIONS_SCRIPT}" @ONLY IMMEDIATE)

  set(CPACK_INSTALL_COMMANDS "${OSX_SET_PERMISSIONS_SCRIPT}")

  install(DIRECTORY "${PROJECT_BINARY_DIR}/${UTOPIA_SUPPORT_LIBRARY_DIR}/" DESTINATION "${LIB_INSTALL_DIR}" COMPONENT "Core")

  add_custom_command(TARGET post-build COMMAND ${CMAKE_COMMAND} -P ${OSX_MAKE_STANDALONE_BUNDLE_CMAKE_SCRIPT})
endif()
