/***************************************************************************\
	Copyright (c) 2004-2007 threewater@up-tech.com, All rights reserved.
	by threewter	2004.4.26
\***************************************************************************/
	

/***************************************************************************\
    #说明: miniGUI 的LCD初始化和接口函数
	----------------------------------  Bug  --------------------------------------

	----------------------------------  TODO list  ----------------------------------

	----------------------------------修正--------------------------------------
	2004-8-12	创建，测试通过

	----------------------------------使用说明--------------------------------

\***************************************************************************/
#include <miniGUI/ucos2lcd.h>
#include "../ucos-ii/includes.h"
#include <stdio.h>

#if USE_MINIGUI==1

static void* pLCDbuffer=NULL;

int lcd_init (void)
{
	pLCDbuffer=LCD_Init();

	return 0;
}

int lcd_getinfo (struct lcd_info *li)
{
	if(pLCDbuffer==NULL){
		printf("Wrong point to LCD buffer\n");
	}

    li->width = LCDWIDTH;
    li->height = LCDHEIGHT;
    li->bpp = LCDBPP;
    li->rlen = LCD_RLEN;
    li->fb = (void *)pLCDbuffer;
#if (LCDBPP==8)
    li->type = FB_TRUE_RGB332;
#elif (LCDBPP==1) ||(LCDBPP==2) ||(LCDBPP==4)
    li->type = 0;	/* Most significant bit is left */
#elif(LCDBPP==16)
    li->type = FB_TRUE_RGB332;
    li->bpp = 8;
#endif
    return 0;
}

#endif //#if USE_MINIGUI==1

