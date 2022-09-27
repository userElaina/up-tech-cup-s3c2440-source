#ifndef _SPI_H_
#define _SPI_H_

#include "generic.h"

#define SPI_CS 		0
#define SPI_SCK		1
#define SPI_MOSI	2
#define SPI_MOSO	3
#define SPI_S_CS 	4

void SPI_Init(void);

void SPIPut(BYTE v);

BYTE SPIGet(void);

void SPIGetArray(BYTE *buffer, BYTE len);

void SPIPutArray(BYTE *buffer, BYTE len);

#endif

