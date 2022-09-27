/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.
Copyright (c) 1998 Samsung Electronics.

Module Name:  

wince.c

Abstract:  

Windows CE specific functions for the CS8900 NDIS miniport driver.

Functions:
    DllEntry

Notes: 

--*/
#include <windows.h>
#include <ndis.h>
#include <cs8900hw.h>
#include <cs8900sw.h>

#ifdef DEBUG

//
// These defines must match the ZONE_* defines in CS8900SW.H
//
#define DBG_ERROR      1
#define DBG_WARN       2
#define DBG_FUNCTION   4
#define DBG_INIT       8
#define DBG_INTR       16
#define DBG_RCV        32
#define DBG_XMIT       64

DBGPARAM dpCurSettings = {
    TEXT("CS8900"), {
    TEXT("Errors"),TEXT("Warnings"),TEXT("Functions"),TEXT("Init"),
    TEXT("Interrupts"),TEXT("Receives"),TEXT("Transmits"),TEXT("Undefined"),
    TEXT("Undefined"),TEXT("Undefined"),TEXT("Undefined"),TEXT("Undefined"),
    TEXT("Undefined"),TEXT("Undefined"),TEXT("Undefined"),TEXT("Undefined") },
    0xFFFFFFFF
    };
#endif  // DEBUG

//
// Standard Windows DLL entrypoint.
// Since Windows CE NDIS miniports are implemented as DLLs, a DLL entrypoint is
// needed.
//
BOOL __stdcall
DllEntry(
  HANDLE hDLL,
  DWORD dwReason,
  LPVOID lpReserved
)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        DEBUGREGISTER(hDLL);
        DEBUGMSG(1, (TEXT("CS8900: DLL_PROCESS_ATTACH\r\n")));
        break;
    case DLL_PROCESS_DETACH:
        DEBUGMSG(1, (TEXT("CS8900: DLL_PROCESS_DETACH\r\n")));
        break;
    }
    return TRUE;
}

