#ifndef _ZMAC_H_
#define _ZMAC_H_

#include "zigbee.h"
#include "zPHY.h"

#define HFRAME_INVALID          (HFRAME)(MAX_MAC_FRAME_STATUS)

// Frame Control flags.
#define MAC_FRAME_TYPE_MASK         (0x03)
#define MAC_FRAME_BEACON            (0x00)
#define MAC_FRAME_DATA              (0x01)
#define MAC_FRAME_ACK               (0x02)
#define MAC_FRAME_CMD               (0x03)

#define MAC_CMD_ASSOCIATE_REQ       (0x01)
#define MAC_CMD_ASSOCIATE_RPLY      (0x02)
#define MAC_CMD_DISASSOCIATE_NOTICE (0x03)
#define MAC_CMD_DATA_REQ            (0x04)
#define MAC_CMD_PAN_ID_CONFLICT     (0x05)
#define MAC_CMD_ORPHAN_NOTICE       (0x06)
#define MAC_CMD_BEACON_REQ          (0x07)
#define MAC_CMD_COORD_REALIGNMENT   (0x08)
#define MAC_CMD_GTS_REQ             (0x09)

// Macros to determine current frame type
#define MACIsData()                 (macCurrentFrame.type == MAC_FRAME_DATA)
#define MACIsCommand()              (macCurrentFrame.type == MAC_FRAME_CMD)
#define MACIsBeacon()               (macCurrentFrame.type == MAC_FRAME_BEACON)
#define MACIsAck()                  (macCurrentFrame.type == MAC_FRAME_ACK)

#define MACIsAssocRequest()         (macCurrentFrame.cmd == MAC_CMD_ASSOCIATE_REQ)
#define MACIsDisassocNotice()       (macCurrentFrame.cmd == MAC_CMD_DISASSOCIATE_NOTICE)
#define MACIsBeaconRequest()        (macCurrentFrame.cmd == MAC_CMD_BEACON_REQ)
#define MACIsDataReq()              (macCurrentFrame.cmd == MAC_CMD_DATA_REQ)
#define MACIsCoordRealign()         (macCurrentFrame.cmd == MAC_CMD_COORD_REALIGNMENT)
#define MACIsOrphanNotice()         (macCurrentFrame.cmd == MAC_CMD_ORPHAN_NOTICE)
#define MACIsAssocResponse()        (macCurrentFrame.cmd == MAC_CMD_ASSOCIATE_RPLY)

/*
 *******************************************
 * Frame Type LSB fields - START
 *******************************************
 */
#define MAC_FRAME_INVALID           (0x07)

#define MAC_SECURITY_YES            (0x08)
#define MAC_SECURITY_NO             (0x00)

#define MAC_FRAME_PENDING_YES       (0x10)
#define MAC_FRAME_PENDING_NO        (0x00)

#define MAC_ACK_YES                 (0x20)
#define MAC_ACK_NO                  (0x00)

#define MAC_INTRA_PAN_YES           (0x40)
#define MAC_INTRA_PAN_NO            (0x00)

#define MAC_BEACON_YES              (0x40)
#define MAC_BEACON_NO               (0x00)

#if defined(I_AM_SECURITY_CAPABLE)
    #define MAC_SECURITY            MAC_SECURITY_YES
#else
    #define MAC_SECURITY            MAC_SECURITY_NO
#endif

/*
 *******************************************
 * Frame Type LSB fields - END
 *******************************************
 */


/*
 *******************************************
 * Frame Type MSB fields - START
 *******************************************
 */
#define MAC_DST_NO_ADDR             (0x00)
#define MAC_DST_SHORT_ADDR          (0x08)
#define MAC_DST_LONG_ADDR           (0x0c)
#define MAC_DST_ADDR_RESERVED       (0x04)

#define MAC_SRC_NO_ADDR             (0x00)
#define MAC_SRC_SHORT_ADDR          (0x80)
#define MAC_SRC_LONG_ADDR           (0xc0)
#define MAC_SRC_ADDR_RESERVED       (0x40)

/*
 *******************************************
 * Frame Type MSB fields - END
 *******************************************
 */

// Constants used to build a an Association Request (CMD frame 0x01)
#if defined(I_AM_COORDINATOR)
    #define IS_COORDINATOR      (0x01)      // 1 is yes, 0 if no.
#else
    #define IS_COORDINATOR      (0x00)
#endif

#if defined(I_AM_FFD)
    #define IS_FFD              (0x02)      // 0x02 if yes, 0x00 if RFD
#else
    #define IS_FFD              (0x00)
#endif

#if defined(I_AM_SECURITY_CAPABLE)
    #define IS_SECURITY_ENABLED (0x40)
#else
    #define IS_SECURITY_ENABLED (0x00)
#endif



#define SUPERFRAME_SPEC_LSB         (0xFF)
#define SUPERFRAME_SPEC_MSB         (0xC0)
#define GTS_FIELD_VAL               (0x00)

typedef BYTE HFRAME;

// Association State Machine states.
typedef enum _SM_ASSOCIATION
{
    SM_SEND_ASSOCIATE_REQ,
    SM_WAIT_FOR_TX_COMPLETE,
    SM_WAIT_FOR_ACK,
    SM_SEND_DATA_REQ,
    SM_DATA_REQ_ACK_WAIT,
    SM_WAIT_FOR_ASSOC_RESP,
    SM_SEND_DISASSOCIATE_REQ,
    SM_SEND_ORPHAN_NOTICE,
    SM_WAIT_FOR_COORD_ALIGNMENT
} SM_ASSOCIATION;

typedef enum _MAC_ASSOCIATE_STATUS
{
    MAC_ASSOCIATE_SUCCESS   = 0,
    MAC_ASSOCIATE_PAN_FULL  = 0x01,
    MAC_ASSOCIATE_DENIED    = 0x02
} MAC_ASSOCIATE_STATUS;

typedef enum _MAC_DISASSOCIATION_REASON_CODES
{
    MAC_COORDINATOR_FORCED_LEAVE = 0x01,
    MAC_DEVICE_LEAVE = 0x02
} MAC_DISASSOCIATION_REASON_CODES;

// A 802.15.4 Node address information.
typedef struct _NODE_INFO
{
    BYTE            addrMode;
    LONG_ADDR       longAddr;
    SHORT_ADDR      shortAddr;
    PAN_ADDR        panID;
} NODE_INFO;

extern NODE_INFO macInfo;

extern NODE_INFO macCoordInfo;

extern NODE_INFO macDestInfo;

// MAC module state
typedef union _MAC_STATE
{
    struct
    {
        unsigned int bIsAssociated      : 1;        // '1' if this node is associated
        unsigned int bIsTxBusy          : 1;        // '1' if transmission is in progress
        unsigned int bPermitAssociation : 1;        // '1' if other nodes are allowed to associat
        unsigned int bIsEnabled         : 1;        // '1' if PHY is enabled.
#if defined(I_AM_COORDINATOR)
        unsigned int bUseTxRAM : 1;                 // '1' if current tx packet should be put in stage
                                                    // buffer.
#elif defined(I_AM_END_DEVICE)
        unsigned int bIsPollDone        : 1;        // '1 if a frame of type data has been received
                                                    // after calling MACPoll()
#endif
    } bits;
    BYTE Val;
} MAC_STATE;

extern MAC_STATE macState;

// MAC Frame header information in decoded manner.
typedef struct _MAC_HEADER
{
    NODE_INFO dst;
    NODE_INFO src;
    BYTE type;
    BYTE cmd;
    BYTE macDSN;
    BYTE frameLength;
    union
    {
        struct
        {
            unsigned int : 3;                   // Frame type field.  Copied to 'type' member for faster access.
            unsigned int SecurityEnabled : 1;
            unsigned int FramePending : 1;
            unsigned int AckRequest : 1;
            unsigned int IntraPAN : 1;
            unsigned int : 1;                   // Bit 7 is reserved
        } bits;

        BYTE Val;
    } frameCONLSB;
    BYTE_VAL frameCONMSB;

    union
    {
        struct
        {
            unsigned int AltPANCoord : 1;
            unsigned int DeviceType: 1;
            unsigned int PowerSource:1;
            unsigned int ReceiveOnWhenIdle:1;
            unsigned int :2;
            unsigned int SecurityCapability:1;
            unsigned int AllocateAddress:1;
        } bits;
        BYTE Val;
    } capInfo;

    union
    {
        struct
        {
            unsigned int bIsGetReady:1;
            unsigned int bToBeQueued:1;
        } bits;
        BYTE Val;
    } Flags;

} MAC_HEADER;

extern MAC_HEADER macCurrentFrame;

// PAN descriptor in decoded manner - this is used by end device to determine whom to associate
typedef struct _PAN_DESC
{
    SHORT_ADDR  CoordPANId;
    union
    {
        SHORT_ADDR shortAddr;
        LONG_ADDR longAddr;
    } CoordAddress;

    BYTE        LogicalChannel;
    WORD_VAL    SuperFrameSpec;
    BYTE        LinkQuality;
    DWORD_VAL   TimeStamp;
    BYTE        ACLEntry;

    union
    {
        struct
        {
            unsigned int CoordAddrMode:1;     // 0 = 16-bit, 1 = 64-bit
            unsigned int GTSPermit:1;
            unsigned int SecurityInUse:1;
            unsigned int SecurityFailure:1;
        } bits;
        BYTE Val;
    } Flags;

} PAN_DESC;

extern PAN_DESC PANDesc;

extern BYTE PANDescCount;

//∑¢ÀÕª∫≥Â∂”¡–
typedef struct _MAC_FRAME_STATUS
{
    union
    {
        struct
        {
            unsigned int bIsInUse : 1;
            unsigned int bIsConfirmed:1;
            unsigned int bIsTimedOut:1;
        } bits;
        BYTE Val;
    } Flags;

    BYTE macDSN;
    TICK lastTick;
    BYTE retryCount;

} MAC_FRAME_STATUS;

// Exact length of frame stauts queue would strictly depend on how long is your ack timeout and
// the rate of new frame transmissions.
#define MAX_MAC_FRAME_STATUS    (8)

extern MAC_FRAME_STATUS macFrameStatusQ[MAX_MAC_FRAME_STATUS];
/////////////////////////////////////////////////////////////////////////////////////////////////

/*********************************************************************
 * Function:        void MACInit(void)
 *
 * PreCondition:    macInfo.longAddr is set as required.
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Initializes data variables used by MAC module.
 *
 * Note:            None
 ********************************************************************/
void MACInit(void);

BOOL MACIsIdle(); 

/*********************************************************************
 * Function:        void MACEnable(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Enables PHY regulator and registers.
 *
 * Note:            None
 ********************************************************************/
void MACEnable(void);

/*********************************************************************
 * Macro:           void MACDisable(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Disbales PHY and marks MAC module as disabled.
 *
 * Note:            None
 ********************************************************************/
#define MACDisable()        {macState.bits.bIsEnabled = FALSE; macCurrentFrame.Flags.bits.bIsGetReady = FALSE; PHYDisable();}

/*********************************************************************
 * Function:        void MACISR(void)
 *
 * PreCondition:    MACInit() is previously called.
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Determines if a frame has completed transmission
 *                  or not.
 *
 * Note:            None
 ********************************************************************/
void MACISR(void);

/*********************************************************************
 * Function:        BOOL MACTask(void)
 *
 * PreCondition:    MACInit() is called
 *
 * Input:           None
 *
 * Output:          TRUE        - If Processing is complete
 *                  FALSE       - If further processing is required
 *
 * Side Effects:    None
 *
 * Overview:        If previous frame is processed, it checks
 *                  for new RX frame.
 *                  Fetches the header and determiens if it is valid
 *                  Automatically sends ACK if remote node has
 *                  requested it.
 *                  Also performs some other automatic processing
 *                  based on the frame type.
 *                  If there is no frame in buffer, it goes through
 *                  DSN queue and calculates timeout for unack'ed
 *                  TX frames.
 *
 * Note:            This function must be called as many times as
 *                  possible to keep handling MAC frames.
 ********************************************************************/
BOOL MACTask(void);

// Callback to application to notify of a frame transmission.
extern void AppMACFrameTransmitted(void);

// Callback to application to notify of a frame reception
extern void AppMACFrameReceived(void);

// Callback to application to notify of an ack timeout
extern void AppMACFrameTimeOutOccurred(void);

/*********************************************************************
 * Macro:           void MACSetFirstChannel(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Selects very first channel in current frequency
 *                  band (i.e. channel 11 for 2.4GHz band)
 *
 * Note:            This macro is designed to allow application to
 *                  browse through all channels irrespective of
 *                  frequency band in use.
 ********************************************************************/
#define MACSetFirstChannel()    PHYSetFirstChannel()

/*********************************************************************
 * Function:        BOOL MACSetNextChannel(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          TRUE if a valid next channel was selected
 *                  FALES if end of available channels was reached.
 *
 * Side Effects:    None
 *
 * Overview:        Sequences to next available channel specific
 *                  to current frequency band.
 *                  When end of channels is reached, FALSE is returned
 *                  and application call SetFirstChannel() to start
 *                  again.
 *
 * Note:            None
 ********************************************************************/
#define MACSetNextChannel()     PHYSetNextChannel()

/*********************************************************************
 * Macro:           BYTE MACGetChannel(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          Channel number.
 *
 * Side Effects:    None
 *
 * Overview:        Selects very first channel in current frequency
 *                  band (i.e. channel 11 for 2.4GHz band)
 *
 * Note:            This macro is designed to allow application to
 *                  browse through all channels irrespective of
 *                  frequency band in use.
 ********************************************************************/
#define MACGetChannel()         PHYGetChannel()

/*********************************************************************
 * Macros:          void MACSetLongAddrByte?(BYTE b)
 *
 * PreCondition:    None
 *
 * Input:           b       - Long address byte '?' value.
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Saves given byte 'b' into corresponding long address
 *                  byte.
 *
 * Note:            There are total of 8 long address bytes.
 *                  If required, you may also access macInfo.longAddr
 *                  directly and load it with a value.
 *
 *                  This macro simply updates local variable. To
 *                  actuall start using the new address, you must call
 *                  MACUpdateAddressInfo()
 ********************************************************************/
#define MACSetLongAddrByte0(b)          (macInfo.longAddr.v[0] = b)
#define MACSetLongAddrByte1(b)          (macInfo.longAddr.v[1] = b)
#define MACSetLongAddrByte2(b)          (macInfo.longAddr.v[2] = b)
#define MACSetLongAddrByte3(b)          (macInfo.longAddr.v[3] = b)
#define MACSetLongAddrByte4(b)          (macInfo.longAddr.v[4] = b)
#define MACSetLongAddrByte5(b)          (macInfo.longAddr.v[5] = b)
#define MACSetLongAddrByte6(b)          (macInfo.longAddr.v[6] = b)
#define MACSetLongAddrByte7(b)          (macInfo.longAddr.v[7] = b)

/*********************************************************************
 * Macros:          BYTE MACGetLongAddrByte?(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          Returns a value for specified address byte.
 *
 * Side Effects:    None
 *
 * Overview:        Returns given byte 'b' of corresponding long address
 *                  byte.
 *
 * Note:            There are total of 8 long address bytes.
 *                  If required, you may also access macInfo.longAddr
 *                  directly and load it with a value.
 *
 ********************************************************************/
#define MACGetLongAddrByte0()           (macInfo.longAddr.v[0])
#define MACGetLongAddrByte1()           (macInfo.longAddr.v[1])
#define MACGetLongAddrByte2()           (macInfo.longAddr.v[2])
#define MACGetLongAddrByte3()           (macInfo.longAddr.v[3])
#define MACGetLongAddrByte4()           (macInfo.longAddr.v[4])
#define MACGetLongAddrByte5()           (macInfo.longAddr.v[5])
#define MACGetLongAddrByte6()           (macInfo.longAddr.v[6])
#define MACGetLongAddrByte7()           (macInfo.longAddr.v[7])

/*********************************************************************
 * Macros:          void MACSetShortAddr?SB(BYTE b)
 *
 * PreCondition:    None
 *
 * Input:           b   - Byte value that is to be set
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Saves given vale to either LSB or MSB as per
 *                  actual macro call.
 *
 * Note:            Normally, short address is automatically set upon
 *                  network association. This macro is provided to
 *                  manually set the value if required.
 *                  In current version, only automatic short address
 *                  is supported.
 *
 *                  This macro simply updates local variable. To
 *                  actuall start using the new address, you must call
 *                  MACUpdateAddressInfo()
 ********************************************************************/
#define MACSetShortAddrLSB(b)           (macInfo.shortAddr.byte.LSB = b)
#define MACSetShortAddrMSB(b)           (macInfo.shortAddr.byte.MSB = b)

/*********************************************************************
 * Macros:          BYTE MACGetShortAddr?SB(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          A byte value of either LSB or MSB.
 *
 * Side Effects:    None
 *
 * Overview:        Returns either LSB or MSB of short address
 *
 * Note:            If required, you may access macInfo.shortAddr
 *                  directly.
 *
 ********************************************************************/
#define MACGetShortAddrLSB()            (macInfo.shortAddr.byte.LSB)
#define MACGetShortAddrMSB()            (macInfo.shortAddr.byte.MSB)
#define MACGetShortAddr                 (macInfo.shortAddr)

/*********************************************************************
 * Macros:          void MACSetPANId?SB(BYTE b)
 *
 * PreCondition:    None
 *
 * Input:           b   - Byte value that is to be set
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Saves given vale to either LSB or MSB as per
 *                  actual macro call.
 *
 * Note:            Normally, PAN id is automatically set upon
 *                  network association. This macro is provided to
 *                  manually set the value if required.
 *                  In current version, only automatic PAN id
 *                  is supported.
 *
 *                  This macro simply updates local variable. To
 *                  actuall start using the new address, you must call
 *                  MACUpdateAddressInfo()
 ********************************************************************/
#define MACSetPANIdLSB(b)               (macInfo.panID.byte.LSB = b)
#define MACSetPANIdMSB(b)               (macInfo.panID.byte.MSB = b)

/*********************************************************************
 * Macros:          BYTE MACGetPANId?SB(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          Returns either LSB or MSB of PAN id
 *
 * Side Effects:    None
 *
 * Overview:        Returns pan id value.
 *
 * Note:            If required, you may access pan ID directly.
 ********************************************************************/
#define MACGetPANIdLSB()                (macInfo.panID.byte.LSB)
#define MACGetPANIdMSB()                (macInfo.panID.byte.MSB)
#define MACGetPANId()                   (macInfo.panID)

/*********************************************************************
 * Function:        void MACUpdateAddressInfo(void)
 *
 * PreCondition:    MACEnable() is called
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Writes our MAC address, PAN ID and short address
 *                  transciever configuration RAM.
 *
 * Note:            None
 ********************************************************************/
void MACUpdateAddressInfo(void);

/*********************************************************************
 * Macros:          void MACSetAssociationPermit?SB(BOOL v)
 *
 * PreCondition:    None
 *
 * Input:           v       - TRUE if association is permitted
 *                            FALSE if otherwise
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Marks the flag that allows/disallows future
 *                  association requests.
 *
 * Note:            None
 ********************************************************************/
#define MACSetAssociationPermit(v)      (macState.bits.bPermitAssociation = v)

/*********************************************************************
 * Function:        BOOL MACIsPutReady(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Determines if it is okay to load new tx frame
 *                  into RF TX buffer.
 *
 * Note:            None
 ********************************************************************/
BOOL MACIsPutReady(void);

/*********************************************************************
 * Function:        void MACPutHeader(NODE_INFO *dest, BYTE frameCON)
 *
 * PreCondition:    MACIsPutReady() = TRUE
 *
 * Input:           dest        - Destination node info
 *                  frameCON    - Frame control for frame header
 *                                Logical AND of
 *                                "Frame Control flags" as defined
 *                                in zMAC.h
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Constructs and loads frame header as per
 *                  IEEE spec.
 *                  If frame requires ack, it makes an entry into
 *                  DSN queue so that future ack can be matches
 *                  against this frame.
 *
 * Note:            None
 ********************************************************************/
void MACPutHeader(NODE_INFO *dst, BYTE frameCON);

/*********************************************************************
 * Function:        void MACPut(BYTE v)
 *
 * PreCondition:    MACIsPutReady() == TRUE
 *
 * Input:           v       - A byte to put
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Copies given byte into RF TX buffer and increments
 *                  write pointer.
 *
 * Note:            None
 ********************************************************************/
void MACPut(BYTE v);

/*********************************************************************
 * Function:        void MACPutArray(BYTE *v, BYTE len)
 *
 * PreCondition:    MACIsPutReady() == TRUE
 *
 * Input:           b       - Buffer that is to be loaded.
 *                  len     - Number of bytes to get
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Loads given bytes into RF TX buffer.
 *
 * Note:            None
 ********************************************************************/
void MACPutArray(BYTE *v, BYTE len);

/*********************************************************************
 * Function:        HFRAME MACFlush(void)
 *
 * PreCondition:    MACIsPutReady() == TRUE and
 *                  a valid TX frame is constructured using
 *                  MACPutHeader() and MACPut()
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Sets the tx frame length, marks the tx frame
 *                  as ready to be transmitted.
 *                  If this frame requires ack from remote node,
 *                  an entry is made into DSN queue so that
 *                  we can match future ACK with this frame.
 *
 * Note:            None
 ********************************************************************/
HFRAME MACFlush(void);

/*********************************************************************
 * Function:        BOOL MACFrameIsAcked(HFRAME h)
 *
 * PreCondition:    MACPutHeader() is called
 *
 * Input:           h       - Handle to frame whose ack state
 *                            is to be determined
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Peeks into DSN queue and checks to see if
 *                  dsn associated with this frame was ack'ed.
 *
 * Note:            None
 ********************************************************************/
BOOL MACFrameIsAcked(HFRAME h);

/*********************************************************************
 * Function:        void MACFrameRemove(HFRAME h)
 *
 * PreCondition:    MACPutHeader() is called
 *
 * Input:           h       - Handle to frame that is to be removed
 *                            from DSN queue
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Sets given DSN queue entry to free.
 *
 * Note:            None
 ********************************************************************/
void MACFrameRemove(HFRAME h);

/*********************************************************************
 * Macro:           BOOL MACFrameIsTimedOut(HFRAME h)
 *
 * PreCondition:    h is a valid hFrame
 *
 * Input:           h       - Handle to frame that is to be checked
 *
 * Output:          TRUE if frame is timed out
 *                  FALSE, otherwise
 *
 * Side Effects:    None
 *
 * Overview:        Returns timeout flag of given frame handle.
 *
 * Note:            None
 ********************************************************************/
#define MACFrameIsTimedOut(h)       (macFrameStatusQ[h].Flags.bits.bIsTimedOut)

/*********************************************************************
 * Macro:           BOOL MACFrameIsRetryExhausted(HFRAME h)
 *
 * PreCondition:    h is a valid hFrame
 *
 * Input:           h       - Handle to frame whose retry status
 *                            is to be checked
 *
 * Output:          TRUE if retry exhausted,
 *                  FALSE otherwise
 *
 * Side Effects:    None
 *
 * Overview:        Checks retry count for this frame.
 *
 * Note:            None
 ********************************************************************/
#define MACFrameIsRetryExhausted(h) (macFrameStatusQ[h].retryCount == 0)

/*********************************************************************
 * Macro:        BOOL MACIsGetReady(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          TRUE if there is a new frame pending in RX buffer
 *
 * Side Effects:    None
 *
 * Overview:        Returns private frame ready flag.
 *                  Actual frame was fetched during MACTask()
 *
 * Note:            None
 ********************************************************************/
#define MACIsGetReady()     (macCurrentFrame.Flags.bits.bIsGetReady)

/*********************************************************************
 * Function:        BYTE MACGet(void)
 *
 * PreCondition:    MACIsGetReady() == TRUE
 *
 * Input:           None
 *
 * Output:          Byte that was read
 *
 * Side Effects:    None
 *
 * Overview:        Retrives one bytes from RF RX buffer.
 *
 * Note:            Caller must make sure that RX buffer contains
 *                  at least one byte calling MACIsGetReady()
 ********************************************************************/
BYTE MACGet(void);

/*********************************************************************
 * Function:        BYTE MACGetArray(BYTE *b, BYTE len)
 *
 * PreCondition:    MACIsGetReady() == TRUE
 *
 * Input:           b       - Buffer to hold the data
 *                  len     - Number of bytes to get
 *
 * Output:          Number of bytes actually read.
 *
 * Side Effects:    None
 *
 * Overview:        Reads RF RX buffer until given len bytes are
 *                  are read or buffer is empty.
 *
 * Note:            Actual number of bytes read may be less than
 *                  asked. Caller must make sure that
 *                  RX buffer contains desired number of bytes.
 ********************************************************************/
BYTE MACGetArray(BYTE *b, BYTE len);

/*********************************************************************
 * Function:        void MACDiscardRx(void)
 *
 * PreCondition:    MACIsGetReady() = TRUE
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Fetches remaining data bytes for current received
 *                  frame.
 *
 * Note:            None
 ********************************************************************/
void MACDiscardRx(void);

/*********************************************************************
 * Function:        BOOL MACProcessAssociation(void)
 *
 * PreCondition:    A valid frame is in RX buffer and
 *                  header is already fetched and processed.
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Fetches rest of the frame and processes it if
 *                  is a valid association request frame.
 *
 * Note:            Available to coordinator only.
 ********************************************************************/
BOOL MACProcessAssociation(void);

/*********************************************************************
 * Function:        HFRAME MACSendAssociateResponse(
 *                              SHORT_ADDR *assocShortAddr,
 *                              MAC_ASSOCIATE_STATUS status)
 *
 * PreCondition:    assocShortAddr  - Short address to be sent
 *                                    as part of association response
 *                  status          - Association result code to be sent
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Creates and sends valid association response frame
 *                  as per IEEE spec.
 *
 * Note:            Available to coordinator only.
 ********************************************************************/
HFRAME MACSendAssociateResponse(SHORT_ADDR *assocShortAddr,
                                MAC_ASSOCIATE_STATUS status);

/*********************************************************************
 * Function:        HFRAME MACSendRealignment(
 *                          SHORT_ADDR *destShortAddr, BOOL bBroadcast)
 *
 * PreCondition:    MACIsPutReady() = TRUE
 *
 * Input:           destShortAddr - node to whom relignment frame to send
 *                  bBroadcast    - indicates if this is to be broadcast
 *
 * Output:          Handle to frame that was just transmitted.
 *                  Caller must use this handle to determine if this
 *                  frame was ack'ed by intended remote node.
 *
 * Side Effects:    None
 *
 * Overview:        Creates and sends valid relignment frame
 *                  as per IEEE spec.
 *
 * Note:            Available to coordinator only.
 ********************************************************************/
HFRAME MACSendRealignment(SHORT_ADDR *destShortAddr, BOOL bBroadcast);

/*********************************************************************
 * Function:        BOOL MACProcessDisassociation(void)
 *
 * PreCondition:    MACGetHeader() is already called and this frame
 *                  is identified as Disassociate command frame.
 *
 * Input:           None
 *
 * Output:          TRUE if found valid association frame.
 *                  FALSE othersie
 *
 * Side Effects:    None
 *
 * Overview:        Fetches rest of disassociation frame and validates
 *                  as per IEEE spec.
 *
 * Note:            Available to coordinator only.
 ********************************************************************/
BOOL MACProcessDisassociation(void);

#if defined(I_AM_COORDINATOR)
	/*********************************************************************
	 * Macro:           void MACStart(void)
	 *
	 * PreCondition:    MACEnable is called.
	 *
	 * Input:           None
	 *
	 * Output:          None
	 *
	 * Side Effects:    None
	 *
	 * Overview:        Enables PHY receiver.
	 *
	 * Note:            Available to coordinator only.
	 ********************************************************************/
	#define MACStart()                  PHYSetTRXState(PHY_TRX_RX_ON)

	/*********************************************************************
	 * Macro:           void MACFormNetwork(void)
	 *
	 * PreCondition:    An empty channel was found with no coordinator
	 *                  operating in that channel..
	 *
	 * Input:           None
	 *
	 * Output:          None
	 *
	 * Side Effects:    None
	 *
	 * Overview:        Remembers that we are now associated.
	 *
	 * Note:            Available to coordinator only.
	 ********************************************************************/
	#define MACFormNetwork()            (macState.bits.bIsAssociated = TRUE)

	/*********************************************************************
	 * Macro:           BOOL MACIsNetworkEstablished(void)
	 *
	 * PreCondition:    None
	 *
	 * Input:           None
	 *
	 * Output:          TRUE if network is established.
	 *                  FALSE otherwise
	 *
	 * Side Effects:    None
	 *
	 * Overview:        Returns network establish status
	 *
	 * Note:            Available to coordinator only.
	 ********************************************************************/
	#define MACIsNetworkEstablished()   (macState.bits.bIsAssociated)

	/*********************************************************************
	 * Macro:           BOOL MACIsJoinPermitted(void)
	 *
	 * PreCondition:    MACEnable is called.
	 *
	 * Input:           None
	 *
	 * Output:          TRUE if new joins are permitted
	 *                  FALSE otherwise
	 *
	 * Side Effects:    None
	 *
	 * Overview:        Returns previously set flag.
	 *
	 * Note:            Available to coordinator only.
	 ********************************************************************/
	#define MACIsJoinPermitted()        (macState.bits.bPermitAssociation)

#elif defined(I_AM_END_DEVICE)
	/*********************************************************************
	 * Function:        void MACStartAssociation(void)
	 *
	 * PreCondition:    MACAcceptCurrentPAN() is already called.
	 *                  A network scan was performed and appropriate
	 *                  coordinator was selected.
	 *
	 * Input:           None
	 *
	 * Output:          None
	 *
	 * Side Effects:    None
	 *
	 * Overview:        Prepares MAC state machine to begin association
	 *                  sequence.
	 *
	 * Note:            This is available for end device only.
	 ********************************************************************/
	void MACStartAssociation(void);

	/*********************************************************************
	 * Function:        BOOL MACIsAssociationComplete(void)
	 *
	 * PreCondition:    MACStartAssociation() is called.
	 *
	 * Input:           None
	 *
	 * Output:          None
	 *
	 * Side Effects:    None
	 *
	 * Overview:        performs association sequence as per 802.15.4
	 *                  spec.
	 *
	 * Note:            This is available for end device only.
	 ********************************************************************/
	BOOL MACIsAssociationComplete(void);

	/*********************************************************************
	 * Macro:           BOOL MACIsAssociated(void)
	 *
	 * PreCondition:    None
	 *
	 * Input:           None
	 *
	 * Output:          TRUE if associated with a coordinator
	 *                  FALSE otherwise
	 *
	 * Side Effects:    None
	 *
	 * Overview:        returns previusly set flag.
	 *
	 * Note:            This is available for end device only.
	 ********************************************************************/
	#define MACIsAssocated()        (macState.bits.bIsAssociated)

	/*********************************************************************
	 * Function:        void MACAcceptCurrentPAN(void)
	 *
	 * PreCondition:    MACIsScanComplete() = TRUE and there is no
	 *                  ERROR.
	 *
	 * Input:           None
	 *
	 * Output:          None
	 *
	 * Side Effects:    None
	 *
	 * Overview:        Accepts last discovered coordinator as ours.
	 *
	 * Note:            Available to end device only.
	 ********************************************************************/
	void MACAcceptCurrentPAN(void);

	/*********************************************************************
	 * Function:        void MACStartOrphanNotification(void)
	 *
	 * PreCondition:    MACIsPutReady() = TRUE
	 *
	 * Input:           None
	 *
	 * Output:          None
	 *
	 * Side Effects:    None
	 *
	 * Overview:        Prepares the state machine for orphan notification
	 *
	 * Note:            Available to end devices only.
	 ********************************************************************/
	void MACStartOrphanNotification(void);

	/*********************************************************************
	 * Function:        BOOL MACIsOrphanNotificationComplete(void)
	 *
	 * PreCondition:    MACStartOrphanNotification() is called.
	 *
	 * Input:           None
	 *
	 * Output:          TRUE if orphan notification process is complete
	 *                  FALSE otherwise.
	 *
	 * Side Effects:    None
	 *
	 * Overview:        Performs orphan notification steps as per
	 *                  IEEE spec.
	 *
	 * Note:            Available to end devices only.
	 ********************************************************************/
	BOOL MACIsOrphanNotificationComplete(void);

	/*********************************************************************
	 * Function:        void MACStartDisassociation(void)
	 *
	 * PreCondition:    MACInit(), MACEnable() are called
	 *                  And MACIsPutReady() == TRUE
	 *
	 * Input:           None
	 *
	 * Output:          None
	 *
	 * Side Effects:    None
	 *
	 * Overview:        Starts the disassociation process.
	 *
	 * Note:            Available to end devices only.
	 ********************************************************************/
	void MACStartDisassociation(void);

	/*********************************************************************
	 * Function:        BOOL MACIsDisassociationComplete(void)
	 *
	 * PreCondition:    MACStartDisassociation() is already called
	 *
	 * Input:           None
	 *
	 * Output:          TRUE, if disassociation is complete
	 *                  FALSE, if otherwise
	 *
	 * Side Effects:    None
	 *
	 * Overview:        Performs disassociation state machine
	 *
	 * Note:            Available to end devices only.
	 ********************************************************************/
	BOOL MACIsDisassociationComplete(void);

#endif

/*********************************************************************
 * Macro:           BOOL MACIsPANAvailable(void)
 *
 * PreCondition:    MACStartAssociation() process is completed.
 *
 * Input:           None
 *
 * Output:          TRUE if there was at least one PAN was detected
 *                  FALSE otherwise
 *
 * Side Effects:    None
 *
 * Overview:        Returns previously detected network count
 *
 * Note:            None
 ********************************************************************/
#define MACIsPANAvailable()     (PANDescCount)
    
/*********************************************************************
 * Function:        void MACStartScan(BOOL bActiveScan)
 *
 * PreCondition:    MACInit(), MACEnable() are called
 *                  And MACIsPutReady() == TRUE
 *
 * Input:           bActiveScan - flag to perform active/passive scan
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        If this is active scan, sends out beacon request
 *
 * Note:            None
 ********************************************************************/
void MACStartScan(BOOL bActiveScan);

/*********************************************************************
 * Function:        BOOL MACIsScanComplete(void)
 *
 * PreCondition:    MACStartScan() is called.
 *
 * Input:           None
 *
 * Output:          TRUE if scan is complete
 *                  FALSE otherwise.
 *
 * Side Effects:    None
 *
 * Overview:        Checks to see a beacon was received and if it is,
 *                  it processes it.
 *
 * Note:            None
 ********************************************************************/
BOOL MACIsScanComplete(void);

/*********************************************************************
 * Function:        void MACStartED(void)
 *
 * PreCondition:    PHYInit() has been called
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Begins an energy detection scan
 *
 * Note:            None
 ********************************************************************/
void MACStartED(void);

/*********************************************************************
 * Function:        BOOL MACIsEDComplete(void)
 *
 * PreCondition:    MACStartED() is called
 *
 * Input:           None
 *
 * Output:          TRUE if this Energy detect is complete
 *                  FALSE otherwise.
 *
 * Side Effects:    None
 *
 * Overview:        Simply determines if predefined time has passed or
 *                  not.
 *
 * Note:            None
 ********************************************************************/
BOOL MACIsEDComplete(void);

/*********************************************************************
 * Macro:           BYTE MACGetEDValue(void)
 *
 * PreCondition:    MACStartED() is complete.
 *
 * Input:           None
 *
 * Output:          Value of measured RF energy
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            None
 ********************************************************************/
#define MACGetEDValue()         PHYGetED()

void MACPutLongAddress(LONG_ADDR *addr);

void MACPutShortAddress(SHORT_ADDR *addr);

BOOL MACIsAddressAssigned(void);

#endif
