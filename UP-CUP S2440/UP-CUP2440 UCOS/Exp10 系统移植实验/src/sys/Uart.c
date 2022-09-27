/***************************************************************************\
	Copyright (c) 2004-2007 threewater@up-tech.com, All rights reserved.
	by threewter	2004.4.26
\***************************************************************************/
	

/***************************************************************************\
    #说明: 串口抽象层函数
	----------------------------------  Bug  --------------------------------------

	----------------------------------  TODO list  --------------------------------------

	----------------------------------修正--------------------------------------
	2004-5-26	创建

\***************************************************************************/

#include "../inc/macro.h"
#include "../ucos-ii/includes.h"

#include "../ucos-ii/uhal/isr.h"
#include "uart.h"

extern struct_Uart_Buffer Uart_Buffer[];
extern serial_driver_t* serial_drv[];
extern int NumberOfUartDrv;

void irq_OSRevUart(int vector, int ndev)
{
	int nrev;
	struct_Uart_Buffer *pUart_Buffer=&Uart_Buffer[ndev];

	if(ndev>=NumberOfUartDrv)
		return;

	while(serial_drv[ndev]->poll()){
		//Receive data
		*(pUart_Buffer->pUart_rev)=(U8)serial_drv[ndev]->read();
		pUart_Buffer->pUart_rev++;
		if(pUart_Buffer->pUart_rev==pUart_Buffer->Buffer+UART_REVBUFFER_SIZE)
			pUart_Buffer->pUart_rev=pUart_Buffer->Buffer;

		nrev=pUart_Buffer->pUart_rev-pUart_Buffer->pUart_read;
		if(nrev<0)//循环缓冲区调整
			nrev+=UART_REVBUFFER_SIZE;
	}

	if(nrev>=pUart_Buffer->nRev_Uart)
		OSMboxPost(pUart_Buffer->Uart_Rev_mbox, (void*)nrev);
}

/*************************
	读串口n的数据
	返回值,实际读取的数据
****************************/
int OSReadUart(int ndev, U8 data[], int num, int ntimeout)
{
	int i;
	INT8U err;
	int nrev;
	struct_Uart_Buffer *pUart_Buffer=&Uart_Buffer[ndev];

	pUart_Buffer->nRev_Uart=num;	//设置Uart1接收数据数

	nrev=pUart_Buffer->pUart_rev-pUart_Buffer->pUart_read;
	if(nrev<0)//循环缓冲区调整
		nrev+=UART_REVBUFFER_SIZE;

	if(nrev<num){
		while((nrev=(int)OSMboxPend(pUart_Buffer->Uart_Rev_mbox, ntimeout,&err))<num){
			if(err==OS_TIMEOUT){
				num=nrev;
				break;
			}
		}
	}

	for(i=0;i<num;i++){
		data[i]=*(pUart_Buffer->pUart_read);
		pUart_Buffer->pUart_read++;
		if(pUart_Buffer->pUart_read==pUart_Buffer->Buffer+UART_REVBUFFER_SIZE)
			pUart_Buffer->pUart_read=pUart_Buffer->Buffer;
	}
	return nrev;
}

void OSInitUart(void)
{
	int i;

	for(i=0;i<NumberOfUartDrv;i++){
		Uart_Buffer[i].pUart_rev=Uart_Buffer[i].Buffer;	//Uart 接收缓冲区指针
		Uart_Buffer[i].pUart_read=Uart_Buffer[i].Buffer;	//Uart 读缓冲区指针
		Uart_Buffer[i].nRev_Uart=1;		//设定的Uart接收字符数
		OpenUartRev(i);
	}
}

/*************************
	开启串口n接收中断
****************************/
int OpenUartRev(int ndev)
{
	struct_Uart_Buffer *pUart_Buffer=&Uart_Buffer[ndev];

	if(ndev>=NumberOfUartDrv)
		return FAIL;

	if(pUart_Buffer->Uart_Rev_mbox==NULL)
		pUart_Buffer->Uart_Rev_mbox=OSMboxCreate((void*)NULL);

	SetISR_Interrupt(serial_drv[ndev]->nIsr, (Interrupt_func_t)irq_OSRevUart, (void*)ndev);

	return OK;
}

