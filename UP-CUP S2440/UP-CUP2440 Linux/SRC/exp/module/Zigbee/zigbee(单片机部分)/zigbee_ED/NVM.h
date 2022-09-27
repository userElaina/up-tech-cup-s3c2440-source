#ifndef _ZNVM_H_
#define _ZNVM_H_

#include "zigbee.h"

void NVMWrite(void *dest, const void *src, size_t count);

void NVMRead(void *dest, const void *src, size_t count);

void ClearNVM( void *dest, BYTE count );

#define GetMACAddress( x )          NVMRead( (void *)x, &macLongAddr, sizeof(LONG_ADDR) )
#define PutMACAddress( x )          NVMWrite((void *)&macLongAddr, (BYTE*)x, sizeof(LONG_ADDR))
#define GetMACShortAddress(x)		NVMRead( (void *)x, &macShortAddr, sizeof(SHORT_ADDR) )
#define PutMACShortAddress(x)		NVMWrite((void *)&macShortAddr, (BYTE*)x, sizeof(SHORT_ADDR))

extern EEMEM LONG_ADDR                macLongAddr;
extern EEMEM SHORT_ADDR               macShortAddr;

#endif

