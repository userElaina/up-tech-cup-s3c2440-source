/***************************************************************************\
	Copyright (c) 2004-2007 threewater@up-tech.com, All rights reserved.
	by threewter	2004.5.12
\***************************************************************************/
/***************************************************************************\
    #˵��: C  main ������ucos-ii��ʼ���ȶ���
	----------------------------------  Bug  --------------------------------------

	----------------------------------  TODO list  --------------------------------------

	----------------------------------����--------------------------------------
	2004-5-12	����

\***************************************************************************/

#include"../ucos-ii/includes.h"               /* uC/OS interface */
#include "../ucos-ii/add/osaddition.h"
#include "../inc/drivers.h"

#include "../inc/sys/lib.h"
#include "../src/gui/gui.h"
#include <string.h>
#include <stdio.h>

#pragma import(__use_no_semihosting_swi)  // ensure no functions that use semihosting 


///******************������***************///

OS_STK Main_Stack[STACKSIZE]={0, };   //Main_Test_Task��ջ
void Main_Task(void *Id);             //Main_Test_Task
#define Main_Task_Prio     12

OS_STK test_Stack[STACKSIZE]={0, };   //test_Test_Task��ջ
void test_Task(void *Id);             //test_Test_Task
#define test_Task_Prio     15


/**************�Ѿ������OS����*************
#define SYS_Task_Prio				1
#define Touch_Screen_Task_Prio		9
#define Main_Task_Prio     12
#define Key_Scan_Task_Prio      58
#define Lcd_Fresh_prio           59
#define Led_Flash_Prio           60

***************************************/////////



///*****************�¼�����*****************///


/////////////////////////////////////////////////////
//                  Main function.                //
////////////////////////////////////////////////////
int main(void)
{

	ARMTargetInit();        // do target (uHAL based ARM system) initialisation //

	OSInit();               // needed by uC/OS-II //

	OSInitUart();

	initOSFile();
#if USE_MINIGUI==0
	initOSMessage();
	initOSList();
	initOSDC();
	initOSCtrl();
	LoadFont();
#endif

	loadsystemParam();
	
	// create the tasks in uC/OS and assign increasing //
	// priorities to them so that Task3 at the end of  //
	// the pipeline has the highest priority.          //
	LCD_printf("Create task on uCOS-II...\n");
	OSTaskCreate(Main_Task,  (void *)0,  (OS_STK *)&Main_Stack[STACKSIZE-1],  Main_Task_Prio);
//	OSTaskCreate(test_Task,  (void *)0,  (OS_STK *)&test_Stack[STACKSIZE-1],  test_Task_Prio);

	OSAddTask_Init(1);
  
	LCD_printf("Starting uCOS-II...\n");
	LCD_printf("Entering graph mode...\n");
	LCD_ChangeMode(DspGraMode);

	OSStart();              // start the OS //

	// never reached //
	return 0;
}//main

void onKey(unsigned int nkey, int fnkey)
{
	PDC pdc;//������ͼ�豸�����Ľṹ
	U16 ch[10];
	char *str, skey[10];
	
	if((nkey&KEY_DOWN)==0)
		return;

	ClearScreen();//����
	pdc=CreateDC();//������ͼ�豸������f

	switch(nkey&0xff){
	case 'N':
		str="NumLock";
		break;
	case '\r':
		str="Enter";
		break;
	default:
		skey[0]=nkey;
		skey[1]=0;
		str=skey;
	}

	printf("key=%s\n",str);
	strChar2Unicode(ch, str);//����������ת����Unicode
	TextOut(pdc, 150, 100, ch, TRUE, FONTSIZE_MIDDLE);//�ı�ģʽ����ʾ����
	DestoryDC(pdc);//ɾ����ͼ�豸������	
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Main_Task(void *Id)             //Main_Test_Task
{
	POSMSG pMsg=0;//������Ϣ�ṹ

	ClearScreen();//����

	//��Ϣѭ��
	for(;;){
		pMsg=WaitMessage(0); //�ȴ���Ϣ
		switch(pMsg->Message){
		case OSM_KEY://������Ϣ
			onKey(pMsg->WParam,pMsg->LParam);
			break;
		}
		DeleteMessage(pMsg);//ɾ����Ϣ,�ͷ���Դ
	}
}

