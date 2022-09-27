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
//  Module: timer.c           
//
//  Interface to OAL timer services.
//
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#include <oal.h>
#include <s3c2440a.h>
#include <bsp_cfg.h>

#ifdef DVS_EN
DWORD dwCurrentidle;

DWORD dwPrevTotalTick1000 = 0;
DWORD dwPrevTotalTick100 = 0;

DWORD dwPrevIdleTick1000 = 0;
DWORD dwPrevIdleTick100 = 0;

DWORD dwPercentIdle1000 = 0;
DWORD dwPercentIdle100 = 0;

extern	volatile int CurrentState;

#endif

//------------------------------------------------------------------------------
// Local Variables 

static S3C2440A_PWM_REG *g_pPWMRegs = NULL;
UINT32	g_idleMSec;
//------------------------------------------------------------------------------
//
//  Global:  g_oalLastSysIntr
//
//  This global variable is set by fake version of interrupt/timer handler
//  to last SYSINTR value.
//
volatile UINT32 g_oalLastSysIntr; 

//------------------------------------------------------------------------------
//
//  Function: OALTimerInit
//
//  This function is typically called from the OEMInit to initialize
//  Windows CE system timer. The tickMSec parameter determine timer
//  period in milliseconds. On most platform timer period will be
//  1 ms, but it can be usefull to use higher value for some
//  specific (low-power) devices.
//
//  Implementation for s3c2440a is using timer 4 as system timer.
//
BOOL OALTimerInit(
    UINT32 msecPerSysTick, UINT32 countsPerMSec, UINT32 countsMargin
) {
    BOOL rc = FALSE;
    UINT32 countsPerSysTick;
    UINT32 sysIntr, irq;
    UINT32 tcon;

    OALMSG(OAL_TIMER&&OAL_FUNC, (
        L"+OALTimerInit( %d, %d, %d )\r\n", 
        msecPerSysTick, countsPerMSec, countsMargin
    ));

    // Validate Input parameters
    countsPerSysTick = countsPerMSec * msecPerSysTick;
    if (
        msecPerSysTick < 1 || msecPerSysTick > 1000 ||
        countsPerSysTick < 1 || countsPerSysTick > 65535
    ) {
        OALMSG(OAL_ERROR, (
            L"ERROR: OALTimerInit: System tick period out of range..."
        ));
        goto cleanUp;
    }

    // Initialize timer state global variable    
    g_oalTimer.msecPerSysTick = msecPerSysTick;
    g_oalTimer.countsPerMSec = countsPerMSec;
    g_oalTimer.countsMargin = countsMargin;
    g_oalTimer.countsPerSysTick = countsPerSysTick;
    g_oalTimer.curCounts = 0;
    g_oalTimer.maxPeriodMSec = 0xFFFF/g_oalTimer.countsPerMSec;

	g_oalTimer.actualMSecPerSysTick = msecPerSysTick;
	g_oalTimer.actualCountsPerSysTick = countsPerSysTick;

    // Set kernel exported globals to initial values
    idleconv = countsPerMSec;
    curridlehigh = 0;
    curridlelow = 0;

    // Initialize high resolution timer function pointers
    pQueryPerformanceFrequency = OALTimerQueryPerformanceFrequency;
    pQueryPerformanceCounter = OALTimerQueryPerformanceCounter;

    // Create SYSINTR for timer
    irq = IRQ_TIMER4;
    sysIntr = OALIntrRequestSysIntr(1, &irq, OAL_INTR_FORCE_STATIC);

    // Hardware Setup
    g_pPWMRegs = (S3C2440A_PWM_REG*)OALPAtoUA(S3C2440A_BASE_REG_PA_PWM);

    // Set prescaler 1 to 1 
    OUTREG32(&g_pPWMRegs->TCFG0, INREG32(&g_pPWMRegs->TCFG0) & ~0x0000FF00);
    OUTREG32(&g_pPWMRegs->TCFG0, INREG32(&g_pPWMRegs->TCFG0) | PRESCALER <<8);
    // Select MUX input 1/2
    OUTREG32(&g_pPWMRegs->TCFG1, INREG32(&g_pPWMRegs->TCFG1) & ~(0xF << 16));
#if( SYS_TIMER_DIVIDER == D2 )
    OUTREG32(&g_pPWMRegs->TCFG1, INREG32(&g_pPWMRegs->TCFG1) | (D1_2 << 16));
#elif ( SYS_TIMER_DIVIDER == D4 )
    OUTREG32(&g_pPWMRegs->TCFG1, INREG32(&g_pPWMRegs->TCFG1) | (D1_4 << 16));
#elif ( SYS_TIMER_DIVIDER == D8 )
    OUTREG32(&g_pPWMRegs->TCFG1, INREG32(&g_pPWMRegs->TCFG1) | (D1_8 << 16));
#elif ( SYS_TIMER_DIVIDER == D16 )
    OUTREG32(&g_pPWMRegs->TCFG1, INREG32(&g_pPWMRegs->TCFG1) | (D1_16 << 16));
#endif
    // Set timer register
    OUTREG32(&g_pPWMRegs->TCNTB4, g_oalTimer.countsPerSysTick);

    // Start timer in auto reload mode
    tcon = INREG32(&g_pPWMRegs->TCON) & ~(0x0F << 20);
    OUTREG32(&g_pPWMRegs->TCON, tcon | (0x2 << 20) );
    OUTREG32(&g_pPWMRegs->TCON, tcon | (0x1 << 20) );

    // Enable System Tick interrupt
    if (!OEMInterruptEnable(sysIntr, NULL, 0)) {
        OALMSG(OAL_ERROR, (
            L"ERROR: OALTimerInit: Interrupt enable for system timer failed"
        ));
        goto cleanUp;

    }

//    
// Define ENABLE_WATCH_DOG to enable watchdog timer support.
// NOTE: When watchdog is enabled, the device will reset itself if watchdog timer is not refreshed within ~4.5 second.
//       Therefore it should not be enabled when kernel debugger is connected, as the watchdog timer will not be refreshed.
//
#ifdef ENABLE_WATCH_DOG
    {
        extern void SMDKInitWatchDogTimer (void);
        SMDKInitWatchDogTimer ();
    }
#endif

    // Done        
    rc = TRUE;
    
cleanUp:
    OALMSG(OAL_TIMER && OAL_FUNC, (L"-OALTimerInit(rc = %d)\r\n", rc));
    return rc;
}

#ifdef DVS_EN
#if (DVS_METHOD == 3)
VOID ChangeSystemStateDVS()
{
	unsigned int dwCurrentMSec, dwCurrentIdleSec;
	unsigned int PercentIdle;
	volatile S3C2440A_LCD_REG *s2440LCD = (S3C2440A_LCD_REG*)OALPAtoVA(S3C2440A_BASE_REG_PA_LCD, FALSE);
	volatile S3C2440A_INTR_REG *s2440INT = (S3C2440A_INTR_REG*)OALPAtoVA(S3C2440A_BASE_REG_PA_INTR, FALSE);
	volatile S3C2440A_IOPORT_REG *s2440IOP = (S3C2440A_IOPORT_REG*)OALPAtoVA(S3C2440A_BASE_REG_PA_IOPORT, FALSE);
	unsigned int i;

	dwCurrentMSec = CurMSec;
	dwCurrentIdleSec = dwCurrentidle;

	if ( dwCurrentMSec - dwPrevTotalTick100 > 100 )
	{
		dwPercentIdle100 = ((100*(dwCurrentIdleSec - dwPrevIdleTick100)) / (dwCurrentMSec - dwPrevTotalTick100));
		dwPrevTotalTick100 = dwCurrentMSec;
		dwPrevIdleTick100 = dwCurrentIdleSec;
		if ( dwPercentIdle100 < 20 )
		{
			if ( CurrentState == SlowActive )		// Change state from SlowActive to Active...
			{
				CurrentState = Active;

				if ( GetCurrentVoltage() != HIGHVOLTAGE )		// State is HCLK is half and DVS is on...
				{
					// DVS OFF...
					ChangeVoltage(HIGHVOLTAGE);
					for(i=0;i<VOLTAGEDELAY;i++)
					{
						s2440IOP->GPFDAT; // for loop operation, just read.
					}
					DVS_OFF();

					// LCD Interrupt Enable for HCLK recover...
					if  ( s2440LCD->LCDSRCPND & 2 ) s2440LCD->LCDSRCPND = 2;
					if  ( s2440LCD->LCDINTPND & 2 ) s2440LCD->LCDINTPND = 2;
					if  ( s2440INT->SRCPND & (1 << IRQ_LCD)) s2440INT->SRCPND = (1 << IRQ_LCD);
					if  ( s2440INT->INTPND & (1 << IRQ_LCD)) s2440INT->INTPND = (1 << IRQ_LCD);
					s2440INT->INTMSK &= ~(1 << IRQ_LCD);		// enable LCD interrupt
				}
				else		// State is same with Active... so just disable the LCD interrupt...
				{
					// LCD Interrupt Disable...
					if  ( s2440LCD->LCDSRCPND & 2 ) s2440LCD->LCDSRCPND = 2;
					if  ( s2440LCD->LCDINTPND & 2 ) s2440LCD->LCDINTPND = 2;
					if  ( s2440INT->SRCPND & (1 << IRQ_LCD)) s2440INT->SRCPND = (1 << IRQ_LCD);
					if  ( s2440INT->INTPND & (1 << IRQ_LCD)) s2440INT->INTPND = (1 << IRQ_LCD);
					s2440INT->INTMSK |= (1 << IRQ_LCD);		// disable LCD interrupt
				}
				RETAILMSG(1, (TEXT("-A-")));
			}
		}
	}
	if ( dwCurrentMSec - dwPrevTotalTick1000 > 1000 )
	{
		dwPercentIdle1000 = ((100*(dwCurrentIdleSec - dwPrevIdleTick1000)) / (dwCurrentMSec - dwPrevTotalTick1000));
		dwPrevTotalTick1000 = dwCurrentMSec;
		dwPrevIdleTick1000 = dwCurrentIdleSec;
		if ( dwPercentIdle1000 > 70 )
		{
			if ( CurrentState == Active )
			{
				volatile S3C2440A_LCD_REG *s2440LCD = (S3C2440A_LCD_REG*)OALPAtoVA(S3C2440A_BASE_REG_PA_LCD, FALSE);
				volatile S3C2440A_INTR_REG *s2440INT = (S3C2440A_INTR_REG*)OALPAtoVA(S3C2440A_BASE_REG_PA_INTR, FALSE);

				CurrentState = SlowActive;

				if  ( s2440LCD->LCDSRCPND & 2 ) s2440LCD->LCDSRCPND = 2;
				if  ( s2440LCD->LCDINTPND & 2 ) s2440LCD->LCDINTPND = 2;
				if  ( s2440INT->SRCPND & (1 << IRQ_LCD)) s2440INT->SRCPND = (1 << IRQ_LCD);
				if  ( s2440INT->INTPND & (1 << IRQ_LCD)) s2440INT->INTPND = (1 << IRQ_LCD);
				s2440INT->INTMSK &= ~(1 << IRQ_LCD);		// enable LCD interrupt

				RETAILMSG(1, (TEXT("-S-")));
			}
		}
	}
}
#endif
#endif



//------------------------------------------------------------------------------
//
//  Function: OALTimerIntrHandler
//
//  This function implement timer interrupt handler. It is called from common
//  ARM interrupt handler.
//
UINT32 OALTimerIntrHandler()
{
    volatile S3C2440A_INTR_REG *pIntr = (S3C2440A_INTR_REG*)OALPAtoVA(S3C2440A_BASE_REG_PA_INTR, FALSE);
    UINT32 sysIntr = SYSINTR_NOP;

    // Update high resolution counter
    g_oalTimer.curCounts += g_oalTimer.countsPerSysTick;
                             
    // Update the millisecond counter
    CurMSec += g_idleMSec;

	OALTimerUpdate(g_oalTimer.countsPerSysTick, g_oalTimer.countsMargin);
	g_idleMSec = g_oalTimer.msecPerSysTick;

    // Reschedule?
    if ((int)(CurMSec - dwReschedTime) >= 0) sysIntr = SYSINTR_RESCHED;

#ifdef OAL_ILTIMING
    if (g_oalILT.active) {
        if (--g_oalILT.counter == 0) {
            sysIntr = SYSINTR_TIMING;
            g_oalILT.counter = g_oalILT.counterSet;
            g_oalILT.isrTime2 = OALTimerCountsSinceSysTick();
        }
    }
#endif

#ifdef DVS_EN
#if (DVS_METHOD == 3)
	ChangeSystemStateDVS();
#endif
#endif
    return sysIntr;
}


//------------------------------------------------------------------------------
//
//  Function: OALTimerCountsSinceSysTick
//
//  This function return count of hi res ticks since system tick.
//
//  Timer 4 counts down, so we should substract actual value from 
//  system tick period.
//

INT32 OALTimerCountsSinceSysTick()
{
	return (INREG32(&g_pPWMRegs->TCNTB4) - INREG32(&g_pPWMRegs->TCNTO4));
}

//------------------------------------------------------------------------------
//
//  Function: OALTimerUpdate
//
//  This function is called to change length of actual system timer period.
//  If end of actual period is closer than margin period isn't changed (so
//  original period elapse). Function returns time which already expires
//  in new period length units. If end of new period is closer to actual time
//  than margin period end is shifted by margin (but next period should fix
//  this shift - this is reason why OALTimerRecharge doesn't read back 
//  compare register and it uses saved value instead).
//
UINT32 OALTimerUpdate(UINT32 period, UINT32 margin)
{
	UINT32 tcon, ret;
	
	ret = OALTimerCountsSinceSysTick();

	OUTREG32(&g_pPWMRegs->TCNTB4, period);
    tcon = INREG32(&g_pPWMRegs->TCON) & ~(0x0F << 20);
    OUTREG32(&g_pPWMRegs->TCON, tcon | (0x2 << 20) );
    OUTREG32(&g_pPWMRegs->TCON, tcon | (0x1 << 20) );

	return (ret);
}
//------------------------------------------------------------------------------
//
//  Function:   OEMIdle
//
//  This Idle function implements a busy idle. It is intend to be used only
//  in development (when CPU doesn't support idle mode it is better to stub
//  OEMIdle function instead use this busy loop). The busy wait is cleared by
//  an interrupt from interrupt handler setting the g_oalLastSysIntr.
//

//
void OEMIdle(DWORD idleParam)
{
    UINT32 baseMSec, idleMSec, idleSysTicks;
    INT32 usedCounts, idleCounts;
    ULARGE_INTEGER idle;

    // Get current system timer counter
    baseMSec = CurMSec;

    // Compute the remaining idle time
    idleMSec = dwReschedTime - baseMSec;
    
    // Idle time has expired - we need to return
    if ((INT32)idleMSec <= 0) return;

	// Limit the maximum idle time to what is supported.  
	// Counter size is the limiting parameter.  When kernel 
	// profiler or interrupt latency timing is active it is set
	// to one system tick.
	if (idleMSec > g_oalTimer.maxPeriodMSec) {
		idleMSec = g_oalTimer.maxPeriodMSec;
	}

	// We can wait only full systick
	idleSysTicks = idleMSec/g_oalTimer.msecPerSysTick;

	// This is idle time in hi-res ticks
	idleCounts = idleSysTicks * g_oalTimer.countsPerSysTick;

    // Find how many hi-res ticks was already used
    usedCounts = OALTimerCountsSinceSysTick();

	if (usedCounts == g_oalTimer.countsPerSysTick)
	{
		return;
	}

    // Prolong beat period to idle time -- don't do it idle time isn't
    // longer than one system tick. Even if OALTimerExtendSysTick function
    // should accept this value it can cause problems if kernel profiler
    // or interrupt latency timing is active.
    if (idleSysTicks > 1) {
        // Extend timer period
        OALTimerUpdate(idleCounts-usedCounts, g_oalTimer.countsMargin);
        // Update value for timer interrupt which wakeup from idle
        g_oalTimer.actualMSecPerSysTick = idleMSec;
        g_oalTimer.actualCountsPerSysTick = idleCounts;
    }

	g_idleMSec = idleMSec;

    // Move SoC/CPU to idle mode
    OALCPUIdle();


    // Return system tick period back to original. Don't call when idle
    // time was one system tick. See comment above.
    if (idleSysTicks > 1) {
        // If there wasn't timer interrupt we have to update CurMSec&curCounts
        if (CurMSec == baseMSec) {
			// Return system tick period back to original
			idleCounts = OALTimerUpdate(g_oalTimer.countsPerSysTick, g_oalTimer.countsMargin);
			idleSysTicks = idleCounts/g_oalTimer.countsPerSysTick;

			g_idleMSec = g_oalTimer.msecPerSysTick;

            // Restore original values
			g_oalTimer.actualMSecPerSysTick = g_oalTimer.msecPerSysTick;		// 1
			g_oalTimer.actualCountsPerSysTick = g_oalTimer.countsPerSysTick;	// 1004

            // Fix system tick counters & idle counter
			CurMSec += idleSysTicks * g_oalTimer.actualMSecPerSysTick;

			//idleCounts = idleSysTicks * g_oalTimer.actualCountsPerSysTick;
			g_oalTimer.curCounts += idleCounts;
			idleCounts += OALTimerCountsSinceSysTick();
			usedCounts = 0;
        }
    } else {
        if (CurMSec == baseMSec) {
//			g_oalTimer.curCounts += idleCounts;
			// Update actual idle counts, if there wasn't timer interrupt
			idleCounts = OALTimerCountsSinceSysTick();
			usedCounts = 0;
        }
    }

    // Get real idle value. If result is negative we didn't idle at all.
    idleCounts -= usedCounts;

    if (idleCounts < 0) idleCounts = 0;

    // Update idle counters
    idle.LowPart = curridlelow;
    idle.HighPart = curridlehigh;
    idle.QuadPart += idleCounts;
    curridlelow  = idle.LowPart;
    curridlehigh = idle.HighPart;

#ifdef DVS_EN
	dwCurrentidle = (idle.QuadPart/idleconv);
#endif
}
