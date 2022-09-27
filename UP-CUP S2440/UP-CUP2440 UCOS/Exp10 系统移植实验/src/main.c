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
/*OS_STK SYS_Task_Stack[STACKSIZE]= {0, }; //system task刷新任务堆栈
#define SYS_Task_Prio				1
 void SYS_Task(void *Id);*/

OS_STK task1_Stack[STACKSIZE]={0, };   //Main_Test_Task堆栈
void Task1(void *Id);             //Main_Test_Task
#define Task1_Prio     12

OS_STK task2_Stack[STACKSIZE]={0, };   //test_Test_Task堆栈
void Task2(void *Id);             //test_Test_Task
#define Task2_Prio     15



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

	//OSInitUart();

	//initOSFile();
//#if USE_MINIGUI==0
//	initOSMessage();
//	initOSDC();
//	LoadFont();
//#endif

	//loadsystemParam();
	
	// create the tasks in uC/OS and assign increasing //
	// priorities to them so that Task3 at the end of  //
	// the pipeline has the highest priority.          //
	//LCD_printf("Create task on uCOS-II...\n");
	
	//OSTaskCreate(SYS_Task,  (void *)0,  (OS_STK *)&SYS_Task_Stack[STACKSIZE-1],  SYS_Task_Prio);
	OSTaskCreate(Task1,  (void *)0,  (OS_STK *)&task1_Stack[STACKSIZE-1],  Task1_Prio);
	OSTaskCreate(Task2,  (void *)0,  (OS_STK *)&task2_Stack[STACKSIZE-1],  Task2_Prio);
	

	OSAddTask_Init(0);
  
	//LCD_printf("Starting uCOS-II...\n");
	//LCD_printf("Entering graph mode...\n");
	//LCD_ChangeMode(DspGraMode);

	OSStart();              // start the OS //

	// never reached //
	return 0;
}//main


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Task1(void *Id)
{
	

	for(;;){
		printf("run task1\n");
		OSTimeDly(1000);
	}
}
void Task2(void *Id)
{
	

	for(;;){
		printf("run task2\n");
		OSTimeDly(3000);
	}
}

/* void SYS_Task(void *Id)
{
	OSRunning=TRUE;	//begin OS

	uHALr_InstallSystemTimer();
	//Uart_Printf(0,"start system task.\n");
	printf("start system task.\n");
	for (;;)
	{
		OSTimeDly(1000);
	}
*/










