#ifndef _ZNWK_H_
#define _ZNWK_H_

#include "zigbee.h"
#include "zMAC.h"

//randam value
#define RANDOM_LSB                      (TCNT1L)
#define RANDOM_MSB                      (TCNT1H)

//路由路径
/////////////////////////////////////////
#define MAX_NODE 5

typedef struct _PATH_INFO
{
	BYTE src;
	BYTE dst;
	BYTE metric;
	BYTE path[MAX_NODE-2];
}PATH_INFO;

extern PATH_INFO path_info[MAX_NODE];

PATH_INFO nwk_get_path_info(BYTE dst);
/////////////////////////////////////////

// NWK_FRAMECON_LSB
#define NWK_FRAME_TYPE_MASK     (0x03)
#define NWK_FRAME_DATA          (0x00)
#define NWK_FRAME_CMD           (0x01)
#define NWKIsData()                 ((nwkCurrentFrame.frameCONLSB.Val & NWK_FRAME_TYPE_MASK) == NWK_FRAME_DATA )
#define NWKIsCommand()              ((nwkCurrentFrame.frameCONLSB.Val & NWK_FRAME_TYPE_MASK) == NWK_FRAME_CMD)

#define NWK_PROT_VER_MASK       (0x3C)
#define NWK_PROT_VER            (0x01 << 2)

#define NWK_DISCOVER_ROUTE_MASK (1<<6)

// NWK_FRAMECON_MSB
#define NWK_SECURITY_MASK       (0x02)
#define NWK_SECURITY            (0x01 << 1)

// NWK task states
typedef enum _SM_NWK
{
    SM_NWK_ENERGY_SCAN,
    SM_NWK_ENERGY_SCAN_WAIT,
    SM_NWK_ACTIVE_SCAN_WAIT,
    SM_NWK_FORMED,
    SM_NWK_NEXT_CHANNEL
} SM_NWK;

// NWK module flags.
typedef union _NWK_FLAGS
{
    struct
    {
        unsigned int bIsNWKDiscovered:1;
    } bits;
    BYTE Val;
} NWK_FLAGS;

extern NWK_FLAGS _NWKFlags;

// Defs for use by stack upper layer only.
typedef union NWK_FRAMECON_LSB
{
    struct
    {
        unsigned int type:2;
        unsigned int version:4;
        unsigned int discover:1;
        unsigned int :1;
    } bits;
    BYTE Val;
} NWK_FRAMECON_LSB;

typedef union NWK_FRAMECON_MSB
{
    struct
    {
        unsigned int :1;
        unsigned int security:1;
        unsigned int :6;
    } bits;
    BYTE Val;
} NWK_FRAMECON_MSB;

typedef struct _NWK_HEADER
{
    NWK_FRAMECON_LSB frameCONLSB;
    NWK_FRAMECON_MSB frameCONMSB;

    SHORT_ADDR destAddr;
    SHORT_ADDR srcAddr;

    BYTE broadcastRadius;
    BYTE broadcastSequence;

    union
    {
        struct
        {
            unsigned int bIsInProcess:1;
        } bits;
        BYTE Val;
    } Flags;
} NWK_HEADER;

extern NWK_HEADER nwkCurrentFrame;

extern BYTE* NWKPayload;

extern BYTE* NWKPayloadHead;

extern BYTE NWKPayloadLength;

extern BYTE NWKPayloadLengthSave;

#define NWK_ISR() MACISR()

#define NWKEnable() MACEnable()

#define NWKIsIdle() MACIsIdle()

#define NWKLeave() MACStartDisassociation()

#define NWKIsLeaveComplete() MACIsDisassociationComplete()

#define NWKUpdateAddressInfo() 	MACUpdateAddressInfo()

#define NWKPutLongAddress(x)  MACPutLongAddress(x)

#define NWKPutShortAddress(x) MACPutShortAddress(x)

#define NWKGetLongAddress(x) MACGetLongAddress(x) 

#define NWKGetShortAddress(x) MACGetShortAddress(x) 

#define NWKIsAddressAssigned()  MACIsAddressAssigned()

#define NWKFrameIsAcked(x) MACFrameIsAcked(x)

#define NWKFrameIsTimedOut(x) MACFrameIsTimedOut(x)

#define NWKFrameRemove(x) MACFrameRemove(x)

//add by lyj_uptech@126.com
//get short_addr
#define NWKGetDestShortAddr() (nwkCurrentFrame.destAddr)

#define NWKGetSrcShortAddr() (nwkCurrentFrame.srcAddr)

/*********************************************************************
 * Function:        void NWKInit(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            None
 ********************************************************************/
void NWKInit(void);

/*********************************************************************
 * Function:        BOOL NWKTask(void)
 *
 * PreCondition:    None
 *
 * Input:           TRUE if task has finished its task
 *                  FALSE, if otherwise
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            This function must be called frequently to
 *                  keep NWK layer going.
 ********************************************************************/
BOOL NWKTask(void);

/*********************************************************************
 * Function:        void NWKStartDiscovery(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Starts network discovery process
 *
 * Note:            Available for coorindator only.
 ********************************************************************/
void NWKStartDiscovery(void);

/*********************************************************************
 * Function:        BOOL NWKIsDiscoveryComplete(void)
 *
 * PreCondition:    NWKStartDiscovery() is called
 *
 * Input:           None
 *
 * Output:          TRUE, if complete
 *                  FALSE otherwise
 *                  Use GetZError() to check for error.
 *
 * Side Effects:    None
 *
 * Overview:        Performs network discovery process.
 *
 * Note:            None
 ********************************************************************/
BOOL NWKIsDiscoveryComplete(void);

/*********************************************************************
 * Function:        BOOL NWKIsDiscovered(void)
 *
 * PreCondition:    NWKStartDiscovery() is called
 *
 * Input:           None
 *
 * Output:          TRUE, if one network is discovered
 *                  FALSE otherwise
 *                  Use GetZError() to check for error.
 *
 * Side Effects:    None
 *
 * Overview:        Returns result of previously initiated NWKDiscovery
 *                  process
 *
 * Note:            None
 ********************************************************************/
#define NWKIsDiscovered()           (_NWKFlags.bits.bIsNWKDiscovered)

#if defined(I_AM_COORDINATOR)

    /*********************************************************************
     * Function:        void NWKCoordInit(void)
     *
     * PreCondition:    None
     *
     * Input:           None
     *
     * Output:          None
     *
     * Side Effects:    None
     *
     * Overview:        As part of coordinator init procedure, this function
     *                  initiates the discovery for another coordinator
     *                  operating in this radio sphere.
     *
     * Note:            None
     ********************************************************************/
    #define NWKCoordInit()          NWKStartDiscovery()

    /*********************************************************************
     * Function:        BOOL NWKIsCoordInitComplete(void)
     *
     * PreCondition:    NWKCoordInit() is already called
     *
     * Input:           None
     *
     * Output:          TRUE is Coordinator initialization is complete.
     *                  FALSE, if otherwise
     *
     * Side Effects:    None
     *
     * Overview:        Performs coordinator initialization state machine
     *                  tasks.
     *
     * Note:            None
     ********************************************************************/
    BOOL NWKIsCoordInitComplete(void);

    /*********************************************************************
     * Function:        void NWKForm(void)
     *
     * PreCondition:    NWKDiscovery is successfully completed.
     *
     * Input:           None
     *
     * Output:          None
     *
     * Side Effects:    None
     *
     * Overview:        Selects PANID and establishes network
     *
     * Note:            Available to coordinators only.
     ********************************************************************/
    void NWKForm(void);

    /*********************************************************************
     * Macro:           BOOL NWKIsFormed(void)
     *
     * PreCondition:    NWKForm is successfully completed.
     *
     * Input:           None
     *
     * Output:          TRUE, if a network is established
     *                  FALSE, otherwise
     *
     * Side Effects:    None
     *
     * Overview:        Performs NWK formation state machine
     *
     * Note:            Available to coordinators only.
     ********************************************************************/
    #define NWKIsFormed()           MACIsNetworkEstablished()

    /*********************************************************************
     * Macro:           void NWKPermitJoining(void)
     *
     * PreCondition:    NWKIsFormed() = TRUE
     *
     * Input:           None
     *
     * Output:          None
     *
     * Side Effects:    None
     *
     * Overview:        Permits new nodes to join with this network.
     *
     * Note:            Available to coordinators only.
     ********************************************************************/
    #define NWKPermitJoining()      MACSetAssociationPermit(TRUE)

    /*********************************************************************
     * Macro:           void NWKDisableJoining(void)
     *
     * PreCondition:    NWKIsFormed() = TRUE
     *
     * Input:           None
     *
     * Output:          None
     *
     * Side Effects:    None
     *
     * Overview:        Does not allow new nodes to join with this network.
     *
     * Note:            Available to coordinators only.
     ********************************************************************/
    #define NWKDisableJoining()     MACSetAssociationPermit(FALSE)

    /*********************************************************************
     * Function:        BOOL AppOkayToAcceptThisNode(LONG_ADDR *longAddr)
     *
     * PreCondition:    None
     *
     * Input:           longAddr    - Long address of a node that wants
     *                                to join to this network.
     *
     * Output:          TRUE if application is okay with this node joining
     *                  FALSE otherwise
     *
     * Side Effects:    None
     *
     * Overview:        This is a callback to application.
     *                  The application would check given long address against
     *                  some predefined algorithm and allow/disallow
     *                  given node from joining the network.
     *
     * Note:            Available to coordinators only.
     ********************************************************************/
    extern BOOL AppOkayToAcceptThisNode(LONG_ADDR *longAddr);

    /*********************************************************************
     * Function:        void AppNewNodeJoined(LONG_ADDR *nodeAddr,
     *                                       BOOL bIsRejoined)
     *
     * PreCondition:    None
     *
     * Input:           longAddr    - Long address of a node that has
     *                                just joined the network
     *                  bIsRejoined - TRUE if this node was already a member
     *                                of this network.
     *                                FALSE, if this node has rejoined
     *                                this network.
     *
     * Output:          None
     *
     * Side Effects:    None
     *
     * Overview:        This is a callback to application.
     *                  This function is provided as a notification to
     *                  application that a new node has joined the network
     *                  The application may use this information to perform
     *                  application spedific process.
     *
     * Note:            Available to coordinators only.
     ********************************************************************/
    extern void AppNewNodeJoined(LONG_ADDR *nodeAddr, BOOL bIsRejoined);

    /*********************************************************************
     * Function:        void AppNodeLeft(LONG_ADDR *nodeAddr)
     *
     * PreCondition:    None
     *
     * Input:           longAddr    - Long address of a node that has
     *                                just left the network
     *
     * Output:          None
     *
     * Side Effects:    None
     *
     * Overview:        This is a callback to application.
     *                  This function is provided as a notification to
     *                  application that a new node has left the network
     *                  The application may use this information to perform
     *                  application spedific process.
     *
     * Note:            Available to coordinators only.
     ********************************************************************/
    extern void AppNodeLeft(LONG_ADDR *nodeAddr);

#endif

#if defined(I_AM_END_DEVICE)
    /*********************************************************************
     * Macro:           void NWKStartJoin(void)
     *
     * PreCondition:    None
     *
     * Input:           None
     *
     * Output:          None
     *
     * Side Effects:    None
     *
     * Overview:        This macro starts end device association procedure.
     *
     * Note:            Available to end devices only.
     ********************************************************************/
    #define NWKStartJoin()          MACStartAssociation()

    /*********************************************************************
     * Macro:           BOOL NWKIsJoinComplete(void)
     *
     * PreCondition:    None
     *
     * Input:           None
     *
     * Output:          TRUE, if nwk join process is complete
     *                  FALSE, otherwise
     *                  Call GetLastZError() to determine success/failure.
     *
     * Side Effects:    None
     *
     * Overview:        This macro runs association state machine.
     *
     * Note:            Available to end devices only.
     ********************************************************************/
    #define NWKIsJoinComplete()     MACIsAssociationComplete()

    /*********************************************************************
     * Macro:           BOOL NWKIsJoined(void)
     *
     * PreCondition:    None
     *
     * Input:           None
     *
     * Output:          TRUE, if this node has joined a network.
     *                  FALSE, otherwise
     *                  Call GetLastZError() to determine success/failure.
     *
     * Side Effects:    None
     *
     * Overview:        This macro returns the result of NWKJoin call.
     *
     * Note:            Available to end devices only.
     ********************************************************************/
    #define NWKIsJoined()           MACIsAssocated()

    /*********************************************************************
     * Macro:           void NWKStartRejoin(void)
     *
     * PreCondition:    None
     *
     * Input:           None
     *
     * Output:          None
     *
     * Side Effects:    None
     *
     * Overview:        This macro starts rejoin (orphan association)
     *                  process
     *
     * Note:            Available to end devices only.
     ********************************************************************/
    #define NWKStartRejoin()        MACStartOrphanNotification()

    /*********************************************************************
     * Macro:           BOOL NWKIsRejoinComplete(void)
     *
     * PreCondition:    None
     *
     * Input:           None
     *
     * Output:          TRUE, if nwk rejoin process is complete
     *                  FALSE, otherwise
     *                  Call GetLastZError() to determine success/failure.
     *
     * Side Effects:    None
     *
     * Overview:        This macro runs orphan notification state machine.
     *
     * Note:            Available to end devices only.
     ********************************************************************/
    #define NWKIsRejoinComplete()   MACIsOrphanNotificationComplete()

    /*********************************************************************
     * Macro:           void NWKAcceptCurrentPAN(void)
     *
     * PreCondition:    NWKIsJoined() = TRUE
     *
     * Input:           None
     *
     * Output:          None
     *
     * Side Effects:    None
     *
     * Overview:        This macro accepts previously discovered PAN.
     *
     * Note:            Available to end devices only.
     ********************************************************************/
    #define NWKAcceptCurrentPAN()   MACAcceptCurrentPAN()

#endif

/*********************************************************************
 * Function:        BOOL AppOkayToUseChannel(BYTE channel)
 *
 * PreCondition:    None
 *
 * Input:           channel - Channel number that is to be verify
 *
 * Output:          TRUE if application wants to use given channel
 *                  FALSE if otherwise
 *
 * Side Effects:    None
 *
 * Overview:        This is a callback to application to ask if app
 *                  wants to use given channel.If not, next channel
 *                  will be selected and app will be asked again.
 *
 * Note:            None
 ********************************************************************/
extern BOOL AppOkayToUseChannel(BYTE channel);

/*********************************************************************
 * Macro:           BOOL NWKIsPutReady(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          TRUE if it is okay to load new byte into transmit buffer
 *                  FALSE if otherwise
 *
 * Side Effects:    None
 *
 * Overview:        This is a wrapper on MAC function to provide a
 *                  level of abstraction.
 *
 * Note:            None
 ********************************************************************/
#define NWKIsPutReady()         MACIsPutReady()

/*********************************************************************
 * Macro:           void NWKPut(BYTE d)
 *
 * PreCondition:    NWKIsPutReady() = TRUE
 *
 * Input:           d       - A data byte that is to be loaded
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This is a wrapper on MAC function to provide a
 *                  level of abstraction.
 *                  It loads given byte into trasnmit buffer.
 *
 * Note:            None
 ********************************************************************/
#define NWKPut(d)               MACPut(d)

/*********************************************************************
 * Macro:           void NWKPutArray(BYTE* v, BYTE s)
 *
 * PreCondition:    NWKIsPutReady() = TRUE
 *
 * Input:           v       - A data byte array that is to be loaded
 *                  s       - Count of bytes
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This is a wrapper on MAC function to provide a
 *                  level of abstraction.
 *                  It loads given array into transmit buffer.
 *
 * Note:            None
 ********************************************************************/
#define NWKPutArray(v, s)       MACPutArray(v, s)

/*********************************************************************
 * Macro:           void NWKFlush(void)
 *
 * PreCondition:    NWKIsPutReady() = TRUE and NWKPutHeader() and
 *                  zero or more NWKPut() are called.
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This is a wrapper on MAC function to provide a
 *                  level of abstraction.
 *                  It transmits current transmit buffer
 *
 * Note:            None
 ********************************************************************/
#define NWKFlush()              MACFlush()

/*********************************************************************
 * Function:        void NWKPutHeader(NODE_INFO *dest,
 *                                      BYTE frameCONLSB,
 *                                      BYTE frameCONMSB)
 *
 * PreCondition:    NWKIsPutReady() = TRUE
 *
 * Input:           dest        - Dest node
 *                  frameCONLSB - NWK frame header control LSB
 *                  frameCONMSB - NWK frame header control MSB
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Prepares NWK frame header and loads it in XCVR
 *
 * Note:            None
 ********************************************************************/
void NWKPutHeader(NODE_INFO *dest,//原为struct NODE_INFO *dest
                  BYTE frameCONLSB,
                  BYTE frameCONMSB,
				  SHORT_ADDR dest_nwk_addr);

/*********************************************************************
 * Macro:           BOOL NWKIsGetReady(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          TRUE if it there is a new NWK frame received
 *                  FALSE if otherwise
 *
 * Side Effects:    None
 *
 * Overview:        This is a wrapper on MAC function to provide a
 *                  level of abstraction.
 *
 * Note:            None
 ********************************************************************/
#define NWKIsGetReady()         MACIsGetReady()

/*********************************************************************
 * Macro:           BYTE NWKGet(void)
 *
 * PreCondition:    NWKIsGetReady() = TRUE
 *
 * Input:           None
 *
 * Output:          A data byte
 *
 * Side Effects:    None
 *
 * Overview:        This is a wrapper on MAC function to provide a
 *                  level of abstraction.
 *                  It fetches one data byte from current receive buffer
 *
 * Note:            None
 ********************************************************************/
//du 直接从NWKPayLoad中取数据
BYTE NWKGet(void);    

/*********************************************************************
 * Macro:           BYTE NWKGetArray(BYTE *v, BYTE n)
 *
 * PreCondition:    NWKIsGetReady() = TRUE
 *
 * Input:           None
 *
 * Output:          v       - Buffer containing received data bytes
 *                  n       - Number of bytes to get
 *
 * Side Effects:    None
 *
 * Overview:        This is a wrapper on MAC function to provide a
 *                  level of abstraction.
 *                  It fetches given number of data bytes from current receive buffer
 *
 * Note:            None
 ********************************************************************/
//du 直接从NWKPayLoad中取数据
BYTE NWKGetArray(BYTE *v, BYTE n);

/*********************************************************************
 * Macro:           void NWKDiscardRx(void)
 *
 * PreCondition:    NWKIsGetReady() = TRUE
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This macro reclaims current receive frame buffer space.
 *
 * Note:            This macro must be called for evey frame recieved
 *                  to reclaim receive buffer space.
 ********************************************************************/
#define NWKDiscardRx()          nwkCurrentFrame.Flags.bits.bIsInProcess = FALSE; MACDiscardRx()

/*********************************************************************
 * Macro:           void NWKDisable(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This macro disables NWK module state machine.
 *
 * Note:            This macro is called in conjunction with MACDisable
 *                  to enter into low power mode.
 ********************************************************************/
#define NWKDisable()            {nwkCurrentFrame.Flags.bits.bIsInProcess = FALSE; MACDisable();}
/*********************************************************************
*
* Note: send destinfo and srcinfo for APL add by lyj_uptech@126.com
*
*
*********************************************************************/



//路由
#define NWK_CMD_ROUTE_REQUEST 0x01
#define NWK_CMD_ROUTE_REPLY 0x02
#define NWK_CMD_ROUTE_ERROR 0x03
#define NWK_CMD_ROUTE_ACK 0x05

//aodv message
//////////////////////////////////////////////////////////////////////////////////////////////////////
#define AODV_RREQ 101
#define AODV_RREP 102
#define AODV_RERR 103
#define AODV_RREP_ACK 104

/*RREQ包格式
  0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |     Type      |J|R|G|D|U|   Reserved          |   Hop Count   |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                            RREQ ID                            |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                    Destination IP Address                     |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                  Destination Sequence Number                  |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                    Originator IP Address                      |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                  Originator Sequence Number                   |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
typedef struct _RREQ
{
	BYTE type;
	union
	{
		struct
		{
			BYTE :3;
			BYTE u:1;
			BYTE d:1;
			BYTE g:1;
			BYTE r:1;
			BYTE j:1;
		}bit;
		BYTE val;
	}flag;
	
	BYTE reserved;
	BYTE hopCount;
	WORD req_id;
	WORD dest_ip;
	WORD dest_seq;
	WORD source_ip;
	WORD sourceSeq;
}RREQ;

/*RREP包格式
    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |     Type      |R|A|    Reserved     |Prefix Sz|   Hop Count   |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                     Destination IP address                    |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                  Destination Sequence Number                  |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                    Originator IP address                      |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                           life_time                            |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
typedef struct _RREP
{
   BYTE type;
   union
   {
		struct
		{
			BYTE :6;
			BYTE a:1;
			BYTE r:1;
		}bit;
		BYTE val;
   }flag;
   BYTE reserved;
   BYTE prefix; //低6位有效
   BYTE hopCount;
   WORD dest_ip;
   WORD dest_seq;
   WORD source_ip;
   WORD life_time;
}RREP;

/*RERR包格式
    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |     Type      |N|          Reserved           |   DestCount   |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |            Unreachable Destination IP Address (1)             |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |         Unreachable Destination Sequence Number (1)           |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-|
   |  Additional Unreachable Destination IP Addresses (if needed)  |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |Additional Unreachable Destination Sequence Numbers (if needed)|
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   */
typedef struct _RERR
{
   BYTE type;
   union
   {
	   struct
	   {
			BYTE :7;
		    BYTE n:1;
	   }bit;
	   BYTE val;
   }flag;
   BYTE reserved;
   BYTE dest_count;

}RERR;
	
struct Err_Dest{
   WORD unreach_dest_ip;
   WORD unreach_dest_seq;
};

struct Err_Dest_List{
   struct Err_Dest err_dest;
   struct Err_Dest_List * next;
};

/*RREP_ACK包格式
    0                   1
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |     Type      |   Reserved    |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
 typedef struct _RREP_ACK
 {
	 BYTE type;
	 BYTE reserved;
 }RREP_ACK;
//////////////////////////////////////////////////////////////////////////////////////////////////////

//路由表项
 struct Route_Table_Entry
{
	WORD dest_ip;
	WORD dest_seq;
	BYTE hopCount;
	WORD next_ip;
	struct Precuror_Entry *precuror;
	WORD life_time;//路由表项的生存时间
	union
	{
		struct
		{	BYTE is_self_route:1;
			BYTE is_route_valid:1;
			BYTE is_route_seq_valid:1;
			BYTE :5;
		}bit;
		BYTE val;
	}flag; 
	WORD req_id;//not use in current edition
	struct Route_Table_Entry * prev;
	struct Route_Table_Entry * next;
};

//先驱表项
 struct Precuror_Entry
{
	WORD prev_ip;
	struct Precuror_Entry *prev;
	struct Precuror_Entry *next;
};

struct REQ_ID
{
		WORD source_ip;
		WORD dest_ip;
		WORD req_id;
		WORD life_time;
		struct REQ_ID *next;
};

struct NWK_FRAME_STATUS
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
    //TICK lastTick;
    BYTE retryCount;

} ; 

//route_table function
/////////////////////////////////////////////////////////////////////////////////////////////////////
void init_route_table();
struct Route_Table_Entry *create_route( WORD ip);
void insert_route(struct Route_Table_Entry * new_route);
struct Route_Table_Entry *lookup_route( WORD dest_ip);
void cleanup_route_table();
//void expire_aodv_route(Route_Table_Entry * tmp_route);
void delete_route( WORD dest_ip);
void update_route( WORD dest_ip, WORD next_ip, WORD dest_seq, BYTE hopCount);
//int flush_route_table();
//Route_Table_Entry *first_aodv_route();
//////////////////////////////////////////////////////////////////////////////////////////////////////

//req_id_queue function
///////////////////////////////////////////////////////////////////////////////////////////////////////
void init_req_id_queue();
void insert_req_id( WORD src_ip, WORD dst_ip, WORD id);
struct REQ_ID *find_req_id( WORD src_ip, WORD id);
void cleanup_req_id_queue();
void flush_req_id_queue();
//////////////////////////////////////////////////////////////////////////////////////////////////////

//aodv task function
//////////////////////////////////////////////////////////////////////////////////////////////////////
void nwk_proc_rreq( WORD prev_ip, RREQ* request, unsigned int length);
void nwk_proc_rrep( WORD prev_ip, RREP *reply, unsigned int length);
void nwk_proc_rerr( WORD prev_ip, RERR* error, unsigned int length);
void nwk_proc_rrep_ack( WORD prev_ip, RREP_ACK* ack, unsigned int length);
//////////////////////////////////////////////////////////////////////////////////////////////////////

void send_rreq( WORD dest_ip);
void forward_rreq( RREQ *request);
void send_rrep( RREQ * request);
void forward_rrep( RREP *reply);
void send_rrep_ack( WORD dest_ip);
void gen_rerr( WORD break_ip);
void send_rerr(struct Err_Dest_List *err_list, WORD dest_count);

BOOL seq_less_or_equal(WORD seq_one, WORD seq_two);
BOOL seq_greater(WORD seq_one,WORD seq_two);

void init_route();

BOOL is_route_discovered(SHORT_ADDR);

#endif
