/***************************************************************************\
	Copyright (c) 2004-2007 threewater@up-tech.com, All rights reserved.
	by threewter	2004.5.12
\***************************************************************************/
/***************************************************************************\
    #说明: C  main 函数，ucos-ii初始化等定义
	----------------------------------  Bug  --------------------------------------

	----------------------------------  TODO list  --------------------------------------

	----------------------------------修正--------------------------------------
	2004-5-12	创建

\***************************************************************************/

//#include"../ucos-ii/includes.h"               /* uC/OS interface */
//#include "../ucos-ii/add/osaddition.h"
//#include "../inc/drivers.h"

//#include "../inc/sys/lib.h"
//#include "../src/gui/gui.h"
#define U8 unsigned char
#include <string.h>
#include <stdio.h>
#define TRUE 	1
#define FALSE 	0
#pragma import(__use_no_semihosting_swi)  // ensure no functions that use semihosting 

#define rUTRSTAT0	(*(volatile unsigned *)0x50000010)
#define rUTRSTAT1	(*(volatile unsigned *)0x50004010)
#define WrUTXH0(ch)	(*(volatile unsigned char *)0x50000020)=(unsigned char)(ch)
#define WrUTXH1(ch)	(*(volatile unsigned char *)0x50004020)=(unsigned char)(ch)
#define RdURXH0()	(*(volatile unsigned char *)0x50000024)
#define RdURXH1()	(*(volatile unsigned char *)0x50004024)
//void Uart_SendByten(int Uartnum, U8 data);
void Uart_SendByten(int,U8);
char Uart_Getchn(char* Revdata, int Uartnum, int timeout);

void ARMTargetInit(void);
void hudelay(int time);
int main(void)
{   //int ndev; 
    char c1[1];
    char err;
	ARMTargetInit();        // do target (uHAL based ARM system) initialisation //

	
	while(1)
	{
       	Uart_SendByten(0,0xa);//换行
		Uart_SendByten(0,0xd);//回车
        	err=Uart_Getchn(c1,0,0);	//从串口采集数据
        	Uart_SendByten(0,c1[0]);	//显示采集的数据
	}
	
	
	
}
	
	void Uart_SendByten(int Uartnum, U8 data)//ok eric rong
{   //int i;
	if(Uartnum==0)
    {
		while(!(rUTRSTAT0 & 0x4)); //Wait until THR is empty.
		
		hudelay(10);
		WrUTXH0(data);
   	}
	else
    {
		while(!(rUTRSTAT1 & 0x4));  //Wait until THR is empty.
		
		hudelay(10);
		WrUTXH1(data);
    }	
}		

   char Uart_Getchn(char* Revdata, int Uartnum, int timeout)
{
	
	if(Uartnum==0){
		while(!(rUTRSTAT0 & 0x1)); //Receive data read
		*Revdata=RdURXH0();
		return TRUE;
	}
	else{
		while(!(rUTRSTAT1 & 0x1));//Receive data read
		*Revdata=RdURXH1();
		return TRUE;
	}
}	

