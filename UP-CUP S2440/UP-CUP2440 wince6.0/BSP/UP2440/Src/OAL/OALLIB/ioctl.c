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
//------------------------------------------------------------------------------
//
//  File: ioctl.c
//
//  This file implements the OEM's IO Control (IOCTL) functions and declares
//  global variables used by the IOCTL component.
//
#include <partdrv.h>
#include <bsp.h>

//------------------------------------------------------------------------------
//
//  Global: g_oalIoctlPlatformType/OEM
//
//  Platform Type/OEM
//
LPCWSTR g_oalIoCtlPlatformType = IOCTL_PLATFORM_TYPE;
LPCWSTR g_oalIoCtlPlatformOEM  = IOCTL_PLATFORM_OEM;

//------------------------------------------------------------------------------
//
//  Global: g_oalIoctlProcessorVendor/Name/Core
//
//  Processor information
//
LPCWSTR g_oalIoCtlProcessorVendor = IOCTL_PROCESSOR_VENDOR;
LPCWSTR g_oalIoCtlProcessorName   = IOCTL_PROCESSOR_NAME;
LPCWSTR g_oalIoCtlProcessorCore   = IOCTL_PROCESSOR_CORE;

//------------------------------------------------------------------------------
//
//  Global: g_oalIoctlInstructionSet
//
//  Processor instruction set identifier
//
UINT32 g_oalIoCtlInstructionSet = IOCTL_PROCESSOR_INSTRUCTION_SET;
extern UINT32 g_oalIoCtlClockSpeed;


//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalGetHiveCleanFlag
//
//  This function is used by Filesys.exe to query the OEM to determine if the registry hives
//  and user profiles should be deleted and recreated.
//
//  Notes: During a OS start up, Filesys.exe calls HIVECLEANFLAG_SYSTEM twice and followed by
//  HIVECLEANFLAG_USER. We'll clear the shared Args flag after that.
//
static BOOL OALIoCtlHalGetHiveCleanFlag(
	UINT32 code, VOID *lpInBuf, UINT32 nInBufSize, VOID *lpOutBuf,
	UINT32 nOutBufSize, UINT32 *pOutSize)
{
	BOOL bRet = FALSE;

	OALMSG(OAL_FUNC, (TEXT("OALIoCtlHalGetHiveCleanFlag\r\n")));

	if (!lpInBuf || (nInBufSize != sizeof(DWORD))
		|| !lpOutBuf || (nOutBufSize != sizeof(BOOL)))
	{
		return FALSE;
	}
	else
	{
		DWORD *pdwFlags = (DWORD*)lpInBuf;
		BOOL  *pfClean  = (BOOL*)lpOutBuf;

		// This is the global shared Args flag
		BOOL *bHiveCleanFlag = (BOOL*) OALArgsQuery(BSP_ARGS_QUERY_HIVECLEAN);

		*pfClean = *bHiveCleanFlag;
		bRet = *bHiveCleanFlag;

		if (*pdwFlags == HIVECLEANFLAG_SYSTEM)
		{
			if(bRet)
			{
				OALMSG(0, (TEXT("OEM: Cleaning system hive\r\n")));
			}
			else
			{
				OALMSG(0, (TEXT("OEM: Not cleaning system hive\r\n")));
			}
	  	}
	  	else if (*pdwFlags == HIVECLEANFLAG_USERS)
	  	{
			if(bRet)
			{
				OALMSG(0, (TEXT("OEM: Cleaning user profiles\r\n")));
			}
			else
			{
				OALMSG(0, (TEXT("OEM: Not cleaning user profiles\r\n")));
			}

			// We are done checking HiveCleanFlag by now (system hive is checked before user hive).
			// Now is the time to clear the global shared Args flag if it is set by switch or software.
			*bHiveCleanFlag = FALSE;
	  	}
	  	else
	  	{
			OALMSG(OAL_ERROR, (TEXT("OEM: Unknown flag 0x%x\r\n"), *pdwFlags));
	  	}
	}

	return(bRet);
}

//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalQueryFormatPartition
//
//  This function is called by Filesys.exe to allow an OEM to specify whether a specific
//  partition is to be formatted on mount. Before Filesys.exe calls this IOCTL, it checks
//  the CheckForFormat registry value in the storage profile for your block driver.
//
static BOOL OALIoCtlHalQueryFormatPartition(
	UINT32 code, VOID *lpInBuf, UINT32 nInBufSize, VOID *lpOutBuf,
	UINT32 nOutBufSize, UINT32 *pOutSize)
{
	OALMSG(OAL_FUNC, (TEXT("OALIoCtlHalFormatPartition\r\n")));

	if (!lpInBuf || (nInBufSize != sizeof(STORAGECONTEXT))
		|| !lpOutBuf || (nOutBufSize < sizeof(BOOL)))
	{
		return FALSE;
	}
	else
	{
		STORAGECONTEXT *pStore = (STORAGECONTEXT *)lpInBuf;
		BOOL  *pfClean = (BOOL*)lpOutBuf;

		// This is the global shared Args flag
		BOOL *bFormatPartFlag = (BOOL*) OALArgsQuery(BSP_ARGS_QUERY_FORMATPART);

		OALMSG(OAL_VERBOSE, (TEXT("Store partition info:\r\n")));
		OALMSG(OAL_VERBOSE, (TEXT("\tszPartitionName=%s\r\n"), pStore->PartInfo.szPartitionName));
		OALMSG(OAL_VERBOSE, (TEXT("\tsnNumSectors=%d\r\n"), pStore->PartInfo.snNumSectors));
		OALMSG(OAL_VERBOSE, (TEXT("\tdwAttributes=0x%x\r\n"), pStore->PartInfo.dwAttributes));
		OALMSG(OAL_VERBOSE, (TEXT("\tbPartType=0x%x\r\n"), pStore->PartInfo.bPartType));

		// Set return value
		*pfClean = *bFormatPartFlag;

		// Clear the flag so that we don't do it again in next boot unless it is set again.
		*bFormatPartFlag = FALSE;

		if(*pfClean)
		{
			if(pStore->dwFlags & AFS_FLAG_BOOTABLE) 
			{
				OALMSG(0, (TEXT("OEM: Clearing storage registry hive\r\n")));
			}
			else
			{
				OALMSG(0, (TEXT("OEM: Clearing storage\r\n")));
			}
	  	}
	  	else
	  	{
			OALMSG(0, (TEXT("OEM: Not clearing storage\r\n")));
	  	}
	}

	if(pOutSize)
	{
		*pOutSize = sizeof(UINT32);
	}

	return(TRUE);
}

//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalInitRTC
//
//  This function is called by WinCE OS to initialize the time after boot.
//  Input buffer contains SYSTEMTIME structure with default time value.
//  If hardware has persistent real time clock it will ignore this value
//  (or all call).
//
BOOL OALIoCtlHalInitRTC(
	UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer,
	UINT32 outSize, UINT32 *pOutSize
) {
	BOOL rc = FALSE;
	SYSTEMTIME *pTime = (SYSTEMTIME*)pInpBuffer;

	OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHalInitRTC(...)\r\n"));

	// Validate inputs
	if (pInpBuffer == NULL || inpSize < sizeof(SYSTEMTIME)) {
		NKSetLastError(ERROR_INVALID_PARAMETER);
		OALMSG(OAL_ERROR, (
			L"ERROR: OALIoCtlHalInitRTC: Invalid parameter\r\n"
		));
		goto cleanUp;
	}

	// Add static mapping for RTC alarm
	OALIntrStaticTranslate(SYSINTR_RTC_ALARM, IRQ_RTC);

	// Set time
//	rc = OEMSetRealTime(pTime);
	rc = TRUE;
	
cleanUp:
	OALMSG(OAL_IOCTL&&OAL_FUNC, (L"-OALIoCtlHalInitRTC(rc = %d)\r\n", rc));
	return rc;
}

//------------------------------------------------------------------------------
//
//  Global: g_oalIoCtlTable[]
//
//  IOCTL handler table. This table includes the IOCTL code/handler pairs  
//  defined in the IOCTL configuration file. This global array is exported 
//  via oal_ioctl.h and is used by the OAL IOCTL component.
//
const OAL_IOCTL_HANDLER g_oalIoCtlTable[] = {
#include "ioctl_tab.h"
};

//------------------------------------------------------------------------------

