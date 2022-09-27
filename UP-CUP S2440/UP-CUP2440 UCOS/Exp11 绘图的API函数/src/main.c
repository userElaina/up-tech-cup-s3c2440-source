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
	int oldx,oldy;//保存原来坐标系位置
	PDC pdc;//定义绘图设备上下文结构
	int x,y;//坐标
	double offset=0;//x坐标偏移量

	ClearScreen();//清屏
	pdc=CreateDC();//创建绘图设备上下文
	SetDrawOrg(pdc, LCDWIDTH/2,LCDHEIGHT/2, &oldx, & oldy); //设置绘图原点为屏幕中心

	Circle(pdc,0, 0, 50);//画圆
	MoveTo(pdc, -50, -50);//移动
	LineTo(pdc, 50, -50);//画线
	ArcTo(pdc, 80, -20, TRUE, 30);//画弧
	LineTo(pdc, 80, 20);
	ArcTo(pdc, 50, 50, TRUE, 30);
	LineTo(pdc, -50, 50);
	ArcTo(pdc, -80, 20, TRUE, 30);
	LineTo(pdc, -80, -20);
	ArcTo(pdc, -50, -50, TRUE, 30);

	OSTimeDly(3000);//将任务挂起3秒
	ClearScreen();
	SetDrawOrg(pdc, 0, LCDHEIGHT/2, &oldx,&oldy);//设置绘图原点为屏幕左边中部
	for(;;)
	{
		MoveTo(pdc, 0, 0);
		for(x=0;x<LCDWIDTH;x++)
		{//画正弦波
			y=(int)(50*sin(((double)x)/20.0+offset));
			LineTo(pdc, x, y);
		}
		offset+=1;
		if(offset>=2*3.14)
			offset=0;
		OSTimeDly(1000);
		ClearScreen();
	}
	
DestoryDC(pdc);//删除绘图设备上下文
}
