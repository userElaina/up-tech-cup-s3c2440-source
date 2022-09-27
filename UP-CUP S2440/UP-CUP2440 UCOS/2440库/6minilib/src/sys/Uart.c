/***************************************************************************\
	Copyright (c) 2004-2007 threewater@up-tech.com, All rights reserved.
	by threewter	2004.4.26
\***************************************************************************/
	

/***************************************************************************\
    #˵��: ���ڳ���㺯��
	----------------------------------  Bug  --------------------------------------

	----------------------------------  TODO list  --------------------------------------

	----------------------------------����--------------------------------------
	2004-5-26	����

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
		if(nrev<0)//ѭ������������
			nrev+=UART_REVBUFFER_SIZE;
	}

	if(nrev>=pUart_Buffer->nRev_Uart)
		OSMboxPost(pUart_Buffer->Uart_Rev_mbox, (void*)nrev);
}

/*************************
	������n������
	����ֵ,ʵ�ʶ�ȡ������
****************************/
int OSReadUart(int ndev, U8 data[], int num, int ntimeout)
{
	int i;
	INT8U err;
	int nrev;
	struct_Uart_Buffer *pUart_Buffer=&Uart_Buffer[ndev];

	pUart_Buffer->nRev_Uart=num;	//����Uart1����������

	nrev=pUart_Buffer->pUart_rev-pUart_Buffer->pUart_read;
	if(nrev<0)//ѭ������������
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
		Uart_Buffer[i].pUart_rev=Uart_Buffer[i].Buffer;	//Uart ���ջ�����ָ��
		Uart_Buffer[i].pUart_read=Uart_Buffer[i].Buffer;	//Uart ��������ָ��
		Uart_Buffer[i].nRev_Uart=1;		//�趨��Uart�����ַ���
		OpenUartRev(i);
	}
}

/*************************
	��������n�����ж�
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

