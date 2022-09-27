/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.
Copyright (c) 1995-1998  Microsoft Corporation

Module Name:

    interrupt.c

Abstract:

    This is a part of the driver for the Cirrus Logic CS8900
    Ethernet controller.  It contains the interrupt-handling routines.
    This driver conforms to the NDIS 3.0 interface.

    The overall structure and much of the code is taken from
    the Lance NDIS driver by Tony Ercolano.

--*/

#include "precomp.h"

//
// On debug builds tell the compiler to keep the symbols for
// internal functions, otw throw them out.
//
#if DBG
#define STATIC
#else
#define STATIC static
#endif


volatile S3C2440A_IOPORT_REG * v_pIOPregs;
extern unsigned long *EthCommand;
//
// This is used to pad short packets.
//
static UCHAR BlankBuffer[60] = "                                                            ";

void CS8900ReceiveEvent(PCS8900_ADAPTER Adapter, unsigned short RxEvent)
{
#if	WINCEDEBUG
	unsigned short i;
#endif
	unsigned short Length, Type;
	unsigned short *pBuffer;
	unsigned short *pBufferLimit;
	unsigned char  *cptr;
	WORD PacketOper;
	
	DEBUGMSG(0, (TEXT("++CS8900ReceiveEvent\r\n")));

	// Verify that it is an RxOK event
	if (!(RxEvent & RX_EVENT_RX_OK))
	{
		DEBUGMSG(1, (TEXT("CS8900ReceiveEvent: Receive Currupted Packet!\r\n")));
		return;
	}

	readIoPort(IO_RX_TX_DATA_0);				// Discard RxStatus
	Length = readIoPort(IO_RX_TX_DATA_0);

	pBuffer = (unsigned short *)Adapter->Lookahead;
	pBufferLimit = (unsigned short *)Adapter->Lookahead + (Length + 1) / 2;
	while (pBuffer < pBufferLimit)
	{
		*pBuffer = readIoPort(IO_RX_TX_DATA_0);
		pBuffer++;
	}

	pBuffer = (unsigned short *)Adapter->Lookahead;
	Type = pBuffer[6];
	PacketOper = pBuffer[10];

	if (Type == 0x0608)
	{
		if (PacketOper == 0x0100)
			DEBUGMSG(1, (TEXT("[CS8900] Receive ARP Request Packet\r\n")));
		else if (PacketOper == 0x0200)
			DEBUGMSG(1, (TEXT("[CS8900] Receive ARP Response Packet\r\n")));
		else if (PacketOper == 0x0300)
			DEBUGMSG(1, (TEXT("[CS8900] Receive RARP Request Packet\r\n")));
		else if (PacketOper == 0x0400)
			DEBUGMSG(1, (TEXT("[CS8900] Receive RARP Response Packet\r\n")));
		else
			DEBUGMSG(1, (TEXT("[CS8900] Receive Unknown ARP Packet\r\n")));
	}
	else if (Type == 0x0008)
		DEBUGMSG(1, (TEXT("[CS8900] Receive IP Packet\r\n")));

	cptr = (unsigned char *)Adapter->Lookahead;

#if	WINCEDEBUG
	DEBUGMSG(1, (TEXT("type = %x, length = %x\r\n"), Type, Length));
	for (i=0; i<Length; i++)
		DEBUGMSG(1, (TEXT("%x "), *cptr++));
	DEBUGMSG(1, (TEXT("\r\n")));
#endif

	cptr = (unsigned char *)Adapter->Lookahead;

	NdisMEthIndicateReceive(
		Adapter->MiniportAdapterHandle,
		(NDIS_HANDLE)Adapter,
		(PCHAR)(Adapter->Lookahead),
		CS8900_HEADER_SIZE,
		(PCHAR)(cptr)+CS8900_HEADER_SIZE,
		Length - CS8900_HEADER_SIZE,
		Length - CS8900_HEADER_SIZE);

	NdisMEthIndicateReceiveComplete(Adapter->MiniportAdapterHandle);

	return;
}

#if 0

VOID
CS8900EnableInterrupt(
    IN NDIS_HANDLE MiniportAdapterContext
    )

/*++

Routine Description:

    This routine is used to turn on the interrupt mask.

Arguments:

    Context - The adapter for the CS8900 to start.

Return Value:

    None.

--*/

{
    PCS8900_ADAPTER Adapter = (PCS8900_ADAPTER)(MiniportAdapterContext);

    DEBUGMSG(1, (TEXT("CS8900EnableInterrupt is called!!!\r\n")));

	v_pIOPRegs->rEINTMASK &= ~(1<<9);	// EINT9
}



VOID
CS8900DisableInterrupt(
    IN NDIS_HANDLE MiniportAdapterContext
    )

/*++

Routine Description:

    This routine is used to turn off the interrupt mask.

Arguments:

    Context - The adapter for the CS8900 to start.

Return Value:

    None.

--*/

{
    PCS8900_ADAPTER Adapter = (PCS8900_ADAPTER)(MiniportAdapterContext);

    DEBUGMSG(1, (TEXT("CS8900DisableInterrupt is called!!!\r\n")));

	v_pIOPRegs->rEINTMASK |= (1<<9);		// EINT9
}

#endif 

VOID
CS8900Isr(
    OUT PBOOLEAN InterruptRecognized,
    OUT PBOOLEAN QueueDpc,
    IN PVOID Context
    )

/*++

Routine Description:

    This is the interrupt handler which is registered with the operating
    system. If several are pending (i.e. transmit complete and receive),
    handle them all.  Block new interrupts until all pending interrupts
    are handled.

Arguments:

    InterruptRecognized - Boolean value which returns TRUE if the
        ISR recognizes the interrupt as coming from this adapter.

    QueueDpc - TRUE if a DPC should be queued.

    Context - pointer to the adapter object

Return Value:

    None.
--*/

{
#undef NODPC
#ifdef NODPC
	unsigned short Event;
#endif
    PCS8900_ADAPTER Adapter = ((PCS8900_ADAPTER)Context);

    *InterruptRecognized = TRUE;
#ifdef NODPC
	*QueueDpc = FALSE;
#else
	*QueueDpc = TRUE;
#endif

#ifdef NODPC
	Event = CS8900ReadRegister(PKTPG_ISQ);

	DEBUGMSG(1, (TEXT("++CS8900Isr event=%x\r\n"), Event));

	while (Event != 0)
	{
		switch (Event & REG_NUM_MASK)
		{
			case REG_NUM_RX_EVENT:
				DEBUGMSG(1, (TEXT("RX\r\n")));
				CS8900ReceiveEvent(Adapter, Event);
				break;

			case REG_NUM_TX_EVENT:
				DEBUGMSG(1, (TEXT("TX\r\n")));
				break;

			case REG_NUM_BUF_EVENT:
				DEBUGMSG(1, (TEXT("BUF\r\n")));
				break;
				
			case REG_NUM_RX_MISS:
				DEBUGMSG(1, (TEXT("CS8900Isr:RX_MISS!\r\n")));
				break;			
				
			case REG_NUM_TX_COL:
				break;
		}
		
		Event = CS8900ReadRegister(PKTPG_ISQ);

		DEBUGMSG(1, (TEXT("event=%x\r\n"), Event));
	}
#endif
    DEBUGMSG(1, (TEXT("CS8900Isr is called!!!\r\n")));
}


VOID
CS8900HandleInterrupt(
    IN NDIS_HANDLE MiniportAdapterContext
    )
/*++

Routine Description:

    This is the defered processing routine for interrupts.  It
    reads from the Interrupt Status Register any outstanding
    interrupts and handles them.

Arguments:

    MiniportAdapterContext - a handle to the adapter block.

Return Value:

    NONE.

--*/
{
    //
    // The adapter to process
    //
    PCS8900_ADAPTER Adapter = ((PCS8900_ADAPTER)MiniportAdapterContext);

	unsigned short Event;

	Event = CS8900ReadRegister(PKTPG_ISQ);

	DEBUGMSG(1, (TEXT("++CS8900HandleInterrupt event=%x\r\n"), Event));

	while (Event != 0)
	{
		switch (Event & REG_NUM_MASK)
		{
			case REG_NUM_RX_EVENT:
				DEBUGMSG(1, (TEXT("RX\r\n")));
				CS8900ReceiveEvent(Adapter, Event);
				break;

			case REG_NUM_TX_EVENT:
				DEBUGMSG(1, (TEXT("TX\r\n")));
				break;

			case REG_NUM_BUF_EVENT:
				DEBUGMSG(1, (TEXT("BUF\r\n")));
				break;
				
			case REG_NUM_RX_MISS:
				DEBUGMSG(1, (TEXT("CS8900HandleInterrupt:RX_MISS!\r\n")));
				break;			
				
			case REG_NUM_TX_COL:
				break;
		}
		
		Event = CS8900ReadRegister(PKTPG_ISQ);

		DEBUGMSG(1, (TEXT("event=%x\r\n"), Event));
	}

	DEBUGMSG(1, (TEXT("--CS8900HandleInterrupt\r\n")));
}


NDIS_STATUS
CS8900TransferData(
    OUT PNDIS_PACKET Packet,
    OUT PUINT BytesTransferred,
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_HANDLE MiniportReceiveContext,
    IN UINT ByteOffset,
    IN UINT BytesToTransfer
    )

/*++

Routine Description:

    A protocol calls the CS8900TransferData request (indirectly via
    NdisTransferData) from within its Receive event handler
    to instruct the driver to copy the contents of the received packet
    a specified packet buffer.

Arguments:

    MiniportAdapterContext - Context registered with the wrapper, really
        a pointer to the adapter.

    MiniportReceiveContext - The context value passed by the driver on its call
    to NdisMEthIndicateReceive.  The driver can use this value to determine
    which packet, on which adapter, is being received.

    ByteOffset - An unsigned integer specifying the offset within the
    received packet at which the copy is to begin.  If the entire packet
    is to be copied, ByteOffset must be zero.

    BytesToTransfer - An unsigned integer specifying the number of bytes
    to copy.  It is legal to transfer zero bytes; this has no effect.  If
    the sum of ByteOffset and BytesToTransfer is greater than the size
    of the received packet, then the remainder of the packet (starting from
    ByteOffset) is transferred, and the trailing portion of the receive
    buffer is not modified.

    Packet - A pointer to a descriptor for the packet storage into which
    the MAC is to copy the received packet.

    BytesTransfered - A pointer to an unsigned integer.  The MAC writes
    the actual number of bytes transferred into this location.  This value
    is not valid if the return status is STATUS_PENDING.

Notes:

  - The MacReceiveContext will be a pointer to the open block for
    the packet.

--*/

{
    //
    // The adapter to transfer from.
    //
    PCS8900_ADAPTER Adapter = ((PCS8900_ADAPTER)MiniportReceiveContext);

	DEBUGMSG(1, (TEXT("+CS8900:CS8900TransferData\r\n")));
	while(1);

    return(NDIS_STATUS_SUCCESS);
}


NDIS_STATUS
CS8900Send(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN PNDIS_PACKET Packet,
    IN UINT Flags
    )

/*++

Routine Description:


    The CS8900Send request instructs a driver to transmit a packet through
    the adapter onto the medium.

Arguments:

    MiniportAdapterContext - Context registered with the wrapper, really
        a pointer to the adapter.

    Packet - A pointer to a descriptor for the packet that is to be
    transmitted.

    SendFlags - Optional send flags

Notes:

    This miniport driver will always accept a send.  This is because
    the CS8900 has limited send resources and the driver needs packets
    to copy to the adapter immediately after a transmit completes in
    order to keep the adapter as busy as possible.

    This is not required for other adapters, as they have enough
    resources to keep the transmitter busy until the wrapper submits
    the next packet.

--*/

{
	ULONG	Len;
	PUCHAR	CurBufAddress;
	UCHAR	TotalPacket[2048];

	PNDIS_BUFFER CurBuffer;
	
	UINT	i;
	UINT	Count = 0;
	UINT	CurBufLen;
	UINT	PacketLength;
	USHORT	BusStatus;

	PCS8900_ADAPTER Adapter = (PCS8900_ADAPTER)(MiniportAdapterContext);

	DEBUGMSG(1, (TEXT("CS8900Send\r\n")));	

	NdisQueryPacket(
		Packet,
		NULL,
		NULL,
		NULL,
		&Len
		);

	NdisQueryPacket(Packet, NULL, NULL, &CurBuffer, &PacketLength);
	NdisQueryBuffer(CurBuffer, (PVOID *)&CurBufAddress, &CurBufLen);

	for (i = 0; i < CurBufLen; i++)
		TotalPacket[Count++] = CurBufAddress[i];

	NdisGetNextBuffer(CurBuffer, &CurBuffer);
	while (CurBuffer && (CurBufLen != 0))
	{
		NdisQueryBuffer(CurBuffer, (PVOID *)&CurBufAddress, &CurBufLen);

		for (i = 0; i < CurBufLen; i++)
		{
			TotalPacket[Count++] = CurBufAddress[i];
		}

		NdisGetNextBuffer(CurBuffer, &CurBuffer);
	}

	// Request that a transmit be started
	BusStatus = CS8900RequestTransmit(PacketLength);
		
	// If there was an error with the transmit bid
	if (BusStatus & BUS_ST_TX_BID_ERR)
	{
		DEBUGMSG(1, (TEXT("##### BUS_ST_TX_BID_ERR #####\r\n")));
	}
	else if (BusStatus & BUS_ST_RDY4TXNOW)
	{
		// The chip is ready for transmission now
		// Copy the message to the chip to start the transmit
		CS8900CopyTxFrame((PCHAR)TotalPacket, PacketLength);
		return(NDIS_STATUS_SUCCESS);
	}
	return(NDIS_STATUS_FAILURE);
}
