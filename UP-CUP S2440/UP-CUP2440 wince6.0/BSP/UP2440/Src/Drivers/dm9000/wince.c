/**************************************************************************
 * 
 * $Id: wince.c,v 1.1.1.1 2004/12/08 08:38:43 hychu Exp $
 *
 * File: Types.h
 *
 * Copyright (c) 2000-2002 Davicom Inc.  All rights reserved.
 *
 *************************************************************************/


#include <ndis.h>


BOOL
DllEntry(
    HANDLE    hInstDll,
    DWORD     dwOp,
    LPVOID    lpvReserved)
{
    switch (dwOp)
    {
        case DLL_PROCESS_ATTACH:
          DisableThreadLibraryCalls ((HMODULE)hInstDll);
          break;

        case DLL_PROCESS_DETACH:
        default:
            break;
    }

    return (TRUE);
}

