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
//  File:  fake.c
//
//  This file contains fake/busy loop implementation if OALCPUIdle function. 
//  It is assumed to be used in development only.
//
#include <windows.h>
#include <nkintr.h>
#include <oal.h>
#include <s3c2440a.h>
#include <bsp.h>
 
volatile BOOL b_oalInterruptFlag;

#ifdef DVS_EN
volatile BOOL IDLEflag = FALSE;
extern BOOL	VSYNCINTR;
extern BOOL	CurrStateIdle;

extern void DVS_ON(void);
extern void DVS_OFF(void);
extern void ChangeVoltage(int);

extern volatile int CurrentState;
#endif


//------------------------------------------------------------------------------
//
//  Function:   OALCPUIdle
//
//  This Idle function implements a busy idle. It is intend to be used only
//  in development (when CPU doesn't support idle mode it is better to stub
//  OEMIdle function instead use this busy loop). The busy wait is cleared by
//  an interrupt from interrupt handler setting the g_oalLastSysIntr.
//

VOID OALCPUIdle()
{
    S3C2440A_IOPORT_REG *pIOPort = (S3C2440A_IOPORT_REG*)OALPAtoVA(S3C2440A_BASE_REG_PA_IOPORT, FALSE);
    S3C2440A_CLKPWR_REG *pClkPwr = (S3C2440A_CLKPWR_REG*)OALPAtoVA(S3C2440A_BASE_REG_PA_CLOCK_POWER, FALSE);
#ifdef DVS_EN
	S3C2440A_LCD_REG *pLCD = (S3C2440A_LCD_REG*)OALPAtoVA(S3C2440A_BASE_REG_PA_LCD, FALSE);
	S3C2440A_INTR_REG *pINTR= (S3C2440A_INTR_REG*)OALPAtoVA(S3C2440A_BASE_REG_PA_INTR, FALSE);
	S3C2440A_IOPORT_REG *g_pPortRegs = (S3C2440A_IOPORT_REG*)OALPAtoVA(S3C2440A_BASE_REG_PA_IOPORT, FALSE);
	int i;

#if(DVS_METHOD == 1)
	if  ( pLCD->LCDSRCPND & 2 ) pLCD->LCDSRCPND = 2;
	if  ( pLCD->LCDINTPND & 2 ) pLCD->LCDINTPND = 2;
	if  ( pINTR->SRCPND & (1 << IRQ_LCD )) pINTR->SRCPND = 1 << IRQ_LCD;
	if  ( pINTR->INTPND & (1 << IRQ_LCD )) pINTR->INTPND = 1 << IRQ_LCD;

	pINTR->INTMSK &= ~(1 << IRQ_LCD);		// enable LCD interrupt

	if ( CurrStateIdle == TRUE )
		pINTR->INTMSK |= (1 << IRQ_LCD);		// disable LCD interrupt

	b_oalInterruptFlag = FALSE;
	IDLEflag = TRUE;
	

	pIOPort->GPFDAT &= ~(1 << 5);
	pClkPwr->CLKCON |=  (1 << 2);		/* Enter IDLE Mode                                */
	INTERRUPTS_ON();
	while (!b_oalInterruptFlag)	{}
	INTERRUPTS_OFF();
	pClkPwr->CLKCON &= ~(1 << 2);		/* turn-off IDLE bit.	*/

	IDLEflag = FALSE;
	pIOPort->GPFDAT |= (1 << 5);

	if  ( pLCD->LCDSRCPND & 2 ) pLCD->LCDSRCPND = 2;
	if  ( pLCD->LCDINTPND & 2 ) pLCD->LCDINTPND = 2;
	if  ( pINTR->SRCPND & (1 << IRQ_LCD) ) pINTR->SRCPND = ( 1 << IRQ_LCD );
	if  ( pINTR->INTPND & (1 << IRQ_LCD) ) pINTR->INTPND = ( 1 << IRQ_LCD );
	
	pINTR->INTMSK &= ~(1 << IRQ_LCD);		// enable LCD interrupt

	if ( CurrStateIdle == FALSE )
		pINTR->INTMSK |= (1 << IRQ_LCD);		// disable LCD interrupt
#elif (DVS_METHOD == 2)
	pIOPort->GPFDAT &= ~(1 << 5);

	DVS_ON();
	ChangeVoltage(MIDVOLTAGE);

	b_oalInterruptFlag = FALSE;
	
	pIOPort->GPFDAT &= ~(1 << 5);
	pClkPwr->CLKCON |=  (1 << 2);		/* Enter IDLE Mode                                */
	INTERRUPTS_ON();
	while (!b_oalInterruptFlag)	{}
	INTERRUPTS_OFF();
	pClkPwr->CLKCON &= ~(1 << 2);		/* turn-off IDLE bit.	*/

	ChangeVoltage(HIGHVOLTAGE);
	for(i=0;i<VOLTAGEDELAY;i++)
	{
		INREG32(&g_pPortRegs->GPFDAT);  // for loop operation, just read.
	}
	DVS_OFF();
	pIOPort->GPFDAT |= (1 << 5);

#elif (DVS_METHOD == 3)
	if ( CurrentState == Active )	// Active -> Idle
	{
		pIOPort->GPFDAT &= ~(1 << 5);
//		DVS_ON();
//		ChangeVoltage(MIDVOLTAGE);

		b_oalInterruptFlag = FALSE;
		IDLEflag = TRUE;
//		RETAILMSG(1, (TEXT("-I-")));

		pClkPwr->CLKCON |=  (1 << 2);
		INTERRUPTS_ON();
		while (!b_oalInterruptFlag)	{}
		INTERRUPTS_OFF();
		pClkPwr->CLKCON &= ~(1 << 2);
		IDLEflag = FALSE;
		pIOPort->GPFDAT |= (1 << 5);
	}
	else if ( CurrentState == SlowActive )	// Slow Active -> Deep Idle
	{
		pIOPort->GPFDAT &= ~(1 << 5);

		if( pLCD->LCDSRCPND & 2 ) pLCD->LCDSRCPND = 2;
		if( pLCD->LCDINTPND & 2 ) pLCD->LCDINTPND = 2;
		if( pINTR->SRCPND & (1 << IRQ_LCD) ) pINTR->SRCPND = ( 1 << IRQ_LCD );
		if( pINTR->INTPND & (1 << IRQ_LCD) ) pINTR->INTPND = ( 1 << IRQ_LCD );
		pINTR->INTMSK &= ~(1 << IRQ_LCD);		// enable LCD interrupt
		if ( CurrStateIdle == TRUE )	pINTR->INTMSK |= (1 << IRQ_LCD);		// disable LCD interrupt

		b_oalInterruptFlag = FALSE;
		IDLEflag = TRUE;
//		RETAILMSG(1, (TEXT("-D-")));

		pClkPwr->CLKCON |=  (1 << 2);
		INTERRUPTS_ON();
		while (!b_oalInterruptFlag)	{}
		INTERRUPTS_OFF();
		pClkPwr->CLKCON &= ~(1 << 2);
		IDLEflag = FALSE;

		if( pLCD->LCDSRCPND & 2 ) pLCD->LCDSRCPND = 2;
		if( pLCD->LCDINTPND & 2 ) pLCD->LCDINTPND = 2;
		if( pINTR->SRCPND & (1 << IRQ_LCD) ) pINTR->SRCPND = ( 1 << IRQ_LCD );
		if( pINTR->INTPND & (1 << IRQ_LCD) ) pINTR->INTPND = ( 1 << IRQ_LCD );
		pINTR->INTMSK &= ~(1 << IRQ_LCD);		// enable LCD interrupt
		if ( CurrStateIdle == FALSE )	pINTR->INTMSK |= (1 << IRQ_LCD);		// disable LCD interrupt
	}
	else
	{
	}
#endif //DVS_METHOD
#else
	b_oalInterruptFlag = FALSE;

	pIOPort->GPFDAT &= ~(1 << 5);
	pClkPwr->CLKCON |=  (1 << 2);		/* Enter IDLE Mode                              */
	INTERRUPTS_ON();
	while (!b_oalInterruptFlag) {}		/* Wait until S3C2440A enters IDLE mode         */
	INTERRUPTS_OFF();
	pClkPwr->CLKCON &= ~(1 << 2);		/* turn-off IDLE bit.                           */
										/* Any interrupt will wake up from IDLE mode    */
	pIOPort->GPFDAT |= (1 << 5);
#endif //DVS_EN	
}

//------------------------------------------------------------------------------
