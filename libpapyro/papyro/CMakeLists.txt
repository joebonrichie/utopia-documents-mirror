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
    abstractprocessor.cpp
    annotationprocessor.cpp
    annotationprocessoraction.cpp
    annotationresultitem.cpp
    annotator.cpp
    annotatorrunnable.cpp
    annotatorrunnablepool.cpp
    capabilities.cpp
    citationpopup.cpp
    citationfinder.cpp
    commentdata.cpp
    cslengine.cpp
    cslengineadapter.cpp
    decorator.cpp
    dispatcher.cpp
    documentfactory.cpp
    documentmanager.cpp
    documentsignalproxy.cpp
    documentview.cpp
    embeddedframe.cpp
    embeddedpanefactory.cpp
    overlayrenderer.cpp
    overlayrenderermapper.cpp
    pager.cpp
    pageview.cpp
    papyrotab.cpp
    papyrowindow.cpp
    phraselookup.cpp
    playercontrols.cpp
    progresslozenge.cpp
    printer.cpp
    raisetabaction.cpp
    resultitem.cpp
    resultsview.cpp
    selectionprocessor.cpp
    selectionprocessoraction.cpp
    searchbar.cpp
    sidebar.cpp
    sliver.cpp
    tabbar.cpp
    uimodifier.cpp
    utils.cpp
    visualiser.cpp
    )

set(MOC_HEADERS
    annotationprocessoraction.h
    annotationresultitem_p.h
    annotatorrunnable.h
    annotatorrunnablepool.h
    annotatorrunnablepool_p.h
    citationfinder_p.h
    citationpopup_p.h
    citationpopup.h
    cslengine.h
    dispatcher.h
    dispatcher_p.h
    documentmanager.h
    documentmanager_p.h
    documentsignalproxy.h
    documentview.h
    documentview_p.h
    embeddedframe.h
    embeddedframe_p.h
    pager_p.h
    pager.h
    pageview.h
    pageview_p.h
    papyrotab.h
    papyrotab_p.h
    papyrowindow.h
    papyrowindow_p.h
    playercontrols.h
    progresslozenge.h
    printer.h
    printer_p.h
    raisetabaction.h
    raisetabaction_p.h
    resultitem.h
    resultsview.h
    resultsview_p.h
    selectionprocessoraction.h
    searchbar.h
    sidebar_p.h
    sidebar.h
    sliver.h
    sliver_p.h
    tabbar.h
    tabbar_p.h
    visualiser_p.h
    )

set(FORMS
    )

include_directories(
  ${PROJECT_BINARY_DIR}
  ${utopia2_INCLUDE_DIR}
  ${utopia2_auth_INCLUDE_DIR}
  ${utopia2_qt_INCLUDE_DIR}
  ${utopia2_auth_qt_INCLUDE_DIR}
  ${graffiti_INCLUDE_DIR}
  ${athenaeum_INCLUDE_DIR}
  ${utf8_INCLUDE_DIR}
  ${PCRE_INCLUDE_DIR}
  ${Boost_INCLUDE_DIR}
  ${spine_INCLUDE_DIR}
  ${CMAKE_BINARY_DIR}
  )

set(RESOURCES papyro.qrc papyro_pages.qrc cslengine.qrc ../../libathenaeum/athenaeum/athenaeum.qrc)

QT4_WRAP_CPP(MOC_SOURCES ${MOC_HEADERS} OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED)
QT4_WRAP_UI(UI_SOURCES ${FORMS})
QT4_ADD_RESOURCES(RES_SOURCES ${RESOURCES})

if(WIN32)
	add_definitions(-DBUILD_LIBPAPYRO=1)
endif()

if(UTOPIA_BUILD_PERSISTENCE)
  add_definitions(-DBUILD_PERSISTENCE=1)
endif()

add_utopia_library(${PROJECT_NAME} SHARED ${SOURCES} ${MOC_SOURCES} ${UI_SOURCES} ${RES_SOURCES})

target_link_libraries(${PROJECT_NAME}
  utopia2
  utopia2_auth
  utopia2_auth_qt
  utopia2_qt
  athenaeum
  ${QT_LIBRARIES}
  spine
  graffiti
  utf8
  ${PCRE_LIBRARIES}
)

install_utopia_library(${PROJECT_NAME} "${COMPONENT}")

add_subdirectory( citeproc )
