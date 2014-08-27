###############################################################################
#   
#    This file is part of the libcrackle library.
#        Copyright (c) 2008-2014 Lost Island Labs
#            <info@utopiadocs.com>
#    
#    The libcrackle library is free software: you can redistribute it and/or
#    modify it under the terms of the GNU AFFERO GENERAL PUBLIC LICENSE
#    VERSION 3 as published by the Free Software Foundation.
#    
#    The libcrackle library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero
#    General Public License for more details.
#    
#    You should have received a copy of the GNU Affero General Public License
#    along with the libcrackle library. If not, see
#    <http://www.gnu.org/licenses/>
#   
###############################################################################

add_subdirectory( libxpdf )

set(XPDF_INCLUDE_DIRS
  ${PROJECT_SOURCE_DIR}/crackle/libxpdf
  ${PROJECT_SOURCE_DIR}/crackle/libxpdf/xpdf-3.03
  ${PROJECT_SOURCE_DIR}/crackle/libxpdf/xpdf-3.03/goo
  ${PROJECT_SOURCE_DIR}/crackle/libxpdf/xpdf-3.03/fofi
  ${PROJECT_SOURCE_DIR}/crackle/libxpdf/xpdf-3.03/splash
  ${PROJECT_SOURCE_DIR}/crackle/libxpdf/xpdf-3.03/xpdf
  )

include_directories(
  ${spine_INCLUDE_DIR}
  ${Boost_INCLUDE_DIRS}
  ${PCRE_INCLUDE_DIR}
  ${utf8_INCLUDE_DIR}
  ${XPDF_INCLUDE_DIRS}
  )

# Hide warnings about const strings being passed to non-const xpdf functions
if(CMAKE_COMPILER_IS_GNUCC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-write-strings")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-write-strings")
endif(CMAKE_COMPILER_IS_GNUCC)

set(SOURCES
  CrackleTextOutputDev.cpp
  PDFDocument.cpp
  PDFPage.cpp
  PDFFontCollection.cpp
  PDFFont.cpp
  PDFTextRegion.cpp
  PDFTextBlock.cpp
  PDFTextLine.cpp
  PDFTextWord.cpp
  PDFTextCharacter.cpp
  crackleapi.cpp
)

add_utopia_library(crackle SHARED ${SOURCES})
target_link_libraries (crackle spine utf8 ${Boost_REGEX_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} ${PCRE_LIBRARIES} xpdf-non-commercial)
install_utopia_library(crackle "${COMPONENT}")

export(TARGETS utf8 crackle spine xpdf-non-commercial FILE crackle.cmake)
