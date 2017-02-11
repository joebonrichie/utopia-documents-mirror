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

project( utopia2_qt )

set(SOURCES
  aboutdialog.cpp
  abstractwidget.cpp
  abstractwindow.cpp
  actionproxy.cpp
  closebutton.cpp
  combinedwidget.cpp
  configurator.cpp
  elidedlabel.cpp
  embeddedwidget.cpp
  fieldeditor.cpp
  filedialog.cpp
  filefixerdialog.cpp
  fileformatdialog.cpp
  flowbrowser.cpp
  hidpi.cpp
  holdablebutton.cpp
  imagecroppingdialog.cpp
  imagedropper.cpp
  imageformatmanager.cpp
  menuproxy.cpp
  imagepreview.cpp
  messagelabel.cpp
  parsercontextmodel.cpp
  preferencesdialog.cpp
  preferencespane.cpp
  progressdialog.cpp
  raiseaction.cpp
  roundedpixmap.cpp
  shimmerwidget.cpp
  slavemenu.cpp
  slavesplitter.cpp
  slidelayout.cpp
  speechbubble.cpp
  spinner.cpp
  splashscreen.cpp
  tearout.cpp
  thoughtbubble.cpp
  thumbnailchooser.cpp
  thumbnailpreview.cpp
  uimanager.cpp
  updatewidget.cpp
  webview.cpp
  )

set(FORMS
  filefixerdialog.ui
  fileformatdialog.ui
  updatewidget.ui
  updatewidgetchecking.ui
  updatewidgeterror.ui
  updatewidgetpending.ui
  updatewidgetuptodate.ui
  )

include_directories(
  ${PROJECT_BINARY_DIR}
  ${utopia2_INCLUDE_DIR}
  ${utopia2_qt_INCLUDE_DIR}
  ${Boost_INCLUDE_DIR}
  ${EXPAT_INCLUDE_DIR}
  ${CMAKE_BINARY_DIR}
  )

# auto-moc misses bubble.h (probably because of templates)
qt5_wrap_cpp(SOURCES bubble.h)
qt5_wrap_ui(UI_SOURCES ${FORMS})

qt5_add_resources(SOURCES utopia2_qt.qrc utopia2_qt_css.qrc)

if(WIN32)
	add_definitions(-DBUILD_LIBUTOPIA_QT=1)
endif()

add_utopia_library(${PROJECT_NAME} SHARED ${SOURCES} ${UI_SOURCES})

target_link_libraries(${PROJECT_NAME} utopia2 ${Boost_LIBRARIES} ${EXPAT_LIBRARIES} ${OPENGL_LIBRARIES} ${ZLIB_LIBRARIES})
qt5_use_modules(${PROJECT_NAME} WebKitWidgets Widgets Network OpenGL Svg Xml)

install_utopia_library(${PROJECT_NAME} "${COMPONENT}")

set(PROJECT_APPLE_NIB qt_menu.nib/classes.nib qt_menu.nib/info.nib qt_menu.nib/keyedobjects.nib)

if(APPLE)
  foreach(f ${PROJECT_APPLE_NIB})
    configure_file(${f} ${CMAKE_BINARY_DIR}/${BUNDLE_CONTENTS_DIR}/Resources/${f} COPYONLY)
    install(DIRECTORY ${CMAKE_BINARY_DIR}/${BUNDLE_CONTENTS_DIR}/Resources/qt_menu.nib
            DESTINATION ${BUNDLE_CONTENTS_DIR}/Resources/ COMPONENT "${COMPONENT}")
  endforeach()
endif()
