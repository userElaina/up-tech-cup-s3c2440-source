/***************************************************************************\
	Copyright (c) 2004-2007 threewater@up-tech.com, All rights reserved.
	by threewter	2004.4.26
\***************************************************************************/
	

/***************************************************************************\
    #说明: C库函数等定义
	----------------------------------  Bug  --------------------------------------

	----------------------------------  TODO list  --------------------------------------

	----------------------------------修正--------------------------------------
	2004-5-2	1、添加udelay，微秒级延时函数

	2004-4-26	创建

\***************************************************************************/

#include "../inc/lib.h"

#include "../inc/macro.h"
#include "../inc/Serial.h"
#include "../inc/reg2440.h"

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

extern unsigned int PCLK;
//--------------------------------SYSTEM---------------------------------//

/*************************************************************\
	100微秒级延时函数
	参数time: 表示延时的时间,单位微秒。
	如果time=0，则启用看门狗定时器，
	自动计算时间
\**************************************************************/
void hudelay(int time)
{
	static int delayLoopCount=100;
	int i,adjust=0, curpc;

	if(time==0)
	{
		time=adjust=200;
		delayLoopCount = 400;
		//PCLK/1M,Watch-dog disable,1/64,interrupt disable,reset disable
		rWTCON = ((PCLK/1000000-1)<<8)|(2<<3); 
		rWTDAT = 0xffff;                              //for first update
		rWTCNT = 0xffff;                              //resolution=64us @any PCLK 
		rWTCON = ((PCLK/1000000-1)<<8)|(2<<3)|(1<<5); //Watch-dog timer start
	}

	for(;time>0;time--){
		__asm{mov curpc, pc};

		for(i=0;i<delayLoopCount;i++);

		__asm{sub curpc, pc, curpc};
	}

	if(adjust)
	{
		rWTCON = ((PCLK/1000000-1)<<8)|(2<<3);   //Watch-dog timer stop
		i = 0xffff - rWTCNT;                     // 1count->64us, 200*400 cycle runtime = 64*i us
		delayLoopCount = 8000000/(i*64);         //200*400:64*i=1*x:100 -> x=80000*100/(64*i)   

		printk("Get delay loop count successfully! The value is %d, ", delayLoopCount);
		printk("Maybe %d MIPS\n", curpc*delayLoopCount/400);
	}
}

/************************* SYSTEM PARAMETER ***************************/
//struct struct_system_param system_param;



/************************* UART ****************************/
extern serial_driver_t* serial_drv[];
extern int NumberOfUartDrv;

serial_loop_func_t Getch_loopfunc[]={(serial_loop_func_t)NULL,(serial_loop_func_t)NULL,
					(serial_loop_func_t)NULL, (serial_loop_func_t)NULL};
#define GETCH_LOOPFUNC_NUM		(sizeof(Getch_loopfunc)/sizeof(serial_loop_func_t))

int Uart_Init(int whichUart, int baud)
{
	if(whichUart>=NumberOfUartDrv)
		return FALSE;

	return serial_drv[whichUart]->init(baud);
}



int Uart_SendByte(int whichUart,int data)
{
	if(whichUart>=NumberOfUartDrv)
		return FALSE;

	return serial_drv[whichUart]->write(data);
}

void Uart_SendString(int whichUart, char *pt)
{
    while(*pt){

	if(*pt=='\n')
		Uart_SendByte(whichUart,'\r');

	Uart_SendByte(whichUart,*pt++);
    }
}

void Uart_Printf(int whichUart, char *fmt,...)
{
	va_list ap;
	static char string[256];

	va_start(ap,fmt);
	vsprintf(string,fmt,ap);
	Uart_SendString(whichUart, string);
	va_end(ap);
}
