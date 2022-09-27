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
//  File:  bsp_cfg.h
//
//  This file contains system constant specific for up2440 board.
//
#ifndef __BSP_CFG_H
#define __BSP_CFG_H


//------------------------------------------------------------------------------
// Misc defines.
//------------------------------------------------------------------------------
#define D1_2			0x0
#define D1_4			0x1
#define D1_8			0x2
#define D1_16			0x3
#define DV2			2
#define DV4			4
#define DV8			8
#define DV16			16



//------------------------------------------------------------------------------
//
//  Define:  BSP_DEVICE_PREFIX
//
//  Prefix used to generate device name for bootload/KITL
//
#define BSP_DEVICE_PREFIX       "UP2440"        // Device name prefix
//------------------------------------------------------------------------------
// Board clock
//------------------------------------------------------------------------------
#if 1
#define S3C2440A_BASE_REG_VA_CLOCK_POWER	0xB0C00000
#define PLLVALUE			(((S3C2440A_CLKPWR_REG*)(S3C2440A_BASE_REG_VA_CLOCK_POWER))->MPLLCON)
#define CLKDIV				(((S3C2440A_CLKPWR_REG*)(S3C2440A_BASE_REG_VA_CLOCK_POWER))->CLKDIVN)
#define CAMDIV				(((S3C2440A_CLKPWR_REG*)(S3C2440A_BASE_REG_VA_CLOCK_POWER))->CAMDIVN)
#define	S3C2440A_FCLK			(2 * (((PLLVALUE >> 12) & 0xff) + 8L) * (12000000L / (((PLLVALUE >> 4) & 0x3f)+2L) / (1<<((PLLVALUE >> 0) & 0x3))))	// PLLVALUE = v_pCLKPWRreg->MPLLCON
#define	HDIVN				((CLKDIV>>1) & 0x3)
#define	PDIVN				((CLKDIV>>0) & 0x1)
#define	HCLK3_HALF			((CAMDIV >> 8) & 0x1)
#define	HCLK4_HALF			((CAMDIV >> 9) & 0x1)
#define	HCLKDIV				((HDIVN & 0x2) == 0 ? (HDIVN == 0 ? 1:2) : (HDIVN == 2 ? (HCLK4_HALF == 0 ? 4:8) : (HCLK3_HALF == 0 ? 3:6)))										// CLKDIV = v_pCLKPWRreg->CLKDIVN
#define	PCLKDIV				(HCLKDIV * ((PDIVN == 0) ? 1 : 2))				// CLKDIV = v_pCLKPWRreg->CLKDIVN

#define S3C2440A_HCLK			(S3C2440A_FCLK/HCLKDIV)
#define S3C2440A_PCLK			(S3C2440A_FCLK/PCLKDIV)   
#else

#define	S3C2440A_FCLK			400000000
#define S3C2440A_HCLK			(S3C2440A_FCLK/3)
#define S3C2440A_PCLK			(S3C2440A_FCLK/6)  
#endif
#define SYS_TIMER_DIVIDER	DV16//DV2

// For configuration 1ms-System Tick Timer in Auto-Reload
//  PCLK: 49392000Hz
//    Divider: 2      
//     Prescaler:0,   TCNTB4:24695
//     Prescaler:2,   TCNTB4:8231
//     Prescaler:8,   TCNTB4:2743 
//     Prescaler:35,  TCNTB4:685 
//     Prescaler:55,  TCNTB4:440 
//     Prescaler:125, TCNTB4:196 
//	   Prescaler:251, TCNTB4:97

//  PCLK: 66483200Hz
//    Divider: 2      
//     Prescaler:0,   TCNTB4:33241
//     Prescaler:1,   TCNTB4:16620
//     Prescaler:10,   TCNTB4:3021 
//     Prescaler:21,  TCNTB4:1510 
//     Prescaler:119, TCNTB4:276 
// !Important! If you use Timer4 as One-Shot Mode, Above TCNTB4 is added more 1.
// So, You define (TCNTB4 + 1) value as OEM_COUNT_1MS

   
#define PRESCALER			(245-1)//35 //50Mhz:35, 66Mhz:21


// Timer count for 1 ms (Default: Auto-Reload mode)
#define OEM_COUNT_1MS			17//685		//50Mhz:685, 66Mhz:1510   In case One-Shot Mode 50Mhz:808, 66Mhz:1511
#define RESCHED_PERIOD			1		// 10				// Reschedule period in ms

// Timer count value for rescheduler period (???? for RESCHED_PERIOD ms system tick)
#define RESCHED_INCREMENT   ( RESCHED_PERIOD * OEM_COUNT_1MS )

//*********************		//use for eboot
#if 1
#define DEBUG_ENABLE		1
#else
#define DEBUG_ENABLE		0
#endif

//*********************		//use for eboot
#define eboot_nouse_UART	0
#define eboot_use_UART0		1
#define eboot_use_UART1		2
#define eboot_UARTn		eboot_use_UART0

//*********************		//use for NK.bin
#define BSP_nouse_UART		(0)
#define BSP_use_UART0		(1)
#define BSP_use_UART1		(2)
#define BSP_UARTn		BSP_use_UART0


#define BSP_UART0_ULCON         0x03                // 8 bits, 1 stop, no parity
#define BSP_UART0_UCON          0x0005              // pool mode, PCLK for UART
#define BSP_UART0_UFCON         0x00                // disable FIFO
#define BSP_UART0_UMCON         0x00                // disable auto flow control
#define BSP_UART0_UBRDIV        (S3C2440A_PCLK/(115200*16) - 1)

#define BSP_UART1_ULCON         0x03                // 8 bits, 1 stop, no parity
#define BSP_UART1_UCON          0x0005              // pool mode, PCLK for UART
#define BSP_UART1_UFCON         0x00                // disable FIFO
#define BSP_UART1_UMCON         0x00                // disable auto flow control
#define BSP_UART1_UBRDIV        (S3C2440A_PCLK/(115200*16) - 1)

//------------------------------------------------------------------------------
// Static SYSINTR Mapping for driver.
#define SYSINTR_OHCI            (SYSINTR_FIRMWARE+1)
#define SYSINTR_ETH            	(SYSINTR_FIRMWARE+19)

// -----------------------------------------------------------------------------
// define For DVS
#define V080	0
#define V090	1
#define V095	2
#define V0975	3
#define V100	4
#define V105	5
#define V110	6
#define V115	7
#define V120	8
#define V125	9
#define V130	10
#define V135	11
#define V140	12
#define V145	13
#define V150	14

#define DVS_METHOD	1  //1:DVS_ON with VSYNC, 2:DVS_ON in idle mode(not change HCLK), 3:mixed
#define USESWPWSAVING	1
#define MVAL_USED		0
#define HIGHVOLTAGE		V130
#define MIDVOLTAGE		V110
#define LOWVOLTAGE		V100
#define VOLTAGEDELAY	16000

#define Eval_Probe		1

#define DVSON		0x1
#define HCLKHALF	0x2
#define ACTIVE		0x4
#define DeepIdle	(DVSON|HCLKHALF)
#define NIdle		(DVSON)
#define LazyActive	(ACTIVE|DVSON|HCLKHALF)
#define SlowActive	(ACTIVE|DVSON)
#define Active		(ACTIVE)
//------------------------------------------------------------

#endif
