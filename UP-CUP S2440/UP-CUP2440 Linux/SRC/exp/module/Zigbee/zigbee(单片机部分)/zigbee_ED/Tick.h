#ifndef TICK_H
#define TICK_H

#include "generic.h"
//时钟频率 8MHz
#define CLOCK_FREQ                      (8000000)    
//滴答数
#define TICKS_PER_SECOND                (50)
//分频
#define TICK_PRESCALE_VALUE             (8)

typedef DWORD TICK;

#define TICK_SECOND             ((TICK)TICKS_PER_SECOND)

extern TICK TickCount;

/*********************************************************************
 * Function:        void TickInit(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          Tick manager is initialized.
 *
 * Side Effects:    None
 *
 * Overview:        Initializes Timer0 as a tick counter.
 *
 * Note:            None
 ********************************************************************/
void TickInit(void);

/*********************************************************************
 * Function:        TICK TickGet(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          Current second value is given
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            None
 ********************************************************************/
TICK TickGet(void);

/*********************************************************************
 * Macro:           TickGetDiff(current, previous)
 *
 * PreCondition:    None
 *
 * Input:           current - Current tick count
 *                  previous - Previous tick count
 *
 * Output:          Different of two
 *
 * Side Effects:    None
 *
 * Overview:        Calculates tick difference.
 *
 * Note:            None
 ********************************************************************/
#define TickGetDiff(a, b)       (a-b)

/*********************************************************************
 * Function:        void TickUpdate(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Internal Tick and Seconds count are updated.
 *
 * Note:            None
 ********************************************************************/
void TickUpdate(void);

#endif
