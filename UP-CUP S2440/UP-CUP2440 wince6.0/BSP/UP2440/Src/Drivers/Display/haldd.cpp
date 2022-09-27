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

#ifdef DEBUG
#ifndef HAL_ZONE_INIT
#define HAL_ZONE_INIT     GPE_ZONE_INIT
#endif
#endif

//extern DDGPE* g_pGPE;
#define DBG_ON 0

DWORD WINAPI HalGetDriverInfo(LPDDHAL_GETDRIVERINFODATA lpInput)
{
	RETAILMSG(DBG_ON, (TEXT("GetDriverInfo invoked !!\r\n")));

    	lpInput->ddRVal = DDERR_CURRENTLYNOTAVAIL;

	if( lpInput->ddRVal != DD_OK )
	{
		RETAILMSG(DBG_ON, (TEXT("HalGetDriverInfo: Currently not available\r\n")));
	}

    	return DDHAL_DRIVER_HANDLED;
}


// not to be confused with
// DWORD WINAPI HalSetColorKey( LPDDHAL_SETCOLORKEYDATA pd )
DWORD WINAPI HalSetColorKey( LPDDHAL_SETCOLORKEYDATA pd )
{
	DEBUGENTER( HalSetColorKey );
	RETAILMSG(DBG_ON, (TEXT("HalSetColorKey\r\n")));
	
	pd->ddRVal = DD_OK;
	return DDHAL_DRIVER_HANDLED;
}

DWORD WINAPI HalGetScanLine( LPDDHAL_GETSCANLINEDATA pd )
{
	DEBUGENTER( HalGetScanLine );
	RETAILMSG(DBG_ON, (TEXT("HalGetScanLine\r\n")));
	
	pd->ddRVal = DDERR_UNSUPPORTED;
	return DDHAL_DRIVER_HANDLED;
}

