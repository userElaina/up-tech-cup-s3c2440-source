#include "../inc/drivers.h"
#include "../inc/lib.h"
#include "inc/gpio.h"
#include "inc/spi.h"
#include "inc/tchScr.h"
#include <stdio.h>

#define DPRINTF(...)		//printfNULL


/*复用功能管脚定义宏*/
#define nYPON 0x3
#define YMON 0x3
#define nXPON 0x3
#define XMON 0x3
/*ACDCON宏*/
#define ECFLG_END 1                 // End of A/D conversion
#define PRSCEN_Enable 1			//prescaler Enable
#define PRSCVL 49 					//A/D converter in process
#define SEL_MUX 7 				// xp
#define STDBM_NORMAL 0				//normal mode
#define STDBM_STANDBY 1			//Standby mode
#define READ_START 0			    //Disable start by read operation
#define ENABLE_START 0					//No operation
/*ADC touch screen control register宏*/
#define YM_SEN_Hi_Z 0 				//YM=Hi-Z
#define YM_SEN_GND 1 			    //YM=GND
#define YP_SEN_External_voltage 0   //YP=External voltage
#define YP_SEN_AIN5 1       		//conntected with AIN5
#define XM_SEN_Hi_Z 0				
#define XM_SEN_GND  1
#define XP_SEN_External_voltage 0
#define xp_SEN_AIN7 1
#define PULL_UP_ENABLE 0  			//ENABLE
#define PULL_UP_DISABLE 1			//DISABLE
#define AUTO_PST_Normal 0			//Normal ADC conversion
#define AUTO_PST_AUTO   1			//Auto(sequential) x/y positioin conversion mode
#define XY_PST 0					//No operation mode
#define XY_PST_INT 0x3				//waiting for interrupt Mode

/*ADC start or interval delay register*/
#define DELAY 
/*ADC conversion data0 register*/
#define UPDOWN_DOWN_0 0
#define UPDOWN_UP_0 1
#define AUTO_PST_sequencing_0 1
#define XY_PST_0 0
/*ADC conversion data1 register*/
#define UPDOWN_DOWN_1 0
#define UPDOWN_UP_1 1
#define AUTO_PST_sequencing_1 1
#define XY_PST_1 0

#define LCDWIDTH 320
#define LCDHEIGHT 240
int TchScr_Xmin=145,TchScr_Xmax=902,
    TchScr_Ymin=142,TchScr_Ymax=902; 

 void TchScr_init()
{
	/*复用管脚功能定义*/
	rGPGCON &= ~((0x03 << 30)|(0x03 << 28)|(0x03 << 26)|(0x03 << 24));
	rGPGCON|=(nYPON<<30)|(YMON<<28)|(nXPON<<26)|(XMON<<24);
	/*set ACDCON*/
	rADCCON=(PRSCEN_Enable<<14)|(PRSCVL<<6)|(SEL_MUX<<3);
	/*ADC start or interval delay register*/
	rADCDLY=0xff;
	/*set ADC touch screen control register*/
	rADCTSC = (0<<8)|(1<<7)|(1<<6)|(0<<5)|(1<<4)|(0<<3)|(0<<2)|(3);
}

#define CLOCK_DELAY()		do{int i; for(i=0; i<20; i++);}while(0)

void TchScr_GetScrXY(int *x, int *y)
{
    int oldx,oldy;
    rADCTSC|=(1<<3)|(1<<2)|(0);
    rADCCON|=1;
    //CLOCK_DELAY();
    while(!(SUBSRCPND&(1<<10)));    
    oldx=rADCDAT0&0x3ff;
    oldy=rADCDAT1&0x3ff;
    if(oldx!=0)
    {
    *x=oldx;
    *y=oldy;
    }
    
    rADCTSC = (0<<8)|(1<<7)|(1<<6)|(0<<5)|(1<<4)|(0<<3)|(0<<2)|(3);
    SUBSRCPND|=(1<<9); 
    SUBSRCPND|=(1<<10);   
    //Uart_Printf(0,"x=%d,\ty=%d\n",*x,*y);
    //rADCCON|=(1<<2);
	/*unsigned int temp;

	*x=(*x-TchScr_Xmin)*LCDWIDTH/(TchScr_Xmax-TchScr_Xmin);
    *y=(*y-TchScr_Ymin)*LCDHEIGHT/(TchScr_Ymax-TchScr_Ymin);

	*/


}



//#endif	
