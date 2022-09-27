#include "../inc/drivers.h"
#include "../inc/lib.h"
#include <string.h>
#include <stdio.h>
#include "inc/max504.h"

#pragma import(__use_no_semihosting_swi)  // ensure no functions that use semihosting 

void  ARMTargetInit(void); 
int main(void)
{   
	
	int flag=0;
	ARMTargetInit();	//开发版初始化
	//SPI_init();
	setgpio();  //   by   sprife 
	Uart_Printf(0,"\nDA device is enabled.....................................\n");
	//while(1)
	//{
		//if(flag==0)
		//{
		//	Max504_SetDA(0);//输出0v
		//	flag=1;
		//}
		//else
		//{
		
		Max504_SetDA(1023);//输出满量程
	    Uart_Printf(0,"The output voltage is 4.07v\n");
        while (1)
        {}
		
		
		//	flag=0;
		//}
		hudelay(1000);
		//DA_Test();
	//}
	return 0;
	
	
	
}
	
	

