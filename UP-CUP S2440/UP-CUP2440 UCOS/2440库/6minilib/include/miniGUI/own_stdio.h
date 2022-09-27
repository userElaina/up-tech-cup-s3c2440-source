/**
 * \file own_stdio.h
 * \author Wei Yongming <ymwei@minigui.org>
 * \date 2002/01/06
 * 
 *  ISO C standard I/O routines - with some POSIX 1003.1 extensions
 *
 \verbatim
    Copyright (C) 1998-2002 Wei Yongming.
    Copyright (C) 2002-2003 Feynman Software.

    This file is part of MiniGUI, a lightweight Graphics User Interface 
    support library for real-time embedded Linux.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 \endverbatim
 */

/*
 * $Id: own_stdio.h,v 1.2 2004/02/25 02:34:49 weiym Exp $
 *
 *             MiniGUI for Linux, uClinux, eCos, and uC/OS-II version 1.5.x
 *             Copyright (C) 1998-2002 Wei Yongming.
 *             Copyright (C) 2002-2004 Feynman Software.
 */

#ifndef OWN_STDIO_H
#define OWN_STDIO_H

#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>

#include "endianrw.h"

#ifdef __CC_ARM
#    define ENOMEM    173
#    define EINVAL    174
#    define EAGAIN    175
#    define ESRCH     176
#    define EDEADLK   177
#    define EBUSY     178
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern MG_RWops __mg_null_dev;

int own_printf( const char * format, ... );

int own_fprintf( FILE * stream, const char * format, ... );

int own_sprintf( char * str, const char * format, ... );

int own_fnprintf( FILE * stream, size_t length, const char * format, ... );

int own_snprintf( char * str, size_t length, const char * format, ... );

int own_vprintf( const char * format, va_list args );

int own_vfprintf( FILE * stream, const char * format, va_list args );

int own_vsprintf( char * str, const char * format, va_list args );

int own_vfnprintf( FILE * stream, size_t length, const char * format, va_list args );

int own_vsnprintf( char * str, size_t length, const char * format, va_list args );

int own_scanf( const char * format, ... );

int own_fscanf( FILE * stream, const char * format, ... );

int own_sscanf( const char * str, const char * format, ... );

int own_vscanf( const char * format, va_list args );

int own_vfscanf( FILE * stream, const char * format, va_list args );

int own_vsscanf( const char * str, const char * format, va_list args );

#ifdef __cplusplus
};  /* end of extern "C" */
#endif

#endif /* OWN_STDIO_H */

