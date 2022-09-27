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
#include <nkintr.h>

#include <pm.h>
#include "bsp.h"
#include "bak_hw.h"

//  Globals
const TCHAR szevtBacklightChange[] = TEXT("BackLightChangeEvent");
const TCHAR szevtPowerChanged[] = TEXT("PowerChangedEvent");
const TCHAR szevtUserInput[] = TEXT("UserInputEvent");


const TCHAR szregRootKey[] = TEXT("ControlPanel\\Backlight");
const TCHAR szregBatteryTimeout[] = TEXT("BatteryTimeout");
const TCHAR szregACTimeout[] = TEXT("ACTimeout");
const TCHAR szregBatteryAuto[] = TEXT("BacklightOnTap");
const TCHAR szregACAuto[] = TEXT("ACBacklightOnTap");

HANDLE   g_evtSignal[NUM_EVENTS];


//  Global structure
BLStruct g_BLInfo;

//
// Perform all one-time initialization of the backlight
//
BOOL 
BacklightInitialize()
{
    BOOL    bRet = TRUE;

	BL_PowerOn(TRUE);

    return bRet;
}


//  Utility function to read from registry for the parameters
void BL_ReadRegistry(BLStruct *pBLInfo)
{
    HKEY    hKey;
    LONG    lResult;
    DWORD   dwType;
    DWORD   dwVal;
    DWORD   dwLen;

    lResult = RegOpenKeyEx(HKEY_CURRENT_USER, szregRootKey, 0, KEY_ALL_ACCESS, &hKey);
    if(ERROR_SUCCESS == lResult) {
        dwType = REG_DWORD;
        dwLen = sizeof(DWORD);

        lResult = RegQueryValueEx(hKey, szregBatteryTimeout, NULL, &dwType, 
                                  (LPBYTE)&dwVal, &dwLen);
        if(ERROR_SUCCESS == lResult) {
            pBLInfo->m_dwBatteryTimeout = dwVal;
        }

        lResult = RegQueryValueEx(hKey, szregACTimeout, NULL, &dwType, (LPBYTE)&dwVal,
                                  &dwLen);
        if(ERROR_SUCCESS == lResult) {
            pBLInfo->m_dwACTimeout = dwVal;
        }

        lResult = RegQueryValueEx(hKey, szregBatteryAuto, NULL, &dwType, (LPBYTE)&dwVal,
                                  &dwLen);
        if(ERROR_SUCCESS == lResult) {
            pBLInfo->m_bBatteryAuto = (BOOL) dwVal;
        }

        lResult = RegQueryValueEx(hKey, szregACAuto, NULL, &dwType, (LPBYTE)&dwVal,
                                  &dwLen);
        if(ERROR_SUCCESS == lResult) {
            pBLInfo->m_bACAuto = (BOOL) dwVal;
        }

        RegCloseKey(hKey);
    }
    else {
        RETAILMSG(1, (TEXT("BAK : HKEY_CURRENT_USER\\%s key doesn't exist!\r\n"), szregRootKey));
    }
}


// uninitialize the backlight
void BL_Deinit()
{
    int i;

    //  Clean up
    for(i=0; i<NUM_EVENTS; i++) {
        if(g_evtSignal[i]) {
            CloseHandle(g_evtSignal[i]);
        }
    }
}


//
// initialize the backlight
//
BOOL BL_Init()
{
    //  Set up all the events we need. 
    g_evtSignal[0] = CreateEvent(NULL, FALSE, FALSE, szevtBacklightChange);
    g_evtSignal[1] = CreateEvent(NULL, FALSE, FALSE, szevtUserInput);
    g_evtSignal[BL_POWEREVT] = CreateEvent(NULL, FALSE, FALSE, szevtPowerChanged);

    if(!g_evtSignal[0] || !g_evtSignal[1] || !g_evtSignal[2]) {
        BL_Deinit();
        return FALSE;
    }
    return TRUE;
}


//
//  find out if AC power is plugged in
//
BOOL IsACOn()
{
//    if (g_pDriverGlobals->power.ACLineStatus == AC_LINE_ONLINE)
//		return TRUE;
//    else
		return FALSE;
}


//
// turn on/off the backlight
//
void BL_On(BOOL bOn)
{
    if(bOn) {
		if (g_BLInfo.m_dwStatus != BL_ON)
		{
			g_BLInfo.m_dwStatus = BL_ON;
		}
    }
    else {
		if (g_BLInfo.m_dwStatus != BL_OFF)
		{
			g_BLInfo.m_dwStatus = BL_OFF;
		}
    }
}


//
// restore power to the backlight
//
void BL_PowerOn(BOOL bInit)
{
    //
    //  Add power-on GPIO register setting
    //

    BL_On(TRUE);
}


// The backlight handling is done by a thread, which monitors those 
// three event and performs some actions based on the parameters specified
// in HKLM/ControlPanel/Backlight
//
// backlight service thread
//
DWORD BL_MonitorThread(PVOID pParms)
{
    DWORD   dwResult;
    DWORD   dwTimeout;
    
    //  Initialization stuff is here
    //
    //  Initialize the events
    //  Initialize the BLInfo data structure
    //  Those are default values. Modify them if necessary
    g_BLInfo.m_bACAuto = TRUE;
    g_BLInfo.m_bBatteryAuto = TRUE;
    g_BLInfo.m_dwBatteryTimeout = 20;   // 20 Seconds
    g_BLInfo.m_dwACTimeout = 60;       // 1 minutes

    //  Now read from the registry to see what they say
    BL_ReadRegistry(&g_BLInfo);

    //  Initialize BL
    if(!BL_Init()) {
        RETAILMSG(1, (TEXT("BL_Init() Failed! Exit from BL_MonitorThread!\r\n")));
        return 0;
    }
    while(1) {
		__try { 
			//  If we are using AC now, use m_dwACTimeout as the timeout
			//  otherwise, use m_dwBatteryTimeout
			if(IsACOn()) {
				dwTimeout = g_BLInfo.m_dwACTimeout * 1000;
			}
			else {
				dwTimeout = g_BLInfo.m_dwBatteryTimeout * 1000;
			}

			//  However, if user wants BL on all the time, we have to let him
			//  do that. Or if we come back here, and BL is off, we want to 
			//  put this thread to sleep until other event happens.
			if(dwTimeout == 0 || g_BLInfo.m_dwStatus == BL_OFF) {
				dwTimeout = INFINITE;
			}

			//  Now let's wait for either there is an update on registry, or
			//  there is user action on the device, or there is activity on
			//  AC power supply.
			dwResult = WaitForMultipleObjects(NUM_EVENTS, &g_evtSignal[0], FALSE, dwTimeout);

			//  If we are signaled by registry event
			if(WAIT_OBJECT_0 == dwResult) {
				//  All we need to do is to read from registry and update the tick count
				BL_ReadRegistry(&g_BLInfo);

				//  Always turn on the Backlight after a change to registry
				BL_On(TRUE);
			}
			else if(dwResult == WAIT_OBJECT_0+1) {
				//  User activity, depending on the situation, we may / may not update 
				//  the tick count

				if(IsACOn()) {
					if(g_BLInfo.m_bACAuto) {
						//  Turn on backlight
						BL_On(TRUE);
					}
				}
				else {
					if(g_BLInfo.m_bBatteryAuto) {
						BL_On(TRUE);
					}
				}  
			}
			else if(dwResult == WAIT_OBJECT_0+2) {
				//  When AC is plugged or un-plugged, we don't really need to do anything
				//  We continue the loop. The correct timeout value will be assigned at
				//  the top of the while loop.
			}
			else if(dwResult == WAIT_TIMEOUT) {
				//  Time out, let's turn the device off
				BL_On(FALSE);
			}
		}
		__except(EXCEPTION_EXECUTE_HANDLER){
			// do nothing
			RETAILMSG(1, (TEXT("an exception is raised in BL_MonitorThread... \r\n")));
		}
	}
}



