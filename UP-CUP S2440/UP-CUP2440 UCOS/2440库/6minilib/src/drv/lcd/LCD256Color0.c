/***************************************************************************\
	Copyright (c) 2004-2007 threewater@up-tech.com, All rights reserved.
	by threewter	2004.4.26
\***************************************************************************/
	

/***************************************************************************\
    #说明: 16级灰度液晶屏驱动程序，工程机械用液晶板20Pin 2.54接口
    #接口函数
		void LCD_pInit();

		void LCDBkLight(U8 isOpen);
		void LCDDisplayOpen(U8 isOpen);
	----------------------------------  Bug  --------------------------------------

	----------------------------------  TODO list  --------------------------------------

	----------------------------------修正--------------------------------------
	2004-5-9	创建

\***************************************************************************/

/***************************************************************************\
	私有的LCD初始化函数,在LCD_Init()设置LCD寄存器之后调用
	可以初始化时序等信息
\***************************************************************************/
void LCD_pInit(void)
{
	rLCDCON1 = LCD1_BPP_8S | LCD1_PNR_8S | LCD1_CLKVAL(10);
	rLCDCON2 = LCD2_LINEVAL(LCDHEIGHT-1);
	rLCDCON3 = LCD3_WDLY_32 | LCD3_LINEBLNK(30)|LCD3_HOZVAL(LCDWIDTH*3/8 - 1);
	rLCDCON4 = LCD4_WLH_32;
	rLCDCON5 = LCD5_BSWP;

	rLCDADDR1 = LCDADDR_BANK(((unsigned long)LCDFrameBuffer >> 22))
		| LCDADDR_BASEU(((unsigned long)LCDFrameBuffer >> 1));
	rLCDADDR2 = LCDADDR_BASEL((unsigned long)LCDFrameBuffer + LCDFBSIZE)>> 1;
	rLCDADDR3 = LCDADDR_OFFSET(0) | LCDADDR_PAGE(LCDWIDTH*LCDBPP/16);

	rDITHMODE=0x0;
	rREDLUT = 0xfdb96420;
	rGREENLUT = 0xfdb96420;
	rBLUELUT = 0xfb40;
}

/***************************************************************************\
	液晶屏被光开关函数
\***************************************************************************/
void LCDBkLight(int isOpen)
{

}

/***************************************************************************\
	液晶屏显示开关函数
\***************************************************************************/
void LCDDisplayOpen(int isOpen)
{
	if(isOpen)
		rLCDCON5 |= LCD5_PWREN;
	else
		rLCDCON5 &= ~LCD5_PWREN;
}

