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
/*

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

*/

#include <windows.h>
#include <pm.h>
#include "bak_hw.h"



//  Global structure
extern BLStruct g_BLInfo;
extern HANDLE   g_evtSignal[NUM_EVENTS];

DWORD 
BAK_Init(DWORD dwContext)
{
	HANDLE  hThread;
	DWORD   dwThreadID;
	

	// Perform all one-time initialization of the backlight
	if (!BacklightInitialize())
	{
		RETAILMSG(1, (TEXT("BAK_Init:couldn't initialize backlight hardware \r\n")));
		return 0;
	}

	// Create the backlight service thread
	hThread = CreateThread(NULL, 0, BL_MonitorThread, NULL, 0, &dwThreadID);
	if (hThread == NULL) 
	{
		RETAILMSG(1, (TEXT("BAK_Init: failed to create BL_MonitorThread\r\n")));
		return 0;
	}

	return dwThreadID;
}



BOOL 
BAK_Deinit(DWORD dwContext)
{
	BL_Deinit();
    return TRUE;
}



DWORD 
BAK_Open(DWORD dwData, DWORD dwAccess, DWORD dwShareMode)
{
	return dwData;
}



BOOL 
BAK_Close(DWORD Handle)
{
    return TRUE;
}



void 
BAK_PowerDown(void)
{
    BL_On(FALSE);
}



void
BAK_PowerUp(void)
{
	BL_PowerOn(TRUE);
}



DWORD 
BAK_Read(DWORD Handle, LPVOID pBuffer, DWORD dwNumBytes)
{
	return 0;
}



DWORD 
BAK_Write(DWORD Handle, LPCVOID pBuffer, DWORD dwNumBytes)
{
	return 0;
}



DWORD 
BAK_Seek(DWORD Handle, long lDistance, DWORD dwMoveMethod)
{
	return (DWORD) -1;
}



BOOL 
BAK_IOControl(
			  DWORD Handle, 
			  DWORD dwCode, 
			  PBYTE pBufIn, 
			  DWORD dwLenIn,
			  PBYTE pBufOut, 
			  DWORD dwLenOut, 
			  PDWORD pdwActualOut
			  )
{
    BOOL RetVal = TRUE;
    DWORD dwErr = ERROR_SUCCESS;    

    switch (dwCode) 
	{
        //
        // Power Management
        //
		case IOCTL_POWER_CAPABILITIES: 
        {
            PPOWER_CAPABILITIES ppc;
            
			if ( !pdwActualOut || !pBufOut || (dwLenOut < sizeof(POWER_CAPABILITIES)) ) {
                RetVal = FALSE;
                dwErr = ERROR_INVALID_PARAMETER;
                break;
            }
			
            ppc = (PPOWER_CAPABILITIES)pBufOut;
            
            memset(ppc, 0, sizeof(POWER_CAPABILITIES));

            // support D0, D4 
            ppc->DeviceDx = 0x11;

   
			// 25 m = 25000 uA
            // TODO: find out a more accurate value
			ppc->Power[D0] = 25000;
            
            *pdwActualOut = sizeof(POWER_CAPABILITIES);
        } break;

		case IOCTL_POWER_SET: 
        {
            CEDEVICE_POWER_STATE NewDx;

            if ( !pdwActualOut || !pBufOut || (dwLenOut < sizeof(CEDEVICE_POWER_STATE)) ) {
                RetVal = FALSE;
                dwErr = ERROR_INVALID_PARAMETER;
                break;
            }
            
            NewDx = *(PCEDEVICE_POWER_STATE)pBufOut;

            if ( VALID_DX(NewDx) ) {
                switch ( NewDx ) {
                case D0:
                    //  Power changed, we need to notify the monitor thread to resync
                    //  the timer
                    SetEvent(g_evtSignal[BL_POWEREVT]);
                    BL_On(TRUE);
                    break;

                default:
                    BL_On(FALSE);
                    break;
                }

                *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);
            } else {
                RetVal = FALSE;
                dwErr = ERROR_INVALID_PARAMETER;
            }
            
        } break;

        case IOCTL_POWER_GET: 
            if ( !pdwActualOut || !pBufOut || (dwLenOut < sizeof(CEDEVICE_POWER_STATE)) ) {
                RetVal = FALSE;
                dwErr = ERROR_INVALID_PARAMETER;
                break;
            }

			CEDEVICE_POWER_STATE Dx;


			if (g_BLInfo.m_dwStatus == BL_ON){
				Dx = D4;
			}
			else {
				Dx = D0;
			}

			*(PCEDEVICE_POWER_STATE)pBufOut = Dx;

            *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);
	        break;

		default:
            RetVal = FALSE;
            break;
	}
	
	return(RetVal);
}



BOOL
WINAPI
DllMain(
    HANDLE  hinstDll,
    DWORD   dwReason,
    LPVOID  lpReserved
    )
{
	switch(dwReason)
	{
		case DLL_PROCESS_ATTACH:
			break;

		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}

