#include "../inc/drivers.h"
#include "../inc/lib.h"
#include <string.h>
#include <stdio.h>
#include "inc/MotorCtrl.h"

#pragma import(__use_no_semihosting_swi)  // ensure no functions that use semihosting 

#define MCLK (50700000)
#define MOTOR_SEVER_FRE		1000		//20kHz
#define MOTOR_CONT				(MCLK/2/2/MOTOR_SEVER_FRE)
#define MOTOR_MID				(MOTOR_CONT/2)

#define ADCCON_FLAG		(0x1<<15)

#define ADCCON_ENABLE_START_BYREAD	(0x1<<1)

#define rADCCON		(*(volatile unsigned *)0x58000000)
#define rADCDAT0	(*(volatile unsigned *)0x5800000C)
#define PRSCVL (49<<6)
#define ADCCON_ENABLE_START (0x1)
#define STDBM (0x0<<2)
#define PRSCEN (0x1<<14)

#define rADCCON  (*(volatile unsigned *)0x58000000)
#define rADCDAT0  (*(volatile unsigned *)0x5800000C)
#define rUTRSTAT0  (*(volatile unsigned *)0x50000010)
//#define rUTRSTAT0  (*(volatile unsigned *)0x50000024) 
#define RdURXH0()	(*(volatile unsigned char *)0x50000024)

void init_ADdevice()
{//初始化
	
	
	rADCCON=(PRSCVL|ADCCON_ENABLE_START|STDBM|PRSCEN);
}

int GetADresult(int channel)
{   
	
	
	  rADCCON=ADCCON_ENABLE_START_BYREAD|(channel<<3)|PRSCEN|PRSCVL;
	
	hudelay(10);
	while(!(rADCCON&ADCCON_FLAG));//转换结束
	
	return (0x3ff&rADCDAT0);//返回采样值
	
}

int main(void)
{
	int i,j,ADData,lastADData,count=0;
	char Revdata[10];
	
	ARMTargetInit();	//开发版初始化
	init_MotorPort();
	init_ADdevice();

	for(;;)
	{
	begin:
		Uart_Printf(0,"\nBegin control DC motor.\t\tPress any key to stop.\n");
		for(;;)
		{   for(i=0;i<2;i++)
			ADData=GetADresult(0);
			//Uart_Printf(0,"addata=%d",ADData);
			hudelay(10);
			SetPWM((ADData-512)*MOTOR_CONT/1024);
			hudelay(10);
			
			if((rUTRSTAT0 & 0x1))	//有输入，则跳出
			{
				*Revdata=RdURXH0();
				goto next;
			}
		}

	next:
		SetPWM(0);
		Uart_Printf(0,"\nDC motor is stoped\t\tPress any key to control DC motor.\n");
		lastADData=GetADresult(0);
		hudelay(10);
		
		for(;;)
		{
	loop:	if((rUTRSTAT0 & 0x1))	//有输入，则返回
			{
				*Revdata=RdURXH0();
				goto begin;
			}

		}
	}


 	return 0;
}

	
	

