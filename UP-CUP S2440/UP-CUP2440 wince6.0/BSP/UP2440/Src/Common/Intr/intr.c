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
//  File: interrupt.c
//
//  This file implement major part of interrupt module for S3C3210X SoC.
//
#include <windows.h>
#include <ceddk.h>
#include <nkintr.h>
#include <oal.h>
#include <s3c2440a.h>
#include <s3c2440a_intr.h>
#include <intr.h>
#ifdef DVS_EN
#include <bsp_cfg.h>
#endif
extern volatile UINT32 g_oalLastSysIntr;


//------------------------------------------------------------------------------
//
//  Globals:  g_pIntrRegs/g_pPortRegs
//
//  The global variables are storing virual address for interrupt and port
//  registers for use in interrupt handling to avoid possible time consumig
//  call to OALPAtoVA function.
//
static S3C2440A_INTR_REG *g_pIntrRegs; 
static S3C2440A_IOPORT_REG *g_pPortRegs;
#ifdef DVS_EN
static S3C2440A_LCD_REG	*g_pLCDRegs;
#endif

extern volatile BOOL b_oalInterruptFlag;
#ifdef DVS_EN
	BOOL	VSYNCINTR;
	BOOL	CurrStateIdle;
	extern volatile BOOL IDLEflag;
	extern DWORD dwCurrentidle;

	volatile int CurrentState = Active;

	extern DWORD dwPrevTotalTick1000;
	extern DWORD dwPrevTotalTick100;

	extern DWORD dwPrevIdleTick1000;
	extern DWORD dwPrevIdleTick100;

	extern DWORD dwPercentIdle1000;
	extern DWORD dwPercentIdle100;

	extern void DVS_ON(void);
	extern void DVS_OFF(void);
	extern void ChangeVoltage(int);
	extern int GetCurrentVoltage(void);
	extern void CLKDIV124();
	extern void CLKDIV144();
	extern void CLKDIV136();
	extern void CLKDIV166();
	extern void CLKDIV148();
	extern void CLKDIV188();
#endif
//  Function pointer to profiling timer ISR routine.
//
PFN_PROFILER_ISR g_pProfilerISR = NULL;
 

//------------------------------------------------------------------------------
//
//  Function:  OALIntrInit
//
//  This function initialize interrupt mapping, hardware and call platform
//  specific initialization.
//
BOOL OALIntrInit()
{
	BOOL rc = FALSE;

	OALMSG( OAL_FUNC&&OAL_INTR, (L"+OALInterruptInit\r\n") );

	// Initialize interrupt mapping
	OALIntrMapInit();

	// First get uncached virtual addresses
	g_pIntrRegs = (S3C2440A_INTR_REG*)OALPAtoVA(
		S3C2440A_BASE_REG_PA_INTR, FALSE
	);
	g_pPortRegs = (S3C2440A_IOPORT_REG*)OALPAtoVA(
		S3C2440A_BASE_REG_PA_IOPORT, FALSE
	);

#ifdef DVS_EN	
	g_pLCDRegs = (S3C2440A_LCD_REG *)OALPAtoVA(
	S3C2440A_BASE_REG_PA_LCD, FALSE
	);
#endif	

	// Mask and clear external interrupts
	OUTREG32(&g_pPortRegs->EINTMASK, 0xFFFFFFFF);
	OUTREG32(&g_pPortRegs->EINTPEND, 0xFFFFFFFF);

	// Mask and clear internal interrupts
	OUTREG32(&g_pIntrRegs->INTMSK, 0xFFFFFFFF);
	OUTREG32(&g_pIntrRegs->SRCPND, 0xFFFFFFFF);

	// S3C2440A developer notice (page 4) warns against writing a 1 to any
	// 0 bit field in the INTPND register.  Instead we'll write the INTPND
	// value itself.
	OUTREG32(&g_pIntrRegs->INTPND, INREG32(&g_pIntrRegs->INTPND));

	// Unmask the system tick timer interrupt
	CLRREG32(&g_pIntrRegs->INTMSK, 1 << IRQ_TIMER4);

#ifdef OAL_BSP_CALLBACKS
	// Give BSP change to initialize subordinate controller
	rc = BSPIntrInit();
#else
	rc = TRUE;
#endif

	OALMSG(OAL_INTR&&OAL_FUNC, (L"-OALInterruptInit(rc = %d)\r\n", rc));
	return rc;
}



//------------------------------------------------------------------------------
//
//  Function:  OALIntrRequestIrqs
//
//  This function returns IRQs for CPU/SoC devices based on their
//  physical address.
//
BOOL OALIntrRequestIrqs(DEVICE_LOCATION *pDevLoc, UINT32 *pCount, UINT32 *pIrqs)
{
	BOOL rc = FALSE;

	OALMSG(OAL_INTR&&OAL_FUNC, (
		L"+OALIntrRequestIrqs(0x%08x->%d/%d/0x%08x/%d, 0x%08x, 0x%08x)\r\n",
		pDevLoc, pDevLoc->IfcType, pDevLoc->BusNumber, pDevLoc->LogicalLoc,
		pDevLoc->Pin, pCount, pIrqs
	));

	// This shouldn't happen
	if (*pCount < 1) goto cleanUp;

#ifdef OAL_BSP_CALLBACKS
	rc = BSPIntrRequestIrqs(pDevLoc, pCount, pIrqs);
#endif

cleanUp:
	OALMSG(OAL_INTR&&OAL_FUNC, (L"-OALIntrRequestIrqs(rc = %d)\r\n", rc));
	return rc;
}


//------------------------------------------------------------------------------
//
//  Function:  OALIntrEnableIrqs
//
BOOL OALIntrEnableIrqs(UINT32 count, const UINT32 *pIrqs)
{
	BOOL rc = TRUE;
	UINT32 i, mask, irq;

	OALMSG(OAL_INTR&&OAL_FUNC, (
		L"+OALIntrEnableIrqs(%d, 0x%08x)\r\n", count, pIrqs
	));

	for (i = 0; i < count; i++) {
#ifndef OAL_BSP_CALLBACKS
		irq = pIrqs[i];
#else
		// Give BSP chance to enable irq on subordinate interrupt controller
		irq = BSPIntrEnableIrq(pIrqs[i]);
#endif
		if (irq == OAL_INTR_IRQ_UNDEFINED) continue;
		// Depending on IRQ number use internal or external mask register
		if (irq <= IRQ_ADC) {
			// Use interrupt mask register
			CLRREG32(&g_pIntrRegs->INTMSK, 1 << irq);
		} else if (irq <= IRQ_EINT7) {
			// Use external mask register
			CLRREG32(&g_pIntrRegs->INTMSK, 1 << IRQ_EINT4_7);
			CLRREG32(&g_pPortRegs->EINTMASK, 1 << (irq - IRQ_EINT4 + 4));
			OUTREG32(&g_pPortRegs->EINTPEND, mask);			//HJ_add 20100112
		} else if (irq <= IRQ_EINT23) {
			// Use external mask register
			mask = 1 << (irq - IRQ_EINT4 + 4);
			OUTREG32(&g_pPortRegs->EINTPEND, mask);
			CLRREG32(&g_pPortRegs->EINTMASK, mask);
			mask = 1 << IRQ_EINT8_23;
			if ((INREG32(&g_pIntrRegs->INTPND) & mask) != 0) {
				OUTREG32(&g_pIntrRegs->INTPND, mask);
			}
			CLRREG32( &g_pIntrRegs->INTMSK, 1 << IRQ_EINT8_23);
		} else {
			rc = FALSE;
		}
	}

	OALMSG(OAL_INTR&&OAL_FUNC, (L"-OALIntrEnableIrqs(rc = %d)\r\n", rc));
	return rc;	
}


//------------------------------------------------------------------------------
//
//  Function:  OALIntrDisableIrqs
//
VOID OALIntrDisableIrqs(UINT32 count, const UINT32 *pIrqs)
{
	UINT32 i, mask, irq;

	OALMSG(OAL_INTR&&OAL_FUNC, (
		L"+OALIntrDisableIrqs(%d, 0x%08x)\r\n", count, pIrqs
	));

	for (i = 0; i < count; i++) {
#ifndef OAL_BSP_CALLBACKS
		irq = pIrqs[i];
#else
		// Give BSP chance to disable irq on subordinate interrupt controller
		irq = BSPIntrDisableIrq(pIrqs[i]);
		if (irq == OAL_INTR_IRQ_UNDEFINED) continue;
#endif
		// Depending on IRQ number use internal or external mask register
		if (irq <= IRQ_ADC) {
			// Use interrupt mask register
			mask = 1 << irq;
			SETREG32(&g_pIntrRegs->INTMSK, mask);
		} else if (irq <= IRQ_EINT23) {
			// Use external mask register
			mask = 1 << (irq - IRQ_EINT4 + 4);
			SETREG32(&g_pPortRegs->EINTMASK, mask);
		}
	}

	OALMSG(OAL_INTR&&OAL_FUNC, (L"-OALIntrDisableIrqs\r\n"));
}


//------------------------------------------------------------------------------
//
//  Function:  OALIntrDoneIrqs
//
VOID OALIntrDoneIrqs(UINT32 count, const UINT32 *pIrqs)
{
	UINT32 i, mask, irq;

	OALMSG(OAL_INTR&&OAL_VERBOSE, (
		L"+OALIntrDoneIrqs(%d, 0x%08x)\r\n", count, pIrqs
	));

	for (i = 0; i < count; i++) {
#ifndef OAL_BSP_CALLBACKS
		irq = pIrqs[i];
#else
		// Give BSP chance to finish irq on subordinate interrupt controller
		irq = BSPIntrDoneIrq(pIrqs[i]);
#endif	
		// Depending on IRQ number use internal or external mask register
		if (irq <= IRQ_ADC) {
			// Use interrupt mask register
			mask = 1 << irq;
			OUTREG32(&g_pIntrRegs->SRCPND, mask);
			CLRREG32(&g_pIntrRegs->INTMSK, mask);
		} else if (irq <= IRQ_EINT23) {
			// Use external mask register
			mask = 1 << (irq - IRQ_EINT4 + 4);
			OUTREG32(&g_pPortRegs->EINTPEND, mask);
			CLRREG32(&g_pPortRegs->EINTMASK, mask);
		}
	}

	OALMSG(OAL_INTR&&OAL_VERBOSE, (L"-OALIntrDoneIrqs\r\n"));
}


//------------------------------------------------------------------------------
//
//  Function:  OEMInterruptHandler
//
ULONG OEMInterruptHandler(ULONG ra)
{
	UINT32 sysIntr = SYSINTR_NOP;
	UINT32 irq, irq2, mask;
#ifdef DVS_EN
#if (DVS_METHOD == 1 || DVS_METHOD == 3)
	unsigned int clkval_calc;
	unsigned int i;
#endif //(DVS_METHOD == 1 || DVS_METHOD == 3)
#endif //DVS_EN
	
	static DWORD HeartBeatCnt, HeartBeatStat;  //LED4 is used for heart beat
	
	b_oalInterruptFlag = TRUE;

	// for forbidding to miss interrupt
	while(!((1<<INREG32(&g_pIntrRegs->INTOFFSET))&INREG32(&g_pIntrRegs->INTPND)))
		INREG32(&g_pIntrRegs->INTOFFSET);
	// Get pending interrupt(s)
	irq = INREG32(&g_pIntrRegs->INTOFFSET);

	if(irq == 35) RETAILMSG(1, (TEXT("OEMinterruptHandle 35 here\r\n")));
#if 0
	 if(irq == IRQ_SDI)
	RETAILMSG(1,(TEXT("IRQ_SDI interrupt is occured!!\r\n")));
	if(irq == IRQ_DMA0)
	RETAILMSG(1,(TEXT("IRQ_DMA0 interrupt is occured!!\r\n")));


	if(irq == IRQ_USBD)
	RETAILMSG(1,(TEXT("IRQ_USBD interrupt is occured!!\r\n")));
	if(irq == IRQ_USBH)	
	RETAILMSG(1,(TEXT("IRQ_USBH interrupt is occured!!\r\n")));   
#endif
	if(irq == IRQ_RTC)
	RETAILMSG(1,(TEXT("IRQ_RTC ALARM interrupt is occured!!\r\n")));
	
#ifdef DVS_EN
#if (DVS_METHOD == 2)
//	if ( GetCurrentVoltage() != HIGHVOLTAGE )
//	{
//		ChangeVoltage(HIGHVOLTAGE);
//		for(i=0;i<VOLTAGEDELAY;i++)
//		{
//			INREG32(&g_pPortRegs->GPFDAT);  // for loop operation, just read.
//		}
//		DVS_OFF();
//	}

//	if ( GetCurrentVoltage() != HIGHVOLTAGE )
//	{
//		ChangeVoltage(HIGHVOLTAGE);
//		for(i=0;i<VOLTAGEDELAY;i++)
//		{
//			INREG32(&g_pPortRegs->GPFDAT);  // for loop operation, just read.
//		}
//		DVS_OFF();
//
//		switch ( HCLKDIV )
//		{
//		case 4:
//			CLKDIV124();
//			break;
//		case 6:
//			CLKDIV136();
//			break;
//		case 8:
//			CLKDIV148();
//			break;
//		}
//	}
	}
#endif //(DVS_METHOD == 3)

#endif //DEV_EN

	// System timer interrupt?
	if (irq == IRQ_TIMER4) {
#if 0				//HJ_add 20100112
		if (++HeartBeatCnt > 500)
		{
			HeartBeatCnt   = 0;
			HeartBeatStat ^= 1;

			if (HeartBeatStat)						
				g_pPortRegs->GPFDAT &= ~(1 << 4); // LED 4 On
			else
				g_pPortRegs->GPFDAT |=  (1 << 4); // LED 4 Off
		}
#endif				//HJ_add 20100112
		// Clear the interrupt
		OUTREG32(&g_pIntrRegs->SRCPND, 1 << IRQ_TIMER4);
		OUTREG32(&g_pIntrRegs->INTPND, 1 << IRQ_TIMER4);

		// Rest is on timer interrupt handler
		sysIntr = OALTimerIntrHandler();
	}	
	// Profiling timer interrupt?
	else if (irq == IRQ_TIMER2)
	{
		// Mask and Clear the interrupt.
		mask = 1 << irq;
		SETREG32(&g_pIntrRegs->INTMSK, mask);
		OUTREG32(&g_pIntrRegs->SRCPND, mask);
		OUTREG32(&g_pIntrRegs->INTPND, mask);

		// The rest is up to the profiling interrupt handler (if profiling
		// is enabled).
		//
		if (g_pProfilerISR)
		{
			sysIntr = g_pProfilerISR(ra);
		}
	}
#if 0						IRQ_EINT2 use to K3
	else if (irq == IRQ_EINT2)		// Softreset
	{
		OUTREG32(&g_pIntrRegs->SRCPND, 1 << IRQ_EINT2);
		OUTREG32(&g_pIntrRegs->INTPND, 1 << IRQ_EINT2);
#ifdef DVS_EN
#if (DVS_METHOD == 1 || DVS_METHOD == 3)
		ChangeVoltage(HIGHVOLTAGE);
		for(i=0;i<VOLTAGEDELAY;i++)
		{
			INREG32(&g_pPortRegs->GPFDAT); // for loop operation, just read.
		}
		DVS_OFF();
		switch ( HCLKDIV )
		{
		case 4:
			CLKDIV124();
			break;
		case 6:
			CLKDIV136();
			break;
		case 8:
			CLKDIV148();
			break;
		}
#endif //(DVS_METHOD == 1 || DVS_METHOD == 3)
#endif //DVS_EN		
		OALIoCtlHalReboot(IOCTL_HAL_REBOOT,NULL,0,NULL,0,NULL);
		sysIntr = SYSINTR_NOP;
	}
#endif
#ifdef DVS_EN
#if (DVS_METHOD == 1)
	else if(irq == IRQ_LCD)
	{
		VSYNCINTR = TRUE;
		if ( IDLEflag == FALSE )
		{

			// 4.1 Mask LCD VSYNC Interrupt
			OUTREG32(&g_pIntrRegs->INTMSK, INREG32(&g_pIntrRegs->INTMSK ) | (1 << IRQ_LCD ));
			if( INREG32(&g_pLCDRegs->LCDSRCPND) & 2) OUTREG32(&g_pLCDRegs->LCDSRCPND, 0x2);
			if( INREG32(&g_pLCDRegs->LCDINTPND) & 2) OUTREG32(&g_pLCDRegs->LCDINTPND, 0x2);			
			if( INREG32(&g_pIntrRegs->SRCPND) & ( 0x1 << IRQ_LCD)) OUTREG32(&g_pIntrRegs->SRCPND, 0x1 << IRQ_LCD);
			if( INREG32(&g_pIntrRegs->INTPND) & ( 0x1 << IRQ_LCD)) OUTREG32(&g_pIntrRegs->INTPND, 0x1 << IRQ_LCD);

			if (INREG32(&g_pLCDRegs->LCDCON5) & (0x3 << 15) )	// Check VSync Area.
			{
				//RETAILMSG(1,(TEXT("-")));
			}
			else
			{
				CurrStateIdle = FALSE;
				OUTREG32(&g_pLCDRegs->LCDCON1, INREG32(&g_pLCDRegs->LCDCON1) & ~(0x1)); //Disable LCD Output

				// 4.2 Return Voltage
				ChangeVoltage(HIGHVOLTAGE);

				// 4.3 Delay 
				for(i=0;i<VOLTAGEDELAY;i++)
				{
					INREG32(&g_pPortRegs->GPFDAT); // for loop operation, just read.
				}

				// 4.4 DVS_ON = 0 (6:6:6->1:6:6)
				DVS_OFF();
				switch ( HCLKDIV )
				{
				case 4:
					CLKDIV124();
					break;
				case 6:
					CLKDIV136();
					break;
				case 8:
					CLKDIV148();
					break;
				}
				clkval_calc = (WORD)((float)(S3C2440A_HCLK)/(2.0*5000000)+0.5)-1;

				OUTREG32(&g_pLCDRegs->LCDCON1, (clkval_calc << 8)|(MVAL_USED << 7)|(3 << 5)|(12 << 1)|(1 << 0));
			}
			//RETAILMSG(1,(TEXT("N")));
		}
		else
		{

			// 2.1 Mask LCD VSYNC Interrupt
			OUTREG32(&g_pIntrRegs->INTMSK, INREG32(&g_pIntrRegs->INTMSK) | (1 << IRQ_LCD)); // disable LCD interrupt
			if( INREG32(&g_pLCDRegs->LCDSRCPND) & 2) OUTREG32(&g_pLCDRegs->LCDSRCPND, 0x2);
			if( INREG32(&g_pLCDRegs->LCDINTPND) & 2) OUTREG32(&g_pLCDRegs->LCDINTPND, 0x2);
			if( INREG32(&g_pIntrRegs->SRCPND) & ( 0x1 << IRQ_LCD)) OUTREG32(&g_pIntrRegs->SRCPND, 0x1 << IRQ_LCD);
			if( INREG32(&g_pIntrRegs->INTPND) & ( 0x1 << IRQ_LCD)) OUTREG32(&g_pIntrRegs->INTPND, 0x1 << IRQ_LCD);

			if (INREG32(&g_pLCDRegs->LCDCON5) & (0x3 << 15) )	// Check VSync Area.
			{
				//RETAILMSG(1,(TEXT("|")));
			}
			else 
			{
				CurrStateIdle = TRUE;
				OUTREG32(&g_pLCDRegs->LCDCON1, INREG32(&g_pLCDRegs->LCDCON1) & ~(0x1)); //Disable LCD Output
				switch ( HCLKDIV )
				{
				case 2:
					CLKDIV144();
					break;
				case 3:
					CLKDIV166();
					break;
				case 4:
					CLKDIV188();
					break;
				}
				DVS_ON();

				// 2.4 Drop Voltage
				ChangeVoltage(LOWVOLTAGE);

				clkval_calc = (WORD)((float)(S3C2440A_HCLK)/(2.0*5000000)+0.5)-1;

				OUTREG32(&g_pLCDRegs->LCDCON1, (clkval_calc << 8)|(MVAL_USED << 7)|(3 << 5)|(12 << 1)|(1 << 0));

			}
			//RETAILMSG(1,(TEXT("I")));
		}
		sysIntr = SYSINTR_NOP;
	}
#elif (DVS_METHOD == 3)
	else if( irq == IRQ_LCD )
	{
		VSYNCINTR = TRUE;
		if ( CurrentState == Active )
		{
			OUTREG32(&g_pIntrRegs->INTMSK, INREG32(&g_pIntrRegs->INTMSK) | (1 << IRQ_LCD)); // Disable LCD interrupt
			if( INREG32(&g_pLCDRegs->LCDSRCPND) & 2) OUTREG32(&g_pLCDRegs->LCDSRCPND, 0x2);
			if( INREG32(&g_pLCDRegs->LCDINTPND) & 2) OUTREG32(&g_pLCDRegs->LCDINTPND, 0x2);			
			if( INREG32(&g_pIntrRegs->SRCPND) & ( 0x1 << IRQ_LCD)) OUTREG32(&g_pIntrRegs->SRCPND, 0x1 << IRQ_LCD);
			if( INREG32(&g_pIntrRegs->INTPND) & ( 0x1 << IRQ_LCD)) OUTREG32(&g_pIntrRegs->INTPND, 0x1 << IRQ_LCD);

			if (INREG32(&g_pLCDRegs->LCDCON5) & (0x3 << 15) )	// Check VSync Area.
			{RETAILMSG(1,(TEXT("-BD-")));}
			else
			{
				OUTREG32(&g_pLCDRegs->LCDCON1, INREG32(&g_pLCDRegs->LCDCON1) & ~(0x1)); //Disable LCD Output...
				ChangeVoltage(HIGHVOLTAGE);
				for(i=0;i<VOLTAGEDELAY;i++) { INREG32(&g_pPortRegs->GPFDAT); } // for loop operation, just read.
				DVS_OFF();
				switch ( HCLKDIV )
				{
				case 4:
					CLKDIV124();
					break;
				case 6:
					CLKDIV136();
					break;
				case 8:
					CLKDIV148();
					break;
				}
				clkval_calc = (WORD)((float)(S3C2440A_HCLK)/(2.0*5000000)+0.5)-1;
				OUTREG32(&g_pLCDRegs->LCDCON1, (clkval_calc << 8)|(MVAL_USED << 7)|(3 << 5)|(12 << 1)|(1 << 0));	// Enable LCD Output...
//				RETAILMSG(1,(TEXT("-a-")));
			}
		}
		else if ( CurrentState == SlowActive )
		{
			VSYNCINTR = TRUE;
			if ( IDLEflag == FALSE )
			{
				OUTREG32(&g_pIntrRegs->INTMSK, INREG32(&g_pIntrRegs->INTMSK) | (1 << IRQ_LCD)); // disable LCD interrupt
				if( INREG32(&g_pLCDRegs->LCDSRCPND) & 2) OUTREG32(&g_pLCDRegs->LCDSRCPND, 0x2);
				if( INREG32(&g_pLCDRegs->LCDINTPND) & 2) OUTREG32(&g_pLCDRegs->LCDINTPND, 0x2);			
				if( INREG32(&g_pIntrRegs->SRCPND) & ( 0x1 << IRQ_LCD)) OUTREG32(&g_pIntrRegs->SRCPND, 0x1 << IRQ_LCD);
				if( INREG32(&g_pIntrRegs->INTPND) & ( 0x1 << IRQ_LCD)) OUTREG32(&g_pIntrRegs->INTPND, 0x1 << IRQ_LCD);

				if (INREG32(&g_pLCDRegs->LCDCON5) & (0x3 << 15) )	// Check VSync Area.
				{RETAILMSG(1,(TEXT("-BD-")));}
				else
				{
					CurrStateIdle = FALSE;
					OUTREG32(&g_pLCDRegs->LCDCON1, INREG32(&g_pLCDRegs->LCDCON1) & ~(0x1)); //Disable LCD Output
					ChangeVoltage(HIGHVOLTAGE);
					for(i=0;i<VOLTAGEDELAY;i++) { INREG32(&g_pPortRegs->GPFDAT); } // for loop operation, just read.
					DVS_OFF();
					switch ( HCLKDIV )
					{
					case 4:
						CLKDIV124();
						break;
					case 6:
						CLKDIV136();
						break;
					case 8:
						CLKDIV148();
						break;
					}
					clkval_calc = (WORD)((float)(S3C2440A_HCLK)/(2.0*5000000)+0.5)-1;
					OUTREG32(&g_pLCDRegs->LCDCON1, (clkval_calc << 8)|(MVAL_USED << 7)|(3 << 5)|(12 << 1)|(1 << 0));
				}
//				RETAILMSG(1,(TEXT("-F-")));
			}
			else //if ( IDLEflag == TRUE )
			{
				OUTREG32(&g_pIntrRegs->INTMSK, INREG32(&g_pIntrRegs->INTMSK) | (1 << IRQ_LCD)); // disable LCD interrupt
				if( INREG32(&g_pLCDRegs->LCDSRCPND) & 2) OUTREG32(&g_pLCDRegs->LCDSRCPND, 0x2);
				if( INREG32(&g_pLCDRegs->LCDINTPND) & 2) OUTREG32(&g_pLCDRegs->LCDINTPND, 0x2);			
				if( INREG32(&g_pIntrRegs->SRCPND) & ( 0x1 << IRQ_LCD)) OUTREG32(&g_pIntrRegs->SRCPND, 0x1 << IRQ_LCD);
				if( INREG32(&g_pIntrRegs->INTPND) & ( 0x1 << IRQ_LCD)) OUTREG32(&g_pIntrRegs->INTPND, 0x1 << IRQ_LCD);

				if (INREG32(&g_pLCDRegs->LCDCON5) & (0x3 << 15) )	// Check VSync Area.
				{RETAILMSG(1,(TEXT("-BD-")));}
				else 
				{
					CurrStateIdle = TRUE;
					OUTREG32(&g_pLCDRegs->LCDCON1, INREG32(&g_pLCDRegs->LCDCON1) & ~(0x1)); //Disable LCD Output
					switch ( HCLKDIV )
					{
					case 2:
						CLKDIV144();
						break;
					case 3:
						CLKDIV166();
						break;
					case 4:
						CLKDIV188();
						break;
					}
					DVS_ON();
					ChangeVoltage(LOWVOLTAGE);
					clkval_calc = (WORD)((float)(S3C2440A_HCLK)/(2.0*5000000)+0.5)-1;
					OUTREG32(&g_pLCDRegs->LCDCON1, (clkval_calc << 8)|(MVAL_USED << 7)|(3 << 5)|(12 << 1)|(1 << 0));
				}
//				RETAILMSG(1,(TEXT("-T-")));
			}
//			RETAILMSG(1,(TEXT("-%d-"), clkval_calc));
		}
		sysIntr = SYSINTR_NOP;
	}
#endif //(DVS_METHOD == 3)


#endif  //DVS_EN

	else
	{

#ifdef OAL_ILTIMING
		if (g_oalILT.active) {
			g_oalILT.isrTime1 = OALTimerCountsSinceSysTick();
			g_oalILT.savedPC = 0;
			g_oalILT.interrupts++;
		}
#endif

		if (irq == IRQ_EINT4_7 || irq == IRQ_EINT8_23) { // 4 or 5

			// Find external interrupt number
			mask = INREG32(&g_pPortRegs->EINTPEND);
			mask &= ~INREG32(&g_pPortRegs->EINTMASK);
			mask = (mask ^ (mask - 1)) >> 5;
			irq2 = IRQ_EINT4;
			while (mask != 0) {
				mask >>= 1;
				irq2++;
			}

			// Mask and clear interrupt
			mask = 1 << (irq2 - IRQ_EINT4 + 4);
			SETREG32(&g_pPortRegs->EINTMASK, mask);
			OUTREG32(&g_pPortRegs->EINTPEND, mask);

			// Clear primary interrupt
			mask = 1 << irq;
			OUTREG32(&g_pIntrRegs->SRCPND, mask);
			OUTREG32(&g_pIntrRegs->INTPND, mask);

			// From now we care about this irq
			irq = irq2;

		}
	  else if(irq == IRQ_CAM)
		{
			if(INREG32(&g_pIntrRegs->SUBSRCPND) & (1<<IRQ_SUB_CAM_C))
			{
			  SETREG32(&g_pIntrRegs->INTSUBMSK, (1<<IRQ_SUB_CAM_C));
			  SETREG32(&g_pIntrRegs->INTMSK, (1<<IRQ_CAM));
			  OUTREG32(&g_pIntrRegs->SUBSRCPND, (1<<IRQ_SUB_CAM_C));
			  OUTREG32(&g_pIntrRegs->SRCPND, (1<<IRQ_CAM));
			  OUTREG32(&g_pIntrRegs->INTPND,(1<<IRQ_CAM));
			  //RETAILMSG(1,(TEXT("IRQ_CAM Codec\r\n")));
			}

			else if(INREG32(&g_pIntrRegs->SUBSRCPND) & (1<<IRQ_SUB_CAM_P))
		 	{
			  SETREG32(&g_pIntrRegs->INTSUBMSK, (1<<IRQ_SUB_CAM_P));
			  SETREG32(&g_pIntrRegs->INTMSK, (1<<IRQ_CAM));
			  OUTREG32(&g_pIntrRegs->SUBSRCPND, (1<<IRQ_SUB_CAM_P));
			  OUTREG32(&g_pIntrRegs->SRCPND, (1<<IRQ_CAM));
			  OUTREG32(&g_pIntrRegs->INTPND,(1<<IRQ_CAM));
			  //RETAILMSG(1,(TEXT("PreView\r\n")));
			}

			else
			{
				SETREG32(&g_pIntrRegs->INTSUBMSK, (1<<IRQ_SUB_CAM_C)|(1<<IRQ_SUB_CAM_P));
				SETREG32(&g_pIntrRegs->INTMSK, (1<<IRQ_CAM));
				SETREG32(&g_pIntrRegs->SUBSRCPND, (1<<IRQ_SUB_CAM_C)|(1<<IRQ_SUB_CAM_P));
				OUTREG32(&g_pIntrRegs->SRCPND, (1<<IRQ_CAM));
				OUTREG32(&g_pIntrRegs->INTPND,(1<<IRQ_CAM));
					//RETAILMSG(1,(TEXT("nop\r\n")));				
				return SYSINTR_NOP; 
			}
		}
		else 
		{

			// Mask and clear interrupt
			mask = 1 << irq;
			SETREG32(&g_pIntrRegs->INTMSK, mask);
			OUTREG32(&g_pIntrRegs->SRCPND, mask);
			OUTREG32(&g_pIntrRegs->INTPND, mask);

		}

		// First find if IRQ is claimed by chain
		sysIntr = NKCallIntChain((UCHAR)irq);
		if (sysIntr == SYSINTR_CHAIN || !NKIsSysIntrValid(sysIntr)) {
			// IRQ wasn't claimed, use static mapping
			sysIntr = OALIntrTranslateIrq(irq);
		}
	}

	g_oalLastSysIntr = sysIntr;

	return sysIntr;
}

