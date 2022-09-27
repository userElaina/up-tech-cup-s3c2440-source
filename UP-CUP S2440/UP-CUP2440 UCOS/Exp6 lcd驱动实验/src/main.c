#include "../inc/drivers.h"
#include "../inc/lib.h"
#include <string.h>
#include <stdio.h>
#include "inc/lcd320.h"
#include "inc/macro.h"
#include  "inc/reg2440.h"
#pragma import(__use_no_semihosting_swi)  // ensure no functions that use semihosting 

void  ARMTargetInit(void); 
extern U32 LCDBufferII2[480][640];


int main(void)
{   
	
	int i,j,k;
    U32 jcolor;


	ARMTargetInit();	//开发版初始化
	LCD_Init();			//LCD初始化
	
   // hudelay(5000);
    

  for (i=0;i<9;i++)
     {  switch (i)
        {  case 0: jcolor=0x00000000;  //RGB均为0   黑色
                   break;
           case 1: jcolor=0x000000f8;  //R   红色
                   break;
           case 2: jcolor=0x0000f0f8;  //R  and G 橙色
                   break;
           case 3: jcolor=0x0000fcf8;   //R  and G 黄
                   break;
           case 4: jcolor=0x0000fc00;  //G  绿色
                   break;
           case 5: jcolor=0x00f8fc00;  //G  B   青色
                   break;
           case 6: jcolor=0x00f80000;  //B    蓝色
                   break;
           case 7: jcolor=0x00f800f8;  //R  and B   紫色
                   break;
           case 8: jcolor=0x00f8fcf8;  //RGB   白色
                   break;   
         }
       
      for (k=0;k<480;k++)
        for (j=i*64;j<i*64+64;j++) 
          LCDBufferII2[k][j]=jcolor; 
      }

  jcolor=0x000000ff;
  for (i=0;i<480;i++)
      {if (i==160||i==320)
         jcolor<<=8;
       for (j=576;j<640;j++)  
          LCDBufferII2[i][j]=jcolor;
       }

    LCD_Refresh() ; 
    //Uart_Printf(0,"\nrLCDCON1 is %x",rLCDCON1);    
    while(1);
	return 0;
	
	
	
}
	
	

