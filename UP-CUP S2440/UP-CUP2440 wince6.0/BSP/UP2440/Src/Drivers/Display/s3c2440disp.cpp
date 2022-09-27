/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.
Copyright (c) 2006. Samsung Electronics, co. ltd  All rights reserved.

Module Name:  

Abstract:

	This file implements the S3C2440 LCD driver as directdraw

Notes: 
--*/

#include "precomp.h"
#include <aablt.h>
#include <gxinfo.h>
#include <s3c2440a_ioport.h>
#include <bsp.h>
#define DBGLCD	0
#define DBGLCD1	0

#define SSBPP_DEFAULT			16
#define SSFREQ_DEFAULT			60

DWORD gdwLCDVirtualFrameBase;

INSTANTIATE_GPE_ZONES(0x3,"MGDI Driver","unused1","unused2")	// Start with errors and warnings

DDGPE	*gGPE = (DDGPE *)NULL;

PENGCALLBACKS  ourengineCallbacks;

static TCHAR gszBaseInstance[256] = _T("Drivers\\Display\\S3C2440\\CONFIG");

#define dim(x) (sizeof(x) / sizeof(x[0]))


static volatile S3C2440A_CLKPWR_REG *v_s2440CLKPWR;		// CLCKPWR (needed to enable display clocks)
volatile S3C2440A_IOPORT_REG *vm_pIOPreg;			// pointer to the GPIO control registers


EGPEFormat eFormat[] =
{
	gpe8Bpp,
	gpe16Bpp,
	gpe24Bpp,
	gpe32Bpp,
};

EDDGPEPixelFormat ePixelFormat[4] = 
{
	ddgpePixelFormat_8bpp,
	ddgpePixelFormat_565,
	ddgpePixelFormat_8880,
	ddgpePixelFormat_8888,
};

ULONG BitMasks[][3] =
{
	{ 0, 0, 0 },
	{ 0xF800, 0x07E0, 0x001F },
	{ 0xFF0000, 0x00FF00, 0x0000FF },
	{ 0x00FF0000, 0x0000FF00, 0x000000FF }
};

// This prototype avoids problems exporting from .lib
BOOL APIENTRY GPEEnableDriver(ULONG engineVersion, ULONG cj, DRVENABLEDATA *data,
							  PENGCALLBACKS  engineCallbacks);
BOOL APIENTRY DrvEnableDriver(ULONG engineVersion, ULONG cj, DRVENABLEDATA *data,
							  PENGCALLBACKS  engineCallbacks)
{
	BOOL fOk = FALSE;
	if(gszBaseInstance[0] != 0) {
		RETAILMSG(DBGLCD,(_T("gszBaseInstance is valid\r\n")));
		fOk = GPEEnableDriver(engineVersion, cj, data, engineCallbacks);
		ourengineCallbacks = engineCallbacks;
	}
	RETAILMSG(DBGLCD,(_T("-DrvEnableDriver\r\n")));
	return fOk;
}

BOOL ConvertStringToGuid (LPCTSTR pszGuid, GUID *pGuid)
{
	UINT Data4[8];
	int  Count;
	BOOL fOk = FALSE;
	TCHAR *pszGuidFormat = _T("{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}");

	RETAILMSG(DBGLCD1,(_T("++ConvertStringToGuid\r\n")));
	
	DEBUGCHK(pGuid != NULL && pszGuid != NULL);
	__try
	{
		if (_stscanf(pszGuid, pszGuidFormat, &pGuid->Data1, 
			&pGuid->Data2, &pGuid->Data3, &Data4[0], &Data4[1], &Data4[2], &Data4[3], 
			&Data4[4], &Data4[5], &Data4[6], &Data4[7]) == 11)
		{
			for(Count = 0; Count < (sizeof(Data4) / sizeof(Data4[0])); Count++)
			{
				pGuid->Data4[Count] = (UCHAR) Data4[Count];
			}
		}
		fOk = TRUE;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
	}
	RETAILMSG(DBGLCD1,(_T("--ConvertStringToGuid\r\n")));
	return fOk;
}

BOOL AdvertisePowerInterface(HMODULE hInst)
{
	BOOL fOk = FALSE;
	HKEY hk;
	DWORD dwStatus;
	TCHAR szTemp[MAX_PATH];
	GUID gClass;

	RETAILMSG(DBGLCD, (TEXT("++AdvertisePowerInterface\n")));

	// assume we are advertising the default class
	fOk = ConvertStringToGuid(PMCLASS_DISPLAY, &gClass);
	DEBUGCHK(fOk);

	// check for an override in the registry
	dwStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("System\\GDI\\Drivers"), 0, 0, &hk);
	if(dwStatus == ERROR_SUCCESS)
	{
		DWORD dwType, dwSize;
		dwSize = sizeof(szTemp);
		dwStatus = RegQueryValueEx(hk, _T("DisplayPowerClass"), NULL, &dwType, (LPBYTE) szTemp, &dwSize);
		if(dwStatus == ERROR_SUCCESS && dwType == REG_SZ)
		{
			// got a guid string, convert it to a guid
			GUID gTemp;
			fOk = ConvertStringToGuid(szTemp, &gTemp);
			DEBUGCHK(fOk);
			if(fOk)
			{
				gClass = gTemp;
			}
		}

		// release the registry key
		RegCloseKey(hk);
	}

	// figure out what device name to advertise
	if(fOk)
	{
		fOk = GetModuleFileName(hInst, szTemp, sizeof(szTemp)/sizeof(szTemp[0]));
		DEBUGCHK(fOk);
	}

	// now advertise the interface
	if(fOk)
	{
		fOk = AdvertiseInterface(&gClass, szTemp, TRUE);		
		DEBUGCHK(fOk);
	}
		RETAILMSG(DBGLCD, (TEXT("--AdvertisePowerInterface\n")));
	return fOk;
}

// Main entry point for a GPE-compliant driver
GPE *GetGPE(void)
{
	RETAILMSG(DBGLCD,(_T("+GetGPE() called\r\n")));
	if (!gGPE)
	{
		gGPE = new S3C2440DISP();
	}

	RETAILMSG(DBGLCD,(_T("-GetGPE()\r\n")));
	return gGPE;
}

// GWES will invoke this routine once prior to making any other calls into the driver.
// This routine needs to save its instance path information and return TRUE.  If it
// returns FALSE, GWES will abort the display initialization.
BOOL APIENTRY
DisplayInit(LPCTSTR pszInstance, DWORD dwNumMonitors)
{
	DWORD dwStatus;
	HKEY hkDisplay;
	BOOL fOk = FALSE;
	
	RETAILMSG(1, (_T("LCD: display instance '%s', num monitors %d\r\n"), \
		pszInstance != NULL ? pszInstance : _T("<NULL>"), dwNumMonitors));


	if(pszInstance != NULL) 
	{
		_tcsncpy(gszBaseInstance, pszInstance, dim(gszBaseInstance));
	}

	v_s2440CLKPWR = (volatile S3C2440A_CLKPWR_REG *)VirtualAlloc(0, sizeof(S3C2440A_CLKPWR_REG), MEM_RESERVE, PAGE_NOACCESS);
	if (!v_s2440CLKPWR)
	{
		RETAILMSG(1, (TEXT("#####Display::InitializeHardware VirtualAlloc failed!\r\n")));
		return FALSE; 
	}
	if (!VirtualCopy((PVOID)v_s2440CLKPWR, (PVOID)(S3C2440A_BASE_REG_PA_CLOCK_POWER>>8), sizeof(S3C2440A_CLKPWR_REG), PAGE_READWRITE | PAGE_NOCACHE | PAGE_PHYSICAL ))
	{
		RETAILMSG(1, (TEXT("#####Display::InitializeHardware VirtualCopy failed!\r\n")));
		return FALSE;
	}	
	vm_pIOPreg = (volatile S3C2440A_IOPORT_REG *)VirtualAlloc(0, sizeof(S3C2440A_IOPORT_REG), MEM_RESERVE, PAGE_NOACCESS);

	if (!vm_pIOPreg)
	{
		RETAILMSG(1, (TEXT("#####Display_vga_IOPregs: VirtualCopy failed!\r\n")));
		return FALSE; 
	}
	if (!VirtualCopy((PVOID)vm_pIOPreg, (PVOID)(S3C2440A_BASE_REG_PA_IOPORT >> 8), sizeof(S3C2440A_IOPORT_REG), PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE)) 
		{
			ERRORMSG(1,(TEXT("#####Display_vga_IOPregs: VirtualCopy failed!\r\n")));
			return FALSE; 
		}
	// setup up display mode related constants
	// sanity check the path by making sure it exists
	dwStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, gszBaseInstance, 0, 0, &hkDisplay);
	if(dwStatus == ERROR_SUCCESS) 
	{
		RegCloseKey(hkDisplay);
		fOk = TRUE;
	} 
	else 
	{
		// 
		RETAILMSG(1, (_T("S3C2440DISP: DisplayInit: can't open '%s'\r\n"), gszBaseInstance));
	}

	RETAILMSG(DBGLCD,(_T("-DisplayInit\r\n")));

	return fOk;
}

S3C2440DISP::S3C2440DISP (void)
{
	ULONG		fbSize;
	ULONG		fbOffset;
	ULONG		offsetX, offsetY;
	HKEY 		hkDisplay = NULL;
	DWORD 		dwLCDPhysicalFrameBase;
	DWORD 		dwStatus, dwType, dwSize;
  	

	m_nScreenWidth = LCD_XSIZE_TFT;
	m_nScreenHeight =LCD_YSIZE_TFT;
	m_colorDepth = SSBPP_DEFAULT;

	m_cxPhysicalScreen =  LCD_XSIZE_TFT;
	m_cyPhysicalScreen =  LCD_YSIZE_TFT;


	m_cbScanLineLength = m_nScreenWidth * (m_colorDepth >> 3);
	m_FrameBufferSize = m_nScreenHeight * m_cbScanLineLength;
	m_VideoPowerState = VideoPowerOn;

	m_pPrimarySurface = NULL;
	m_pVideoMemoryHeap = NULL;
	

	//APR check
	m_pModeEx = &m_ModeInfoEx;
	m_pMode = &m_ModeInfoEx.modeInfo;
	memset(m_pModeEx, 0, sizeof(GPEModeEx));
	m_pModeEx->dwSize = sizeof(GPEModeEx);
	m_pModeEx->dwVersion = GPEMODEEX_CURRENTVERSION;

	// Setup main ModeInfo
	m_pMode->modeId = 0; 
	m_pMode->Bpp = SSBPP_DEFAULT;
	m_pMode->frequency = SSFREQ_DEFAULT;
	m_pMode->width = m_nScreenWidth;
	m_pMode->height = m_nScreenHeight;

	// Setup Rotation Info
	m_nScreenHeightSave = m_pMode->height;
	m_nScreenWidthSave = m_pMode->width;
	m_iRotate = GetRotateModeFromReg();
	SetRotateParams();
	
	// compute frame buffer displayable area offset
	offsetX = (m_cxPhysicalScreen - m_nScreenWidth) / 2;
	offsetY = (m_cyPhysicalScreen - m_nScreenHeight) / 2;
	fbOffset = (offsetY * m_cbScanLineLength) + offsetX;

	int nBPP = m_pMode->Bpp/8 - 1;
	switch (m_pMode->Bpp)
	{
//		case    8:
		case    16:
		case    24:
		case    32:
			m_pMode->format = eFormat[nBPP];
			m_pModeEx->ePixelFormat = ePixelFormat[nBPP];
			m_pModeEx->lPitch = m_pMode->width * m_pMode->Bpp / 8;
			m_pModeEx->dwRBitMask = BitMasks[nBPP][0];
			m_pModeEx->dwGBitMask = BitMasks[nBPP][1];
			m_pModeEx->dwBBitMask = BitMasks[nBPP][2];
			break;

		default:
			RETAILMSG(1,(TEXT("Invalid BPP value passed to driver - Bpp = %d\r\n"), m_pMode->Bpp));
			m_pMode->format = gpeUndefined;
			break;
	}
	
    	// compute physical frame buffer size
    	// for DDraw enabled, make sure we also have some off-screen video memory available for surface allocations
    	//fbSize = (((m_cyPhysicalScreen * m_cbScanLineLength) >> 20) + 1) << 20;        // set size to next highest 1MB boundary
	//APR: for now we are taking it as 1 MB 
	fbSize = 0x1A0000;//(1024 * 1024);
	
	// open the registry key and read our configuration
	dwStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, gszBaseInstance, 0, 0, &hkDisplay);
	dwType = REG_DWORD;

	if(dwStatus == ERROR_SUCCESS && dwType == REG_DWORD) 
	{
		dwSize = sizeof(DWORD);
		dwStatus = RegQueryValueEx(hkDisplay, _T("LCDVirtualFrameBase"), NULL, &dwType, 
			(LPBYTE) &gdwLCDVirtualFrameBase, &dwSize);
	}
	
	if(dwStatus == ERROR_SUCCESS && dwType == REG_DWORD) 
	{
		dwSize = sizeof(DWORD);
		dwStatus = RegQueryValueEx(hkDisplay, _T("LCDPhysicalFrameBase"), NULL, &dwType, 
			(LPBYTE) &dwLCDPhysicalFrameBase, &dwSize);
	}

	// close the registry key
	if(hkDisplay != NULL) 
	{
		RegCloseKey(hkDisplay);
	}

	RETAILMSG(DBGLCD, (TEXT("LCDPhysicalFrameBase (PHY : 0x%x) and of size 0x%x \n\r"), dwLCDPhysicalFrameBase, fbSize));


	//did we get everything?
	if(dwStatus != ERROR_SUCCESS) 
	{
		RETAILMSG(1, (_T("S3C2440LCD: InitializeHardware: couldn't read registry configuration\r\n")));
		return;
	}

    	// Direct Access for frame buffer, Because GWES is running in kernel region

    	m_VirtualFrameBuffer = gdwLCDVirtualFrameBase; //(DWORD)VirtualAlloc(0, fbSize, MEM_RESERVE, PAGE_NOACCESS);
/*
	if (m_VirtualFrameBuffer == NULL)
	{
		RETAILMSG(1,(TEXT("m_VirtualFrameBuffer is not allocated\n\r")));
		return;
	}
	else if (!VirtualCopy((void *)m_VirtualFrameBuffer, (PVOID)gdwLCDVirtualFrameBase, fbSize, PAGE_READWRITE | PAGE_NOCACHE)) 
	{
		RETAILMSG(1, (TEXT("m_VirtualFrameBuffer VirtualCopy is Failed\r\n")));
		VirtualFree((PVOID)m_VirtualFrameBuffer, 0, MEM_RELEASE);
		m_VirtualFrameBuffer = NULL;
		return;
	}
*/
	//CeSetMemoryAttributes ((void *)m_VirtualFrameBuffer, (void *)(dwLCDPhysicalFrameBase >> 8), fbSize, PAGE_WRITECOMBINE);
	
	RETAILMSG(DBGLCD, (TEXT("m_VirtualFrameBuffer is mapped at %x(PHY : %x) and of size %x \n\r"), m_VirtualFrameBuffer, dwLCDPhysicalFrameBase, fbSize));

	memset ((void*)m_VirtualFrameBuffer, 0x0, fbSize);

	//m_VirtualFrameBuffer += fbOffset;
	//fbSize -= fbOffset;

       m_pVideoMemoryHeap = new SurfaceHeap(fbSize, m_VirtualFrameBuffer, NULL, NULL);
	if(!m_pVideoMemoryHeap)
	{
		RETAILMSG (1, (L"Failed to create surface heap on internal SRAM memory\n"));
		return;
	}
#if 0	//{ taken to setmode
	//Allocate our primary surface here
	if(FAILED(AllocSurface((DDGPESurf **)&m_pPrimarySurface, m_nScreenWidth, 
		m_nScreenHeight, m_pMode->format, m_pModeEx->ePixelFormat, 
		GPE_REQUIRE_VIDEO_MEMORY)))
	{
		RETAILMSG (1, (L"Couldn't allocate primary surface\n"));
		return;
	}
#endif	//}
	//memset((PUCHAR) m_pPrimarySurface->Buffer(), 0,
       //     (m_nScreenWidth * m_pMode->width * m_pMode->Bpp/8) * m_nScreenHeight);
	RETAILMSG(DBGLCD, (TEXT("10\r\n")));
	m_CursorVisible = FALSE;
	m_CursorDisabled = TRUE;
	m_CursorForcedOff = FALSE;
	RETAILMSG(DBGLCD, (TEXT("11\r\n")));
	memset (&m_CursorRect, 0x0, sizeof(m_CursorRect));
	RETAILMSG(DBGLCD, (TEXT("12\r\n")));
	m_CursorBackingStore = NULL;
	m_CursorXorShape = NULL;
	m_CursorAndShape = NULL;
	RETAILMSG(DBGLCD, (TEXT("13\r\n")));
	AdvertisePowerInterface(g_hmodDisplayDll);
	RETAILMSG(DBGLCD, (TEXT("--S3C2440DISP::S3C2440DISP\r\n")));
}

S3C2440DISP::~S3C2440DISP()
{
	RETAILMSG(DBGLCD,(_T("++S3C2440DISP::~S3C2440DISP\r\n")));

	if (v_s2440CLKPWR)
		VirtualFree((PVOID)v_s2440CLKPWR, 0, MEM_RELEASE);    
	RETAILMSG(DBGLCD,(_T("--S3C2440DISP::~S3C2440DISP\r\n")));
}


SCODE S3C2440DISP::SetMode (INT modeId, HPALETTE *palette)
{
	RETAILMSG(DBGLCD, (TEXT("++S3C2440DISP::SetMode\r\n")));

	if (modeId != 0)
	{
		RETAILMSG(1, (TEXT("S3C2440DISP::SetMode Want mode %d, only have mode 0\r\n"),modeId));
		return	E_INVALIDARG;
	}

	if (palette)
	{
		RETAILMSG(DBGLCD, (TEXT("Calling EngCreatePalette\r\n")));
//		*palette = ourengineCallbacks->EngCreatePalette(PAL_BITFIELDS, 0, NULL, gBitMasks[0], gBitMasks[1], gBitMasks[2]);			

		int nBPP = m_pMode->Bpp/8 - 1;
		
		switch (m_pMode->Bpp)
		{
		//case 	8:
		case    16:
		case    24:
		case    32:
		        *palette = ourengineCallbacks->EngCreatePalette (PAL_BITFIELDS,
		                                     0,
		                                     NULL,
		                                     BitMasks[nBPP][0],
		                                     BitMasks[nBPP][1],
		                                     BitMasks[nBPP][2]);
		        break;
		default : 
		      RETAILMSG(1, (TEXT("ERR !!! S3C2440DISP::SetMode , m_pMode->Bpp = %d \r\n"),m_pMode->Bpp));
			
		}

		// This sets the surface address alignment
    		m_nSurfaceBitsAlign = (m_pMode->Bpp == 24) ? (128 * 3) : 128;

		//Allocate our primary surface here
		if(NULL == m_pPrimarySurface)
		{
			if(FAILED(AllocSurface((DDGPESurf **)&m_pPrimarySurface, m_nScreenWidth, 
				m_nScreenHeight, m_pMode->format, m_pModeEx->ePixelFormat, 
				GPE_REQUIRE_VIDEO_MEMORY)))
			{
				RETAILMSG (1, (L"Couldn't allocate primary surface\n"));
				return E_INVALIDARG;
			}
		}

		m_pPrimarySurface->SetRotation(m_nScreenWidth, m_nScreenHeight, m_iRotate);
	}

	DynRotate(m_iRotate);
	RETAILMSG(DBGLCD, (TEXT("--S3C2440DISP::SetMode\r\n")));

	return S_OK;
}

SCODE S3C2440DISP::GetModeInfo(GPEMode *mode, INT modeNumber)
{
	RETAILMSG(DBGLCD, (TEXT("++S3C2440DISP::GetModeInfo\r\n")));

	if (modeNumber != 0)
	{
		return E_INVALIDARG;
	}	

	*mode = *m_pMode;	

	RETAILMSG(DBGLCD, (TEXT("--S3C2440DISP::GetModeInfo\r\n")));

	return S_OK;
}

int	S3C2440DISP::NumModes()
{
	RETAILMSG(DBGLCD, (TEXT("++S3C2440DISP::NumModes\r\n")));
	RETAILMSG(DBGLCD, (TEXT("--S3C2440DISP::NumModes\r\n")));
	return	1;
}

void    S3C2440DISP::CursorOn (void)
{
	UCHAR    *ptrScreen = (UCHAR*)m_pPrimarySurface->Buffer();
	UCHAR    *ptrLine;
	UCHAR    *cbsLine;
	int        x, y;

	if (!m_CursorForcedOff && !m_CursorDisabled && !m_CursorVisible)
	{
		RECTL cursorRectSave = m_CursorRect;
		int   iRotate;
		RotateRectl(&m_CursorRect);
		for (y = m_CursorRect.top; y < m_CursorRect.bottom; y++)
		{
			if (y < 0)
			{
				continue;
			}
			if (y >= m_nScreenHeightSave)
			{
				break;
			}

			ptrLine = &ptrScreen[y * m_pPrimarySurface->Stride()];
			cbsLine = &m_CursorBackingStore[(y - m_CursorRect.top) * (m_CursorSize.x * (m_colorDepth >> 3))];

			for (x = m_CursorRect.left; x < m_CursorRect.right; x++)
			{
				if (x < 0)
				{
					continue;
				}
				if (x >= m_nScreenWidthSave)
				{
					break;
				}

				// x' = x - m_CursorRect.left; y' = y - m_CursorRect.top;
				// Width = m_CursorSize.x;   Height = m_CursorSize.y;
				switch (m_iRotate)
				{
					case DMDO_0:
						iRotate = (y - m_CursorRect.top)*m_CursorSize.x + x - m_CursorRect.left;
						break;
					case DMDO_90:
						iRotate = (x - m_CursorRect.left)*m_CursorSize.x + m_CursorSize.y - 1 - (y - m_CursorRect.top);   
						break;
					case DMDO_180:
						iRotate = (m_CursorSize.y - 1 - (y - m_CursorRect.top))*m_CursorSize.x + m_CursorSize.x - 1 - (x - m_CursorRect.left);
						break;
					case DMDO_270:
						iRotate = (m_CursorSize.x -1 - (x - m_CursorRect.left))*m_CursorSize.x + y - m_CursorRect.top;
						break;
					default:
						iRotate = (y - m_CursorRect.top)*m_CursorSize.x + x - m_CursorRect.left;
						break;
				}
				cbsLine[(x - m_CursorRect.left) * (m_colorDepth >> 3)] = ptrLine[x * (m_colorDepth >> 3)];
				ptrLine[x * (m_colorDepth >> 3)] &= m_CursorAndShape[iRotate];
				ptrLine[x * (m_colorDepth >> 3)] ^= m_CursorXorShape[iRotate];
				if (m_colorDepth > 8)
				{
					cbsLine[(x - m_CursorRect.left) * (m_colorDepth >> 3) + 1] = ptrLine[x * (m_colorDepth >> 3) + 1];
					ptrLine[x * (m_colorDepth >> 3) + 1] &= m_CursorAndShape[iRotate];
					ptrLine[x * (m_colorDepth >> 3) + 1] ^= m_CursorXorShape[iRotate];
					if (m_colorDepth > 16)
					{
						cbsLine[(x - m_CursorRect.left) * (m_colorDepth >> 3) + 2] = ptrLine[x * (m_colorDepth >> 3) + 2];
						ptrLine[x * (m_colorDepth >> 3) + 2] &= m_CursorAndShape[iRotate];
						ptrLine[x * (m_colorDepth >> 3) + 2] ^= m_CursorXorShape[iRotate];
					}
				}
			}
		}
		m_CursorRect = cursorRectSave;
		m_CursorVisible = TRUE;
	}
}

void    S3C2440DISP::CursorOff (void)
{
	UCHAR	*ptrScreen = (UCHAR*)m_pPrimarySurface->Buffer();
	UCHAR	*ptrLine;
	UCHAR	*cbsLine;
	int		x, y;

	if (!m_CursorForcedOff && !m_CursorDisabled && m_CursorVisible)
	{
		RECTL rSave = m_CursorRect;
		RotateRectl(&m_CursorRect);
		for (y = m_CursorRect.top; y < m_CursorRect.bottom; y++)
		{
			// clip to displayable screen area (top/bottom)
			if (y < 0)
			{
				continue;
			}
			if (y >= m_nScreenHeightSave)
			{
				break;
			}

			ptrLine = &ptrScreen[y * m_pPrimarySurface->Stride()];
			cbsLine = &m_CursorBackingStore[(y - m_CursorRect.top) * (m_CursorSize.x * (m_colorDepth >> 3))];

			for (x = m_CursorRect.left; x < m_CursorRect.right; x++)
			{
				// clip to displayable screen area (left/right)
				if (x < 0)
				{
					continue;
				}
				if (x >= m_nScreenWidthSave)
				{
					break;
				}

				ptrLine[x * (m_colorDepth >> 3)] = cbsLine[(x - m_CursorRect.left) * (m_colorDepth >> 3)];
				if (m_colorDepth > 8)
				{
					ptrLine[x * (m_colorDepth >> 3) + 1] = cbsLine[(x - m_CursorRect.left) * (m_colorDepth >> 3) + 1];
					if (m_colorDepth > 16)
					{
						ptrLine[x * (m_colorDepth >> 3) + 2] = cbsLine[(x - m_CursorRect.left) * (m_colorDepth >> 3) + 2];
					}
				}
			}
		}
		m_CursorRect = rSave;
		m_CursorVisible = FALSE;
	}
}

SCODE	S3C2440DISP::SetPointerShape(GPESurf *pMask, GPESurf *pColorSurf, INT xHot, INT yHot, INT cX, INT cY)
{
	UCHAR	*andPtr;		// input pointer
	UCHAR	*xorPtr;		// input pointer
	UCHAR	*andLine;		// output pointer
	UCHAR	*xorLine;		// output pointer
	char	bAnd;
	char	bXor;
	int		row;
	int		col;
	int		i;
	int		bitMask;

	RETAILMSG(DBGLCD1, (TEXT("++S3C2440DISP::SetPointerShape(0x%X, 0x%X, %d, %d, %d, %d)\r\n"),pMask, pColorSurf, xHot, yHot, cX, cY));

	// turn current cursor off
	CursorOff();

	// release memory associated with old cursor
	if (m_CursorBackingStore)
	{
		delete (void*)m_CursorBackingStore;
		m_CursorBackingStore = NULL;
	}
	if (m_CursorXorShape)
	{
		delete (void*)m_CursorXorShape;
        m_CursorXorShape = NULL;
	}
	if (m_CursorAndShape)
	{
		delete (void*)m_CursorAndShape;
        m_CursorAndShape = NULL;
	}

	if (!pMask)							// do we have a new cursor shape
	{
		m_CursorDisabled = TRUE;		// no, so tag as disabled
	}
	else
	{
		m_CursorDisabled = FALSE;		// yes, so tag as not disabled

		// allocate memory based on new cursor size
		m_CursorBackingStore = new UCHAR[(cX * (m_colorDepth >> 3)) * cY];
		m_CursorXorShape = new UCHAR[cX * cY];
		m_CursorAndShape = new UCHAR[cX * cY];

		// store size and hotspot for new cursor
		m_CursorSize.x = cX;
		m_CursorSize.y = cY;
		m_CursorHotspot.x = xHot;
		m_CursorHotspot.y = yHot;

		andPtr = (UCHAR*)pMask->Buffer();
		xorPtr = (UCHAR*)pMask->Buffer() + (cY * pMask->Stride());

		// store OR and AND mask for new cursor
		for (row = 0; row < cY; row++)
		{
			andLine = &m_CursorAndShape[cX * row];
			xorLine = &m_CursorXorShape[cX * row];

			for (col = 0; col < cX / 8; col++)
			{
				bAnd = andPtr[row * pMask->Stride() + col];
				bXor = xorPtr[row * pMask->Stride() + col];

				for (bitMask = 0x0080, i = 0; i < 8; bitMask >>= 1, i++)
				{
					andLine[(col * 8) + i] = bAnd & bitMask ? 0xFF : 0x00;
					xorLine[(col * 8) + i] = bXor & bitMask ? 0xFF : 0x00;
				}
			}
		}
	}

	return	S_OK;
}

SCODE	S3C2440DISP::MovePointer(INT xPosition, INT yPosition)
{
	RETAILMSG(DBGLCD1, (TEXT("S3C2440DISP::MovePointer(%d, %d)\r\n"), xPosition, yPosition));

	CursorOff();

	if (xPosition != -1 || yPosition != -1)
	{
		// compute new cursor rect
		m_CursorRect.left = xPosition - m_CursorHotspot.x;
		m_CursorRect.right = m_CursorRect.left + m_CursorSize.x;
		m_CursorRect.top = yPosition - m_CursorHotspot.y;
		m_CursorRect.bottom = m_CursorRect.top + m_CursorSize.y;

		CursorOn();
	}

	RETAILMSG(DBGLCD1, (TEXT("--S3C2440DISP::SetPointerShape()\r\n")));

	return	S_OK;
}

void	S3C2440DISP::WaitForNotBusy(void)
{
	RETAILMSG(DBGLCD1, (TEXT("S3C2440DISP::WaitForNotBusy\r\n")));
	return;
}

int		S3C2440DISP::IsBusy(void)
{
	RETAILMSG(DBGLCD1, (TEXT("S3C2440DISP::IsBusy\r\n")));
	return	0;
}

void	S3C2440DISP::GetPhysicalVideoMemory(unsigned long *physicalMemoryBase, unsigned long *videoMemorySize)
{
	RETAILMSG(DBGLCD, (TEXT("S3C2440DISP::GetPhysicalVideoMemory\r\n")));

	*physicalMemoryBase = gdwLCDVirtualFrameBase;
	*videoMemorySize = m_cbScanLineLength * m_nScreenHeight;
	//*videoMemorySize = m_cbScanLineLength * m_cyPhysicalScreen;
}

void	S3C2440DISP::GetVirtualVideoMemory(unsigned long *virtualMemoryBase, unsigned long *videoMemorySize)
{
	RETAILMSG(DBGLCD1, (TEXT("+++S3C2440DISP::GetVirtualVideoMemory\r\n")));

	*virtualMemoryBase = m_VirtualFrameBuffer;

	//*videoMemorySize = m_cbScanLineLength * m_cyPhysicalScreen;
	*videoMemorySize = 0x1A0000;
	RETAILMSG(DBGLCD1, (TEXT("---S3C2440DISP::GetVirtualVideoMemory\r\n")));
}

SCODE	S3C2440DISP::WrappedEmulatedLine (GPELineParms *lineParameters)
{
	SCODE	retval;
	RECT	bounds;
	int		N_plus_1;				// Minor length of bounding rect + 1

	RETAILMSG(DBGLCD1, (TEXT("++S3C2440DISP::WrappedEmulatedLine\r\n")));

	// calculate the bounding-rect to determine overlap with cursor
	if (lineParameters->dN)			// The line has a diagonal component (we'll refresh the bounding rect)
	{
		N_plus_1 = 2 + ((lineParameters->cPels * lineParameters->dN) / lineParameters->dM);
	}
	else
	{
		N_plus_1 = 1;
	}

	switch(lineParameters->iDir)
	{
		case 0:
			bounds.left = lineParameters->xStart;
			bounds.top = lineParameters->yStart;
			bounds.right = lineParameters->xStart + lineParameters->cPels + 1;
			bounds.bottom = bounds.top + N_plus_1;
			break;
		case 1:
			bounds.left = lineParameters->xStart;
			bounds.top = lineParameters->yStart;
			bounds.bottom = lineParameters->yStart + lineParameters->cPels + 1;
			bounds.right = bounds.left + N_plus_1;
			break;
		case 2:
			bounds.right = lineParameters->xStart + 1;
			bounds.top = lineParameters->yStart;
			bounds.bottom = lineParameters->yStart + lineParameters->cPels + 1;
			bounds.left = bounds.right - N_plus_1;
			break;
		case 3:
			bounds.right = lineParameters->xStart + 1;
			bounds.top = lineParameters->yStart;
			bounds.left = lineParameters->xStart - lineParameters->cPels;
			bounds.bottom = bounds.top + N_plus_1;
			break;
		case 4:
			bounds.right = lineParameters->xStart + 1;
			bounds.bottom = lineParameters->yStart + 1;
			bounds.left = lineParameters->xStart - lineParameters->cPels;
			bounds.top = bounds.bottom - N_plus_1;
			break;
		case 5:
			bounds.right = lineParameters->xStart + 1;
			bounds.bottom = lineParameters->yStart + 1;
			bounds.top = lineParameters->yStart - lineParameters->cPels;
			bounds.left = bounds.right - N_plus_1;
			break;
		case 6:
			bounds.left = lineParameters->xStart;
			bounds.bottom = lineParameters->yStart + 1;
			bounds.top = lineParameters->yStart - lineParameters->cPels;
			bounds.right = bounds.left + N_plus_1;
			break;
		case 7:
			bounds.left = lineParameters->xStart;
			bounds.bottom = lineParameters->yStart + 1;
			bounds.right = lineParameters->xStart + lineParameters->cPels + 1;
			bounds.top = bounds.bottom - N_plus_1;
			break;
		default:
			RETAILMSG(DBGLCD1, (TEXT("Invalid direction: %d\r\n"), lineParameters->iDir));
			return E_INVALIDARG;
	}

	// check for line overlap with cursor and turn off cursor if overlaps
	if (m_CursorVisible && !m_CursorDisabled &&
		m_CursorRect.top < bounds.bottom && m_CursorRect.bottom > bounds.top &&
		m_CursorRect.left < bounds.right && m_CursorRect.right > bounds.left)
	{
		CursorOff();
		m_CursorForcedOff = TRUE;
	}

	// do emulated line
	retval = EmulatedLine (lineParameters);

	// se if cursor was forced off because of overlap with line bouneds and turn back on
	if (m_CursorForcedOff)
	{
		m_CursorForcedOff = FALSE;
		CursorOn();
	}

	RETAILMSG(DBGLCD1, (TEXT("--S3C2440DISP::WrappedEmulatedLine\r\n")));

	return	retval;

}


SCODE	S3C2440DISP::Line(GPELineParms *lineParameters, EGPEPhase phase)
{
	RETAILMSG(DBGLCD1, (TEXT("++S3C2440DISP::Line\r\n")));

	if (phase == gpeSingle || phase == gpePrepare)
	{
		//DispPerfStart(ROP_LINE); //Guru

		if ((lineParameters->pDst != m_pPrimarySurface))
		{
			lineParameters->pLine = &GPE::EmulatedLine;
		}
		else
		{
			lineParameters->pLine = (SCODE (GPE::*)(struct GPELineParms *)) &S3C2440DISP::WrappedEmulatedLine;
		}
	}
	else if (phase == gpeComplete)
	{
	}
	RETAILMSG(DBGLCD1, (TEXT("--S3C2440DISP::Line\r\n")));
	return S_OK;
}


SCODE	S3C2440DISP::BltPrepare(GPEBltParms *blitParameters)
{
	RECTL	rectl;

	RETAILMSG(DBGLCD1, (TEXT("++S3C2440DISP::BltPrepare\r\n")));
	//DispPerfStart(blitParameters->rop4); //Guru

	// default to base EmulatedBlt routine
	blitParameters->pBlt = &GPE::EmulatedBlt;

	// see if we need to deal with cursor
	if (m_CursorVisible && !m_CursorDisabled)
	{
		// check for destination overlap with cursor and turn off cursor if overlaps
		if (blitParameters->pDst == m_pPrimarySurface)	// only care if dest is main display surface
		{
			if (blitParameters->prclDst != NULL)		// make sure there is a valid prclDst
			{
				rectl = *blitParameters->prclDst;		// if so, use it
			}
			else
			{
				rectl = m_CursorRect;					// if not, use the Cursor rect - this forces the cursor to be turned off in this case
			}

			if (m_CursorRect.top <= rectl.bottom && m_CursorRect.bottom >= rectl.top &&
				m_CursorRect.left <= rectl.right && m_CursorRect.right >= rectl.left)
			{
				CursorOff();
				m_CursorForcedOff = TRUE;
			}
		}

		// check for source overlap with cursor and turn off cursor if overlaps
		if (blitParameters->pSrc == m_pPrimarySurface)	// only care if source is main display surface
		{
			if (blitParameters->prclSrc != NULL)		// make sure there is a valid prclSrc
			{
				rectl = *blitParameters->prclSrc;		// if so, use it
			}
			else
			{
				rectl = m_CursorRect;					// if not, use the CUrsor rect - this forces the cursor to be turned off in this case
			}
			if (m_CursorRect.top < rectl.bottom && m_CursorRect.bottom > rectl.top &&
				m_CursorRect.left < rectl.right && m_CursorRect.right > rectl.left)
			{
				CursorOff();
				m_CursorForcedOff = TRUE;
			}
		}
	}


	// see if there are any optimized software blits available
	EmulatedBltSelect02(blitParameters);
	EmulatedBltSelect08(blitParameters);
	EmulatedBltSelect16(blitParameters);

	RETAILMSG(DBGLCD1, (TEXT("--S3C2440DISP::BltPrepare\r\n")));

	return S_OK;
}

SCODE	S3C2440DISP::BltComplete(GPEBltParms *blitParameters)
{
	RETAILMSG(DBGLCD1, (TEXT("++S3C2440DISP::BltComplete\r\n")));

	// see if cursor was forced off because of overlap with source or destination and turn back on
	if (m_CursorForcedOff)
	{
		m_CursorForcedOff = FALSE;
		CursorOn();
	}

	//DispPerfEnd(0);
	
	RETAILMSG(DBGLCD1, (TEXT("--S3C2440DISP::BltComplete\r\n")));
	return S_OK;
}

INT		S3C2440DISP::InVBlank(void)
{
	static	BOOL	value = FALSE;
	RETAILMSG(DBGLCD, (TEXT("S3C2440DISP::InVBlank\r\n")));
	value = !value;
	return value;
}

SCODE	S3C2440DISP::SetPalette(const PALETTEENTRY *source, USHORT firstEntry, USHORT numEntries)
{
	RETAILMSG(DBGLCD, (TEXT("S3C2440DISP::SetPalette\r\n")));

	if (firstEntry < 0 || firstEntry + numEntries > 256 || source == NULL)
	{
		return	E_INVALIDARG;
	}

	return	S_OK;
}


VIDEO_POWER_STATE
PmToVideoPowerState(CEDEVICE_POWER_STATE pmDx)
{
	VIDEO_POWER_STATE vps;
	RETAILMSG(DBGLCD,(_T("++PmToVideoPowerState\r\n")));

	switch(pmDx) {
	case D0:        // turn the display on
		vps = VideoPowerOn;
		break;

	case D1:        // if asked for a state we don't support, go to the next lower one
	case D2:
	case D3:
	case D4:
		vps = VideoPowerOff;
		break;

	default:
		RETAILMSG(DBGLCD , (L"PmToVideoPowerState: mapping unknown PM state %d to VideoPowerOn\r\n", pmDx));
		vps = VideoPowerOn;
		break;
	}

	return vps;
}

// this routine maps video power states to PM power states.
CEDEVICE_POWER_STATE
VideoToPmPowerState(VIDEO_POWER_STATE vps)
{
	CEDEVICE_POWER_STATE pmDx;
	RETAILMSG(DBGLCD,(_T("++VideoToPmPowerState\r\n")));

	switch(vps)
	{
	case VideoPowerOn:
		pmDx = D0;
		break;

	case VideoPowerStandBy:
		pmDx = D1;
		break;

	case VideoPowerSuspend:
		pmDx = (CEDEVICE_POWER_STATE)D2;
		break;

	case VideoPowerOff:
		pmDx = (CEDEVICE_POWER_STATE)D4;
		break;

	default:
		pmDx = D0;
		RETAILMSG(DBGLCD, (L"VideoToPmPowerState: mapping unknown video state %d to pm state %d\r\n",
		         vps, pmDx));
		break;
	}

	return pmDx;
}

#define ESC_SUCCESS             0x00000001
#define ESC_FAILED              0xFFFFFFFF
#define ESC_NOT_SUPPORTED       0x00000000
ULONG  S3C2440DISP::DrvEscape(
                        SURFOBJ *pso,
                        ULONG    iEsc,
                        ULONG    cjIn,
                        PVOID    pvIn,
                        ULONG    cjOut,
                        PVOID    pvOut)
{	
	
    ULONG Result = 0;
	
    LPWSTR pszFname = L"S3C2440LCD::DrvEscape";

	RETAILMSG(DBGLCD1,(_T("++DrvEscape\r\n")));
	
    switch (iEsc)
    {
	case QUERYESCSUPPORT:
		if (sizeof (DWORD) == cjIn)
		{
			DWORD SupportChk;
			SupportChk = *(DWORD *)pvIn;
			
			if ((SupportChk == QUERYESCSUPPORT)          ||
			    (SupportChk == GETGXINFO)				 ||
				(SupportChk == GETPOWERMANAGEMENT)       ||
				(SupportChk == SETPOWERMANAGEMENT)       ||
				(SupportChk == IOCTL_POWER_CAPABILITIES) ||
				(SupportChk == IOCTL_POWER_QUERY)        ||
				(SupportChk == IOCTL_POWER_SET)          ||
				(SupportChk == IOCTL_POWER_GET))
			{
				Result = ESC_SUCCESS;
			}
			else
			{
				Result = ESC_NOT_SUPPORTED;
			}
		}
		else
		{
			SetLastError(ERROR_INVALID_PARAMETER);
			Result = ESC_FAILED;
		}
        break;

	case DRVESC_GETSCREENROTATION:
		RETAILMSG(DBGLCD,(TEXT("DRVESC_GETSCREENROTATION\r\n")));
		*(int *)pvOut = ((DMDO_0 | DMDO_90 | DMDO_180 | DMDO_270) << 8) | ((BYTE)m_iRotate);
        	Result =  DISP_CHANGE_SUCCESSFUL;
		break;
		
	case DRVESC_SETSCREENROTATION:
		RETAILMSG(DBGLCD,(TEXT("DRVESC_SETSCREENROTATION\r\n")));
		if ((cjIn == DMDO_0) ||(cjIn == DMDO_90) ||(cjIn == DMDO_180) ||(cjIn == DMDO_270))
            	{
                	return DynRotate(cjIn);
            	}
        	Result = DISP_CHANGE_BADMODE;
		break;
    
  
	case GETGXINFO:
		RETAILMSG(DBGLCD,(TEXT("GETGXINFO\r\n")));
		Result = GetGameXInfo(iEsc, cjIn, pvIn, cjOut, pvOut);
		if (Result != ESC_SUCCESS)
		{
			// Shouldn't get here if everything was ok.
			SetLastError(ERROR_INVALID_PARAMETER);
			Result = ESC_FAILED;
		}
		break;
    
	case SETPOWERMANAGEMENT:
		RETAILMSG(DBGLCD,(TEXT("SETPOWERMANAGEMENT\r\n")));
		if ((cjIn >= sizeof (VIDEO_POWER_MANAGEMENT)) && (pvIn != NULL))
		{
			PVIDEO_POWER_MANAGEMENT pvpm = (PVIDEO_POWER_MANAGEMENT)pvIn;

 			if (pvpm->Length >= sizeof (VIDEO_POWER_MANAGEMENT))
			{
				switch (pvpm->PowerState)
				{
					case VideoPowerStandBy:
					case VideoPowerOn:
						SetDisplayPower(VideoPowerOn);
						Result = ESC_SUCCESS;
						break;
						
					case VideoPowerOff:
					case VideoPowerSuspend:
						SetDisplayPower(VideoPowerOff);
						Result = ESC_SUCCESS;
						break;
				}
			}
		}

		if (Result != ESC_SUCCESS)
		{
			// Shouldn't get here if everything was ok.
			SetLastError(ERROR_INVALID_PARAMETER);
			Result = ESC_FAILED;
		}
		break;

	case GETPOWERMANAGEMENT:
		RETAILMSG(DBGLCD, (L"%s::GETPOWERMANAGEMENT\n", pszFname));
		if ((cjOut >= sizeof (VIDEO_POWER_MANAGEMENT)) && (pvOut != NULL))
		{
			PVIDEO_POWER_MANAGEMENT pvpm = (PVIDEO_POWER_MANAGEMENT)pvOut;

			pvpm->Length = sizeof (VIDEO_POWER_MANAGEMENT);
			pvpm->DPMSVersion = 0;

			pvpm->PowerState = m_VideoPowerState;

			Result = ESC_SUCCESS;
		}
		else
		{
			// Shouldn't get here if everything was ok.
			SetLastError(ERROR_INVALID_PARAMETER);
			Result = ESC_FAILED;
		}
		break;

	case IOCTL_POWER_CAPABILITIES:
		// tell the power manager about ourselves
		RETAILMSG(DBGLCD, (L"%s: IOCTL_POWER_CAPABILITIES\r\n", pszFname));
		if (pvOut != NULL && cjOut == sizeof(POWER_CAPABILITIES))
		{
			__try
			{
				PPOWER_CAPABILITIES ppc = (PPOWER_CAPABILITIES) pvOut;
				memset(ppc, 0, sizeof(*ppc));
				ppc->DeviceDx = 0x11;	// support D0 and D4
				Result = ESC_SUCCESS;
			}
			__except(EXCEPTION_EXECUTE_HANDLER)
			{
				RETAILMSG(DBGLCD, (L"%s: exception in ioctl1\r\n"));
			}
		}
		break;

	case IOCTL_POWER_QUERY:
		RETAILMSG(DBGLCD,(TEXT("IOCTL_POWER_QUERY\r\n")));
		if(pvOut != NULL && cjOut == sizeof(CEDEVICE_POWER_STATE))
		{
			// return a good status on any valid query, since we are always ready to
			// change power states.
			__try
			{
				CEDEVICE_POWER_STATE NewDx = *(PCEDEVICE_POWER_STATE) pvOut;
				if(VALID_DX(NewDx))
				{
					// this is a valid Dx state so return a good status
					Result = ESC_SUCCESS;
				}
					RETAILMSG(DBGLCD, (L"%s: IOCTL_POWER_QUERY %u %s\r\n", pszFname, 
						NewDx, Result == ESC_SUCCESS ? L"succeeded" : L"failed"));
			}
			__except(EXCEPTION_EXECUTE_HANDLER)
			{
				RETAILMSG(DBGLCD, (L"%s: exception in ioctl2\r\n"));
			}
		}
		break;

	case IOCTL_POWER_SET:
		if(pvOut != NULL && cjOut == sizeof(CEDEVICE_POWER_STATE))
		{
			__try
			{
				CEDEVICE_POWER_STATE NewDx = *(PCEDEVICE_POWER_STATE) pvOut;
				CEDEVICE_POWER_STATE CurrentDx;
				if(VALID_DX(NewDx))
				{
					VIDEO_POWER_STATE ulPowerState = PmToVideoPowerState(NewDx);

					SetDisplayPower(ulPowerState);

					CurrentDx = VideoToPmPowerState((VIDEO_POWER_STATE)m_VideoPowerState);

					Result = ESC_SUCCESS;
					RETAILMSG(DBGLCD, (L"%s: IOCTL_POWER_SET %u: passing back %u\r\n", pszFname,
							NewDx, CurrentDx));
				}
				else
				{
					RETAILMSG(DBGLCD, (L"%s: IOCTL_POWER_SET: invalid state request %u\r\n", pszFname, NewDx));
				}
			}
			__except(EXCEPTION_EXECUTE_HANDLER)
			{
				RETAILMSG(DBGLCD, (L"%s: exception in ioctl3\r\n"));
			}
		}
		break;

	case IOCTL_POWER_GET:
		if(pvOut != NULL && cjOut == sizeof(CEDEVICE_POWER_STATE))
		{
			__try
			{
				CEDEVICE_POWER_STATE CurrentDx = D0;//VideoToPmPowerState((VIDEO_POWER_STATE)m_VideoPowerState);
				*(PCEDEVICE_POWER_STATE) pvOut = D0; //CurrentDx;
				Result = ESC_SUCCESS;
				RETAILMSG(DBGLCD, (L"%s: IOCTL_POWER_GET: passing back %u\r\n", pszFname, CurrentDx));
			}
			__except(EXCEPTION_EXECUTE_HANDLER)
			{
				RETAILMSG(DBGLCD, (L"%s: exception in ioctl4\r\n"));
			}
		}
		break;

	}
	RETAILMSG(DBGLCD1,(_T("--DrvEscape\r\n")));
	return Result;
}


#undef ESC_NOT_SUPPORTED
#undef ESC_FAILED
#undef ESC_SUCCESS

void S3C2440DISP::SetDisplayPower(ULONG PowerState)
{
	return;			//don't use this function
    	static BYTE * pVideoMemory = NULL;
	

    	WORD *ptr;

	RETAILMSG(DBGLCD,(_T("++SetDisplayPower\r\n")));

	// If we're already in the appropriate state, just return 
	if (m_VideoPowerState == PowerState)
	{
		return;
	}

	if (PowerState == VideoPowerOff)
	{
		m_VideoPowerState = VideoPowerOff;

		m_CursorDisabled = TRUE;
		CursorOff();

		// Save video memory
		if (NULL == pVideoMemory)
		{
			pVideoMemory = new BYTE [m_FrameBufferSize];
		}

		if (NULL != pVideoMemory)
		{
			memcpy(pVideoMemory, m_pPrimarySurface->Buffer(), m_FrameBufferSize);
		}

		// Blank the screen
		memset ((void*)m_pPrimarySurface->Buffer(), 0x0, m_FrameBufferSize);

		// Swap the buffer in the primary surface with that of our off screen buffer.
		if (NULL != pVideoMemory)
		{
			BYTE * pTempBuffer = (BYTE*)m_pPrimarySurface->Buffer();

			m_pPrimarySurface->Init(m_nScreenWidth, m_nScreenHeight, pVideoMemory, m_cbScanLineLength, m_pMode->format);

			DynRotate(m_iRotate);

			pVideoMemory = pTempBuffer;
		}

		// Let the setting take effect before disabling the controller
		Sleep(100);

		// disable LCD controller
		RETAILMSG(DBGLCD, (TEXT("SetDisplayPower: disable LCD controller \r\n")));

		// Set Board Control Register (BCR) for Sharp LCD mode
		//set_BCRVal (NOMASK, BCR_LCD_LQ039Q2DS01_16BPP | BCR_LCD_POWER_OFF | BCR_BACKLIGHT_OFF, INPOWERHANDLER);


		//v_s2440CLKPWR->CLKCON &= ~(1<<5);
		RETAILMSG(1,(TEXT("S3C2440DISP::SetDisplayPower Powered Down.\n")));
	}
	else
	{
		//v_s2440CLKPWR->CLKCON |= (1<<5);
		RETAILMSG(1,(TEXT("S3C2440DISP::SetDisplayPower Powered Up.\n")));

		m_VideoPowerState = VideoPowerOn;

		// init "palette" area - just has some flags for 16bpp active display mode
		ptr = (WORD*)m_VirtualFrameBuffer;
		ptr[0] = 0x2000;
		for(int index = 1; index < 16; index++)
		{
			ptr[index] = 0x00000000;
		}

		// clear rest of frame buffer out
		for(index = 0; index < 320*240; index++)
		{
			if(index < 3200)
			{
				ptr[index + 16] = 0xf800;
			}
			else if(index < 6400)
			{
				ptr[index + 16] = 0x07e0;
			}
			else if(index < 9600)
			{
				ptr[index + 16] = 0x001f;
			}
			else
			{
				ptr[index + 16] = 0xffff;
			}
		}


		// Restore the screen
		if (NULL != pVideoMemory)
		{
			// Swap the buffers.
			BYTE * pTempBuffer = (BYTE*)m_pPrimarySurface->Buffer();

			m_pPrimarySurface->Init(m_nScreenWidth, m_nScreenHeight, pVideoMemory, m_cbScanLineLength, m_pMode->format);

			pVideoMemory = pTempBuffer;

			// Actually copy the bits.
			memcpy(m_pPrimarySurface->Buffer(), pVideoMemory, m_FrameBufferSize);

			delete [] pVideoMemory;
			pVideoMemory = NULL;
		}
		else
		{
			memset ((void*)m_pPrimarySurface->Buffer(), 0x0, m_FrameBufferSize);
		}
		DynRotate(m_iRotate);

		m_CursorDisabled = FALSE;
		//CursorOn();
		m_CursorVisible = TRUE;
	}
}

//APR added func override
SCODE S3C2440DISP::GetModeInfoEx(GPEModeEx *pModeEx, int modeNo)
{
	RETAILMSG(DBGLCD, (TEXT("In S3C2440DISP::GetModeInfoEx\r\n")));
	if (modeNo != 0)
	{
		return    E_INVALIDARG;
	}

	*pModeEx = *m_pModeEx;

	return S_OK;
}

ULONG *APIENTRY DrvGetMasks(DHPDEV dhpdev)
{
	RETAILMSG(DBGLCD, (TEXT("DrvGetMasks\r\n")));
	if (gGPE == NULL)
		return(BitMasks[0]);

	int nBPP = ((S3C2440DISP *)gGPE)->GetBpp()/8 - 1;
	switch (((S3C2440DISP *)gGPE)->GetBpp())
	{
//		case 8:
		case 16:
		case 24:
		case 32:
			{
				return(BitMasks[nBPP]);            	
			}
				
	}

	return(BitMasks[0]);
	
	//return gBitMasks;
}

//
// GetBpp
//
// Return the Bpp of the current monitor
//
int S3C2440DISP::GetBpp(void)
{
    return m_pMode->Bpp;
}

//------------------------------------------------------------------------------
//
//   GetGameXInfo
//
//   fill out GAPI information for DrvEscape call
//
int S3C2440DISP::GetGameXInfo(
    ULONG iEsc, ULONG cjIn, VOID *pvIn, ULONG cjOut, VOID *pvOut
) {
	int rc = 0;  

	RETAILMSG(1, (L"+GetGameXInfo \r\n"));

	// GAPI only support P8, RGB444, RGB555, RGB565, and RGB888
	if (
		pvOut != NULL && cjOut >= sizeof(GXDeviceInfo) && (
		m_pMode->Bpp == 8 || m_pMode->Bpp == 12 ||
		m_pMode->Bpp == 16 || m_pMode->Bpp == 24 ||
		m_pMode->Bpp == 32
        )) 
	{
		if(((GXDeviceInfo*)pvOut)->idVersion == kidVersion100)
		{  
			GXDeviceInfo *pInfo = (GXDeviceInfo*)pvOut;
			pInfo->idVersion = kidVersion100;

			pInfo->pvFrameBuffer = (UCHAR*)m_pPrimarySurface->Buffer();
			pInfo->cbStride = m_pPrimarySurface->Stride();
			pInfo->cxWidth  = m_pPrimarySurface->Width();
			pInfo->cyHeight = m_pPrimarySurface->Height();

			switch (m_pMode->Bpp) {
				case 8:
					pInfo->cBPP = 8;
					pInfo->ffFormat |= kfPalette;
					break;
				case 12:
					pInfo->cBPP = 12;
					pInfo->ffFormat |= kfDirect | kfDirect444;
					break;
				case 16:
					pInfo->cBPP = 16;
					pInfo->ffFormat |= kfDirect | kfDirect565;
					break;
				case 24:
					pInfo->cBPP = 24;
					pInfo->ffFormat |= kfDirect | kfDirect888;
				case 32:
					pInfo->cBPP = 32;
					pInfo->ffFormat |= kfDirect | kfDirect888;
				default:
					goto cleanUp;
			}

			RETAILMSG(1,(TEXT("Stride=%d, Width=%d, Height=%d, FrmaeBuffer=%x \r\n"),pInfo->cbStride,pInfo->cxWidth,pInfo->cyHeight,pInfo->pvFrameBuffer));

			if (m_iRotate == DMDO_90 || m_iRotate == DMDO_270 )
				pInfo->ffFormat |= kfLandscape;  // Rotated
			// todo: get keys from registry
			pInfo->vkButtonUpPortrait = VK_UP;
			pInfo->vkButtonUpLandscape = VK_LEFT;
			pInfo->ptButtonUp.x = 88;
			pInfo->ptButtonUp.y = 250;
			pInfo->vkButtonDownPortrait = VK_DOWN;
			pInfo->vkButtonDownLandscape = VK_RIGHT;
			pInfo->ptButtonDown.x = 88;
			pInfo->ptButtonDown.y = 270;
			pInfo->vkButtonLeftPortrait = VK_LEFT;
			pInfo->vkButtonLeftLandscape = VK_DOWN;
			pInfo->ptButtonLeft.x = 78;
			pInfo->ptButtonLeft.y = 260;
			pInfo->vkButtonRightPortrait = VK_RIGHT;
			pInfo->vkButtonRightLandscape = VK_UP;
			pInfo->ptButtonRight.x = 98;
			pInfo->ptButtonRight.y = 260;

#if 1			// default to Smartphone settings for now
			pInfo->vkButtonAPortrait = VK_F1;   // Softkey 1
			pInfo->vkButtonALandscape = VK_F1;
			pInfo->ptButtonA.x = 10;
			pInfo->ptButtonA.y = 240;
			pInfo->vkButtonBPortrait = VK_F2;   // Softkey 2
			pInfo->vkButtonBLandscape = VK_F2;
			pInfo->ptButtonB.x = 166;
			pInfo->ptButtonB.y = 240;
			pInfo->vkButtonCPortrait = VK_F8;   // Asterisk on keypad
			pInfo->vkButtonCLandscape = VK_F8;
			pInfo->ptButtonC.x = 10;
			pInfo->ptButtonC.y = 320;
#else
			//  Added For GAPI to map Literal A, B, C
			pInfo->vkButtonAPortrait = VK_APP_LAUNCH1;
			pInfo->vkButtonALandscape = VK_APP_LAUNCH1;
			pInfo->ptButtonA.x = 10;
			pInfo->ptButtonA.y = 240;
			pInfo->vkButtonBPortrait = VK_APP_LAUNCH4;
			pInfo->vkButtonBLandscape = VK_APP_LAUNCH4;
			pInfo->ptButtonB.x = 166;
			pInfo->ptButtonB.y = 240;
			pInfo->vkButtonCPortrait = VK_APP_LAUNCH2;   // Asterisk on keypad
			pInfo->vkButtonCLandscape = VK_APP_LAUNCH2;
			pInfo->ptButtonC.x = 10;
			pInfo->ptButtonC.y = 320;
#endif
			pInfo->vkButtonStartPortrait  = '\r';
			pInfo->vkButtonStartLandscape = '\r';
			pInfo->ptButtonStart.x = 88;
			pInfo->ptButtonStart.y = 260;
			pInfo->pvReserved1 = (void *) 0;
			pInfo->pvReserved2 = (void *) 0;
			RETAILMSG(1, (L"-GetGameXInfo \r\n"));
			rc = 1;
		}
	}
	else {
		SetLastError (ERROR_INVALID_PARAMETER);
		rc = -1;
	}

cleanUp:
	RETAILMSG(DBGLCD, (L"GetGameXInfo(rc = %d\r\n", rc));
	return rc;
}

int S3C2440DISP::GetRotateModeFromReg()
{
	HKEY hKey;

	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\GDI\\ROTATION"), 0, 0, &hKey))
	{
		DWORD dwSize, dwAngle, dwType = REG_DWORD;
		dwSize = sizeof(DWORD);
		if (ERROR_SUCCESS == RegQueryValueEx(hKey,
                                             TEXT("ANGLE"),
                                             NULL,
                                             &dwType,
                                             (LPBYTE)&dwAngle,
                                             &dwSize))
		{
			switch (dwAngle)
			{
				case 0:
					return DMDO_0;
	
				case 90:
					return DMDO_90;

				case 180:
					return DMDO_180;

				case 270:
					return DMDO_270;

				default:
					return DMDO_0;
			}
		}

		RegCloseKey(hKey);
	}

	return DMDO_0;
}

void S3C2440DISP::SetRotateParams()
{
	switch(m_iRotate)
	{
		case DMDO_90:
		case DMDO_270:
			m_pMode->height = m_nScreenWidthSave;
			m_pMode->width = m_nScreenHeightSave;
			break;

		case DMDO_0:
		case DMDO_180:
		default:
			m_pMode->width = m_nScreenWidthSave;
			m_pMode->height = m_nScreenHeightSave;
		break;
	}
	return;
}

LONG S3C2440DISP::DynRotate(int angle)
{
	int curRot, oldAngle;
	int ang2idxArr[5]={0,1,2,2,3};
	int angArr[4]={0,1,2,4};

	RETAILMSG(DBGLCD,(TEXT("++S3C2440DISP::DynRotate()\r\n")));
	oldAngle = m_iRotate;
	m_iRotate = angle;

	curRot = ang2idxArr[angle] - ang2idxArr[oldAngle];
	if(curRot < 0)
		curRot += 4;

	//RotateCursorShape(angArr[curRot]);

	SetRotateParams();

	m_nScreenWidth = m_pMode->width;
	m_nScreenHeight = m_pMode->height;
	m_pPrimarySurface->SetRotation(m_nScreenWidth, m_nScreenHeight, angle);

	RETAILMSG(DBGLCD,(TEXT("--S3C2440DISP::DynRotate()\r\n")));
	return DISP_CHANGE_SUCCESSFUL;
}

SCODE S3C2440DISP::RotateCursorShape(int angle)
{
#if 0 	//{
	if (angle == 0)
		return S_OK;

	BYTE *pByte, *pCursor;
	ULONG ulBase, ulIndex;
	BYTE srcAnd[128],srcXor[128], dstAnd[128], dstXor[128]; /* 128 = 4 x 32 */
	BYTE  jMask,jAnd,jXor,bitMask;
	int x, y, cx = 32, cy = 32, tx;

	pCursor = pByte = m_pLAW + m_nCursorMemory + 1024;

	memset(srcAnd, 0x00, sizeof(srcAnd));
	memset(srcXor, 0x00, sizeof(srcXor));
	memset(dstAnd, 0x00, sizeof(dstAnd));
	memset(dstXor, 0x00, sizeof(dstXor));

	//Save the original pointer shape into local memory shapeRow[]
	for (y=0; y<cy; y++)
	{
		srcAnd[y*4+0] = pByte[0x0];
		srcXor[y*4+0] = pByte[0x1];
		srcAnd[y*4+1] = pByte[0x2];
		srcXor[y*4+1] = pByte[0x3];
		srcAnd[y*4+2] = pByte[0x8];
		srcXor[y*4+2] = pByte[0x9];
		srcAnd[y*4+3] = pByte[0xA];
		srcXor[y*4+3] = pByte[0xB];

		pByte+=16;
	}

//	pShape = shapeSrcRow;
	switch (angle)
	{
		case DMDO_90:
			for (y = 0; y < cy; y++)
			{		
				jMask = 0x80 >> (y&7);
				for (x = 0; x < cx; x++)
				{
					ulBase  = (31 - x) * 4;
					ulIndex = (ulBase + y / 8);

					bitMask = 0x80 >> (x&7);
					jAnd = srcAnd[y*4 + (x/8)];
					jXor = srcXor[y*4 + (x/8)];

					if (jAnd & bitMask)
					{
						dstAnd[ulIndex] |= jMask;
					}
					if (jXor & bitMask)
					{
						dstXor[ulIndex] |= jMask;
					}
				}
			}

			tx = m_nXHot;
			m_nXHot = m_nYHot;
			m_nYHot = 31 - tx;
			break;

		case DMDO_180:
			for (y = 0; y < cy; y++)
			{
				ulBase = (31 - y) * 4;
				for (x = 0; x < cx; x++)
				{
					jAnd  = srcAnd[y*4 + x/8];
					jXor  = srcXor[y*4 + x/8];
					bitMask  = 0x80 >> (x & 7);
					jMask = 0x01 << (x & 7);

					ulIndex = (ulBase + (32 - x - 1) / 8);
					if (jAnd & bitMask)
					{
						dstAnd[ulIndex] |= jMask;
					}
					if (jXor & bitMask)
					{
						dstXor[ulIndex] |= jMask;
					}
				}
			}

			m_nXHot = 31 - m_nXHot;
			m_nYHot = 31 - m_nYHot;
			break;

		case DMDO_270:
			for (y = 0; y < cy; y++)
			{
				jMask  = 0x01 << (y & 7);

				// Write available bits into shapeRow
				for (x = 0; x < cx; x++)
				{
					jAnd  = srcAnd[y*4 + x/8];
					jXor  = srcXor[y*4 + x/8];
					bitMask = 0x80 >> (x & 7);			// Starting here achieves byte swapping

					ulBase = x * 4;
					ulIndex = (ulBase + (31 - y) / 8);

					if (jAnd & bitMask)
						dstAnd[ulIndex] |= jMask;
					if (jXor & bitMask)
						dstXor[ulIndex] |= jMask;
				}
			}
			tx = m_nXHot;
			m_nXHot = 31 - m_nYHot;
			m_nYHot = tx;
			break;
	}

	for (y=0; y<cy; y++)
	{
		pCursor[0x0] = dstAnd[y*4+0];
		pCursor[0x1] = dstXor[y*4+0];
		pCursor[0x2] = dstAnd[y*4+1];
		pCursor[0x3] = dstXor[y*4+1];
		pCursor[0x8] = dstAnd[y*4+2];
		pCursor[0x9] = dstXor[y*4+2];
		pCursor[0xa] = dstAnd[y*4+3];
		pCursor[0xb] = dstXor[y*4+3];
		pCursor += 16;
	}
#endif	//}
	return S_OK;
}


