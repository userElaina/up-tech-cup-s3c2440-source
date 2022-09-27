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

   
Module Name:	HWCTXT.CPP

Abstract:		Platform dependent code for the mixing audio driver.

Notes:			The following file contains all the hardware specific code
				for the mixing audio driver.  This code's primary responsibilities
				are:

					* Initialize audio hardware (including codec chip)
					* Schedule DMA operations (move data from/to buffers)
					* Handle audio interrupts

				All other tasks (mixing, volume control, etc.) are handled by the "upper"
				layers of this driver.

				****** IMPORTANT ******
				For the SC2440 CPU, DMA channel 2 can be used for both input and output.  In this,
				configuration, however, only one type operation (input or output) can execute.  In 
				order to implement simultaneous playback and recording, two things must be done:
 
					1) Input DMA should be moved to DMA Channel 1; Output DMA still uses DMA Channel 2.
					2) Step #3 in InterruptThread() needs to be implemented so that the DMA interrupt
					   source (input DMA or output DMA?) can be determined.  The interrupt source needs
					   to be determined so that the appropriate buffers can be copied (Steps #4,#5...etc.).

				Lastly, the m_OutputDMAStatus and m_InputDMAStatus variables shouldn't need to be modified.  
				The logic surrounding these drivers is simply used to determine which buffer (A or B) needs
				processing.

-*/
#include <windows.h>
#include <ceddk.h>

#include "wavemain.h"
#include "s3c2440a.h"
#include "s3c2440a_dmatransfer.h"
#include "I2S.h"
#include "hwctxt.h"


#define DMA_CH_MIC 2
#define DMA_CH_OUT 1

/*
#define L3M (0x04)	// TOUT2
#define L3C (0x10)	// TCLK0
#define L3D (0x08)	// TOUT3
*/

#define L3M (0x0100)	// TOUT2	GPG8	
#define L3C (0x0200)	// TCLK0		GPG9
#define L3D (0x0001)	// TOUT3	GPG0

// UDA1341 Register definitions
// The codec has three "addressing modes", which could also be thought of as registers
#define UDA1341_ADDR_DATA0  0x14    // 00010100 | 00  (ID | data0)
#define UDA1341_ADDR_DATA1  0x15    // 00010100 | 01  (ID | data1)
#define UDA1341_ADDR_STATUS	0x16    // 00010100 | 10  (ID | status)

// Status register bits
// Think of the status register as two separate 7-bit register (StatusA and StatusB)
// with bit 7 selecting between the two.

#define UDA1341_STATUS_A            0x00
#define UDA1341_STATUS_B            0x80
#define UDA1341_STATUS_DCFILTER     0x01 // bit 0: DC Filter on
#define UDA1341_STATUS_FMTIIS       0x00 // bit 1-3: format status
#define UDA1341_STATUS_FMT_LSB16    0x02
#define UDA1341_STATUS_FMT_LSB18    0x04
#define UDA1341_STATUS_FMT_LSB20    0x06
#define UDA1341_STATUS_FMT_MSB      0x08
#define UDA1341_STATUS_FMT_MSB16    0x0A
#define UDA1341_STATUS_FMT_MSB18    0x0C
#define UDA1341_STATUS_FMT_MSB20    0x0E
#define UDA1341_STATUS_CLK512       0x00 // bits 4,5: system clock
#define UDA1341_STATUS_CLK384       0x10 // 
#define UDA1341_STATUS_CLK256       0x20 // 
#define UDA1341_STATUS_RESET        0x40 // bit 6: reset

#define UDA1341_STATUS_PWR_DAC      0x01 // bit 0: DAC power on
#define UDA1341_STATUS_PWR_ADC      0x02 // bit 1: ADC power on
#define UDA1341_STATUS_IGS          0x20 // bit 5: IGS - ADC 6dB Boost
#define UDA1341_STATUS_OGS          0x40 // bit 6: OGS - DAC 6dB Boost

#define DATA0 (0x00) 
#define DATA0_VOLUME_MASK (0x3f) 
#define DATA0_VOLUME(x) (x) 
#define DATA1 (0x40) 
#define DATA1_BASS(x) ((x) << 2) 
#define DATA1_BASS_MASK (15 << 2) 
#define DATA1_TREBLE(x) ((x)) 
#define DATA1_TREBLE_MASK (3) 
#define DATA2 (0x80) 
#define DATA2_PEAKAFTER (0x1 << 5) 
#define DATA2_DEEMP_NONE (0x0 << 3) 
#define DATA2_DEEMP_32KHz (0x1 << 3) 
#define DATA2_DEEMP_44KHz (0x2 << 3) 
#define DATA2_DEEMP_48KHz (0x3 << 3) 
#define DATA2_MUTE (0x1 << 2) 
#define DATA2_FILTER_FLAT (0x0 << 0) 
#define DATA2_FILTER_MIN (0x1 << 0) 
#define DATA2_FILTER_MAX (0x3 << 0)
#define EXTADDR(n) (0xc0 | (n)) 
#define EXTDATA(d) (0xe0 | (d)) 
#define EXT0 0 
#define EXT0_CH1_GAIN(x) (x) 
#define EXT1 1 
#define EXT1_CH2_GAIN(x) (x) 
#define EXT2 2 
#define EXT2_MIC_GAIN_MASK (7 << 2) 
#define EXT2_MIC_GAIN(x) ((x) << 2) 
#define EXT2_MIXMODE_DOUBLEDIFF (0) 
#define EXT2_MIXMODE_CH1 (1) 
#define EXT2_MIXMODE_CH2 (2) 
#define EXT2_MIXMODE_MIX (3) 
#define EXT4 4 
#define EXT4_AGC_ENABLE (1 << 4) 
#define EXT4_INPUT_GAIN_MASK (3) 
#define EXT4_INPUT_GAIN(x) ((x) & 3) 
#define EXT5 5 
#define EXT5_INPUT_GAIN(x) ((x) >> 2) 
#define EXT6 6 
#define EXT6_AGC_CONSTANT_MASK (7 << 2) 
#define EXT6_AGC_CONSTANT(x) ((x) << 2)
#define EXT6_AGC_LEVEL_MASK (3) 
#define EXT6_AGC_LEVEL(x) (x)
#define DEF_VOLUME 65
static int uda1341_volume;
static int uda_sampling;
static int uda1341_boost;

int rec_mode=0;
//-------------------------------- Global Variables --------------------------------------
volatile S3C2440A_IISBUS_REG *g_pIISregs		= NULL;		// I2S control registers
volatile S3C2440A_IOPORT_REG *g_pIOPregs		= NULL;		// GPIO registers (needed to enable I2S and SPI)
volatile S3C2440A_SPI_REG    *g_pSPIregs        = NULL;		// SPI control registers

volatile S3C2440A_DMA_REG    *g_pDMAregs		= NULL;		// DMA registers (needed for I/O on I2S bus)
volatile S3C2440A_CLKPWR_REG *g_pCLKPWRreg	= NULL;		// Clock power registers (needed to enable I2S and SPI clocks)
volatile S3C2440A_INTR_REG *s2440INT 		= NULL;
				
HardwareContext *g_pHWContext		= NULL;

PHYSICAL_ADDRESS g_PhysDMABufferAddr;

//----------------------------------------------------------------------------------------

#ifdef DEBUG
DBGPARAM dpCurSettings = {
    TEXT("WaveDriver"), {
		 TEXT("Test")           //  0   ZONE_TEST
		,TEXT("Params")         //  1   ZONE_PARAMS     
		,TEXT("Verbose")        //  2   ZONE_VERBOSE    
		,TEXT("Interrupt")      //  3   ZONE_INTERRUPT  
		,TEXT("WODM")           //  4   ZONE_WODM       
		,TEXT("WIDM")           //  5   ZONE_WIDM       
		,TEXT("PDD")            //  6   ZONE_PDD        
		,TEXT("MDD")            //  7   ZONE_MDD        
		,TEXT("Regs")           //  8   ZONE_REGS       
		,TEXT("Misc")           //  9   ZONE_MISC       
		,TEXT("Init")           // 10   ZONE_INIT       
		,TEXT("IOcontrol")      // 11   ZONE_IOCTL      
		,TEXT("Alloc")          // 12   ZONE_ALLOC      
		,TEXT("Function")       // 13   ZONE_FUNCTION   
		,TEXT("Warning")        // 14   ZONE_WARN       
		,TEXT("Error")          // 15   ZONE_ERROR      
	},
        (1 << 15)   // Errors
    |   (1 << 14)   // Warnings
}; 
#endif

void WriteL3Addr(unsigned char data)
{	
    int i,j;
	g_pIOPregs->GPGDAT |= L3C;
	for(j=0;j<10;j++);
	
    g_pIOPregs->GPGDAT &= ~(L3D|L3M);		//L3D=L/L3M=L(in address mode)/L3C=L
    for(j=0;j<10;j++);
		                
	//RETAILMSG(1,(L"g_pIOPregs->GPGDAT%B, %X\r\n", g_pIOPregs->GPGDAT, g_pIOPregs->GPGDAT));
    		                    //tsu(L3) > 190ns
    
    //PD[8:6]=L3D:L3C:L3M
    for(i=0;i<8;i++)	                        //LSB first
    {
	if(data&0x1)	                            //if data's LSB is 'H'
	{
	    g_pIOPregs->GPGDAT &= ~L3C;	            //L3C=L
	    g_pIOPregs->GPGDAT |= L3D;	            //L3D=H		    
	    for(j=0;j<10;j++);	                    //tcy(L3) > 500ns
	    g_pIOPregs->GPGDAT |= L3C;	            //L3C=H
	    //g_pIOPregs->GPGDAT |= L3D;	            //L3D=H
	    for(j=0;j<10;j++);	                    //tcy(L3) > 500ns
	}
	else		                                //if data's LSB is 'L'
	{
	    g_pIOPregs->GPGDAT &= ~L3C;	            //L3C=L
	    g_pIOPregs->GPGDAT &= ~L3D;	            //L3D=L
	    for(j=0;j<10;j++);	                    //tcy(L3) > 500ns
	    g_pIOPregs->GPGDAT |= L3C;	            //L3C=H
	    //g_pIOPregs->GPGDAT &= ~L3D;	            //L3D=L
	    for(j=0;j<10;j++);	                    //tcy(L3) > 500ns
	}
	data >>=1;
    }
	 for(j=0;j<10;j++);
	
    g_pIOPregs->GPGDAT|=L3M;	            //L3M=H,L3C=H
	 for(j=0;j<10;j++);
	
	//RETAILMSG(1,(TEXT("smdk2440 WAVE::WriteL3Addr+\r\n"))); 
}


void WriteL3Data(unsigned char data,int halt)
{
    int i,j;

    if(halt)
    {
        g_pIOPregs->GPGDAT|=L3C;	            //L3C=H(while tstp, L3 interface halt condition)
        for(j=0;j<10;j++);                       //tstp(L3) > 190ns
    }

    g_pIOPregs->GPGDAT|=L3M;	            //L3M=H(in data transfer mode)	
    for(j=0;j<10;j++);	                        //tsu(L3)D > 190ns

    //PD[8:6]=L3D:L3C:L3M
    for(i=0;i<8;i++)
    {
        if(data&0x1)	                        //if data's LSB is 'H'
        {
	    g_pIOPregs->GPGDAT &= ~L3C;	            //L3C=L
            g_pIOPregs->GPGDAT |= L3D;	        //L3D=H
            for(j=0;j<10;j++);	                //tcy(L3) > 500ns
            g_pIOPregs->GPGDAT |= (L3C);    //L3C=H,L3D=H
            for(j=0;j<10;j++);	                //tcy(L3) > 500ns
        }
        else		//if data's LSB is 'L'
        {
	    g_pIOPregs->GPGDAT &= ~L3C;	            //L3C=L
	    g_pIOPregs->GPGDAT &= ~L3D;	            //L3D=L
            for(j=0;j<10;j++);	                //tcy(L3) > 500ns
            g_pIOPregs->GPGDAT |= L3C;	        //L3C=H
	    //g_pIOPregs->GPGDAT &= ~L3D;	            //L3D=L
            for(j=0;j<10;j++);	                //tcy(L3) > 500ns
        }
        data>>=1;	//for check next bit
    }
	for(j=0;j<10;j++);	
    g_pIOPregs->GPGDAT &= ~L3M;
	 for(j=0;j<10;j++);	 
	for(j=0;j<10;j++); 
    g_pIOPregs->GPGDAT|=L3M;	            //L3M=H,L3C=H
		 
	//RETAILMSG(1,(TEXT("smdk2440 WAVE::WriteL3Data+\r\n"))); 
}

BOOL HardwareContext::CreateHWContext(DWORD Index)
{
    if (g_pHWContext)
    {
        return(TRUE);
    }

    g_pHWContext = new HardwareContext;
    if (!g_pHWContext)
    {
        return(FALSE);
    }

    return(g_pHWContext->Init(Index));
}

HardwareContext::HardwareContext()
: m_InputDeviceContext(), m_OutputDeviceContext()
{
    InitializeCriticalSection(&m_Lock);
    m_Initialized=FALSE;
}

HardwareContext::~HardwareContext()
{
    DeleteCriticalSection(&m_Lock);
}

BOOL HardwareContext::Init(DWORD Index)
{
    UINT32 Irq;
	RETAILMSG(1,(TEXT("smdk2440 WAVE::INIT()++\r\n"))); 
	m_dwInputGain = 0xFFFF;
	m_dwOutputGain = 0xFFFF;
    m_fInputMute = FALSE;
    m_fOutputMute = FALSE;


    if (m_Initialized)
    {
        return(FALSE);
    }

    // Call the OAL to translate the audio IRQ into a SYSINTR value.
    //
    Irq = IRQ_DMA2;  // audio output DMA interrupt.
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &Irq, sizeof(UINT32), &m_dwSysintrOutput, sizeof(UINT32), NULL))
    {
        RETAILMSG(TRUE, (TEXT("ERROR: HardwareContext::Init: Failed to obtain sysintr value for output interrupt.\r\n")));
        return FALSE;
    }

    Irq = IRQ_DMA1;  // audio input DMA interrupt.
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &Irq, sizeof(UINT32), &m_dwSysintrInput, sizeof(UINT32), NULL))
    {
        RETAILMSG(TRUE, (TEXT("ERROR: HardwareContext::Init: Failed to obtain sysintr value for input interrupt.\r\n")));
        return FALSE;
    }

	//----- 1. Initialize the state/status variables -----
    m_DriverIndex		= Index;
    m_InPowerHandler    = FALSE;
    m_InputDMARunning   = FALSE;
    m_OutputDMARunning  = FALSE;
	m_InputDMAStatus	= DMA_CLEAR;				
	m_OutputDMAStatus	= DMA_CLEAR;				

    //----- 2. Map the necessary descriptory channel and control registers into the driver's virtual address space -----
	if(!MapRegisters())
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("WAVEDEV.DLL:HardwareContext::Init() - Failed to map config registers.\r\n")));
        goto Exit;
	}

    //----- 3. Map the DMA buffers into driver's virtual address space -----
    if(!MapDMABuffers())
    {
		DEBUGMSG(ZONE_ERROR, (TEXT("WAVEDEV.DLL:HardwareContext::Init() - Failed to map DMA buffers.\r\n")));
        goto Exit;
    }

    //----- 4. Configure the Codec -----
    InitCodec();
    
	//----- 5. Initialize the interrupt thread -----
    if (!InitInterruptThread())
    {
		DEBUGMSG(ZONE_ERROR, (TEXT("WAVEDEV.DLL:HardwareContext::Init() - Failed to initialize interrupt thread.\r\n")));
        goto Exit;
    }
    m_Initialized=TRUE;

Exit:
    return(m_Initialized);
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		MapRegisters()

Description:	Maps the config registers used by both the SPI and
				I2S controllers.

Notes:			The SPI and I2S controllers both use the GPIO config
				registers, so these MUST be initialized FIRST.

Returns:		Boolean indicating success
-------------------------------------------------------------------*/
BOOL HardwareContext::MapRegisters()
{

	// IIS registers.
	//
    g_pIISregs = (volatile S3C2440A_IISBUS_REG*)VirtualAlloc(0, sizeof(S3C2440A_IISBUS_REG), MEM_RESERVE, PAGE_NOACCESS);
	if (!g_pIISregs)
	{
		DEBUGMSG(1, (TEXT("IISreg: VirtualAlloc failed!\r\n")));
		return(FALSE);
	}
	if (!VirtualCopy((PVOID)g_pIISregs, (PVOID)(S3C2440A_BASE_REG_PA_IISBUS >> 8), sizeof(S3C2440A_IISBUS_REG), PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE))
	{
		DEBUGMSG(1, (TEXT("IISreg: VirtualCopy failed!\r\n")));
		return(FALSE);
	}

	// SPI registers.
	//
    g_pSPIregs = (volatile S3C2440A_SPI_REG*)VirtualAlloc(0, sizeof(S3C2440A_SPI_REG), MEM_RESERVE, PAGE_NOACCESS);
	if (!g_pSPIregs)
	{
		DEBUGMSG(1, (TEXT("SPIreg: VirtualAlloc failed!\r\n")));
		return(FALSE);
	}
	if (!VirtualCopy((PVOID)g_pSPIregs, (PVOID)(S3C2440A_BASE_REG_PA_SPI >> 8), sizeof(S3C2440A_SPI_REG), PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE))
	{
		DEBUGMSG(1, (TEXT("SPIreg: VirtualCopy failed!\r\n")));
		return(FALSE);
	}

	g_pIOPregs = (volatile S3C2440A_IOPORT_REG*)VirtualAlloc(0, sizeof(S3C2440A_IOPORT_REG), MEM_RESERVE, PAGE_NOACCESS);
	if (!g_pIOPregs)
	{
		DEBUGMSG(1, (TEXT("IOPreg: VirtualAlloc failed!\r\n")));
		return(FALSE);
	}
	if (!VirtualCopy((PVOID)g_pIOPregs, (PVOID)(S3C2440A_BASE_REG_PA_IOPORT >> 8), sizeof(S3C2440A_IOPORT_REG), PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE))
	{
		DEBUGMSG(1, (TEXT("IOPreg: VirtualCopy failed!\r\n")));
		return(FALSE);
	}

	g_pDMAregs = (volatile S3C2440A_DMA_REG*)VirtualAlloc(0, sizeof(S3C2440A_DMA_REG), MEM_RESERVE, PAGE_NOACCESS);
	if (!g_pDMAregs)
	{
		DEBUGMSG(1, (TEXT("DMAreg: VirtualAlloc failed!\r\n")));
		return(FALSE);
	}
	if (!VirtualCopy((PVOID)g_pDMAregs, (PVOID)(S3C2440A_BASE_REG_PA_DMA >> 8), sizeof(S3C2440A_DMA_REG), PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE))
	{
		DEBUGMSG(1, (TEXT("DMAreg: VirtualCopy failed!\r\n")));
		return(FALSE);
	}

	s2440INT = (volatile S3C2440A_INTR_REG*)VirtualAlloc(0, sizeof(S3C2440A_INTR_REG), MEM_RESERVE, PAGE_NOACCESS);
	if (!g_pDMAregs)
	{
		DEBUGMSG(1, (TEXT("INTreg: VirtualAlloc failed!\r\n")));
		return(FALSE);
	}
	if (!VirtualCopy((PVOID)s2440INT, (PVOID)(S3C2440A_BASE_REG_PA_INTR >> 8), sizeof(S3C2440A_INTR_REG), PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE))
	{
		DEBUGMSG(1, (TEXT("INTreg: VirtualCopy failed!\r\n")));
		return(FALSE);
	}

	g_pCLKPWRreg = (volatile S3C2440A_CLKPWR_REG*)VirtualAlloc(0, sizeof(S3C2440A_CLKPWR_REG), MEM_RESERVE, PAGE_NOACCESS);
	if (!g_pCLKPWRreg)
	{
		DEBUGMSG(1, (TEXT("DMAreg: VirtualAlloc failed!\r\n")));
		return(FALSE);
	}
	if (!VirtualCopy((PVOID)g_pCLKPWRreg, (PVOID)(S3C2440A_BASE_REG_PA_CLOCK_POWER >> 8), sizeof(S3C2440A_CLKPWR_REG), PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE))
	{
		DEBUGMSG(1, (TEXT("DMAreg: VirtualCopy failed!\r\n")));
		return(FALSE);
	}
   
    PowerUp();

	return(TRUE);
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		Deinit()

Description:	Deinitializest the hardware: disables DMA channel(s), 
				clears any pending interrupts, powers down the audio
				codec chip, etc.

Returns:		Boolean indicating success
-------------------------------------------------------------------*/
BOOL HardwareContext::Deinit()
{
	//----- 1. Disable the input/output channels -----
	AUDIO_IN_DMA_DISABLE();
	AUDIO_OUT_DMA_DISABLE();

	//----- 2. Disable/clear DMA input/output interrupts -----
	AUDIO_IN_CLEAR_INTERRUPTS();
	AUDIO_OUT_CLEAR_INTERRUPTS();

	//----- 3. Turn the audio hardware off -----
    AudioMute(DMA_CH_OUT | DMA_CH_MIC, TRUE);

    //----- 4. Unmap the control registers and DMA buffers -----
    UnmapRegisters();
	UnmapDMABuffers();

    return TRUE;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		UnmapRegisters()

Description:	Unmaps the config registers used by both the SPI and
				I2S controllers.

Notes:			The SPI and I2S controllers both use the GPIO config
				registers, so these MUST be deinitialized LAST.

Returns:		Boolean indicating success
-------------------------------------------------------------------*/
BOOL HardwareContext::UnmapRegisters()
{
	return TRUE;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		MapDMABuffers()

Description:	Maps the DMA buffers used for audio input/output
				on the I2S bus.

Returns:		Boolean indicating success
-------------------------------------------------------------------*/
BOOL HardwareContext::MapDMABuffers()
{
	//RETAILMSG(1,(TEXT("MapDMABuffers\n")));
    PBYTE pVirtDMABufferAddr = NULL;
    DMA_ADAPTER_OBJECT Adapter;


    memset(&Adapter, 0, sizeof(DMA_ADAPTER_OBJECT));
    Adapter.InterfaceType = Internal;
    Adapter.ObjectSize = sizeof(DMA_ADAPTER_OBJECT);

    // Allocate a block of virtual memory (physically contiguous) for the DMA buffers.
    //
    pVirtDMABufferAddr = (PBYTE)HalAllocateCommonBuffer(&Adapter, (AUDIO_DMA_PAGE_SIZE * 4), &g_PhysDMABufferAddr, FALSE);
    if (pVirtDMABufferAddr == NULL)
    {
        RETAILMSG(TRUE, (TEXT("WAVEDEV.DLL:HardwareContext::MapDMABuffers() - Failed to allocate DMA buffer.\r\n")));
        return(FALSE);
    }

    // Setup the DMA page pointers.
    // NOTE: Currently, input and output each have two DMA pages: these pages are used in a round-robin
    // fashion so that the OS can read/write one buffer while the audio codec chip read/writes the other buffer.
    //
    m_Output_pbDMA_PAGES[0] = pVirtDMABufferAddr;
    m_Output_pbDMA_PAGES[1] = pVirtDMABufferAddr + AUDIO_DMA_PAGE_SIZE;
    m_Input_pbDMA_PAGES[0]  = pVirtDMABufferAddr + (2 * AUDIO_DMA_PAGE_SIZE);
    m_Input_pbDMA_PAGES[1]  = pVirtDMABufferAddr + (3 * AUDIO_DMA_PAGE_SIZE);

    return(TRUE);
	
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		UnmapDMABuffers()

Description:	Unmaps the DMA buffers used for audio input/output
				on the I2S bus.

Returns:		Boolean indicating success
-------------------------------------------------------------------*/
BOOL HardwareContext::UnmapDMABuffers()
{
	if(m_Output_pbDMA_PAGES[0])
	{
		VirtualFree((PVOID)m_Output_pbDMA_PAGES[0], 0, MEM_RELEASE);
	}

	return TRUE;
}

BOOL HardwareContext::Codec_channel()
{
	//RETAILMSG(1,(TEXT("Codec_channel\n")));

    UCHAR status = UDA1341_STATUS_B | UDA1341_STATUS_IGS; //enable 6dB mic boost
    // which parts to turn on?
    if (m_InputDMARunning) {
        status |= UDA1341_STATUS_PWR_ADC; // turn on ADC
    }
    if (m_OutputDMARunning) {
        status |= UDA1341_STATUS_PWR_DAC; // turn on DAC
    }

    //****** Port B Initialize *****
   g_pIOPregs->GPGDAT |= L3M|L3C;	//start condition : L3M=H, L3C=H
   

	
	WriteL3Addr(UDA1341_ADDR_STATUS);
    WriteL3Data(status, 0);

    return(TRUE);
}

MMRESULT HardwareContext::SetOutputGain (DWORD dwGain)
{
	RETAILMSG(1,(TEXT("SetOutputGain\n")));
    m_dwOutputGain = dwGain & 0xffff; // save off so we can return this from GetGain - but only MONO
    // convert 16-bit gain to 5-bit attenuation
    UCHAR ucGain;
    if (m_dwOutputGain == 0) {
        ucGain = 0x3F; // mute: set maximum attenuation
    }
    else {
        ucGain = (UCHAR) ((0xffff - m_dwOutputGain) >> 11); // codec supports 64dB attenuation, we'll only use 32
    }
    ASSERT((ucGain & 0xC0) == 0); // bits 6,7 clear indicate DATA0 in Volume mode.

    WriteL3Addr(UDA1341_ADDR_DATA0);
    WriteL3Data(ucGain, 0); 

    return MMSYSERR_NOERROR;
}

MMRESULT HardwareContext::SetOutputMute (BOOL fMute)
{
	//RETAILMSG(1,(TEXT("SetOutputMute\n")));
    m_fOutputMute = fMute;

    WriteL3Addr(UDA1341_ADDR_DATA0);
    WriteL3Data(fMute ? 0x84 : 0x80, 0); // DATA0: 0x80 + fMute << 2

    return MMSYSERR_NOERROR;
}

BOOL HardwareContext::GetOutputMute (void)
{
    return m_fOutputMute;
}

DWORD HardwareContext::GetOutputGain (void)
{
    return m_dwOutputGain;
}

BOOL HardwareContext::GetInputMute (void)
{
    return m_fInputMute;
}

MMRESULT HardwareContext::SetInputMute (BOOL fMute)
{
    m_fInputMute = fMute;
    return m_InputDeviceContext.SetGain(fMute ? 0: m_dwInputGain);
}

DWORD HardwareContext::GetInputGain (void)
{
    return m_dwInputGain;
}

MMRESULT HardwareContext::SetInputGain (DWORD dwGain)
{
    m_dwInputGain = dwGain;
    if (! m_fInputMute) {
        m_InputDeviceContext.SetGain(dwGain);
    }
    return MMSYSERR_NOERROR;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		InitCodec()

Description:	Initializes the audio codec chip.

Notes:			The audio codec chip is intialized for output mode
				but powered down.  To conserve battery life, the chip
				is only powered up when the user starts playing a 
				file.

				Specifically, the powerup/powerdown logic is done 
				in the AudioMute() function.  If either of the 
				audio channels are unmuted, then the chip is powered
				up; otherwise the chip is powered own.

Returns:		Boolean indicating success
-------------------------------------------------------------------*/
BOOL HardwareContext::InitCodec()
{
	
	DEBUGMSG(1, (TEXT("+++InitCodec\n")));

    g_pIOPregs->GPGDAT |= L3M|L3C;	//start condition : L3M=H, L3C=H

	g_pIOPregs->GPGUP  |= 0x0301;	
	g_pIOPregs->GPGCON = ((g_pIOPregs->GPGCON & 0xFFF0FFFC) | 0x00050001); 
	
	unsigned long flags;
	uda1341_volume = 300;//62 - ((DEF_VOLUME * 61) / 100);
	uda1341_boost = 0;
	uda_sampling = DATA2_DEEMP_NONE;
	uda_sampling &= ~(DATA2_MUTE);
	g_pIOPregs->GPGDAT|=L3C|L3M;
    WriteL3Addr(UDA1341_ADDR_STATUS);
    WriteL3Data(UDA1341_STATUS_RESET ,0);

    WriteL3Addr(UDA1341_ADDR_STATUS);
    WriteL3Data(UDA1341_STATUS_CLK384|UDA1341_STATUS_FMT_MSB|UDA1341_STATUS_DCFILTER,0);

    // set default gain, power state
    WriteL3Addr(UDA1341_ADDR_STATUS);
    WriteL3Data(UDA1341_STATUS_B | UDA1341_STATUS_IGS | UDA1341_STATUS_OGS|UDA1341_STATUS_PWR_DAC|UDA1341_STATUS_PWR_ADC,0);

    // extended address:
	WriteL3Addr(UDA1341_ADDR_DATA0);

	WriteL3Data(DATA0 |DATA0_VOLUME(uda1341_volume),0);
	WriteL3Data(DATA1 |DATA1_BASS(uda1341_boost)| DATA1_TREBLE(0),0);
	WriteL3Data(DATA2 |uda_sampling,0);
	WriteL3Data(EXTADDR(EXT2),0);
	WriteL3Data(EXTDATA(EXT2_MIC_GAIN(0x6)| EXT2_MIXMODE_CH2),0);
	WriteL3Data(EXTADDR(EXT5),0);
	WriteL3Data(EXTDATA(0x7),0);
	

	return(TRUE);
}



/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		InitOutputDMA()

Description:	Initializes the DMA channel for output.

Notes:			DMA Channel 2 is used for transmitting output sound
				data from system memory to the I2S controller.

Returns:		Boolean indicating success
-------------------------------------------------------------------*/
BOOL HardwareContext::InitOutputDMA()
{
	//RETAILMSG(1,(TEXT("InitOutputDMA\r\n")));
	
	//----- 1. Initialize the DMA channel for output mode and use the first output DMA buffer -----
    if (!g_PhysDMABufferAddr.LowPart)
    {
        DEBUGMSG(TRUE, (TEXT("ERROR:HardwareContext::InitOutputDMA: Invalid DMA buffer physical address.\r\n")));
        return(FALSE);
    }
    g_pDMAregs->DISRC2	= (int)(g_PhysDMABufferAddr.LowPart); 
    g_pDMAregs->DISRCC2 &= ~(SOURCE_PERIPHERAL_BUS | FIXED_SOURCE_ADDRESS);				// Source is system bus, increment addr

    //----- 2. Initialize the DMA channel to send data over the I2S bus -----
    g_pDMAregs->DIDST2	= (int)(S3C2440A_BASE_REG_PA_IISBUS+0x10); 
    g_pDMAregs->DIDSTC2 |= (DESTINATION_PERIPHERAL_BUS | FIXED_DESTINATION_ADDRESS);	// Dest is  periperal bus, fixed addr

	//----- 3. Configure the DMA channel's transfer characteristics: handshake, sync PCLK, interrupt, -----
	//		   single tx, single service, I2SSDO, I2S request, no auto-reload, half-word, tx count
	g_pDMAregs->DCON2	= (  HANDSHAKE_MODE | GENERATE_INTERRUPT | I2SSDO_DMA2 | DMA_TRIGGERED_BY_HARDWARE 
                           | TRANSFER_HALF_WORD | (AUDIO_DMA_PAGE_SIZE / 2) );
	
	//RETAILMSG(1,(L"g_pDMAregs->DISRC2%X, DISRCC2%X,DIDST2%x,DIDSTC2,DCON2%x\r\n", g_pDMAregs->DISRC2, g_pDMAregs->DISRCC2,g_pDMAregs->DIDST2,g_pDMAregs->DIDSTC2,g_pDMAregs->DCON2));
	//----- 4. Reset the playback pointers -----
	AUDIO_RESET_PLAYBACK_POINTER();
	
	DEBUGMSG(ZONE_FUNCTION,(TEXT("---InitOutputDMA\n")));

	return TRUE;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		StartOutputDMA()

Description:	Starts outputting the sound data to the audio codec
				chip via DMA.

Notes:			Currently, both playback and record share the same
				DMA channel.  Consequently, we can only start this
				operation if the input channel isn't using DMA.

Returns:		Boolean indicating success
-------------------------------------------------------------------*/
BOOL HardwareContext::StartOutputDMA()
{
	DEBUGMSG(ZONE_FUNCTION,(TEXT("+++StartOutputDMA\r\n")));
	
    if(!m_OutputDMARunning)
    {
        //----- 1. Initialize our buffer counters -----
        m_OutputDMARunning=TRUE;
        m_OutBytes[OUT_BUFFER_A]=m_OutBytes[OUT_BUFFER_B]=0;

        //----- 2. Prime the output buffer with sound data -----
		m_OutputDMAStatus = (DMA_DONEA | DMA_DONEB) & ~DMA_BIU;	
		ULONG OutputTransferred = TransferOutputBuffers(m_OutputDMAStatus);
        
		//----- 3. If we did transfer any data to the DMA buffers, go ahead and enable DMA -----
		if(OutputTransferred)
        {
			//----- 4. Configure the DMA channel for playback -----
			if(!InitOutputDMA())
			{
				DEBUGMSG(ZONE_ERROR, (TEXT("HardwareContext::StartOutputDMA() - Unable to initialize output DMA channel!\r\n")));
				goto START_ERROR;
			}
			g_pIISregs->IISCON |= TRANSMIT_DMA_REQUEST_ENABLE;
			g_pIISregs->IISCON &= ~TRANSMIT_IDLE_CMD;
			g_pIISregs->IISFCON |= (  TRANSMIT_FIFO_ACCESS_DMA | TRANSMIT_FIFO_ENABLE  );
			g_pIISregs->IISMOD  |= IIS_TRANSMIT_MODE;
			//----- 5. Make sure the audio isn't muted -----
			AudioMute(DMA_CH_OUT, FALSE);					
			//RETAILMSG(1,(L"g_pIISregs->IISCON%X, IISCON%X,IISFCON%x,IISMOD%x\r\n", g_pIISregs->IISCON, g_pIISregs->IISCON,g_pIISregs->IISFCON,g_pIISregs->IISMOD));
			//----- 6. Start the DMA controller -----
			AUDIO_RESET_PLAYBACK_POINTER();
			SELECT_AUDIO_DMA_OUTPUT_BUFFER_A();
			Codec_channel();								// Turn ON output channel
			// charlie, start A buffer
			AUDIO_OUT_DMA_ENABLE();
	        // wait for DMA to start.
	        while((g_pDMAregs->DSTAT2&0xfffff)==0);        
	        // change the buffer pointer
	        SELECT_AUDIO_DMA_OUTPUT_BUFFER_B();
	        // Set DMA for B Buffer
        }
        else    // We didn't transfer any data, so DMA wasn't enabled
        {
            m_OutputDMARunning=FALSE;
        }
    }
	//RETAILMSG(1,(L"g_pDMAregs->DISRC2%X, DISRCC2%X,DIDST2%x,DIDSTC2,DCON2%x\r\n", g_pDMAregs->DISRC2, g_pDMAregs->DISRCC2,g_pDMAregs->DIDST2,g_pDMAregs->DIDSTC2,g_pDMAregs->DCON2));
	
    DEBUGMSG(ZONE_FUNCTION,(TEXT("---StartOutputDMA\r\n")));

	return TRUE;

START_ERROR:
	return FALSE;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		StopOutputDMA()

Description:	Stops any DMA activity on the output channel.

Returns:		Boolean indicating success
-------------------------------------------------------------------*/
void HardwareContext::StopOutputDMA()
{
	//RETAILMSG(1,(TEXT("StopOutputDMA\r\n")));
	//----- 1. If the output DMA is running, stop it -----
    if (m_OutputDMARunning)
	{
		m_OutputDMAStatus = DMA_CLEAR;				
		AUDIO_OUT_DMA_DISABLE();
		AUDIO_OUT_CLEAR_INTERRUPTS();
		g_pIISregs->IISCON &= ~TRANSMIT_DMA_REQUEST_ENABLE;
		g_pIISregs->IISCON |= TRANSMIT_IDLE_CMD;
		g_pIISregs->IISFCON &= ~(  TRANSMIT_FIFO_ACCESS_DMA | TRANSMIT_FIFO_ENABLE  );
		g_pIISregs->IISMOD  &= ~IIS_TRANSMIT_MODE;
		AudioMute(DMA_CH_OUT, TRUE);		
    }

	m_OutputDMARunning = FALSE;
	Codec_channel();
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		InitInputDMA()

Description:	Initializes the DMA channel for input.

Notes:			***** NOT IMPLEMENTED *****

				The following routine is not implemented due to a
				hardware bug in the revision of the Samsung SC2440
				CPU this driver was developed on.  See the header
				at the top of this file for details.

Returns:		Boolean indicating success
-------------------------------------------------------------------*/
BOOL HardwareContext::InitInputDMA()
{

	DEBUGMSG(ZONE_FUNCTION,(TEXT("+++InitInputDMA\r\n")));

    if (!g_PhysDMABufferAddr.LowPart)
    {
        DEBUGMSG(TRUE, (TEXT("ERROR:HardwareContext::InitInputDMA: Invalid DMA buffer physical address.\r\n")));
        return(FALSE);
    }

	//============================ Configure DMA Channel 1 ===========================
	//------ On platforms with the revsion of the Samsung SC2440 CPU with the IIS SLAVE bug fix, this -----
	//		 code can be used to configure DMA channel 1 for input.

	//----- 1. Initialize the DMA channel for input mode and use the first input DMA buffer -----
	g_pDMAregs->DISRC1	= (int)(S3C2440A_BASE_REG_PA_IISBUS+0x10);
	g_pDMAregs->DISRCC1 = (SOURCE_PERIPHERAL_BUS | FIXED_SOURCE_ADDRESS);				// Source is periperal bus, fixed addr
    
    //----- 2. Initialize the DMA channel to receive data over the I2S bus -----
    g_pDMAregs->DIDST1	= (int)(g_PhysDMABufferAddr.LowPart); 
    g_pDMAregs->DIDSTC1 &= ~(DESTINATION_PERIPHERAL_BUS | FIXED_DESTINATION_ADDRESS);	// Destination is system bus, increment addr

	//----- 3. Configure the DMA channel's transfer characteristics: handshake, sync PCLK, interrupt, -----
	//		   single tx, single service, I2SSDI, I2S request, no auto-reload, half-word, tx count
	g_pDMAregs->DCON1	= (  HANDSHAKE_MODE | GENERATE_INTERRUPT | I2SSDI_DMA1 | DMA_TRIGGERED_BY_HARDWARE 
                           | TRANSFER_HALF_WORD | (AUDIO_DMA_PAGE_SIZE / 2)
                            );

	
	return(TRUE);
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		StartInputDMA()

Description:	Starts inputting the recorded sound data from the 
				audio codec chip via DMA.

Notes:			***** NOT IMPLEMENTED *****

				The following routine is not implemented due to a
				hardware bug in the revision of the Samsung SC2440
				CPU this driver was developed on.  See the header
				at the top of this file for details.

Returns:		Boolean indicating success
-------------------------------------------------------------------*/
BOOL HardwareContext::StartInputDMA()
{
	
	//------ On platforms with the revsion of the Samsung SC2440 CPU with the IIS SLAVE bug fix, this -----
	//		 code can be used to configure DMA channel 1 for input.

	DEBUGMSG(ZONE_FUNCTION,(TEXT("+++StartInputDMA\r\n")));

	if(!m_InputDMARunning)
    {
        //----- 1. Initialize our buffer counters -----
        m_InputDMARunning=TRUE;
		Codec_channel();        // Turn On Input channel
        m_InBytes[IN_BUFFER_A]=m_InBytes[IN_BUFFER_B]=0;

        //----- 2. Prime the output buffer with sound data -----
		m_InputDMAStatus = (DMA_DONEA | DMA_DONEB) & ~DMA_BIU;	

		//----- 3. Configure the DMA channel for record -----
		if(!InitInputDMA())
		{
			DEBUGMSG(ZONE_ERROR, (TEXT("HardwareContext::StartInputDMA() - Unable to initialize input DMA channel!\r\n")));
			goto START_ERROR;
		}
		g_pIISregs->IISFCON |= (RECEIVE_FIFO_ACCESS_DMA  | RECEIVE_FIFO_ENABLE);
		g_pIISregs->IISCON  |= RECEIVE_DMA_REQUEST_ENABLE;	 
		g_pIISregs->IISMOD  |= IIS_RECEIVE_MODE;
	
		//----- 4. Make sure the audio isn't muted -----
		AudioMute(DMA_CH_MIC, FALSE);					

		//----- 5. Start the input DMA -----
		AUDIO_RESET_RECORD_POINTER();
		SELECT_AUDIO_DMA_INPUT_BUFFER_A();

		Codec_channel();        // Turn On Input channel
		g_pDMAregs->DMASKTRIG1 = ENABLE_DMA_CHANNEL;		
		
        // wait for DMA to start.
        while((g_pDMAregs->DSTAT1&0xfffff)==0);        
        // change the buffer pointer
        SELECT_AUDIO_DMA_INPUT_BUFFER_B();
				
    }
    DEBUGMSG(ZONE_FUNCTION,(TEXT("---StartInputDMA\n")));
	return(TRUE);

START_ERROR:
	return(FALSE);
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		StopInputDMA()

Description:	Stops any DMA activity on the input channel.

Notes:			***** NOT IMPLEMENTED *****

				The following routine is not implemented due to a
				hardware bug in the revision of the Samsung SC2440
				CPU this driver was developed on.  See the header
				at the top of this file for details.

Returns:		Boolean indicating success
-------------------------------------------------------------------*/
void HardwareContext::StopInputDMA()
{
	RETAILMSG(1,(TEXT("StopInputDMA\r\n")));

	//------ On platforms with the revsion of the Samsung SC2440 CPU with the IIS SLAVE bug fix, this -----
	//		 code can be used to configure DMA channel 1 for input.

	//----- 1. If the output DMA is running, stop it -----
    if (m_InputDMARunning)
	{
		m_InputDMAStatus = DMA_CLEAR;				
		
		g_pIISregs->IISCON &= ~RECEIVE_DMA_REQUEST_ENABLE;
		g_pIISregs->IISFCON &= ~(RECEIVE_FIFO_ACCESS_DMA  | RECEIVE_FIFO_ENABLE);
		g_pDMAregs->DMASKTRIG1 |= STOP_DMA_TRANSFER;
		g_pDMAregs->DMASKTRIG1 &= ~ENABLE_DMA_CHANNEL;
		g_pIISregs->IISMOD  &= ~IIS_RECEIVE_MODE;
		AUDIO_IN_CLEAR_INTERRUPTS();
		AudioMute(DMA_CH_MIC, TRUE);		
    }

	m_InputDMARunning = FALSE;
	Codec_channel();
}


DWORD HardwareContext::GetInterruptThreadPriority()
{
	RETAILMSG(1,(TEXT("GetInterruptThreadPriority\r\n")));
    HKEY hDevKey;
    DWORD dwValType;
    DWORD dwValLen;
    DWORD dwPrio = 249; // Default priority

    hDevKey = OpenDeviceKey((LPWSTR)m_DriverIndex);
    if (hDevKey)
    {
        dwValLen = sizeof(DWORD);
        RegQueryValueEx(
            hDevKey,
            TEXT("Priority256"),
            NULL,
            &dwValType,
            (PUCHAR)&dwPrio,
            &dwValLen);
        RegCloseKey(hDevKey);
    }

    return dwPrio;
}



/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		InitInterruptThread()

Description:	Initializes the IST for handling DMA interrupts.

Returns:		Boolean indicating success
-------------------------------------------------------------------*/
BOOL HardwareContext::InitInterruptThread()
{
	//RETAILMSG(1,(TEXT("InitInterruptThread\r\n")));

    m_hAudioInterrupt = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!m_hAudioInterrupt)
    {
        ERRMSG("Unable to create interrupt event");
        return(FALSE);
    }

    if (! InterruptInitialize(m_dwSysintrOutput, m_hAudioInterrupt, NULL, 0)) {
        ERRMSG("Unable to initialize output interrupt");
        return FALSE;
    }
    if (! InterruptInitialize(m_dwSysintrInput, m_hAudioInterrupt, NULL, 0)) {
        ERRMSG("Unable to initialize input interrupt");
        return FALSE;
    }

    m_hAudioInterruptThread  = CreateThread((LPSECURITY_ATTRIBUTES)NULL,
                                            0,
                                            (LPTHREAD_START_ROUTINE)CallInterruptThread,
                                            this,
                                            0,
                                            NULL);
    if (!m_hAudioInterruptThread)
    {
        ERRMSG("Unable to create interrupt thread");
        return FALSE;
    }

    // Bump up the priority since the interrupt must be serviced immediately.
	CeSetThreadPriority(m_hAudioInterruptThread, GetInterruptThreadPriority());

    return(TRUE);
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		PowerUp()

Description:	Powers up the audio codec chip.

Notes:			Currently, this function is unimplemented because
				the audio codec chip is ONLY powered up when the 
				user wishes to play or record.  The AudioMute() function
				handles the powerup sequence.

Returns:		Boolean indicating success
-------------------------------------------------------------------*/
void HardwareContext::PowerUp()
{
//    SPI_Init();
    I2S_Init();
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		PowerDown()

Description:	Powers down the audio codec chip.

Notes:			Even if the input/output channels are muted, this
				function powers down the audio codec chip in order
				to conserve battery power.

Returns:		Boolean indicating success
-------------------------------------------------------------------*/
void HardwareContext::PowerDown()
{
    StopOutputDMA();
	AudioMute((DMA_CH_OUT | DMA_CH_MIC), TRUE);
}


//############################################ Helper Functions #############################################

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		TransferOutputBuffer()

Description:	Retrieves the next "mixed" audio buffer of data to
				DMA into the output channel.

Returns:		Number of bytes needing to be transferred.
-------------------------------------------------------------------*/
ULONG HardwareContext::TransferOutputBuffer(ULONG NumBuf)
{
	//RETAILMSG(1,(TEXT("TransferOutputBuffer\r\n")));
    ULONG BytesTransferred = 0;

    PBYTE pBufferStart = m_Output_pbDMA_PAGES[NumBuf];
    PBYTE pBufferEnd = pBufferStart + AUDIO_DMA_PAGE_SIZE;
    PBYTE pBufferLast;

	__try
	{
		pBufferLast = m_OutputDeviceContext.TransferBuffer(pBufferStart, pBufferEnd,NULL);
		BytesTransferred = m_OutBytes[NumBuf] = pBufferLast-pBufferStart;

		// Enable if you need to clear the rest of the DMA buffer
		StreamContext::ClearBuffer(pBufferLast,pBufferEnd);
		if(NumBuf == OUT_BUFFER_A)			// Output Buffer A
		{
			m_OutputDMAStatus &= ~DMA_DONEA;
			m_OutputDMAStatus |= DMA_STRTA;
			//RETAILMSG(1,(TEXT("TransferOutputBufferA\r\n")));
		}
		else								// Output Buffer B
		{
			m_OutputDMAStatus &= ~DMA_DONEB;
			m_OutputDMAStatus |= DMA_STRTB;
			//RETAILMSG(1,(TEXT("TransferOutputBufferB\r\n")));
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER) 
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("WAVDEV2.DLL:TransferOutputBuffer() - EXCEPTION: %d"), GetExceptionCode()));
	}

    return BytesTransferred;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		TransferOutputBuffers()

Description:	Determines which output buffer (A or B) needs to 
				be filled with sound data.  The correct buffer is
				then populated with data and ready to DMA to the 
				output channel.

Returns:		Boolean indicating success
-------------------------------------------------------------------*/
ULONG HardwareContext::TransferOutputBuffers(DWORD dwDCSR)
{
	//RETAILMSG(1,(TEXT("TransferOutputBuffers\r\n")));
    ULONG BytesTransferred = 0;

    ULONG BytesTotal;
    DWORD Bits = dwDCSR & (DMA_DONEA|DMA_DONEB|DMA_BIU);

//	RETAILMSG(1,(TEXT("%x\n"),Bits));
	switch (Bits)
    {
    case 0:
    case DMA_BIU:
        // No done bits set- must not be my interrupt
        return 0;
    case DMA_DONEA|DMA_DONEB|DMA_BIU:
        // Load B, then A
        BytesTransferred = TransferOutputBuffer(OUT_BUFFER_B);
        // fall through
    case DMA_DONEA: // This should never happen!
    case DMA_DONEA|DMA_BIU:
		BytesTransferred += TransferOutputBuffer(OUT_BUFFER_A);		// charlie, A => B
        break;
    case DMA_DONEA|DMA_DONEB:
        // Load A, then B
        BytesTransferred = TransferOutputBuffer(OUT_BUFFER_A);
        // charlie
        BytesTransferred += TransferOutputBuffer(OUT_BUFFER_B);
        break;		// charlie
        // fall through
    case DMA_DONEB|DMA_BIU: // This should never happen!
    case DMA_DONEB:
        // Load B
        BytesTransferred += TransferOutputBuffer(OUT_BUFFER_B);		// charlie, B => A
        break;
    }

    // If it was our interrupt, but we weren't able to transfer any bytes
    // (e.g. no full buffers ready to be emptied)
    // and all the output DMA buffers are now empty, then stop the output DMA
    BytesTotal = m_OutBytes[OUT_BUFFER_A]+m_OutBytes[OUT_BUFFER_B];

	if (BytesTotal==0)
    {
		StopOutputDMA();
    }
    return BytesTransferred;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		TransferInputBuffer()

Description:	Retrieves the chunk of recorded sound data and inputs
				it into an audio buffer for potential "mixing".

Returns:		Number of bytes needing to be transferred.
-------------------------------------------------------------------*/
ULONG HardwareContext::TransferInputBuffer(ULONG NumBuf)
{
		//RETAILMSG(1,(TEXT("TransferInputBuffer\r\n")));
    ULONG BytesTransferred = 0;

    PBYTE pBufferStart = m_Input_pbDMA_PAGES[NumBuf];
    PBYTE pBufferEnd = pBufferStart + AUDIO_DMA_PAGE_SIZE;
    PBYTE pBufferLast;

	__try
	{
		pBufferLast = m_InputDeviceContext.TransferBuffer(pBufferStart, pBufferEnd,NULL);
		BytesTransferred = m_InBytes[NumBuf] = pBufferLast-pBufferStart;

		if(NumBuf == IN_BUFFER_A)			// Input Buffer A
		{
			m_InputDMAStatus &= ~DMA_DONEA;
			m_InputDMAStatus |= DMA_STRTA;
		}
		else								// Input Buffer B
		{
			m_InputDMAStatus &= ~DMA_DONEB;
			m_InputDMAStatus |= DMA_STRTB;
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER) 
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("WAVDEV2.DLL:TransferInputBuffer() - EXCEPTION: %d"), GetExceptionCode()));
	}

    return BytesTransferred;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		TransferInputBuffers()

Description:	Determines which input buffer (A or B) needs to 
				be filled with recorded sound data.  The correct 
				buffer is then populated with recorded sound data
				from the input channel.

Returns:		Boolean indicating success
-------------------------------------------------------------------*/
ULONG HardwareContext::TransferInputBuffers(DWORD dwDCSR)
{
	//RETAILMSG(1,(TEXT("TransferInputBuffers\r\n")));
    ULONG BytesTransferred=0;
    DWORD Bits = dwDCSR & (DMA_DONEA|DMA_DONEB|DMA_BIU);

    switch (Bits)
    {
    case 0:
    case DMA_BIU:
        // No done bits set- must not be my interrupt
        return 0;
    case DMA_DONEA|DMA_DONEB|DMA_BIU:
        // Load B, then A
        BytesTransferred = TransferInputBuffer(IN_BUFFER_B);
        // fall through
    case DMA_DONEA: // This should never happen!
    case DMA_DONEA|DMA_BIU:
        // Load A
        BytesTransferred += TransferInputBuffer(IN_BUFFER_A);
        break;
    case DMA_DONEA|DMA_DONEB:
        // Load A, then B
        BytesTransferred = TransferInputBuffer(IN_BUFFER_A);
		BytesTransferred += TransferInputBuffer(IN_BUFFER_B);
		break;
        // fall through
    case DMA_DONEB|DMA_BIU: // This should never happen!
    case DMA_DONEB:
        // Load B
        BytesTransferred += TransferInputBuffer(IN_BUFFER_B);
        break;
    }

    // If it was our interrupt, but we weren't able to transfer any bytes
    // (e.g. no empty buffers ready to be filled)
    // Then stop the input DMA
    if (BytesTransferred==0)
    {
        StopInputDMA();
    }
    return BytesTransferred;
}


void HardwareContext::InterruptThread()
{
    ULONG InputTransferred, OutputTransferred;
	BOOL dmaInterruptSource = 0;

    // Fast way to access embedded pointers in wave headers in other processes.
	SetProcPermissions((DWORD)-1);

    while(TRUE)
    {
		//RETAILMSG(1,(TEXT("InterruptThread\r\n")));
		WaitForSingleObject(m_hAudioInterrupt, INFINITE);
		dmaInterruptSource = 0;
		//----- 1. Grab the lock -----
		Lock();

		__try
		{

			//----- 3. Determine the interrupt source (input DMA operation or output DMA operation?) -----
			//----- NOTE:	Often, platforms use two separate DMA channels for input/output operations but
			//				have the OAL return SYSINTR_AUDIO as the interrupt source.  If this is the case,
			//				then the interrupt source (input or output DMA channel) must be determined in
			//				this step.

			if( s2440INT->INTMSK & (1 << IRQ_DMA1) ){
				dmaInterruptSource |= DMA_CH_MIC;								// Input DMA is supported...
			    //----- 2. Acknowledge the DMA interrupt -----
			    InterruptDone(m_dwSysintrInput);
			}
			
			if( s2440INT->INTMSK & (1 << IRQ_DMA2) ){
				dmaInterruptSource |= DMA_CH_OUT;								// Output DMA is supported...
			    //----- 2. Acknowledge the DMA interrupt -----
			    InterruptDone(m_dwSysintrOutput);
			}
						
           
            //----- 4. Handle any interrupts on the input source -----
			//		   NOTE: The InterruptDone() call below automatically clears the interrupt.
			if ((dmaInterruptSource & DMA_CH_MIC) != 0)
			{
				//----- Determine which buffer just completed the DMA transfer -----
				if(m_InputDMAStatus & DMA_BIU)
				{
					m_InputDMAStatus &= ~DMA_STRTB;							// Buffer B just completed...
					m_InputDMAStatus |= DMA_DONEB;

					m_InputDMAStatus &= ~DMA_BIU;							// Buffer A is in use
					SELECT_AUDIO_DMA_INPUT_BUFFER_B();
					DEBUGMSG(ZONE_FUNCTION,(TEXT("1\n")));
				}else
				{
					m_InputDMAStatus &= ~DMA_STRTA;							// Buffer A just completed...
					m_InputDMAStatus |= DMA_DONEA;

					m_InputDMAStatus |= DMA_BIU;							// Buffer B is in use
					SELECT_AUDIO_DMA_INPUT_BUFFER_A();
					DEBUGMSG(ZONE_FUNCTION,(TEXT("2\n")));
				}
				
				//----- 6. Retrieve the next chunk of recorded data from the non-playing buffer -----
				InputTransferred = TransferInputBuffers(m_InputDMAStatus);			
			}

			//----- 7. Handle any interrupts on the output source -----
			//		   NOTE: The InterruptDone() call below automatically clears the interrupt.
			if ((dmaInterruptSource & DMA_CH_OUT) != 0)
			{	
				//----- Determine which buffer just completed the DMA transfer -----
				if(m_OutputDMAStatus & DMA_BIU)
				{
					m_OutputDMAStatus &= ~DMA_STRTB;						// Buffer A just completed...
					m_OutputDMAStatus |= DMA_DONEB;

					m_OutputDMAStatus &= ~DMA_BIU;							// Buffer B is in use
					
					while((g_pDMAregs->DSTAT2&0xfffff)==0);
					SELECT_AUDIO_DMA_OUTPUT_BUFFER_B();		// charlie. B => A
				}else
				{
					m_OutputDMAStatus &= ~DMA_STRTA;						// Buffer B just completed...
					m_OutputDMAStatus |= DMA_DONEA;

					m_OutputDMAStatus |= DMA_BIU;							// Buffer A is in use
					
					while((g_pDMAregs->DSTAT2&0xfffff)==0);
					SELECT_AUDIO_DMA_OUTPUT_BUFFER_A();		// charlie. B => A
				}
				//----- 9. Fill the non-playing buffer with the next chunk of audio data to play -----
				OutputTransferred = TransferOutputBuffers(m_OutputDMAStatus);			
			}

		}
		__except(EXCEPTION_EXECUTE_HANDLER) 
		{
			DEBUGMSG(ZONE_ERROR, (TEXT("WAVDEV2.DLL:InterruptThread() - EXCEPTION: %d"), GetExceptionCode()));
		}

		//----- 10. Give up the lock ----- 
		Unlock();
	}  
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		AudioMute()

Description:	Mutes/unmutes the specified audio channel.

Notes:			If both audio channels are MUTED, then the chip 
				is powered down to conserve battery life.  
				Alternatively, if either audio channel is unMUTED, 
				the chip is powered up.

Returns:		Boolean indicating success
-------------------------------------------------------------------*/
BOOL HardwareContext::AudioMute(DWORD channel, BOOL bMute)
{
	static DWORD dwActiveChannel = 0;
	return(TRUE);
}


void CallInterruptThread(HardwareContext *pHWContext)
{
    pHWContext->InterruptThread();
}
