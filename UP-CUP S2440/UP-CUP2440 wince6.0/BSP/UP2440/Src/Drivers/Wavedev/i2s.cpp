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
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

   
Module Name:	I2S.CPP

Abstract:		I2S Interface Routines for Samsung SC2440 CPU
  
Notes:			This code assumes that the CPU acts as the master that is 
				connected to a CODEC chip configured in slave mode.

				Some platforms require the audio CODEC chip to act as the master
				while the CPU's IIS controller is put into slave mode.  For these
				environments, the IIS_MASTER_MODE flag (located in I2S_Init())
				should be replaced with the IIS_SLAVE_MODE flag.

Environment:	Samsung SC2440 CPU and Windows 3.0 (or later)
    
-*/

#include <windows.h>
#include "i2s.h"
#include <bsp.h>

//------------------------------ GLOBALS -------------------------------------------
extern volatile	S3C2440A_IISBUS_REG *g_pIISregs;								// I2S control registers
extern volatile S3C2440A_IOPORT_REG *g_pIOPregs;								// GPIO registers (needed to enable SPI)
extern volatile S3C2440A_CLKPWR_REG	*g_pCLKPWRreg;							// CLCKPWR (needed to enable SPI clocks)
//----------------------------------------------------------------------------------

#ifdef DEBUG
#define ZONE_ERROR  1
#endif

#define DEFAULT_S3C2440A_PCLK (399651840 / 6)

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		I2S_Init()

Description:	Initializes the IIS bus and controller.

Notes:			This routine assumes that the control registers (see 
				the globals section above) have already been initialized.

Returns:		Boolean indicating success.
-------------------------------------------------------------------*/
BOOL I2S_Init()
{
	//----- 1. IMPORTANT: By default, the internal clock is disabled.  To configure the controller ------
	//					  we must first enable it.
//	StartI2SClock();
	g_pCLKPWRreg->CLKCON |= IIS_INTERNAL_CLOCK_ENABLE;		// Enable the CPU clock to the IIS controller
	//----- 2. Configure the GPIO pins for I2S mode -----
	//		   
	//		   I2SSDO	- GPE4
	//		   I2SSDI	- GPE3
	//		   I2SCDCLK	- GPE2
	//		   I2SSCLK	- GPE1
	//		   I2SLRCLK	- GPE0
	//
	// Port Init for IIS
	g_pIOPregs->GPEUP  |= 0x1f;
	//PG[4:0]=I2SSDI:I2SSDO:CDCLK:I2SSCLK:I2SLRCK
	g_pIOPregs->GPECON = ((g_pIOPregs->GPECON & 0xfffffc00) | 0x2aa);
	
	//----- 3. Configure the I2S controller with reasonable default values -----
	g_pIISregs->IISCON  = ((1<<1)|(1<<0)|(1<<4)|(1<<5));//(TRANSMIT_DMA_REQUEST_ENABLE | IIS_PRESCALER_ENABLE);
	
	g_pIISregs->IISMOD  = ((0<<5)|(1<<4)|(1<<3)|(1<<2)|(3<<6)|(1<<0));//(  /*MASTER_CLOCL_MPLLIN | */ IIS_MASTER_MODE | IIS_TRANSMIT_RECEIVE_MODE | ACTIVE_CHANNEL_LEFT
						   //| SERIAL_INTERFACE_IIS_COMPAT | DATA_16_BITS_PER_CHANNEL
						   //| MASTER_CLOCK_FREQ_384fs | SERIAL_BIT_CLOCK_FREQ_32fs);

	g_pIISregs->IISFCON = ((1<<14)|(1<<12)|(1<<15)|(1<<13));//(  TRANSMIT_FIFO_ACCESS_DMA | TRANSMIT_FIFO_ENABLE  );
//						   | RECEIVE_FIFO_ACCESS_DMA  | RECEIVE_FIFO_ENABLE);		

	SetI2SClockRate(IS2LRCLK_44100);

	//----- 4. For power management purposes, shut the clocks off! -----
    StopI2SClock();

	// charlie
	g_pIISregs->IISCON |= IIS_INTERFACE_ENABLE;				// Enable the I2S clock
	
	return(TRUE);
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		I2S_Deinit()

Description:	Deinitializes the I2S bus and controller.

Notes:			This routine DOES NOT unmap the control registers;
				the caller is responsible for freeing this memory.

Returns:		Boolean indicating success.
-------------------------------------------------------------------*/
BOOL I2S_Deinit()
{
    //----- 1. Stop the I2S clocks -----
    StopI2SClock();

	return TRUE;
}



/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		I2S_WriteData()

Description:	Outputs the specified data onto the I2S bus.

Notes:			This routine expects that the I2S clock is already
				running when it is called.

Returns:		Boolean indicating success.
-------------------------------------------------------------------*/
BOOL I2S_WriteData(LPWORD lpBuff, DWORD dwLen)
{
	static DWORD waitCount	= 0;
	static DWORD i			= 0;

	//----- 1. Check the parameters -----
	if(!lpBuff || (dwLen == 0))
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("WAVEDEV.DLL:I2S_WriteData() - Invalid parameters!\r\n")));  
        goto WRITE_ERROR;
	}

//	StartI2SClock();

	//----- 2. Enable the transmit FIFO -----
	g_pIISregs->IISFCON |= TRANSMIT_FIFO_ENABLE;
	
	//----- 3. Write all of the bytes in the input buffer onto the I2S bus -----
	while(dwLen)
	{
		//----- 4. Write a FIFO's worth of data out onto the I2S bus ----- 
		if(dwLen > MAX_TRANSMIT_FIFO_ENTRIES)
		{
			i = MAX_TRANSMIT_FIFO_ENTRIES;

		}else
		{
			i = dwLen;
		}

		dwLen -= i;
		while(i--);
		{
			g_pIISregs->IISFIFO = *lpBuff++;
		}
	}

	StopI2SClock();
	return TRUE;

WRITE_ERROR:
	StopI2SClock();
	return FALSE;
}


//------------------------------------ Helper Routines ------------------------------------

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		StartI2SClock()

Description:	Enables the I2S clock that drives the audio codec chip.

Returns:		N/A
-------------------------------------------------------------------*/
VOID StartI2SClock(VOID)
{
	
	return;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		StopI2SClock()

Description:	Disables the I2S clock that drives the audio codec chip.

Returns:		N/A
-------------------------------------------------------------------*/
VOID StopI2SClock(VOID)
{
	return;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		SetI2SClockRate()

Description:	Sets the I2S clock that drives the audio codec chip:

Params:			ClockRate	IS2LRCLK_800		800   Khz
							IS2LRCLK_11025		11025 Khz
							IS2LRCLK_16000		16000 Khz
							IS2LRCLK_22050		22050 Khz
							IS2LRCLK_32000		32000 Khz
							IS2LRCLK_44100		44100 Khz
							IS2LRCLK_48000		48000 Khz
							IS2LRCLK_64000		64000 Khz
							IS2LRCLK_88200		88200 Khz
							IS2LRCLK_96000		96000 Khz

Returns:		N/A
-------------------------------------------------------------------*/
VOID SetI2SClockRate(DWORD ClockRate)
{
	BYTE  prescaler;
	DWORD codeclock, i2scdclk;
    PROCESSOR_INFO procInfo;
    DWORD dwBytesReturned;
    UINT32 s3c2440_pclk = S3C2440A_PCLK;

#if 0
	if(g_pIISregs->IISMOD & MASTER_CLOCL_MPLLIN)
	{
		return;
	}
	
    // Determine the CPU clock speed by querying the OAL.
    //
    if (!KernelIoControl(IOCTL_PROCESSOR_INFORMATION, NULL, 0, &procInfo, sizeof(PROCESSOR_INFO), &dwBytesReturned))
    {
        RETAILMSG(TRUE, (TEXT("WARNING: WAVEDEV.DLL: SetI2SClockRate: Failed to obtain processor frequency - using default value(%d).\r\n"), s3c2440_pclk));
    }
    else
    {
        s3c2440_pclk = procInfo.dwClockSpeed;
        RETAILMSG(TRUE, (TEXT("INFO: WAVEDEV.DLL: SetI2SClockRate:  Using processor frequency reported by the OAL (%d).\r\n"), s3c2440_pclk));
    }
#endif

    //----- 1. Set the clock rate  -----
	//		FORMAT:			bits[9:5] - Prescaler Control A
	//						bits[4:0] - Prescaler Control B
	//
	//						Range: 0-31 and the division factor is N+1 (a.k.a. 1-32)
	//
	//		The I2SLRCLK frequency is determined as follows:
	//
	//				I2SLRCLK = CODECLK / I2SCDCLK		and		(prescaler+1) = PCLK / CODECLK
	//
	//		Thus, rearranging the equations a bit we can see that:
	//
	//				prescaler = (PCLK / CODECLK) - 1 
	//		or
	//				prescaler = ((PCLK / (IS2LRCLK * IS2CDCLK)) - 1
	//	
	//	    NOTE: The following formula is actually used in order to avoid floating point arithmetic:
	//
	//				prescaler = ((PCLK + ((IS2LRCLK * IS2CDCLK) - 1)) / (IS2LRCLK * IS2CDCLK)) - 1
	//
	if(g_pIISregs->IISMOD & MASTER_CLOCK_FREQ_384fs)
	{
		i2scdclk = 384;				// Sampling frequency
	}else
	{
		i2scdclk = 256;				// Sampling frequency
	}

	codeclock = ClockRate * i2scdclk;
	prescaler = (BYTE)((s3c2440_pclk + (codeclock - 1)) / codeclock) - 1;
	RETAILMSG(1,(TEXT("Prescaler:%d\n"),prescaler));

	//----- IMPORTANT: Make sure we set both channel prescalers to the same value (see datasheet for details) -----
	g_pIISregs->IISPSR = ((prescaler << 5) +prescaler);

    return;
}




