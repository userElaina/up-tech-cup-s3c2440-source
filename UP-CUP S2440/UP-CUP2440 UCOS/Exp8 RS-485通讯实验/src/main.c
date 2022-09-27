
//#include"../ucos-ii/includes.h"               /* uC/OS interface */
//#include "../ucos-ii/add/osaddition.h"
#include "../inc/drivers.h"

#include "inc/lib.h"
//#include "../src/gui/gui.h"
#include <string.h>
#include <stdio.h>
#include "inc/macro.h"


#pragma import(__use_no_semihosting_swi)  // ensure no functions that use semihosting 


#define WrUTXH0(ch)	(*(volatile unsigned char *)0x50000020)=(unsigned char)(ch)
#define WrUTXH1(ch)	(*(volatile unsigned char *)0x50004020)=(unsigned char)(ch)

#define GPHCON	(*(volatile unsigned char *)0x56000070)
#define GPHDAT	(*(volatile unsigned char *)0x56000074)


int main(void)
{
	
	 char c1[1];
    char err;
    ARMTargetInit();        // do target (uHAL based ARM system) initialisation //
	 GPHCON=(GPHCON&0x3ffff3)|(0x01<<2);   //配置成输出
	 
	Uart_Printf(0,"\n");
	Uart_Printf(0,"please input 1 or 2, 1 is master,2 is slave");
	err=Uart_Getchn(c1,0,0);
	//Uart_Printf(0,"err is: %s",err);
    if(*c1=='1'){
      Uart_Printf(0,"\nyou have input 1");
      while(1)
	  {   GPHDAT|=(1<<1);
		  Uart_SendByte(0,0xa);//换行
		  Uart_SendByte(0,0xd);//回车
		   
        	err=Uart_Getchn(c1,0,0);	//从串口采集数据
        	
        	Uart_SendByte(0,c1[0]);	//显示采集的数据
        	Uart_SendByte(2,c1[0]);
		
	  }
	}
	
	 else if(*c1=='2'){
	    Uart_Printf(0,"\nyou have input 2");
          while(1)
	      {   GPHDAT&=~(1<<1);
		      Uart_SendByte(0,0xa);//换行
		      Uart_SendByte(0,0xd);//回车
		   
        	  err=Uart_Getchn(c1,2,0);	//从串口采集数据 // 485串口
        	  
        	  Uart_SendByte(0,c1[0]);	//显示采集的数据
        	  //Uart_SendByte(2,c1[0]);
		
	      }
	}
	return 0;
}
	
 
	
