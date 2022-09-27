/***************************************************************************\
	Copyright (c) 2004-2007 threewater@up-tech.com, All rights reserved.
	by threewter	2004.4.26
\***************************************************************************/


/***************************************************************************\
    #说明: 命令行和菜单函数定义
    			不要添加到工程中，已经被
    			/src/command/console.c include
	----------------------------------  Bug  --------------------------------------

	----------------------------------  TODO list  --------------------------------------

	----------------------------------修正--------------------------------------
	2004-5-2	1、创建

\***************************************************************************/
#include "../inc/drivers.h"
#include "../inc/sys/lib.h"
#include "../inc/command/filecmd.h"

#if 0
/*------------------------bootloader function-------------------------------------*/
void EnterUsbDisk()
{
	Set_UartLoopFunc(USB_Loop);

	reconnect_USB();
	Uart_Printf("\nEntering USB Disk state,please waiting...");
}

void BootSystem()
{
	Boot(NULL);
}

cmd_function Mifconfig_func={
		SetNetWork,
		"Show and Set Net address",
		'n',
		"mifconfig",
		NULL
		};

cmd_function EnterUSB_func={
		EnterUsbDisk,
		"Enter USB Disk",
		'u',
		"usbstart",
		NULL
		};

cmd_function ADTest_func={
		ADTest,
		"Enter Test ADC Mode",
		'a',
		"testad",
		NULL
		};

cmd_function TchScr_func={
		TchScr_Test,
		"Test Touch pad",
		's',
		"testtp",
		NULL
		};

cmd_function TchScrSet_func={
		SetTouchScr,
		"Set Touch pad Mode",
		'h',
		"settp",
		NULL
		};

cmd_function DATest_func={
		DA_Test,
		"Enter Test DA Mode",
		'd',
		"testDA",
		NULL
		};

cmd_function RTCTest_func={
		RTC_Test,
		"Enter RTC Test Mode",
		't',
		"time",
		NULL
		};

cmd_function AudioTest_func={
		Audio_Test,
		"Enter Audio Test Mode",
		'i',
		"audio",
		NULL
		};

cmd_function MotorTest_func={
		Motor_Test,
		"Enter Motor Test Mode",
		'm',
		"motor",
		NULL
		};

cmd_function CAN_func={
		CAN_Test,
		"Enter CAN bus Test Mode",
		'c',
		"can",
		NULL
		};

cmd_function boot_func={
		BootSystem,
		"Boot default system",
		'b',
		"boot",
		NULL
		};
#endif

static int LED_Test(int argc, char *argv[])
{
	//fixed me!
	char latter[]={1,2,3,4,5,6,7,8};

	ZLG7290_SetLEDS(latter);

	return 0;
}

static cmd_function LED_Test_func={
		LED_Test,
		"Test LED",
		'e',
		"LEDtest",
		NULL
		};


static int LCD_Test(int argc, char *argv[])
{
	printf("Please look at LCD.\n");
	printf("Press any key to enter LCD graph mode.\n");

	LCD_printf("Hello.\n");
	LCD_printf("This is LCD text Mode\n");
	LCD_printf("Please wait.....\n");
	LCD_printf("Then show LCD gray bar.\n");
	LCD_printf("Press any key to enter LCD graph mode.\n");
	getchar();

	printf("Press any key return.\n");
	LCD_TestShow();
	getchar();
	
	return OK;
}

//extern void tchscr_calibrate_test(void);
static int Tchscr_Set(int argc, char *argv[])
{
//	tchscr_calibrate_test();
	return OK;
}


static cmd_function LCD_Test_func={
		LCD_Test,
		"Test LCD",
		'l',
		"Lcdtest",
		NULL
		};

cmd_function Flash_Tools_func={
		Flash_Tools,
		"Flash Rom Tools",
		'f',
		"menuflash",
		NULL
		};

cmd_function Tchscr_set_func={
		Tchscr_Set,
		"Set touch screeen",
		's',
		"settch",
		NULL
		};
/*
static cmd_function Format_func={
		FormatNandFlash,
		"Format Nand Flash To FAT",
		'o',
		"format",
		NULL
		};
*/
/*---------------------------------------------------------------------------------*\
	文件系统的命令行定义
\*---------------------------------------------------------------------------------*/
static cmd_function ls_func={
		ls_callback,
		NULL,
		0,
		"ls",
		NULL
		};
static cmd_function dl_func={
		dl_callback,
		NULL,
		0,
		"dl",
		NULL
		};
static cmd_function ul_func={
		ul_callback,
		NULL,
		0,
		"ul",
		NULL
		};

static cmd_function cat_func={
		cat_callback,
		NULL,
		0,
		"cat",
		NULL
		};

static cmd_function rmdir_func={
		rmdir_callback,
		NULL,
		0,
		"rmdir",
		NULL
		};

static cmd_function rm_func={
		rm_callback,
		NULL,
		0,
		"rm",
		NULL
		};

static cmd_function mv_func={
		mv_callback,
		NULL,
		0,
		"mv",
		NULL
		};

static cmd_function mkdir_func={
		mkdir_callback,
		NULL,
		0,
		"mkdir",
		NULL
		};

static cmd_function cd_func={
		cd_callback,
		NULL,
		0,
		"cd",
		NULL
		};

/*---------------------------------------------------------------------------------*\
	系统定义的命令行
\*---------------------------------------------------------------------------------*/
static cmd_function Console_input_func={
		Console_input,
		"Enter into command line",
		'c',
		"cmd",
		NULL
		};

static cmd_function help_func={
		help_callback,
		NULL,
		0,
		"help",
		NULL
		};

static cmd_function echo_func={
		echo_callback,
		NULL,
		0,
		"echo",
		NULL
		};

extern cmd_function Flash_Tools_func;
static cmd_function* sysfunc[]={&Console_input_func, &help_func, &echo_func, 
			//硬件测试
			&LED_Test_func, 
			//下载程序，启动的命令
	//		&load_func, &flashsystem_func, &boot_func,
			//文件系统操作函数
			&ls_func, &dl_func, &ul_func, &cat_func, &rm_func, &mv_func, &cd_func, &mkdir_func, &rmdir_func, 
			//用户测试函数
			&Flash_Tools_func, &LCD_Test_func, &Tchscr_set_func};

#define SYSFUNCNUM		(sizeof(sysfunc)/sizeof(cmd_function*))

