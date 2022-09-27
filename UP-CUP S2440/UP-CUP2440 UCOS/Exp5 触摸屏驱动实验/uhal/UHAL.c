/***************************************************************************\
	Copyright (c) 2004-2007 threewater@up-tech.com, All rights reserved.
	by threewter	2004.5.12
\***************************************************************************/
/***************************************************************************\
    #˵��: C  main ������ucos-ii��ʼ���ȶ���
	----------------------------------  Bug  --------------------------------------

	----------------------------------  TODO list  --------------------------------------

	----------------------------------����--------------------------------------

	2004-5-12	��ֲ

\***************************************************************************/


#include "../inc/lib.h"
#include "../inc/macro.h"
#include "../inc/reg2440.h"
#include "uhal/UHAL.h"
#include "isr.h"
#include <stdio.h>

#define _TICK		OS_TICKS_PER_SEC

void BreakPoint(void)
{
	printf("!!!Enter break point.\n");

	for(;;);
}//BreakPoint

void OutDebug(unsigned int num)
{
	printf("\n***STACK***:%d\r\n", num);
}//OutDebug

//Define pre & post-process routines for Interrupt.
void uHALir_DefineIRQ(void *is, void *iq, void *n)
{
}//uHALir_DefineIRQ

static int I_COUNT=0;

void Enter_UNDEF(void)
{
	printf("!!!Enter UNDEFINED. %d\r\n", I_COUNT++);

	for(;;);
}//BreakPoint


void Enter_SWI(void)
{
	printf("!!!Enter SWI. %d\r\n", I_COUNT++);	

	for(;;);
}

void Enter_PABORT(void)
{
	printf("!!!Enter Prefetch ABORT %d\r\n", I_COUNT++);

	for(;;);
}

void Enter_DABORT(void)
{
	printf("!!!Enter Data ABORT %d\r\n", I_COUNT++);

	for(;;);
}

void Enter_FIQ(void)
{
	printf("!!!Enter FIQ. %d\r\n", I_COUNT++);

	for(;;);
}

unsigned int PCLK=0, HCLK=0, FCLK=0;

void ARMTargetInit(void)
{
	
	s3c2440_cpu_init();

	Uart_Init(0,115200);
	

}


