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


	ARMTargetInit();	//�������ʼ��
	LCD_Init();			//LCD��ʼ��
	
   // hudelay(5000);
    

  for (i=0;i<9;i++)
     {  switch (i)
        {  case 0: jcolor=0x00000000;  //RGB��Ϊ0   ��ɫ
                   break;
           case 1: jcolor=0x000000f8;  //R   ��ɫ
                   break;
           case 2: jcolor=0x0000f0f8;  //R  and G ��ɫ
                   break;
           case 3: jcolor=0x0000fcf8;   //R  and G ��
                   break;
           case 4: jcolor=0x0000fc00;  //G  ��ɫ
                   break;
           case 5: jcolor=0x00f8fc00;  //G  B   ��ɫ
                   break;
           case 6: jcolor=0x00f80000;  //B    ��ɫ
                   break;
           case 7: jcolor=0x00f800f8;  //R  and B   ��ɫ
                   break;
           case 8: jcolor=0x00f8fcf8;  //RGB   ��ɫ
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
	
	

