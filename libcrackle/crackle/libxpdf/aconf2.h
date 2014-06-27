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

/*
 * aconf2.h
 *
 * This gets included by aconf.h, and contains miscellaneous global
 * settings not directly controlled by autoconf.  This is a separate
 * file because otherwise the configure script will munge any
 * #define/#undef constructs.
 *
 * Copyright 2002-2003 Glyph & Cog, LLC
 */

#ifndef ACONF2_H
#define ACONF2_H

/*
 * This controls the use of the interface/implementation pragmas.
 */
#ifdef __GNUC__
#define USE_GCC_PRAGMAS
#endif
/* There is a bug in the version of gcc which ships with MacOS X 10.2 */
#if defined(__APPLE__) && defined(__MACH__)
#  include <AvailabilityMacros.h>
#endif
#ifdef MAC_OS_X_VERSION_MAX_ALLOWED
#  if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_2
#    undef USE_GCC_PRAGMAS
#  endif
#endif

/*
 * Make sure WIN32 is defined if appropriate.
 */
#if defined(_WIN32) && !defined(WIN32)
#  define WIN32
#endif

#endif
