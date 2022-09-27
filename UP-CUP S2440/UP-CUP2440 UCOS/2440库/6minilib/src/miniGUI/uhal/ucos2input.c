/***************************************************************************\
	Copyright (c) 2004-2009 threewater@up-tech.com, All rights reserved.
	by threewter	2004.4.26
\***************************************************************************/
	

/***************************************************************************\
    #说明: miniGUI键盘、触摸屏输入程序
	----------------------------------  Bug  --------------------------------------

	----------------------------------  TODO list  --------------------------------------

	----------------------------------修正--------------------------------------
	2004-8-12	创建，测试通过

\***************************************************************************/
#include <miniGUI/common.h>
#include "includes.h"
#include "ucos2input.h"
#include "../ucos-ii/includes.h"
#include "../inc/sys/lib.h"
#include <stdio.h>


//#define DPRINTF(x...)		Uart_Printf(1,x...)
#define DPRINTF		printfNULL

#if USE_MINIGUI==1

#if OS_Touch_Screen_Task==1
extern tsdata tchscr_data;
#endif
extern OS_FLAG_GRP *Input_Flag;

int ucos2_ts_getdata (short *x, short *y, short *button)
{
#if OS_Touch_Screen_Task==1
	*x=tchscr_data.x;
	*y=tchscr_data.y;
	if(tchscr_data.action==TCHSCR_ACTION_DOWN ||tchscr_data.action==TCHSCR_ACTION_MOVE)
		*button=1;

	DPRINTF("x=%d, y=%d, button=%d\n", *x, *y,*button);
#endif

	return 0;
}

int ucos2_kb_getdata (short *key, short *status)
{
	int keyraw=KeyBoard_Read(0, FALSE);

	if(keyraw==-1)
		return -1;

	if(keyraw&KEY_DOWN){
		*status=1;
		*key = (keyraw&(~KEY_DOWN));
	}
	else{
		*status=0;
		*key = keyraw;
	}
	DPRINTF("key=%d, status=%d\n", *key, *status);

	return 0;
}

int ucos2_wait_for_input (void)
{
	INT8U err;
	OS_FLAGS flag;

	flag=OSFlagPend(Input_Flag, UCOS2_KBINPUT|UCOS2_MOUSEINPUT, OS_FLAG_WAIT_SET_ANY, 	0, &err);
	OSFlagPost(Input_Flag, flag, OS_FLAG_CLR, &err);

	DPRINTF("receive input: flag=0x%x\n", flag);

	return flag;//OSFlagPendGetFlagsRdy();
}
#endif	//#if USE_MINIGUI==1

