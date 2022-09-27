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

#include <windows.h>
#include <halether.h>
#define __OAL_ETHDRV_H	// Temporary: clean up build warning until EDBG prototypes are moved.
#include <bsp.h>
#include "loader.h"

#define FROM_BCD(n)    ((((n) >> 4) * 10) + ((n) & 0xf))
#define TO_BCD(n)      ((((n) / 10) << 4) | ((n) % 10))

// Function pointers to the support library functions of the currently installed debug ethernet controller.
//
PFN_EDBG_INIT             pfnEDbgInit;
PFN_EDBG_ENABLE_INTS      pfnEDbgEnableInts;
PFN_EDBG_DISABLE_INTS     pfnEDbgDisableInts;
PFN_EDBG_GET_PENDING_INTS pfnEDbgGetPendingInts;
PFN_EDBG_GET_FRAME        pfnEDbgGetFrame;
PFN_EDBG_SEND_FRAME       pfnEDbgSendFrame;
PFN_EDBG_READ_EEPROM      pfnEDbgReadEEPROM;
PFN_EDBG_WRITE_EEPROM     pfnEDbgWriteEEPROM;
PFN_EDBG_SET_OPTIONS      pfnEDbgSetOptions;

// Function prototypes.
//
BOOL    DM9000DBG_Init(PBYTE iobase, DWORD membase, USHORT MacAddr[3]);
UINT16  DM9000DBG_GetFrame(PBYTE pbData, UINT16 *pwLength);
UINT16  DM9000DBG_SendFrame(PBYTE pbData, DWORD dwLength);

/*
    @func   BOOL | InitEthDevice | Initializes the Ethernet device to be used for download.
    @rdesc  TRUE = Success, FALSE = Failure.
    @comm    
    @xref   
*/
BOOL InitEthDevice(PBOOT_CFG pBootCfg)
{
    PBYTE  pBaseIOAddress = NULL;
    UINT32 MemoryBase = 0;  
    BOOL bResult = FALSE;

    OALMSG(OAL_FUNC, (TEXT("+InitEthDevice.\r\n")));

    // Use the MAC address programmed into flash by the user.
    //
    memcpy(pBSPArgs->kitl.mac, pBootCfg->EdbgAddr.wMAC, 6);

    // Use the DM9000 Ethernet controller for download.
    //
    pfnEDbgInit      = DM9000DBG_Init;
    pfnEDbgGetFrame  = DM9000DBG_GetFrame;
    pfnEDbgSendFrame = DM9000DBG_SendFrame;

    pBaseIOAddress   = (PBYTE)OALPAtoVA(pBSPArgs->kitl.devLoc.LogicalLoc, FALSE);
    MemoryBase       = (UINT32)OALPAtoVA(BSP_BASE_REG_PA_DM9000_MEMBASE, FALSE);
    
    // Initialize the Ethernet controller.
    //
    if (!pfnEDbgInit((PBYTE)pBaseIOAddress, MemoryBase, pBSPArgs->kitl.mac))
    {
        OALMSG(OAL_ERROR, (TEXT("ERROR: InitEthDevice: Failed to initialize Ethernet controller.\r\n")));
        goto CleanUp;
    }

    // Make sure MAC address has been programmed.
    //
    if (!pBSPArgs->kitl.mac[0] && !pBSPArgs->kitl.mac[1] && !pBSPArgs->kitl.mac[2])
    {
        OALMSG(OAL_ERROR, (TEXT("ERROR: InitEthDevice: Invalid MAC address.\r\n")));
        goto CleanUp;
    }

    bResult = TRUE;

CleanUp:

    OALMSG(OAL_FUNC, (TEXT("-InitEthDevice.\r\n")));
    return(bResult);
}


/*
    @func   BOOL | OEMGetRealTime | Returns the current wall-clock time from the RTC.
    @rdesc  TRUE = Success, FALSE = Failure.
    @comm    
    @xref   
*/
static BOOL OEMGetRealTime(LPSYSTEMTIME lpst) 
{
    volatile S3C2440A_RTC_REG *s2440RTC = (S3C2440A_RTC_REG *)OALPAtoVA(S3C2440A_BASE_REG_PA_RTC, FALSE);

    do
    {
        lpst->wYear         = FROM_BCD(s2440RTC->BCDYEAR) + 2000 ;
        lpst->wMonth        = FROM_BCD(s2440RTC->BCDMON   & 0x1f);
        lpst->wDay          = FROM_BCD(s2440RTC->BCDDAY   & 0x3f);

        lpst->wDayOfWeek    = (s2440RTC->BCDDATE - 1);

        lpst->wHour         = FROM_BCD(s2440RTC->BCDHOUR  & 0x3f);
        lpst->wMinute       = FROM_BCD(s2440RTC->BCDMIN   & 0x7f);
        lpst->wSecond       = FROM_BCD(s2440RTC->BCDSEC   & 0x7f);
        lpst->wMilliseconds = 0;
    }
    while (!(lpst->wSecond));

    return(TRUE);
}


/*
    @func   DWORD | OEMEthGetSecs | Returns a free-running seconds count.
    @rdesc  Number of elapsed seconds since last roll-over.
    @comm    
    @xref   
*/
DWORD OEMEthGetSecs(void)
{
    SYSTEMTIME sTime;

    OEMGetRealTime(&sTime);
    return((60UL * (60UL * (24UL * (31UL * sTime.wMonth + sTime.wDay) + sTime.wHour) + sTime.wMinute)) + sTime.wSecond);
}


/*
    @func   BOOL | OEMEthGetFrame | Reads data from the Ethernet device.
    @rdesc  TRUE = Success, FALSE = Failure.
    @comm    
    @xref   
*/
BOOL OEMEthGetFrame(PUCHAR pData, PUSHORT pwLength)
{
    return(pfnEDbgGetFrame(pData, pwLength));
}


/*
    @func   BOOL | OEMEthSendFrame | Writes data to an Ethernet device.
    @rdesc  TRUE = Success, FALSE = Failure.
    @comm    
    @xref   
*/
BOOL OEMEthSendFrame(PUCHAR pData, DWORD dwLength)
{
    BYTE Retries = 0;

    while (Retries++ < 4)
    {
        if (!pfnEDbgSendFrame(pData, dwLength))
            return(TRUE);

        KITLOutputDebugString("INFO: OEMEthSendFrame: retrying send (%u)\r\n", Retries);
    }

    return(FALSE);
}

