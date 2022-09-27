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
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:	surface allocation/manipulation/free routines

Abstract:

Functions:


Notes:


--*/

#include "precomp.h"


#define DBGLCD	0
#define DBGLCD1	0

static UINT nCount = 0;


//  This method is used to allocate DirectDraw enabled surfaces
SCODE	S3C2440DISP::AllocSurface(DDGPESurf **ppSurf, int width, int height, 
					EGPEFormat format, EDDGPEPixelFormat pixelFormat, int surfaceFlags)
{
	SCODE sc = S_OK;
	DWORD pa;
#if 1
	DWORD 	bpp  	= EGPEFormatToBpp[format];
	LONG 	align 	= m_nSurfaceBitsAlign / 8;
    	DWORD 	alignedWidth = ((width + align - 1) & (- align));
    	DWORD 	nSurfaceSize = (bpp * (alignedWidth * height)) / 8;
#endif
	ULONG stride = (width * EGPEFormatToBpp[format] + 7) >> 3;
	ULONG size = MulDiv(stride, height, 1);

	if(width == 0)
		RETAILMSG(1, (TEXT("WidthSave = %d, HeightSave = %d"), m_nScreenWidthSave, m_nScreenHeightSave));

	RETAILMSG(DBGLCD, (TEXT("AllocSurface DirectDraw, surfaceFlags = %d, f = %d, m_pMode->format = %d , pF = %d\r\n"), surfaceFlags, format, m_pMode->format , pixelFormat));
	RETAILMSG(DBGLCD, (TEXT("align = %d, alignedwidth = %d, surfacesize = %d, height = %d, width = %d stride = %d\r\n"), align, alignedWidth, nSurfaceSize, height, width, stride));


	// Is video memory required/prefered?
	//apart from the primary and directdraw surfaces, all others are 
	//allocated in system memory
	if ((surfaceFlags & GPE_BACK_BUFFER) ||
        	(surfaceFlags & GPE_REQUIRE_VIDEO_MEMORY) || 
        	((surfaceFlags & GPE_PREFER_VIDEO_MEMORY) && (format == m_pMode->format)))
	{		
		if (size == -1) 
		{
	    		return E_OUTOFMEMORY;
		}
		if(surfaceFlags & GPE_BACK_BUFFER)
			RETAILMSG(DBGLCD, (TEXT("BB -------------- In BackBuffer alloc -----\r\n")));

		// Attempt to allocate from internal SRAM
		SurfaceHeap *pHeap = m_pVideoMemoryHeap->Alloc(size);
		RETAILMSG(DBGLCD, (TEXT("pHeap->Address() = 0x%x, pHeap = 0x%x, remains = 0x%x, size = 0x%x\r\n"), pHeap->Address(), pHeap, m_pVideoMemoryHeap->Available(), m_pVideoMemoryHeap->Size()));
		if (pHeap != NULL) 
		{
			// We get memory in SRAM, build surface there			
			pa  = pHeap->Address() - (DWORD)m_VirtualFrameBuffer + IMAGE_FRAMEBUFFER_DMA_BASE;
			
			RETAILMSG(DBGLCD, (TEXT("---------------------> pa = 0x%x\r\n"), pa));
			if ((*ppSurf = new S3C2440Surf(width, height, pa, (VOID*)pHeap->Address(),
				stride, format, pixelFormat, pHeap)) == NULL)
			{
			    	// When allocation failed, we are out of memory...
			    	RETAILMSG(DBGLCD, (TEXT("--------------------> Failed allocating memory for S3C2440Surf --------------------------\r\n")));
			    	pHeap->Free();
			    	return E_OUTOFMEMORY;
			}

			// We are done
			RETAILMSG(DBGLCD, (TEXT("--------------------> Succeeded allocating memory for S3C2440Surf --------------------------\r\n")));
			return S_OK;
		}
		if (surfaceFlags & GPE_REQUIRE_VIDEO_MEMORY)
		{
		    	*ppSurf = (DDGPESurf *)NULL;
		    	return E_OUTOFMEMORY;
		}
	}

	if (surfaceFlags & GPE_REQUIRE_VIDEO_MEMORY)
	{
	    	*ppSurf = (DDGPESurf *)NULL;

	    	return  E_OUTOFMEMORY;
	}

	RETAILMSG(DBGLCD, (TEXT("* Allocating surface in system memory DD*\r\n")));
    	*ppSurf = new DDGPESurf(width, height, stride, format, pixelFormat);
#if 0	//{		
	if (*ppSurf == NULL || (*ppSurf)->Buffer() == NULL) 
	{
	    	RETAILMSG(DBGLCD, (L"ERROR: S3C2440Surf::AllocSurface: Failed allocate surface (width: %d, height: %d, format %d)\r\n", width, height, format));
	    	delete *ppSurf;
	    	*ppSurf = NULL;
	    	sc = E_OUTOFMEMORY;
	    	goto cleanUp;
	}
#else
	if (*ppSurf != NULL)
    	{
		// check we allocated bits succesfully
		if ( (*ppSurf)->Buffer() == NULL)
		{
		    	delete *ppSurf;
		}
		else
		{
			// We are done
			RETAILMSG(DBGLCD, (L"S3C2440Surf::AllocSurface: Surface in memory (width: %d, height: %d, format %d, 0x%08x)\r\n",
	    			width, height, format, (*ppSurf)->Buffer()));
		    	return  S_OK;
		}
	}
#endif	//}

		return E_OUTOFMEMORY;

}

/*
SCODE S3C2440DISP::AllocSurface(DDGPESurf **ppSurf, int width, int height, EGPEFormat format, EDDGPEPixelFormat pixelFormat, int surfaceFlags)
{
	RETAILMSG(DBGLCD, (TEXT("AllocSurface DirectDraw, surfaceFlags = %d, f = %d, m_pMode->format = %d , pF = %d\r\n"), surfaceFlags, format, m_pMode->format , pixelFormat));
	RETAILMSG(DBGLCD, (TEXT("height = %d, width = %d\r\n"), height, width));

    if ((surfaceFlags & GPE_REQUIRE_VIDEO_MEMORY) || (format == m_pMode->format) && (surfaceFlags & GPE_PREFER_VIDEO_MEMORY))
    {
        // Attempt to allocate from video memory

        WORD bytepp = (EGPEFormatToBpp[format]/8);
        ulong stride = ((width+7)&~0x7)*bytepp;        
        ulong heapsize = MulDiv (stride, height, 1);
        if (heapsize == -1)
        {
            return E_OUTOFMEMORY;       // overflow
        }

        SurfaceHeap *pHeap = m_pVideoMemoryHeap->Alloc(heapsize);
        if (pHeap)
        {
            *ppSurf = new S3C2440Surf ( width, height,
                                    (DWORD)pHeap->Address() - (DWORD)m_VirtualFrameBuffer,
                                    (void*)pHeap->Address(),
                                    stride,
                                    format,
                                    pixelFormat,
                                    pHeap);

            if (!(*ppSurf))
            {
                pHeap->Free();
                return E_OUTOFMEMORY;
            }
            return S_OK;
        }

        if (surfaceFlags & GPE_REQUIRE_VIDEO_MEMORY)
        {
            *ppSurf = (DDGPESurf *)NULL;
            return E_OUTOFMEMORY;
        }
    }

    if (surfaceFlags & GPE_REQUIRE_VIDEO_MEMORY)
    {
        *ppSurf = (DDGPESurf *)NULL;

        return  E_OUTOFMEMORY;
    }

    // Allocate from system memory
    RETAILMSG(1, (TEXT("Creating a GPESurf in system memory DDDD. EGPEFormat = %d\r\n"), (int) format));
    {
        DWORD bpp  = EGPEFormatToBpp[format];
        DWORD stride = ((bpp * width + 31) >> 5) << 2;
        DWORD nSurfaceBytes = stride * height;

        *ppSurf = new DDGPESurf(width, height, stride, format, pixelFormat);
    }
    if (*ppSurf != NULL)
    {
        // check we allocated bits succesfully
        if ( (*ppSurf)->Buffer() == NULL)
        {
            delete *ppSurf;
        }
        else
        {
            return  S_OK;
        }
    }

    return E_OUTOFMEMORY;
}
*/

//  This method is called for all normal surface allocations from ddgpe and gpe
SCODE S3C2440DISP::AllocSurface(GPESurf **ppSurf, INT width, INT height, EGPEFormat format, INT surfaceFlags)
{
	SCODE sc = S_OK;
	
	ULONG stride = (width * EGPEFormatToBpp[format] + 7) >> 3;
	
	RETAILMSG(DBGLCD, (TEXT("* Allocating surface in system memory normal*\r\n")));

    	*ppSurf = new GPESurf(width, height, format);

	if (*ppSurf == NULL || (*ppSurf)->Buffer() == NULL) 
	{
	    	RETAILMSG(DBGLCD, (L"ERROR: S3C2440Surf::AllocSurface: Failed allocate surface (width: %d, height: %d, format %d)\r\n",width, height, format));
	    	delete *ppSurf;
	    	*ppSurf = NULL;
	    	sc = E_OUTOFMEMORY;
	    	goto cleanUp;
	}

	// We are done
	RETAILMSG(DBGLCD, (L"S3C2440Surf::AllocSurface: Surface in memory (width: %d, height: %d, format %d, pixelformat %d 0x%08x)\r\n",
	    width, height, format, EGPEFormatToEDDGPEPixelFormat[format], (*ppSurf)->Buffer()));

cleanUp:
	return sc;
}

void S3C2440DISP::SetVisibleSurface( GPESurf *pTempSurf, BOOL bWaitForVBlank)
{
    	S3C2440Surf *pSurf = (S3C2440Surf *) pTempSurf;
	RETAILMSG(DBGLCD, (TEXT("S3C2440DISP::SetVisibleSurface\r\n")));
}

S3C2440Surf::S3C2440Surf(int width, int height, DWORD pa, VOID *pBits, int stride, 
			EGPEFormat format, EDDGPEPixelFormat pixelFormat, SurfaceHeap *pHeap) 
			: DDGPESurf(width, height, pBits, stride, format, pixelFormat)
{
	BOOL rc = FALSE;
	RETAILMSG(DBGLCD, (TEXT("nCount = %d\r\n"), ++nCount));
	m_pHeap = pHeap;
	m_nOffsetInVideoMemory = pa;
	m_fInVideoMemory = (pa != 0);
	m_fInUserMemory = (pHeap == NULL);
	m_hSurface = NULL;
	m_pSurface = NULL;
}

//-----------------------------------------------------------------------------

S3C2440Surf::~S3C2440Surf()
{	
    // Delete surface mapping if exists
    	if (m_pSurface != NULL) 
	{
        	ADDRESS address = (ADDRESS)m_pSurface;
        	address &= ~(PAGE_SIZE - 1);
        	UnmapViewOfFile((VOID*)address);
    	}

    	if (m_hSurface != NULL) 
		CloseHandle(m_hSurface);

    	// Free memory if it was allocated in SRAM...
    	if (m_pHeap != NULL) 
		m_pHeap->Free();
	RETAILMSG(DBGLCD, (TEXT("-------------------S3C2440Surf::~S3C2440Surf  nCount = %d\r\n"), --nCount));
}

//------------------------------------------------------------------------------
// 
//  Method:  WriteBack
//
//  Flush surface memory in cache.
//
VOID S3C2440Surf::WriteBack()
{
    	ASSERT(m_pSurface != NULL);

	RETAILMSG(DBGLCD, (TEXT("S3C2440Surf::WriteBack\n")));

    	if (m_pSurface != NULL) 
		CacheRangeFlush((VOID*)m_pSurface, m_nStrideBytes * m_nHeight, CACHE_SYNC_WRITEBACK);
}

//------------------------------------------------------------------------------
// 
//  Method:  Discard
//
//  Flush and invalidate surface memory in cache.
//
VOID S3C2440Surf::Discard()
{
    	ASSERT(m_pSurface != NULL);
		RETAILMSG(DBGLCD, (TEXT("S3C2440Surf::Discard\n")));
    	if (m_pSurface != NULL) 
		CacheRangeFlush((VOID*)m_pSurface, m_nStrideBytes * m_nHeight, CACHE_SYNC_DISCARD);
}

