/*****************************************************************************
 *  
 *   This file is part of the libcrackle library.
 *       Copyright (c) 2008-2014 Lost Island Labs
 *   
 *   The libcrackle library is free software: you can redistribute it and/or
 *   modify it under the terms of the GNU AFFERO GENERAL PUBLIC LICENSE
 *   VERSION 3 as published by the Free Software Foundation.
 *   
 *   The libcrackle library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero
 *   General Public License for more details.
 *   
 *   You should have received a copy of the GNU Affero General Public License
 *   along with the libcrackle library. If not, see
 *   <http://www.gnu.org/licenses/>
 *  
 *****************************************************************************/

/* aconf.h.  Generated from aconf.h.in by configure.  */
/*
 * aconf.h
 *
 * Copyright 2002-2003 Glyph & Cog, LLC
 */

#ifndef ACONF_H
#define ACONF_H

#include <aconf2.h>

/*
 * Use A4 paper size instead of Letter for PostScript output.
 */
#define A4_PAPER 1

/*
 * Do not allow text selection.
 */
/* #undef NO_TEXT_SELECT */

/*
 * Include support for OPI comments.
 */
#define OPI_SUPPORT 1

/*
 * Enable multithreading support.
 */
#define MULTITHREADED 1

/*
 * Enable C++ exceptions.
 */
#define USE_EXCEPTIONS 1

/*
 * Enable word list support.
 */
#define TEXTOUT_WORD_LIST 1

/*
 * Use fixed point (instead of floating point) arithmetic.
 */
/* #undef USE_FIXEDPOINT */

/*
 * Directory with the Xpdf app-defaults file.
 */
/* #undef APPDEFDIR */

/*
 * Full path for the system-wide xpdfrc file.
 */
#define SYSTEM_XPDFRC "/etc/xpdfrc"

/*
 * Various include files and functions.
 */
#define HAVE_DIRENT_H 1
/* #undef HAVE_SYS_NDIR_H */
/* #undef HAVE_SYS_DIR_H */
/* #undef HAVE_NDIR_H */
/* #undef HAVE_SYS_SELECT_H */
/* #undef HAVE_SYS_BSDTYPES_H */
#define HAVE_STRINGS_H 1
/* #undef HAVE_BSTRING_H */
#define HAVE_POPEN 1
#define HAVE_MKSTEMP 1
#define HAVE_MKSTEMPS 1
/* #undef SELECT_TAKES_INT */
#define HAVE_STD_SORT 1
#ifndef __WIN32__
#  define HAVE_FSEEKO 1
#endif
/* #undef HAVE_FSEEK64 */
/* #undef _FILE_OFFSET_BITS */
/* #undef _LARGE_FILES */
/* #undef _LARGEFILE_SOURCE */
/* #undef HAVE_XTAPPSETEXITFLAG */

/*
 * This is defined if using libXpm.
 */
/* #undef HAVE_X11_XPM_H */

/*
 * This is defined if using t1lib.
 */
/* #undef HAVE_T1LIB_H */

/*
 * One of these is defined if using FreeType 2.
 */
/* #undef HAVE_FREETYPE_H */
#define HAVE_FREETYPE_FREETYPE_H 1

/*
 * This is defined if using libpaper.
 */
/* #undef HAVE_PAPER_H */

/*
 * Enable support for loading plugins.
 */
/* #undef ENABLE_PLUGINS */

/*
 * Defined if the Splash library is avaiable.
 */
#define HAVE_SPLASH 1

/*
 * Enable support for CMYK output.
 */
/* #undef SPLASH_CMYK */

#endif
