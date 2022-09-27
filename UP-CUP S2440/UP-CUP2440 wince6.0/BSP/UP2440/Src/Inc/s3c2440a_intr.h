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
//  Header:  s3c2440a_intr.h
//
//  Defines the interrupt controller register layout and associated interrupt
//  sources and bit masks.
//
#ifndef __S3C2440A_INTR_H
#define __S3C2440A_INTR_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Type: S3C2440A_INTR_REG    
//
//  Interrupt control registers. This register bank is located by the constant 
//  S3C2440A_BASE_REG_XX_INTR in the configuration file s3c2440a_base_reg_cfg.h.
//

typedef struct {
    UINT32 SRCPND;                     // interrupt request status reg
    UINT32 INTMOD;                     // interrupt mode reg
    UINT32 INTMSK;                     // interrupt mask reg
    UINT32 PRIORITY;                   // priority reg
    UINT32 INTPND;                     // interrupt pending reg
    UINT32 INTOFFSET;                  // interrupt offset reg
    UINT32 SUBSRCPND;                  // SUB source pending reg
    UINT32 INTSUBMSK;                  // interrupt SUB mask reg

} S3C2440A_INTR_REG, *PS3C2440A_INTR_REG;


//------------------------------------------------------------------------------
//
//  Define: IRQ_XXX
//
//  Interrupt sources numbers
//

#define IRQ_EINT0           0           // Arbiter 0
#define IRQ_EINT1           1
#define IRQ_EINT2           2
#define IRQ_EINT3           3

#define IRQ_EINT4_7         4           // Arbiter 1
#define IRQ_EINT8_23        5
#define IRQ_CAM             6
#define IRQ_BAT_FLT         7
#define IRQ_TICK            8
#define IRQ_WDT        9

#define IRQ_TIMER0          10          // Arbiter 2
#define IRQ_TIMER1          11
#define IRQ_TIMER2          12
#define IRQ_TIMER3          13
#define IRQ_TIMER4          14
#define IRQ_UART2           15

#define IRQ_LCD             16          // Arbiter 3
#define IRQ_DMA0            17
#define IRQ_DMA1            18
#define IRQ_DMA2            19
#define IRQ_DMA3            20
#define IRQ_SDI             21

#define IRQ_SPI0            22          // Arbiter 4
#define IRQ_UART1           23
#define IRQ_NFCON           24
#define IRQ_USBD            25
#define IRQ_USBH            26
#define IRQ_IIC             27

#define IRQ_UART0           28          // Arbiter 5
#define IRQ_SPI1            29
#define IRQ_RTC             30
#define IRQ_ADC             31

#define IRQ_EINT4           32
#define IRQ_EINT5           33
#define IRQ_EINT6           34
#define IRQ_EINT7           35
#define IRQ_EINT8           36
#define IRQ_EINT9           37
#define IRQ_EINT10          38
#define IRQ_EINT11          39
#define IRQ_EINT12          40
#define IRQ_EINT13          41
#define IRQ_EINT14          42
#define IRQ_EINT15          43
#define IRQ_EINT16          44
#define IRQ_EINT17          45
#define IRQ_EINT18          46
#define IRQ_EINT19          47
#define IRQ_EINT20          48
#define IRQ_EINT21          49
#define IRQ_EINT22          50
#define IRQ_EINT23          51


// Interrupt sub-register source numbers
//
#define IRQ_SUB_RXD0     0
#define IRQ_SUB_TXD0     1
#define IRQ_SUB_ERR0     2
#define IRQ_SUB_RXD1     3
#define IRQ_SUB_TXD1     4
#define IRQ_SUB_ERR1     5
#define IRQ_SUB_RXD2     6
#define IRQ_SUB_TXD2     7
#define IRQ_SUB_ERR2     8
#define IRQ_SUB_TC       9
#define IRQ_SUB_ADC      10
#define	IRQ_SUB_CAM_C	 11		// 030610
#define	IRQ_SUB_CAM_P	 12		// 040218, INTSUB_CAM_S - 030610
#define	IRQ_SUB_WDT		 13		// 040218
//#define	IRQ_SUB_AC97	 14		// 040218

//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif 
