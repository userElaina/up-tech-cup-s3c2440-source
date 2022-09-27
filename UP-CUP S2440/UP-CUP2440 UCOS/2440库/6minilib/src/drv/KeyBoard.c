/***************************************************************************\
	Copyright (c) 2004-2007 threewater@up-tech.com, All rights reserved.
	by threewter	2004.4.26
\***************************************************************************/
	

/***************************************************************************\
    #说明: 键盘驱动
	----------------------------------  Bug  --------------------------------------

	----------------------------------  TODO list  --------------------------------------

	----------------------------------修正--------------------------------------
	2004-5-9	创建

\***************************************************************************/
#include "includes.h"

#include "../inc/drv/reg2410.h"
#include "../inc/drv/zlg7290.h"
#include "../inc/sys/lib.h"
#include "../inc/macro.h"
#include "../src/miniGUI/uhal/ucos2input.h"
#include <stdio.h>
#include <string.h>

#undef DPRINTK

#undef DEBUG
//#define DEBUG
#ifdef DEBUG
#define DPRINTK			printf
#else
#define DPRINTK			printfNULL
#endif

#define KBDTYPE_ZLG7290	1
#define KBDTYPE_MCUIIC		2
#define KEYBOARD_TYPE		KBDTYPE_MCUIIC

#if OS_KeyBoard_Scan_Task==1

static OS_STK Key_Scan_Stack[STACKSIZE]={0, };   //Key_Test_Task堆栈
static void Key_Scan_Task(void *Id);             //键盘扫描任务
#define Key_Scan_Task_Prio      10

static void KeyMap_init(void);

OS_FLAG_GRP *Input_Flag=NULL;

#if KEYBOARD_TYPE==KBDTYPE_ZLG7290

#define IRQ_KBD		IRQ_EINT4

typedef struct{
	unsigned char key;
	unsigned char RepeatCnt;
	unsigned char Funckey;
}Zlg7290Key;

static unsigned short KeyBoard_Map[64] ;

typedef struct{
	unsigned short int keyraw;
	unsigned short int map2;
}KeyBoard_Map2;

#if USE_MINIGUI==1
#include <miniGUI/common.h>
static KeyBoard_Map2 KeyBoard_MapArray[]={{17, SCANCODE_NUMLOCK},
		{21, SCANCODE_KEYPADDIVIDE},{19, SCANCODE_KEYPADMULTIPLY},
		{11, SCANCODE_KEYPADMINUS},{49, SCANCODE_KEYPAD7},{53, SCANCODE_KEYPAD8},
		{51, SCANCODE_KEYPAD9},{41, SCANCODE_KEYPAD4},{45, SCANCODE_KEYPAD5},
		{43, SCANCODE_KEYPAD6},{52, SCANCODE_KEYPADPLUS},{33, SCANCODE_KEYPAD1},
		{37, SCANCODE_KEYPAD2},{35, SCANCODE_KEYPAD3},{36, SCANCODE_KEYPADENTER},
		{29, SCANCODE_KEYPAD0},{27, SCANCODE_KEYPADPERIOD},};
#else

static KeyBoard_Map2 KeyBoard_MapArray[]={{17, 'N'},{21, '/'},{19, '*'},{11, '-'},
		{49, '7'},{53, '8'},{51, '9'},{41, '4'},{45, '5'},{43, '6'},{52, '+'},
		{33, '1'},{37, '2'},{35, '3'},{36, '\r'},{29, '0'},{27, '.'},};

#endif //#if USE_MINIGUI==1

static void Key_ISR(int vector, void* data)
{
	INT8U err;
	OSFlagPost(Input_Flag, UCOS2_KBINPUT, OS_FLAG_SET, &err);
}

void Key_init(void)
{
	INT8U err;
	int ret;

	KeyMap_init();

	IIC_init();
	set_external_irq(IRQ_KBD, EXT_FALLING_EDGE, GPIO_PULLUP_EN);
	
	Set_IIC_mode(ZLG7289_IICCON, NULL);
	ret=IIC_Read(ZLG7290_ADDR, 0);

	DPRINTK("zlg7290 system register=0x%x\n", ret);

	if(!Input_Flag)
		Input_Flag=OSFlagCreate(0, &err);

	SetISR_Interrupt(IRQ_KBD, Key_ISR, NULL);

#if USE_MINIGUI==0
	OSTaskCreate(Key_Scan_Task,  (void *)0,  (OS_STK *)&Key_Scan_Stack[STACKSIZE-1],  Key_Scan_Task_Prio);
#endif
}

//读取键盘按键扫描码,如果没有按键，则返回-1
//参数: ndev,设备号，isBlock是否阻塞
int KeyBoard_Read(int ndev, BOOL isBlock)
{
	Zlg7290Key key;
	static unsigned char lastkey=0;

	IIC_ReadSerial(ZLG7290_ADDR, 1, (unsigned char*)&key, 3);

	if(key.RepeatCnt==0 && key.Funckey!=0xff)
		return -1;
	if(key.key>=NumberOfArray(KeyBoard_Map))
		return -1;

	DPRINTK("key=0x%x, repeat=%d, func=0x%x\n", key.key, key.RepeatCnt, key.Funckey);

	if(key.key!=0){
		lastkey=KeyBoard_Map[key.key];
		return lastkey|KEY_DOWN;
	}

	return lastkey;
}

#elif KEYBOARD_TYPE==KBDTYPE_MCUIIC

#include "../inc/drv/i2c-tomega8.h"

#define IRQ_KBD		IRQ_IIC
//#define MCU_IICCON		(IICCON_ACKEN |IICCON_CLK512 | IICCON_INTR | IICCON_CLKPRE(0x3))
#define MCU_IICCON		(IICCON_ACKEN  | IICCON_INTR | IICCON_CLKPRE(7))

static unsigned short KeyBoard_Map[256];
static unsigned short mcukey;

typedef struct{
	unsigned short keyraw;
	unsigned short map2;
}KeyBoard_Map2;

#if USE_MINIGUI==1
#include <miniGUI/common.h>
static KeyBoard_Map2 KeyBoard_MapArray[]={{0x2, SCANCODE_NUMLOCK},
		{0xa, SCANCODE_KEYPADDIVIDE},{0x12, SCANCODE_KEYPADMULTIPLY},
		{0x11, SCANCODE_KEYPADMINUS},{0x5, SCANCODE_KEYPAD7},{0xd, SCANCODE_KEYPAD8},
		{0x15, SCANCODE_KEYPAD9},{0x6, SCANCODE_KEYPAD4},{0xe, SCANCODE_KEYPAD5},
		{0x16, SCANCODE_KEYPAD6},{0x1d, SCANCODE_KEYPADPLUS},{0x7, SCANCODE_KEYPAD1},
		{0xf, SCANCODE_KEYPAD2},{0x17, SCANCODE_KEYPAD3},{0x1f, SCANCODE_KEYPADENTER},
		{0x10, SCANCODE_KEYPAD0},{0x18, SCANCODE_KEYPADPERIOD},};
#else

static KeyBoard_Map2 KeyBoard_MapArray[]={{0x2, 'N'},{0xa, '/'},{0x12, '*'},{0x11, '-'},
		{0x5, '7'},{0xd, '8'},{0x15, '9'},{0x6, '4'},{0xe, '5'},{0x16, '6'},{0x1d, '+'},
		{0x7, '1'},{0xf, '2'},{0x17, '3'},{0x1f, '\r'},{0x10, '0'},{0x18, '.'},};

#endif //#if USE_MINIGUI==1

static void Key_ISR(int vector, void* data)
{
	static int framecnt=0;
	INT8U err;
	U8 status ,kdata;
	status = rIICSTAT ;

	if( (status & IICSTAT_MODE_MSK) != IICSTAT_MODE_SR)
		return;
	//must slave receive mode

	kdata = rIICDS;

	switch(framecnt){
	case 0:
		if(status & IICSTAT_SLAVEADDR){
			//match the own slave address. begin to slave receive data.
			DPRINTK("key irq: 111\n");
			framecnt++;
		}
		break;
	case 1:
		if(GetI2C_Devtype(kdata) != DTYPE_MKEYB){
			DPRINTK("key irq: not key farme\n");
			framecnt=0;
		}
		framecnt++;
		break;
	case 2:
		mcukey = kdata;
		DPRINTK("key irq: key=0x%x\n", mcukey);
		OSFlagPost(Input_Flag, UCOS2_KBINPUT, OS_FLAG_SET, &err);
		framecnt=0;
		break;
	}

	rIICCON &= ~IICCON_INTPEND;
}

void Key_init(void)
{
	INT8U err;
	I2C_control i2c_ctrl;

	KeyMap_init();

	IIC_init();
	Set_IIC_mode(MCU_IICCON, NULL);

	//set owner i2c address to mega8
	i2c_ctrl.cmd=CMD_MST_ADDR;
	i2c_ctrl.ctrl=I2COWNER_ADDRESS;
	IIC_Send(MEGA8_I2CADDRESS, (const char*)&i2c_ctrl, sizeof(i2c_ctrl));

	//enable keyboard
	i2c_ctrl.cmd=CMD_CTRL;
	i2c_ctrl.ctrl = CTRL_MKEn;
	IIC_Send(MEGA8_I2CADDRESS, (const char*)&i2c_ctrl, sizeof(i2c_ctrl));

//	err = IIC_Read(MEGA8_I2CADDRESS, CMD_MST_ADDR);
//	printf("master address=0x%x\n", err);
//	err = IIC_Read(MEGA8_I2CADDRESS, CMD_CTRL);
//	printf("ctrl=0x%x\n", err);

	if(!Input_Flag)
		Input_Flag=OSFlagCreate(0, &err);

	SetISR_Interrupt(IRQ_KBD, Key_ISR, NULL);

#if USE_MINIGUI==0
	OSTaskCreate(Key_Scan_Task,  (void *)0,  (OS_STK *)&Key_Scan_Stack[STACKSIZE-1],  Key_Scan_Task_Prio);
#endif
}

//读取键盘按键扫描码,如果没有按键，则返回-1
//参数: ndev,设备号，isBlock是否阻塞
int KeyBoard_Read(int ndev, BOOL isBlock)
{
	if(mcukey&0x80){
		//key down
		return KeyBoard_Map[mcukey&0x7f]|KEY_DOWN;
	}

	return KeyBoard_Map[mcukey&0x7f];
}

#else

#error "no keyboard support!"
#endif



//initializition map board map table
static void KeyMap_init(void)
{
	int i;

	memset(KeyBoard_Map, -1, sizeof(KeyBoard_Map));

	for(i=0; i<NumberOfArray(KeyBoard_MapArray); i++){
		KeyBoard_Map[KeyBoard_MapArray[i].keyraw]=KeyBoard_MapArray[i].map2;
	}
}

#if USE_MINIGUI==0
static void Key_Scan_Task(void *Id)
{
	U32 key;
	INT8U err;
	POSMSG pmsg;

	printk("begin key task \n");

	for (;;){
		OS_FLAGS flag;

		flag=OSFlagPend(Input_Flag, UCOS2_KBINPUT, OS_FLAG_WAIT_SET_ANY, 0, &err);
		OSFlagPost(Input_Flag, flag, OS_FLAG_CLR, &err);
		key=KeyBoard_Read(0,FALSE);
		if(key==-1)
			continue;

		pmsg=OSCreateMessage(NULL, OSM_KEY,key,0);
		if(pmsg)
			SendMessage(pmsg);
	}
}
#endif

#endif	//#if OS_KeyBoard_Scan_Task==1

