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

if(UNIX AND NOT APPLE)
project(utopia-documents)
else()
project(Documents)
endif()

set(SOURCES
  main.cpp
  qtlocalpeer.cpp
  qtsingleapplication.cpp
  )

include_directories(
  ${papyro_INCLUDE_DIR}
  ${utopia2_INCLUDE_DIR}
  ${utopia2_qt_INCLUDE_DIR}
  ${utopia2_auth_INCLUDE_DIR}
  ${utopia2_auth_qt_INCLUDE_DIR}
  ${Boost_INCLUDE_DIR}
  ${spine_INCLUDE_DIR}
  ${athenaeum_INCLUDE_DIR}
  ${utf8_INCLUDE_DIR}
  )

if(MINGW)
  ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Utopiaico.o
             COMMAND windres.exe -I${CMAKE_CURRENT_SOURCE_DIR} -i${CMAKE_CURRENT_SOURCE_DIR}/Utopia.rc
             -o ${CMAKE_CURRENT_BINARY_DIR}/Utopiaico.o)
  set(SOURCES ${SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/Utopiaico.o)
else()
  set(SOURCES ${SOURCES} Utopia.rc)
endif()

add_utopia_executable(${PROJECT_NAME} ${GUI_TYPE} ${SOURCES})

target_link_libraries(${PROJECT_NAME}
  papyro
  spine
  utf8
  utopia2
  utopia2_qt
  utopia2_auth
  utopia2_auth_qt
  athenaeum
  ${Boost_LIBRARIES}
  )
qt5_use_modules(${PROJECT_NAME} Widgets Network Gui Core OpenGL)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${BIN_INSTALL_DIR} COMPONENT "${COMPONENT}")

if(UNIX AND NOT APPLE)
  install(DIRECTORY icons/linux/ DESTINATION share/icons/hicolor COMPONENT "${COMPONENT}" FILES_MATCHING PATTERN "*.png" PATTERN ".svn" EXCLUDE )
  install(DIRECTORY icons/linux/ DESTINATION share/icons/gnome COMPONENT "${COMPONENT}" FILES_MATCHING PATTERN "*.png" PATTERN ".svn" EXCLUDE )

  install(FILES UtopiaDocuments.desktop DESTINATION share/applications COMPONENT "${COMPONENT}")
endif()

set(PROJECT_APPLE_PLIST utopia.plist.in)
set(PROJECT_APPLE_ICON documents.icns)

if(APPLE)
  configure_file(${PROJECT_APPLE_ICON} ${CMAKE_BINARY_DIR}/${BUNDLE_CONTENTS_DIR}/Resources/${PROJECT_APPLE_ICON} COPYONLY)
  configure_file(${PROJECT_APPLE_PLIST} ${CMAKE_BINARY_DIR}/${BUNDLE_CONTENTS_DIR}/Info.plist @ONLY)
  install(FILES ${CMAKE_BINARY_DIR}/${BUNDLE_CONTENTS_DIR}/Resources/${PROJECT_APPLE_ICON}
          DESTINATION ${BUNDLE_CONTENTS_DIR}/Resources/ COMPONENT "${COMPONENT}")
  install(FILES ${CMAKE_BINARY_DIR}/${BUNDLE_CONTENTS_DIR}/Info.plist DESTINATION ${BUNDLE_CONTENTS_DIR} COMPONENT "${COMPONENT}")
endif()
