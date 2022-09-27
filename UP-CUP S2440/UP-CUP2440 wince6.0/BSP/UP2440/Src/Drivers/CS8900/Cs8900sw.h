/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.
Copyright (c) 1998 Samsung Electronics.

Module Name:

    cs8900sw.h

Abstract:

    The main header for an Cirrus Logic CS8900 Miniport driver.

Notes:

--*/

#ifndef _CS8900SFT_
#define _CS8900SFT_


#define DRIVER_MAJOR_VERSION        0x01
#define DRIVER_MINOR_VERSION        0x00

#define CS8900_NDIS_MAJOR_VERSION 5
#define CS8900_NDIS_MINOR_VERSION 1

#define	MAXLOOP						0x8888

//
// This macro is used along with the flags to selectively
// turn on debugging.
//

#if DBG

#define IF_CS8900DEBUG(f) if (Ne2000DebugFlag & (f))
extern ULONG Ne2000DebugFlag;

#define CS8900_DEBUG_LOUD               0x00000001  // debugging info
#define CS8900_DEBUG_VERY_LOUD          0x00000002  // excessive debugging info
#define CS8900_DEBUG_LOG                0x00000004  // enable Ne2000Log
#define CS8900_DEBUG_CHECK_DUP_SENDS    0x00000008  // check for duplicate sends
#define CS8900_DEBUG_TRACK_PACKET_LENS  0x00000010  // track directed packet lens
#define CS8900_DEBUG_WORKAROUND1        0x00000020  // drop DFR/DIS packets
#define CS8900_DEBUG_CARD_BAD           0x00000040  // dump data if CARD_BAD
#define CS8900_DEBUG_CARD_TESTS         0x00000080  // print reason for failing

//
// Macro for deciding whether to print a lot of debugging information.
//

#define IF_LOUD(A) IF_CS8900DEBUG( CS8900_DEBUG_LOUD ) { A }
#define IF_VERY_LOUD(A) IF_CS8900DEBUG( CS8900_DEBUG_VERY_LOUD ) { A }

//
// Whether to use the Ne2000Log buffer to record a trace of the driver.
//
#define IF_LOG(A) IF_CS8900DEBUG( CS8900_DEBUG_LOG ) { A }
extern VOID Ne2000Log(UCHAR);

//
// Whether to do loud init failure
//
#define IF_INIT(A) A

//
// Whether to do loud card test failures
//
#define IF_TEST(A) IF_CS8900DEBUG( CS8900_DEBUG_CARD_TESTS ) { A }

//
// Windows CE debug zones
//
#define ZONE_ERROR      DEBUGZONE(0)
#define ZONE_WARN       DEBUGZONE(1)
#define ZONE_FUNCTION   DEBUGZONE(2)
#define ZONE_INIT       DEBUGZONE(3)
#define ZONE_INTR       DEBUGZONE(4)
#define ZONE_RCV        DEBUGZONE(5)
#define ZONE_XMIT       DEBUGZONE(6)

#else

//
// This is not a debug build, so make everything quiet.
//
#define IF_LOUD(A)
#define IF_VERY_LOUD(A)
#define IF_LOG(A)
#define IF_INIT(A)
#define IF_TEST(A)

#endif

//
// Create a macro for moving memory from place to place.  Makes
// the code more readable and portable in case we ever support
// a shared memory CS8900 adapter.
//
#define CS8900_MOVE_MEM(dest,src,size) NdisMoveMemory(dest,src,size)

//
// The status of transmit buffers.
//

typedef enum {
    EMPTY = 0x00,
    FULL = 0x02
} BUFFER_STATUS;

//
// Type of an interrupt.
//

typedef enum {
    RECEIVE     = 0x01,
    TRANSMIT    = 0x02,
    BUFOVERFLOW = 0x04,
    COUNTER     = 0x08,
    UNKNOWN     = 0x10
} INTERRUPT_TYPE;

//
// Size of the ethernet header
//
#define	CS8900_HEADER_SIZE		14

//
// Size of the ethernet address
//
#define	CS8900_LENGTH_OF_ADDRESS 6

//
// Number of bytes allowed in a lookahead (max)
//
#define CS8900_MAX_LOOKAHEAD (1514 - CS8900_HEADER_SIZE)

//
// Maximum number of transmit buffers on the card.
//
//#define MAX_XMIT_BUFS   12

//
// Definition of a transmit buffer.
//
//typedef UINT XMIT_BUF;

//
// Number of 256-byte buffers in a transmit buffer.
//
#define BUFS_PER_TX 1

//
// Size of a single transmit buffer.
//
#define TX_BUF_SIZE (BUFS_PER_TX*256)

//
// This structure contains information about the driver
// itself.  There is only have one of these structures.
//
typedef struct _DRIVER_BLOCK {

    //
    // NDIS wrapper information.
    //
    NDIS_HANDLE NdisMacHandle;          // returned from NdisRegisterMac
    NDIS_HANDLE NdisWrapperHandle;      // returned from NdisInitializeWrapper

    //
    // Adapters registered for this Miniport driver.
    //
    struct _CS8900_ADAPTER * AdapterQueue;

} DRIVER_BLOCK, * PDRIVER_BLOCK;



//
// This structure contains all the information about a single
// adapter that this driver is controlling.
//
typedef struct _CS8900_ADAPTER {

    //
    // This is the handle given by the wrapper for calling ndis
    // functions.
    //
    NDIS_HANDLE MiniportAdapterHandle;

    //
    // Interrupt object.
    //
    NDIS_MINIPORT_INTERRUPT Interrupt;

    //
    // used by DriverBlock->AdapterQueue
    //
    struct _CS8900_ADAPTER * NextAdapter;

    //
    // This is a count of the number of receives that have been
    // indicated in a row.  This is used to limit the number
    // of sequential receives so that one can periodically check
    // for transmit complete interrupts.
    //
    ULONG ReceivePacketCount;

    //
    // Configuration information
    //

    //
    // Physical address of the IoBaseAddress
    //
    PVOID IoBaseAddr;

    //
    // Interrupt number this adapter is using.
    //
    CHAR InterruptNumber;

    //
    // Number of multicast addresses that this adapter is to support.
    //
    UINT MulticastListMax;

    //
    // The type of bus that this adapter is running on.  Either ISA or
    // MCA.
    //
    UCHAR BusType;

    //
    // InterruptType is the next interrupt that should be serviced.
    //
    UCHAR InterruptType;

    //
    //  Type of CS8900 card.
    //
    UINT    CardType;

    //
    //  Address of the memory window.
    //
    ULONG   AttributeMemoryAddress;
    ULONG   AttributeMemorySize;

    //
    // Operational information.
    //

    //
    // Mapped address of the base io port.
    //
    ULONG IoPAddr;
  
    //
    // InterruptStatus tracks interrupt sources that still need to be serviced,
    // it is the logical OR of all card interrupts that have been received and not
    // processed and cleared. (see also INTERRUPT_TYPE definition in cs8900.h)
    //
    UCHAR InterruptStatus;

    //
    // The ethernet address currently in use.
    //
    UCHAR StationAddress[CS8900_LENGTH_OF_ADDRESS];

    //
    // The ethernet address that is burned into the adapter.
    //
    UCHAR PermanentAddress[CS8900_LENGTH_OF_ADDRESS];

    //
    // The current packet filter in use.
    //
    ULONG PacketFilter;

    //
    // Flag that is set when we are shutting down the interface
    //
	BOOLEAN ShuttingDown;
    
    //
    // Statistics used by Set/QueryInformation.
    //

    ULONG FramesXmitGood;               // Good Frames Transmitted
    ULONG FramesRcvGood;                // Good Frames Received
    ULONG FramesXmitBad;                // Bad Frames Transmitted
    ULONG FramesXmitOneCollision;       // Frames Transmitted with one collision
    ULONG FramesXmitManyCollisions;     // Frames Transmitted with > 1 collision
    ULONG FrameAlignmentErrors;         // FAE errors counted
    ULONG CrcErrors;                    // CRC errors counted
    ULONG MissedPackets;                // missed packet counted

    //
    // Reset information.
    //

    UCHAR NicMulticastRegs[8];          // contents of card multicast registers
    UCHAR NicReceiveConfig;             // contents of NIC RCR
    UCHAR NicInterruptMask;             // contents of NIC IMR

    //
    // The lookahead buffer size in use.
    //
    ULONG MaxLookAhead;

    //
    // CS8900 address of the beginning of the packet.
    //
    PUCHAR PacketHeaderLoc;

    //
    // Lookahead buffer
    //
    UCHAR Lookahead[CS8900_MAX_LOOKAHEAD + CS8900_HEADER_SIZE];

    //
    // List of multicast addresses in use.
    //
    CHAR Addresses[DEFAULT_MULTICASTLISTMAX][CS8900_LENGTH_OF_ADDRESS];

} CS8900_ADAPTER, * PCS8900_ADAPTER;



//
// Given a MiniportContextHandle return the PCS8900_ADAPTER
// it represents.
//
#define PCS8900_ADAPTER_FROM_CONTEXT_HANDLE(Handle) \
    ((PCS8900_ADAPTER)(Handle))

//
// Given a pointer to a CS8900_ADAPTER return the
// proper MiniportContextHandle.
//
#define CONTEXT_HANDLE_FROM_PCS8900_ADAPTER(Ptr) \
    ((NDIS_HANDLE)(Ptr))

//
// Macros to extract high and low bytes of a word.
//
#define MSB(Value) ((UCHAR)((((ULONG)Value) >> 8) & 0xff))
#define LSB(Value) ((UCHAR)(((ULONG)Value) & 0xff))

//
// What we map into the reserved section of a packet.
// Cannot be more than 8 bytes (see ASSERT in cs8900.c).
//
typedef struct _MINIPORT_RESERVED {
    PNDIS_PACKET Next;    // used to link in the queues (4 bytes)
} MINIPORT_RESERVED, * PMINIPORT_RESERVED;


//
// Retrieve the MINIPORT_RESERVED structure from a packet.
//
#define RESERVED(Packet) ((PMINIPORT_RESERVED)((Packet)->MiniportReserved))

//
// Procedures which log errors.
//

typedef enum _CS8900_PROC_ID {
    cardReset,
    cardCopyDownPacket,
    cardCopyDownBuffer,
    cardCopyUp
} CS8900_PROC_ID;


//
// Special error log codes.
//
#define CS8900_ERRMSG_CARD_SETUP          (ULONG)0x01
#define CS8900_ERRMSG_DATA_PORT_READY     (ULONG)0x02
#define CS8900_ERRMSG_HANDLE_XMIT_COMPLETE (ULONG)0x04

//
// Declarations for functions in cs8900.c.
//

NDIS_STATUS
CS8900SetInformation(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesRead,
    OUT PULONG BytesNeeded
    );

VOID
CS8900Shutdown(
    IN NDIS_HANDLE MiniportAdapterContext
    );

VOID
CS8900Halt(
    IN NDIS_HANDLE MiniportAdapterContext
    );

NDIS_STATUS
CS8900RegisterAdapter(
    IN PCS8900_ADAPTER Adapter,
    IN NDIS_HANDLE ConfigurationHandle,
    IN BOOLEAN ConfigError,
    IN ULONG ConfigErrorValue
    );

NDIS_STATUS
MiniportInitialize(
    OUT PNDIS_STATUS OpenErrorStatus,
    OUT PUINT SelectedMediumIndex,
    IN PNDIS_MEDIUM MediumArray,
    IN UINT MediumArraySize,
    IN NDIS_HANDLE MiniportAdapterHandle,
    IN NDIS_HANDLE ConfigurationHandle
    );

NDIS_STATUS
CS8900TransferData(
    OUT PNDIS_PACKET Packet,
    OUT PUINT BytesTransferred,
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_HANDLE MiniportReceiveContext,
    IN UINT ByteOffset,
    IN UINT BytesToTransfer
    );

NDIS_STATUS
CS8900Send(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN PNDIS_PACKET Packet,
    IN UINT Flags
    );

NDIS_STATUS
CS8900Reset(
    OUT PBOOLEAN AddressingReset,
    IN NDIS_HANDLE MiniportAdapterContext
    );

NDIS_STATUS
CS8900QueryInformation(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesWritten,
    OUT PULONG BytesNeeded
    );

VOID
CS8900Halt(
    IN NDIS_HANDLE MiniportAdapterContext
    );

VOID
CS8900CancelSendPackets(
   IN NDIS_HANDLE hMiniportAdapterContext,
   IN PVOID pvCancelId
);

VOID
CS8900DevicePnPEvent(
   IN NDIS_HANDLE hMiniportAdapterContext,
   IN NDIS_DEVICE_PNP_EVENT  devicePnPEvent,
   IN PVOID pvInformationBuffer,
   IN ULONG ulInformationBufferLength
);

VOID
CS8900AdapterShutdown(
   IN NDIS_HANDLE hMiniportAdapterContext
);

//
// Interrup.c
//

void CS8900ReceiveEvent(PCS8900_ADAPTER Adapter, unsigned short RxEvent);

//VOID
//CS8900EnableInterrupt(
//    IN NDIS_HANDLE MiniportAdapterContext
//    );

//VOID
//CS8900DisableInterrupt(
//    IN NDIS_HANDLE MiniportAdapterContext
//    );

VOID
CS8900Isr(
    OUT PBOOLEAN InterruptRecognized,
    OUT PBOOLEAN QueueDpc,
    IN PVOID Context
    );

VOID
CS8900HandleInterrupt(
    IN NDIS_HANDLE MiniportAdapterContext
    );

//
// Declarations of functions in cs8900.c.
//

unsigned short
CS8900ReadRegister(
	unsigned short offset
	);

void
CS8900WriteRegister(
	unsigned short offset,
	unsigned short data
	);

BOOLEAN
CS8900Initialize(
    IN PCS8900_ADAPTER Adapter
    );

BOOLEAN
CS8900ReadEthernetAddress(
    IN PCS8900_ADAPTER Adapter
    );

unsigned short CS8900RequestTransmit
(
	UINT PacketLength
    );

void CS8900CopyTxFrame
(
	PCHAR pPacket,
	UINT PacketLength
    );

#endif // CS8900SFT

