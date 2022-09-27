/***************************************************************************\
	Copyright (c) 2004-2007 threewater@up-tech.com, All rights reserved.
	by threewter	2004.4.26
\***************************************************************************/
	

/***************************************************************************\
    #说明: C库函数等定义
	----------------------------------  Bug  --------------------------------------

	----------------------------------  TODO list  --------------------------------------

	----------------------------------修正--------------------------------------
	2004-5-2	1、添加udelay，微秒级延时函数

	2004-4-26	创建

\***************************************************************************/

#include "../inc/sys/lib.h"
#include "../inc/sys/mmu.h"
#include "../inc/macro.h"
#include "../inc/drv/Serial.h"
#include "../inc/drv/reg2410.h"

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

//--------------------------------SYSTEM---------------------------------//

/*************************************************************\
	100微秒级延时函数
	参数time: 表示延时的时间,单位微秒。
	如果time=0，则启用看门狗定时器，
	自动计算时间
\**************************************************************/
void hudelay(int time)
{
	static int delayLoopCount=100;
	int i,adjust=0, curpc;

	if(time==0)
	{
		time=adjust=200;
		delayLoopCount = 400;
		//PCLK/1M,Watch-dog disable,1/64,interrupt disable,reset disable
		rWTCON = ((PCLK/1000000-1)<<8)|(2<<3); 
		rWTDAT = 0xffff;                              //for first update
		rWTCNT = 0xffff;                              //resolution=64us @any PCLK 
		rWTCON = ((PCLK/1000000-1)<<8)|(2<<3)|(1<<5); //Watch-dog timer start
	}

	for(;time>0;time--){
		__asm{mov curpc, pc};

		for(i=0;i<delayLoopCount;i++);

		__asm{sub curpc, pc, curpc};
	}

	if(adjust)
	{
		rWTCON = ((PCLK/1000000-1)<<8)|(2<<3);   //Watch-dog timer stop
		i = 0xffff - rWTCNT;                     // 1count->64us, 200*400 cycle runtime = 64*i us
		delayLoopCount = 8000000/(i*64);         //200*400:64*i=1*x:100 -> x=80000*100/(64*i)   

		printk("Get delay loop count successfully! The value is %d, ", delayLoopCount);
		printk("Maybe %d MIPS\n", curpc*delayLoopCount/400);
	}
}

/************************* SYSTEM PARAMETER ***************************/
struct struct_system_param system_param;

void loadsystemParam(void)
{
#if 0
	if((sizeof(system_param)>PARAM_SIZE)){
		printk("Too big PARAM struct!\n");
		for(;;);
	}
	
	//Load system param
	memcpy(&system_param, (void*)PARAM_ADDRESS, sizeof(system_param));

	if(system_param.magic!=PARAM_MAGIC)
		printk("WARNING: wrong system parameter!\n");
#endif
}

/************************* UART ****************************/
extern serial_driver_t* serial_drv[];
extern int NumberOfUartDrv;

serial_loop_func_t Getch_loopfunc[]={(serial_loop_func_t)NULL,(serial_loop_func_t)NULL,
					(serial_loop_func_t)NULL, (serial_loop_func_t)NULL};
#define GETCH_LOOPFUNC_NUM		(sizeof(Getch_loopfunc)/sizeof(serial_loop_func_t))

int Uart_Init(int whichUart, int baud)
{
	if(whichUart>=NumberOfUartDrv)
		return FALSE;

	return serial_drv[whichUart]->init(baud);
}

/*************************************************************
	设置等待串口数据的时候的循环函数，
	成功返回函数的序号(删除的时候使用)，
	如果失败则返回-1，
	
**************************************************************/
int Set_UartLoopFunc(serial_loop_func_t func)
{
	int i;

	for(i=0;Getch_loopfunc[i];i++);

	if(i>=GETCH_LOOPFUNC_NUM)
		return -1;

	Getch_loopfunc[i]=func;
	return i;
}

/*************************************************************\
	清除等待串口数据的时候的循环函数，
	参数是函数的序号，
	成功返回TURE，失败则返回FALSE
	
\**************************************************************/
int Clear_UartLoopFunc(int index)
{
	if(index>=GETCH_LOOPFUNC_NUM || index<0)
		return FALSE;

	Getch_loopfunc[index]=NULL;

	return TRUE;
}

/*陈文华新添*/
char Uart_Getch1(int whichUart)
{
	/*int ret;
	U8 data;

	ret=OSReadUart(whichUart, &data, sizeof(data), 0);

	return data;*/
    int i;
	if(whichUart>=NumberOfUartDrv)
	return FALSE;
	while(!serial_drv[whichUart]->poll()){
		for(i=0;i<GETCH_LOOPFUNC_NUM;i++){
			if(Getch_loopfunc[i])
				(*Getch_loopfunc[i])();
		}
	}
	return serial_drv[whichUart]->read();
}

char Uart_Getch(int whichUart)
{
	int ret;
	U8 data;

	ret=OSReadUart(whichUart, &data, sizeof(data), 0);

	return data;

	
	/*if(whichUart>=NumberOfUartDrv)
		return FALSE;
	while(!serial_drv[whichUart]->poll()){
		for(i=0;i<GETCH_LOOPFUNC_NUM;i++){
			if(Getch_loopfunc[i])
				(*Getch_loopfunc[i])();
		}
	}
	return serial_drv[whichUart]->read();*/
}

//串口是否有数据输入
int Uart_Poll(int whichUart)
{
	if(whichUart>=NumberOfUartDrv)
		return FALSE;

	return serial_drv[whichUart]->poll();
}

//发送缓冲区清空
void Uart_TxEmpty(int whichUart)
{
	if(whichUart<NumberOfUartDrv)
		serial_drv[whichUart]->flush_output();
}

//接收缓冲区清空
void Uart_RxEmpty(int whichUart)
{
	if(whichUart<NumberOfUartDrv)
		serial_drv[whichUart]->flush_input();
}

int Uart_SendByte(int whichUart,int data)
{
	if(whichUart>=NumberOfUartDrv)
		return FALSE;

	return serial_drv[whichUart]->write(data);
}
void Uart_GetString1(int whichUart, char *string)
{
    char *string2=string;
    char c;
    while((c=Uart_Getch1(whichUart))!='\r')
    {
		if(c=='\b')
		{
		    if(	(int)string2 < (int)string )
		    {
				Uart_Printf(whichUart,"\b \b");
				string--;
		    }
		}
		else 
		{
		    *string++=c;
		    Uart_SendByte(whichUart,c);
		}
    }
    *string='\0';
    Uart_SendByte(whichUart,'\r');
    Uart_SendByte(whichUart,'\n');
}

void Uart_GetString(int whichUart, char *string)
{
    char *string2=string;
    char c;
    while((c=Uart_Getch(whichUart))!='\r')
    {
		if(c=='\b')
		{
		    if(	(int)string2 < (int)string )
		    {
				Uart_Printf(whichUart,"\b \b");
				string--;
		    }
		}
		else 
		{
		    *string++=c;
		    Uart_SendByte(whichUart,c);
		}
    }
    *string='\0';
    Uart_SendByte(whichUart,'\r');
    Uart_SendByte(whichUart,'\n');
}


int Uart_GetIntNum(int whichUart)
{
    char str[30];
    char *string=str;
    int base=10;
    int minus=0;
    int lastIndex;
    int result=0;
    int i;
    
    Uart_GetString(whichUart, string);
    
    if(string[0]=='-')
    {
        minus=1;
        string++;
    }
    
    if(string[0]=='0' && (string[1]=='x' || string[1]=='X'))
    {
		base=16;
		string+=2;
    }
    
    lastIndex=strlen(string)-1;
    if( string[lastIndex]=='h' || string[lastIndex]=='H' )
    {
		base=16;
		string[lastIndex]=0;
		lastIndex--;
    }

    if(base==10)
    {
		result=atoi(string);
		result=minus ? (-1*result):result;
    }
    else
    {
		for(i=0;i<=lastIndex;i++)
		{
    	    if(isalpha(string[i]))
			{
				if(isupper(string[i]))
					result=(result<<4)+string[i]-'A'+10;
				else
				    result=(result<<4)+string[i]-'a'+10;
			}
			else
			{
				result=(result<<4)+string[i]-'0';
			}
		}
		result=minus ? (-1*result):result;
    }
    return result;
}


void Uart_SendString(int whichUart, char *pt)
{
    while(*pt){

	if(*pt=='\n')
		Uart_SendByte(whichUart,'\r');

	Uart_SendByte(whichUart,*pt++);
    }
}

//if you don't use vsprintf(), the code size is reduced very much.
void Uart_Printf(int whichUart, char *fmt,...)
{
	va_list ap;
	static char string[256];

	va_start(ap,fmt);
	vsprintf(string,fmt,ap);
	Uart_SendString(whichUart, string);
	va_end(ap);
}

/**************************************************************\
	等待接收串口数据，带超时
	
\**************************************************************/
int Uart_GetchTimeout(int whichUart, char* pbuffer, int n, int timeout)
{
	int i, t;

	if(whichUart>=NumberOfUartDrv)
		return FALSE;

	for(;n>0;n--){
		t=timeout*10;
		while(!serial_drv[whichUart]->poll()){
			for(i=0;i<GETCH_LOOPFUNC_NUM;i++){
				if(Getch_loopfunc[i])
					(*Getch_loopfunc[i])();
			}
			t--;
			if(t==0)
				return FALSE;
		}
		*pbuffer = serial_drv[whichUart]->read();
		pbuffer++;
	}
	return TRUE;
}

void printfNULL(char *fmt, ...)
{
}
