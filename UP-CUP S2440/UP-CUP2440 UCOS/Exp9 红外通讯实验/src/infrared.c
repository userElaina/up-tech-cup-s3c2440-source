#include "inc/lib.h"
#define ULCON2	(*(volatile unsigned char *)0x50008000)
#define UCON2	(*(volatile unsigned char *)0x50008004)
 void infraredT(void)   
   {  UCON2=UCON2&0x7fc;         //��ֹ����
      UCON2=UCON2&0x7f3;         //��ֹ����
      ULCON2=ULCON2|(1<<6);      //����ģʽ
      hudelay(100);
      UCON2=UCON2|(0x01<<2);     //������
    }
    
 void infraredR(void) 
   { UCON2=UCON2&0x7f3;        //��ֹ����
	 UCON2=UCON2&0x7fc;        //��ֹ����
	 ULCON2=ULCON2|(1<<6);     //����ģʽ
	 hudelay(1000);
	 UCON2=UCON2|0x01;        //������� 
	}   