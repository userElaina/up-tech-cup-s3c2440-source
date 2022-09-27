#include "inc/macro.h"

//#define LCDDisplayPin			0x0010	//GPB4
#define LCDWIDTH		640
#define LCDHEIGHT		480
//U32* pLCDBuffer16=(U32*)0x03000000;
/*#define LCDCON1		(*(volatile unsigned *)0X4D000000)
#define LCDCON2		(*(volatile unsigned *)0X4D000004)
#define LCDCON3		(*(volatile unsigned *)0X4D000008)
#define LCDCON4		(*(volatile unsigned *)0X4D00000C)
#define LCDCON5		(*(volatile unsigned *)0X4D000010)
#define LCDSADDR1		(*(volatile unsigned *)0X4D000014)
#define LCDSADDR2		(*(volatile unsigned *)0X4D000018)
#define LCDSADDR3	    (*(volatile unsigned *)0X4D00001C)*/
//#define GPGCON	    (*(volatile unsigned *)0x56000060)
void LCD_Refresh(void);
void LCD_Init(void);
//void LCDDisplayOpen(U8 isOpen);
