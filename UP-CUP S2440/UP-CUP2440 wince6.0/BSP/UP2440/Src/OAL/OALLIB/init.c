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
//  File:  init.c
//
// 
//
#include <bsp.h>
static void InitDisplay(void);
void ConfigureGPIO(void);

#ifdef DVS_EN
//-------------------------------------------
// Variables for DVS
static int CurrVoltage;
extern volatile int CurrentState;
extern volatile int PrevState;
extern volatile int NextState;

//---------------------------------------------------------------------------
// Function for DVS
void ChangeVoltage(int);
int GetCurrentVoltage(void);

// ----------------------------------------------------------------------------
#endif

UINT32 g_oalIoCtlClockSpeed;
//------------------------------------------------------------------------------
//
//  Function:  OEMInit
//
//  This is Windows CE OAL initialization function. It is called from kernel
//  after basic initialization is made.
//
void OEMInit()
{
	volatile S3C2440A_IOPORT_REG *s2440IOP = (S3C2440A_IOPORT_REG *)OALPAtoVA(S3C2440A_BASE_REG_PA_IOPORT, FALSE);
	// Set up the debug zones according to the fix-up variable initialOALLogZones
	OALLogSetZones(initialOALLogZones);

	OALMSG(1, (L"+OEMInit\r\n"));

#ifdef DVS_EN
	volatile S3C2440A_IOPORT_REG *s2440IOP = (S3C2440A_IOPORT_REG *)OALPAtoVA(S3C2440A_BASE_REG_PA_IOPORT, FALSE);
#if (DVS_METHOD == 3)
	volatile S3C2440A_PWM_REG *g_pPWMRegs = (S3C2440A_PWM_REG*)OALPAtoUA(S3C2440A_BASE_REG_PA_PWM, FALSE);
	UINT32 sysIntr = SYSINTR_NOP;
	UINT32 irq;
	UINT32 tcon;
#endif
#endif

	NKForceCleanBoot();

	g_oalIoCtlClockSpeed = S3C2440A_FCLK;

	CEProcessorType=PROCESSOR_STRONGARM;

	// Set memory size for DrWatson kernel support
	dwNKDrWatsonSize = 128 * 1024;

	// Initilize cache globals
	OALCacheGlobalsInit();

	OALLogSerial(
		L"DCache: %d sets, %d ways, %d line size, %d size\r\n", 
		g_oalCacheInfo.L1DSetsPerWay, g_oalCacheInfo.L1DNumWays,
		g_oalCacheInfo.L1DLineSize, g_oalCacheInfo.L1DSize
	);
	OALLogSerial(
		L"ICache: %d sets, %d ways, %d line size, %d size\r\n", 
		g_oalCacheInfo.L1ISetsPerWay, g_oalCacheInfo.L1INumWays,
		g_oalCacheInfo.L1ILineSize, g_oalCacheInfo.L1ISize
	);

	// Check and initialize the BSP Args area
	//
	OALArgsInit((BSP_ARGS *) IMAGE_SHARE_ARGS_UA_START);

	// Check clean boot flag in BSP Args area
	//
	{
		// This is the global shared Args flag
		BOOL *bCleanBootFlag = (BOOL*) OALArgsQuery(BSP_ARGS_QUERY_CLEANBOOT);

		if(*bCleanBootFlag)
		{
			OALMSG(1, (TEXT("OEM: Force clean boot.\r\n")));

			// Clear the flag so that we don't get here in the next boot unless it is set again.
			*bCleanBootFlag = FALSE;

			// Tell filesys.exe that we want a clean boot.
			NKForceCleanBoot();

			// Also set the hive and storage clean flags if not already set
			{
				BOOL *bHiveCleanFlag  = (BOOL*) OALArgsQuery(BSP_ARGS_QUERY_HIVECLEAN);
				BOOL *bFormatPartFlag = (BOOL*) OALArgsQuery(BSP_ARGS_QUERY_FORMATPART);

				*bHiveCleanFlag  = TRUE;
				*bFormatPartFlag = TRUE;
			}
		}
	}

	// Initialize interrupts
	if (!OALIntrInit()) {
		OALMSG(OAL_ERROR, (
			L"ERROR: OEMInit: failed to initialize interrupts\r\n"
		));
	}

	// Initialize system clock
	OALTimerInit(RESCHED_PERIOD, OEM_COUNT_1MS, 0);

	// If StepLoader have Main OS image launched, This ConfigureGPIO() Function is not nedded.
	ConfigureGPIO();

	InitDisplay();
//	s2440IOP->GPBDAT |= (1<<1);


	// Make high-res Monte Carlo profiling available to the kernel
	g_pOemGlobal->pfnProfileTimerEnable = OEMProfileTimerEnable;
	g_pOemGlobal->pfnProfileTimerDisable = OEMProfileTimerDisable;

	// Initialize the KITL connection if required
	KITLIoctl(IOCTL_KITL_STARTUP, NULL, 0, NULL, 0, NULL);

	OALMSG(1, (L"-OEMInit\r\n"));

#ifdef DVS_EN
#if (Eval_Probe == 1)
	s2440IOP->MISCCR = (s2440IOP->MISCCR & ~(0x7 << 8)) | (0x3 << 8);		// CLKSEL1,     011 -> HCLK
	s2440IOP->MISCCR = (s2440IOP->MISCCR & ~(0x7 << 4)) | (0x4 << 4);		// CLKSEL0,     100 -> PCLK
	s2440IOP->GPHCON = (s2440IOP->GPHCON & ~(0x3 << 18)) | (0x2 << 18);		// GPH9 -> CLKOUT0
	s2440IOP->GPHCON = (s2440IOP->GPHCON & ~(0x3 << 20)) | (0x2 << 20);		// GPH10 -> CLKOUT1
#endif //(Eval_Probe == 1)	

#if (DVS_METHOD == 3)
	CurrentState = Active;
#endif
#endif
}




//------------------------------------------------------------------------------

static void InitDisplay(void)
{
#if 1
	volatile USHORT *UartCpld = (USHORT *)OALPAtoVA(0x08000114, FALSE);//2440串口CPLD设置：UART1（COM2）为DB9，UART2（COM3）为168帧扩展
	
	volatile S3C2440A_IOPORT_REG *s2440IOP = (S3C2440A_IOPORT_REG *)OALPAtoVA(S3C2440A_BASE_REG_PA_IOPORT, FALSE);
	volatile S3C2440A_LCD_REG    *s2440LCD = (S3C2440A_LCD_REG *)OALPAtoVA(S3C2440A_BASE_REG_PA_LCD, FALSE);
	volatile USHORT *s2440Cpld = (USHORT *)OALPAtoVA(0x08000118, FALSE);//2440触摸屏CPLD选择
	*s2440Cpld = ((*s2440Cpld)&(~(0xff))| 0xCC);
	RETAILMSG(1,(TEXT("s2440Cpld:%x\r\n"),*s2440Cpld));
	RETAILMSG(1,(TEXT("InitDisplay:%d*%d\r\n"),LCD_XSIZE_TFT,LCD_YSIZE_TFT));
	*UartCpld = 0xf9;
	RETAILMSG(1,(TEXT("UartCpld:%x\r\n"),*UartCpld));
	
	s2440IOP->GPCUP     = 0xFFFFFFFF;
	s2440IOP->GPCCON    = 0xAAAAAAAA;

	s2440IOP->GPEDAT    = 0x1;
	s2440IOP->GPDUP     = 0xFFFFFFFF;
	s2440IOP->GPDCON    = 0xAAAAAAA1; 

	s2440LCD->LCDCON1   =  (CLKVAL_TFT      <<  8) |       /* VCLK = HCLK / ((CLKVAL + 1) * 2) -> About 7 Mhz  */
	                       (LCD_MVAL_USED   <<  7)  |       /* 0 : Each Frame                                   */
	                       (3               <<  5) |       /* TFT LCD Pannel                                   */
	                       (12              <<  1) |       /* 16bpp Mode                                       */
	                       (0               <<  0) ;       /* Disable LCD Output                               */

	s2440LCD->LCDCON2   =  (LCD_VBPD        << 24) |   /* VBPD          :   1                              */
	                       (LCD_LINEVAL_TFT << 14) |   /* Vertical Size : 240 - 1                          */
	                       (LCD_VFPD        <<  6) |   /* VFPD          :   2                              */
	                       (LCD_VSPW        <<  0) ;   /* VSPW          :   1                              */

	s2440LCD->LCDCON3   =  (LCD_HBPD        << 19) |   /* HBPD          :   6                              */
	                       (LCD_HOZVAL_TFT  <<  8) |   /* HOZVAL_TFT    : 320 - 1                          */
	                       (LCD_HFPD        <<  0) ;   /* HFPD          :   2                              */


	s2440LCD->LCDCON4   =  (LCD_MVAL        <<  8) |   /* MVAL          :  13                              */
	                       (LCD_HSPW        <<  0) ;   /* HSPW          :   4                              */

#if (LCD_TYPE == LCD_TYPE_VGA1024_768)
	s2440LCD->LCDCON5   =  (1               << 11) |       /* BPP24BL       : LSB valid                        */
	                       (0               <<  5) |       /* INVPWREN      : Normal                           */
	                       (1               <<  3) |       /* PWREN         : Disable PWREN                    */
	                       (1               <<  0) ;       /* HWSWP         : Swap Enable                      */
#else
	s2440LCD->LCDCON5   =  (1               << 12) |       /* BPP24BL       : LSB valid                        */
	                       (1               << 11) |       /* FRM565 MODE   : 5:6:5 Format                     */
	                       (0               << 10) |       /* INVVCLK       : VCLK Falling Edge                */
	                       (1               <<  9) |       /* INVVLINE      : Inverted Polarity                */
	                       (1               <<  8) |       /* INVVFRAME     : Inverted Polarity                */
	                       (0               <<  7) |       /* INVVD         : Normal                           */
	                       (0               <<  6) |       /* INVVDEN       : Normal                           */
	                       (0               <<  5) |       /* INVPWREN      : Normal                           */
	                       (0               <<  4) |       /* INVENDLINE    : Normal                           */
	                       (1               <<  3) |       /* PWREN         : Disable PWREN                    */
	                       (0               <<  2) |       /* ENLEND        : Disable LEND signal              */
	                       (0               <<  1) |       /* BSWP          : Swap Disable                     */
	                       (1               <<  0) ;       /* HWSWP         : Swap Enable                      */
#endif

	s2440LCD->LCDSADDR1 = ((IMAGE_FRAMEBUFFER_DMA_BASE >> 22)     << 21) | 
	                      ((M5D(IMAGE_FRAMEBUFFER_DMA_BASE >> 1)) <<  0);

	s2440LCD->LCDSADDR2 = M5D((IMAGE_FRAMEBUFFER_DMA_BASE + (LCD_XSIZE_TFT* LCD_YSIZE_TFT * 2)) >> 1);

	s2440LCD->LCDSADDR3 = (((LCD_XSIZE_TFT - LCD_XSIZE_TFT) / 1) << 11) | (LCD_XSIZE_TFT / 1);        

	//s2440LCD->TCONSEL   |= 0x3;
	s2440LCD->TCONSEL   &= (~7);
	//s2440LCD->TCONSEL   |= (0x1<<4);

	s2440LCD->TPAL      = 0x0;        

#if (LCD_TYPE == LCD_TYPE_VGA1024_768)
	s2440LCD->LCDINTMSK |= 3;
   	s2440LCD->TCONSEL &= ~((1<<4) | 1);								// Disable LCC3600, LCP3600
#endif
	s2440LCD->LCDCON1  |= 1;

	// Display a bitmap image on the LCD...
#if (LCD_TYPE != LCD_TYPE_VGA1024_768)
//	memcpy((void *)IMAGE_FRAMEBUFFER_UA_BASE, imageArray, LCD_ARRAY_SIZE_TFT_16BIT);
#endif

#ifdef DVS_EN
#if (DVS_METHOD == 1 || DVS_METHOD == 3)
	// Enable LCD Interrupt for DVS + CLOCK Change.
	s2440LCD->LCDINTMSK = (1); // UnMASK LCD Frame Sub Interrupt
	//s2440INT->INTMSK &= ~(1 << IRQ_LCD);
	//RETAILMSG(1,(TEXT("-E5-\r\n")));
#endif
#endif
#endif
	
}


// If StepLoader have Main OS image launched, This ConfigureGPIO() Function is not nedded.
void ConfigureGPIO()
{
	volatile S3C2440A_IOPORT_REG *s2440IOP = (S3C2440A_IOPORT_REG *)OALPAtoVA(S3C2440A_BASE_REG_PA_IOPORT, FALSE);
	volatile S3C2440A_CLKPWR_REG *s2440CLKPWR = (S3C2440A_CLKPWR_REG *)OALPAtoVA(S3C2440A_BASE_REG_PA_CLOCK_POWER, FALSE);

	s2440IOP->GPACON	= 0x7fffff;

	//s2440IOP->GPBDAT	= 0x62;
	//s2440IOP->GPBUP	 	= 0x7FF;
	//s2440IOP->GPBCON	= 0x2A96A4;

	s2440IOP->GPCUP	 	= 0xFFFF;
	s2440IOP->GPCCON	= 0xAAAAAAAA;

	s2440IOP->GPEDAT	= 0x1;
	s2440IOP->GPDUP	 	= 0xFFFF;
	s2440IOP->GPDCON	= 0xAAAAAAA1;

	s2440IOP->GPEDAT	= 0x0;
	s2440IOP->GPEUP	 	= 0xFFFF;
	s2440IOP->GPECON	= 0xAAAAAAAA;

	//s2440IOP->GPFDAT	= 0xFF;
	//s2440IOP->GPFUP	 	= 0x00;
	//s2440IOP->GPFCON	= 0xAAAA;

	s2440IOP->GPGDAT	= 0x1000;
	s2440IOP->GPGUP	 	= 0xFBFF;
	s2440IOP->GPGCON	= 0x0484FFBA;
	s2440IOP->GPGCON	|=2<<20;
	s2440IOP->GPGDAT	|= 0x1<<12;
	s2440IOP->GPGUP		|= 0x1<<12;
	s2440IOP->GPGCON	= (s2440IOP->GPGCON & ~(0x3<<24)) | 0x1<<24;

	s2440IOP->GPHDAT	= 0x0;
	s2440IOP->GPHUP	 	= 0x7FF;
	s2440IOP->GPHCON	= 0x14AAAA;

	s2440IOP->GPJDAT	= 0x1000;
	s2440IOP->GPJUP	 	= 0x1FFF;
	s2440IOP->GPJCON	= 0x1AAAAA;

	s2440IOP->MISCCR	&= ~(7<<20);
	s2440IOP->MISCCR	|= (4<<20);
	s2440IOP->MISCCR	&= ~(7<<8);
	s2440IOP->MISCCR	|= (5<<8);
	s2440IOP->MISCCR	&= ~(7<<4);
	s2440IOP->MISCCR	|= (5<<4);
	s2440IOP->MISCCR	|= (3<<0);
	
}
#if 0
//#ifdef DVS_EN
void ChangeVoltage(int vtg)
{
	volatile S3C2440A_IOPORT_REG *s2440IOP = (S3C2440A_IOPORT_REG *)OALPAtoVA(S3C2440A_BASE_REG_PA_IOPORT, FALSE);
	UINT8 temp;
	
//  port setting  
// GPF4:D0, GPF5:D1, GPF6:D2, GPF7:D3, GPB7:D4, GPB8:Latch enable
	CurrVoltage = vtg;
	temp = s2440IOP->GPFDAT;
	switch(vtg) {
	case V090:	// 0.9V
		s2440IOP->GPBDAT=(s2440IOP->GPBDAT&0x77f)|(1<<7);   //D4
		s2440IOP->GPFDAT=(s2440IOP->GPFDAT&0x0f)|(0<<7)|(0<<6)|(1<<5)|(1<<4);   //D3~0
		break;
	case V095:	// 0.95V
		s2440IOP->GPBDAT=(s2440IOP->GPBDAT&0x77f)|(1<<7);   //D4
		s2440IOP->GPFDAT=(s2440IOP->GPFDAT&0x0f)|(0<<7)|(0<<6)|(0<<5)|(1<<4);   //D3~0
		break;
	case V0975:	// 0.975V
		s2440IOP->GPBDAT=(s2440IOP->GPBDAT&0x77f)|(1<<7);   //D4
		s2440IOP->GPFDAT=(s2440IOP->GPFDAT&0x0f)|(0<<7)|(0<<6)|(0<<5)|(0<<4);   //D3~0
		break;
	case V100:	// 1.0V
		s2440IOP->GPBDAT=(s2440IOP->GPBDAT&0x77f)|(0<<7);   //D4
		s2440IOP->GPFDAT=(s2440IOP->GPFDAT&0x0f)|(1<<7)|(1<<6)|(1<<5)|(1<<4);   //D3~0
		break;
	case V105:	// 1.05V
		s2440IOP->GPBDAT=(s2440IOP->GPBDAT&0x77f)|(0<<7);   //D4
		s2440IOP->GPFDAT=(s2440IOP->GPFDAT&0x0f)|(1<<7)|(1<<6)|(1<<5)|(0<<4);   //D3~0
		break;
	case V110:	// 1.1V
		s2440IOP->GPBDAT=(s2440IOP->GPBDAT&0x77f)|(0<<7);   //D4
		s2440IOP->GPFDAT=(s2440IOP->GPFDAT&0x0f)|(1<<7)|(1<<6)|(0<<5)|(1<<4);   //D3~0
		break;
	case V115:	// 1.15V
		s2440IOP->GPBDAT=(s2440IOP->GPBDAT&0x77f)|(0<<7);   //D4
		s2440IOP->GPFDAT=(s2440IOP->GPFDAT&0x0f)|(1<<7)|(1<<6)|(0<<5)|(0<<4);   //D3~0
		break;
	case V120:	// 1.2V
		s2440IOP->GPBDAT=(s2440IOP->GPBDAT&0x77f)|(0<<7);   //D4
		s2440IOP->GPFDAT=(s2440IOP->GPFDAT&0x0f)|(1<<7)|(0<<6)|(1<<5)|(1<<4);   //D3~0
		break;
	case V125:	// 1.25V
		s2440IOP->GPBDAT=(s2440IOP->GPBDAT&0x77f)|(0<<7);   //D4
		s2440IOP->GPFDAT=(s2440IOP->GPFDAT&0x0f)|(1<<7)|(0<<6)|(1<<5)|(0<<4);   //D3~0
		break;
	case V130:	// 1.3V
		s2440IOP->GPBDAT=(s2440IOP->GPBDAT&0x77f)|(0<<7);   //D4
		s2440IOP->GPFDAT=(s2440IOP->GPFDAT&0x0f)|(1<<7)|(0<<6)|(0<<5)|(1<<4);   //D3~0
		break;
	case V135:	// 1.35V
		s2440IOP->GPBDAT=(s2440IOP->GPBDAT&0x77f)|(0<<7);   //D4
		s2440IOP->GPFDAT=(s2440IOP->GPFDAT&0x0f)|(1<<7)|(0<<6)|(0<<5)|(0<<4);   //D3~0
		break;
	case V140:	// 1.4V
		s2440IOP->GPBDAT=(s2440IOP->GPBDAT&0x77f)|(0<<7);   //D4
		s2440IOP->GPFDAT=(s2440IOP->GPFDAT&0x0f)|(0<<7)|(1<<6)|(1<<5)|(1<<4);   //D3~0
		break;
	case V145:	// 1.45V
		s2440IOP->GPBDAT=(s2440IOP->GPBDAT&0x77f)|(0<<7);   //D4
		s2440IOP->GPFDAT=(s2440IOP->GPFDAT&0x0f)|(0<<7)|(1<<6)|(1<<5)|(0<<4);   //D3~0
		break;
	case V150:	// 1.5V
		s2440IOP->GPBDAT=(s2440IOP->GPBDAT&0x77f)|(0<<7);   //D4
		s2440IOP->GPFDAT=(s2440IOP->GPFDAT&0x0f)|(0<<7)|(1<<6)|(0<<5)|(1<<4);   //D3~0
		break;

	default:
		break;
	}

//  port setting  
// GPF4:D0, GPF5:D1, GPF6:D2, GPF7:D3, GPB7:D4, GPB8:Latch enable, GPB10: OE
	s2440IOP->GPBCON=(s2440IOP->GPBCON&0x3f3fff)|(1<<14);   // GPB7: Output
//	s2440IOP->rGPBUP&=~(1<<7);
	s2440IOP->GPFCON=(s2440IOP->GPFCON&0x00ff)|(0x9500);  //GPF4~7: Output 0x5500
//	s2440IOP->rGPFUP&=~(0xf<<4);

	s2440IOP->GPBDAT&=~(1<<8);   //Latch enable
	s2440IOP->GPBCON=(s2440IOP->GPBCON&0x3cffff)|(1<<16);   // GPB8: Output

	s2440IOP->GPBDAT|=(1<<10);   //Output enable
	s2440IOP->GPBCON=(s2440IOP->GPBCON&0x0fffff)|(1<<20);   // GPB10: Output
	
//	s2440IOP->rGPBUP&=~(1<<8);
	s2440IOP->GPBDAT|=(1<<8);   //Latch disable
	
}

int GetCurrentVoltage()
{
	return CurrVoltage;
}
#endif
