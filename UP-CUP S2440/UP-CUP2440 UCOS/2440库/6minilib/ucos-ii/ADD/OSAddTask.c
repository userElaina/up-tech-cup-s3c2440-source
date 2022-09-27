#include"../ucos-ii/includes.h"               /* uC/OS interface */
#include "../ucos-ii/add/osaddition.h"
#include "../inc/drivers.h"
#include "../inc/macro.h"
#include "../inc/sys/lib.h"

#include "../inc/drv/tchScr.h"

#include <stdio.h>

#if OS_Touch_Screen_Task==1
	OS_STK Touch_Screen_Stack[STACKSIZE]={0, };   //Touch_Screen_Task��ջ
	#define Touch_Screen_Task_Prio      8
#endif

OS_STK SYS_Task_Stack[STACKSIZE]= {0, }; //system taskˢ�������ջ
#define SYS_Task_Prio				1
static void SYS_Task(void *Id);


#if (DIRECT_DISPLAY==0) && (USE_MINIGUI==0)
OS_STK Lcd_Fresh_Stack[STACKSIZE]= {0, }; //LCDˢ�������ջ
static void Lcd_Fresh_Task(void *Id);            //LCDˢ������
#define Lcd_Fresh_Task_Prio           59
OS_EVENT *LCDFresh_MBox;		//LCDˢ������
#endif


///*****************�¼�����*****************///
OS_EVENT *Lcd_Disp_Sem;                   //LCD����Ȩ����
//and you can use it as folloeing:
//    Lcd_Disp_Sem=OSSemCreate(1);        //����LCD����Ȩ����,��ֵΪ1���㻥������//
//    OSSemPend(Lcd_Disp_Sem,0,&err);
//    OSSemPost(Lcd_Disp_Sem);

/***************************************

	ϵͳ��������ߵ����ȼ�(1)������ϵͳ����������

****************************************/
static void SYS_Task(void *Id)
{
	OSRunning=TRUE;	//begin OS

	uHALr_InstallSystemTimer();
	OpenUartRev(0);

	printk("start system task.\n");

#if OS_KeyBoard_Scan_Task==1
	Key_init();
#endif

	if(!Id)	//don't use cmd line
		OSTaskDel(OS_PRIO_SELF);

	for(;;)
		Console_input(1, "Console_input");
}


void OSAddTask_Init(int use_cmd)
{

#if OS_Touch_Screen_Task==1
	OSTaskCreate(Touch_Screen_Task,  (void *)0,  (OS_STK *)&Touch_Screen_Stack[STACKSIZE-1],  Touch_Screen_Task_Prio);
#endif

#if OS_KeyBoard_Scan_Task==1
//	Key_init();
#endif

#if (DIRECT_DISPLAY==0) && (USE_MINIGUI==0)
	OSTaskCreate(Lcd_Fresh_Task,  (void *)0,  (OS_STK *)&Lcd_Fresh_Stack[STACKSIZE-1],  Lcd_Fresh_Task_Prio);
	LCDFresh_MBox=OSMboxCreate(NULL);//����LCDˢ������
	Lcd_Disp_Sem=OSSemCreate(1);  //����LCD����������Ȩ����,��ֵΪ1���㻥������//
#endif

	OSTaskCreate(SYS_Task,  (void *)use_cmd,  (OS_STK *)&SYS_Task_Stack[STACKSIZE-1],  SYS_Task_Prio);

}

#if (DIRECT_DISPLAY==0) && (USE_MINIGUI==0)
static void Lcd_Fresh_Task(void *Id)
{
	INT8U err;
	printk("begin Lcd Fresh Task\n");
	for (;;)
	{
		OSMboxPend(LCDFresh_MBox,0,&err);
		LCD_Refresh();
		OSTimeDly(250);
	}
}//Lcd_Fresh_Task
#endif
