#include "inc/lib.h"
#define ULCON2	(*(volatile unsigned char *)0x50008000)
#define UCON2	(*(volatile unsigned char *)0x50008004)
 void infraredT(void)   
   {  UCON2=UCON2&0x7fc;         //禁止接收
      UCON2=UCON2&0x7f3;         //禁止发送
      ULCON2=ULCON2|(1<<6);      //红外模式
      hudelay(100);
      UCON2=UCON2|(0x01<<2);     //允许发送
    }
    
 void infraredR(void) 
   { UCON2=UCON2&0x7f3;        //禁止发送
	 UCON2=UCON2&0x7fc;        //禁止接收
	 ULCON2=ULCON2|(1<<6);     //红外模式
	 hudelay(1000);
	 UCON2=UCON2|0x01;        //允许接收 
	}   