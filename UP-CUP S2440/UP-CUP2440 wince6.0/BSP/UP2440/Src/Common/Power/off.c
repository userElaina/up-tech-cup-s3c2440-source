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
// NOTE: stubs are being used - this isn't done

#include <windows.h>
#include <oal.h>
#include <s3c2440a.h>

extern void ConfigStopGPIO(void);
//------------------------------------------------------------------------------
//
// Function:     OEMPowerOff
//
// Description:  Called when the system is to transition to it's lowest
//               power mode (off)
//

void OEMPowerOff()
{
    static UINT32 saveArea[51];
    S3C2440A_INTR_REG *pIntr = (S3C2440A_INTR_REG*)OALPAtoVA(S3C2440A_BASE_REG_PA_INTR, FALSE);
    S3C2440A_IOPORT_REG *pIOPort = (S3C2440A_IOPORT_REG*)OALPAtoVA(S3C2440A_BASE_REG_PA_IOPORT, FALSE);
    S3C2440A_LCD_REG *pLCD = (S3C2440A_LCD_REG*)OALPAtoVA(S3C2440A_BASE_REG_PA_LCD, FALSE);
	
	OALMSG(1,(L"+OEMPowerOff"));

    // Then save system registers
    saveArea[0]  = INPORT32(&pIOPort->GPACON);
    saveArea[1]  = INPORT32(&pIOPort->GPADAT);
    saveArea[2]  = INPORT32(&pIOPort->GPBCON);
    saveArea[3]  = INPORT32(&pIOPort->GPBDAT);
    saveArea[4]  = INPORT32(&pIOPort->GPBUP);
    saveArea[5]  = INPORT32(&pIOPort->GPCCON);
    saveArea[6]  = INPORT32(&pIOPort->GPCDAT);
    saveArea[7]  = INPORT32(&pIOPort->GPCUP);
    saveArea[8]  = INPORT32(&pIOPort->GPDCON);
    saveArea[9]  = INPORT32(&pIOPort->GPDDAT);
    saveArea[10] = INPORT32(&pIOPort->GPDUP);
    saveArea[11] = INPORT32(&pIOPort->GPECON);
    saveArea[12] = INPORT32(&pIOPort->GPEDAT);
    saveArea[13] = INPORT32(&pIOPort->GPEUP);
    saveArea[14] = INPORT32(&pIOPort->GPFCON);
    saveArea[15] = INPORT32(&pIOPort->GPFDAT);
    saveArea[16] = INPORT32(&pIOPort->GPFUP);
    saveArea[17] = INPORT32(&pIOPort->GPGCON);
    saveArea[18] = INPORT32(&pIOPort->GPGDAT);
    saveArea[19] = INPORT32(&pIOPort->GPGUP);
    saveArea[20] = INPORT32(&pIOPort->GPHCON);
    saveArea[21] = INPORT32(&pIOPort->GPHDAT);
    saveArea[22] = INPORT32(&pIOPort->GPHUP);

    saveArea[23] = INPORT32(&pIOPort->MISCCR);
    saveArea[24] = INPORT32(&pIOPort->DCLKCON);
    saveArea[25] = INPORT32(&pIOPort->EXTINT0);
    saveArea[26] = INPORT32(&pIOPort->EXTINT1);
    saveArea[27] = INPORT32(&pIOPort->EXTINT2);
    saveArea[28] = INPORT32(&pIOPort->EINTFLT0);
    saveArea[29] = INPORT32(&pIOPort->EINTFLT1);
    saveArea[30] = INPORT32(&pIOPort->EINTFLT2);
    saveArea[31] = INPORT32(&pIOPort->EINTFLT3);
    saveArea[32] = INPORT32(&pIOPort->EINTMASK);

    saveArea[33] = INPORT32(&pIntr->INTMOD);
    saveArea[34] = INPORT32(&pIntr->INTMSK);
    saveArea[35] = INPORT32(&pIntr->INTSUBMSK);

    saveArea[36] = INPORT32(&pLCD->TCONSEL);
    saveArea[37] = INPORT32(&pLCD->LCDINTMSK);
    saveArea[38] = INPORT32(&pLCD->TPAL);
    saveArea[39] = INPORT32(&pLCD->DITHMODE);
    saveArea[40] = INPORT32(&pLCD->BLUELUT);
    saveArea[41] = INPORT32(&pLCD->GREENLUT);
    saveArea[42] = INPORT32(&pLCD->REDLUT);
    saveArea[43] = INPORT32(&pLCD->LCDSADDR3);
    saveArea[44] = INPORT32(&pLCD->LCDSADDR2);
    saveArea[45] = INPORT32(&pLCD->LCDSADDR1);
    saveArea[46] = INPORT32(&pLCD->LCDCON5);
    saveArea[47] = INPORT32(&pLCD->LCDCON4);
    saveArea[48] = INPORT32(&pLCD->LCDCON3);
    saveArea[49] = INPORT32(&pLCD->LCDCON2);
    saveArea[50] = INPORT32(&pLCD->LCDCON1);

    pLCD->LCDCON1   = 0;
    pLCD->LCDCON2   = 0;
    pLCD->LCDCON3   = 0;
    pLCD->LCDCON4   = 0;
    pLCD->LCDCON5   = 0;
    pLCD->LCDSADDR1 = 0;
    pLCD->LCDSADDR2 = 0;
    pLCD->LCDSADDR3 = 0;
    pLCD->TCONSEL    = 0;
    pLCD->TPAL      = 0;

	ConfigStopGPIO();

	// Do platform power off specific actions
    BSPPowerOff();
    
    // Go to power off mode
    OALCPUPowerOff();

    
    /* Recover Process, Load CPU Regs       */
    OUTPORT32(&pIOPort->GPACON,   saveArea[0]);
    OUTPORT32(&pIOPort->GPADAT,   saveArea[1]);
    OUTPORT32(&pIOPort->GPBCON,   saveArea[2]);
    OUTPORT32(&pIOPort->GPBDAT,   saveArea[3]);
    OUTPORT32(&pIOPort->GPBUP,    saveArea[4]);
    OUTPORT32(&pIOPort->GPCCON,   saveArea[5]);
    OUTPORT32(&pIOPort->GPCDAT,   saveArea[6]);
    OUTPORT32(&pIOPort->GPCUP,    saveArea[7]);
    OUTPORT32(&pIOPort->GPDCON,   saveArea[8]);
    OUTPORT32(&pIOPort->GPDDAT,   saveArea[9]);
    OUTPORT32(&pIOPort->GPDUP,    saveArea[10]);
    OUTPORT32(&pIOPort->GPECON,   saveArea[11]);
    OUTPORT32(&pIOPort->GPEDAT,   saveArea[12]);
    OUTPORT32(&pIOPort->GPEUP,    saveArea[13]);
    OUTPORT32(&pIOPort->GPFCON,   saveArea[14]);
    OUTPORT32(&pIOPort->GPFDAT,   saveArea[15]);
    OUTPORT32(&pIOPort->GPFUP,    saveArea[16]);
    OUTPORT32(&pIOPort->GPGCON,   saveArea[17]);
    OUTPORT32(&pIOPort->GPGDAT,   saveArea[18]);
    OUTPORT32(&pIOPort->GPGUP,    saveArea[19]);
    OUTPORT32(&pIOPort->GPHCON,   saveArea[20]);
    OUTPORT32(&pIOPort->GPHDAT,   saveArea[21]);
    OUTPORT32(&pIOPort->GPHUP,    saveArea[22]);
                                
    OUTPORT32(&pIOPort->MISCCR,   saveArea[23]);
    OUTPORT32(&pIOPort->DCLKCON,   saveArea[24]);
    OUTPORT32(&pIOPort->EXTINT0,  saveArea[25]);
    OUTPORT32(&pIOPort->EXTINT1,  saveArea[26]);
    OUTPORT32(&pIOPort->EXTINT2,  saveArea[27]);
    OUTPORT32(&pIOPort->EINTFLT0, saveArea[28]);
    OUTPORT32(&pIOPort->EINTFLT1, saveArea[29]);
    OUTPORT32(&pIOPort->EINTFLT2, saveArea[30]);
    OUTPORT32(&pIOPort->EINTFLT3, saveArea[31]);
    OUTPORT32(&pIOPort->EINTMASK, saveArea[32]);

    OUTPORT32(&pIntr->INTMOD,     saveArea[33]);
    OUTPORT32(&pIntr->INTMSK,     saveArea[34]); 
    OUTPORT32(&pIntr->INTSUBMSK,  saveArea[35]); 

    OUTPORT32(&pLCD->TCONSEL,   saveArea[36]);
    OUTPORT32(&pLCD->LCDINTMSK,   saveArea[37]);
    OUTPORT32(&pLCD->TPAL,    saveArea[38]);
    OUTPORT32(&pLCD->DITHMODE,   saveArea[39]);
    OUTPORT32(&pLCD->BLUELUT,   saveArea[40]);
    OUTPORT32(&pLCD->GREENLUT,  saveArea[41]);
    OUTPORT32(&pLCD->REDLUT,  saveArea[42]);
    OUTPORT32(&pLCD->LCDSADDR3,  saveArea[43]);
    OUTPORT32(&pLCD->LCDSADDR2, saveArea[44]);
    OUTPORT32(&pLCD->LCDSADDR1, saveArea[45]);
    OUTPORT32(&pLCD->LCDCON5, saveArea[46]);
    OUTPORT32(&pLCD->LCDCON4, saveArea[47]);
    OUTPORT32(&pLCD->LCDCON3, saveArea[48]);
    OUTPORT32(&pLCD->LCDCON2,     saveArea[49]);
    OUTPORT32(&pLCD->LCDCON1,     saveArea[50]); 
                                   
    /* Interrupt Clear                      */
    OUTPORT32(&pIOPort->EINTPEND, INPORT32(&pIOPort->EINTPEND));
    OUTPORT32(&pIntr->SUBSRCPND, INPORT32(&pIntr->SUBSRCPND));
    OUTPORT32(&pIntr->SRCPND, INPORT32(&pIntr->SRCPND));
    OUTPORT32(&pIntr->INTPND, INPORT32(&pIntr->INTPND));
	OUTPORT32(&pLCD->LCDSRCPND, INPORT32(&pLCD->LCDSRCPND));
	OUTPORT32(&pLCD->LCDINTPND, INPORT32(&pLCD->LCDINTPND));
	

    // Do platform dependent power on actions
    BSPPowerOn();
    
    OALMSG(1,(L"-OEMPowerOff"));
}

//------------------------------------------------------------------------------
//
// Function:     OALIoCtlHalPresuspend
//
// Description:  
//

BOOL OALIoCtlHalPresuspend(
    UINT32 code, VOID* pInpBuffer, UINT32 inpSize, VOID* pOutBuffer, 
    UINT32 outSize, UINT32 *pOutSize
) {
    return TRUE;
}

#if 0
//------------------------------------------------------------------------------
//
// Function:     OALIoCtlHalEnableWake
//
// Description:  
//

BOOL OALIoCtlHalEnableWake(
    UINT32 code, VOID* pInpBuffer, UINT32 inpSize, VOID* pOutBuffer, 
    UINT32 outSize, UINT32 *pOutSize
) {
    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function:     OALIoCtlHalDisableWake
//
// Description:  
//

BOOL OALIoCtlHalDisableWake(
    UINT32 code, VOID* pInpBuffer, UINT32 inpSize, VOID* pOutBuffer, 
    UINT32 outSize, UINT32 *pOutSize
) {
    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function:     OALIoCtlHalDisableWake
//
// Description:  
//

BOOL OALIoCtlHalGetWakeSource(
    UINT32 code, VOID* pInpBuffer, UINT32 inpSize, VOID* pOutBuffer, 
    UINT32 outSize, UINT32 *pOutSize
) {
    return TRUE;
}
#endif //0.

//------------------------------------------------------------------------------

