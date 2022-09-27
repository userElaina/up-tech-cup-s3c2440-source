/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.
Copyright (c) 1995-1998	 Microsoft Corporation

Module Name:

    miniport.c

Abstract:

    This is the	main file for the CS8900 Ethernet controller.
    This driver	conforms to the	NDIS 3.0 miniport interface.

--*/

#include "precomp.h"


//
// On debug builds tell	the compiler to	keep the symbols for
// internal functions, otw throw them out.
//
#if DBG
#define	STATIC
#else
#define	STATIC static
#endif

extern int resetCS(void);
extern int initCS();

//
// Debugging definitions
//
#if DBG

//
// Default debug mode
//
ULONG Cs8900DebugFlag =	
		       CS8900_DEBUG_LOUD
		       | CS8900_DEBUG_VERY_LOUD
		       // | CS8900_DEBUG_LOG
		       // | CS8900_DEBUG_CHECK_DUP_SENDS
		       // | CS8900_DEBUG_TRACK_PACKET_LENS
		       // | CS8900_DEBUG_WORKAROUND1
		       // | CS8900_DEBUG_CARD_BAD
		       // | CS8900_DEBUG_CARD_TESTS 
		       ;

//
// Debug tracing defintions
//
#define	CS8900_LOG_SIZE	256
UCHAR Cs8900LogBuffer[CS8900_LOG_SIZE]={0};
UINT Cs8900LogLoc = 0;

extern
VOID
Cs8900Log(UCHAR	c) {

    Cs8900LogBuffer[Cs8900LogLoc++] = c;

	Cs8900LogBuffer[(Cs8900LogLoc +	4) % CS8900_LOG_SIZE] =	'\0';

	if (Cs8900LogLoc >= CS8900_LOG_SIZE)
		Cs8900LogLoc = 0;
}

#endif

volatile unsigned char*	ioPacketPage;
volatile struct	gpioreg	*gpioRegs;
volatile unsigned long *v_pBlank, *v_pPort2;
volatile PLONG		v_pGpioRegs, v_pIcRegs;

//
// This	constant is used for places where NdisAllocateMemory
// needs to be called and the HighestAcceptableAddress does
// not matter.
//
NDIS_PHYSICAL_ADDRESS HighestAcceptableMax =
    NDIS_PHYSICAL_ADDRESS_CONST(-1,-1);

//
// The global Miniport driver block.
//

DRIVER_BLOCK CS8900MiniportBlock={0};

//
// List	of supported OID for this driver.
//
STATIC UINT CS8900SupportedOids[] = {
    OID_GEN_SUPPORTED_LIST,
    OID_GEN_HARDWARE_STATUS,
    OID_GEN_MEDIA_SUPPORTED,
    OID_GEN_MEDIA_IN_USE,
    OID_GEN_MAXIMUM_LOOKAHEAD,
    OID_GEN_MAXIMUM_FRAME_SIZE,
    OID_GEN_MAXIMUM_TOTAL_SIZE,
    OID_GEN_MAC_OPTIONS,
    OID_GEN_PROTOCOL_OPTIONS,
    OID_GEN_LINK_SPEED,
    OID_GEN_TRANSMIT_BUFFER_SPACE,
    OID_GEN_RECEIVE_BUFFER_SPACE,
    OID_GEN_TRANSMIT_BLOCK_SIZE,
    OID_GEN_RECEIVE_BLOCK_SIZE,
    OID_GEN_VENDOR_DESCRIPTION,
    OID_GEN_VENDOR_ID,
    OID_GEN_DRIVER_VERSION,
    OID_GEN_CURRENT_PACKET_FILTER,
    OID_GEN_CURRENT_LOOKAHEAD,
    OID_GEN_XMIT_OK,
    OID_GEN_RCV_OK,
    OID_GEN_XMIT_ERROR,
    OID_GEN_RCV_ERROR,
    OID_GEN_RCV_NO_BUFFER,
    OID_802_3_PERMANENT_ADDRESS,
    OID_802_3_CURRENT_ADDRESS,
    OID_802_3_MULTICAST_LIST,
    OID_802_3_MAXIMUM_LIST_SIZE,
    OID_802_3_RCV_ERROR_ALIGNMENT,
    OID_802_3_XMIT_ONE_COLLISION,
    OID_802_3_XMIT_MORE_COLLISIONS,
    
    OID_GEN_MEDIA_CONNECT_STATUS,
    OID_GEN_MAXIMUM_SEND_PACKETS,
    OID_GEN_VENDOR_DRIVER_VERSION,
    };

//
// Determines whether failing the initial card test will prevent
// the adapter from being registered.
//
#ifdef CARD_TEST

BOOLEAN	InitialCardTest	= TRUE;

#else  // CARD_TEST

BOOLEAN	InitialCardTest	= FALSE;

#endif // CARD_TEST

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

#pragma	NDIS_INIT_FUNCTION(DriverEntry)


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

/*++

Routine	Description:

    This is the	primary	initialization routine for the CS8900 driver.
    It is simply responsible for the intializing the wrapper and registering
    the	Miniport driver.  It then calls	a system and architecture specific
    routine that will initialize and register each adapter.

Arguments:

    DriverObject - Pointer to driver object created by the system.

    RegistryPath - Path	to the parameters for this driver in the registry.

Return Value:

    The	status of the operation.

--*/

{
    //
    // Receives	the status of the NdisMRegisterMiniport	operation.
    //
    NDIS_STATUS	Status;

    //
    // Characteristics table for this driver.
    //
    // NDIS_MINIPORT_CHARACTERISTICS CS8900Char;
	NDIS51_MINIPORT_CHARACTERISTICS CS8900Char;
    //
    // Pointer to the global information for this driver
    //
    PDRIVER_BLOCK NewDriver = &CS8900MiniportBlock;

    //
    // Handle for referring to the wrapper about this driver.
    //
    NDIS_HANDLE	NdisWrapperHandle;

    DEBUGMSG(1, (TEXT("+CS8900:DriverEntry\r\n")));
    
    RETAILMSG(1, (TEXT("+CS8900:DriverEntry\r\n")));
    
    //
    // Initialize the wrapper.
    //
    NdisMInitializeWrapper(
		&NdisWrapperHandle,
		DriverObject,
		RegistryPath,
		NULL
		);

    //
    // Save the	global information about this driver.
    //
    NewDriver->NdisWrapperHandle = NdisWrapperHandle;
    NewDriver->AdapterQueue = (PCS8900_ADAPTER)NULL;

    //
    // Initialize the Miniport characteristics for the call to
    // NdisMRegisterMiniport.
    //
    memset(&CS8900Char,0,sizeof(CS8900Char));
    CS8900Char.MajorNdisVersion	= CS8900_NDIS_MAJOR_VERSION;
    CS8900Char.MinorNdisVersion	= CS8900_NDIS_MINOR_VERSION;
    CS8900Char.CheckForHangHandler = NULL;
    //CS8900Char.DisableInterruptHandler = CS8900DisableInterrupt;
    //CS8900Char.EnableInterruptHandler =	CS8900EnableInterrupt;
	CS8900Char.DisableInterruptHandler = NULL;
    CS8900Char.EnableInterruptHandler =	NULL;
    CS8900Char.HaltHandler = CS8900Halt;
    CS8900Char.HandleInterruptHandler =	CS8900HandleInterrupt;
    CS8900Char.InitializeHandler = MiniportInitialize;
    CS8900Char.ISRHandler = CS8900Isr;
    CS8900Char.QueryInformationHandler = CS8900QueryInformation;
    CS8900Char.ReconfigureHandler = NULL;
    CS8900Char.ResetHandler = CS8900Reset;
    CS8900Char.SendHandler = CS8900Send;
    CS8900Char.SetInformationHandler = CS8900SetInformation;
    CS8900Char.TransferDataHandler = CS8900TransferData;

	CS8900Char.ReturnPacketHandler     = NULL;
	CS8900Char.SendPacketsHandler      = NULL;
	CS8900Char.AllocateCompleteHandler = NULL;
	CS8900Char.CancelSendPacketsHandler = CS8900CancelSendPackets;
	CS8900Char.AdapterShutdownHandler = CS8900AdapterShutdown;
	CS8900Char.PnPEventNotifyHandler = CS8900DevicePnPEvent;

    DEBUGMSG(1, (TEXT("CS8900:	-> NdisMRegisterMiniport\r\n")));

    Status = NdisMRegisterMiniport(
		 NdisWrapperHandle,
		 &CS8900Char,
		 sizeof(CS8900Char)
		 );

    if (Status == NDIS_STATUS_SUCCESS)
    {
		DEBUGMSG(1,
				 (TEXT("-CS8900:DriverEntry: Success!\r\n")));
		return STATUS_SUCCESS;
    }

	// Terminate the wrapper.
	NdisTerminateWrapper (CS8900MiniportBlock.NdisWrapperHandle, NULL);
	CS8900MiniportBlock.NdisWrapperHandle =	NULL;

    DEBUGMSG(1, (TEXT("-CS8900:DriverEntry: Fail!\r\n")));
    return STATUS_UNSUCCESSFUL;
}


#pragma	NDIS_PAGEABLE_FUNCTION(MiniportInitialize)
extern
NDIS_STATUS
MiniportInitialize(
    OUT	PNDIS_STATUS OpenErrorStatus,
    OUT	PUINT SelectedMediumIndex,
    IN PNDIS_MEDIUM MediumArray,
    IN UINT MediumArraySize,
    IN NDIS_HANDLE MiniportAdapterHandle,
    IN NDIS_HANDLE ConfigurationHandle
    )

/*++

Routine	Description:

    CS8900Initialize starts an adapter and registers resources with the
    wrapper.

Arguments:

    OpenErrorStatus - Extra status bytes for opening token ring	adapters.

    SelectedMediumIndex	- Index	of the media type chosen by the	driver.

    MediumArray	- Array	of media types for the driver to chose from.

    MediumArraySize - Number of	entries	in the array.

    MiniportAdapterHandle - Handle for passing to the wrapper when
       referring to this adapter.

    ConfigurationHandle	- A handle to pass to NdisOpenConfiguration.

Return Value:

    NDIS_STATUS_SUCCESS
    NDIS_STATUS_PENDING

--*/

{
    //
    // Pointer to our newly allocated adapter.
    //
    PCS8900_ADAPTER Adapter;

    //
    // The handle for reading from the registry.
    //
    NDIS_HANDLE	ConfigHandle;

    //
    // The value read from the registry.
    //
    PNDIS_CONFIGURATION_PARAMETER ReturnedValue;

    //
    // String names of all the parameters that will be read.
    //
    NDIS_STRING	IOAddressStr = IOADDRESS;
    NDIS_STRING	InterruptStr = INTERRUPT;
    NDIS_STRING	MaxMulticastListStr = MAX_MULTICAST_LIST;
    NDIS_STRING	NetworkAddressStr = NETWORK_ADDRESS;
    NDIS_STRING	BusTypeStr = NDIS_STRING_CONST("BusType");
    NDIS_STRING	CardTypeStr = NDIS_STRING_CONST("CardType");

    //
    // TRUE if there is	a configuration	error.
    //
    BOOLEAN ConfigError	= FALSE;

    //
    // A special value to log concerning the error.
    //
    ULONG ConfigErrorValue = 0;

    //
    // The slot	number the adapter is located in, used for
    // Microchannel adapters.
    //
    UINT SlotNumber = 0;

    //
    // TRUE if it is unnecessary to read the Io	Base Address
    // and Interrupt from the registry.	 Used for Microchannel
    // adapters, which get this	information from the slot
    // information.
    //
    BOOLEAN SkipIobaseAndInterrupt = FALSE;

    //
    // The network address the adapter should use instead of the
    // the default burned in address.
    //
    PVOID NetAddress;

    //
    // The number of bytes in the address.  It should be
    // CS8900_LENGTH_OF_ADDRESS
    //
    ULONG Length;

    //
    // These are used when calling CS8900RegisterAdapter.
    //

    //
    // The physical address of the base	I/O port.
    //
    PVOID IoBaseAddr;

    //
    // The interrupt number to use.
    //
    CCHAR InterruptNumber;

    //
    // The number of multicast address to be supported.
    //
    UINT MaxMulticastList;

    //
    // Temporary looping variable.
    //
    ULONG i;

    //
    // Status of Ndis calls.
    //
    NDIS_STATUS	Status;

//    NDIS_MCA_POS_DATA	McaData;

	DEBUGMSG(1, (TEXT("+CS8900:CS8900Initialize\r\n")));

    //
    // Search for the medium type (802.3) in the given array.
    //
    for	(i = 0;	i < MediumArraySize; i++)
    {
		if (MediumArray[i] == NdisMedium802_3)
		{
		    break;
		}
    }

    if (i == MediumArraySize)
    {
		DEBUGMSG(1, (TEXT("CS8900: No Supported Media!\r\n")));
		return(	NDIS_STATUS_UNSUPPORTED_MEDIA );
    }

    *SelectedMediumIndex = i;

    //
    // Set default values.
    //
    IoBaseAddr = DEFAULT_IOBASEADDR;
    InterruptNumber = DEFAULT_INTERRUPTNUMBER;
    MaxMulticastList = DEFAULT_MULTICASTLISTMAX;

    //
    // Allocate	memory for the adapter block now.
    //
    Status = NdisAllocateMemory( (PVOID	*)&Adapter,
		   sizeof(CS8900_ADAPTER),
		   0,
		   HighestAcceptableMax
		   );

    if (Status != NDIS_STATUS_SUCCESS)
    {
		DEBUGMSG(1,
	    	(TEXT("CS8900: NdisAllocateMemory(CS8900_ADAPTER) Fail!\r\n")));
		return Status;
    }

    //
    // Clear out the adapter block, which sets all default values to FALSE,
    // or NULL.
    //
    NdisZeroMemory (Adapter, sizeof(CS8900_ADAPTER));

    //
    // Open the	configuration space.
    //
    NdisOpenConfiguration(
	    &Status,
	    &ConfigHandle,
	    ConfigurationHandle
	    );

    if (Status != NDIS_STATUS_SUCCESS)
    {
		NdisFreeMemory(Adapter,	sizeof(CS8900_ADAPTER),	0);

		DEBUGMSG(1,
	 	   (TEXT("CS8900: NdisOpenconfiguration Fail! - 0x%x\n"),
	 	   Status));
		return Status;
    }

    //
    //	Read in	the card type.
    //
    NdisReadConfiguration(
	    &Status,
	    &ReturnedValue,
	    ConfigHandle,
	    &CardTypeStr,
	    NdisParameterHexInteger
	    );
	    
    if (Status == NDIS_STATUS_SUCCESS)
		Adapter->CardType = (UINT)ReturnedValue->ParameterData.IntegerData;

	//
	// Read net	address
	//
    NdisReadNetworkAddress(
		&Status,
		&NetAddress,
		&Length,
		ConfigHandle
		);

    if ((Length	== CS8900_LENGTH_OF_ADDRESS) &&
		(Status	== NDIS_STATUS_SUCCESS)) {

		//
		// Save	the address that should	be used.
		//
		NdisMoveMemory(
			Adapter->StationAddress,
			NetAddress,
			CS8900_LENGTH_OF_ADDRESS
			);

   	}

    //
    // Read Bus	Type (for NE2/AE2 support)
    //
    NdisReadConfiguration(
	   	&Status,
	    &ReturnedValue,
	    ConfigHandle,
		&BusTypeStr,
		NdisParameterHexInteger
		);

	if (Status == NDIS_STATUS_SUCCESS) {
		Adapter->BusType = (UCHAR)ReturnedValue->ParameterData.IntegerData;
	}

	if (!SkipIobaseAndInterrupt)
	{
		//
		// Read	I/O Address
		//
		NdisReadConfiguration(
			&Status,
			&ReturnedValue,
			ConfigHandle,
			&IOAddressStr,
			NdisParameterHexInteger
			);

		if (Status == NDIS_STATUS_SUCCESS) {
	    	IoBaseAddr = (PVOID)(ReturnedValue->ParameterData.IntegerData);
		}

		//
		// Read	interrupt number
		//
		NdisReadConfiguration(
			&Status,
			&ReturnedValue,
			ConfigHandle,
			&InterruptStr,
			NdisParameterInteger
			);

		if (Status == NDIS_STATUS_SUCCESS) {
	    	InterruptNumber = (CCHAR)(ReturnedValue->ParameterData.IntegerData);
		}
    }

    //
    // Read MaxMulticastList
	//
    NdisReadConfiguration(
		&Status,
	    &ReturnedValue,
	    ConfigHandle,
	    &MaxMulticastListStr,
	    NdisParameterInteger
	    );

    if (Status == NDIS_STATUS_SUCCESS) {
		MaxMulticastList = ReturnedValue->ParameterData.IntegerData;
		if (ReturnedValue->ParameterData.IntegerData <=	DEFAULT_MULTICASTLISTMAX)
			MaxMulticastList = ReturnedValue->ParameterData.IntegerData;
    }


	// RegisterAdapter:

    //
    // Now to use this information and register	with the wrapper
    // and initialize the adapter.
    //

    //
    // First close the configuration space.
    //
    NdisCloseConfiguration(ConfigHandle);

    DEBUGMSG(1, (TEXT("CS8900:Card type: 0x%x\r\n"), Adapter->CardType));
    DEBUGMSG(1, (TEXT("CS8900:I/O base	addr 0x%lx\r\n"), IoBaseAddr));
    DEBUGMSG(1, (TEXT("CS8900:interrupt number	%ld\r\n"),
	InterruptNumber));
    DEBUGMSG(1, (TEXT("CS8900:max multicast %ld\r\n"),
	DEFAULT_MULTICASTLISTMAX));
    DEBUGMSG(1, (TEXT("CS8900:attribute memory	address	0x%X\r\n"),
	Adapter->AttributeMemoryAddress));
    DEBUGMSG(1, (TEXT("CS8900:attribute memory	size 0x%X\r\n"),
	Adapter->AttributeMemorySize));
    DEBUGMSG(1, (TEXT("CS8900:Bus type: %d\r\n"), Adapter->BusType));

    //
    // Set up the parameters.
    //
    Adapter->IoBaseAddr	= IoBaseAddr;

	Adapter->InterruptNumber = InterruptNumber;

    Adapter->MulticastListMax =	MaxMulticastList;
    Adapter->MiniportAdapterHandle = MiniportAdapterHandle;

    Adapter->MaxLookAhead = CS8900_MAX_LOOKAHEAD;

    //
    // Now do the work.
    //
    if (CS8900RegisterAdapter(Adapter,
		ConfigurationHandle,
		ConfigError,
		ConfigErrorValue
		) != NDIS_STATUS_SUCCESS) {

		//
		// CS8900RegisterAdapter failed.
		//
		NdisFreeMemory(Adapter,	sizeof(CS8900_ADAPTER),	0);

		return NDIS_STATUS_FAILURE;

	}

    DEBUGMSG(1, (TEXT("-CS8900:CS8900Initialize Success!\r\n")));
    return NDIS_STATUS_SUCCESS;
}


#pragma	NDIS_PAGEABLE_FUNCTION(CS8900RegisterAdapter)
NDIS_STATUS
CS8900RegisterAdapter(
    IN PCS8900_ADAPTER Adapter,
    IN NDIS_HANDLE ConfigurationHandle,
    IN BOOLEAN ConfigError,
    IN ULONG ConfigErrorValue
    )

/*++

Routine	Description:

    Called when	a new adapter should be	registered. It allocates space for
    the	adapter, initializes the adapter's block, registers resources
    with the wrapper and initializes the physical adapter.

Arguments:

    Adapter - The adapter structure.

    ConfigurationHandle	- Handle passed	to CS8900Initialize.

    ConfigError	- Was there an error during configuration reading.

    ConfigErrorValue - Value to	log if there is	an error.

Return Value:

    Indicates the success or failure of	the registration.

--*/

{
    //
    // Temporary looping variable.
    //
    unsigned char *m_pCS8900IoPort;

    //
    // General purpose return from NDIS	calls
    //
    NDIS_STATUS	status;

	DEBUGMSG(1, (TEXT("+CS8900:CS8900RegisterAdapter\r\n")));

    //
    // Check for a configuration error
    //
    if (ConfigError)
    {
		//
		// Log Error and exit.
		//
		NdisWriteErrorLogEntry(
			Adapter->MiniportAdapterHandle,
			NDIS_ERROR_CODE_UNSUPPORTED_CONFIGURATION,
			1,
			ConfigErrorValue
			);
		DEBUGMSG(1,
			(TEXT("CS8900:RegisterAdapter: CS8900Initialize had	a config error %d\r\n"),
			ConfigErrorValue));

		return(NDIS_STATUS_FAILURE);
	}

    //
    // Inform the wrapper of the physical attributes of	this adapter.
    //
	DEBUGMSG(1, (TEXT("CS8900: -> NdisMSetAttributes\r\n")));
		
    NdisMSetAttributes(
		Adapter->MiniportAdapterHandle,
		(NDIS_HANDLE)Adapter,
		FALSE,
		Adapter->BusType
		);

	m_pCS8900IoPort	= VirtualAlloc(0, 0x1000, MEM_RESERVE, PAGE_NOACCESS);
	DEBUGMSG(1,
		(TEXT("[CS8900] VirtualAlloc of m_pCS8900IoPort returns %x\r\n"), m_pCS8900IoPort));

	if (!VirtualCopy(m_pCS8900IoPort, (LPVOID)((IO_PACKET_PAGE_BASE_ADDR) & 0xFFFFF000),
						0x1000,	PAGE_READWRITE|PAGE_NOCACHE) )
		DEBUGMSG(1, (TEXT("[CS8900] m_pCS8900IoPort Virtual Copy failed\r\n")));
	else
		DEBUGMSG(1, (TEXT("[CS8900] m_pCS8900IoPort Virtual Copy OK!\r\n")));


	ioPacketPage = (volatile unsigned char *)(m_pCS8900IoPort + 
		(IO_PACKET_PAGE_BASE_ADDR - ((IO_PACKET_PAGE_BASE_ADDR) & 0xFFFFF000)) );
	Adapter->IoPAddr = (unsigned long)ioPacketPage;

	DEBUGMSG(1, (TEXT("[CS8900] m_pCS8900IoPort= %x\r\n"), Adapter->IoPAddr));


/*    
    //
    // Register	the port addresses.
    //

	DEBUGMSG(1,
		(TEXT("CS8900: -> NdisMRegisterIoPortRange\r\n")));
    status = NdisMRegisterIoPortRange(
		 (PVOID	*)(&(Adapter->IoPAddr)),
		 Adapter->MiniportAdapterHandle,
		 (ULONG)Adapter->IoBaseAddr,
		 0x20
	     );

    if (status != NDIS_STATUS_SUCCESS)
    {
	DEBUGMSG(1,
			(TEXT("CS8900:NdisMRegisterIoPortRange unsuccessful!\r\n")));
	return(status);
	}     
*/
    if (Adapter->IoPAddr == 0)
    {
		DEBUGMSG(1, (TEXT("CS8900:Invalid IoPAddr!\r\n")));
		return NDIS_STATUS_FAILURE;
    }

    //
    // Initialize the card.
    //
    DEBUGMSG(1, (TEXT("+CS8900:CS8900Initialize\r\n")));

    if (!CS8900Initialize(Adapter))
    {
		DEBUGMSG(1, (TEXT("-CS8900:CS8900Initialize - Fail!\r\n")));

		NdisWriteErrorLogEntry(
	    	Adapter->MiniportAdapterHandle,
	    	NDIS_ERROR_CODE_ADAPTER_NOT_FOUND,
	    	0
			);

		status = NDIS_STATUS_ADAPTER_NOT_FOUND;

		goto fail2;
    }

    DEBUGMSG(1, (TEXT("-CS8900:CS8900Initialize	- Success!\r\n")));

    //
    // Read the	Ethernet address off of	the PROM.
    //
    if (!CS8900ReadEthernetAddress(Adapter))
    {
		DEBUGMSG(1, (TEXT("CS8900:RegisterAdapter Could	not read the ethernet address\n")));

		NdisWriteErrorLogEntry(
			Adapter->MiniportAdapterHandle,
	    	NDIS_ERROR_CODE_ADAPTER_NOT_FOUND,
			0
	    	);

		status = NDIS_STATUS_ADAPTER_NOT_FOUND;

		goto fail2;
    }

    //
    // Initialize the interrupt.
    //
#if 0
    status = NdisMRegisterInterrupt(
		 &Adapter->Interrupt,
		 Adapter->MiniportAdapterHandle,
		 Adapter->InterruptNumber,
		 Adapter->InterruptNumber,
		 FALSE,
		 FALSE,
		 NdisInterruptLatched
	     );
#else
    status = NdisMRegisterInterrupt(
		 &Adapter->Interrupt,
		 Adapter->MiniportAdapterHandle,
		 Adapter->InterruptNumber,
		 Adapter->InterruptNumber,
		 TRUE,
		 FALSE,
		 NdisInterruptLatched
	     );
#endif
    if (status != NDIS_STATUS_SUCCESS)
    {
		DEBUGMSG(1, (TEXT("CS8900:RegisterAdapter -> NdisMRegisterInterrupt failed 0x%x\r\n"), status));
		NdisWriteErrorLogEntry(
	    	Adapter->MiniportAdapterHandle,
	    	NDIS_ERROR_CODE_INTERRUPT_CONNECT,
	    	0
		);

		goto fail3;
    }

    DEBUGMSG(1,
	(TEXT("CS8900:RegisterAdapter Interrupt Connected\r\n")));

    // register	a shutdown handler for this card
	NdisMRegisterAdapterShutdownHandler(
		Adapter->MiniportAdapterHandle,		// miniport handle.
		Adapter,							// shutdown context.
		CS8900Shutdown						// shutdown handler.
		);
    
	/* Initialize The Interrupt */

	DEBUGMSG(1, (TEXT("CS8900RegisterAdapter:Initialize the Interrupt!\r\n")));

    //
    // Initialization completed	successfully.
    //
    DEBUGMSG(1, (TEXT("CS8900:RegisterAdapter OK\r\n")));

    return(NDIS_STATUS_SUCCESS);

    //
    // Code to unwind what has already been set	up when	a part of
    // initialization fails, which is jumped into at various
    // points based on where the failure occured. Jumping to
    // a higher-numbered failure point will execute the	code
    // for that	block and all lower-numbered ones.
    //

fail3:

    //
    // Take us out of the AdapterQueue.
    //

    if (CS8900MiniportBlock.AdapterQueue == Adapter)
    {
		CS8900MiniportBlock.AdapterQueue = Adapter->NextAdapter;
    }
    else
    {
		PCS8900_ADAPTER	TmpAdapter = CS8900MiniportBlock.AdapterQueue;

		while (TmpAdapter->NextAdapter != Adapter)
		{
			TmpAdapter = TmpAdapter->NextAdapter;
		}

		TmpAdapter->NextAdapter	= TmpAdapter->NextAdapter->NextAdapter;
    }

fail2:

    NdisMDeregisterIoPortRange(
		Adapter->MiniportAdapterHandle,
		(ULONG)Adapter->IoBaseAddr,
		0x20,
		(PVOID)Adapter->IoPAddr
    	);

    return(status);
}

extern
VOID
CS8900Shutdown(
    IN NDIS_HANDLE MiniportAdapterContext
    )
/*++

Routine	Description:

    CS8900Shutdown is called to	shut down the adapter. We need to unblock any
    threads which may be called	in, and	terminate any loops.  This function is
    called by the WinCE	NDIS.DLL when a	PCMCIA card removal is detected.  At that
    point, any access to the CS8900 registers may return random	data, as the bus
    is floating.

Arguments:

    MiniportAdapterContext - The context value that the	Miniport returned
	from CS8900Initialize; actually	as pointer to an CS8900_ADAPTER.

Return Value:

    None.

--*/	
{
    PCS8900_ADAPTER Adapter;

    Adapter = PCS8900_ADAPTER_FROM_CONTEXT_HANDLE(MiniportAdapterContext);

    Adapter->ShuttingDown = TRUE;

	DEBUGMSG(1, (TEXT("+CS8900:CS8900Shutdown - Not Implemented!\r\n")));
}


extern
VOID
CS8900Halt(
    IN NDIS_HANDLE MiniportAdapterContext
    )

/*++

Routine	Description:

    CS8900Halt removes an adapter that was previously initialized.

Arguments:

    MiniportAdapterContext - The context value that the	Miniport returned
	from CS8900Initialize; actually	as pointer to an CS8900_ADAPTER.

Return Value:

    None.

--*/

{
    PCS8900_ADAPTER Adapter;

    Adapter = PCS8900_ADAPTER_FROM_CONTEXT_HANDLE(MiniportAdapterContext);

	DEBUGMSG(1, (TEXT("+CS8900:CS8900Halt\r\n")));
//	while(1);
	
    return;
}




NDIS_STATUS
CS8900Reset(
    OUT	PBOOLEAN AddressingReset,
    IN NDIS_HANDLE MiniportAdapterContext
    )
/*++

Routine	Description:

    The	CS8900Reset request instructs the Miniport to issue a hardware reset
    to the network adapter.  The driver	also resets its	software state.	 See
    the	description of NdisMReset for a	detailed description of	this request.

Arguments:

    AddressingReset - Does the adapter need the	addressing information reloaded.

    MiniportAdapterContext - Pointer to	the adapter structure.

Return Value:

    The	function value is the status of	the operation.

--*/

{
    //
    // Pointer to the adapter structure.
    //
    PCS8900_ADAPTER Adapter = (PCS8900_ADAPTER)MiniportAdapterContext;

	DEBUGMSG(1, (TEXT("+CS8900:CS8900Reset\r\n")));
//	while (1);
	resetCS();

	initCS();

	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS
CS8900QueryInformation(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID	Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT	PULONG BytesWritten,
    OUT	PULONG BytesNeeded
)

/*++

Routine	Description:

    The	CS8900QueryInformation process a Query request for
    NDIS_OIDs that are specific	about the Driver.

Arguments:

    MiniportAdapterContext - a pointer to the adapter.

    Oid	- the NDIS_OID to process.

    InformationBuffer -	 a pointer into	the
    NdisRequest->InformationBuffer into	which store the	result of the query.

    InformationBufferLength - a	pointer	to the number of bytes left in the
    InformationBuffer.

    BytesWritten - a pointer to	the number of bytes written into the
    InformationBuffer.

    BytesNeeded	- If there is not enough room in the information buffer
    then this will contain the number of bytes needed to complete the
    request.

Return Value:

    The	function value is the status of	the operation.

--*/
{

    //
    // Pointer to the adapter structure.
    //
    PCS8900_ADAPTER Adapter = (PCS8900_ADAPTER)MiniportAdapterContext;

    //
    //	 General Algorithm:
    //
    //	    Switch(Request)
    //	       Get requested information
    //	       Store results in	a common variable.
    //	    default:
    //	       Try protocol query information
    //	       If that fails, fail query.
    //
    //	    Copy result	in common variable to result buffer.
    //	 Finish	processing

    UINT BytesLeft = InformationBufferLength;
    PUCHAR InfoBuffer =	(PUCHAR)(InformationBuffer);
    NDIS_STATUS	StatusToReturn = NDIS_STATUS_SUCCESS;
    NDIS_HARDWARE_STATUS HardwareStatus	= NdisHardwareStatusReady;
    NDIS_MEDIUM	Medium = NdisMedium802_3;

    //
    // This variable holds result of query
    //
    ULONG GenericULong;
    USHORT GenericUShort;
    UCHAR GenericArray[6];
    UINT MoveBytes = sizeof(ULONG);
    PVOID MoveSource = (PVOID)(&GenericULong);

	DEBUGMSG(1, (TEXT("+CS8900:CS8900QueryInformation\r\n")));

    //
    // Make sure that int is 4 bytes.  Else GenericULong must change
    // to something of size 4.
    //
    ASSERT(sizeof(ULONG) == 4);

    //
    // Switch on request type
    //

    switch (Oid) {

    case OID_GEN_MAC_OPTIONS:

		GenericULong = (ULONG)(NDIS_MAC_OPTION_TRANSFERS_NOT_PEND  |
			       NDIS_MAC_OPTION_RECEIVE_SERIALIZED  |
			       NDIS_MAC_OPTION_COPY_LOOKAHEAD_DATA |
			       NDIS_MAC_OPTION_NO_LOOPBACK
			       );
			       
		DEBUGMSG(1, (TEXT("----> OID_GEN_MAC_OPTIONS\r\n")));
		break;

    case OID_GEN_SUPPORTED_LIST:

		MoveSource = (PVOID)(CS8900SupportedOids);
		MoveBytes = sizeof(CS8900SupportedOids);
	
		DEBUGMSG(1, (TEXT("----> OID_GEN_SUPPORTED_LIST\r\n")));
		break;

    case OID_GEN_HARDWARE_STATUS:

		HardwareStatus = NdisHardwareStatusReady;
		MoveSource = (PVOID)(&HardwareStatus);
		MoveBytes = sizeof(NDIS_HARDWARE_STATUS);

		DEBUGMSG(1, (TEXT("----> OID_GEN_HARDWARE_STATUS\r\n")));
		break;

    case OID_GEN_MEDIA_SUPPORTED:
    case OID_GEN_MEDIA_IN_USE:

		MoveSource = (PVOID) (&Medium);
		MoveBytes = sizeof(NDIS_MEDIUM);

		DEBUGMSG(1, (TEXT("----> OID_GEN_MEDIA\r\n")));
		break;

    case OID_GEN_MAXIMUM_LOOKAHEAD:

		GenericULong = CS8900_MAX_LOOKAHEAD;

		DEBUGMSG(1, (TEXT("----> OID_GEN_MAXIMUM_LOOKAHEAD\r\n")));
		break;


    case OID_GEN_MAXIMUM_FRAME_SIZE:

		GenericULong = (ULONG)(1514 - CS8900_HEADER_SIZE);

		DEBUGMSG(1, (TEXT("----> OID_GEN_MAXIMUM_FRAME_SIZE\r\n")));
		break;


    case OID_GEN_MAXIMUM_TOTAL_SIZE:

		GenericULong = (ULONG)(1514);

		DEBUGMSG(1, (TEXT("----> OID_GEN_TOTAL_SIZE\r\n")));
		break;


    case OID_GEN_LINK_SPEED:

		GenericULong = (ULONG)(100000);

		DEBUGMSG(1, (TEXT("----> OID_GEN_LINK_SPEED\r\n")));
		break;


    case OID_GEN_TRANSMIT_BUFFER_SPACE:

		GenericULong = (ULONG)(1514);

		DEBUGMSG(1, (TEXT("----> OID_GEN_TRANSMIT_BUFFER_SPACE\r\n")));
//		while(1);
		break;

    case OID_GEN_RECEIVE_BUFFER_SPACE:

		GenericULong = (ULONG)(1514);

		DEBUGMSG(1, (TEXT("----> OID_GEN_RECEIVE_BUFFER_SPACE\r\n")));
//		while(1);
		break;

    case OID_GEN_TRANSMIT_BLOCK_SIZE:

		GenericULong = (ULONG)(1514);

		DEBUGMSG(1, (TEXT("----> OID_GEN_TRANSMIT_BLOCK_SIZE\r\n")));
//		while(1);
		break;

    case OID_GEN_RECEIVE_BLOCK_SIZE:

		GenericULong = (ULONG)(1514);

		DEBUGMSG(1, (TEXT("----> OID_GEN_RECEIVE_BLOCK_SIZE\r\n")));
//		while(1);
		break;

#ifdef CS8900

    case OID_GEN_VENDOR_ID:

		NdisMoveMemory(
	    	(PVOID)&GenericULong,
	    	Adapter->PermanentAddress,
	    	3
	    	);
		GenericULong &=	0xFFFFFF00;
		MoveSource = (PVOID)(&GenericULong);
		MoveBytes = sizeof(GenericULong);

		DEBUGMSG(1, (TEXT("----> OID_GEN_VENDER_ID\r\n")));
		break;

    case OID_GEN_VENDOR_DESCRIPTION:

		MoveSource = (PVOID)"CS8900A Adapter";
		MoveBytes = 21;

		DEBUGMSG(1, (TEXT("----> OID_GEN_VENDOR_DESCRIPTION\r\n")));
		break;

#else

    case OID_GEN_VENDOR_ID:

		NdisMoveMemory(
	    	(PVOID)&GenericULong,
	    	Adapter->PermanentAddress,
	    	3
	    	);
		GenericULong &=	0xFFFFFF00;
		GenericULong |=	0x01;
		MoveSource = (PVOID)(&GenericULong);
		MoveBytes = sizeof(GenericULong);

		break;

    case OID_GEN_VENDOR_DESCRIPTION:

		MoveSource = (PVOID)"Novell 1000 Adapter.";
		MoveBytes = 21;

		break;

#endif

    case OID_GEN_DRIVER_VERSION:

		GenericUShort =	((USHORT)CS8900_NDIS_MAJOR_VERSION << 8) |
			CS8900_NDIS_MINOR_VERSION;

		MoveSource = (PVOID)(&GenericUShort);
		MoveBytes = sizeof(GenericUShort);
		break;

    case OID_GEN_CURRENT_LOOKAHEAD:

		GenericULong = (ULONG)(Adapter->MaxLookAhead);

		DEBUGMSG(1, (TEXT("----> OID_GEN_CURRENT_LOOKAHEAD\r\n")));
		break;

    case OID_802_3_PERMANENT_ADDRESS:

		CS8900_MOVE_MEM((PCHAR)GenericArray,
		    Adapter->PermanentAddress,
		    CS8900_LENGTH_OF_ADDRESS);

		MoveSource = (PVOID)(GenericArray);
		MoveBytes = sizeof(Adapter->PermanentAddress);

		DEBUGMSG(1, (TEXT("----> OID_802_3_PERMANENT_ADDRESS\r\n")));
		break;

    case OID_802_3_CURRENT_ADDRESS:

		CS8900_MOVE_MEM((PCHAR)GenericArray,
		    Adapter->StationAddress,
		    CS8900_LENGTH_OF_ADDRESS);

		MoveSource = (PVOID)(GenericArray);
		MoveBytes = sizeof(Adapter->StationAddress);

		DEBUGMSG(1, (TEXT("----> OID_802_3_CURRENT_ADDRESS\r\n")));
		break;

    case OID_802_3_MAXIMUM_LIST_SIZE:

		GenericULong = (ULONG) (Adapter->MulticastListMax);

		DEBUGMSG(1, (TEXT("----> OID_802_3_MAXIMUM_LIST_SIZE\r\n")));
		break;

    case OID_GEN_XMIT_OK:

		GenericULong = (UINT)(Adapter->FramesXmitGood);

		DEBUGMSG(1, (TEXT("----> OID_GEN_XMIT_OK\r\n")));
		break;

    case OID_GEN_RCV_OK:

		GenericULong = (UINT)(Adapter->FramesRcvGood);

		DEBUGMSG(1, (TEXT("----> OID_GEN_RCV_OK\r\n")));
		break;

    case OID_GEN_XMIT_ERROR:

		GenericULong = (UINT)(Adapter->FramesXmitBad);
		DEBUGMSG(1, (TEXT("----> OID_GEN_XMIT_ERROR\r\n")));
		break;

    case OID_GEN_RCV_ERROR:

		GenericULong = (UINT)(Adapter->CrcErrors);
		DEBUGMSG(1, (TEXT("----> OID_GEN_RCV_ERROR\r\n")));
		break;

    case OID_GEN_RCV_NO_BUFFER:

		GenericULong = (UINT)(Adapter->MissedPackets);
		DEBUGMSG(1, (TEXT("----> OID_GEN_RCV_NO_BUFFER\r\n")));
		break;

    case OID_802_3_RCV_ERROR_ALIGNMENT:

		GenericULong = (UINT)(Adapter->FrameAlignmentErrors);
		DEBUGMSG(1, (TEXT("----> OID_802_3_RCV_ERROR_ALIGNMENT\r\n")));
		break;

    case OID_802_3_XMIT_ONE_COLLISION:

		GenericULong = (UINT)(Adapter->FramesXmitOneCollision);
		DEBUGMSG(1, (TEXT("----> OID_802_3_XMIT_ONE_COLLISION\r\n")));
		break;

    case OID_802_3_XMIT_MORE_COLLISIONS:

		GenericULong = (UINT)(Adapter->FramesXmitManyCollisions);
		break;


    case OID_GEN_MEDIA_CONNECT_STATUS:		
		GenericULong = NdisMediaStateConnected;                 
		break;

    case OID_GEN_MAXIMUM_SEND_PACKETS:

        	GenericULong = 1;
        	break;
		
    case OID_GEN_VENDOR_DRIVER_VERSION:		
		GenericULong = (DRIVER_MAJOR_VERSION << 16) | DRIVER_MINOR_VERSION;
		break;
		

    default:

		StatusToReturn = NDIS_STATUS_INVALID_OID;
		break;

    }


    if (StatusToReturn == NDIS_STATUS_SUCCESS) {

		if (MoveBytes >	BytesLeft) {

			//
	    	// Not enough room in InformationBuffer. Punt
	    	//

	    	*BytesNeeded = MoveBytes;

	    	StatusToReturn = NDIS_STATUS_INVALID_LENGTH;

		} else {

	    	//
	    	// Store result.
	    	//

	    	CS8900_MOVE_MEM(InfoBuffer,	MoveSource, MoveBytes);

	    	(*BytesWritten) += MoveBytes;

		}
    }

	DEBUGMSG(1, (TEXT("-CS8900:CS8900QueryInformation\r\n")));
    return StatusToReturn;
}

extern
NDIS_STATUS
CS8900SetInformation(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID	Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT	PULONG BytesRead,
    OUT	PULONG BytesNeeded
    )

/*++

Routine	Description:

    CS8900SetInformation handles a set operation for a
    single OID.

Arguments:

    MiniportAdapterContext - Context registered	with the wrapper, really
	a pointer to the adapter.

    Oid	- The OID of the set.

    InformationBuffer -	Holds the data to be set.

    InformationBufferLength - The length of InformationBuffer.

    BytesRead -	If the call is successful, returns the number
	of bytes read from InformationBuffer.

    BytesNeeded	- If there is not enough data in InformationBuffer
	to satisfy the OID, returns the	amount of storage needed.

Return Value:

    NDIS_STATUS_SUCCESS
    NDIS_STATUS_PENDING
    NDIS_STATUS_INVALID_LENGTH
    NDIS_STATUS_INVALID_OID

--*/
{
    //
    // Pointer to the adapter structure.
    //
    PCS8900_ADAPTER Adapter = (PCS8900_ADAPTER)MiniportAdapterContext;

    //
    // General Algorithm:
    //
    //	   Verify length
    //	   Switch(Request)
    //	      Process Request
    //

    UINT BytesLeft = InformationBufferLength;
    PUCHAR InfoBuffer =	(PUCHAR)(InformationBuffer);

    //
    // Variables for a particular request
    //
    UINT OidLength;

    //
    // Variables for holding the new values to be used.
    //
    ULONG LookAhead;
    ULONG Filter;

    //
    // Status of the operation.
    //
    NDIS_STATUS	StatusToReturn = NDIS_STATUS_SUCCESS;


    DEBUGMSG(1,
	(TEXT("+CS8900:CS8900SetInformation\r\n")));

    //
    // Get Oid and Length of request
    //
    OidLength =	BytesLeft;

    switch (Oid) {

    case OID_802_3_MULTICAST_LIST:

		DEBUGMSG(1, (TEXT("OID_802_3_MULTICAST_LIST\r\n")));

		//
		// Verify length
		//
		if ((OidLength % CS8900_LENGTH_OF_ADDRESS) != 0)
		{
			StatusToReturn = NDIS_STATUS_INVALID_LENGTH;

	    	*BytesRead = 0;
	    	*BytesNeeded = 0;

	   		break;
		}

		//
		// Set the new list on the adapter.
		//
		NdisMoveMemory(Adapter->Addresses, InfoBuffer, OidLength);
	
		break;

    case OID_GEN_CURRENT_PACKET_FILTER:

		DEBUGMSG(1, (TEXT("OID_GEN_CURRENT_PACKET_FILTER\r\n")));

		//
		// Verify length
		//

		if (OidLength != 4 ) {

	    	StatusToReturn = NDIS_STATUS_INVALID_LENGTH;

	    	*BytesRead = 0;
	    	*BytesNeeded = 0;

			break;

		}

		CS8900_MOVE_MEM(&Filter, InfoBuffer, 4);

		//
		// Verify bits
		//

		if (Filter & (NDIS_PACKET_TYPE_SOURCE_ROUTING |
	      	NDIS_PACKET_TYPE_SMT |
	      	NDIS_PACKET_TYPE_MAC_FRAME |
	      	NDIS_PACKET_TYPE_FUNCTIONAL |
	      	NDIS_PACKET_TYPE_ALL_FUNCTIONAL |
	      	NDIS_PACKET_TYPE_GROUP
	     	))	{

	    	StatusToReturn = NDIS_STATUS_NOT_SUPPORTED;

	    	*BytesRead = 4;
	    	*BytesNeeded = 0;

	    	break;

		}

		break;

    case OID_GEN_CURRENT_LOOKAHEAD:

		DEBUGMSG(1, (TEXT("OID_GEN_CURRENT_LOOKAHEAD\r\n")));

		//
		// Verify length
		//

		if (OidLength != 4) {
		
			StatusToReturn = NDIS_STATUS_INVALID_LENGTH;

	    	*BytesRead = 0;
	    	*BytesNeeded = 0;

	    	break;

		}

		//
		// Store the new value.
		//

		CS8900_MOVE_MEM(&LookAhead, InfoBuffer,	4);

		if (LookAhead <= CS8900_MAX_LOOKAHEAD) {
			Adapter->MaxLookAhead = LookAhead;
		} else {
			StatusToReturn = NDIS_STATUS_INVALID_LENGTH;
		}

		break;

    default:

		StatusToReturn = NDIS_STATUS_INVALID_OID;

		*BytesRead = 0;
		*BytesNeeded = 0;

		break;
    }


    if (StatusToReturn == NDIS_STATUS_SUCCESS) {

		*BytesRead = BytesLeft;
		*BytesNeeded = 0;

    }

    DEBUGMSG(1, (TEXT("-CS8900:CS8900SetInformation\r\n")));
    return(StatusToReturn);
}

VOID
CS8900CancelSendPackets(
   IN NDIS_HANDLE MiniportAdapterContext,
   IN PVOID pvCancelId
)
/*++

Routine Description:
   The miniport entry point to handle cancellation of all send packets
   that match the given CancelId. If we have queued any packets that match
   this, then we should dequeue them and call NdisMSendComplete for all
   such packets, with a status of NDIS_STATUS_REQUEST_ABORTED.

   We should also call NdisCancelSendPackets in turn, on each lower binding
   that this adapter corresponds to. This is to let miniports below cancel
   any matching packets.

Arguments:
   hMiniportAdapterContext - pointer to the binding structure
   pvCancelId - ID of packets to be cancelled.

Return Value:
   None

--*/
{
    PCS8900_ADAPTER Adapter = (PCS8900_ADAPTER)MiniportAdapterContext;


   //
   // If we queue packets on our adapter structure, this would be 
   // the place to acquire a spinlock to it, unlink any packets whose
   // Id matches CancelId, release the spinlock and call NdisMSendComplete
   // with NDIS_STATUS_REQUEST_ABORTED for all unlinked packets.
   //

   //
   // Next, pass this down so that we let the miniport(s) below cancel
   // any packets that they might have queued.
   //
//   NdisCancelSendPackets(Adapter->hPTBinding, pvCancelId);
   DEBUGMSG(ETHDBG, (TEXT("CS8900CancelSendPackets\n")));
   return;
}

VOID
CS8900DevicePnPEvent(
   IN NDIS_HANDLE MiniportAdapterContext,
   IN NDIS_DEVICE_PNP_EVENT  devicePnPEvent,
   IN PVOID pvInformationBuffer,
   IN ULONG ulInformationBufferLength
)
/*++

Routine Description:
   This handler is called to notify us of PnP events directed to
   our miniport device object.

Arguments:
   hMiniportAdapterContext  - pointer to the binding structure
   devicePnPEvent - the event
   pvInformationBuffer - points to additional event-specific information
   ulInformationBufferLength - length of above

Return Value:
   None

--*/
{
   // TBD - add code/comments about processing this.
    PCS8900_ADAPTER Adapter = (PCS8900_ADAPTER)MiniportAdapterContext;


   DEBUGMSG(ETHDBG, (TEXT("MiniportDevicePnPEvent\n")));
   return ;
}


VOID
CS8900AdapterShutdown(
   IN NDIS_HANDLE MiniportAdapterContext
)
/*++

Routine Description:
   This handler is called to notify us of an impending system shutdown.

Arguments:
   MiniportAdapterContext  - pointer to the binding structure

Return Value:
   None

--*/
{
    PCS8900_ADAPTER Adapter = (PCS8900_ADAPTER)MiniportAdapterContext;


   DEBUGMSG(ETHDBG, (TEXT("MiniportAdapterShutdown:\r\n")));
   return;
}
