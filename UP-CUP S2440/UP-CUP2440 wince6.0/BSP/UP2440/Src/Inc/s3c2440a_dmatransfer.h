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
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:	DMA.H

Abstract:		Definitions for the DMA controller found on the Samsung 2440 CPU.
  
Notes:			

Environment:	
-----------------------------------------------------------------------------*/


#include <windows.h>

//===================== Register Configuration Constants ======================

//----- Register definitions for DISRCn control register -----
//
// bits[30-0] = start address of source data to transfer


//----- Register definitions for DISRCCn control register -----
//
#define SOURCE_PERIPHERAL_BUS				0x00000002
#define FIXED_SOURCE_ADDRESS				0x00000001


//----- Register definitions for DIDSTn control register -----
//
// bits[30-0] = start address of destination for the transfer


//----- Register definitions for DIDSTCn control register -----
//
#define DESTINATION_PERIPHERAL_BUS			0x00000002
#define FIXED_DESTINATION_ADDRESS			0x00000001


//----- Register definitions for DCONn control register -----
//
#define HANDSHAKE_MODE						0x80000000
#define DREQ_DACK_SYNC						0x40000000
#define GENERATE_INTERRUPT					0x20000000
#define SELECT_BURST_TRANSFER				0x10000000
#define SELECT_WHOLE_SERVICE_MODE			0x08000000

// bits[26-24] = select DMA source for the respective channel:
//------------------------------------------------------------
#define XDREQ0_DMA0							0x00000000
#define UART0_DMA0							0x01000000
#define MMC_DMA0							0x02000000
#define TIMER_DMA0							0x03000000
#define USB_EP1_DMA0						0x04000000

#define XDREQ1_DMA1							0x00000000
#define UART1_DMA1							0x01000000
#define I2SSDI_DMA1							0x02000000
#define SPI_DMA1							0x03000000
#define USB_EP2_DMA1						0x04000000

#define I2SSDO_DMA2							0x00000000
#define I2SSDI_DMA2							0x01000000
#define MMC_DMA2							0x02000000
#define TIMER_DMA2							0x03000000
#define USB_EP3_DMA2						0x04000000

#define UART2_DMA3							0x00000000
#define MMC_DMA3							0x01000000
#define SPI_DMA3							0x02000000
#define TIMER_DMA3							0x03000000
#define USB_EP4_DMA3						0x04000000
//------------------------------------------------------------

#define DMA_TRIGGERED_BY_HARDWARE			0x00800000
#define NO_DMA_AUTO_RELOAD					0x00400000

// bits[21-20] = select transfer word size
//------------------------------------------------------------
#define TRANSFER_BYTE						0x00000000				// 8  bits
#define TRANSFER_HALF_WORD					0x00100000				// 16 bits
#define TRANSFER_WORD						0x00200000				// 32 bits
//
// bits[19-0] = used to specify the number of transfer operations in a DMA request



//----- Register definitions for DSTATn status register -----
#define DMASTAT_TRANSFER_IN_PROGRESS			0x00100000
//
// bits[19-0] = the current transfer count


//----- Register definitions for DCSRCn configuration register -----
//
// bits[30-0] = current source address for DMA channel n


//----- Register definitions for DCDSTn configuration register -----
//
// bits[30-0] = current destination address for DMA channel n


//----- Register definitions for DMASKTRIGn configuration register -----
#define STOP_DMA_TRANSFER					0x00000004
#define ENABLE_DMA_CHANNEL					0x00000002
#define DMA_SW_TRIGGER						0x00000001



//=============================================================================

