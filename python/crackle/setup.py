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

#! /usr/bin/env python

import os
import sys
from setuptools import setup, Extension

from distutils.command.build import build
from setuptools.command.install import install


if sys.platform.startswith('linux'):
    boost_tag=''
else:
    boost_tag='-mt'


def _findall_with_symlink_dirs(dir=os.curdir):
    """Find all files under 'dir' and return the list of full filenames
    (relative to 'dir').
    """
    from stat import ST_MODE, S_ISREG, S_ISDIR, S_ISLNK

    list = []
    stack = [dir]
    pop = stack.pop
    push = stack.append

    while stack:
        dir = pop()
        names = os.listdir(dir)

        for name in names:
            if dir != os.curdir:        # avoid the dreaded "./" syndrome
                fullname = os.path.join(dir, name)
            else:
                fullname = name

            # Avoid excess stat calls -- just one will do, thank you!
            stat = os.stat(fullname)
            mode = stat[ST_MODE]

            if S_ISREG(mode) and not S_ISLNK(mode):
                list.append(fullname)
            elif S_ISDIR(mode):
                push(fullname)
    return list

# monkey patch distutils annoyance
import distutils.filelist
distutils.filelist.findall=_findall_with_symlink_dirs

class CustomBuild(build):
    def run(self):
        self.run_command('build_ext')
        build.run(self)


class CustomInstall(install):
    def run(self):
        self.run_command('build_ext')
        self.do_egg_install()

use_xpdf= False # default

if '--xpdf' in sys.argv:
    use_xpdf= True
    sys.argv.remove('--xpdf')
elif '--poppler' in sys.argv:
    use_xpdf= False
    sys.argv.remove('--poppler')

spine_sources=[
    'cpp/libspine/spine/Annotation.cpp',
    'cpp/libspine/spine/Character.cpp',
    'cpp/libspine/spine/Document.cpp',
    'cpp/libspine/spine/spineapi.cpp',
    'cpp/libspine/spine/TextSelection.cpp',
    'cpp/libspine/spine/fingerprint.cpp',
    'python/spine/spineapi.i',
    ]

utf8_sources=[
    'cpp/libutf8/utf8/utf8proc/utf8proc.c',
]

crackle_sources= [
    'cpp/libcrackle/crackle/crackleapi.cpp',
    'cpp/libcrackle/crackle/CrackleTextOutputDev.cpp',
    'cpp/libcrackle/crackle/PDFDocument.cpp',
    'cpp/libcrackle/crackle/PDFFont.cpp',
    'cpp/libcrackle/crackle/PDFPage.cpp',
    'cpp/libcrackle/crackle/PDFTextBlock.cpp',
    'cpp/libcrackle/crackle/PDFTextCharacter.cpp',
    'cpp/libcrackle/crackle/PDFTextLine.cpp',
    'cpp/libcrackle/crackle/PDFTextRegion.cpp',
    'cpp/libcrackle/crackle/PDFTextWord.cpp',
    'python/crackle/crackleapi.i',
]

xpdf_sources = [
    'cpp/libcrackle/crackle/libxpdf/GlobalParams.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/fofi/FoFiBase.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/fofi/FoFiEncodings.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/fofi/FoFiIdentifier.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/fofi/FoFiTrueType.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/fofi/FoFiType1.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/fofi/FoFiType1C.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/goo/FixedPoint.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/goo/gfile.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/goo/GHash.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/goo/GList.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/goo/gmem.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/goo/gmempp.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/goo/GString.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/goo/parseargs.c',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/splash/Splash.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/splash/SplashBitmap.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/splash/SplashClip.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/splash/SplashFont.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/splash/SplashFontEngine.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/splash/SplashFontFile.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/splash/SplashFontFileID.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/splash/SplashFTFont.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/splash/SplashFTFontEngine.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/splash/SplashFTFontFile.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/splash/SplashPath.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/splash/SplashPattern.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/splash/SplashScreen.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/splash/SplashState.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/splash/SplashXPath.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/splash/SplashXPathScanner.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/AcroForm.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/Annot.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/Array.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/BuiltinFont.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/BuiltinFontTables.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/Catalog.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/CoreOutputDev.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/CharCodeToUnicode.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/CMap.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/Decrypt.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/Dict.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/Error.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/Form.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/FontEncodingTables.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/Function.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/Gfx.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/GfxFont.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/GfxState.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/ImageOutputDev.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/JArithmeticDecoder.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/JBIG2Stream.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/JPXStream.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/Lexer.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/Link.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/NameToCharCode.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/Object.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/OptionalContent.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/Outline.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/OutputDev.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/Page.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/Parser.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/pdfdetach.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/PDFDoc.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/PDFDocEncoding.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/PreScanOutputDev.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/PSOutputDev.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/PSTokenizer.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/SecurityHandler.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/SplashOutputDev.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/Stream.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/TextOutputDev.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/TextString.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/UnicodeMap.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/UnicodeTypeTable.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/XFAForm.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/XRef.cc',
    'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf/Zoox.cc',
]

crackle_include_dirs = [
    'cpp/libutf8',
    'cpp/libspine',
    'cpp/libcrackle',
    '/usr/local/include',
    '/usr/include/freetype2/',
    'python',
    '../../dependencies/include',
    '../../dependencies/include/freetype2',
]

crackle_libraries = [
    'z',
    'freetype',
    'pcre',
    'pcrecpp',
    'ssl',
    'boost_system{0}'.format(boost_tag),
    'crypto'
]

crackle_sources = utf8_sources + crackle_sources + spine_sources

if use_xpdf:
    crackle_include_dirs += [
        'cpp/libcrackle/crackle/libxpdf',
        'cpp/libcrackle/crackle/libxpdf/xpdf-3.04',
        'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/goo',
        'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/fofi',
        'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/splash',
        'cpp/libcrackle/crackle/libxpdf/xpdf-3.04/xpdf',
    ]

    crackle_sources += xpdf_sources

    backend_define_macro = [
        ('UTOPIA_SPINE_BACKEND_XPDF', "1")
    ]
else:

    crackle_include_dirs += [
        'cpp/libcrackle/crackle/libpoppler-compat',
        'cpp/libcrackle/crackle/libpoppler-compat/goo',
        '/usr/include/fontconfig',
        '/usr/include/poppler',
        '/usr/include/poppler/goo',
        '/usr/include/poppler/fofi',
        '/usr/include/poppler/splash',
        '../../dependencies/include/fontconfig',
        '../../dependencies/include/poppler',
        '../../dependencies/include/poppler/goo',
        '../../dependencies/include/poppler/fofi',
        '../../dependencies/include/poppler/splash',
    ]

    crackle_libraries += [
        'fontconfig',
        'poppler',
    ]

    backend_define_macro = [
        ('UTOPIA_SPINE_BACKEND_POPPLER', "1")
    ]

spineapi = Extension('_spineapi',
                     define_macros = [
                         ('MAJOR_VERSION', '1'),
                         ('MINOR_VERSION', '0')
                     ] + backend_define_macro,
                     include_dirs = [
                        'python/spine',
                        'cpp/libutf8',
                        'cpp/libspine',
                         '/usr/local/include',
                         '../../dependencies/include'
                     ],
                     libraries = ['z',
                                  'pcre',
                                  'pcrecpp',
                                  'ssl',
                                  'boost_system{0}'.format(boost_tag),
                                  'crypto'
                              ],
                     library_dirs = ['/usr/local/lib', '../../dependencies/lib'],
                     sources = utf8_sources + spine_sources,
#                     extra_compile_args = ['-Wno-strict-prototypes'],
                     swig_opts=['-I/usr/local/include', '-outdir', '.'],
)


crackleapi = Extension('_crackleapi',
                       define_macros = [
                           ('MAJOR_VERSION', '1'),
                           ('MINOR_VERSION', '0'),
                           ('HAVE_POPPLER_SPLASH_SET_FONT_ANTIALIAS', '1'),
                           ('HAVE_POPPLER_SPLASH_SET_VECTOR_ANTIALIAS', '1'),
                       ] + backend_define_macro,
                       include_dirs = crackle_include_dirs,
                       libraries = crackle_libraries,
                       library_dirs = ['/usr/local/lib', '../../dependencies/lib'],
                       sources = crackle_sources,
                       extra_compile_args = [
#                           '-Wno-strict-prototypes',
                                             '-Wno-write-strings'],
                       swig_opts=['-I/usr/local/include', '-Ipython/spine', '-outdir', '.'],
)

setup (name = 'crackle',
       version = '1.0',
       description = 'Standalone Utopia PDF API',
       author = 'James Marsh',
       author_email = 'james.marsh@manchester.ac.uk',
       url = 'http://utopia.cs.manchester.ac.uk',
       long_description = '''Standalone Utopia PDF API.''',
       cmdclass={'build': CustomBuild, 'install': CustomInstall},
       py_modules=['spineapi', 'crackleapi'],
       ext_modules = [spineapi, crackleapi],
       install_requires=[
        'Pillow',
       ],
       zip_safe=True,
       )
