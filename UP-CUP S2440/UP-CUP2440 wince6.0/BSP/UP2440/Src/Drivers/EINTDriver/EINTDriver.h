#ifndef __EINTDriver_H_
#define __EINTDriver_H_

#ifdef __cplusplus
extern "C" {
#endif

BOOL INT_Close(DWORD hOpenContext);
BOOL INT_Deinit(DWORD hDeviceContext);
DWORD INT_Init(DWORD dwContext);
DWORD INT_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode);
void INT_PowerUp(DWORD hDeviceContext);
void INT_PowerDown(DWORD hDeviceContext);
DWORD INT_Read(DWORD hOpenContext, LPVOID pBuffer, DWORD Count);
DWORD INT_Seek(DWORD hOpenContext, long Amount, DWORD Type);
DWORD INT_Write(DWORD hOpenContext, LPCVOID pSourceBytes, DWORD NumberOfBytes);

BOOL INT_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut);

#ifdef __cplusplus
}
#endif


#endif /* __EINTDriver_ */