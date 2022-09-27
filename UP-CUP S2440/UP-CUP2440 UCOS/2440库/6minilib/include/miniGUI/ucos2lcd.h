/*
**  $Id: ucos2lcd.h,v 1.2 2004/02/25 02:34:51 weiym Exp $
**  
**  Copyring (C) 2003 Feynman Software.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef _GAL_ucos2lcd_h
#define _GAL_ucos2lcd_h

//#include "sysvideo.h"

/* Hidden "this" pointer for the video functions */
#define _THIS	GAL_VideoDevice *this

/* Private display data */

struct GAL_PrivateVideoData {
    int w, h, pitch;
    void *fb;
};

struct lcd_info {
    short height, width;  // Pixels
    short bpp;            // Depth (bits/pixel)
    short type;
    short rlen;           // Length of one raster line in bytes
    void  *fb;            // Frame buffer
};

#define FB_TRUE_RGB565  1
#define FB_TRUE_RGB332	2

#endif /* _GAL_ucos2lcd_h */
