#include "Tick.h"


#define TICK_TEMP_VALUE   (65535-( (CLOCK_FREQ/TICK_PRESCALE_VALUE)/TICKS_PER_SECOND))

#if TICK_TEMP_VALUE < 5535
	#error timer set wrong 
#endif

#define TICK_COUNTER_HIGH       ((TICK_TEMP_VALUE >> 8) & 0xff)

#define TICK_COUNTER_LOW        (TICK_TEMP_VALUE & 0xff)

#if (TICK_PRESCALE_VALUE == 1)
    #define TIMER_PRESCALE  (1)
#elif ( TICK_PRESCALE_VALUE == 8 )
    #define TIMER_PRESCALE  (2)
#elif ( TICK_PRESCALE_VALUE == 64 )
    #define TIMER_PRESCALE  (3)
#elif ( TICK_PRESCALE_VALUE == 256 )
    #define TIMER_PRESCALE  (4)
#elif ( TICK_PRESCALE_VALUE == 1024 )
    #define TIMER_PRESCALE  (5)
#else
    #error Invalid _TICK_PRESCALE_VALUE_ specified.
#endif

TICK TickCount;

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
void TickInit(void)
{
	//停止定时器
	TCCR1B = 0x00;
	TickCount = 0;
	
	//计数器初值 
	TCNT1L = TICK_COUNTER_LOW;
    TCNT1H = TICK_COUNTER_HIGH;
	
	TIMSK |= 0b00000000|BM(TOIE1);
	//定时器分频,开始定时
	TCCR1A = 0x00;
	TCCR1B = TIMER_PRESCALE;

}

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
TICK TickGet(void)
{
	TICK temp_count;
	TIMSK &=~ BM(TOIE1);
	temp_count = TickCount;
	TIMSK |= BM(TOIE1);
	
    return temp_count;
}

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
void TickUpdate(void)
{	
	TCNT1H = TICK_COUNTER_HIGH;
    TCNT1L = TICK_COUNTER_LOW;
	TickCount++;
}
	







