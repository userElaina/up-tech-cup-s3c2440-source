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
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
#include <windows.h>
#include <nkintr.h>
#include <ddkreg.h>
#include <S3C2440A.h>
#include "sc2440pdd.h"


#define UDC_REG_PRIORITY_VAL _T("Priority256")

enum EP0_STATE {
    EP0_STATE_IDLE = 0,
    EP0_STATE_IN_DATA_PHASE,
    EP0_STATE_OUT_DATA_PHASE
};

#define IN_TRANSFER  1
#define OUT_TRANSFER 2




#define EP0_STALL_BITS (EP0_SEND_STALL | EP0_SENT_STALL)


typedef struct EP_STATUS {
    DWORD                   dwEndpointNumber;
    DWORD                   dwDirectionAssigned;
    DWORD                   dwPacketSizeAssigned;
    BOOL                    fInitialized;
    DWORD                   dwEndpointType;
    PSTransfer              pTransfer;
    CRITICAL_SECTION        cs;
} *PEP_STATUS;

#define LOCK_ENDPOINT(peps)     EnterCriticalSection(&peps->cs)
#define UNLOCK_ENDPOINT(peps)   LeaveCriticalSection(&peps->cs)


#define EP_0_PACKET_SIZE    0x8    // Could also be 16 but they recommend 8  

#define ENDPOINT_COUNT  5
#define EP_VALID(x)     ((x) < ENDPOINT_COUNT)


#define DEFAULT_PRIORITY 100

typedef struct CTRL_PDD_CONTEXT {
    PVOID             pvMddContext;
    DWORD             dwSig;
    HANDLE            hIST;
    HANDLE            hevInterrupt;
    BOOL              fRunning;
    CRITICAL_SECTION  csIndexedRegisterAccess;
    BOOL              fSpeedReported;
    BOOL              fRestartIST;
    BOOL              fExitIST;
    BOOL              attachedState;
    BOOL              sendDataEnd;
    BOOL              isattachedusbcable;
    EP0_STATE         Ep0State;
    
    // registry 
    DWORD             dwIOBase;
    DWORD             dwSysIntr;
    DWORD             dwIrq;
    DWORD             dwIOLen;
    DWORD             dwISTPriority;

    USB_DEVICE_REQUEST udr;
    EP_STATUS         rgEpStatus[ENDPOINT_COUNT];
    
    PFN_UFN_MDD_NOTIFY      pfnNotify;
    HANDLE                  hBusAccess;
    CEDEVICE_POWER_STATE    cpsCurrent;
} *PCTRLR_PDD_CONTEXT;

#define SC2440_SIG '2440' // "SC2440" signature

#define IS_VALID_SC2440_CONTEXT(ptr) \
    ( (ptr != NULL) && (ptr->dwSig == SC2440_SIG) )


#ifdef DEBUG

#define ZONE_POWER           DEBUGZONE(8)

UFN_GENERATE_DPCURSETTINGS(UFN_DEFAULT_DPCURSETTINGS_NAME, 
    _T("Power"), _T(""), _T(""), _T(""), 
    DBG_ERROR | DBG_INIT); 
// Caution: Turning on more debug zones can cause STALLs due
// to corrupted setup packets.

// Validate the context.
static
VOID
ValidateContext(
                PCTRLR_PDD_CONTEXT pContext
                )
{
    PREFAST_DEBUGCHK(pContext);
    DEBUGCHK(pContext->dwSig == SC2440_SIG);
    DEBUGCHK(!pContext->hevInterrupt || pContext->hIST);
    DEBUGCHK(VALID_DX(pContext->cpsCurrent));
    DEBUGCHK(pContext->pfnNotify);
}

#else
#define ValidateContext(ptr)
#endif

volatile BYTE *g_pUDCBase;

static BYTE USBClassInfo;
#define USB_RNDIS  0
#define USB_Serial 1
#define USB_MSF    2

#define CTRLR_BASE_REG_ADDR(offset) ((volatile ULONG*) ( (g_pUDCBase) + (offset)))

#define DRIVER_USB_KEY TEXT("Drivers\\USB\\FunctionDrivers")
#define DRIVER_USB_VALUE TEXT("DefaultClientDriver")

// Read a register.
inline
BYTE
ReadReg(
        PCTRLR_PDD_CONTEXT pContext,
        DWORD dwOffset
        )
{
    DEBUGCHK(IS_VALID_SC2440_CONTEXT(pContext));

    volatile ULONG *pbReg = CTRLR_BASE_REG_ADDR(dwOffset);
    BYTE bValue = (BYTE) *pbReg;
    return bValue;
}


// Write a register.
inline
VOID
WriteReg(
         PCTRLR_PDD_CONTEXT pContext,
         DWORD dwOffset,
         BYTE bValue
         )
{
    DEBUGCHK(IS_VALID_SC2440_CONTEXT(pContext));

    volatile ULONG *pbReg = CTRLR_BASE_REG_ADDR(dwOffset);
    *pbReg = (ULONG) bValue;
}



// Calling with dwMask = 0 and SET reads and writes the contents unchanged.
inline
BYTE
SetClearReg(
            PCTRLR_PDD_CONTEXT pContext,
            DWORD dwOffset,
            BYTE dwMask,
            BOOL  bSet
            )
{
    DEBUGCHK(IS_VALID_SC2440_CONTEXT(pContext));

    volatile ULONG *pbReg = CTRLR_BASE_REG_ADDR(dwOffset);
    BYTE bValue = (BYTE) *pbReg;

    if (bSet) {
        bValue |= dwMask;
    }
    else {
        bValue &= ~dwMask;
    }

    *pbReg = bValue;

    return bValue;
}
// Calling with dwMask = 0 and SET reads and writes the contents unchanged.
inline
BYTE
SetClearIndexedReg(
                   PCTRLR_PDD_CONTEXT pContext,
                   DWORD dwEndpoint,
                   DWORD dwOffset,
                   BYTE dwMask,
                   BOOL  bSet
                   )
{
    DEBUGCHK(IS_VALID_SC2440_CONTEXT(pContext));
    BYTE bValue = 0;

    EnterCriticalSection(&pContext->csIndexedRegisterAccess);
    // Write the EP number to the index reg
    WriteReg(pContext, IDXADDR_REG_OFFSET, (BYTE) dwEndpoint);    
    // Now Write the Register associated with this Endpoint for a given offset
    bValue = SetClearReg(pContext, dwOffset, dwMask, bSet);
    LeaveCriticalSection(&pContext->csIndexedRegisterAccess);

    return bValue;
}
#define SET   TRUE
#define CLEAR FALSE



// Read an indexed register.
inline
BYTE
ReadIndexedReg(
               PCTRLR_PDD_CONTEXT pContext,
               DWORD dwEndpoint,
               DWORD regOffset
               )
{
    DEBUGCHK(IS_VALID_SC2440_CONTEXT(pContext));

    EnterCriticalSection(&pContext->csIndexedRegisterAccess);
    // Write the EP number to the index reg
    WriteReg(pContext, IDXADDR_REG_OFFSET, (BYTE) dwEndpoint);
    // Now Read the Register associated with this Endpoint for a given offset
    BYTE bValue = ReadReg(pContext, regOffset);
    LeaveCriticalSection(&pContext->csIndexedRegisterAccess);
    return bValue;
}


// Write an indexed register.
inline
VOID
WriteIndexedReg(
                PCTRLR_PDD_CONTEXT pContext,
                DWORD dwEndpoint,
                DWORD regOffset,
                BYTE  bValue
                )
{  
    DEBUGCHK(IS_VALID_SC2440_CONTEXT(pContext));

    EnterCriticalSection(&pContext->csIndexedRegisterAccess);
    // Write the EP number to the index reg
    WriteReg(pContext, IDXADDR_REG_OFFSET, (BYTE) dwEndpoint);    
    // Now Write the Register associated with this Endpoint for a given offset
    WriteReg(pContext, regOffset, bValue);
    LeaveCriticalSection(&pContext->csIndexedRegisterAccess);
}

/*++

Routine Description:

Return the data register of an endpoint.

Arguments:

dwEndpoint - the target endpoint

Return Value:

The data register of the target endpoint.

--*/
static
volatile ULONG*
_GetDataRegister(
                 DWORD        dwEndpoint    
                 )
{
    volatile ULONG *pulDataReg = NULL;

    //
    // find the data register (non-uniform offset)
    //
    switch (dwEndpoint) {
        case  0: pulDataReg = CTRLR_BASE_REG_ADDR(EP0_FIFO_REG_OFFSET);  break;
        case  1: pulDataReg = CTRLR_BASE_REG_ADDR(EP1_FIFO_REG_OFFSET);  break;
        case  2: pulDataReg = CTRLR_BASE_REG_ADDR(EP2_FIFO_REG_OFFSET);  break;
        case  3: pulDataReg = CTRLR_BASE_REG_ADDR(EP3_FIFO_REG_OFFSET);  break;
        case  4: pulDataReg = CTRLR_BASE_REG_ADDR(EP4_FIFO_REG_OFFSET);  break;
        default:
            DEBUGCHK(FALSE);
            break;
    }

    return pulDataReg;
} // _GetDataRegister


// Retrieve the endpoint status structure.
inline
static
PEP_STATUS
GetEpStatus(
            PCTRLR_PDD_CONTEXT pContext,
            DWORD dwEndpoint
            )
{
    ValidateContext(pContext);
    DEBUGCHK(EP_VALID(dwEndpoint));

    PEP_STATUS peps = &pContext->rgEpStatus[dwEndpoint];

    return peps;
}


// Return the irq bit for this endpoint.
inline
static
BYTE
EpToIrqStatBit(
               DWORD dwEndpoint
               )
{
    DEBUGCHK(EP_VALID(dwEndpoint));

    return (1 << (BYTE)dwEndpoint);
}

/*++

Routine Description:

Enable the interrupt of an endpoint.

Arguments:

dwEndpoint - the target endpoint

Return Value:

None.

--*/
static
VOID
EnableDisableEndpointInterrupt(
                        PCTRLR_PDD_CONTEXT  pContext,
                        DWORD               dwEndpoint,
                        BOOL                fEnable
                        )
{
    // TODO: Make new cs or rename this one
    EnterCriticalSection(&pContext->csIndexedRegisterAccess);
    
    // Disable the Endpoint Interrupt
    BYTE bEpIntReg = ReadReg(pContext, EP_INT_EN_REG_OFFSET);
    BYTE bIrqEnBit = EpToIrqStatBit(dwEndpoint);

    if (fEnable) {
        bEpIntReg |= bIrqEnBit;
    }
    else {
        bEpIntReg &= ~bIrqEnBit;
    }
    
    WriteReg(pContext, EP_INT_EN_REG_OFFSET, bEpIntReg);

    LeaveCriticalSection(&pContext->csIndexedRegisterAccess);    
}

static
inline
VOID
EnableEndpointInterrupt(
    PCTRLR_PDD_CONTEXT  pContext,
    DWORD               dwEndpoint
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();
    
    EnableDisableEndpointInterrupt(pContext, dwEndpoint, TRUE);

    FUNCTION_LEAVE_MSG();
}

static
inline
VOID
DisableEndpointInterrupt(
    PCTRLR_PDD_CONTEXT  pContext,
    DWORD               dwEndpoint
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();
    
    EnableDisableEndpointInterrupt(pContext, dwEndpoint, FALSE);

    FUNCTION_LEAVE_MSG();
}

/*++

Routine Description:

Clear the interrupt status register index of an endpoint.

Arguments:

dwEndpoint - the target endpoint

Return Value:

None.

--*/
static
VOID
ClearEndpointInterrupt(
                       PCTRLR_PDD_CONTEXT pContext,
                       DWORD        dwEndpoint
                       )
{    
    SETFNAME();
    FUNCTION_ENTER_MSG();
    
    // Clear the Endpoint Interrupt
    BYTE bIntBit = EpToIrqStatBit(dwEndpoint);
    WriteReg(pContext, EP_INT_REG_OFFSET, bIntBit);

    FUNCTION_LEAVE_MSG();
} // _ClearInterrupt


// Reset an endpoint
static
VOID
ResetEndpoint(
              PCTRLR_PDD_CONTEXT pContext,
              EP_STATUS *peps
              )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    ValidateContext(pContext);
    PREFAST_DEBUGCHK(peps);

    // Since Reset can be called before/after an Endpoint has been configured,
    // it is best to clear all IN and OUT bits associated with endpoint. 
    DWORD dwEndpoint = peps->dwEndpointNumber;
    if(dwEndpoint == 0 ) {
        // Clear all EP0 Status bits
        WriteIndexedReg(pContext, 0, IN_CSR1_REG_OFFSET,
            (SERVICED_OUT_PKY_RDY | SERVICED_SETUP_END));
    }
    else if(dwEndpoint < ENDPOINT_COUNT) {
        // Clear the desired Endpoint - Clear both the In & Out Status bits
        BYTE bRegIn;
        BYTE bRegOut;
        if(peps->fInitialized){
            PREFAST_DEBUGCHK(peps->fInitialized); // Give prefast a clue.
            // First Read the CSR2 Reg bit so that it can be restored
            bRegIn = ReadIndexedReg(pContext, dwEndpoint, IN_CSR2_REG_OFFSET);
            bRegOut = ReadIndexedReg(pContext, dwEndpoint, OUT_CSR2_REG_OFFSET);
        }
        // Clear the in register - Must set the Mode_in to IN
        WriteIndexedReg(pContext, dwEndpoint, IN_CSR2_REG_OFFSET,(SET_MODE_IN | IN_DMA_INT_DISABLE));
//        WriteIndexedReg(pContext, dwEndpoint, IN_CSR2_REG_OFFSET,(AUTO_MODE |SET_MODE_IN | IN_DMA_INT_DISABLE));
        WriteIndexedReg(pContext, dwEndpoint, IN_CSR1_REG_OFFSET, ( IN_CLR_DATA_TOGGLE));

        // Clear the Out register  - Must set the Mode_in to OUT
        WriteIndexedReg(pContext, dwEndpoint, IN_CSR2_REG_OFFSET, (IN_DMA_INT_DISABLE)); // mode_in bit = OUT 
//        WriteIndexedReg(pContext, dwEndpoint, IN_CSR2_REG_OFFSET, (AUTO_CLR | IN_DMA_INT_DISABLE)); // mode_in bit = OUT 
        WriteIndexedReg(pContext, dwEndpoint, OUT_CSR1_REG_OFFSET, (FLUSH_OUT_FIFO | OUT_CLR_DATA_TOGGLE));
        WriteIndexedReg(pContext, dwEndpoint,  OUT_CSR2_REG_OFFSET, OUT_DMA_INT_DISABLE);

        if(peps->fInitialized) {
            // Set the Mode_In, ISO and other Modality type bits back to the way it was - this allows 
            // ResetEndpoint to be called without having to re-init the endpoint.
            WriteIndexedReg(pContext, dwEndpoint, IN_CSR2_REG_OFFSET, bRegIn);
			WriteIndexedReg(pContext, dwEndpoint, OUT_CSR2_REG_OFFSET, bRegOut);	//jassi
        }

        // Clear and disable interrupt
        WriteReg(pContext, EP_INT_REG_OFFSET, EpToIrqStatBit(peps->dwEndpointNumber));
        DisableEndpointInterrupt(pContext, peps->dwEndpointNumber);
    }
    else {
        DEBUGCHK(FALSE);
    }

    FUNCTION_LEAVE_MSG();
}



// Reset the device and EP0.
static
VOID
ResetDevice(
            PCTRLR_PDD_CONTEXT pContext
            )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    DEBUGCHK(IS_VALID_SC2440_CONTEXT(pContext));

    // Disable Device  interrupts - write Zeros to Disable
    WriteReg(pContext, USB_INT_EN_REG_OFFSET, 0 );

    // Disable endpoint interrupts - write Zeros to Disable
    WriteReg(pContext, EP_INT_EN_REG_OFFSET, 0);

    // Clear any outstanding device & endpoint interrupts
    // USB Device Interrupt Status - Write a '1' to Clear 
    WriteReg(pContext, USB_INT_REG_OFFSET, 
        (USB_RESET_INTR | USB_RESUME_INTR | USB_SUSPEND_INTR));
    // End point Interrupt Status - Write a '1' to Clear
    WriteReg(pContext, EP_INT_REG_OFFSET, CLEAR_ALL_EP_INTRS);

    // Reset all endpoints
    for (DWORD dwEpIdx = 0; dwEpIdx < ENDPOINT_COUNT; ++dwEpIdx) {
        EP_STATUS *peps = GetEpStatus(pContext, dwEpIdx);
        ResetEndpoint(pContext, peps);
    }

    FUNCTION_LEAVE_MSG();
}


static
VOID
CompleteTransfer(
                 PCTRLR_PDD_CONTEXT pContext,
                 PEP_STATUS peps,
                 DWORD dwUsbError
                 )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    PSTransfer pTransfer = peps->pTransfer;
    peps->pTransfer = NULL;

    pTransfer->dwUsbError = dwUsbError;
    pContext->pfnNotify(pContext->pvMddContext, UFN_MSG_TRANSFER_COMPLETE, 
        (DWORD) pTransfer);

    FUNCTION_LEAVE_MSG();
}


#ifdef DEBUG
static
VOID
ValidateTransferDirection(
                          PCTRLR_PDD_CONTEXT pContext,
                          PEP_STATUS peps,
                          PSTransfer pTransfer
                          )
{
    DEBUGCHK(pContext);
    PREFAST_DEBUGCHK(peps);
    PREFAST_DEBUGCHK(pTransfer);

    if (peps->dwEndpointNumber != 0) {
        DEBUGCHK(peps->dwDirectionAssigned == pTransfer->dwFlags);
    }
}
#else
#define ValidateTransferDirection(ptr1, ptr2, ptr3)
#endif


// Read data from an endpoint.
static
BYTE
HandleRx(
         PCTRLR_PDD_CONTEXT       pContext,
         PEP_STATUS peps,
         PBOOL pfCompleted,
         PDWORD pdwStatus
         )
{
    BOOL fCompleted = FALSE;
    DWORD dwStatus = ERROR_GEN_FAILURE;
    DWORD dwEndpoint = peps->dwEndpointNumber;
    BYTE bRet = 0;

    SETFNAME();
    FUNCTION_ENTER_MSG();

    PSTransfer pTransfer = peps->pTransfer;

    pTransfer = peps->pTransfer;
    if (pTransfer) {
        DEBUGCHK(pTransfer->dwFlags == USB_OUT_TRANSFER);
        DEBUGCHK(pTransfer->dwUsbError == UFN_NOT_COMPLETE_ERROR);

        ValidateTransferDirection(pContext, peps, pTransfer);

        DEBUGCHK(peps->fInitialized);

        DWORD dwCurrentPermissions = GetCurrentPermissions();
//        SetProcPermissions(pTransfer->dwCallerPermissions);

        __try {
            volatile ULONG *pulFifoReg = _GetDataRegister(dwEndpoint);
            DEBUGCHK(pulFifoReg != NULL);
loop:

            PBYTE  pbBuffer =  (PBYTE)pTransfer->pvBuffer + pTransfer->cbTransferred;

            DWORD cbBuffer = pTransfer->cbBuffer - pTransfer->cbTransferred;
            DWORD cbFifo = ReadIndexedReg(pContext, dwEndpoint, OUT_FIFO_CNT1_REG_OFFSET);
            DEBUGCHK(cbFifo <= peps->dwPacketSizeAssigned);

            // Read from the FIFO
            const DWORD cbRead = min(cbFifo, cbBuffer);
            DWORD cbToRead = cbRead;
            while (cbToRead--) {
                *pbBuffer++ = (BYTE) *pulFifoReg;
            }

            DEBUGCHK(cbRead <= pTransfer->cbBuffer - pTransfer->cbTransferred);
            pTransfer->cbTransferred += cbRead;

            if ( (cbRead < peps->dwPacketSizeAssigned) ||
                 (pTransfer->cbTransferred == pTransfer->cbBuffer) ) {
                // Short packet or filled buffer. Complete transfer.
                fCompleted = TRUE;
                dwStatus = UFN_NO_ERROR;
            }

            if (dwEndpoint == 0) {
                bRet |= SERVICED_OUT_PKY_RDY;

                if (fCompleted) {
                    bRet |= DATA_END;
                    pContext->Ep0State = EP0_STATE_IDLE;
                }
            }
            else {
		 if((peps->dwEndpointType == USB_ENDPOINT_TYPE_BULK) && (!fCompleted)){
	  	    BYTE bEpIrqStat;
	  	    BYTE outCSR1 = ReadIndexedReg(pContext, dwEndpoint,OUT_CSR1_REG_OFFSET);

		    outCSR1 = outCSR1 & (OUT_SEND_STALL | OUT_SENT_STALL) & 
						~(OUT_CLR_DATA_TOGGLE) & (~OUT_PACKET_READY);	

	           WriteIndexedReg(pContext, dwEndpoint,OUT_CSR1_REG_OFFSET,outCSR1);			
	
		    outCSR1 = ReadIndexedReg(pContext,dwEndpoint,OUT_CSR1_REG_OFFSET);	
	  	    bEpIrqStat = ReadReg(pContext, EP_INT_REG_OFFSET);		


		if(outCSR1 & OUT_PACKET_READY)  {

			if (bEpIrqStat & (1<< dwEndpoint)) {
				WriteReg(pContext, EP_INT_REG_OFFSET,(1 << dwEndpoint));		
			}
			goto  loop;
			
		}	else {

				bRet |=OUT_PACKET_READY;
		    }		
		
		}

		                 // Clear Out Packet Ready - Allow next packet to come in.
                DEBUGCHK( (bRet & OUT_PACKET_READY) == 0);

            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            DEBUGMSG(ZONE_ERROR, (_T("%s Exception!\r\n"), pszFname));
            fCompleted = TRUE;
            dwStatus = UFN_CLIENT_BUFFER_ERROR;
        }

//        SetProcPermissions(dwCurrentPermissions);
    
        DEBUGMSG(ZONE_RECEIVE, (_T("%s Rx Ep%x BufferSize=%u,Xfrd=%u \r\n"), 
            pszFname, dwEndpoint, pTransfer->cbBuffer, pTransfer->cbTransferred));

        if (fCompleted) {
            DEBUGMSG(ZONE_RECEIVE, (_T("%s RxDone Ep%x BufferSize=%u, Xfrd=%u\r\n"), 
                pszFname, dwEndpoint,pTransfer->cbBuffer, pTransfer->cbTransferred));
        }
    }
    else {
            // Packet is not loaded properly into the FIFO, so leave OUT_PKT_RDY bit alone.
            bRet |=OUT_PACKET_READY;
        }

    *pfCompleted = fCompleted;
    *pdwStatus = dwStatus;
    FUNCTION_LEAVE_MSG();

    return bRet;
}


// Write data to an endpoint.
static 
BYTE
HandleTx(
         PCTRLR_PDD_CONTEXT       pContext,
         PEP_STATUS peps,
         BOOL fEnableInterrupts
         )
{
    SETFNAME();
    DEBUGCHK(pContext);
    PREFAST_DEBUGCHK(peps);

    // This routine can be entered from both ISTMain and MDD/Client threads so
    // need critical section.

    FUNCTION_ENTER_MSG();

    BYTE bRet = 0;

    BOOL fCompleted = FALSE; 
    PSTransfer pTransfer = peps->pTransfer;
    DWORD dwStatus = ERROR_GEN_FAILURE;
    DEBUGCHK(peps->fInitialized);
    DWORD dwEndpoint = peps->dwEndpointNumber;

    pTransfer = peps->pTransfer;
    if (pTransfer) {
        ValidateTransferDirection(pContext, peps, pTransfer);

        DEBUGCHK(pTransfer->dwFlags == USB_IN_TRANSFER);
        DEBUGCHK(pTransfer->dwUsbError == UFN_NOT_COMPLETE_ERROR);

        DWORD dwCurrentPermissions = GetCurrentPermissions();
//        SetProcPermissions(pTransfer->dwCallerPermissions);

        // Transfer is ready
        __try {
            PBYTE pbBuffer = (PBYTE) pTransfer->pvBuffer + pTransfer->cbTransferred;
            DWORD cbBuffer = pTransfer->cbBuffer - pTransfer->cbTransferred;

			
            volatile ULONG *pulFifoReg = _GetDataRegister(dwEndpoint);

            DWORD cbWritten = 0;


            // Min of input byte count and supported size
            DWORD cbToWrite = min(cbBuffer, peps->dwPacketSizeAssigned);
            BYTE bRegStatus = ReadIndexedReg(pContext, dwEndpoint, 
                IN_CSR1_REG_OFFSET);

            DEBUGMSG(ZONE_SEND, (_T("%s Tx on EP %u, Bytes = %u\r\n"), 
                pszFname, dwEndpoint, cbToWrite));

            if (dwEndpoint == 0) {
                for (cbWritten = 0; cbWritten < cbToWrite; cbWritten++) {
                    *pulFifoReg = *pbBuffer++;
                }

                /* We can complete on a packet which is full. We need to wait till
                * next time and generate a zero length packet, so only complete
                * if we're at the end and it is not the max packet size.
                */
                pTransfer->cbTransferred += cbWritten;
                if (pTransfer->cbTransferred == pTransfer->cbBuffer && pTransfer->pvPddData == 0) {
                    dwStatus = UFN_NO_ERROR;
                    fCompleted = TRUE;
                    pContext->Ep0State = EP0_STATE_IDLE;
                    bRet |= DATA_END;
                }
                
                /* Update the register - Set the IPR bit
                and possibly data end*/
                if( (cbWritten > 0) || (pTransfer->cbBuffer == 0) ) {
                    bRet |= EP0_IN_PACKET_RDY;
                }
                // Also set IPR if a 0 byte packet needs to go out.
                else if(pTransfer->pvPddData) {
                    bRet |= EP0_IN_PACKET_RDY;
                    pTransfer->pvPddData = 0;
                }
            }
            else {
                // Enable Interrupts before writing to the FIFO. This insures
                // That any interrupts generated because of the write will be
                // "latched"
                if (fEnableInterrupts) {
                    DEBUGCHK(dwEndpoint != 0);
		       EnableEndpointInterrupt(pContext, dwEndpoint);
                }

                // Write to the FIFO directly to send the bytes.
                for (cbWritten = 0; cbWritten < cbToWrite; cbWritten++) {
                    *pulFifoReg = (ULONG) *pbBuffer++;
                }

                // By Placing the check for packet complete here, before
                // cbTransferred is updated, there is a 1 interrupt cycle delay
                // That is complete is not declared until the data has actually
                // been ACKd (TPC set) by the host

		if ( (pTransfer->cbTransferred == pTransfer->cbBuffer) || 
                     (cbWritten == 0) ){
                    fCompleted = TRUE;
                    dwStatus = UFN_NO_ERROR;
                    
                    if ( cbWritten == 0 && (USBClassInfo == USB_RNDIS) )
                    	bRet |= IN_PACKET_READY;
                }
                else {
										/* Set In Packet Ready , this will Tells the HW the FIFO is 
                    ready to be transitted to be sent
                    */
                    bRet |= IN_PACKET_READY;		
                }
                // Update the Transfered Count
                pTransfer->cbTransferred += cbWritten;
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            DEBUGMSG(ZONE_ERROR, (_T("%s Exception!\r\n"), pszFname));
            fCompleted = TRUE;
            dwStatus = UFN_CLIENT_BUFFER_ERROR;
        }

        SetProcPermissions(dwCurrentPermissions);
    }   
    else {
        // It is possible for an interrupt to come in while still in this 
        // function for first pass of transfer. If this happens it is possible
        // to complete the transfer and have that interrupt be unnecessary
        // so... just ignore it.
        goto EXIT;
    }


    if (fCompleted) {
        // Disable transfer interrupts until another transfer is issued.
#if 1        
        if (peps->dwEndpointNumber != 0) {
            DisableEndpointInterrupt(pContext, peps->dwEndpointNumber);
        }
#endif

        DEBUGMSG(ZONE_SEND, (_T("%s Tx Done  Ep%x  Status %u\r\n"), pszFname, 
            dwEndpoint, dwStatus));
        CompleteTransfer(pContext, peps, dwStatus);
    }
    else {
        DEBUGMSG(ZONE_SEND, (_T("%s Tx EP%x BufferSize=%u, Xfrd=%u\r\n"), 
            pszFname, dwEndpoint, pTransfer->cbBuffer, pTransfer->cbTransferred));
    }

EXIT:
    FUNCTION_LEAVE_MSG();

    return bRet;
}


static
VOID
HandleEndpoint0Event(
                    PCTRLR_PDD_CONTEXT  pContext
                    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    ValidateContext(pContext);
    DEBUGCHK(pContext->fRunning);

    PEP_STATUS peps = GetEpStatus(pContext, 0);
    LOCK_ENDPOINT(peps);

    ClearEndpointInterrupt(pContext, 0);
    BYTE bEP0IrqStatus = ReadIndexedReg(pContext, 0, EP0_CSR_REG_OFFSET);

    DEBUGMSG(ZONE_USB_EVENTS, (_T("%s EP0_CSR_REG = 0x%02x. Data phase = %u\r\n"), 
        pszFname, bEP0IrqStatus, pContext->Ep0State));

    // Write 0 to SEND_STALL and SENT_STALL to clear them, so we need to 
    // leave them unchanged by default.
    BYTE bEp0CsrToWrite = 0;

    if (bEP0IrqStatus & SETUP_END) {
        bEp0CsrToWrite |= SERVICED_SETUP_END;

        pContext->Ep0State = EP0_STATE_IDLE;
        PSTransfer pTransfer = peps->pTransfer;

        // Make MDD think everything is ok.
        if(pTransfer) {
            pContext->sendDataEnd = FALSE;
            CompleteTransfer(pContext, peps, UFN_NO_ERROR);
        }

        DEBUGMSG(ZONE_USB_EVENTS, (_T("%s Setup End, %x\r\n"), 
            pszFname, bEP0IrqStatus));
    }

    // Set By USB if protocol violation detected
    if (bEP0IrqStatus & EP0_SENT_STALL) {
        // Must Clear both Send and Sent Stall
        pContext->Ep0State = EP0_STATE_IDLE;
    }

    BOOL fSendUdr = FALSE;
    
    DWORD cbFifo = ReadIndexedReg(pContext, 0, OUT_FIFO_CNT1_REG_OFFSET);
    BOOL fCompleted = FALSE;
    DWORD dwStatus;

    if (pContext->Ep0State == EP0_STATE_IDLE) {
        if (bEP0IrqStatus & EP0_OUT_PACKET_RDY) {
            if (pContext->fSpeedReported == FALSE) {
                // After Every Reset Notify MDD of Speed setting.
                // This device can only support FULL Speed.
                pContext->pfnNotify(pContext->pvMddContext, UFN_MSG_BUS_SPEED,
                    BS_FULL_SPEED);
                pContext->fSpeedReported = TRUE;
            }
            
            // New setup packet
            const DWORD cbOutFifo = ReadIndexedReg(pContext, 0, 
                OUT_FIFO_CNT1_REG_OFFSET);

            PBYTE pbUdr = (PBYTE) &pContext->udr;
            volatile ULONG *pulFifoReg = _GetDataRegister(0);
            
            DWORD cbBytesRemaining = cbOutFifo;
            while (cbBytesRemaining--) {
                *pbUdr++ = (BYTE) *pulFifoReg;
            }

            if (cbOutFifo != sizeof(pContext->udr)) {
                DEBUGMSG(ZONE_ERROR, (_T("%s Setup packet was only %x bytes!\r\n"),
                    pszFname, cbOutFifo));
                
                // Ideally this should not hapen. This is a recovery mechanism if 
                // we get out of sync somehow.  
                bEp0CsrToWrite |= (EP0_SEND_STALL | SERVICED_OUT_PKY_RDY | 
                    DATA_END);
            }
            else {
                // Parse the Setup Command this is necessary to Configure the 
                // SW State Machine and to set bits to enable the HW to 
                // ACK/NAK correctly.
                
                // Determine if this is a NO Data Packet
                if (pContext->udr.wLength > 0) {
                    // Determine transfer Direction
                    if (pContext->udr.bmRequestType & USB_ENDPOINT_DIRECTION_MASK) {
                        // Start the SW OUT State Machine
                        pContext->Ep0State = EP0_STATE_IN_DATA_PHASE;                      
                    }
                    else {
                        // Start the SW OUT State Machine
                        pContext->Ep0State = EP0_STATE_OUT_DATA_PHASE; 
                    }

                    pContext->sendDataEnd = FALSE;
                }
                else { // udr.wLength == 0
                    // ClientDriver will issue a SendControlStatusHandshake to 
                    // complete the transaction.
                    pContext->sendDataEnd = TRUE;

                    // Nothing left to do... stay in IDLE.
                    DEBUGCHK(pContext->Ep0State == EP0_STATE_IDLE);
                }

                fSendUdr = TRUE;
            }
        }
    }
    else if (pContext->Ep0State == EP0_STATE_OUT_DATA_PHASE) {
        if (bEP0IrqStatus & OUT_PACKET_READY) {
            bEp0CsrToWrite |= SERVICED_OUT_PKY_RDY;
            
            // Check For out packet read && receive fifo not empty -> out token event
            if (cbFifo) {
                DEBUGMSG(ZONE_RECEIVE, (_T("%s out token packet on endpoint 0 \r\n"),
                    pszFname));                
                bEp0CsrToWrite |= HandleRx(pContext, peps, &fCompleted, &dwStatus);
            }
            // status stage of control transfer; zero-length packet received
            else {                     
                DEBUGMSG(ZONE_RECEIVE, (_T("%s status stage of control transfer on endpoint 0\r\n"),
                    pszFname));
                pContext->Ep0State = EP0_STATE_IDLE;
            }
        }
    }
    else {
        DEBUGMSG(ZONE_SEND, (_T("%s Data Phase In Token\r\n"), pszFname));
        
        DEBUGCHK(pContext->Ep0State == EP0_STATE_IN_DATA_PHASE);

        BOOL bHandleTx = FALSE;

        // IN Data Phase and IPR is cleared 
        // Check For status stage - End control transfer; zero-length packet received
        if ( (bEP0IrqStatus &  OUT_PACKET_READY) && (cbFifo == 0) ) {
            bHandleTx = TRUE;

            DEBUGMSG(ZONE_SEND, (_T("%s In - end xfer\r\n"),
                pszFname));
        }
        else if ((bEP0IrqStatus & EP0_IN_PACKET_RDY) == 0) {
            bHandleTx = TRUE;
        }

        if (bHandleTx) {
            bEp0CsrToWrite |= HandleTx(pContext, peps, 0);
        }
    }

    // Clear any interrupts
    DEBUGMSG(ZONE_COMMENT, (_T("%s Writing 0x%02x to EP0_CSR\r\n"), pszFname, bEp0CsrToWrite));
    WriteIndexedReg(pContext, 0, EP0_CSR_REG_OFFSET, bEp0CsrToWrite);

    if (fCompleted) {
        CompleteTransfer(pContext, peps, dwStatus);
    }

    if (fSendUdr) {
        pContext->pfnNotify(pContext->pvMddContext, UFN_MSG_SETUP_PACKET, (DWORD) &pContext->udr);
    }

    FUNCTION_LEAVE_MSG();
    UNLOCK_ENDPOINT(peps);
}


// Process an endpoint interrupt.  Call interrupt-specific handler.
static
VOID
HandleEndpointEvent(
                    PCTRLR_PDD_CONTEXT  pContext,
                    DWORD               dwEndpoint,
                    DWORD               epIrqStat
                    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    ValidateContext(pContext);
    DEBUGCHK(pContext->fRunning);
    DEBUGCHK(dwEndpoint != 0);

    DWORD dwPendingEvents = 0;
    EP_STATUS *peps = GetEpStatus(pContext, dwEndpoint);
    PREFAST_DEBUGCHK(peps);

    LOCK_ENDPOINT(peps);

    ClearEndpointInterrupt(pContext, dwEndpoint);

    BYTE bEpIrqStat;
    BYTE bEpIrqStatToWrite;
    DWORD dwIndexRegOffset;
	
    if (peps->dwDirectionAssigned == USB_IN_TRANSFER) {
        dwIndexRegOffset = IN_CSR1_REG_OFFSET;
    }
    else {
        dwIndexRegOffset = OUT_CSR1_REG_OFFSET;
    }

    bEpIrqStat = ReadIndexedReg(pContext, dwEndpoint, dwIndexRegOffset);

    if (peps->dwDirectionAssigned == USB_IN_TRANSFER) {
        bEpIrqStatToWrite = bEpIrqStat & (IN_SEND_STALL  | IN_SENT_STALL) & 
            ~(IN_CLR_DATA_TOGGLE);
        DEBUGMSG(ZONE_COMMENT, (_T("%s Endpoint %u IN_CSR1_REG = 0x%02x\r\n"), 
            pszFname, dwEndpoint, bEpIrqStat));

        // Stall "acknowledged" from Host
        if (bEpIrqStat & IN_SENT_STALL) {
            USB_DEVICE_REQUEST udr;
            udr.bmRequestType = USB_REQUEST_FOR_ENDPOINT;
            udr.bRequest = USB_REQUEST_CLEAR_FEATURE;
            udr.wValue = USB_FEATURE_ENDPOINT_STALL;
            udr.wIndex = USB_ENDPOINT_DIRECTION_MASK | (BYTE) dwEndpoint;
            udr.wLength = 0;

            DisableEndpointInterrupt(pContext, dwEndpoint);

            DEBUGMSG(ZONE_PIPE, (_T("%s Got IN_SENT_STALL EP%u \r\n"), 
                pszFname, dwEndpoint));

            pContext->pfnNotify(pContext->pvMddContext, UFN_MSG_SETUP_PACKET, (DWORD) &udr);
            // Must Clear both Send and Sent Stall
            bEpIrqStatToWrite &= ~(IN_SEND_STALL  | IN_SENT_STALL);
        }
        
        if( !(bEpIrqStat & IN_PACKET_READY) ) {// If Transmit Complete 
            // send another
            DEBUGMSG(ZONE_SEND, (_T("%s Transmit packet complete on endpoint %u \r\n"), 
                pszFname, dwEndpoint));
            dwPendingEvents = IN_TRANSFER;
        }
    }
    else {
        bEpIrqStatToWrite = bEpIrqStat & (OUT_SEND_STALL | OUT_SENT_STALL) &
            ~(OUT_CLR_DATA_TOGGLE);
        DEBUGMSG(ZONE_COMMENT, (_T("%s Endpoint %u OUT_CSR1_REG = 0x%02x\r\n"), 
            pszFname, dwEndpoint, bEpIrqStat));
        
        // Stall "acknowledged" from Host
        if (bEpIrqStat & OUT_SENT_STALL) {
            USB_DEVICE_REQUEST udr;
            udr.bmRequestType = USB_REQUEST_FOR_ENDPOINT;
            udr.bRequest = USB_REQUEST_CLEAR_FEATURE;
            udr.wValue = USB_FEATURE_ENDPOINT_STALL;
            PREFAST_SUPPRESS(12006, "Buffer access index expression 'udr.wIndex=(USHORT)dwEndpoint' is being truncated in a shortening cast.");
            udr.wIndex = (USHORT) dwEndpoint;
            udr.wLength = 0;
            
            DisableEndpointInterrupt(pContext, dwEndpoint);

            DEBUGMSG(ZONE_PIPE, (_T("%s Got OUT_SENT_STALL EP%u \r\n"), 
                pszFname, dwEndpoint));

            pContext->pfnNotify(pContext->pvMddContext, UFN_MSG_SETUP_PACKET, (DWORD) &udr);
            // Must Clear both Send and Sent Stall
            bEpIrqStatToWrite &= ~(OUT_SEND_STALL | OUT_SENT_STALL);
        }
        
        // receive packet complete && receive fifo not empty -> out token + data rx event
        if (bEpIrqStat & OUT_PACKET_READY) {
            DEBUGMSG(ZONE_RECEIVE, (_T("%s received packet complete on endpoint %u\r\n"), 
                pszFname, dwEndpoint ));
            // the UDC combines the OUT token event with the Data RX event
            dwPendingEvents = OUT_TRANSFER;
        }
    }

    BOOL fCompleted = FALSE;
    DWORD dwStatus;

    if (dwPendingEvents == IN_TRANSFER) {
        bEpIrqStatToWrite |= HandleTx(pContext, peps, 0); 
    }
    else if (dwPendingEvents == OUT_TRANSFER) {
        bEpIrqStatToWrite |= HandleRx(pContext, peps, &fCompleted, &dwStatus);
        // We have not yet cleared the interrupt so no other packets
        // should be in the FIFO.
        DEBUGCHK(ReadIndexedReg(pContext, peps->dwEndpointNumber, OUT_FIFO_CNT1_REG_OFFSET) == 0);
    }

    if (fCompleted) {
        // Disable transfer interrupts until another transfer is issued.
        DisableEndpointInterrupt(pContext, peps->dwEndpointNumber);
	}

    DEBUGMSG(ZONE_COMMENT, (_T("%s Writing 0x%02x to index 0x%02x\r\n"), pszFname, 
        bEpIrqStatToWrite, dwIndexRegOffset));
    WriteIndexedReg(pContext, dwEndpoint, dwIndexRegOffset, bEpIrqStatToWrite);

    if (fCompleted) {
        CompleteTransfer(pContext, peps, dwStatus);
	}
	
	
  
	FUNCTION_LEAVE_MSG();
    UNLOCK_ENDPOINT(peps);
}


// Process USB Bus interrupt
static
VOID
HandleUSBBusIrq(
                    PCTRLR_PDD_CONTEXT  pContext,
                    BYTE                bUSBBusIrqStat
                    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();
    
#if 0
        if (bUSBBusIrqStat & USB_SUSPEND_INTR) {
            WriteReg(pContext, USB_INT_REG_OFFSET, USB_SUSPEND_INTR);
            
            // Disable the interrupt
            SetClearReg(pContext, USB_INT_EN_REG_OFFSET, USB_SUSPEND_INTR, CLEAR);

            // Enable resume interrupt
            SetClearReg(pContext, USB_INT_EN_REG_OFFSET, USB_RESUME_INTR, SET);

            if (pContext->attachedState == UFN_ATTACH) {
                DEBUGMSG(ZONE_USB_EVENTS, (_T("%s Suspend\r\n"), pszFname));
    //            pContext->pfnNotify(pContext->pvMddContext, UFN_MSG_BUS_EVENTS, UFN_SUSPEND);

                pContext->pfnNotify(pContext->pvMddContext, UFN_MSG_BUS_EVENTS, UFN_DETACH);
                pContext->attachedState = UFN_DETACH;
            }
        }

        if (bUSBBusIrqStat & USB_RESUME_INTR) {
            WriteReg(pContext, USB_INT_REG_OFFSET, USB_RESUME_INTR);
            
            // Disable the interrupt
            SetClearReg(pContext, USB_INT_EN_REG_OFFSET, USB_RESUME_INTR, CLEAR);

            // Enable suspend interrupt
            SetClearReg(pContext, USB_INT_EN_REG_OFFSET, USB_SUSPEND_INTR, SET);

            if (pContext->attachedState == UFN_DETACH) {
                DEBUGMSG(ZONE_USB_EVENTS, (_T("%s Resume\r\n"), pszFname));
    //            pContext->pfnNotify(pContext->pvMddContext, UFN_MSG_BUS_EVENTS, UFN_RESUME);
                pContext->pfnNotify(pContext->pvMddContext, UFN_MSG_BUS_EVENTS, UFN_ATTACH);
                pContext->attachedState = UFN_ATTACH;
            }
        }
#endif

        if (bUSBBusIrqStat & USB_RESET_INTR) {
            WriteReg(pContext, USB_INT_REG_OFFSET, USB_RESET_INTR);

            DEBUGMSG(ZONE_USB_EVENTS, (_T("%s Reset\r\n"), pszFname));

            pContext->fSpeedReported = FALSE;

            // for checking usb cable plug or unplug

			if (pContext->isattachedusbcable == FALSE) {
				if (pContext->attachedState == UFN_DETACH) {
		        	pContext->pfnNotify(pContext->pvMddContext, UFN_MSG_BUS_EVENTS, UFN_ATTACH);
		            pContext->attachedState = UFN_ATTACH;
		        }
	        }
			else {
#if 1
            if (pContext->attachedState == UFN_DETACH){
                pContext->pfnNotify(pContext->pvMddContext, UFN_MSG_BUS_EVENTS, UFN_ATTACH);
                pContext->attachedState = UFN_ATTACH;
	   pContext->isattachedusbcable = FALSE;
#else
				if (pContext->attachedState == UFN_ATTACH) {
					pContext->pfnNotify(pContext->pvMddContext, UFN_MSG_BUS_EVENTS, UFN_DETACH);
			        pContext->attachedState = UFN_DETACH;
	              	pContext->isattachedusbcable = FALSE;
#endif
				}
			}

            pContext->Ep0State = EP0_STATE_IDLE;
            pContext->pfnNotify(pContext->pvMddContext, UFN_MSG_BUS_EVENTS, UFN_RESET);

            // Enable the Suspend interrupt...
    //        SetClearReg(pContext, USB_INT_EN_REG_OFFSET, USB_SUSPEND_INTR, SET);
        }

    FUNCTION_LEAVE_MSG();
}


// Process a SC2440 interrupt.
static
VOID
HandleUSBEvent(
               PCTRLR_PDD_CONTEXT pContext
               )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();
    ValidateContext(pContext);

    BYTE bEpIrqStat = ReadReg(pContext, EP_INT_REG_OFFSET);
    BYTE bUSBBusIrqStat = ReadReg(pContext, USB_INT_REG_OFFSET);

 //  RETAILMSG(1, (TEXT(" Hnt\r\n")));

  //  while (bEpIrqStat || bUSBBusIrqStat) {
        if (bUSBBusIrqStat) {
            DEBUGMSG(ZONE_COMMENT, (_T("%s USB_INT_REG = 0x%02x\r\n"), 
                pszFname, bUSBBusIrqStat));
            HandleUSBBusIrq(pContext, bUSBBusIrqStat);
        }

        if (bEpIrqStat) {
            DEBUGMSG(ZONE_COMMENT, (_T("%s EP_INT_REG = 0x%02x\r\n"), 
                pszFname, bEpIrqStat));

            if (bEpIrqStat & EP0_INT_INTR) {
                HandleEndpoint0Event(pContext);
            }
            
            // Process All Other (besides EP0) Endpoints
            for (DWORD dwEndpoint = 1; dwEndpoint < ENDPOINT_COUNT; ++dwEndpoint) {
                // Check the Interrupt Mask 
                // Check the Interrupt Status 
                BYTE bEpBit =  EpToIrqStatBit(dwEndpoint);
                if (bEpIrqStat & bEpBit) {
                    HandleEndpointEvent(pContext, dwEndpoint, bEpIrqStat);
                }
   
		}
        	}
        bEpIrqStat = ReadReg(pContext, EP_INT_REG_OFFSET);
        bUSBBusIrqStat = ReadReg(pContext, USB_INT_REG_OFFSET);
   // }
    
    FUNCTION_LEAVE_MSG();
}


/****************************************************************
@doc INTERNAL

@func VOID | SerUSB_InternalMapRegisterAddresses |
This routine maps the ASIC registers. 
It's an artifact of this
implementation.

@rdesc None.
****************************************************************/
static
DWORD MapRegisterSet(PCTRLR_PDD_CONTEXT pContext)
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    PREFAST_DEBUGCHK(pContext);

    DEBUGCHK(g_pUDCBase == NULL);

    PBYTE   pVMem;
    DWORD   dwRet = ERROR_SUCCESS;

    // Map CSR registers.
    pVMem = (PBYTE)VirtualAlloc(0, PAGE_SIZE, MEM_RESERVE, PAGE_NOACCESS);

    if (pVMem) {
        BOOL fSuccess = VirtualCopy(pVMem, (LPVOID)pContext->dwIOBase,
            pContext->dwIOLen, PAGE_READWRITE | PAGE_NOCACHE);
        if (!fSuccess) {
            VirtualFree(pVMem, 0, MEM_RELEASE);
            dwRet = GetLastError();
            DEBUGMSG(ZONE_ERROR, (_T("%s Virtual Copy: FAILED\r\n"), pszFname));
        }
        else {
            g_pUDCBase = pVMem + BASE_REGISTER_OFFSET;

            DEBUGMSG(ZONE_INIT, (_T("%s VirtualCopy Succeeded, pVMem:%x\r\n"), 
                pszFname, pVMem));
        }
    } 
    else {
        dwRet = GetLastError();
        DEBUGMSG(ZONE_ERROR, (_T("%s Virtual Alloc: FAILED\r\n"), pszFname));
    }

    FUNCTION_LEAVE_MSG();
    
    return dwRet;
}

/*++

Routine Description:

Deallocate register space.

Arguments:

None.

Return Value:

None.

--*/
static
VOID
UnmapRegisterSet(PCTRLR_PDD_CONTEXT pContext)
{
    // Unmap any memory areas that we may have mapped.
    if (g_pUDCBase) {
        VirtualFree((PVOID) g_pUDCBase, 0, MEM_RELEASE);
        g_pUDCBase = NULL;
    }
}


// interrupt service routine.
static
DWORD
WINAPI
ISTMain(
        LPVOID lpParameter
        )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    PCTRLR_PDD_CONTEXT pContext = (PCTRLR_PDD_CONTEXT) lpParameter;
    ValidateContext(pContext);

    CeSetThreadPriority(pContext->hIST, pContext->dwISTPriority);

    while (!pContext->fExitIST) {
        pContext->fRestartIST = FALSE;

        // Enable Suspend Mode in the Power Register
//        SetClearReg(pContext, PWR_REG_OFFSET, SUSPEND_MODE_ENABLE_CTRL, SET);

        // Disable All Endpoint interrupts
        WriteReg(pContext, EP_INT_EN_REG_OFFSET, 0); // Disable All

        // Enable Device interrupts
        WriteReg(pContext, USB_INT_EN_REG_OFFSET, (USB_RESET_INTR));// | USB_SUSPEND_INTR));

        // Enable Endpoint interrupt 0
        EnableEndpointInterrupt(pContext, 0);

        while (TRUE) {
            DWORD dwWait = WaitForSingleObject(pContext->hevInterrupt, INFINITE);
            if (pContext->fExitIST || pContext->fRestartIST) {
                break;
            }

            if (dwWait == WAIT_OBJECT_0) {
                HandleUSBEvent(pContext);
                InterruptDone(pContext->dwSysIntr);
            }
            else {
                DEBUGMSG(ZONE_INIT, (_T("%s WaitForMultipleObjects failed. Exiting IST.\r\n"), 
                    pszFname));
                break;
            }
        }

        // Disable Device  interrupts - write Zeros to Disable
        WriteReg(pContext, USB_INT_EN_REG_OFFSET, 0 );

        // Disable endpoint interrupts - write Zeros to Disable
        WriteReg(pContext, EP_INT_EN_REG_OFFSET, 0);
        
        // Clear any outstanding device & endpoint interrupts
        // USB Device Interrupt Status - Write a '1' to Clear 
        WriteReg(pContext, USB_INT_REG_OFFSET, 
            (USB_RESET_INTR | USB_RESUME_INTR | USB_SUSPEND_INTR));
        // End point Interrupt Status - Write a '1' to Clear
        WriteReg(pContext, EP_INT_REG_OFFSET, CLEAR_ALL_EP_INTRS);

        // Send detach
        pContext->pfnNotify(pContext->pvMddContext, 
            UFN_MSG_BUS_EVENTS, UFN_DETACH);

        pContext->fSpeedReported = FALSE;
        pContext->attachedState = UFN_DETACH;
        pContext->isattachedusbcable = FALSE;
    }

    FUNCTION_LEAVE_MSG();

    return 0;
}

static
VOID
StartTransfer(
              PCTRLR_PDD_CONTEXT pContext,
              PEP_STATUS peps,
              PSTransfer pTransfer
              )
{
    SETFNAME();

    DEBUGCHK(pContext);
    PREFAST_DEBUGCHK(peps);

    DEBUGCHK(!peps->pTransfer);
    ValidateTransferDirection(pContext, peps, pTransfer);

    LOCK_ENDPOINT(peps);
    FUNCTION_ENTER_MSG();

    DEBUGMSG(ZONE_TRANSFER, (_T("%s Setting up %s transfer on ep %u for %u bytes\r\n"),
        pszFname, (pTransfer->dwFlags == USB_IN_TRANSFER) ? _T("in") : _T("out"),
        peps->dwEndpointNumber, pTransfer->cbBuffer));

    // Enable transfer interrupts.
    peps->pTransfer = pTransfer;

    if (pTransfer->dwFlags == USB_IN_TRANSFER) {
        // Must Clear both Send and Sent Stall - the HW is setting this bit 
        // during the Endpoint initialization process. It must be cleared here 
        // to insure proper operation.
//        SetClearIndexedReg(pContext, peps->dwEndpointNumber, IN_CSR1_REG_OFFSET,
//            (IN_SEND_STALL | IN_SENT_STALL), CLEAR);               

        if (peps->dwEndpointNumber == 0) {
            if ( (pTransfer->cbBuffer < pContext->udr.wLength) &&
                 (pTransfer->cbBuffer != 0) &&
                 ( (pTransfer->cbBuffer % peps->dwPacketSizeAssigned) == 0 ) ) {
                // This transfer will need a 0-length packet to terminate it.
                pTransfer->pvPddData = (PVOID) 1;
            }
            
            BYTE bEP0IrqStatus = ReadIndexedReg(pContext, 0, EP0_CSR_REG_OFFSET);

            // Write 0 to SEND_STALL and SENT_STALL to clear them, so we need to 
            // leave them unchanged by default.
            BYTE bEp0CsrToWrite = (bEP0IrqStatus & EP0_STALL_BITS);

            if (bEP0IrqStatus & EP0_OUT_PACKET_RDY) {
                bEp0CsrToWrite |= SERVICED_OUT_PKY_RDY;
                DEBUGMSG(ZONE_COMMENT, (_T("%s Writing 0x%02x to EP0_CSR_REG\r\n"), pszFname, bEp0CsrToWrite));
                WriteIndexedReg(pContext, 0, EP0_CSR_REG_OFFSET, bEp0CsrToWrite);
                bEp0CsrToWrite &= ~SERVICED_OUT_PKY_RDY;
            }

            bEp0CsrToWrite |= HandleTx(pContext, peps, 0);
            
            DEBUGMSG(ZONE_COMMENT, (_T("%s Writing 0x%02x to EP0_CSR_REG\r\n"), pszFname, bEp0CsrToWrite));
            WriteIndexedReg(pContext, 0, EP0_CSR_REG_OFFSET, bEp0CsrToWrite);
        }
        else {
            BYTE bIrqStatus = ReadIndexedReg(pContext, 0, IN_CSR1_REG_OFFSET);

            // Write 0 to SEND_STALL and SENT_STALL to clear them, so we need to 
            // leave them unchanged by default.
            BYTE bEpStatToWrite = (bIrqStatus & (IN_SENT_STALL & IN_SEND_STALL));

            bEpStatToWrite |= HandleTx(pContext, peps, 1);

            DEBUGMSG(ZONE_COMMENT, (_T("%s Writing 0x%02x to IN_CSR1_REG\r\n"), pszFname));
            WriteIndexedReg(pContext, peps->dwEndpointNumber, 
                IN_CSR1_REG_OFFSET, bEpStatToWrite);
        }
    }
    else {
        if (peps->dwEndpointNumber == 0) {
            BYTE bEP0IrqStatus = ReadIndexedReg(pContext, 0, EP0_CSR_REG_OFFSET);

            // Write 0 to SEND_STALL and SENT_STALL to clear them, so we need to 
            // leave them unchanged by default.
            BYTE bEp0CsrToWrite = (bEP0IrqStatus & EP0_STALL_BITS);
            
            // Mark that we are done with the setup packet. The IST will take care
            // of the data transfer.
            if (bEP0IrqStatus & EP0_OUT_PACKET_RDY) {
                bEp0CsrToWrite |= SERVICED_OUT_PKY_RDY;
                DEBUGMSG(ZONE_COMMENT, (_T("%s Writing 0x%02x to EP0_CSR_REG\r\n"), pszFname, bEp0CsrToWrite));
                WriteIndexedReg(pContext, 0, EP0_CSR_REG_OFFSET, bEp0CsrToWrite);
            }
        }
        else {
            EnableEndpointInterrupt(pContext, peps->dwEndpointNumber);

            // There may be a packet available.  If so process it...
            BYTE bEpIrqStat = ReadIndexedReg(pContext, peps->dwEndpointNumber,
                OUT_CSR1_REG_OFFSET);
            
            if (bEpIrqStat & OUT_PACKET_READY) {
                BOOL fCompleted;
                DWORD dwStatus;
                BYTE bToWrite = HandleRx(pContext, peps, &fCompleted, &dwStatus);
                DEBUGMSG(ZONE_COMMENT, (_T("%s Writing 0x%02x to OUT_CSR1_REG\r\n"), pszFname, 
                    bToWrite));

                // There should be no double-buffering
                DEBUGCHK(ReadIndexedReg(pContext, peps->dwEndpointNumber, OUT_FIFO_CNT1_REG_OFFSET) == 0);

                if (fCompleted) {
                    // Disable transfer interrupts until another transfer is issued.
                    DisableEndpointInterrupt(pContext, peps->dwEndpointNumber);
                }

                WriteIndexedReg(pContext, peps->dwEndpointNumber, OUT_CSR1_REG_OFFSET, bToWrite);
                
                if (fCompleted) {
                    CompleteTransfer(pContext, peps, dwStatus);
                }    
            }
        }
    }
    
    FUNCTION_LEAVE_MSG();

    UNLOCK_ENDPOINT(peps);
}
DWORD
WINAPI
UfnPdd_IssueTransfer(
    PVOID  pvPddContext,
    DWORD  dwEndpoint,
    PSTransfer pTransfer
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    DEBUGCHK(EP_VALID(dwEndpoint));

    PCTRLR_PDD_CONTEXT pContext = (PCTRLR_PDD_CONTEXT) pvPddContext;
    ValidateContext(pContext);

    PEP_STATUS peps = GetEpStatus(pContext, dwEndpoint);
    LOCK_ENDPOINT(peps);
    DEBUGCHK(peps->fInitialized);
    DEBUGCHK(pTransfer->cbTransferred == 0);

    DWORD dwRet = ERROR_SUCCESS;

    // Note For the HW NAKs IN requests and DOES NOT let SW
    // know that the Host is trying to send a request. SO... Start the Transfer 
    // In Now!
    // Start the Transfer
    DEBUGCHK(peps->pTransfer == NULL);
    StartTransfer(pContext, peps, pTransfer);

    UNLOCK_ENDPOINT(peps);

    FUNCTION_LEAVE_MSG();

    return dwRet;
}


DWORD
WINAPI
UfnPdd_AbortTransfer(
    PVOID           pvPddContext,
    DWORD           dwEndpoint,
    PSTransfer      pTransfer
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    PREFAST_DEBUGCHK(pTransfer);
    DEBUGCHK(EP_VALID(dwEndpoint));

    PCTRLR_PDD_CONTEXT pContext = (PCTRLR_PDD_CONTEXT) pvPddContext;
    ValidateContext(pContext);

    PEP_STATUS peps = GetEpStatus(pContext, dwEndpoint);
    LOCK_ENDPOINT(peps);
    DEBUGCHK(peps->fInitialized);

    ValidateTransferDirection(pContext, peps, pTransfer);

    DEBUGCHK(pTransfer == peps->pTransfer);
    CompleteTransfer(pContext, peps, UFN_CANCELED_ERROR);

    if (dwEndpoint == 0) {
        pContext->Ep0State = EP0_STATE_IDLE;
    }
    
    ResetEndpoint( pContext,peps);

    UNLOCK_ENDPOINT(peps);

    FUNCTION_LEAVE_MSG();

    return ERROR_SUCCESS;
}


#define CLKCON_USBD (1<<7)

volatile S3C2440A_CLKPWR_REG *pCLKPWR	= NULL;		// Clock power registers (needed to enable I2S and SPI clocks)
volatile S3C2440A_IOPORT_REG *pIOPregs	= NULL;

BOOL HW_USBClocks(CEDEVICE_POWER_STATE    cpsNew)
{

	pCLKPWR = (volatile S3C2440A_CLKPWR_REG*)VirtualAlloc(0, sizeof(S3C2440A_CLKPWR_REG), MEM_RESERVE, PAGE_NOACCESS);
	if (!pCLKPWR)
	{
		DEBUGMSG(1, (TEXT("pCLKPWR: VirtualAlloc failed!\r\n")));
		return(FALSE);
	}
	if (!VirtualCopy((PVOID)pCLKPWR, (PVOID)(S3C2440A_BASE_REG_PA_CLOCK_POWER >> 8), sizeof(S3C2440A_CLKPWR_REG), PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE))
	{
		DEBUGMSG(1, (TEXT("pCLKPWR: VirtualCopy failed!\r\n")));
		return(FALSE);
	}

	pIOPregs = (volatile S3C2440A_IOPORT_REG *)VirtualAlloc(0, sizeof(S3C2440A_IOPORT_REG), MEM_RESERVE, PAGE_NOACCESS);
	if(!pIOPregs) {
		DEBUGMSG(1, (TEXT("pIOPregs: VirtualAlloc failed!\r\n")));
		return(FALSE);
	}
	if(!VirtualCopy((PVOID)pIOPregs, (PVOID)(S3C2440A_BASE_REG_PA_IOPORT >> 8), sizeof(S3C2440A_IOPORT_REG), PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE)) 
	{
		DEBUGMSG(1, (TEXT("pIOPregs: VirtualCopy failed!\r\n")));
		return(FALSE);
	}

	if (cpsNew == D0)
	{
		RETAILMSG(1, (TEXT("HW_USBClocks::D0 \r\n")));
        // Enable the USB Clocks
        pCLKPWR->CLKCON |= (CLKCON_USBD);
        // MISCCR: USBD Pads, Normal mode
        pIOPregs->MISCCR &= ~((3 << 12) | (1 << 3));
        // Enable USB_PULLUP on GPIO PIN (tied to USB D+) & set high
		pIOPregs->GPGCON &= ~(3 << 24);    // clear GPG12
		pIOPregs->GPGCON |=  (1 << 24);    // config as output
		pIOPregs->GPGUP  |=  (1 << 12);    // pullup disabled
		pIOPregs->GPGDAT |=  (1 << 12);    // set high
	}
	else if (cpsNew == D4) 
	{
		RETAILMSG(1, (TEXT("HW_USBClocks::D4 \r\n")));
        // Disable the USB Clocks
        pCLKPWR->CLKCON &= ~(CLKCON_USBD);
        // MISCCR: USBD Pads, Suspend mode
        pIOPregs->MISCCR |= (3 << 12);
        // Disable USB_PULLUP to remove us from the bus
		pIOPregs->GPGCON &= ~(3 << 24);    // clear GPG12
		pIOPregs->GPGCON |=  (1 << 24);    // config as output
		pIOPregs->GPGUP  |=  (1 << 12);    // pullup disabled
		pIOPregs->GPGDAT &= ~(1 << 12);    // set low
	}
	if (pIOPregs)
    {
        VirtualFree((PVOID)pIOPregs, 0, MEM_RELEASE);
        pIOPregs = NULL;
    }
    if (pCLKPWR)
    {   
        VirtualFree((PVOID)pCLKPWR, 0, MEM_RELEASE);
        pCLKPWR = NULL;
    } 
	return TRUE;
}

// This does not do much because there is not any way to control
// power on this controller.
static
CEDEVICE_POWER_STATE
SetPowerState(
    PCTRLR_PDD_CONTEXT      pContext,
    CEDEVICE_POWER_STATE    cpsNew
    )
{
    SETFNAME();
    
    PREFAST_DEBUGCHK(pContext);
    DEBUGCHK(VALID_DX(cpsNew));
    ValidateContext(pContext);

    // Adjust cpsNew.
    if (cpsNew != pContext->cpsCurrent) {
        if (cpsNew == D1 || cpsNew == D2) {
            // D1 and D2 are not supported.
            cpsNew = D0;
        }
        else if (pContext->cpsCurrent == D4) {
            // D4 can only go to D0.
            cpsNew = D0;
        }
    }

    if (cpsNew != pContext->cpsCurrent) {
        DEBUGMSG(ZONE_POWER, (_T("%s Going from D%u to D%u\r\n"),
            pszFname, pContext->cpsCurrent, cpsNew));
        
        if ( (cpsNew < pContext->cpsCurrent) && pContext->hBusAccess ) {
            SetDevicePowerState(pContext->hBusAccess, cpsNew, NULL);
        }

        switch (cpsNew) {
        case D0:
			HW_USBClocks(D0);
            KernelIoControl(IOCTL_HAL_DISABLE_WAKE, &pContext->dwSysIntr,  
                sizeof(pContext->dwSysIntr), NULL, 0, NULL);
            
            if (pContext->fRunning) {
                // Cause the IST to restart.
                pContext->fRestartIST = TRUE;
                SetInterruptEvent(pContext->dwSysIntr);
            }           
            break;

        case D3:
            KernelIoControl(IOCTL_HAL_ENABLE_WAKE, &pContext->dwSysIntr,  
                sizeof(pContext->dwSysIntr), NULL, 0, NULL);
            break;

        case D4:
			HW_USBClocks(D4);
            KernelIoControl(IOCTL_HAL_DISABLE_WAKE, &pContext->dwSysIntr,  
                sizeof(pContext->dwSysIntr), NULL, 0, NULL);
            break;
        }

        if ( (cpsNew > pContext->cpsCurrent) && pContext->hBusAccess ) {
            SetDevicePowerState(pContext->hBusAccess, cpsNew, NULL);
        }

		pContext->cpsCurrent = cpsNew;
    }

    return pContext->cpsCurrent;
}


static
VOID
FreeCtrlrContext(
    PCTRLR_PDD_CONTEXT pContext
    )
{
    PREFAST_DEBUGCHK(pContext);
    DEBUGCHK(!pContext->hevInterrupt);
    DEBUGCHK(!pContext->hIST);
    DEBUGCHK(!pContext->fRunning);

    pContext->dwSig = GARBAGE_DWORD;

    UnmapRegisterSet(pContext);

    if (pContext->hBusAccess) CloseBusAccessHandle(pContext->hBusAccess);
    
    if (pContext->dwSysIntr) {
        KernelIoControl(IOCTL_HAL_DISABLE_WAKE, &pContext->dwSysIntr,  
    	    sizeof(pContext->dwSysIntr), NULL, 0, NULL);
    }

    if (pContext->dwIrq != IRQ_UNSPECIFIED) {
        KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &pContext->dwSysIntr, 
            sizeof(DWORD), NULL, 0, NULL);
    }
    
    DeleteCriticalSection(&pContext->csIndexedRegisterAccess);

    LocalFree(pContext);
}


DWORD
WINAPI
UfnPdd_Deinit(
    PVOID pvPddContext
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    PCTRLR_PDD_CONTEXT pContext = (PCTRLR_PDD_CONTEXT) pvPddContext;
    ValidateContext(pContext);

    FUNCTION_ENTER_MSG();

    FreeCtrlrContext(pContext);

    FUNCTION_LEAVE_MSG();

    return ERROR_SUCCESS;
}


DWORD
WINAPI
UfnPdd_Start(
    PVOID        pvPddContext
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    DWORD dwRet;
    PCTRLR_PDD_CONTEXT pContext = (PCTRLR_PDD_CONTEXT) pvPddContext;
    ValidateContext(pContext);

    FUNCTION_ENTER_MSG();

    DEBUGCHK(!pContext->fRunning);

    BOOL fIntInitialized = FALSE;

    // Create the interrupt event
    pContext->hevInterrupt = CreateEvent(0, FALSE, FALSE, NULL);
    if (pContext->hevInterrupt == NULL) {
        dwRet = GetLastError();
        DEBUGMSG(ZONE_ERROR, (_T("%s Error creating  interrupt event. Error = %d\r\n"), 
            pszFname, dwRet));
        goto EXIT;
    }

    fIntInitialized = InterruptInitialize(pContext->dwSysIntr, 
        pContext->hevInterrupt, NULL, 0);
    if (fIntInitialized == FALSE) {
        dwRet = ERROR_GEN_FAILURE;
        DEBUGMSG(ZONE_ERROR, (_T("%s  interrupt initialization failed\r\n"),
            pszFname));
        goto EXIT;
    }
    InterruptDone(pContext->dwSysIntr);

    pContext->fExitIST = FALSE;
    pContext->hIST = CreateThread(NULL, 0, ISTMain, pContext, 0, NULL);    
    if (pContext->hIST == NULL) {
        DEBUGMSG(ZONE_ERROR, (_T("%s IST creation failed\r\n"), pszFname));
        dwRet = GetLastError();
        goto EXIT;
    }

    pContext->fRunning = TRUE;
    dwRet = ERROR_SUCCESS;

EXIT:
    if (pContext->fRunning == FALSE) {
        DEBUGCHK(dwRet != ERROR_SUCCESS);
        if (fIntInitialized) InterruptDisable(pContext->dwSysIntr);
        if (pContext->hevInterrupt) CloseHandle(pContext->hevInterrupt);
        pContext->hevInterrupt = NULL;
    }

    FUNCTION_LEAVE_MSG();

    return dwRet;
}


// Stop the device.
DWORD
WINAPI
UfnPdd_Stop(
            PVOID pvPddContext
            )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    PCTRLR_PDD_CONTEXT pContext = (PCTRLR_PDD_CONTEXT) pvPddContext;
    ValidateContext(pContext);

    DEBUGCHK(pContext->fRunning);

    // Stop the IST
    pContext->fExitIST = TRUE;
    InterruptDisable(pContext->dwSysIntr);
    SetEvent(pContext->hevInterrupt);
    WaitForSingleObject(pContext->hIST, INFINITE);
    CloseHandle(pContext->hevInterrupt);
    CloseHandle(pContext->hIST);
    pContext->hIST = NULL;
    pContext->hevInterrupt = NULL;

    ResetDevice(pContext);

    pContext->fRunning = FALSE;

    DEBUGMSG(ZONE_FUNCTION, (_T("%s Device has been stopped\r\n"), 
        pszFname));

    FUNCTION_LEAVE_MSG();

    return ERROR_SUCCESS;
}


DWORD 
WINAPI 
UfnPdd_IsConfigurationSupportable(
    PVOID                       pvPddContext,
    UFN_BUS_SPEED               Speed,
    PUFN_CONFIGURATION          pConfiguration
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    DEBUGCHK(Speed == BS_FULL_SPEED);

    PCTRLR_PDD_CONTEXT pContext = (PCTRLR_PDD_CONTEXT) pvPddContext;
    ValidateContext(pContext);

    // This PDD does not have any special requirements that cannot be 
    // handled through IsEndpointSupportable.
    DWORD dwRet = ERROR_SUCCESS;

    FUNCTION_LEAVE_MSG();

    return dwRet;
}


// 
// Is this endpoint supportable.
DWORD
WINAPI
UfnPdd_IsEndpointSupportable(
    PVOID                       pvPddContext,
    DWORD                       dwEndpoint,
    UFN_BUS_SPEED               Speed,
    PUSB_ENDPOINT_DESCRIPTOR    pEndpointDesc,
    BYTE                        bConfigurationValue,
    BYTE                        bInterfaceNumber,
    BYTE                        bAlternateSetting
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    DEBUGCHK(EP_VALID(dwEndpoint));
    DEBUGCHK(Speed == BS_FULL_SPEED);

    DWORD dwRet = ERROR_SUCCESS;
    PCTRLR_PDD_CONTEXT pContext = (PCTRLR_PDD_CONTEXT) pvPddContext;
    ValidateContext(pContext);

    PEP_STATUS peps = GetEpStatus(pContext, dwEndpoint);

    // Special case for endpoint 0
    if (dwEndpoint == 0) {
        DEBUGCHK(pEndpointDesc->bmAttributes == USB_ENDPOINT_TYPE_CONTROL);
        
        // Endpoint 0 only supports 8 or 16 byte packet size
        if (pEndpointDesc->wMaxPacketSize < EP_0_PACKET_SIZE) {
            DEBUGMSG(ZONE_ERROR, (_T("%s Endpoint 0 only supports %u byte packets\r\n"),
                pszFname, EP_0_PACKET_SIZE));
            dwRet = ERROR_INVALID_PARAMETER;
        }
        else{  
            // Larger than EP 0 Max Packet Size - reduce to Max
            pEndpointDesc->wMaxPacketSize = EP_0_PACKET_SIZE;
        }
    }
    else if (dwEndpoint < ENDPOINT_COUNT) {
        BYTE bTransferType = pEndpointDesc->bmAttributes & USB_ENDPOINT_TYPE_MASK;
        DEBUGCHK(bTransferType != USB_ENDPOINT_TYPE_CONTROL);

        // Validate and adjust packet size
        WORD wPacketSize = 
            (pEndpointDesc->wMaxPacketSize & USB_ENDPOINT_MAX_PACKET_SIZE_MASK);

        switch(bTransferType) {

        // Isoch not currently supported by Samsung HW
        case USB_ENDPOINT_TYPE_ISOCHRONOUS:
            DEBUGMSG(ZONE_ERROR, (_T("%s Isochronous endpoints are not supported\r\n"),
                pszFname));
            dwRet = ERROR_INVALID_PARAMETER;
            break;

        case USB_ENDPOINT_TYPE_BULK:
        case USB_ENDPOINT_TYPE_INTERRUPT:
            // HW Can only Support 8, 16, 32, 64 byte packets
            if((wPacketSize >= 8) && (wPacketSize < 16)){
                wPacketSize = 8;
            }
            else if ((wPacketSize >= 16) && (wPacketSize < 64)){
                // Note that 32 => Dual Packet mode - Do NOT allow
                wPacketSize = 16;
            }
            else if (wPacketSize >= 64 ){
                wPacketSize = 64;                
            }
            else{ // wPacketSize < 8
                dwRet = ERROR_INVALID_PARAMETER;
            }
            break;

        default:
            dwRet = ERROR_INVALID_PARAMETER;
            break;
        }  
        
        // If Requested Size is larger than what is supported ... change it.
        // Note only try and change it if no errors so far... meaning Ep is
        // Supportable.
        if ( (wPacketSize != (pEndpointDesc->wMaxPacketSize & USB_ENDPOINT_MAX_PACKET_SIZE_MASK)) && 
             (dwRet == ERROR_SUCCESS) ) {
            pEndpointDesc->wMaxPacketSize &= ~USB_ENDPOINT_MAX_PACKET_SIZE_MASK;
            pEndpointDesc->wMaxPacketSize |= wPacketSize;
        }
    }

    FUNCTION_LEAVE_MSG();

    return dwRet;
}


// Clear an endpoint stall.
DWORD
WINAPI
UfnPdd_ClearEndpointStall(
                          PVOID pvPddContext,
                          DWORD dwEndpoint
                          )
{
    DWORD dwRet = ERROR_SUCCESS;

    SETFNAME();
    FUNCTION_ENTER_MSG();

    DEBUGCHK(EP_VALID(dwEndpoint));

    PCTRLR_PDD_CONTEXT pContext = (PCTRLR_PDD_CONTEXT) pvPddContext;
    ValidateContext(pContext);

    PEP_STATUS peps = GetEpStatus(pContext, dwEndpoint);
    LOCK_ENDPOINT(peps);

    if (dwEndpoint == 0){
        // Must Clear both Send and Sent Stall
        WriteIndexedReg(pContext, 0, EP0_CSR_REG_OFFSET, 0);
    }
    else if (peps->dwDirectionAssigned == USB_IN_TRANSFER){
        SetClearIndexedReg(pContext, dwEndpoint, IN_CSR1_REG_OFFSET,
            (IN_SEND_STALL | IN_CLR_DATA_TOGGLE ), SET);

        // Must Clear both Send and Sent Stall
        SetClearIndexedReg(pContext, dwEndpoint, IN_CSR1_REG_OFFSET,
            (IN_SEND_STALL  | IN_SENT_STALL), CLEAR);               
    }
    else{ // Out Endpoint
        // Must Clear both Send and Sent Stall
        SetClearIndexedReg(pContext, dwEndpoint, OUT_CSR1_REG_OFFSET,
            ( OUT_SEND_STALL | OUT_CLR_DATA_TOGGLE ), SET);
        SetClearIndexedReg(pContext ,dwEndpoint, OUT_CSR1_REG_OFFSET,
            ( OUT_SEND_STALL  | OUT_SENT_STALL), CLEAR);
    }
    
    UNLOCK_ENDPOINT(peps);
    FUNCTION_LEAVE_MSG();

    return ERROR_SUCCESS;
}


// Initialize an endpoint.
DWORD
WINAPI
UfnPdd_InitEndpoint(
    PVOID                       pvPddContext,
    DWORD                       dwEndpoint,
    UFN_BUS_SPEED               Speed,
    PUSB_ENDPOINT_DESCRIPTOR    pEndpointDesc,
    PVOID                       pvReserved,
    BYTE                        bConfigurationValue,
    BYTE                        bInterfaceNumber,
    BYTE                        bAlternateSetting
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    DEBUGCHK(EP_VALID(dwEndpoint));
    PREFAST_DEBUGCHK(pEndpointDesc);

#ifdef DEBUG
    {
        USB_ENDPOINT_DESCRIPTOR EndpointDesc;
        memcpy(&EndpointDesc, pEndpointDesc, sizeof(EndpointDesc));
        DEBUGCHK(UfnPdd_IsEndpointSupportable(pvPddContext, dwEndpoint, Speed,
            &EndpointDesc, bConfigurationValue, bInterfaceNumber, 
            bAlternateSetting) == ERROR_SUCCESS);
        DEBUGCHK(memcmp(&EndpointDesc, pEndpointDesc, sizeof(EndpointDesc)) == 0);
    }
#endif

    PCTRLR_PDD_CONTEXT pContext = (PCTRLR_PDD_CONTEXT) pvPddContext;
    ValidateContext(pContext);
    BYTE bEndpointAddress = 0;

    PEP_STATUS peps = GetEpStatus(pContext, dwEndpoint);
    DEBUGCHK(!peps->fInitialized);

    InitializeCriticalSection(&peps->cs);

    WORD wMaxPacketSize = 
        pEndpointDesc->wMaxPacketSize & USB_ENDPOINT_MAX_PACKET_SIZE_MASK;
    DEBUGCHK(wMaxPacketSize);
    
    // If the target is endpoint 0, then only allow the function driver 
    // to register a notification function.
    if (dwEndpoint == 0) {
       peps->dwPacketSizeAssigned = wMaxPacketSize;
       // Interrupts for endpoint 0 are enabled in ISTMain
    }
    else if (dwEndpoint < ENDPOINT_COUNT) {
        // Clear all Status bits and leave the Endpoint interrupt disabled
        // Clear Fifos, and all register bits
        ResetEndpoint(pContext,peps);

        // Clear all bits in CSR2 - Disable DMA for now...
        WriteIndexedReg(pContext, dwEndpoint, IN_CSR2_REG_OFFSET, 0); 

        // Setup Direction (mode_in bit) 
        bEndpointAddress = pEndpointDesc->bEndpointAddress;
        BOOL fModeOut = USB_ENDPOINT_DIRECTION_OUT(bEndpointAddress);
        if (fModeOut) {
            SetClearIndexedReg(pContext, dwEndpoint,IN_CSR2_REG_OFFSET, 
              SET_MODE_IN, CLEAR);
			peps->dwDirectionAssigned = USB_OUT_TRANSFER;
        }
        else {
            SetClearIndexedReg(pContext, dwEndpoint, IN_CSR2_REG_OFFSET, 
                SET_MODE_IN, SET);
            peps->dwDirectionAssigned = USB_IN_TRANSFER;
        }

        // Set Transfer Type
        BYTE bTransferType = pEndpointDesc->bmAttributes & USB_ENDPOINT_TYPE_MASK;
        DEBUGCHK(bTransferType != USB_ENDPOINT_TYPE_CONTROL);
        switch(bTransferType) {
            case USB_ENDPOINT_TYPE_ISOCHRONOUS:
                // Set the ISO bit
                SetClearIndexedReg(pContext, dwEndpoint,IN_CSR2_REG_OFFSET, 
                    SET_TYPE_ISO, SET);
                break;

            case USB_ENDPOINT_TYPE_BULK:
            case USB_ENDPOINT_TYPE_INTERRUPT:
            default:
                // Clear ISO bit - Set type to Bulk
                SetClearIndexedReg(pContext, dwEndpoint,IN_CSR2_REG_OFFSET, 
                    SET_TYPE_ISO, CLEAR);
        }

        peps->dwEndpointType = bTransferType;
        peps->dwPacketSizeAssigned = wMaxPacketSize;
        
        // Set the Max Packet Size Register
        BYTE maxPacketBits = (BYTE) (peps->dwPacketSizeAssigned >> 3);
        WriteIndexedReg(pContext, dwEndpoint, MAX_PKT_SIZE_REG_OFFSET, 
            maxPacketBits); 

        UfnPdd_ClearEndpointStall(pvPddContext,dwEndpoint);

        // Clear outstanding interrupts
        ClearEndpointInterrupt(pContext, dwEndpoint);
    }
    
    peps->fInitialized = TRUE;
    FUNCTION_LEAVE_MSG();

    return ERROR_SUCCESS;
}


// Deinitialize an endpoint.
DWORD
WINAPI
UfnPdd_DeinitEndpoint(
                      PVOID pvPddContext,
                      DWORD dwEndpoint
                      )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    DEBUGCHK(EP_VALID(dwEndpoint));

    PCTRLR_PDD_CONTEXT pContext = (PCTRLR_PDD_CONTEXT) pvPddContext;
    ValidateContext(pContext);

    PEP_STATUS peps = GetEpStatus(pContext, dwEndpoint);
    LOCK_ENDPOINT(peps);

    DEBUGCHK(peps->fInitialized);
    DEBUGCHK(peps->pTransfer == NULL);

    // Reset and disable the endpoint 
    // Mask endpoint interrupts    
    ResetEndpoint(pContext, peps);

    // Clear endpoint interrupts
    ClearEndpointInterrupt(pContext, dwEndpoint);

    peps->fInitialized = FALSE;
    UNLOCK_ENDPOINT(peps);

    DeleteCriticalSection(&peps->cs);

    FUNCTION_LEAVE_MSG();

    return ERROR_SUCCESS;
}


// Stall an endpoint.
DWORD
WINAPI
UfnPdd_StallEndpoint(
                     PVOID pvPddContext,
                     DWORD dwEndpoint
                     )
{
    DWORD dwRet = ERROR_SUCCESS;
    SETFNAME();
    FUNCTION_ENTER_MSG();

    DEBUGCHK(EP_VALID(dwEndpoint));

    PCTRLR_PDD_CONTEXT pContext = (PCTRLR_PDD_CONTEXT) pvPddContext;
    ValidateContext(pContext);

    PEP_STATUS peps = GetEpStatus(pContext, dwEndpoint);
    DEBUGCHK(peps->fInitialized);
    LOCK_ENDPOINT(peps);

    if (dwEndpoint == 0) {
        // Must Clear Out Packet Ready when sending Stall
        BYTE bEp0StallBits = (DATA_END | SERVICED_OUT_PKY_RDY | EP0_SEND_STALL);
		DEBUGMSG(ZONE_SEND, (_T("%s Writing 0x%02x to EP0_CSR_REG\r\n"), pszFname,
			bEp0StallBits));
        WriteIndexedReg(pContext, 0, EP0_CSR_REG_OFFSET, bEp0StallBits);
        // Set Flag so that SendControlStatusHandshked does not
        // duplicate this HW Write. Manual says all bits need
        // to be set at the same time.
        pContext->sendDataEnd = FALSE;
        pContext->Ep0State = EP0_STATE_IDLE;
    }
    else if (peps->dwDirectionAssigned == USB_IN_TRANSFER) {
        SetClearIndexedReg(pContext, dwEndpoint, IN_CSR1_REG_OFFSET,
            (IN_SEND_STALL), SET);
    }
    else { // Out Endpoint
        // Must Clear Out Packet Ready when sending Stall
        SetClearIndexedReg(pContext, dwEndpoint, OUT_CSR1_REG_OFFSET,
            (OUT_SEND_STALL), SET);
        SetClearIndexedReg(pContext ,dwEndpoint, OUT_CSR1_REG_OFFSET,
            (OUT_PACKET_READY), CLEAR);
    }
    
    UNLOCK_ENDPOINT(peps);
    FUNCTION_LEAVE_MSG();

    return ERROR_SUCCESS;
}


// Send the control status handshake.
DWORD
WINAPI
UfnPdd_SendControlStatusHandshake(
    PVOID           pvPddContext,
    DWORD           dwEndpoint
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    PCTRLR_PDD_CONTEXT pContext = (PCTRLR_PDD_CONTEXT) pvPddContext;
    ValidateContext(pContext);

    DEBUGCHK(dwEndpoint == 0);
    
    // This function is only valid for Endpoint 0
    EP_STATUS *peps = GetEpStatus(pContext, 0);
    DEBUGCHK(peps->fInitialized);

    // Remove the Out Packet Ready Condition
    if(pContext->sendDataEnd) {
		LOCK_ENDPOINT(peps);

        DEBUGMSG(ZONE_SEND, (_T("%s Sending 0 packet \r\n"), pszFname));

        BYTE bEP0IrqStatus = ReadIndexedReg(pContext, 0, EP0_CSR_REG_OFFSET);

        // Write 0 to SEND_STALL and SENT_STALL to clear them, so we need to 
        // leave them unchanged by default.
        BYTE bEp0CsrToWrite = (bEP0IrqStatus & EP0_STALL_BITS);

        bEp0CsrToWrite |= (DATA_END | SERVICED_OUT_PKY_RDY);

		DEBUGMSG(ZONE_SEND, (_T("%s Status - 0x%02x, Writing 0x%02x\r\n"), pszFname,
			bEP0IrqStatus, bEp0CsrToWrite));

        WriteIndexedReg(pContext, 0, EP0_CSR_REG_OFFSET, bEp0CsrToWrite);
        
        pContext->sendDataEnd = FALSE;

		UNLOCK_ENDPOINT(peps);
    } 

    FUNCTION_LEAVE_MSG();

    return ERROR_SUCCESS;
}


// Set the address of the device on the USB.
DWORD
WINAPI
UfnPdd_SetAddress(
                  PVOID pvPddContext,
                  BYTE  bAddress
                  )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    PCTRLR_PDD_CONTEXT pContext = (PCTRLR_PDD_CONTEXT) pvPddContext;
    ValidateContext(pContext);

    // Make sure that the Address Update bit is set (0x80)
    WriteReg(pContext, SET_ADDRESS_REG_OFFSET, (0x80 | bAddress));

    // for check whether usb cable is attached or not attached.
    // if isattachedusbcable is true, usb cable is attached.
	if (bAddress==0)   
		pContext->isattachedusbcable = FALSE;
	else
		pContext->isattachedusbcable = TRUE;

    FUNCTION_LEAVE_MSG();

    return ERROR_SUCCESS;
}


// Is endpoint Stalled?
DWORD
WINAPI
UfnPdd_IsEndpointHalted(
    PVOID pvPddContext,
    DWORD dwEndpoint,
    PBOOL pfHalted
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    DWORD dwRet = ERROR_SUCCESS;
    BYTE bRegVal;
    BOOL fHalted = FALSE;

    DEBUGCHK(EP_VALID(dwEndpoint));
    PREFAST_DEBUGCHK(pfHalted);

    PCTRLR_PDD_CONTEXT pContext = (PCTRLR_PDD_CONTEXT) pvPddContext;
    ValidateContext(pContext);

    PEP_STATUS peps = GetEpStatus(pContext, dwEndpoint);
    LOCK_ENDPOINT(peps);

    // Check the Appropriate Stall Bit
    if (dwEndpoint == 0) {
        // Must Clear Out Packet Ready when sending Stall
        bRegVal = ReadIndexedReg(pContext,dwEndpoint, EP0_CSR_REG_OFFSET);
        if (bRegVal & EP0_SEND_STALL)
            fHalted = TRUE;            
    }
    else if (peps->dwDirectionAssigned == USB_IN_TRANSFER) {
        bRegVal = ReadIndexedReg(pContext,dwEndpoint, IN_CSR1_REG_OFFSET);
        if (bRegVal & IN_SEND_STALL)
            fHalted = TRUE;
    }
    else{ // Out Endpoint
        bRegVal = ReadIndexedReg(pContext,dwEndpoint, OUT_CSR1_REG_OFFSET);
        if (bRegVal & OUT_SEND_STALL)
            fHalted = TRUE;
    }

    *pfHalted = fHalted;
    pContext->sendDataEnd  = FALSE;
    UNLOCK_ENDPOINT(peps);

    FUNCTION_LEAVE_MSG();

    return dwRet;
}


DWORD
WINAPI
UfnPdd_InitiateRemoteWakeup(
    PVOID pvPddContext
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    PCTRLR_PDD_CONTEXT pContext = (PCTRLR_PDD_CONTEXT) pvPddContext;
    ValidateContext(pContext);

    SetClearReg(pContext, PWR_REG_OFFSET, MCU_RESUME, SET);

    FUNCTION_LEAVE_MSG();

    return ERROR_SUCCESS;
}


DWORD
WINAPI
UfnPdd_RegisterDevice(
    PVOID                           pvPddContext,
    PCUSB_DEVICE_DESCRIPTOR         pHighSpeedDeviceDesc,
    PCUFN_CONFIGURATION             pHighSpeedConfig,
    PCUSB_CONFIGURATION_DESCRIPTOR  pHighSpeedConfigDesc,
    PCUSB_DEVICE_DESCRIPTOR         pFullSpeedDeviceDesc,
    PCUFN_CONFIGURATION             pFullSpeedConfig,
    PCUSB_CONFIGURATION_DESCRIPTOR  pFullSpeedConfigDesc,
    PCUFN_STRING_SET                pStringSets,
    DWORD                           cStringSets
    )
{
    PCTRLR_PDD_CONTEXT pContext = (PCTRLR_PDD_CONTEXT) pvPddContext;
    ValidateContext(pContext);
    
    // Nothing to do.

    return ERROR_SUCCESS;
}


DWORD
WINAPI
UfnPdd_DeregisterDevice(
    PVOID   pvPddContext
    )
{
    PCTRLR_PDD_CONTEXT pContext = (PCTRLR_PDD_CONTEXT) pvPddContext;
    ValidateContext(pContext);

    return ERROR_SUCCESS;
}


VOID
WINAPI
UfnPdd_PowerDown(
    PVOID pvPddContext
    )
{
    SETFNAME();
    DEBUGMSG(ZONE_POWER, (_T("%s\r\n"), pszFname));
    
    PCTRLR_PDD_CONTEXT pContext = (PCTRLR_PDD_CONTEXT) pvPddContext;
    ValidateContext(pContext);

    // Nothing to do.
}


VOID
WINAPI
UfnPdd_PowerUp(
    PVOID pvPddContext
    )
{
    SETFNAME();
    DEBUGMSG(ZONE_POWER, (_T("%s\r\n"), pszFname));
    
    PCTRLR_PDD_CONTEXT pContext = (PCTRLR_PDD_CONTEXT) pvPddContext;
    ValidateContext(pContext);

    // Nothing to do.
}


DWORD
WINAPI
UfnPdd_IOControl(
                 PVOID           pvPddContext,
                 IOCTL_SOURCE    source,
                 DWORD           dwCode,
                 PBYTE           pbIn,
                 DWORD           cbIn,
                 PBYTE           pbOut,
                 DWORD           cbOut,
                 PDWORD          pcbActualOut
                 )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    PCTRLR_PDD_CONTEXT pContext = (PCTRLR_PDD_CONTEXT) pvPddContext;
    ValidateContext(pContext);

    DWORD dwRet = ERROR_INVALID_PARAMETER;

    switch (dwCode) {
    case IOCTL_UFN_GET_PDD_INFO:
        if ( source != BUS_IOCTL || pbOut == NULL || 
             cbOut != sizeof(UFN_PDD_INFO) ) {
            break;
        }

        // Not currently supported.
        break;

    case IOCTL_BUS_GET_POWER_STATE:
        if (source == MDD_IOCTL) {
            PREFAST_DEBUGCHK(pbIn);
            DEBUGCHK(cbIn == sizeof(CE_BUS_POWER_STATE));

            PCE_BUS_POWER_STATE pCePowerState = (PCE_BUS_POWER_STATE) pbIn;
            PREFAST_DEBUGCHK(pCePowerState->lpceDevicePowerState);

            DEBUGMSG(ZONE_POWER, (_T("%s IOCTL_BUS_GET_POWER_STATE\r\n"), pszFname));

            *pCePowerState->lpceDevicePowerState = pContext->cpsCurrent;

            dwRet = ERROR_SUCCESS;
        }
        break;

    case IOCTL_BUS_SET_POWER_STATE:
        if (source == MDD_IOCTL) {
            PREFAST_DEBUGCHK(pbIn);
            DEBUGCHK(cbIn == sizeof(CE_BUS_POWER_STATE));

            PCE_BUS_POWER_STATE pCePowerState = (PCE_BUS_POWER_STATE) pbIn;

            PREFAST_DEBUGCHK(pCePowerState->lpceDevicePowerState);
            DEBUGCHK(VALID_DX(*pCePowerState->lpceDevicePowerState));

            DEBUGMSG(ZONE_POWER, (_T("%s IOCTL_BUS_GET_POWER_STATE(D%u)\r\n"), 
                pszFname, *pCePowerState->lpceDevicePowerState));

            SetPowerState(pContext, *pCePowerState->lpceDevicePowerState);

            dwRet = ERROR_SUCCESS;
        }
        break;
    }

    FUNCTION_LEAVE_MSG();

    return dwRet;
}


// Initialize the device.
DWORD
WINAPI
UfnPdd_Init(
    LPCTSTR                     pszActiveKey,
    PVOID                       pvMddContext,
    PUFN_MDD_INTERFACE_INFO     pMddInterfaceInfo,
    PUFN_PDD_INTERFACE_INFO     pPddInterfaceInfo
    )
{    
    SETFNAME();
    FUNCTION_ENTER_MSG();

    static const UFN_PDD_INTERFACE_INFO sc_PddInterfaceInfo = {
        UFN_PDD_INTERFACE_VERSION,
        UFN_PDD_CAPS_SUPPORTS_FULL_SPEED,
        ENDPOINT_COUNT,
        NULL, // This gets filled in later
        
        &UfnPdd_Deinit,
        &UfnPdd_IsConfigurationSupportable,
        &UfnPdd_IsEndpointSupportable,
        &UfnPdd_InitEndpoint,
        &UfnPdd_RegisterDevice,
        &UfnPdd_DeregisterDevice,
        &UfnPdd_Start,
        &UfnPdd_Stop,
        &UfnPdd_IssueTransfer,
        &UfnPdd_AbortTransfer,
        &UfnPdd_DeinitEndpoint,
        &UfnPdd_StallEndpoint,
        &UfnPdd_ClearEndpointStall,
        &UfnPdd_SendControlStatusHandshake,
        &UfnPdd_SetAddress,
        &UfnPdd_IsEndpointHalted,
        &UfnPdd_InitiateRemoteWakeup,
        &UfnPdd_PowerDown,
        &UfnPdd_PowerUp,
        &UfnPdd_IOControl,
    };

    DWORD dwType;
    DWORD dwRet;

    HKEY hkDevice = NULL;
    HKEY hKey = NULL;
    PCTRLR_PDD_CONTEXT pContext = NULL;

    DEBUGCHK(pszActiveKey);
    DEBUGCHK(pMddInterfaceInfo);
    DEBUGCHK(pPddInterfaceInfo);

    hkDevice = OpenDeviceKey(pszActiveKey);
    if (!hkDevice) {
        dwRet = GetLastError();
        DEBUGMSG(ZONE_ERROR, (_T("%s Could not open device key. Error: %d\r\n"), 
            pszFname, dwRet));
        goto EXIT;        
    }
    
    DWORD dwTmp;
    WCHAR Data[20];
    DWORD cbData;
    cbData = sizeof( Data );
    
    dwTmp = RegOpenKeyEx( HKEY_LOCAL_MACHINE, DRIVER_USB_KEY, 0, 0, &hKey );
    dwType = REG_SZ;
    USBClassInfo = USB_RNDIS; // RNDIS is default class

    if( dwTmp == ERROR_SUCCESS )
    {        
        dwTmp = RegQueryValueEx( hKey, DRIVER_USB_VALUE, NULL, &dwType, (LPBYTE)Data, &cbData );
        DEBUGMSG(ZONE_INIT, (_T("RegOpenKeyEx success %d, %d, %s, %d \r\n"),
        	dwTmp, dwType, Data, cbData ));
        if( dwTmp == ERROR_SUCCESS && dwType == REG_SZ )
        {
            DEBUGMSG(ZONE_INIT, (_T("RegQueryValueEx success \r\n")));
            if( wcscmp(Data, TEXT("RNDIS")) == 0 )
            {
                USBClassInfo = USB_RNDIS;
                DEBUGMSG(ZONE_INIT, (_T("USB RNDIS Function Class Enabled : %s \r\n"), Data));
            }
            else if( wcscmp(Data, TEXT("Serial_Class")) == 0 )
            {
            	USBClassInfo = USB_Serial;
                DEBUGMSG(ZONE_INIT, (_T("USB Serial Function Class Enabled : %s \r\n"), Data));
            }
            else if( wcscmp(Data, TEXT("Mass_Storage_Class")) == 0 )
            {
            	USBClassInfo = USB_MSF;
                DEBUGMSG(ZONE_INIT, (_T("USB MSF Function Class Enabled : %s \r\n"), Data));
            }
        }
        
        RegCloseKey( hKey );
    }
    
    

    pContext = (PCTRLR_PDD_CONTEXT) LocalAlloc(LPTR, sizeof(*pContext));
    if (pContext == NULL) {
        dwRet = GetLastError();
        PREFAST_DEBUGCHK(dwRet != ERROR_SUCCESS);
        DEBUGMSG(ZONE_ERROR, (_T("%s LocalAlloc failed. Error: %d\r\n"), pszFname, dwRet));
        goto EXIT;
    }
    pContext->dwSig = SC2440_SIG;

    pContext->pvMddContext = pvMddContext;
    pContext->cpsCurrent = D4;
    pContext->dwIrq = IRQ_UNSPECIFIED;
    pContext->pfnNotify = pMddInterfaceInfo->pfnNotify;
    InitializeCriticalSection(&pContext->csIndexedRegisterAccess);

    for (DWORD dwEp = 0; dwEp < dim(pContext->rgEpStatus); ++dwEp) {
        pContext->rgEpStatus[dwEp].dwEndpointNumber = dwEp;
    }

    DWORD dwDataSize;
    DWORD dwPriority;

    DDKISRINFO dii;
    DDKWINDOWINFO dwi;

    // read window configuration from the registry
    dwi.cbSize = sizeof(dwi);
    dwRet = DDKReg_GetWindowInfo(hkDevice, &dwi);
    if(dwRet != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_ERROR, (_T("%s DDKReg_GetWindowInfo() failed %d\r\n"), 
            pszFname, dwRet));
        goto EXIT;
    } 
    else if (dwi.dwNumIoWindows != 1) {
        DEBUGMSG(ZONE_ERROR, (_T("%s %d windows configured, expected 1\r\n"), 
            pszFname, dwi.dwNumIoWindows));
        dwRet = ERROR_INVALID_DATA;
        goto EXIT;
    } 
    else if (dwi.ioWindows[0].dwLen < REGISTER_SET_SIZE) {
        DEBUGMSG(ZONE_INIT, (_T("%s ioLen of 0x%x is less than required 0x%x\r\n"),
            pszFname, dwi.ioWindows[0].dwLen, REGISTER_SET_SIZE));
        dwRet = ERROR_INVALID_DATA;
        goto EXIT;        
    }
    else if (dwi.ioWindows[0].dwBase == 0){
        DEBUGMSG(ZONE_INIT, (_T("%s no ioBase value specified\r\n"),pszFname));
        dwRet = ERROR_INVALID_DATA;
        goto EXIT;
    }
    else {
        pContext->dwIOBase = dwi.ioWindows[0].dwBase;
        pContext->dwIOLen = dwi.ioWindows[0].dwLen;
    }
    
    // get ISR configuration information
    dii.cbSize = sizeof(dii);
    dwRet = DDKReg_GetIsrInfo(hkDevice, &dii);
    if (dwRet != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_ERROR, (_T("%s DDKReg_GetIsrInfo() failed %d\r\n"), 
            pszFname, dwRet));
        goto EXIT;
    }
    else if( (dii.dwSysintr == SYSINTR_NOP) && (dii.dwIrq == IRQ_UNSPECIFIED) ) {
        DEBUGMSG(ZONE_ERROR, (_T("%s no IRQ or SYSINTR value specified\r\n"), pszFname));
        dwRet = ERROR_INVALID_DATA;
        goto EXIT;
    } 
    else {
        if (dii.dwSysintr == SYSINTR_NOP) {
            BOOL fSuccess = KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &dii.dwIrq, 
                sizeof(DWORD), &dii.dwSysintr, sizeof(DWORD), NULL);
            if (!fSuccess) {
                DEBUGMSG(ZONE_ERROR, (_T("%s IOCTL_HAL_REQUEST_SYSINTR failed!\r\n"), 
                    pszFname));
                goto EXIT;
            }

            pContext->dwIrq = dii.dwIrq;
            pContext->dwSysIntr = dii.dwSysintr;
        }
        else {
            pContext->dwSysIntr = dii.dwSysintr;
        }
    }

    // Read the IST priority
    dwDataSize = sizeof(dwPriority);
    dwRet = RegQueryValueEx(hkDevice, UDC_REG_PRIORITY_VAL, NULL, &dwType,
        (LPBYTE) &dwPriority, &dwDataSize);
    if (dwRet != ERROR_SUCCESS) {
        dwPriority = DEFAULT_PRIORITY;
    }


    pContext->hBusAccess = CreateBusAccessHandle(pszActiveKey);
    if (pContext->hBusAccess == NULL) {
        // This is not a failure.
        DEBUGMSG(ZONE_WARNING, (_T("%s Could not create bus access handle\r\n"),
            pszFname));
    }

    DEBUGMSG(ZONE_INIT, (_T("%s Using IO Base %x\r\n"), 
        pszFname, pContext->dwIOBase));
    DEBUGMSG(ZONE_INIT, (_T("%s Using SysIntr %u\r\n"), 
        pszFname, pContext->dwSysIntr));
    DEBUGMSG(ZONE_INIT, (_T("%s Using IST priority %u\r\n"), 
        pszFname, dwPriority));

    pContext->dwISTPriority = dwPriority;

    // map register space to virtual memory
    dwRet = MapRegisterSet(pContext);
    if (dwRet != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_ERROR, (_T("%s failed to map register space\r\n"),
            pszFname));
        goto EXIT;
    }
    
    pContext->attachedState = UFN_DETACH;

    ResetDevice(pContext);
    ValidateContext(pContext);

    memcpy(pPddInterfaceInfo, &sc_PddInterfaceInfo, sizeof(sc_PddInterfaceInfo));
    pPddInterfaceInfo->pvPddContext = pContext;

EXIT:
    if (hkDevice) RegCloseKey(hkDevice);
    
    if (dwRet != ERROR_SUCCESS && pContext) {
        FreeCtrlrContext(pContext);
    }

    FUNCTION_LEAVE_MSG();

    return dwRet;
}


// Called by MDD's DllEntry.
extern "C" 
BOOL
UfnPdd_DllEntry(
    HANDLE hDllHandle,
    DWORD  dwReason, 
    LPVOID lpReserved
    )
{
    SETFNAME();

    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
            g_pUDCBase = NULL;
            break;
    }

    return TRUE;
}


