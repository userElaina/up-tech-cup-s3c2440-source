#include "../inc/drivers.h"
#include "inc/lib.h"
#include <string.h>
#include <stdio.h>
#include "inc/macro.h"
#include "inc/infrared.h"

#pragma import(__use_no_semihosting_swi)  // ensure no functions that use semihosting 
#define UCON2	(*(volatile unsigned char *)0x50008004)
void ARMTargetInit(void);
int main(void)
{
	int i,j,k=0,z=0;
	char transmit[8]={'1','2','3','4','5','6','7','8'};
	char receive[1],pass[2];
	char  store[100][8];
    char err,judge[1];
    ARMTargetInit();        // do target (uHAL based ARM system) initialisation //
   
 while(1){	
	Uart_Printf(0,"\n");
	Uart_Printf(0,"请输入1,2,3或4, 1是单个字符发送, 2是单个字符接收,3是数组发送,4是数组接收");
	err=Uart_Getchn(judge,0,0);
	switch(*judge){
	
	case '1':{
       Uart_Printf(0,"\n当前输入是 1");    
       infraredT();          //发送使能
      while(1)
	  {   
		  Uart_SendByte(0,0xa);//换行
		  Uart_SendByte(0,0xd);//回车		   
          err=Uart_Getchn(pass,0,0);	//从串口采集数据
          Uart_SendByte(2,pass[0]);          
          if(pass[0]==27)  //ESC exit
	        break;
          Uart_SendByte(0,pass[0]);	//显示采集的数据		 
	  }
	  hudelay(1000);
	  UCON2=UCON2&0x7f3;             //禁止发送  
	  break;
	}
	
	 case '2':{
	    Uart_Printf(0,"\n当前输入是 2");
	    infraredR();         //接收使能      	     
          while(1)
	      { 
		      Uart_SendByte(0,0xa);//换行
		      Uart_SendByte(0,0xd);//回车
	     
        	  err=Uart_Getchn(pass,2,0);	//从串口采集数据
        	  if(pass[0]==27)  //ESC exit
                break;
        	  Uart_SendByte(0,pass[0]);	//显示采集的数据
	      }
	      
	      hudelay(1000);	      
	      UCON2=UCON2&0x7fc;  //禁止接收
	      break;
	}
	
    case '3':{
      Uart_Printf(0,"\n当前输入是 3");
      infraredT();              //发送使能
            Uart_Printf(0,"\n");
        	for(i=0;i<100;i++)
        	  for(j=0;j<8;j++)
             	Uart_SendByte(2,transmit[j]);            
        	Uart_Printf(0,"发送完毕");
	  hudelay(1000);
      UCON2=UCON2&0x7f3;         //禁止发送
      break;
        	
	}
	
	 case '4':{
	    Uart_Printf(0,"\n当前收入是 4");	    
        infraredR();           //接收使能
        	  Uart_Printf(0,"\n");
        	  z=k=0;
		      for(i=0;i<100;i++)
		        for(j=0;j<8;j++)
        	    {err=Uart_Getchn(receive,2,0);	//从串口采集数据
        	     z++;
        	     store[i][j]=*receive;
        	     if(store[i][j]!=transmit[j])
        	     k++;
        	     Uart_SendByte(0,store[i][j]);	    //显示采集的数据
        	
        	    }        	  
        	  Uart_Printf(0,"\n错误数据个数是:%d",k);
        	  Uart_Printf(0,"\n接收数据个数是:%d",z);
	          hudelay(1000);
        	  UCON2=UCON2&0x7fc;  //禁止接收
        	  break;
       	}
	}//end switch
	
 }//end while
}
	
 
	
