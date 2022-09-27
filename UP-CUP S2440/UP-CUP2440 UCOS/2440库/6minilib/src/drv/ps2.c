/***************************************************************************\
	Copyright (c) 2004-2007 threewater@up-tech.com, All rights reserved.
	by threewter	2004.4.26
\***************************************************************************/
	

/***************************************************************************\
    #说明: PS/2 接口驱动
	----------------------------------  Bug  --------------------------------------

	----------------------------------  TODO list  --------------------------------------

	----------------------------------修正--------------------------------------
	2004-5-9	创建

\***************************************************************************/
#include "../inc/drv/reg2410.h"
#include "../inc/drv/ps2.h"
#include "../inc/sys/lib.h"
#include <stdio.h>

void GetPS2_poll(void)
{
	if(rPSINTR & PSINTR_RX)
	{
		printf("data=%02X\n", rPSDATA);
	}
}

void PS2_init(void)
{
	rPSTDLO = 0xff;
	rPSTPRI = 0xfc;
	rPSTXMT = 0x0f;
	rPSTREC = 0x0f;
	rPSPWDN |= 0x01;

	rPSCONF = PSCONF_LCE|PSCONF_ENABlE;	// enable line control detection, Enable PS/2 controller
}

int PS2_Test(int argc, char *argv[])
{
	int index;

	PS2_init();
	printf("receive from PS/2\n");

	index=Set_UartLoopFunc(GetPS2_poll);
	getchar();
	Clear_UartLoopFunc(index);

	return OK;
}
