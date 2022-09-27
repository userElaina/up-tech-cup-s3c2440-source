
#include <stdio.h>

#include "../inc/reg2440.h"


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

//unsigned int PCLK=0, HCLK=0, FCLK=0;

void ARMTargetInit(void)
{
	
	s3c2440_cpu_init();


}


