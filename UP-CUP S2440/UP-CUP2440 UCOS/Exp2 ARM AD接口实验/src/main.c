
//#include"../ucos-ii/includes.h"               /* uC/OS interface */
//#include "../ucos-ii/add/osaddition.h"
#include "../inc/drivers.h"

#include "../inc/lib.h"
//#include "../src/gui/gui.h"
#include <string.h>
#include <stdio.h>

#pragma import(__use_no_semihosting_swi)  // ensure no functions that use semihosting 
#define ADCCON_FLAG		(0x1<<15)

#define ADCCON_ENABLE_START_BYREAD	(0x1<<1)

#define rADCCON		(*(volatile unsigned *)0x58000000)
#define rADCDAT0	(*(volatile unsigned *)0x5800000C)
#define PRSCVL (49<<6)
#define ADCCON_ENABLE_START (0x1)
#define STDBM (0x0<<2)
#define PRSCEN (0x1<<14)
void  ARMTargetInit(void); 
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
	int i,j;
	float d;
	
    ARMTargetInit();        // do target (uHAL based ARM system) initialisation //
	
	init_ADdevice();
	//printf("\n");
	Uart_Printf(0,"\n");

   while(1)
	{
		for(i=0; i<=2; i++)
		{//采样0~3路A/D值
		    
		    for(j=0;j<=1;j++)
			{d=GetADresult(i)*3.3/1023; // 数据 采集，处理
			 
			}
			
			
            Uart_Printf(0,"a%d=%f\t",i,d);
			//printf("a%d=%f\t",i,d);
			
			hudelay(1000);
			
			
		}	
		//printf("\r");
		 Uart_Printf(0,"\r");
		
	}

	return 0;
}
	
	

