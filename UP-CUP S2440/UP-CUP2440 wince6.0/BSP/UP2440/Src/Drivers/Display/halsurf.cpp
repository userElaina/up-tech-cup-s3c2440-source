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

DWORD WINAPI HalCreateSurface( LPDDHAL_CREATESURFACEDATA pd )
{
	DEBUGENTER( HalCreateSurface );
	RETAILMSG(DBGLCD, (TEXT("HalCreateSurface\r\n")));
	
	// Implementation
	//EGPEFormat format;
	//EDDGPEPixelFormat pixelFormat;
	EDDGPEPixelFormat primaryPixelFormat;
	DWORD dwFlags = pd->lpDDSurfaceDesc->dwFlags;
	DWORD dwCaps = pd->lpDDSurfaceDesc->ddsCaps.dwCaps;

	RETAILMSG(DBGLCD, (TEXT("++**********************************************************************\r\n")));
	RETAILMSG(DBGLCD, (TEXT("R = 0x%x, G = 0x%x, B = 0x%x WIDTH=%d, HEIGHT=%d\r\n"), pd->lpDDSurfaceDesc->ddpfPixelFormat.dwRBitMask
		, pd->lpDDSurfaceDesc->ddpfPixelFormat.dwGBitMask, pd->lpDDSurfaceDesc->ddpfPixelFormat.dwBBitMask, pd->lpDDSurfaceDesc->dwWidth, pd->lpDDSurfaceDesc->dwHeight));
	RETAILMSG(DBGLCD, (TEXT("--**********************************************************************\r\n")));

	// get the pixel format for the primary surface
	GPEMode modeInfo;
	SCODE sc = ((S3C2440DISP*)GetDDGPE())->GetModeInfo(&modeInfo, 0);
	if (FAILED(sc))
	{
		RETAILMSG(1, (TEXT("Error: can't get mode info\r\n")));
		pd->ddRVal = DDERR_GENERIC;
		return DDHAL_DRIVER_HANDLED;
	}

	GPEModeEx modeInfoEx;
	sc = ((S3C2440DISP*)GetDDGPE())->GetModeInfoEx(&modeInfoEx, 0);
	if (FAILED(sc))
	{
		RETAILMSG(1, (TEXT("In if------------>>>\r\n")));
	    	primaryPixelFormat = EGPEFormatToEDDGPEPixelFormat[modeInfo.format];
	}
	else
	{
		RETAILMSG(1, (TEXT("In else------------>>>\r\n")));
	    	primaryPixelFormat = modeInfoEx.ePixelFormat;
//		pd->lpDDSurfaceDesc->ddpfPixelFormat.dwRBitMask = modeInfoEx.dwRBitMask;
//		pd->lpDDSurfaceDesc->ddpfPixelFormat.dwGBitMask = modeInfoEx.dwGBitMask;
//		pd->lpDDSurfaceDesc->ddpfPixelFormat.dwBBitMask = modeInfoEx.dwBBitMask;		
	}
#if 0	
	RETAILMSG(1, (TEXT("---------------->>>Primary pixel format is %d\r\n"), primaryPixelFormat));
	RETAILMSG(1, (TEXT("R = 0x%x, G = 0x%x, B = 0x%x\r\n"), pd->lpDDSurfaceDesc->ddpfPixelFormat.dwRBitMask
		, pd->lpDDSurfaceDesc->ddpfPixelFormat.dwGBitMask, pd->lpDDSurfaceDesc->ddpfPixelFormat.dwBBitMask));
	//{
	// get the pixel format for the surface to be created
	if (dwFlags & DDSD_PIXELFORMAT)
	{
	    	pd->ddRVal = DetectPixelFormat(
	                     &pd->lpDDSurfaceDesc->ddpfPixelFormat,
	                     &format,
	                     &pixelFormat);
	    	if (FAILED(pd->ddRVal))
	    	{
	        	goto TryDDGPE;
	    	}
	}
	else
	{
	    	pixelFormat = primaryPixelFormat;
	}

   	if ((!(dwCaps & DDSCAPS_OVERLAY)) &&
        	(dwCaps & DDSCAPS_VIDEOMEMORY) &&
        	(pixelFormat != primaryPixelFormat))
    	{
        	DEBUGMSG(1, (TEXT("HalCreateSurface: non-overlay video memory surface must have same format as primary surface\r\n")));
        	pd->ddRVal = DDERR_UNSUPPORTEDFORMAT;
        	return DDHAL_DRIVER_HANDLED;
    	}
#endif	//}	

//TryDDGPE:
    	return DDGPECreateSurface(pd);
}

//////////////////////////// DDHAL_DDEXEBUFCALLBACKS ////////////////////////////

DWORD WINAPI HalCanCreateSurface( LPDDHAL_CANCREATESURFACEDATA pd )
{	
    RETAILMSG(DBGLCD, (TEXT("------------------------------------------------------->>>>>>> HalCanCreateSurface: OK\r\n")));
    //pd->ddRVal = DD_OK;
    return DDGPECanCreateSurface(pd);
}

DWORD WINAPI HalCreateExecuteBuffer( LPDDHAL_CREATESURFACEDATA pd )
{
	DEBUGENTER( HalCreateExecuteBuffer );
	RETAILMSG(0, (TEXT("HalCreateExecuteBuffer\r\n")));
	
	// Implementation
	pd->ddRVal = DD_OK;

	return DDHAL_DRIVER_HANDLED;
}


DWORD WINAPI HalDestroyExecuteBuffer( LPDDHAL_DESTROYSURFACEDATA pd )
{
	DEBUGENTER( HalDestroyExecutebuffer );
	RETAILMSG(0, (TEXT("HalDestroyExecuteBuffer\r\n")));
	
	// Implementation
	pd->ddRVal = DD_OK;

	return DDHAL_DRIVER_HANDLED;
}

DWORD WINAPI HalLock( LPDDHAL_LOCKDATA pd )
{
	DEBUGENTER( HalLock );
	RETAILMSG(0, (TEXT("HalLock\r\n")));
	DDGPE* pDDGPE = GetDDGPE();
	DDGPESurf* pSurf = DDGPESurf::GetDDGPESurf(pd->lpDDSurface);
	S3C2440Surf* pS3C2440Surf = static_cast<S3C2440Surf*>(pSurf);
	DWORD address;

	if ((pd->dwFlags & DDLOCK_WAITNOTBUSY) != 0) 
	{
	    	while (pDDGPE->SurfaceBusyFlipping(pSurf)) 
			Sleep(0);
	    	pDDGPE->WaitForNotBusy();
	} 
	else if (pDDGPE->SurfaceBusyFlipping(pSurf) || pDDGPE->IsBusy()) 
	{
	    	pd->ddRVal = DDERR_WASSTILLDRAWING;
	    	goto cleanUp;
	}

	if (pSurf->InUserMemory()) 
	{
	    	address = (DWORD)pS3C2440Surf->SurfaceAddress();
	    	pS3C2440Surf->WriteBack();
	}
	else 
	{
	    	address = (DWORD)pSurf->Buffer();
	}

	if (pd->bHasRect) 
	{
	    	address += pd->rArea.top * pSurf->Stride();
	    	address += (pd->rArea.left * pSurf->Bpp()) >> 3;
	}

	pd->lpSurfData = (VOID*)address;
	pd->ddRVal = DD_OK;

cleanUp:
	DEBUGLEAVE(HalLock);
    	return DDHAL_DRIVER_HANDLED;
}

DWORD WINAPI HalUnlock( LPDDHAL_UNLOCKDATA pd )
{
	DEBUGENTER( HalUnlock );
	RETAILMSG(0, (TEXT("HalUnlock\r\n")));
	
	DDGPESurf* pSurf = DDGPESurf::GetDDGPESurf(pd->lpDDSurface);
	S3C2440Surf* pS3C2440Surf = static_cast<S3C2440Surf*>(pSurf);

	// When surface is in cached SDRAM write back cache
	if (pSurf->InUserMemory()) 
		pS3C2440Surf->WriteBack();

	pd->ddRVal = DD_OK;

	DEBUGLEAVE(HalUnlock);
	return DDHAL_DRIVER_HANDLED;
}

//////////////////////////// DDHAL_DDSURFACECALLBACKS ////////////////////////////

DWORD WINAPI HalDestroySurface( LPDDHAL_DESTROYSURFACEDATA pd )
{
	DEBUGENTER( HalDestroySurface );
	RETAILMSG(0, (TEXT("HalDestroySurface\r\n")));
	
	// Implementation
	pd->ddRVal = DD_OK;

	return DDHAL_DRIVER_HANDLED;
}

DWORD WINAPI HalFlip( LPDDHAL_FLIPDATA pd )
{
	DEBUGENTER( HalFlip );
	RETAILMSG(0, (TEXT("HALFlip\r\n")));

	DDGPE* pDDGPE = GetDDGPE();
    	DDGPESurf* pSurf = DDGPESurf::GetDDGPESurf(pd->lpSurfTarg);    

    	if (pDDGPE->SurfaceBusyFlipping(pSurf)) 
	{
        	RETAILMSG(1, (L"Graphics engine busy\r\n"));
        	pd->ddRVal = DDERR_WASSTILLDRAWING;
        	goto cleanUp;
    	}

    	DDGPEFlip(pd);

cleanUp:    
	DEBUGLEAVE(HalFlip);
    	return DDHAL_DRIVER_HANDLED;
}

DWORD WINAPI HalGetBltStatus( LPDDHAL_GETBLTSTATUSDATA pd )
{
	DEBUGENTER( HalGetBltStatus );
	RETAILMSG(0, (TEXT("HalGetBltStatus\r\n")));
	
	DDGPE* pDDGPE = GetDDGPE();
	DDGPESurf* pSurf = DDGPESurf::GetDDGPESurf(pd->lpDDSurface);


	pd->ddRVal = DD_OK;
	if ((pd->dwFlags & DDGBS_CANBLT) != 0) 
	{
	    	if (pDDGPE->SurfaceBusyFlipping(pSurf) || pDDGPE->IsBusy()) 
		{
	       	pd->ddRVal = DDERR_WASSTILLDRAWING;
		}
	}
	else if ((pd->dwFlags & DDGBS_ISBLTDONE) != 0) 
	{
	    	if (pDDGPE->IsBusy()) 
		{
	        	pd->ddRVal = DDERR_WASSTILLDRAWING;
	    	}
	}

	DEBUGLEAVE(HalGetBltStatus);
	return DDHAL_DRIVER_HANDLED;
}

DWORD WINAPI HalGetFlipStatus( LPDDHAL_GETFLIPSTATUSDATA pd )
{
	DEBUGENTER( HalGetFlipStatus );
	RETAILMSG(0, (TEXT("HalGetFlipStatus\r\n")));
	
	DDGPE* pDDGPE = GetDDGPE();
	DDGPESurf* pSurf = DDGPESurf::GetDDGPESurf(pd->lpDDSurface);

	if (pDDGPE->SurfaceBusyFlipping(pSurf)) 
	{
	    	pd->ddRVal = DDERR_WASSTILLDRAWING;
	} 
	else 
	{
	    	pd->ddRVal = DD_OK;
	}

	DEBUGLEAVE(HalGetFlipStatus);
	return DDHAL_DRIVER_HANDLED;
}

DWORD WINAPI HalUpdateOverlay( LPDDHAL_UPDATEOVERLAYDATA pd )
{
	DEBUGENTER( HalUpdateOverlay );
	RETAILMSG(0, (TEXT("HalUpdateOverlay\r\n")));
	
	// Implementation
	pd->ddRVal = DD_OK;

	return DDHAL_DRIVER_HANDLED;
}

DWORD WINAPI HalSetOverlayPosition( LPDDHAL_SETOVERLAYPOSITIONDATA pd )
{
	DEBUGENTER( HalSetOverlayPosition );
	RETAILMSG(0, (TEXT("HalSetOverlayPosition\r\n")));
	
	// Implementation
	pd->ddRVal = DD_OK;

	return DDHAL_DRIVER_HANDLED;
}

DWORD WINAPI HalSetPalette( LPDDHAL_SETPALETTEDATA pd )
{
	DEBUGENTER( HalSetPalette );
	RETAILMSG(0, (TEXT("HalSetPalette\r\n")));
	
	// Implementation
	pd->ddRVal = DD_OK;

	return DDHAL_DRIVER_HANDLED;
}

