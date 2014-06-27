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

set(SOURCES
  axis.cpp
  entrydelegate.cpp
  graphicsFlipWidget.cpp
  graphswidget.cpp
  graphtableview.cpp
  qtcolorpicker.cpp
  tablewidget.cpp
  view.cpp
  )

set(MOC_HEADERS
  axis.h
  graphicsFlipWidget.h
  graphswidget.h
  graphtableview.h
  qtcolorpicker.h
  qtcolorpicker_p.h
  tablewidget.h
  view.h
  )

include_directories(
  ${utopia2_INCLUDE_DIR}
  ${utopia2_qt_INCLUDE_DIR}
  )

set(RESOURCES tableeditor.qrc)

QT4_WRAP_CPP(MOC_SOURCES ${MOC_HEADERS} )
QT4_ADD_RESOURCES(RES_SOURCES ${RESOURCES})

if(WIN32)
	add_definitions(-DBUILD_LIBGRAFFITI=1)
endif()

add_utopia_library( ${PROJECT_NAME} SHARED ${SOURCES} ${MOC_SOURCES} ${RES_SOURCES})
target_link_libraries(${PROJECT_NAME} utopia2 utopia2_qt ${QT_LIBRARIES})

install_utopia_library(${PROJECT_NAME} "${COMPONENT}")
