#include"../ucos-ii/includes.h"               /* uC/OS interface */
#include "../ucos-ii/add/osaddition.h"
#include "../inc/drivers.h"
#include "../inc/macro.h"
#include "../inc/sys/lib.h"

//#include "../inc/drv/tchScr.h"

#include <stdio.h>



OS_STK SYS_Task_Stack[STACKSIZE]= {0, }; //system task刷新任务堆栈
#define SYS_Task_Prio				1
static void SYS_Task(void *Id);

/***************************************

	系统任务，有最高的优先级(1)，启动系统的其他任务

****************************************/
static void SYS_Task(void *Id)
{
	OSRunning=TRUE;	//begin OS

	uHALr_InstallSystemTimer();
//	OpenUartRev(0);

	printk("start system task.\n");

//#if OS_KeyBoard_Scan_Task==1
//	KeyBoard_init();
//#endif

	/*if(!Id)	//don't use cmd line
		OSTaskDel(OS_PRIO_SELF);

	for(;;)
		Console_input(1, "Console_input");*/
		for (;;)
	{
		OSTimeDly(1000);
	}
}


void OSAddTask_Init(int use_cmd)
{

//#if OS_Touch_Screen_Task==1
//	OSTaskCreate(Touch_Screen_Task,  (void *)0,  (OS_STK *)&Touch_Screen_Stack[STACKSIZE-1],  Touch_Screen_Task_Prio);
//#endif

//#if OS_KeyBoard_Scan_Task==1
//	Key_init();
//#endif

//#if (DIRECT_DISPLAY==0) && (USE_MINIGUI==0)
//	OSTaskCreate(Lcd_Fresh_Task,  (void *)0,  (OS_STK *)&Lcd_Fresh_Stack[STACKSIZE-1],  Lcd_Fresh_Task_Prio);
//	LCDFresh_MBox=OSMboxCreate(NULL);//创建LCD刷新邮箱
//	Lcd_Disp_Sem=OSSemCreate(1);  //创建LCD缓冲区控制权旗语,初值为1满足互斥条件//
//#endif

	OSTaskCreate(SYS_Task,  (void *)use_cmd,  (OS_STK *)&SYS_Task_Stack[STACKSIZE-1],  SYS_Task_Prio);

}


