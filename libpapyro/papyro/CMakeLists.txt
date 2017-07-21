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

set(SOURCES
    abstractfilter.cpp
    abstractprocessor.cpp
    aggregatingproxymodel.cpp
    annotationprocessor.cpp
    annotationprocessoraction.cpp
    annotationresultitem.cpp
    annotator.cpp
    annotatorrunnable.cpp
    annotatorrunnablepool.cpp
    articledelegate.cpp
    articleview.cpp
    bibliographicmimedata.cpp
    bibliographicsearchbox.cpp
    bibliography.cpp
    capabilities.cpp
    citation.cpp
    citationfinder.cpp
    citationpopup.cpp
    citations.cpp
    cJSON.c
    collection.cpp
    commentdata.cpp
    cslengine.cpp
    cslengineadapter.cpp
    decorator.cpp
    dispatcher.cpp
    documentfactory.cpp
    documentmanager.cpp
    documentproxy.cpp
    documentview.cpp
    embeddedframe.cpp
    embeddedpane.cpp
    embeddedpanefactory.cpp
    exporter.cpp
    filters.cpp
    importer.cpp
    librarydelegate.cpp
    librarymodel.cpp
    libraryview.cpp
    overlayrenderer.cpp
    overlayrenderermapper.cpp
    pager.cpp
    pageview.cpp
    papyrotab.cpp
    papyrowindow.cpp
    persistencemodel.cpp
    phraselookup.cpp
    playercontrols.cpp
    printer.cpp
    progresslozenge.cpp
    raisetabaction.cpp
    remotequery.cpp
    remotequerybibliography.cpp
    resolver.cpp
    resolverqueue.cpp
    resolverrunnable.cpp
    resultitem.cpp
    resultsview.cpp
    searchbar.cpp
    selectionprocessor.cpp
    selectionprocessoraction.cpp
    sidebar.cpp
    sliver.cpp
    sortfilterproxymodel.cpp
    tabbar.cpp
    uimodifier.cpp
    utils.cpp
    visualiser.cpp
    )

include_directories(
  ${athenaeum_INCLUDE_DIR}
  ${Boost_INCLUDE_DIR}
  ${CMAKE_BINARY_DIR}
  ${graffiti_INCLUDE_DIR}
  ${ICU_INCLUDE_DIR}
  ${papyro_INCLUDE_DIR}
  ${PCRE_INCLUDE_DIR}
  ${PROJECT_BINARY_DIR}
  ${spine_INCLUDE_DIR}
  ${utf8_INCLUDE_DIR}
  ${utopia2_auth_INCLUDE_DIR}
  ${utopia2_auth_qt_INCLUDE_DIR}
  ${utopia2_INCLUDE_DIR}
  ${utopia2_qt_INCLUDE_DIR}
  )

# Preprocess any scss files
file(GLOB_RECURSE SASS_SOURCES *.scss)
preprocess_sass(CSS_SOURCES "${SASS_SOURCES}")

# -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED
qt5_add_resources(SOURCES papyro.qrc papyro_pages.qrc cslengine.qrc ../../libathenaeum/athenaeum/athenaeum.qrc)

if(WIN32)
  add_definitions(-DBUILD_LIBPAPYRO=1)
  add_definitions(-DBUILD_LIBATHENAEUM=1)
endif()

if(UTOPIA_BUILD_PERSISTENCE)
  add_definitions(-DBUILD_PERSISTENCE=1)
endif()

add_utopia_library(${PROJECT_NAME} SHARED ${SOURCES})

target_link_libraries(${PROJECT_NAME}
  ${ICU_LIBRARIES}
  ${PCRE_LIBRARIES}
  athenaeum
  graffiti
  spine
  utf8
  utopia2
  utopia2_auth
  utopia2_auth_qt
  utopia2_qt
)
qt5_use_modules(${PROJECT_NAME} Widgets Xml Script Network Svg Concurrent WebKitWidgets PrintSupport OpenGL)

install_utopia_library(${PROJECT_NAME} "${COMPONENT}")

add_subdirectory( citeproc )
