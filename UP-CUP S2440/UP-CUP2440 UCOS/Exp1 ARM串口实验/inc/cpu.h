/***************************************************************************\
	Copyright (c) 2004-2007 threewater@up-tech.com, All rights reserved.
	by threewter	2005.6.24
\***************************************************************************/
	

/***************************************************************************\
    #说明: s3c2440 寄存器定义
	----------------------------------  Bug  --------------------------------------

	----------------------------------  TODO list  --------------------------------------

	----------------------------------修正--------------------------------------
	2005-5-24	创建

\***************************************************************************/

#include "inc/bitfield.h"
#ifndef _CPU_S3C2440_H_
#define _CPU_S3C2440_H_

#define GET_PCLK	0
#define GET_HCLK	1

#define GET_MDIV(x)	FExtr(x, fPLL_MDIV)
#define GET_PDIV(x)	FExtr(x, fPLL_PDIV)
#define GET_SDIV(x)	FExtr(x, fPLL_SDIV)

unsigned long s3c2440_get_cpu_clk(void);
unsigned long s3c2440_get_bus_clk(int);
int s3c2440_cpu_init(void);

#endif /* _CPU_S3C2440_H_ */

