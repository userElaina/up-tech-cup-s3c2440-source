#include "../ucos-ii/includes.h"
#include "../inc/sys/can.h"
#include "../inc/sys/lib.h"
#include <string.h>
#include <stdio.h>

//#define DPRINTF		printf
#define DPRINTF		printfNULL

extern can_driver_t* can_driver[];
extern int Can_Device_Num;

#define NextCanRevDataPos(pos)		do{(pos)=((pos)+1>=CAN_REVBUFFER_SIZE? 0: (pos)+1);}while(0)

extern struct_CAN_Buffer CAN_Buffer[];
extern can_driver_t* can_driver[];
extern int Can_Device_Num;

/***********************************************************************************\
								Can�жϴ�����
 	����: 
 		vector��	�ж�����
		ndev��		CAN�豸��

	Tip: �豸����������Drivers.c�ж���
\***********************************************************************************/
void irq_OSCAN(int vector , int ndev)
{
	int result;
	struct_CAN_Buffer *pcan_Buffer=&CAN_Buffer[ndev];

	DPRINTF("CAN interrupt: vector=%d, device=%d\n",vector, ndev);

	if(ndev>=Can_Device_Num)
		return;

	while((result=can_driver[ndev]->poll())!=0){
		if(result & CANPOLL_Rev){
			//Receive data

			if(can_driver[ndev]->read(&pcan_Buffer->buffer[pcan_Buffer->nCanRevpos])!=OK){
				DPRINTF("CAN interrupt: receive data error!\n",ndev);
				return;
			}

			DPRINTF("CAN interrupt: receive data, device=%d\n",ndev);
			NextCanRevDataPos(pcan_Buffer->nCanRevpos);
			OSSemPost(pcan_Buffer->Rev_mbox);
		}
	}
}

/***********************************************************************************\
								��ʼ��Can�豸									
 	����: 
		ndev��		CAN�豸��
		bandrate��	CAN������

	Tip: �豸����������Drivers.c�ж���
\***********************************************************************************/
void Init_Can(int ndev, CanBandRate bandrate, PCanFilter pfilter)
{
	if(ndev>=Can_Device_Num)
		return;

	if(can_driver[ndev]->init)
		can_driver[ndev]->init(bandrate, TRUE, pfilter);
}

/***********************************************************************************\
				��Can�豸(�����ж�)
 	����: 
		ndev��		CAN�豸��
\***********************************************************************************/
void Open_Can(int ndev)
{
	INT8U err;
	struct_CAN_Buffer *pcan_Buffer=&CAN_Buffer[ndev];

	if(ndev>=Can_Device_Num)
		return;

	SetISR_Interrupt(can_driver[ndev]->nIsr, (Interrupt_func_t)irq_OSCAN, (void*)ndev);

	if(pcan_Buffer->Rev_mbox)
		OSSemDel(pcan_Buffer->Rev_mbox, OS_DEL_ALWAYS, &err);

	pcan_Buffer->Rev_mbox=OSSemCreate(0);

	if(can_driver[ndev]->Open)
		can_driver[ndev]->Open();
}

/***********************************************************************************\
				�ر�Can�豸ֹͣ��������
 	����: 
		ndev��		CAN�豸��
\***********************************************************************************/
void Close_Can(int ndev)
{
	if(ndev>=Can_Device_Num)
		return;

	if(can_driver[ndev]->Close)
		can_driver[ndev]->Close();
}

/***********************************************************************************\
								��ȡCan�豸�յ�������
	����: 
		ndev��		CAN�豸��
		data��		���ݽṹ��
		ntimeout��	��ʱʱ�䣬��λ���룬���Ϊ0������Զ�ȴ�
	����ֵ:
		�ɹ�����TRUE�����򣬷���FALSE
\***********************************************************************************/
BOOL CanRead(int ndev, PCanData data, int ntimeout)
{
	INT8U err;
	struct_CAN_Buffer *pcan_Buffer=&CAN_Buffer[ndev];

	if(ndev>=Can_Device_Num)
		return FALSE;

	OSSemPend(pcan_Buffer->Rev_mbox, ntimeout, &err);
	if(err==OS_TIMEOUT)
		return FALSE;

	memcpy(data, &(pcan_Buffer->buffer[pcan_Buffer->nCanReadpos]),sizeof(CanData));
	NextCanRevDataPos(pcan_Buffer->nCanReadpos);


//	if(can_driver[ndev]->read)
//		return can_driver[ndev]->read(data,ntimeout);

	return FALSE;
}

/***********************************************************************************\
								Can�豸��������
	����: 
		ndev��		CAN�豸��
		data��		���ݽṹ��
\***********************************************************************************/
void CanSend(int ndev, PCanData data)
{
	if(ndev>=Can_Device_Num)
		return;

	if(can_driver[ndev]->write)
		can_driver[ndev]->write(data);
}

#define CANTEXT_TX	1
#define CANTEXT_RX	1

void CAN_Test()
{
	BOOL temp;
	int i;
	CanData revdata, senddata={0x1, {1,2,3,4,5,6,7,8},8, FALSE,FALSE};

	Init_Can(0,BandRate_500kbps,NULL);
	Init_Can(1,BandRate_500kbps,NULL);
	Open_Can(0);
	Open_Can(1);

	for(i=0;;i++){
#if (CANTEXT_TX == 1)
		CanSend(1,&senddata);
		senddata.id++;
#endif

#if (CANTEXT_RX == 1)
		memset(&revdata,0,sizeof(revdata));
		temp=CanRead(0, &revdata,0);
			printk("id=%x\n",revdata.id);
			printk("data=%x,%x,%x,%x,%x,%x,%x,%x\n",
				revdata.data[0],revdata.data[1],revdata.data[2],revdata.data[3],
				revdata.data[4],revdata.data[5],revdata.data[6],revdata.data[7]);
#endif

#if (CANTEXT_TX == 1)
		OSTimeDly(100);
#endif
	}	
}

