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
//  File:  intr.h
//
//  This file contains UP2440 board specific interrupt code.
//
#include <bsp.h>

//------------------------------------------------------------------------------
//
//  Function:  BSPIntrInit
//
BOOL BSPIntrInit()
{
	S3C2440A_IOPORT_REG *pOalPortRegs;
	S3C2440A_INTR_REG *pIntrRegs; 
	ULONG value;
	UINT32 sysIntr, irq;

	OALMSG(OAL_INTR&&OAL_FUNC, (L"+BSPIntrInit\r\n"));

	// Then get virtual address for IO port
	pOalPortRegs = OALPAtoVA(S3C2440A_BASE_REG_PA_IOPORT, FALSE);

	pIntrRegs = OALPAtoVA(S3C2440A_BASE_REG_PA_INTR, FALSE);

	// Set GPF7 as EINT7 for DM9000
	value = INREG32(&pOalPortRegs->GPFCON);
	//OUTREG32(&pOalPortRegs->GPFCON, (value & ~(3 << 14))|(2 << 14));
	OUTREG32(&pOalPortRegs->GPFCON, (value & ~(3 << 4))|(2 << 4));
	//OUTREG32(&pOalPortRegs->GPGCON, (value & ~(3 << 2))|(2 << 2));

	// Disable pulldown (for EINT7)
	value = INREG32(&pOalPortRegs->GPFUP);
	OUTREG32(&pOalPortRegs->GPFUP, value | (1 << 2));

	// High level interrupt(for EINT7)
	value = INREG32(&pOalPortRegs->EXTINT0);
	OUTREG32(&pOalPortRegs->EXTINT0, (value & ~(0x7 << 8))|(0x4 << 8));

	// Add static mapping for Built-In OHCI 
	OALIntrStaticTranslate(SYSINTR_OHCI, IRQ_USBH);
	OALIntrStaticTranslate(SYSINTR_ETH, IRQ_EINT2);

	// OALIntrStaticTranslate(SYSINTR_USBFN, IRQ_USBFN);

	value = IRQ_EINT2;			//HJ_add 20100112
	OALIntrEnableIrqs(1, &value);		//HJ_add 20100112

	OALMSG(OAL_INTR&&OAL_FUNC, (L"-BSPIntrInit(rc = 1)\r\n"));
	return TRUE;
}

//------------------------------------------------------------------------------

BOOL BSPIntrRequestIrqs(DEVICE_LOCATION *pDevLoc, UINT32 *pCount, UINT32 *pIrqs)
{
	BOOL rc = FALSE;

	OALMSG(OAL_INTR&&OAL_FUNC, (
		L"+BSPIntrRequestIrq(0x%08x->%d/%d/0x%08x/%d, 0x%08x, 0x%08x)\r\n",
		pDevLoc, pDevLoc->IfcType, pDevLoc->BusNumber, pDevLoc->LogicalLoc,
		pDevLoc->Pin, pCount, pIrqs
	));

	if (pIrqs == NULL || pCount == NULL || *pCount < 1) goto cleanUp;

	switch (pDevLoc->IfcType)
	{
		case Internal:
			switch ((ULONG)pDevLoc->LogicalLoc)
			{
				case BSP_BASE_REG_PA_DM9000_IOBASE:
					pIrqs[0] = IRQ_EINT7;
					*pCount = 1;
					rc = TRUE;
					break;
			}
			break;
	}

cleanUp:
	OALMSG(OAL_INTR&&OAL_FUNC, (L"-BSPIntrRequestIrq(rc = %d)\r\n", rc));
	return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  BSPIntrEnableIrq
//
//  This function is called from OALIntrEnableIrq to enable interrupt on
//  secondary interrupt controller.
//
UINT32 BSPIntrEnableIrq(UINT32 irq)
{
	OALMSG(OAL_INTR&&OAL_VERBOSE, (L"+BSPIntrEnableIrq(%d)\r\n", irq));
	OALMSG(OAL_INTR&&OAL_VERBOSE, (L"-BSPIntrEnableIrq(irq = %d)\r\n", irq));
	return irq;
}

//------------------------------------------------------------------------------
//
//  Function:  BSPIntrDisableIrq
//
//  This function is called from OALIntrDisableIrq to disable interrupt on
//  secondary interrupt controller.
//
UINT32 BSPIntrDisableIrq(UINT32 irq)
{
	OALMSG(OAL_INTR&&OAL_VERBOSE, (L"+BSPIntrDisableIrq(%d)\r\n", irq));
	OALMSG(OAL_INTR&&OAL_VERBOSE, (L"-BSPIntrDisableIrq(irq = %d\r\n", irq));
	return irq;
}


//------------------------------------------------------------------------------
//
//  Function:  BSPIntrDoneIrq
//
//  This function is called from OALIntrDoneIrq to finish interrupt on
//  secondary interrupt controller.
//
UINT32 BSPIntrDoneIrq(UINT32 irq)
{
	OALMSG(OAL_INTR&&OAL_VERBOSE, (L"+BSPIntrDoneIrq(%d)\r\n", irq));
	OALMSG(OAL_INTR&&OAL_VERBOSE, (L"-BSPIntrDoneIrq(irq = %d)\r\n", irq));
	return irq;
}


//------------------------------------------------------------------------------
//
//  Function:  BSPIntrActiveIrq
//
//  This function is called from interrupt handler to give BSP chance to 
//  translate IRQ in case of secondary interrupt controller.
//
UINT32 BSPIntrActiveIrq(UINT32 irq)
{
	OALMSG(OAL_INTR&&OAL_VERBOSE, (L"+BSPIntrActiveIrq(%d)\r\n", irq));
	OALMSG(OAL_INTR&&OAL_VERBOSE, (L"-BSPIntrActiveIrq(%d)\r\n", irq));
	return irq;
}

//------------------------------------------------------------------------------

