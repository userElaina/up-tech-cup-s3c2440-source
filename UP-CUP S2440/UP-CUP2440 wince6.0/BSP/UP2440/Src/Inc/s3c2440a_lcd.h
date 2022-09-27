//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
//  Header: s3c2440a_lcd.h
//
//  Defines the LCD controller CPU register layout and definitions.
//
#ifndef __S3C2440A_LCD_H
#define __S3C2440A_LCD_H

#if __cplusplus
    extern "C" 
    {
#endif

//------------------------------------------------------------------------------
//  Type: S3C2440A_LCD_REG    
//
//  LCD control registers. This register bank is located by the constant
//  CPU_BASE_REG_XX_LCD in the configuration file cpu_base_reg_cfg.h.
//

typedef struct {
    UINT32 LCDCON1;             // 0x00
    UINT32 LCDCON2;             // 0x04
    UINT32 LCDCON3;             // 0x08
    UINT32 LCDCON4;             // 0x0C
    UINT32 LCDCON5;             // 0x10
    UINT32 LCDSADDR1;           // 0x14
    UINT32 LCDSADDR2;           // 0x18
    UINT32 LCDSADDR3;           // 0x1C
    UINT32 REDLUT;              // 0x20
    UINT32 GREENLUT;            // 0x24
    UINT32 BLUELUT;             // 0x28
    UINT32 PAD[8];              // 0x2C - 0x48
    UINT32 DITHMODE;            // 0x4C
    UINT32 TPAL;                // 0x50
    UINT32 LCDINTPND;           // 0x54
    UINT32 LCDSRCPND;           // 0x58
    UINT32 LCDINTMSK;           // 0x5C   
    UINT32 TCONSEL;             // 0x60

} S3C2440A_LCD_REG, *PS3C2440A_LCD_REG;


//------------------------------------------------------------------------------
//  Define: LCD_TYPE_XXX
//
//  Enumerates the types of LCD displays available.
//

#define		LCD_TYPE_STN8BPP	(1)
#define		LCD_TYPE_TFT240_320	(2)  //T35
#define		LCD_TYPE_TFT320_240	(3)  //W35 or S35 or Q35
#define		LCD_TYPE_TFT480_272	(4)  //W43 or Q43
#define		LCD_TYPE_TFT640_480	(5)  //VGA
#define		LCD_TYPE_VGA640_480	(6)  //VGA 640*480
#define		LCD_TYPE_TFT800_480	(7)  //A70
#define		LCD_TYPE_TFT800_600	(8)  //A104
#define		LCD_TYPE_VGA800_600	(9)  //VGA 800*600
#define		LCD_TYPE_VGA1024_768	(10)  //VGA 1024*768
#define		S35			1
#define		W35			0

//------------------------------------------------------------------------------
//  Define: LCD_TYPE
//
//  Defines the active LCD type from above choices.
//

#define		LCD_TYPE		LCD_TYPE_TFT640_480
#define		TFT_TYPE		W35

//------------------------------------------------------------------------------
//  Define: LCD_MODE_XXX
//
//  Defines the LCD mode.
//

#define    LCD_MODE_STN_1BIT       (1)
#define    LCD_MODE_STN_2BIT       (2)
#define    LCD_MODE_STN_4BIT       (4)
#define    LCD_MODE_CSTN_8BIT      (108)
#define    LCD_MODE_CSTN_12BIT     (112)
#define    LCD_MODE_TFT_1BIT       (201)
#define    LCD_MODE_TFT_2BIT       (202)
#define    LCD_MODE_TFT_4BIT       (204)
#define    LCD_MODE_TFT_8BIT       (208)
#define    LCD_MODE_TFT_16BIT      (216)

//------------------------------------------------------------------------------
//  Define: LCD_SCR_XXX
//
//  Screen size definitions.
//

#define    LCD_SCR_XSIZE           (1024)           // virtual screen  
#define    LCD_SCR_YSIZE           (768)



//------------------------------------------------------------------------------
//  Define: LCD_*SIZE_XXX
//
//  Defines physical screen sizes and orientation.
//

#define    LCD_XSIZE_STN           (320)
#define    LCD_YSIZE_STN           (240)

#define    LCD_XSIZE_CSTN          (320)
#define    LCD_YSIZE_CSTN          (240)


#if (LCD_TYPE == LCD_TYPE_TFT240_320)
	#define    LCD_XSIZE_TFT           (240)   
	#define    LCD_YSIZE_TFT           (320)
	#define    CLKVAL_TFT              (4)

	#define    LCD_VBPD                ((1)&0xff)
	#define    LCD_VFPD                ((5)&0xff)
	#define    LCD_VSPW                ((5)&0x3f)
	#define    LCD_HBPD                ((5)&0x7f)
	#define    LCD_HFPD                ((10)&0xff)
	#define    LCD_HSPW                ((20)&0xff)

#elif (LCD_TYPE == LCD_TYPE_TFT320_240)
	#define    LCD_XSIZE_TFT           (320)   
	#define    LCD_YSIZE_TFT           (240)
	#define    CLKVAL_TFT              (6)

#if (TFT_TYPE == S35)
	#define    LCD_VBPD                ((3)&0xff)
	#define    LCD_VFPD                ((5)&0xff)
	#define    LCD_VSPW                ((15)&0x3f)
	#define    LCD_HBPD                ((5)&0x7f)
	#define    LCD_HFPD                ((15)&0xff)
	#define    LCD_HSPW                ((8)&0xff)
#elif (TFT_TYPE == W35)
	#define    LCD_VBPD                ((4)&0xff)
	#define    LCD_VFPD                ((5)&0xff)
	#define    LCD_VSPW                ((15)&0x3f)
	#define    LCD_HBPD                ((21)&0x7f)
	#define    LCD_HFPD                ((32)&0xff)
	#define    LCD_HSPW                ((45)&0xff)
#endif

#elif (LCD_TYPE == LCD_TYPE_TFT480_272)
	#define    LCD_XSIZE_TFT           (480)   
	#define    LCD_YSIZE_TFT           (272)
	#define    CLKVAL_TFT              (4)

	#define    LCD_VBPD                ((2)&0xff)
	#define    LCD_VFPD                ((4)&0xff)
	#define    LCD_VSPW                ((8)&0x3f)
	#define    LCD_HBPD                ((10)&0x7f)
	#define    LCD_HFPD                ((19)&0xff)
	#define    LCD_HSPW                ((30)&0xff)

#elif (LCD_TYPE == LCD_TYPE_TFT640_480)
	#define    LCD_XSIZE_TFT           (640)   
	#define    LCD_YSIZE_TFT           (480)
	#define    CLKVAL_TFT              (1)

	#define    LCD_VBPD                ((32)&0xff) //25
	#define    LCD_VFPD                ((9)&0xff)
	#define    LCD_VSPW                ((1)&0x3f)
	#define    LCD_HBPD                ((47)&0x7f)
	#define    LCD_HFPD                ((15)&0xff)
	#define    LCD_HSPW                ((95)&0xff)

#elif (LCD_TYPE == LCD_TYPE_TFT800_480)
	#define    LCD_XSIZE_TFT           (800)   
	#define    LCD_YSIZE_TFT           (480)
	#define    CLKVAL_TFT              (1)

	#define    LCD_VBPD                ((5)&0xff)
	#define    LCD_VFPD                ((6)&0xff)
	#define    LCD_VSPW                ((1)&0x3f)
	#define    LCD_HBPD                ((28)&0x7f)
	#define    LCD_HFPD                ((14)&0xff)
	#define    LCD_HSPW                ((180)&0xff)

#elif (LCD_TYPE == LCD_TYPE_TFT800_600)
	#define    LCD_XSIZE_TFT           (800)   
	#define    LCD_YSIZE_TFT           (600)
	#define    CLKVAL_TFT              (3)

	#define    LCD_VBPD                ((24-1)&0xff)
	#define    LCD_VFPD                ((2-1)&0xff)
	#define    LCD_VSPW                ((2-1)&0x3f)
	#define    LCD_HBPD                ((89-1)&0x7f)
	#define    LCD_HFPD                ((41-1)&0xff)
	#define    LCD_HSPW                ((81-1)&0xff)

#elif (LCD_TYPE == LCD_TYPE_VGA640_480)
	#define    LCD_XSIZE_TFT           (640)   
	#define    LCD_YSIZE_TFT           (480)
	#define    CLKVAL_TFT              (1)

	#define    LCD_VBPD                ((25)&0xff)
	#define    LCD_VFPD                ((9)&0xff)
	#define    LCD_VSPW                ((1)&0x3f)
	#define    LCD_HBPD                ((47)&0x7f)
	#define    LCD_HFPD                ((15)&0xff)
	#define    LCD_HSPW                ((95)&0xff)

#elif (LCD_TYPE == LCD_TYPE_VGA800_600)
	#define    LCD_XSIZE_TFT           (800)   
	#define    LCD_YSIZE_TFT           (600)
	#define    CLKVAL_TFT              (1)

	#define    LCD_VBPD                ((24-1)&0xff)
	#define    LCD_VFPD                ((2-1)&0xff)
	#define    LCD_VSPW                ((2-1)&0x3f)
	#define    LCD_HBPD                ((89-1)&0x7f)
	#define    LCD_HFPD                ((41-1)&0xff)
	#define    LCD_HSPW                ((81-1)&0xff)

#elif (LCD_TYPE == LCD_TYPE_VGA1024_768)
	#define    LCD_XSIZE_TFT           (1024)   
	#define    LCD_YSIZE_TFT           (768)
	#define    CLKVAL_TFT              (3)

	#define    LCD_VBPD                ((1)&0xff)
	#define    LCD_VFPD                ((1)&0xff)
	#define    LCD_VSPW                ((1)&0x3f)
	#define    LCD_HBPD                ((15)&0x7f)
	#define    LCD_HFPD                ((199)&0xff)
	#define    LCD_HSPW                ((15)&0xff)

#endif

#define    LCD_SCR_XSIZE_TFT       (LCD_XSIZE_TFT * 2)
#define    LCD_SCR_YSIZE_TFT       (LCD_YSIZE_TFT * 2)
//------------------------------------------------------------------------------
//  Define: LCD_ARRAY_SIZE_XXX
//
//  Array Sizes based on screen configuration.
//

#define    LCD_ARRAY_SIZE_STN_1BIT     (LCD_SCR_XSIZE/8*LCD_SCR_YSIZE)
#define    LCD_ARRAY_SIZE_STN_2BIT     (LCD_SCR_XSIZE/4*LCD_SCR_YSIZE)
#define    LCD_ARRAY_SIZE_STN_4BIT     (LCD_SCR_XSIZE/2*LCD_SCR_YSIZE)
#define    LCD_ARRAY_SIZE_CSTN_8BIT    (LCD_SCR_XSIZE/1*LCD_SCR_YSIZE)
#define    LCD_ARRAY_SIZE_CSTN_12BIT   (LCD_SCR_XSIZE*2*LCD_SCR_YSIZE)
#define    LCD_ARRAY_SIZE_TFT_8BIT     (LCD_XSIZE_TFT*1*LCD_YSIZE_TFT)
#define    LCD_ARRAY_SIZE_TFT_16BIT    (LCD_XSIZE_TFT*2*LCD_YSIZE_TFT)

//------------------------------------------------------------------------------
//  Define: LCD_HOZVAL_XXX
//
//  Desc...
//

#define    LCD_HOZVAL_STN          (LCD_XSIZE_STN/4-1)
#define    LCD_HOZVAL_CSTN         (LCD_XSIZE_CSTN*3/8-1)
#define    LCD_HOZVAL_TFT          (LCD_XSIZE_TFT-1)

//------------------------------------------------------------------------------
//  Define: LCD_LINEVAL_XXX
//
//  Desc...
//

#define    LCD_LINEVAL_STN         (LCD_YSIZE_STN-1)
#define    LCD_LINEVAL_CSTN        (LCD_YSIZE_CSTN-1)
#define    LCD_LINEVAL_TFT         (LCD_YSIZE_TFT-1)


#define    LCD_MVAL                (13)
#define    LCD_MVAL_USED           (0)

// STN/CSTN timing parameter for LCBHBT161M(NANYA)

#define    LCD_WLH                 (3)
#define    LCD_WDLY                (3)
#define    LCD_LINEBLANK           ((1)&0xff)

//------------------------------------------------------------------------------
//  Define: LCD_CLKVAL_XXX
//
//  Clock values
//

#define     CLKVAL_STN_MONO         (22)    

// 69.14hz @60Mhz,WLH=16clk,WDLY=16clk,LINEBLANK=1*8,VD=4 

#define     CLKVAL_STN_GRAY         (12)    

//124hz @60Mhz,WLH=16clk,WDLY=16clk,LINEBLANK=1*8,VD=4  

#define     CLKVAL_CSTN             (8)     

// NOTE: 1)SDRAM should have 32-bit bus width. 
//      2)HBPD,HFPD,HSPW should be optimized. 
// 44.6hz @75Mhz
// VSYNC,HSYNC should be inverted
// HBPD=48VCLK,HFPD=16VCLK,HSPW=96VCLK
// VBPD=33HSYNC,VFPD=10HSYNC,VSPW=2HSYNC

#define     M5D(n)                  ((n)&0x1fffff)


#if __cplusplus
    }
#endif

#endif 
