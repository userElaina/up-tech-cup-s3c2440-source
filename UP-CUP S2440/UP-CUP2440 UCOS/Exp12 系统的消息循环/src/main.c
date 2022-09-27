/***************************************************************************\
	Copyright (c) 2004-2007 threewater@up-tech.com, All rights reserved.
	by threewter	2004.5.12
\***************************************************************************/
/***************************************************************************\
    #说明: C  main 函数，ucos-ii初始化等定义
	----------------------------------  Bug  --------------------------------------

	----------------------------------  TODO list  --------------------------------------

	----------------------------------修正--------------------------------------
	2004-5-12	创建

\***************************************************************************/

#include"../ucos-ii/includes.h"               /* uC/OS interface */
#include "../ucos-ii/add/osaddition.h"
#include "../inc/drivers.h"

#include "../inc/sys/lib.h"
#include "../src/gui/gui.h"
#include <string.h>
#include <stdio.h>

#pragma import(__use_no_semihosting_swi)  // ensure no functions that use semihosting 


///******************任务定义***************///

OS_STK Main_Stack[STACKSIZE]={0, };   //Main_Test_Task堆栈
void Main_Task(void *Id);             //Main_Test_Task
#define Main_Task_Prio     12

OS_STK test_Stack[STACKSIZE]={0, };   //test_Test_Task堆栈
void test_Task(void *Id);             //test_Test_Task
#define test_Task_Prio     15


/**************已经定义的OS任务*************
#define SYS_Task_Prio				1
#define Touch_Screen_Task_Prio		9
#define Main_Task_Prio     12
#define Key_Scan_Task_Prio      58
#define Lcd_Fresh_prio           59
#define Led_Flash_Prio           60

***************************************/////////



///*****************事件定义*****************///


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
	PDC pdc;//创建绘图设备上下文结构
	U16 ch[10];
	char *str, skey[10];
	
	if((nkey&KEY_DOWN)==0)
		return;

	ClearScreen();//清屏
	pdc=CreateDC();//创建绘图设备上下文f

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
	strChar2Unicode(ch, str);//将整形数据转化成Unicode
	TextOut(pdc, 150, 100, ch, TRUE, FONTSIZE_MIDDLE);//文本模式下显示文字
	DestoryDC(pdc);//删除绘图设备上下文	
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Main_Task(void *Id)             //Main_Test_Task
{
	POSMSG pMsg=0;//创建消息结构

	ClearScreen();//清屏

	//消息循环
	for(;;){
		pMsg=WaitMessage(0); //等待消息
		switch(pMsg->Message){
		case OSM_KEY://键盘消息
			onKey(pMsg->WParam,pMsg->LParam);
			break;
		}
		DeleteMessage(pMsg);//删除消息,释放资源
	}
}

