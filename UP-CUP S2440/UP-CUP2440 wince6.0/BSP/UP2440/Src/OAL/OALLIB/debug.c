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
//  File:  debug.c            
//
//  This module is provides the interface to the serial port.
//
#include <bsp.h>
#include <nkintr.h>
 
//------------------------------------------------------------------------------
// Defines 
 
//------------------------------------------------------------------------------
// Externs
 
//------------------------------------------------------------------------------
// Global Variables 
 
//------------------------------------------------------------------------------
// Local Variables 
static S3C2440A_UART_REG *g_pUARTReg;

//------------------------------------------------------------------------------
// Local Functions 


//------------------------------------------------------------------------------
//
//  Function: OEMInitDebugSerial
//
//  Initializes the debug serial port
//
VOID OEMInitDebugSerial() 
{
//#if(BSP_UARTn == BSP_nouse_UART)
	S3C2440A_IOPORT_REG *pIOPortReg;
//#ifdef DEBUG
	UINT32 logMask;


	// At this moment we must suppress logging.
	//
	logMask = dpCurSettings.ulZoneMask;
	dpCurSettings.ulZoneMask = 0;
//#endif

	// Configure port H for UART
	//
	pIOPortReg = (S3C2440A_IOPORT_REG*)OALPAtoVA(S3C2440A_BASE_REG_PA_IOPORT, FALSE);


#if(BSP_UARTn == BSP_use_UART0)
	// GPH2 and GHP3 are UART0 Tx and Rx, respectively.
	//
	CLRREG32(&pIOPortReg->GPHCON, (3 << 4)|(3 << 6));
	SETREG32(&pIOPortReg->GPHCON, (2 << 4)|(2 << 6));

	// Disable Pull-up on TXD0 and RXD0.
	//
	SETREG32(&pIOPortReg->GPHUP, (1 << 2)|(1 << 3));

	// UART1 (TXD0 & RXD0) used for debug serial.
	//
	g_pUARTReg = (S3C2440A_UART_REG *)OALPAtoVA(S3C2440A_BASE_REG_PA_UART0, FALSE);

	// Configure the UART.
	//
	OUTREG32(&g_pUARTReg->UFCON,  BSP_UART0_UFCON);
	OUTREG32(&g_pUARTReg->UMCON,  BSP_UART0_UMCON);
	OUTREG32(&g_pUARTReg->ULCON,  BSP_UART0_ULCON);
	OUTREG32(&g_pUARTReg->UCON,   BSP_UART0_UCON);
	OUTREG32(&g_pUARTReg->UBRDIV, BSP_UART0_UBRDIV);

#endif

#if(BSP_UARTn == BSP_use_UART1)
	// GPH2 and GHP3 are UART1 Tx and Rx, respectively.
	//
	CLRREG32(&pIOPortReg->GPHCON, (3 << 8)|(3 << 10));
	SETREG32(&pIOPortReg->GPHCON, (2 << 8)|(2 << 10));

	// Disable pull-up on TXD1 and RXD1.
	//
	SETREG32(&pIOPortReg->GPHUP, (1 << 4)|(1 << 5));

	// UART1 (TXD1 & RXD1) used for debug serial.
	//
	g_pUARTReg = (S3C2440A_UART_REG *)OALPAtoVA(S3C2440A_BASE_REG_PA_UART1, FALSE);

	// Configure the UART.
	//
	OUTREG32(&g_pUARTReg->UFCON,  BSP_UART1_UFCON);
	OUTREG32(&g_pUARTReg->UMCON,  BSP_UART1_UMCON);
	OUTREG32(&g_pUARTReg->ULCON,  BSP_UART1_ULCON);
	OUTREG32(&g_pUARTReg->UCON,   BSP_UART1_UCON);
	OUTREG32(&g_pUARTReg->UBRDIV, BSP_UART1_UBRDIV);

#endif

//#ifdef DEBUG
	// Restore the logging mask.
	//
	dpCurSettings.ulZoneMask = logMask;
//#endif
//#endif
}


//------------------------------------------------------------------------------
//
//  Function: OEMWriteDebugByte
//
//  Transmits a character out the debug serial port.
//
VOID OEMWriteDebugByte(UINT8 ch) 
{
#if(BSP_UARTn != BSP_nouse_UART)
	// Wait for transmit buffer to be empty
	while ((INREG32(&g_pUARTReg->UTRSTAT) & 0x02) == 0);

	// Send character
	OUTREG32(&g_pUARTReg->UTXH, ch);
#endif
}


//------------------------------------------------------------------------------
//
//  Function: OEMReadDebugByte
//
//  Reads a byte from the debug serial port. Does not wait for a character. 
//  If a character is not available function returns "OEM_DEBUG_READ_NODATA".
//

int OEMReadDebugByte() 
{
#if(BSP_UARTn != BSP_nouse_UART)
	UINT32 status, ch;

	status = INREG32(&g_pUARTReg->UTRSTAT);
	if ((status & 0x01) != 0) {
		ch = INREG32(&g_pUARTReg->URXH);
	// if ((status & UART_LINESTAT_RF) != 0) ch = OEM_DEBUG_COM_ERROR;
	} else {
		ch = OEM_DEBUG_READ_NODATA;
	}
	return (int)ch;
#else
	return 0;
#endif
}


/*
    @func   void | OEMWriteDebugLED | Writes specified pattern to debug LEDs 1-4.
    @rdesc  None.
    @comm    
    @xref   
*/
void OEMWriteDebugLED(UINT16 Index, DWORD Pattern)
{
	volatile S3C2440A_IOPORT_REG *s2440IOP = (S3C2440A_IOPORT_REG *)OALPAtoVA(S3C2440A_BASE_REG_PA_IOPORT, FALSE);

	// The S24x0X01 Eval platform supports 4 LEDs..
	//
	s2440IOP->GPBDAT=(s2440IOP->GPBDAT & 0xf) | ((Pattern & 0xf)<<5);
}

//------------------------------------------------------------------------------
