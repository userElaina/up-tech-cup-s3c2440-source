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

/* ++

    PCF50606 Power Supply Controller & Battery Management

    Notes:
    ======
    o) I2c client

-- */

#include <windows.h>
#include <nkintr.h>
#include <windev.h>
#include <winbase.h>

#include "pwr.h"

static DWORD PWR_IST(LPVOID Context);

// Make sure the driver builds OK in tinykern configuration
HMODULE g_hMod = NULL;
typedef void (*PFN_GwesPowerOffSystem)(void);
PFN_GwesPowerOffSystem g_pfnGwesPowerOffSystem = NULL;

UINT32 g_PwrButtonIrq = IRQ_EINT0;	// Determined by SMDK2440 board layout.
UINT32 g_PwrButtonSysIntr = SYSINTR_UNDEFINED;

S3C2440A_IOPORT_REG *v_pIOPregs;

#define ENABLE  TRUE
#define DISABLE FALSE

DWORD
HW_InitRegisters(
    PPWR_CONTEXT pPWR
    )
{
    DWORD retry = 0;

	v_pIOPregs = (volatile S3C2440A_IOPORT_REG *)VirtualAlloc(0, sizeof(S3C2440A_IOPORT_REG), MEM_RESERVE, PAGE_NOACCESS);
	VirtualCopy((PVOID)v_pIOPregs, (PVOID)(S3C2440A_BASE_REG_PA_IOPORT >> 8), sizeof(S3C2440A_IOPORT_REG), PAGE_PHYSICAL|PAGE_READWRITE|PAGE_NOCACHE );

	v_pIOPregs->GPFCON  &= ~(0x3 << 0);		/* Set EINT0(GPF0) as EINT0							*/
	v_pIOPregs->GPFCON  |=  (0x2 << 0);

	v_pIOPregs->EXTINT0 &= ~(0x7 << 0);
	v_pIOPregs->EXTINT0 |=  (0x2 << 0);		/* Configure EINT0 as Falling Edge Mode				*/
//	v_pIOPregs->EXTINT0 |=  (0x6 << 0);		/* Configure EINT0 as Both Edge Mode				*/

    return 0;
}


/* ++

    The reset value of the PCF50606 INTxM registers is 0: Interrupt enabled.
    Once we enable the interrupt they start firing unless we mask them _before_
    enabeling the interrupt(s).
    
-- */
DWORD
HW_Init(
    PPWR_CONTEXT pPWR
    )
{
    DWORD dwErr = ERROR_SUCCESS;
    BOOL    Ret;

    if ( !pPWR )
        return ERROR_INVALID_PARAMETER;

    // Init H/W
    pPWR->State = INITIALIZE;
    
    RETAILMSG(1, (TEXT("HW_Init : HW_InitRegisters \r\n")));
	dwErr = HW_InitRegisters(pPWR);
    if ( dwErr ) {
	    RETAILMSG(1, (TEXT("HW_Init : HW_InitRegisters  Error (0x%x) \r\n"), dwErr));
        goto _error_exit;
    }

    RETAILMSG(1, (TEXT("HW_Init : CreateEvent \r\n")));
	pPWR->ISTEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    // Request a SYSINTR value from the OAL.
    //
    RETAILMSG(1, (TEXT("HW_Init : IOCTL_HAL_REQUEST_SYSINTR \r\n")));
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &g_PwrButtonIrq, sizeof(UINT32), &g_PwrButtonSysIntr, sizeof(UINT32), NULL))
    {
        RETAILMSG(1, (TEXT("ERROR: PwrButton: Failed to request sysintr value for power button interrupt.\r\n")));
        return(0);
    }
    RETAILMSG(1,(TEXT("INFO: PwrButton: Mapped Irq 0x%x to SysIntr 0x%x.\r\n"), g_PwrButtonIrq, g_PwrButtonSysIntr));

	if (!(InterruptInitialize(g_PwrButtonSysIntr, pPWR->ISTEvent, 0, 0))) 
	{
		RETAILMSG(1, (TEXT("ERROR: PwrButton: Interrupt initialize failed.\r\n")));
	}

    RETAILMSG(1, (TEXT("HW_Init : CreateThread \r\n"), dwErr));
    if ( (pPWR->IST = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) PWR_IST, pPWR, 0, NULL)) == NULL) {
        dwErr = GetLastError();
        RETAILMSG(1,(TEXT("PCF50606 ERROR: Unable to create IST: %u\r\n"), dwErr));
        goto _error_exit;
    }
    
    RETAILMSG(1, (TEXT("HW_Init : CeSetThreadPriority \r\n"), dwErr));
    // TODO: registry override
    if ( !CeSetThreadPriority(pPWR->IST, POWER_THREAD_PRIORITY)) {
        dwErr = GetLastError();
        RETAILMSG(1, (TEXT("PCF50606 ERROR: CeSetThreadPriority ERROR:%d \r\n"), dwErr));
        goto _error_exit;
    }

    pPWR->State = RUN;

    RETAILMSG(1, (TEXT("HW_Init : Done \r\n"), dwErr));

_error_exit:
    return dwErr;
}


DWORD
HW_Deinit(
    PPWR_CONTEXT pPWR
    )
{
    if ( !pPWR )
        return ERROR_INVALID_PARAMETER;
    
    RETAILMSG(1,(TEXT("+PWR_Deinit \r\n")));

    pPWR->State = UNINITIALIZED;

    if (pPWR->hADC && CloseHandle(pPWR->hADC))
        pPWR->hADC = NULL;

    if (pPWR->hTSCPRES && CloseHandle(pPWR->hTSCPRES))
        pPWR->hTSCPRES = NULL;

    if (pPWR->hI2C && pPWR->hI2C != INVALID_HANDLE_VALUE && CloseHandle(pPWR->hI2C))
        pPWR->hI2C = INVALID_HANDLE_VALUE;
    
    InterruptDisable(g_PwrButtonSysIntr);

    if (pPWR->ISTEvent && CloseHandle(pPWR->ISTEvent))
        pPWR->ISTEvent = NULL;
    
    if (pPWR->IST && CloseHandle(pPWR->IST))
        pPWR->IST = NULL;  

    //  Free the coredll instance if we have allocated one
    if(g_hMod) {
        FreeLibrary(g_hMod);
        g_hMod = NULL;
    }

	VirtualFree((PVOID) v_pIOPregs, 0, MEM_RELEASE);
	// EINT0, GPF0 Port Init Done

    RETAILMSG(1,(TEXT("-PWR_Deinit \r\n")));

    return ERROR_SUCCESS;
}


DWORD 
HW_Open(
    PPWR_CONTEXT pPWR
    )
{
    RETAILMSG(1, (TEXT("PCF: HW_Open \r\n")));
    return ERROR_SUCCESS;
}


DWORD 
HW_Close(
    PPWR_CONTEXT pPWR
    )
{
    RETAILMSG(1, (TEXT("PCF: HW_Close \r\n")));
    return ERROR_SUCCESS;
}


BOOL
HW_PowerUp(
    PPWR_CONTEXT pPWR
   )
{
    RETAILMSG(1, (TEXT("PCF: HW_PowerUp \r\n")));
    pPWR->State = RESUME;
//    pPWR->State = RUN;
//    SetInterruptEvent(g_PwrButtonSysIntr);
    return TRUE;
}


BOOL
HW_PowerDown(
    PPWR_CONTEXT pPWR
   )
{
    RETAILMSG(1, (TEXT("PCF: HW_PowerDown \r\n")));
    pPWR->State = SUSPEND;
    return TRUE;
}


BOOL
HW_PowerCapabilities(
    PPWR_CONTEXT pPWR,
    PPOWER_CAPABILITIES ppc
    )
{
    return TRUE;
}


BOOL
HW_PowerSet(
    PPWR_CONTEXT pPWR,
    PCEDEVICE_POWER_STATE pDx   // IN, OUT
   )
{   
    CEDEVICE_POWER_STATE NewDx = *pDx;

    if ( VALID_DX(NewDx) ) 
    {
        // We only support D0, so do nothing.
        // Just return current state.
        pPWR->Dx = *pDx = D0;
        RETAILMSG(1, (TEXT("PCF: IOCTL_POWER_SET: D%u => D%u \r\n"), NewDx, pPWR->Dx));
        return TRUE;
    }

    return FALSE;
}


BOOL
HW_PowerGet(
    PPWR_CONTEXT pPWR,
    PCEDEVICE_POWER_STATE pDx
   )
{   
    // return our Current Dx value
    *pDx = pPWR->Dx;
    RETAILMSG(1, (TEXT("PCF: IOCTL_POWER_GET: D%u \r\n"), pPWR->Dx));

    return TRUE;
}


static DWORD 
PWR_IST(
    PPWR_CONTEXT pPWR
    )
{
    DWORD we;
	WCHAR  state[1024] = {0};
	LPWSTR pState = &state[0];
	DWORD dwBufChars = (sizeof(state) / sizeof(state[0]));
	DWORD  dwStateFlags = 0;
	DWORD dwErr;
    DWORD dwTimeout = 1000;
    
    if (!pPWR)
        return ERROR_INVALID_PARAMETER;

    while (1) {
        __try {
            we = WaitForSingleObject(pPWR->ISTEvent, INFINITE);
			RETAILMSG(1,(TEXT("\r\nPWR_IST: pPWR->State = 0x%x \r\n"), pPWR->State));

			InterruptDone(g_PwrButtonSysIntr);

			if (v_pIOPregs->GPFDAT & 0x1)
			{
				continue;
			}

            we = WaitForSingleObject(pPWR->ISTEvent, dwTimeout);

			dwErr = GetSystemPowerState(pState, dwBufChars, &dwStateFlags);
			if (ERROR_SUCCESS != dwErr) 
			{
				RETAILMSG(1, (TEXT("PMGET!GetSystemPowerState:ERROR:%d\n"), dwErr));
			} 
			else 
			{
				RETAILMSG(1, (TEXT("PMGET! System Power state is '%s', flags 0x%08x\n"), state, dwStateFlags));
			}

			RETAILMSG(1, (TEXT(">>>>>>>>>>>>>>>>>>> state == %s <<<<<<<<<<<<<<<<<<<\r\n"), state));
            
			if (we != WAIT_TIMEOUT)
			{
				InterruptDone(g_PwrButtonSysIntr);
			}

			if ((pPWR->State == RESUME) || (pPWR->State == RUN))
			{
				SetSystemPowerState( NULL, POWER_STATE_SUSPEND, POWER_FORCE );
			}
			else///APR if ( pPWR->State == 0x2 )
			{
				SetSystemPowerState( NULL, POWER_STATE_ON, POWER_FORCE );
				pPWR->State = RUN;
			}
        } _except(EXCEPTION_EXECUTE_HANDLER) {
            RETAILMSG(1,(TEXT("!!! PWR_IST EXCEPTION: 0x%X !!!\r\n"), GetExceptionCode() ));
        }
	}
}


/* ++

 Get/Set the PCF RTC.
 
 One neat PCF feature is the stand alone RTC.
 You could power the ARM core to full off and maintin the RTC & ALARM on the PCF.
 This is not a required feature for PPC2002 release, but OEMs are 
 free to add this as desired. You should sync the ARM & PCF RTC
 and update as appropriate. If you choose to implement this it would
 be good to power as much of the PCF off as possible to maintin it's RTC
 for much longer time.

-- */
BOOL 
PWR_GetRealTime(
    PPWR_CONTEXT pPWR,
    LPSYSTEMTIME lpst
    )
{
    return TRUE;
}


BOOL
PWR_SetRealTime(
    PPWR_CONTEXT pPWR,
    LPSYSTEMTIME lpst
    ) 
{
    return TRUE;
}


BOOL
HW_IOControl(
    PPWR_CONTEXT pPWR,
    DWORD dwCode,
    PBYTE pBufIn,
    DWORD dwLenIn,
    PBYTE pBufOut,
    DWORD dwLenOut,
    PDWORD pdwActualOut
   )
{
	return FALSE;
}


