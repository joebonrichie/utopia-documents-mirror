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

cmake_minimum_required (VERSION 2.6)

project ( libxpdf)

# Specify the xpdf files we need

# Note: GlobalParams.cc has been modified from xpdf to prevent
# loading of a config file
set ( libxpdf_SRCS
  GlobalParams.cc
  xpdf-3.03/fofi/FoFiBase.cc
  xpdf-3.03/fofi/FoFiEncodings.cc
  xpdf-3.03/fofi/FoFiIdentifier.cc
  xpdf-3.03/fofi/FoFiTrueType.cc
  xpdf-3.03/fofi/FoFiType1.cc
  xpdf-3.03/fofi/FoFiType1C.cc
  xpdf-3.03/goo/FixedPoint.cc
  xpdf-3.03/goo/gfile.cc
  xpdf-3.03/goo/GHash.cc
  xpdf-3.03/goo/GList.cc
  xpdf-3.03/goo/gmem.cc
  xpdf-3.03/goo/gmempp.cc
  xpdf-3.03/goo/GString.cc
  xpdf-3.03/goo/parseargs.c
  xpdf-3.03/splash/Splash.cc
  xpdf-3.03/splash/SplashBitmap.cc
  xpdf-3.03/splash/SplashClip.cc
  xpdf-3.03/splash/SplashFont.cc
  xpdf-3.03/splash/SplashFontEngine.cc
  xpdf-3.03/splash/SplashFontFile.cc
  xpdf-3.03/splash/SplashFontFileID.cc
  xpdf-3.03/splash/SplashFTFont.cc
  xpdf-3.03/splash/SplashFTFontEngine.cc
  xpdf-3.03/splash/SplashFTFontFile.cc
  xpdf-3.03/splash/SplashPath.cc
  xpdf-3.03/splash/SplashPattern.cc
  xpdf-3.03/splash/SplashScreen.cc
  xpdf-3.03/splash/SplashState.cc
  xpdf-3.03/splash/SplashT1Font.cc
  xpdf-3.03/splash/SplashT1FontEngine.cc
  xpdf-3.03/splash/SplashT1FontFile.cc
  xpdf-3.03/splash/SplashXPath.cc
  xpdf-3.03/splash/SplashXPathScanner.cc
  xpdf-3.03/xpdf/Annot.cc
  xpdf-3.03/xpdf/Array.cc
  xpdf-3.03/xpdf/BuiltinFont.cc
  xpdf-3.03/xpdf/BuiltinFontTables.cc
  xpdf-3.03/xpdf/Catalog.cc
  xpdf-3.03/xpdf/CharCodeToUnicode.cc
  xpdf-3.03/xpdf/CMap.cc
  xpdf-3.03/xpdf/Decrypt.cc
  xpdf-3.03/xpdf/Dict.cc
  xpdf-3.03/xpdf/Error.cc
  xpdf-3.03/xpdf/FontEncodingTables.cc
  xpdf-3.03/xpdf/Function.cc
  xpdf-3.03/xpdf/Gfx.cc
  xpdf-3.03/xpdf/GfxFont.cc
  xpdf-3.03/xpdf/GfxState.cc
#  xpdf-3.03/xpdf/GlobalParams.cc
  xpdf-3.03/xpdf/ImageOutputDev.cc
  xpdf-3.03/xpdf/JArithmeticDecoder.cc
  xpdf-3.03/xpdf/JBIG2Stream.cc
  xpdf-3.03/xpdf/JPXStream.cc
  xpdf-3.03/xpdf/Lexer.cc
  xpdf-3.03/xpdf/Link.cc
  xpdf-3.03/xpdf/NameToCharCode.cc
  xpdf-3.03/xpdf/Object.cc
  xpdf-3.03/xpdf/OptionalContent.cc
  xpdf-3.03/xpdf/Outline.cc
  xpdf-3.03/xpdf/OutputDev.cc
  xpdf-3.03/xpdf/Page.cc
  xpdf-3.03/xpdf/Parser.cc
  xpdf-3.03/xpdf/pdfdetach.cc
  xpdf-3.03/xpdf/PDFDoc.cc
  xpdf-3.03/xpdf/PDFDocEncoding.cc
  xpdf-3.03/xpdf/PreScanOutputDev.cc
  xpdf-3.03/xpdf/PSOutputDev.cc
  xpdf-3.03/xpdf/PSTokenizer.cc
  xpdf-3.03/xpdf/SecurityHandler.cc
  xpdf-3.03/xpdf/SplashOutputDev.cc
  xpdf-3.03/xpdf/Stream.cc
  xpdf-3.03/xpdf/TextOutputDev.cc
  xpdf-3.03/xpdf/UnicodeMap.cc
  xpdf-3.03/xpdf/UnicodeTypeTable.cc
#  xpdf-3.03/xpdf/XpdfPluginAPI.cc
  xpdf-3.03/xpdf/XRef.cc
  )

set(XPDF_INCLUDE_DIRS
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/xpdf-3.03
  ${PROJECT_SOURCE_DIR}/xpdf-3.03/goo
  ${PROJECT_SOURCE_DIR}/xpdf-3.03/fofi
  ${PROJECT_SOURCE_DIR}/xpdf-3.03/splash
  ${PROJECT_SOURCE_DIR}/xpdf-3.03/xpdf
  )

include_directories (
  ${XPDF_INCLUDE_DIRS}
  ${FREETYPE_INCLUDE_DIRS}
  )

add_utopia_library( xpdf-non-commercial STATIC ${libxpdf_SRCS} )

target_link_libraries (xpdf-non-commercial ${FREETYPE_LIBRARIES})
if(WIN32)
  set_target_properties (xpdf-non-commercial PROPERTIES COMPILE_FLAGS "-O0")
endif()

# Hide warnings about const strings being passed to non-const functions
if(CMAKE_COMPILER_IS_GNUCC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-write-strings")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-write-strings")
endif(CMAKE_COMPILER_IS_GNUCC)
