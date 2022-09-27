#include "windows.h"
#include "tchar.h"
//#include "stdafx.h"

#define	BUFSIZE		256
WCHAR achBuffer[BUFSIZE];

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}


DWORD STR_Init(DWORD dwContext)
{
DWORD dwReturn = 0;
RETAILMSG (1,(TEXT("TRINGS:STR_Init\r\n")));
memset (achBuffer,0,BUFSIZE * sizeof (WCHAR));
dwReturn = 1;
return dwReturn;
}

///////////////////////////////////////////////////
BOOL STR_Deinit(DWORD hDeviceContext)
{
BOOL bReturn = TRUE;
RETAILMSG(1,(TEXT("MYSTRINGS:STR_Deinit\t\n")));
return bReturn;
}
///////////////////////////////////////////////////
DWORD STR_Open (DWORD hDeviceContext,
DWORD AccessCode,
DWORD ShareMode)
{
DWORD dwReturn = 0;
RETAILMSG (1,(TEXT("YSTRINGS:STR_Open\r\n")));
dwReturn = 1;
return dwReturn;
}

//////////////////////////////////////////////////
BOOL STR_Close(DWORD hOpenContext)
{
BOOL dwReturn = TRUE;
RETAILMSG(1,(TEXT("MYSTRINGS:STR_Close\r\n")));
return dwReturn;
}

/////////////////////////////////////////////////////////////
BOOL STR_IOControl(DWORD hOpenContext,
DWORD dwCode,
PBYTE pBufIn,
DWORD dwLenIn,
PBYTE pBufOut,
DWORD dwLenOut,
PDWORD pdwActualOut)
{
BOOL bReturn = TRUE;
RETAILMSG(1,(TEXT("MYSTRINGS:STR_IOControl\r\n")));
return bReturn;
}

////////////////////////////////////////////////////////////////////
void STR_PowerDown (DWORD hDeviceContext)
{
RETAILMSG(1,(TEXT("MYSTRINGS:STR_PowerDown\r\n")));
}



/////////////////////////////////////////////////////////
void STR_PowerUp(DWORD hDeviceContext)
{
RETAILMSG(1,(TEXT("MYSTRINGS:STR_PowerUp\r\n")));

}

/////////////////////////////////////////////////////////
DWORD STR_Read(DWORD hOpenContext,
LPVOID pBuffer,
DWORD Count)
{
DWORD dwReturn = 0;
RETAILMSG (1,(TEXT("MYSTRINGS:STR_Read\r\n")));

DWORD cbBuffer = wcslen ( achBuffer);
dwReturn = min(cbBuffer, Count);
wcsncpy((LPWSTR)pBuffer, achBuffer, dwReturn);

return dwReturn;

}

///////////////////////////////////////
DWORD STR_Seek(DWORD hOpenContext,
long Amount, 
DWORD Type)
{
DWORD dwReturn =0;
RETAILMSG(1,(TEXT("MYSTRINGS:STR_Seek\r\n")));
return dwReturn;
}
/////////////////////////////////////////
DWORD STR_Write(DWORD hOpenContext,
LPVOID pSourceBytes,
DWORD NumberOfBytes)
{
DWORD dwReturn =0;
RETAILMSG(1,(TEXT("MYSTRINGS:STR_Write\r\n")));
dwReturn = min(BUFSIZE, NumberOfBytes);
wcsncpy (achBuffer, (LPWSTR) pSourceBytes, dwReturn);

return dwReturn;
}