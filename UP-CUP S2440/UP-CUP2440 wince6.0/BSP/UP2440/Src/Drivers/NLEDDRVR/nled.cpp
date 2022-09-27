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

Module Name:  

Abstract:  

Notes: 
--*/
#include <windows.h>
#include <nled.h>
#include <led_drvr.h>
#include <types.h>
#include "BSP.h"


BOOL InitializeAddresses(VOID);
void NLED_Thread(void);

// Pointer to device control registers
volatile S3C2440A_IOPORT_REG *v_pIOPregs;

HANDLE gLEDThread;
HANDLE gLEDEvent;
DWORD gLEDTimeout;

BOOL InitializeAddresses(VOID)
{
	BOOL	RetValue = TRUE;
	
	/* IO Register Allocation */
	v_pIOPregs = (volatile S3C2440A_IOPORT_REG *)VirtualAlloc(0, sizeof(S3C2440A_IOPORT_REG), MEM_RESERVE, PAGE_NOACCESS);
	if (v_pIOPregs == NULL) 
	{
		ERRORMSG(1,(TEXT("For IOPregs : VirtualAlloc failed!\r\n")));
		RetValue = FALSE;
	}
	else 
	{
		if (!VirtualCopy((PVOID)v_pIOPregs, (PVOID)(S3C2440A_BASE_REG_PA_IOPORT >> 8), sizeof(S3C2440A_IOPORT_REG), PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE)) 
		{
			ERRORMSG(1,(TEXT("For IOPregs: VirtualCopy failed!\r\n")));
			RetValue = FALSE;
		}
	}
	if (!RetValue) 
	{
		if (v_pIOPregs) 
		{
			VirtualFree((PVOID) v_pIOPregs, 0, MEM_RELEASE);
		}

		v_pIOPregs = NULL;
	}

	return(RetValue);
}

void NLED_Thread(void)
{
	BOOL LED_Blink=0;

	gLEDTimeout = INFINITE;
	gLEDEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	
	SetProcPermissions((DWORD)-1);

 	for (;;)
 	{ 	
	 	WaitForSingleObject(gLEDEvent, gLEDTimeout);

   		if (gLEDTimeout == 250)
   		{
   			LED_Blink ^= 1;
          	if (LED_Blink == TRUE)
   				;//v_pIOPregs->GPFDAT  &= ~(0x1 << 7);
  			else
        		;//v_pIOPregs->GPFDAT  |=  (0x1 << 7);
        }
 	}
}

// The NLED MDD calls this routine to initialize the underlying NLED hardware.
// This routine should return TRUE if successful.  If there's a problem
// it should return FALSE and call SetLastError() to pass back the reason
// for the failure.
BOOL WINAPI
NLedDriverInitialize(
                    VOID
                    )
{
    DEBUGMSG(ZONE_PDD, (_T("NLedDriverInitialize: invoked\r\n")));
    if (!InitializeAddresses())
    	return (FALSE);

    gLEDThread= CreateThread(0, 0, (LPTHREAD_START_ROUTINE) NLED_Thread, 0, 0, 0);

    return (TRUE);
}

// The NLED MDD calls this routine to deinitialize the underlying NLED
// hardware as the NLED driver is unloaded.  It should return TRUE if 
// successful.  If there's a problem this routine should return FALSE 
// and call SetLastError() to pass back the reason for the failure.
BOOL WINAPI
NLedDriverDeInitialize(
                    VOID
                    )
{
    DEBUGMSG(ZONE_PDD, (_T("NLedDriverDeInitialize: invoked\r\n")));

	if (v_pIOPregs) 
	{
		VirtualFree((PVOID) v_pIOPregs, 0, MEM_RELEASE);
		v_pIOPregs = NULL;
	}
	if(gLEDThread)
	{
		CloseHandle(gLEDThread);
		gLEDThread = NULL;	
	}
	if(gLEDEvent)
	{
		CloseHandle(gLEDEvent);
		gLEDEvent = NULL;	
	}
    
    return (TRUE);
}

// This routine retrieves information about the NLED device(s) that
// this driver supports.  The nInfoId parameter indicates what specific
// information is being queried and pOutput is a buffer to be filled in.
// The size of pOutput depends on the type of data being requested.  This
// routine returns TRUE if successful, or FALSE if there's a problem -- in
// which case it also calls SetLastError() to pass back more complete
// error information.  The NLED MDD invokes this routine when an application
// calls NLedGetDeviceInfo().
BOOL
WINAPI
NLedDriverGetDeviceInfo(
                       INT     nInfoId,
                       PVOID   pOutput
                       )
{
    BOOL fOk = TRUE;
    SETFNAME(_T("NLedDriverGetDeviceInfo"));
        
    if ( nInfoId == NLED_COUNT_INFO_ID ) {
        struct NLED_COUNT_INFO  *p = (struct NLED_COUNT_INFO*)pOutput;

        __try {
            p -> cLeds = 1;
        } 
        __except(EXCEPTION_EXECUTE_HANDLER) {
            SetLastError(ERROR_INVALID_PARAMETER);
            fOk = FALSE;
        }
    } else {
        fOk = FALSE;
        SetLastError(ERROR_INVALID_PARAMETER);
    }

    DEBUGMSG(ZONE_PDD || (!fOk && ZONE_WARN), 
        (_T("%s: returning %d\r\n"), pszFname, fOk));
    return (fOk);
}


// This routine changes the configuration of an LED.  The nInfoId parameter
// indicates what kind of configuration information is being changed.  
// Currently only the NLED_SETTINGS_INFO_ID value is supported.  The pInput
// parameter points to a buffer containing the data to be updated.  The size
// of the buffer depends on the value of nInfoId.  This routine returns TRUE
// if successful or FALSE if there's a problem -- in which case it also calls
// SetLastError().  The NLED MDD invokes this routine when an application 
// calls NLedSetDevice().
BOOL
WINAPI
NLedDriverSetDevice(
                   INT     nInfoId,
                   PVOID   pInput
                   )
{
    DEBUGMSG(ZONE_WARN, (_T("NLedDriverSetDevice: stub driver, can't set LED state\r\n")));
    
    struct NLED_SETTINGS_INFO *pInfo = (struct NLED_SETTINGS_INFO*)pInput;
    
    if(pInfo->OffOnBlink==0)
    {
		gLEDTimeout = INFINITE;
    }
    else if( pInfo->OffOnBlink==1)
    {
		gLEDTimeout = INFINITE;
    }
    else if( pInfo->OffOnBlink==2)
    {
		gLEDTimeout = 250;
    	SetEvent(gLEDEvent);
    }
    else 
    {
		SetLastError(ERROR_INVALID_PARAMETER);
		return (FALSE);
    }
    return (TRUE);
}


// This routine is invoked by the driver MDD when the system suspends or
// resumes.  The power_down flag indicates whether the system is powering 
// up or powering down.
VOID WINAPI
NLedDriverPowerDown(
                   BOOL power_down
                   )
{
    return;
}

