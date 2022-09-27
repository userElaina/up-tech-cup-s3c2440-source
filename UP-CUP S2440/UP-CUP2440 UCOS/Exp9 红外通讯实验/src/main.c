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
	Uart_Printf(0,"������1,2,3��4, 1�ǵ����ַ�����, 2�ǵ����ַ�����,3�����鷢��,4���������");
	err=Uart_Getchn(judge,0,0);
	switch(*judge){
	
	case '1':{
       Uart_Printf(0,"\n��ǰ������ 1");    
       infraredT();          //����ʹ��
      while(1)
	  {   
		  Uart_SendByte(0,0xa);//����
		  Uart_SendByte(0,0xd);//�س�		   
          err=Uart_Getchn(pass,0,0);	//�Ӵ��ڲɼ�����
          Uart_SendByte(2,pass[0]);          
          if(pass[0]==27)  //ESC exit
	        break;
          Uart_SendByte(0,pass[0]);	//��ʾ�ɼ�������		 
	  }
	  hudelay(1000);
	  UCON2=UCON2&0x7f3;             //��ֹ����  
	  break;
	}
	
	 case '2':{
	    Uart_Printf(0,"\n��ǰ������ 2");
	    infraredR();         //����ʹ��      	     
          while(1)
	      { 
		      Uart_SendByte(0,0xa);//����
		      Uart_SendByte(0,0xd);//�س�
	     
        	  err=Uart_Getchn(pass,2,0);	//�Ӵ��ڲɼ�����
        	  if(pass[0]==27)  //ESC exit
                break;
        	  Uart_SendByte(0,pass[0]);	//��ʾ�ɼ�������
	      }
	      
	      hudelay(1000);	      
	      UCON2=UCON2&0x7fc;  //��ֹ����
	      break;
	}
	
    case '3':{
      Uart_Printf(0,"\n��ǰ������ 3");
      infraredT();              //����ʹ��
            Uart_Printf(0,"\n");
        	for(i=0;i<100;i++)
        	  for(j=0;j<8;j++)
             	Uart_SendByte(2,transmit[j]);            
        	Uart_Printf(0,"�������");
	  hudelay(1000);
      UCON2=UCON2&0x7f3;         //��ֹ����
      break;
        	
	}
	
	 case '4':{
	    Uart_Printf(0,"\n��ǰ������ 4");	    
        infraredR();           //����ʹ��
        	  Uart_Printf(0,"\n");
        	  z=k=0;
		      for(i=0;i<100;i++)
		        for(j=0;j<8;j++)
        	    {err=Uart_Getchn(receive,2,0);	//�Ӵ��ڲɼ�����
        	     z++;
        	     store[i][j]=*receive;
        	     if(store[i][j]!=transmit[j])
        	     k++;
        	     Uart_SendByte(0,store[i][j]);	    //��ʾ�ɼ�������
        	
        	    }        	  
        	  Uart_Printf(0,"\n�������ݸ�����:%d",k);
        	  Uart_Printf(0,"\n�������ݸ�����:%d",z);
	          hudelay(1000);
        	  UCON2=UCON2&0x7fc;  //��ֹ����
        	  break;
       	}
	}//end switch
	
 }//end while
}
	
 
	
