/***************************************************************************\
	Copyright (c) 2004-2007 threewater@up-tech.com, All rights reserved.
	by threewter	2005.2.24
\***************************************************************************/
	

/***************************************************************************\
    #说明: s3c2440 cpu 时钟驱动
	----------------------------------  Bug  --------------------------------------

	----------------------------------  TODO list  --------------------------------------

	----------------------------------修正--------------------------------------
	2005-2-24	创建

\***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "../inc/reg2440.h"
#include "../inc/serial.h"
//#include "../inc/sys/io.h"
#include "../inc/lib.h"

/* Externl clock frequency used by CPU */
#define FIN	12000000

static __inline unsigned long
cal_bus_clk(unsigned long cpu_clk, unsigned long ratio, int who)
{
	if (!who) {	/* PCLK */
		switch (ratio) {
			case 0:
				return (cpu_clk);
			case 1:
			case 2:
				return (cpu_clk/2);
			case 3:
				return (cpu_clk/4);
			default:
				return 0;
		}
	} else {	/* HCLK */
		switch (ratio) {
			case 0:
			case 1:
				return (cpu_clk);
			case 2:
			case 3:
				return (cpu_clk/2);
			default:
				return 0;
		}
	}
}


/*
 * cpu clock = (((mdiv + 8) * FIN) / ((pdiv + 2) * (1 << sdiv)))
 *  FIN = Input Frequency (to CPU)
 */
unsigned long s3c2440_get_cpu_clk(void)
{
	unsigned long val = rMPLLCON;
	return (((GET_MDIV(val) + 8) * FIN) / \
		((GET_PDIV(val) + 2) * (1 << GET_SDIV(val))));
}

unsigned long s3c2440_get_bus_clk(int who)
{
	unsigned long cpu_clk = s3c2440_get_cpu_clk();
	
	//unsigned long ratio = rCLKDIVN;
	//in 2440 rCLKDIVN =3;IN 2440 rCLKDIVN = 5;rCLKDIVN was inited in uboot
	// here if ratio = 5,it run bad-------by memo,uptech
     unsigned long ratio = 3;
	
	return (cal_bus_clk(cpu_clk, ratio, who));
}

#define MEGA	(1000 * 1000)
int s3c2440_cpu_init(void)
{
	FCLK = s3c2440_get_cpu_clk();
	HCLK = s3c2440_get_bus_clk(GET_HCLK);
	PCLK = s3c2440_get_bus_clk(GET_PCLK);

	printk("CPU clock = %d.%03d Mhz,", FCLK / MEGA, FCLK % MEGA);
	printk(" HCLK = %d.%03d Mhz,", HCLK / MEGA, HCLK % MEGA);
	printk(" PCLK = %d.%03d Mhz\n", PCLK / MEGA, PCLK % MEGA);
	return 0;
}

