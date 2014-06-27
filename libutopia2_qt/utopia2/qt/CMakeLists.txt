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

project( utopia2_qt )

set(SOURCES
  aboutdialog.cpp
  abstractwidget.cpp
  abstractwindow.cpp
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
  holdablebutton.cpp
  imagecroppingdialog.cpp
  imagedropper.cpp
  imageformatmanager.cpp
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

set(HEADERS
  abstractcommentdata.h
  cache.h
  cacheditem.h
  roundedpixmap.h
  slavemenu.h
  webview.h
  )

set(MOC_HEADERS
  aboutdialog.h
  abstractwidget.h
  abstractwindow.h
  abstractwindow_p.h
  bubble.h
  closebutton.h
  combinedwidget.h
  elidedlabel.h
  embeddedwidget.h
  fieldeditor.h
  filefixerdialog.h
  fileformatdialog.h
  flowbrowser.h
  flowbrowser_p.h
  holdablebutton.h
  imagecroppingdialog.h
  imagedropper.h
  imageformatmanager.h
  messagelabel.h
  parsercontextmodel.h
  progressdialog.h
  preferencesdialog_p.h
  preferencesdialog.h
  preferencespane.h
  raiseaction.h
  shimmerwidget.h
  slavesplitter_p.h
  slavesplitter.h
  slidelayout_p.h
  slidelayout.h
  speechbubble.h
  spinner.h
  splashscreen.h
  tearout.h
  thoughtbubble.h
  thumbnailchooser.h
  thumbnailpreview.h
  uimanager_p.h
  uimanager.h
  updatewidget.h
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
#  ${Graphviz_INCLUDE_DIR}
  ${EXPAT_INCLUDE_DIR}
  ${CMAKE_BINARY_DIR}
  )

QT4_WRAP_UI(UI_SOURCES ${FORMS})

set(RESOURCES utopia2_qt.qrc utopia2_qt_css.qrc)

QT4_WRAP_CPP(MOC_SOURCES ${MOC_HEADERS} )

QT4_ADD_RESOURCES(RES_SOURCES ${RESOURCES})

if(WIN32)
	add_definitions(-DBUILD_LIBUTOPIA_QT=1)
endif()

add_utopia_library( ${PROJECT_NAME} SHARED ${SOURCES} ${MOC_SOURCES} ${UI_SOURCES} ${RES_SOURCES})

#target_link_libraries(${PROJECT_NAME} utopia2 ${QT_LIBRARIES} ${Graphviz_LIBRARIES} ${Boost_LIBRARIES} ${EXPAT_LIBRARIES} ${OPENGL_LIBRARIES} ${Graphviz_GVC_LIBRARIES} ${Graphviz_CDT_LIBRARIES} ${Graphviz_DOT_LIBRARIES} ${Graphviz_GRAPH_LIBRARIES} ${Graphviz_PATHPLAN_LIBRARIES} ${ZLIB_LIBRARIES})
target_link_libraries(${PROJECT_NAME} utopia2 ${QT_LIBRARIES} ${Boost_LIBRARIES} ${EXPAT_LIBRARIES} ${OPENGL_LIBRARIES} ${ZLIB_LIBRARIES})

install_utopia_library(${PROJECT_NAME} "${COMPONENT}")

set(PROJECT_APPLE_NIB qt_menu.nib/classes.nib qt_menu.nib/info.nib qt_menu.nib/keyedobjects.nib)

if(APPLE)
  FOREACH(f ${PROJECT_APPLE_NIB})
    configure_file(${f} ${CMAKE_BINARY_DIR}/${BUNDLE_CONTENTS_DIR}/Resources/${f} COPYONLY)
    install(DIRECTORY ${CMAKE_BINARY_DIR}/${BUNDLE_CONTENTS_DIR}/Resources/qt_menu.nib
            DESTINATION ${BUNDLE_CONTENTS_DIR}/Resources/ COMPONENT "${COMPONENT}")
  ENDFOREACH()
endif()
