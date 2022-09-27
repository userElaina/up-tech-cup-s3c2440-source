
#ifndef __LIB_H__
#define __LIB_H__

//#include	"io.h"
//#include	"sysdrv.h"		//驱动抽象层头文件
//#include	"../inc/macro.h"

#define printk		printf




/***********************System define***************************************/
extern unsigned int PCLK, HCLK, FCLK;



/***********************macro define***************************************/
#define mdelay(n)	hudelay(n*10)

/***********************function define***************************************/
typedef void (*serial_loop_func_t)(void);

/***********************arm define***************************************/
//#define FlushCache()		__asm{mcr p15, 0, r0, c7, c7, 0}

/*lib.c*/
void hudelay(int time); //Watchdog Timer is used.


int Uart_Init(int whichUart,int baud);

int Uart_SendByte(int whichUart, int data);

void Uart_Printf(int whichUart, char *fmt,...);
void Uart_SendString(int whichUart, char *pt);




#endif
