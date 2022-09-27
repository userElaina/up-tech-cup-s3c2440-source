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

//
// This module contains a stub implementation of the battery PDD.  OEMs
// that want to support the battery APIs on their platform can copy this
// file to their platform and link it into their own version of the power
// manager DLL.
//
// If the platform-specific power manager provides its own version of these
// entry points, this module will not be pulled into the link map from
// the pm_battapi library.
//
// To simplify testing the behavior of battery-sensitive applications on
// platforms (like CEPC or EMULATOR) that don't include physical batteries,
// this driver uses a memory-mapped file to store power information.  This
// file is shared with the SETSTUBBAT test program in 
// public\common\oak\drivers\battdrvr\test\setstubbat, which can update 
// arbitrary battery values.  Note that memory-mapped files require that
// the OS image include the "nkmapfile" component.
//

#include <battimpl.h>
#include <devload.h>
#include "battery.h"

// typedefs for APIs that require the "nkmapfile" component
typedef WINBASEAPI HANDLE (WINAPI *PFN_CreateFileMappingW) (
    HANDLE hFile,
    LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
    DWORD flProtect,
    DWORD dwMaximumSizeHigh,
    DWORD dwMaximumSizeLow,
    LPCWSTR lpName
    );
typedef LPVOID (WINAPI *PFN_MapViewOfFile) (
    HANDLE hFileMappingObject,
    DWORD dwDesiredAccess,
    DWORD dwFileOffsetHigh,
    DWORD dwFileOffsetLow,
    DWORD dwNumberOfBytesToMap
    );
typedef BOOL (WINAPI *PFN_UnmapViewOfFile) (
    LPCVOID lpBaseAddress
    );


typedef BOOL (WINAPI * PFN_GwesPowerDown)(void);

#define MUTEX_TIMEOUT 5000

static PBATTERY_STATUS gpStatus;
static HANDLE ghMutex;
static HANDLE ghFileMap;
static PFN_CreateFileMappingW gpfnCreateFileMappingW;
static PFN_MapViewOfFile gpfnMapViewOfFile;
static PFN_UnmapViewOfFile gpfnUnmapViewOfFile;

// this routine takes the battery mutex
DWORD
LockBattery(void)
{
    DWORD dwStatus;
    SETFNAME(_T("LockBattery"));

    DEBUGCHK(ghMutex != NULL);

    dwStatus = WaitForSingleObject(ghMutex, MUTEX_TIMEOUT);
    if(dwStatus == WAIT_OBJECT_0) {
        dwStatus = ERROR_SUCCESS;
    } else {
        dwStatus = GetLastError();
        DEBUGCHK(dwStatus != ERROR_SUCCESS);
    }
    
    DEBUGMSG(dwStatus != ERROR_SUCCESS && ZONE_WARN,
        (_T("%s: WaitForSingleObject() failed %d\r\n"), pszFname, 
        GetLastError()));
    DEBUGCHK(dwStatus == ERROR_SUCCESS);

    return dwStatus;
}

// this routine releases the battery mutex
DWORD
UnlockBattery(void)
{
    DWORD dwStatus = ERROR_SUCCESS;
    BOOL fOk;
    SETFNAME(_T("UnlockBattery"));

    DEBUGCHK(ghMutex != NULL);

    fOk = ReleaseMutex(ghMutex);
    if(!fOk) {
        dwStatus = GetLastError();
        DEBUGCHK(dwStatus != ERROR_SUCCESS);
    }
    
    DEBUGMSG(dwStatus != ERROR_SUCCESS && ZONE_WARN,
        (_T("%s: ReleaseMutex() failed %d\r\n"), pszFname, GetLastError()));
    DEBUGCHK(dwStatus == ERROR_SUCCESS);

    return dwStatus;
}

BOOL WINAPI 
BatteryPDDInitialize(LPCTSTR pszRegistryContext)
{
    BOOL fOk = TRUE;
    HKEY hk;
    SYSTEM_POWER_STATUS_EX2 sps;
    WORD wMainLevels = 3, wBackupLevels = 3;
    BOOL fSupportsChange = FALSE;
    SETFNAME(_T("BatteryPDDInitialize"));
    
    DEBUGCHK(ghMutex == NULL);
    DEBUGCHK(ghFileMap == NULL);
    DEBUGCHK(gpStatus == NULL);
    DEBUGCHK(pszRegistryContext != NULL);
	
    // intialize the battery status structure -- assume AC power, no battery info
    sps.ACLineStatus               = AC_LINE_ONLINE;
    sps.BatteryFlag                = BATTERY_FLAG_HIGH;
    sps.BatteryLifePercent         = BATTERY_PERCENTAGE_UNKNOWN;
    sps.Reserved1                  = 0;
    sps.BatteryLifeTime            = BATTERY_LIFE_UNKNOWN;
    sps.BatteryFullLifeTime        = BATTERY_LIFE_UNKNOWN;
    sps.Reserved2                  = 0;
    sps.BackupBatteryFlag          = BATTERY_FLAG_HIGH;
    sps.BackupBatteryLifePercent   = BATTERY_PERCENTAGE_UNKNOWN;
    sps.Reserved3                  = 0;
    sps.BackupBatteryLifeTime      = BATTERY_LIFE_UNKNOWN;
    sps.BackupBatteryFullLifeTime  = BATTERY_LIFE_UNKNOWN;
    sps.BatteryChemistry           = BATTERY_CHEMISTRY_UNKNOWN;
    sps.BatteryVoltage             = 0;
    sps.BatteryCurrent             = 0;
    sps.BatteryAverageCurrent      = 0;
    sps.BatteryAverageInterval     = 0;
    sps.BatterymAHourConsumed      = 0;
    sps.BatteryTemperature         = 0;
    sps.BackupBatteryVoltage       = 0;
	
    // get registry values, if present
    hk = OpenDeviceKey(pszRegistryContext);
    if(hk != NULL) {
        DWORD dwSize, dwStatus, dwType, dwValue;
        SYSTEM_POWER_STATUS_EX2 spstemp;
		
        // get the number of main levels
        dwSize = sizeof(dwValue);
        dwStatus = RegQueryValueEx(hk, _T("MainLevels"), NULL, &dwType, (LPBYTE) &dwValue, &dwSize);
        if(dwStatus == ERROR_SUCCESS && dwType == REG_DWORD) {
            wMainLevels = (WORD) dwValue;
        }
		
        // get the number of backup levels
        dwSize = sizeof(dwValue);
        dwStatus = RegQueryValueEx(hk, _T("BackupLevels"), NULL, &dwType, (LPBYTE) &dwValue, &dwSize);
        if(dwStatus == ERROR_SUCCESS && dwType == REG_DWORD) {
            wBackupLevels = (WORD) dwValue;
        }
		
        // does the battery support change?
        dwSize = sizeof(dwValue);
        dwStatus = RegQueryValueEx(hk, _T("SupportsChange"), NULL, &dwType, (LPBYTE) &dwValue, &dwSize);
        if(dwStatus == ERROR_SUCCESS && dwType == REG_DWORD) {
            fSupportsChange = dwValue != 0 ? TRUE : FALSE;
        }
		
        // do we have an initial battery status?
        dwSize = sizeof(spstemp);
        dwStatus = RegQueryValueEx(hk, _T("InitialStatus"), NULL, &dwType, (LPBYTE) &spstemp, &dwSize);
        if(dwStatus == ERROR_SUCCESS && dwType == REG_BINARY && dwSize == sizeof(spstemp)) {
            memcpy(&sps, &spstemp, dwSize);
        }
		
        // close the key
        RegCloseKey(hk);
    }
	
    // allocate resources
    if((ghMutex = CreateMutex(NULL, FALSE, BATTERY_FILE_MUTEX)) == NULL) {
        DEBUGMSG(ZONE_ERROR || ZONE_PDD || ZONE_INIT,
            (_T("%s: Could not aquire battery info file mutex handle\n"), pszFname));
        fOk = FALSE;
    } else {
        HINSTANCE hiCoreDll = NULL; 
        BOOL fNewMapping = TRUE;
        
        // get pointers to file-mapping functions
        hiCoreDll = LoadLibrary(_T("coredll.dll"));
        if(hiCoreDll != NULL) {
            gpfnCreateFileMappingW = (PFN_CreateFileMappingW) GetProcAddress((HMODULE) hiCoreDll, _T("CreateFileMappingW"));
            gpfnMapViewOfFile = (PFN_MapViewOfFile) GetProcAddress((HMODULE) hiCoreDll, _T("MapViewOfFile"));
            gpfnUnmapViewOfFile = (PFN_UnmapViewOfFile) GetProcAddress((HMODULE) hiCoreDll, _T("UnmapViewOfFile"));
        }
        FreeLibrary(hiCoreDll);		// we're already linked to coredll
        
        // serialize access to the mapping file
        LockBattery();
        
        // create the mapping
        if(gpfnCreateFileMappingW == NULL ) {
            // no file mapping, use a global variable
            static BATTERY_STATUS sBatteryStatus;
            gpStatus = &sBatteryStatus;
        } else if((ghFileMap = gpfnCreateFileMappingW((HANDLE)INVALID_HANDLE_VALUE, NULL, 
            PAGE_READWRITE, 0, sizeof(BATTERY_STATUS), BATTERY_STATUS_FILE)) == NULL) {
            DEBUGMSG(ZONE_ERROR || ZONE_PDD || ZONE_INIT,
                (_T("%s: Could not create file mapping for battery info file\n"), pszFname));
            fOk = FALSE;
        } else {
            // is this a new mapping?
            if(GetLastError() == ERROR_ALREADY_EXISTS) {
                fNewMapping = FALSE;
            }
            
            // map the object into our address space
            if(gpfnMapViewOfFile == NULL 
            || (gpStatus = (PBATTERY_STATUS) gpfnMapViewOfFile(ghFileMap, FILE_MAP_ALL_ACCESS, 
                0, 0, sizeof(BATTERY_STATUS))) == NULL) {
                DEBUGMSG(ZONE_ERROR || ZONE_PDD || ZONE_INIT,
                    (_T("Could not map view of battery info file into process address space\n"), pszFname));
                fOk = FALSE;
            } 
        }
        
        // should we initialize our structure?
        if(fOk && fNewMapping) {
            // initialize the memory mapped object
            memcpy(&gpStatus->sps, &sps, sizeof(gpStatus->sps));
            gpStatus->fSupportsChange = fSupportsChange;
            gpStatus->fChanged = FALSE;
            gpStatus->wMainLevels = wMainLevels;
            gpStatus->wBackupLevels = wBackupLevels;
        }
        
        // allow access to the battery buffer
        UnlockBattery();
    }
    
    // clean up if necessary
    if(!fOk) {
        if(gpStatus != NULL && gpfnUnmapViewOfFile != NULL) gpfnUnmapViewOfFile(gpStatus);
        if(ghFileMap != NULL) CloseHandle(ghFileMap);
        if(ghMutex != NULL) CloseHandle(ghMutex);
        gpStatus = NULL;
        ghFileMap = NULL;
        ghMutex = NULL;
    }
    
    DEBUGMSG(ZONE_PDD, (_T("%s: returning %d\r\n"), pszFname, fOk));
    
    return fOk;
}

void WINAPI 
BatteryPDDDeinitialize(void)
{
    SETFNAME(_T("BatteryPDDDeinitialize"));

    DEBUGMSG(ZONE_PDD, (_T("%s: invoked\r\n"), pszFname));
    if(gpStatus != NULL && gpfnUnmapViewOfFile != NULL) gpfnUnmapViewOfFile(gpStatus);
    if(ghFileMap != NULL) CloseHandle(ghFileMap);
    if(ghMutex != NULL) CloseHandle(ghMutex);
    gpStatus = NULL;
    ghFileMap = NULL;
    ghMutex = NULL;
}

void WINAPI 
BatteryPDDResume(void)
{
    SETFNAME(_T("BatteryPDDResume"));

    DEBUGMSG(ZONE_PDD, (_T("%s: invoked\r\n"), pszFname));
}

void WINAPI 
BatteryPDDPowerHandler(BOOL bOff)
{
    SETFNAME(_T("BatteryPDDPowerHandler"));

    UNREFERENCED_PARAMETER(bOff);
    
    DEBUGMSG(ZONE_PDD | ZONE_RESUME, (_T("%s: invoked w/ bOff %d\r\n"), pszFname, bOff));
}

// This routine obtains the most current battery/power status available
// on the platform.  It fills in the structures pointed to by its parameters
// and returns TRUE if successful.  If there's an error, it returns FALSE.
BOOL WINAPI
BatteryPDDGetStatus(
                    PSYSTEM_POWER_STATUS_EX2 pstatus,
                    PBOOL pfBatteriesChangedSinceLastCall
                    )
{
    BOOL fOk = TRUE;
    SETFNAME(_T("BatteryPDDGetStatus"));
    
    DEBUGMSG(ZONE_PDD, (_T("%s: invoked w/ pstatus 0x%08x, pfChange 0x%08x\r\n"),
        pszFname, pstatus, pfBatteriesChangedSinceLastCall));
    
    PREFAST_DEBUGCHK(gpStatus != NULL);
    PREFAST_DEBUGCHK(pstatus != NULL);
    PREFAST_DEBUGCHK(pfBatteriesChangedSinceLastCall != NULL);
    
    // get battery status information from the shared structure
    LockBattery();
    memcpy(pstatus, &gpStatus->sps, sizeof(*pstatus));
    *pfBatteriesChangedSinceLastCall = gpStatus->fChanged;
    if(*pfBatteriesChangedSinceLastCall) {
        gpStatus->fChanged = FALSE;		// reset changed flag if it's set
    }
    UnlockBattery();
    
    DEBUGMSG(ZONE_PDD, (_T("%s: returning %d\r\n"), pszFname, fOk));
    return (fOk);
}


// This routine indicates how many battery levels will be reported
// in the BatteryFlag and BackupBatteryFlag fields of the PSYSTEM_POWER_STATUS_EX2
// filed in by BatteryPDDGetStatus().  This number ranges from 0 through 3 --
// see the Platform Builder documentation for details.  The main battery
// level count is reported in the low word of the return value; the count 
// for the backup battery is in the high word.
LONG
BatteryPDDGetLevels(
    void
   	)
{
    LONG lLevels;
    SETFNAME(_T("BatteryPDDPowerHandler"));

    PREFAST_DEBUGCHK(gpStatus != NULL);

    LockBattery();
    lLevels = MAKELONG (gpStatus->wMainLevels, gpStatus->wBackupLevels);
    UnlockBattery();

    DEBUGMSG(ZONE_PDD, (_T("%s: returning %u (%d main levels, %d backup levels)\r\n"),
        pszFname, lLevels, LOWORD(lLevels), HIWORD(lLevels)));

    return lLevels;
}



// This routine returns TRUE to indicate that the pfBatteriesChangedSinceLastCall
// value filled in by BatteryPDDGetStatus() is valid.  If there is no way to
// tell that the platform's batteries have been changed this routine should
// return FALSE.
BOOL
BatteryPDDSupportsChangeNotification(
    void
   	)
{
    BOOL fSupportsChange;
    SETFNAME(_T("BatteryPDDPowerHandler"));

    PREFAST_DEBUGCHK(gpStatus != NULL);

    LockBattery();
    fSupportsChange = gpStatus->fSupportsChange;
    UnlockBattery();

    DEBUGMSG(ZONE_PDD, (_T("%s: returning %d\r\n"), pszFname, fSupportsChange));

    return fSupportsChange;
}

