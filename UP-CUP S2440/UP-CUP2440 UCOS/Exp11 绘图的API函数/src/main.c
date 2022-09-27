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
	initOSDC();
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


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Main_Task(void *Id)             //Main_Test_Task
{
	int oldx,oldy;//����ԭ������ϵλ��
	PDC pdc;//�����ͼ�豸�����Ľṹ
	int x,y;//����
	double offset=0;//x����ƫ����

	ClearScreen();//����
	pdc=CreateDC();//������ͼ�豸������
	SetDrawOrg(pdc, LCDWIDTH/2,LCDHEIGHT/2, &oldx, & oldy); //���û�ͼԭ��Ϊ��Ļ����

	Circle(pdc,0, 0, 50);//��Բ
	MoveTo(pdc, -50, -50);//�ƶ�
	LineTo(pdc, 50, -50);//����
	ArcTo(pdc, 80, -20, TRUE, 30);//����
	LineTo(pdc, 80, 20);
	ArcTo(pdc, 50, 50, TRUE, 30);
	LineTo(pdc, -50, 50);
	ArcTo(pdc, -80, 20, TRUE, 30);
	LineTo(pdc, -80, -20);
	ArcTo(pdc, -50, -50, TRUE, 30);

	OSTimeDly(3000);//���������3��
	ClearScreen();
	SetDrawOrg(pdc, 0, LCDHEIGHT/2, &oldx,&oldy);//���û�ͼԭ��Ϊ��Ļ����в�
	for(;;)
	{
		MoveTo(pdc, 0, 0);
		for(x=0;x<LCDWIDTH;x++)
		{//�����Ҳ�
			y=(int)(50*sin(((double)x)/20.0+offset));
			LineTo(pdc, x, y);
		}
		offset+=1;
		if(offset>=2*3.14)
			offset=0;
		OSTimeDly(1000);
		ClearScreen();
	}
	
DestoryDC(pdc);//ɾ����ͼ�豸������
}
