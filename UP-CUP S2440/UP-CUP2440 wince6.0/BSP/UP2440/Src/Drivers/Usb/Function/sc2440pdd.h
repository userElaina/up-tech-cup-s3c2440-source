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
/*++

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name: 

        SC4240PDD.H

Abstract:

       Samsung S3SC2440 USB Function Platform-Dependent Driver header.

--*/

#ifndef _SC2440PDD_H_
#define _SC2440PDD_H_

#include <windows.h>
#include <ceddk.h>
#include <usbfntypes.h>
#include <usbfn.h>
#include <devload.h>

#define RegOpenKey(hkey, lpsz, phk) \
        RegOpenKeyEx((hkey), (lpsz), 0, 0, (phk))

#ifndef SHIP_BUILD
#define STR_MODULE _T("SC2440UsbFn!")
#define SETFNAME() LPCTSTR pszFname = STR_MODULE _T(__FUNCTION__) _T(":")
#else
#define SETFNAME()
#endif

#define SET_ADDRESS_REG_OFFSET      0x0
#define PWR_REG_OFFSET              0x4
#define EP_INT_REG_OFFSET           0x8
#define USB_INT_REG_OFFSET          0x18
#define EP_INT_EN_REG_OFFSET        0x1C
#define USB_INT_EN_REG_OFFSET       0x2C

#define EP0_FIFO_REG_OFFSET        0x80
#define EP1_FIFO_REG_OFFSET        0x84
#define EP2_FIFO_REG_OFFSET        0x88
#define EP3_FIFO_REG_OFFSET        0x8C
#define EP4_FIFO_REG_OFFSET        0x90
 
#define IDXADDR_REG_OFFSET          0x38
// Indexed Registers
#define MAX_PKT_SIZE_REG_OFFSET     0x40
#define EP0_CSR_REG_OFFSET          0x44
#define IN_CSR1_REG_OFFSET          0x44
#define IN_CSR2_REG_OFFSET          0x48
#define OUT_CSR1_REG_OFFSET         0x50
#define OUT_CSR2_REG_OFFSET         0x54
#define OUT_FIFO_CNT1_REG_OFFSET    0x58
#define OUT_FIFO_CNT2_REG_OFFSET    0x5C

// DMA interface control register
#define EP1_DMA_CON_OFFSET			0xC0
#define EP2_DMA_CON_OFFSET			0xD8
#define EP3_DMA_CON_OFFSET			0x100
#define EP4_DMA_CON_OFFSET			0x118

// DMA unit counter register
#define EP1_DMA_UNIT_OFFSET			0xC4
#define EP2_DMA_UNIT_OFFSET			0xDC
#define EP3_DMA_UNIT_OFFSET			0x104
#define EP4_DMA_UNIT_OFFSET			0x11C

// DMA fifo counter register
#define EP1_DMA_FIFO_OFFSET			0xC8
#define EP2_DMA_FIFO_OFFSET			0xE0
#define EP3_DMA_FIFO_OFFSET			0x108
#define EP4_DMA_FIFO_OFFSET			0x120

// DMA total transfer counter register
#define EP1_DMA_TTC_L_OFFSET		0xCC
#define EP1_DMA_TTC_M_OFFSET		0xD0
#define EP1_DMA_TTC_H_OFFSET		0xD4
#define EP2_DMA_TTC_L_OFFSET		0xE4
#define EP2_DMA_TTC_M_OFFSET		0xE8
#define EP2_DMA_TTC_H_OFFSET		0xEC
#define EP3_DMA_TTC_L_OFFSET		0x10C
#define EP3_DMA_TTC_M_OFFSET		0x110
#define EP3_DMA_TTC_H_OFFSET		0x114
#define EP4_DMA_TTC_L_OFFSET		0x124
#define EP4_DMA_TTC_M_OFFSET		0x128
#define EP4_DMA_TTC_H_OFFSET		0x12C

//
#define BASE_REGISTER_OFFSET        0x140
#define REGISTER_SET_SIZE           0x200

// Power Reg Bits
#define USB_RESET                   0x8
#define MCU_RESUME                  0x4
#define SUSPEND_MODE                0x2
#define SUSPEND_MODE_ENABLE_CTRL    0x1

// EP0 CSR
#define EP0_OUT_PACKET_RDY          0x1
#define EP0_IN_PACKET_RDY           0x2
#define EP0_SENT_STALL              0x4
#define DATA_END                    0x8
#define SETUP_END                   0x10
#define EP0_SEND_STALL              0x20
#define SERVICED_OUT_PKY_RDY        0x40
#define SERVICED_SETUP_END          0x80

// IN_CSR1_REG Bit definitions
#define IN_PACKET_READY             0x1
#define UNDER_RUN                   0x4   // Iso Mode Only
#define FLUSH_IN_FIFO               0x8
#define IN_SEND_STALL               0x10
#define IN_SENT_STALL               0x20
#define IN_CLR_DATA_TOGGLE          0x40

// OUT_CSR1_REG Bit definitions
#define OUT_PACKET_READY            0x1
#define FLUSH_OUT_FIFO              0x10
#define OUT_SEND_STALL              0x20
#define OUT_SENT_STALL              0x40
#define OUT_CLR_DATA_TOGGLE         0x80

// IN_CSR2_REG Bit definitions
#define IN_DMA_INT_DISABLE          0x10
#define SET_MODE_IN                 0x20 
#define SET_TYPE_ISO                0x40  // Note that Samsung does not currently support ISOCH 
#define AUTO_MODE                   0x80

// OUT_CSR2_REG Bit definitions
#define AUTO_CLR					0x40
#define OUT_DMA_INT_DISABLE         0x20

// Can be used for Interrupt and Interrupt Enable Reg - common bit def
#define EP0_INT_INTR                0x1
#define EP1_INT_INTR                0x2
#define EP2_INT_INTR                0x4
#define EP3_INT_INTR                0x8
#define EP4_INT_INTR                0x10

#define CLEAR_ALL_EP_INTRS          (EP0_INT_INTR | EP1_INT_INTR | EP2_INT_INTR | EP3_INT_INTR | EP4_INT_INTR)

#define  EP_INTERRUPT_DISABLE_ALL   0x0   // Bits to write to EP_INT_EN_REG - Use CLEAR

// Bit Definitions for USB_INT_REG and USB_INT_EN_REG_OFFSET
#define USB_RESET_INTR              0x4
#define USB_RESUME_INTR             0x2
#define USB_SUSPEND_INTR            0x1

// DMA control register bit definitions
#define RUN_OB						0x80
#define STATE						0x70
#define DEMAND_MODE					0x8
#define OUT_DMA_RUN					0x4
#define IN_DMA_RUN					0x2
#define DMA_MODE_EN					0x1

//
#define REAL_PHYSICAL_ADDR_EP0_FIFO		(0x520001c0) //Endpoint 0 FIFO
#define REAL_PHYSICAL_ADDR_EP1_FIFO		(0x520001c4) //Endpoint 1 FIFO
#define REAL_PHYSICAL_ADDR_EP2_FIFO		(0x520001c8) //Endpoint 2 FIFO
#define REAL_PHYSICAL_ADDR_EP3_FIFO		(0x520001cc) //Endpoint 3 FIFO
#define REAL_PHYSICAL_ADDR_EP4_FIFO		(0x520001d0) //Endpoint 4 FIFO

#define DMA_BUFFER_BASE					0xAC000000
#define DMA_PHYSICAL_BASE				0x30000000
#define DRIVER_GLOBALS_PHYSICAL_MEMORY_START  (DMA_BUFFER_BASE + 0x10000)

// We poll for device detach at the following rate.
#define S3C2440_USB_POLL_RATE 1000

// For USB DMA
BOOL InitUsbdDriverGlobals(void);  	//:-)
void UsbdDeallocateVm(void);	   	//:-)
BOOL UsbdAllocateVm(void);	   		//:-)
void UsbdInitDma(int epnum, int bufIndex,int bufOffset);	//:-)


#endif //_SC2440PDD_H_


