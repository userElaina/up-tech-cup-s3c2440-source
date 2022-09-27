//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
// -----------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
// -----------------------------------------------------------------------------

#include "precomp.h"
#define DBGLCD 0
// callbacks from the DIRECTDRAW object
DDHAL_DDCALLBACKS cbDDCallbacks =
{
    	sizeof( DDHAL_DDCALLBACKS ),                // dwSize        
       DDHAL_CB32_CREATESURFACE |
       DDHAL_CB32_WAITFORVERTICALBLANK |
       DDHAL_CB32_CANCREATESURFACE |
       DDHAL_CB32_CREATEPALETTE |
       DDHAL_CB32_GETSCANLINE |
       0,
	HalCreateSurface,//DDGPECreateSurface,                     	// CreateSurface
	DDGPEWaitForVerticalBlank,          	// WaitForVerticalBlank
	HalCanCreateSurface,//DDGPECanCreateSurface,               	// CanCreateSurface
	DDGPECreatePalette,                     		// CreatePalette
	HalGetScanLine                              		// GetScanLine
};

// callbacks from the DIRECTDRAWPALETTE object
DDHAL_DDPALETTECALLBACKS cbDDPaletteCallbacks =
{
	sizeof( DDHAL_DDPALETTECALLBACKS ), 	// dwSize
	DDHAL_PALCB32_DESTROYPALETTE |       	// dwFlags
	DDHAL_PALCB32_SETENTRIES |
	0,
	DDGPEDestroyPalette,                           // DestroyPalette
	DDGPESetEntries                                    // SetEntries
};


// callbacks from the DIRECTDRAWSURFACE object

DDHAL_DDSURFACECALLBACKS cbDDSurfaceCallbacks =
{
	sizeof( DDHAL_DDSURFACECALLBACKS ), 	// dwSize
	DDHAL_SURFCB32_DESTROYSURFACE |    // dwFlags
	DDHAL_SURFCB32_FLIP |        
	DDHAL_SURFCB32_LOCK |
	DDHAL_SURFCB32_UNLOCK |
	DDHAL_SURFCB32_SETCOLORKEY |
	DDHAL_SURFCB32_GETBLTSTATUS |
	DDHAL_SURFCB32_GETFLIPSTATUS |	
	DDHAL_SURFCB32_SETPALETTE |
	0,
	DDGPEDestroySurface,                        	// DestroySurface
	HalFlip,                                            	// Flip
	HalLock,								// Lock
	HalUnlock,                                        	// Unlock
	DDGPESetColorKey,                           	// SetColorKey
	HalGetBltStatus,                            		// GetBltStatus
	DDGPEGetFlipStatus,                         	// GetFlipStatus
	NULL,                           				// UpdateOverlay
	NULL,                      					// SetOverlayPosition
	DDGPESetPalette                            		// SetPalette
};


DDGPESurf*          g_pDDrawPrimarySurface  	= NULL;

// InitDDHALInfo must set up this information
unsigned long           g_nVideoMemorySize    	= 0L;
unsigned char *        g_pVideoMemory          	= NULL; // virtual address of video memory from client's side
DWORD                   g_nTransparentColor     = 0L;

EXTERN_C void buildDDHALInfo( LPDDHALINFO lpddhi, DWORD modeidx )
{
  	memset( lpddhi, 0, sizeof(DDHALINFO) );         // Clear the DDHALINFO structure  	

  	// The g_pGPE pointer has laready been filled in with our GPE pointer 
  	// which we know is really a 2440 pointer.
  	S3C2440DISP *pGPE = static_cast<S3C2440DISP *>(GetDDGPE());
	RETAILMSG(DBGLCD, (TEXT("In buildDDHALInfo\r\n")));
	
  	if( !g_pVideoMemory )   // in case this is called more than once...
  	{
		unsigned long VideoMemoryStart;
		pGPE->GetVirtualVideoMemory( &VideoMemoryStart, &g_nVideoMemorySize );
		DEBUGMSG( GPE_ZONE_INIT,(TEXT("GetVirtualVideoMemory returned addr=0x%08x size=%d\r\n"), VideoMemoryStart, g_nVideoMemorySize));

		g_pVideoMemory = (BYTE*)VideoMemoryStart;
		DEBUGMSG( GPE_ZONE_INIT,(TEXT("gpVidMem=%08x\r\n"), g_pVideoMemory ));
  	}

  	lpddhi->dwSize = sizeof(DDHALINFO);
  	lpddhi->lpDDCallbacks 		= &cbDDCallbacks;
  	lpddhi->lpDDSurfaceCallbacks 	= &cbDDSurfaceCallbacks;
  	lpddhi->lpDDPaletteCallbacks 	= &cbDDPaletteCallbacks;
  	lpddhi->GetDriverInfo 			= HalGetDriverInfo;
	lpddhi->lpdwFourCC 			= 0;

    	// hw specific caps:
       lpddhi->ddCaps.dwSize = sizeof(DDCAPS);         // size of the DDDRIVERCAPS structure
       
        lpddhi->ddCaps.dwVidMemTotal = g_nVideoMemorySize;      // total amount of video memory
        lpddhi->ddCaps.dwVidMemFree = g_nVideoMemorySize;       // amount of free video memory
        lpddhi->ddCaps.dwVidMemStride = 0;

                                                                                                // maximum number of visible overlays
        lpddhi->ddCaps.dwCurrVisibleOverlays = 0;       	// current number of visible overlays
        lpddhi->ddCaps.dwNumFourCCCodes = 0;            	// number of four cc codes
        lpddhi->ddCaps.dwAlignBoundarySrc = 0;          	// source rectangle alignment
        lpddhi->ddCaps.dwAlignSizeSrc = 0;                     // source rectangle byte size
        
        // DirectDraw Blttting caps refer to hardware blitting support only.
	lpddhi->ddCaps.dwBltCaps = 0;
	lpddhi->ddCaps.dwCKeyCaps = 0;
	lpddhi->ddCaps.dwAlphaCaps = 0;
	lpddhi->ddCaps.dwPalCaps=0;                              	// palette capabilities

	// Overlay caps.
	lpddhi->ddCaps.dwOverlayCaps = 0;                   
	lpddhi->ddCaps.dwMaxVisibleOverlays = 0;
	lpddhi->ddCaps.dwCurrVisibleOverlays = 0;

	lpddhi->ddCaps.dwAlignBoundarySrc = 0;
	lpddhi->ddCaps.dwAlignSizeSrc = 0;
	lpddhi->ddCaps.dwAlignBoundaryDest = 0;
	lpddhi->ddCaps.dwAlignSizeDest = 0;

	lpddhi->ddCaps.dwMinOverlayStretch = 0;
	lpddhi->ddCaps.dwMaxOverlayStretch = 0;

        lpddhi->ddCaps.ddsCaps.dwCaps=                        // DDSCAPS structure has all the general capabilities
                // DDSCAPS_ALPHA |                              	// Can create alpha-only surfaces
                DDSCAPS_BACKBUFFER |                            	// Can create backbuffer surfaces                
                DDSCAPS_FLIP |                               		// Can flip between surfaces
                DDSCAPS_FRONTBUFFER |                           	// Can create front-buffer surfaces
                // DDSCAPS_OVERLAY |                            	// Can create overlay surfaces
                DDSCAPS_PALETTE |                               	// Has one palette ???
                DDSCAPS_PRIMARYSURFACE |                       // Has a primary surface
                // DDSCAPS_PRIMARYSURFACELEFT |            // Has a left-eye primary surface
                // DDSCAPS_TEXTURE |                            	// Supports texture surrfaces
                DDSCAPS_SYSTEMMEMORY |                     	// Surfaces are in system memory
                DDSCAPS_VIDEOMEMORY |                           // Surfaces are in video memory 
                // DDSCAPS_ZBUFFER |                            	// Can create (pseudo) Z buffer
                // DDSCAPS_EXECUTEBUFFER |                     // Can create execute buffer
                // DDSCAPS_3DDEVICE |                           	// Surfaces can be 3d targets
                // DDSCAPS_WRITEONLY |                          	// Can create write-only surfaces
                // DDSCAPS_ALLOCONLOAD |                       // Can create alloconload surfaces
                // DDSCAPS_MIPMAP |                             	// Can create mipmap
                0;

    	SETROPBIT(lpddhi->ddCaps.dwRops,SRCCOPY);                   // Set bits for ROPS supported
    	SETROPBIT(lpddhi->ddCaps.dwRops,PATCOPY);
    	SETROPBIT(lpddhi->ddCaps.dwRops,BLACKNESS);
    	SETROPBIT(lpddhi->ddCaps.dwRops,WHITENESS);
}


