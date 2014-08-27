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

set(SOURCES
  abstractbibliographicmodel.cpp
  abstractfilter.cpp
  aggregatingproxymodel.cpp
  articledelegate.cpp
  articleview.cpp
  bibliographicitem.cpp
  bibliographicmimedata.cpp
  bibliographicsearchbox.cpp
  cJSON.c
  exporter.cpp
  filters.cpp
  importer.cpp
  librarydelegate.cpp
  librarymodel.cpp
  libraryview.cpp
  persistentbibliographicmodel.cpp
  remotequery.cpp
  remotequerybibliographicmodel.cpp
  resolver.cpp
  resolverrunnable.cpp
  sortfilterproxymodel.cpp
  )

set(HEADERS
  cJSON.h
  config.h
  exporter.h
  importer.h
  resolver.h
  )

set(MOC_HEADERS
  abstractbibliographicmodel.h
  abstractfilter.h
  aggregatingproxymodel.h
  aggregatingproxymodel_p.h
  articledelegate.h
  articleview.h
  articleview_p.h
  bibliographicmimedata_p.h
  bibliographicsearchbox.h
  bibliographicsearchbox_p.h
  filters.h
  librarydelegate.h
  librarymodel.h
  librarymodel_p.h
  libraryview.h
  libraryview_p.h
  persistentbibliographicmodel.h
  persistentbibliographicmodel_p.h
  remotequery.h
  remotequerybibliographicmodel.h
  remotequerybibliographicmodel_p.h
  resolverrunnable.h
  sortfilterproxymodel.h
  )

set(FORMS
  )

include_directories(
  ${PROJECT_BINARY_DIR}
  ${utopia2_INCLUDE_DIR}
  ${utopia2_auth_INCLUDE_DIR}
  ${utopia2_qt_INCLUDE_DIR}
  ${utopia2_auth_qt_INCLUDE_DIR}
  ${ICU_INCLUDE_DIR}
  ${Boost_INCLUDE_DIR}
  ${papyro_INCLUDE_DIR}
  ${utf8_INCLUDE_DIR}
  ${spine_INCLUDE_DIR}
  ${utf8_INCLUDE_DIR}
  )

set(RESOURCES athenaeum.qrc)

QT4_WRAP_CPP(MOC_SOURCES ${MOC_HEADERS})
QT4_WRAP_UI(UI_SOURCES ${FORMS})
QT4_ADD_RESOURCES(RES_SOURCES ${RESOURCES})

if(WIN32)
	add_definitions(-DBUILD_LIBATHENAEUM=1)
endif()

add_utopia_library(${PROJECT_NAME} SHARED ${SOURCES} ${MOC_SOURCES} ${UI_SOURCES} ${RES_SOURCES})

target_link_libraries(${PROJECT_NAME}
  utopia2
  utopia2_auth
  utopia2_auth_qt
  utopia2_qt
  utf8
  ${QT_LIBRARIES}
  ${ICU_LIBRARIES}
  ${QJSON_LIBRARIES}
  utf8
  spine
)

install_utopia_library(${PROJECT_NAME} "${COMPONENT}")
