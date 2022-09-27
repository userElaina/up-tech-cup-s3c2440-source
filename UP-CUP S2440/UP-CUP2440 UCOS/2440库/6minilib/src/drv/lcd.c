/***************************************************************************\
	Copyright (c) 2004-2007 threewater@up-tech.com, All rights reserved.
	by threewter	2004.4.26
\***************************************************************************/
	

/***************************************************************************\
    #说明: 液晶屏驱动程序
    #接口函数
		void LCD_Init();
	----------------------------------  Bug  --------------------------------------

	----------------------------------  TODO list  --------------------------------------

	----------------------------------修正--------------------------------------
	2004-5-9	创建

\***************************************************************************/
#include "../inc/drv/reg2410.h"
#include "../inc/sys/lib.h"
#include "../inc/macro.h"
#include "../inc/drv/lcd.h"
#include <stdio.h>
#include <string.h>

#define fLCD1_LINECNT		Fld(10,18)	/* the status of the line counter */
#define LCD1_LINECNT		FMsk(fLCD1_LINECNT)

#define fLCD1_CLKVAL		Fld(10,8)	/* rates of VCLK and CLKVAL[9:0] */
#define LCD1_CLKVAL(x)		FInsrt((x), fLCD1_CLKVAL)
#define LCD1_CLKVAL_MSK	FMask(fLCD1_CLKVAL)

#define LCD1_MMODE (1<<7)

#define fLCD1_PNR			Fld(2,5)	/* select the display mode */
#define LCD1_PNR_4D			FInsrt(0x0, fLCD1_PNR)	/* STN: 4-bit dual scan */
#define LCD1_PNR_4S			FInsrt(0x1, fLCD1_PNR)	/* STN: 4-bit single scan */
#define LCD1_PNR_8S			FInsrt(0x2, fLCD1_PNR)	/* STN: 8-bit single scan */
#define LCD1_PNR_TFT		FInsrt(0x3, fLCD1_PNR)	/* TFT LCD */

#define fLCD1_BPP			Fld(4,1)	/* select BPP(Bit Per Pixel) */
#define LCD1_BPP_1S			FInsrt(0x0, fLCD1_BPP)	/* STN: 1 bpp, mono */
#define LCD1_BPP_2S			FInsrt(0x1, fLCD1_BPP)	/* STN: 2 bpp, 4-grey */
#define LCD1_BPP_4S			FInsrt(0x2, fLCD1_BPP)	/* STN: 4 bpp, 16-grey */
#define LCD1_BPP_8S			FInsrt(0x3, fLCD1_BPP)	/* STN: 8 bpp, color */
#define LCD1_BPP_12S		FInsrt(0x4, fLCD1_BPP)	/* STN: 12 bpp, color */
#define LCD1_BPP_1T			FInsrt(0x8, fLCD1_BPP)	/* TFT: 1 bpp */
#define LCD1_BPP_2T			FInsrt(0x9, fLCD1_BPP)	/* TFT: 2 bpp */
#define LCD1_BPP_4T			FInsrt(0xa, fLCD1_BPP)	/* TFT: 4 bpp */
#define LCD1_BPP_8T			FInsrt(0xb, fLCD1_BPP)	/* TFT: 8 bpp */
#define LCD1_BPP_16T		FInsrt(0xc, fLCD1_BPP)	/* TFT: 16 bpp */

#define LCD1_ENVID			(1 << 0)	/* 1: Enable the video output */

/* TFT: (Vertical Back Porch) of inactive lines at the start of a frame,
	   after vertical synchronization period. */
#define fLCD2_VBPD			Fld(8,24)
#define LCD2_VBPD(x)			FInsrt((x), fLCD2_VBPD)

#define fLCD2_LINEVAL		Fld(10,14)	/* TFT/STN: vertical size of LCD */
#define LCD2_LINEVAL(x)		FInsrt((x), fLCD2_LINEVAL)
#define LCD2_LINEVAL_MSK	FMsk(fLCD2_LINEVAL)

/* TFT: (Vertical Front Porch) of inactive lines at the end of a frame,
	   before vertical synchronization period. */
#define fLCD2_VFPD			Fld(8,6)	
#define LCD2_VFPD(x)			FInsrt((x), fLCD2_VFPD)

/* TFT: (Vertical Sync Pulse Width) the VSYNC pulse's high level width
	   by counting the # of inactive lines */
#define fLCD2_VSPW			Fld(6,0)	
#define LCD2_VSPW(x)		FInsrt((x), fLCD2_VSPW)

/* TFT: (Horizontal Back Porch) of VCLK periods between the falling edge of HSYNC
	   and the start of active data */
#define fLCD3_HBPD			Fld(7,19)
#define LCD3_HBPD(x)		FInsrt((x), fLCD3_HBPD)

/* STN: delay between VLINE and VCLK by counting the # of the HCLK */
#define fLCD3_WDLY			Fld(7,19)	
#define LCD3_WDLY			FMsk(fLCD3_WDLY)
#define LCD3_WDLY			FMsk(fLCD3_WDLY)
#define LCD3_WDLY_16		FInsrt(0x0, fLCD3_WDLY)	/* 16 clock */
#define LCD3_WDLY_32		FInsrt(0x1, fLCD3_WDLY)	/* 32 clock */
#define LCD3_WDLY_64		FInsrt(0x2, fLCD3_WDLY)	/* 64 clock */
#define LCD3_WDLY_128		FInsrt(0x3, fLCD3_WDLY)	/* 128 clock */

#define fLCD3_HOZVAL		Fld(11,8)	/* horizontal size of LCD */
#define LCD3_HOZVAL(x)		FInsrt((x), fLCD3_HOZVAL)
#define LCD3_HOZVAL_MSK	FMsk(fLCD3_HOZVAL)

/* TFT: (Horizontal Front Porch) of VCLK periods between the end of active date
	   and the rising edge of HSYNC */
#define fLCD3_HFPD			Fld(8,0)	
#define LCD3_HFPD(x)		FInsrt((x), fLCD3_HFPD)

/* STN: the blank time in one horizontal line duration time.  the unit of LINEBLNK is HCLK x 8 */
#define fLCD3_LINEBLNK		Fld(8,0)	
#define LCD3_LINEBLNK(x)	FInsrt((x),fLCD3_LINEBLNK)

/* STN: the rate at which the VM signal will toggle if the MMODE bit is set logic '1' */
#define fLCD4_MVAL			Fld(8,8)
#define LCD4_MVAL(x)		FInsrt((x), fLCD4_MVAL)

// TFT: (Horizontal Sync Pulse Width) HSYNC pulse's high lvel width by counting the # of the VCLK
#define fLCD4_HSPW			Fld(8,0)
#define LCD4_HSPW(x)		FInsrt((x), fLCD4_HSPW)

// STN: VLINE pulse's high level width by counting the # of the HCLK
#define fLCD4_WLH			Fld(8,0)	
#define LCD4_WLH(x)			FInsrt((x), fLCD4_WLH)
#define LCD4_WLH_16		FInsrt(0x0, fLCD4_WLH)	/* 16 clock */
#define LCD4_WLH_32		FInsrt(0x1, fLCD4_WLH)	/* 32 clock */
#define LCD4_WLH_64		FInsrt(0x2, fLCD4_WLH)	/* 64 clock */
#define LCD4_WLH_128		FInsrt(0x3, fLCD4_WLH)	/* 128 clock */

#define fLCD5_VSTAT			Fld(2,19)	/* TFT: Vertical Status (ReadOnly) */
#define LCD5_VSTAT			FMsk(fLCD5_VSTAT)
#define LCD5_VSTAT_VS		0x00	/* VSYNC */
#define LCD5_VSTAT_BP		0x01	/* Back Porch */
#define LCD5_VSTAT_AC		0x02	/* Active */
#define LCD5_VSTAT_FP		0x03	/* Front Porch */

#define fLCD5_HSTAT			Fld(2,17)	/* TFT: Horizontal Status (ReadOnly) */
#define LCD5_HSTAT			FMsk(fLCD5_HSTAT)
#define LCD5_HSTAT_HS		0x00	/* HSYNC */
#define LCD5_HSTAT_BP		0x01	/* Back Porch */
#define LCD5_HSTAT_AC		0x02	/* Active */
#define LCD5_HSTAT_FP		0x03	/* Front Porch */

#define LCD5_BPP24BL		(1 << 12)
#define LCD5_FRM565			(1 << 11)
#define LCD5_INVVCLK		(1 << 10)	/* STN/TFT :
	   1 : video data is fetched at VCLK falling edge
	   0 : video data is fetched at VCLK rising edge */
#define LCD5_INVVLINE		(1 << 9)	/* STN/TFT :
	   1 : VLINE/HSYNC pulse polarity is inverted */
#define LCD5_INVVFRAME		(1 << 8)	/* STN/TFT :
	   1 : VFRAME/VSYNC pulse polarity is inverted */
#define LCD5_INVVD			(1 << 7)	/* STN/TFT :
	   1 : VD (video data) pulse polarity is inverted */
#define LCD5_INVVDEN		(1 << 6)	/* TFT :
	   1 : VDEN signal polarity is inverted */
#define LCD5_INVPWREN		(1 << 5)
#define LCD5_INVLEND		(1 << 4)	/* TFT :
	   1 : LEND signal polarity is inverted */
#define LCD5_PWREN			(1 << 3)
#define LCD5_LEND			(1 << 2)	/* TFT,1 : Enable LEND signal */
#define LCD5_BSWP			(1 << 1)	/* STN/TFT,1 : Byte swap enable */
#define LCD5_HWSWP			(1 << 0)	/* STN/TFT,1 : HalfWord swap enable */

#define fLCDADDR_BANK		Fld(9,21)	/* bank location for video buffer */
#define LCDADDR_BANK(x)		FInsrt((x), fLCDADDR_BANK)

#define fLCDADDR_BASEU		Fld(21,0)	/* address of upper left corner */
#define LCDADDR_BASEU(x)	FInsrt((x), fLCDADDR_BASEU)

#define fLCDADDR_BASEL		Fld(21,0)	/* address of lower right corner */
#define LCDADDR_BASEL(x)	FInsrt((x), fLCDADDR_BASEL)

#define fLCDADDR_OFFSET	Fld(11,11)	/* Virtual screen offset size (# of half words) */
#define LCDADDR_OFFSET(x)	FInsrt((x), fLCDADDR_OFFSET)

#define fLCDADDR_PAGE		Fld(11,0)	/* Virtual screen page width  (# of half words) */
#define LCDADDR_PAGE(x)		FInsrt((x), fLCDADDR_PAGE)

#define TPAL_LEN			(1 << 24)	/* 1 : Temp. Pallete Register enable */
#define fTPAL_VAL			Fld(24,0)	/* Temp. Pallete Register value */
#define TPAL_VAL(x)			FInsrt((x), fTPAL_VAL)
#define TPAL_VAL_RED(x)		FInsrt((x), Fld(8,16))
#define TPAL_VAL_GREEN(x)	FInsrt((x), Fld(8,8))
#define TPAL_VAL_BLUE(x)	FInsrt((x), Fld(8,0))

static U32 LCDFrameBuffer[LCDFBSIZE/4];

void LCD_pInit(void);

/***************************************************************************\
	LCD初始化函数，返回液晶屏帧缓冲区指针
\***************************************************************************/
unsigned int* LCD_Init()
{
	//configure gpio for LCD controller
	rGPDCON = 0xaaaaaaaa;
	rGPCCON = 0xaaaaaaaa;
	set_gpio_ctrl(GPIO_G4 | GPIO_PULLUP_EN | GPIO_MODE_LCD_PWRDN);

	LCDDisplayOpen(FALSE);

	memset(LCDFrameBuffer, 0, LCDFBSIZE);	//clear LCD

	rLCDCON1 = 0;
	LCD_pInit();

	rLCDLPCSEL &= (~0x3);
	rTPAL = 0;
	rLCDCON1 |= LCD1_ENVID;

	LCDBkLight(TRUE);
	LCDDisplayOpen(TRUE);

	LCDLib_Init(LCDFrameBuffer);

	printf("LCD initialization is OK\n");

	return LCDFrameBuffer;
}

#if( LCDTYPE == LCDTYPE_320x240xMONO)
#include "lcd/lcdmono0.c"
#elif( LCDTYPE == LCDTYPE_320x240x16GRAY)
#include "lcd/lcd16gray0.c"
#elif( LCDTYPE == LCDTYPE_320x240x256COLOR)
#include "lcd/lcd256color0.c"
#elif( LCDTYPE == LCDTYPE_640x480x64KCOLOR)
#include "lcd/lcd64kcolor0.c"
#endif
