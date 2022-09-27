#include "zAPS.h"

// Various APS frame header flags.
#define APS_FRAME_TYPE_MASK         0x03
#define APS_FRAME_DATA              0x00
#define APS_FRAME_COMMAND           0x01
#define APS_FRAME_ACKNOWLEDGE       0x02

#define APS_DELIVERY_MODE_MASK      (0x0C)
#define APS_DELIVERY_DIRECT         (0x00 << 2)
#define APS_DELIVERY_INDIRECT       (0x01 << 2)
#define APS_DELIVERY_BROADCAST      (0x02 << 2)
#define APS_DELIVERY_RESERVED       (0x03 << 2)

#define APS_INDIRECT_ADDRESS_MODE_MASK          0x10
#define APS_INDIRECT_ADDRESS_MODE_TO_COORD      0x10
#define APS_INDIRECT_ADDRESS_MODE_FROM_COORD    0x00
#define APS_CLEAR_INDIRECT_ADDRESS_MODE         0xEF

#define APS_DESTINATION_EP_PRESENT      0
#define APS_SOURCE_EP_PRESENT           1

#define APS_SECURITY_MASK           (0x20)
#define APS_SECURITY                (0x20)

#define APS_ACK_REQ_MASK            (0x40)
#define APS_ACK_REQ                 (0x40)

#define BROADCAST_EP                0xff

#define APSIsAcknowledgeFrame() ((apsCurrentFrame.frameCON.Val & APS_FRAME_TYPE_MASK) == APS_FRAME_ACKNOWLEDGE)

#define APSIsCommandFrame()     ((apsCurrentFrame.frameCON.Val & APS_FRAME_TYPE_MASK) == APS_FRAME_COMMAND)

#define APSIsDataFrame()        ((apsCurrentFrame.frameCON.Val & APS_FRAME_TYPE_MASK) == APS_FRAME_DATA)

#define APSAckIsRequested()     (apsCurrentFrame.frameCON.bits.ackRequested)

APS_HEADER      apsCurrentFrame;                // Current APS frame header

EP              epList[NUM_DEFINED_ENDPOINTS];  // Array of endpoints

EP              *pCurrentEP;                    // Pointer to current endpoint for faster access.

MESSAGE_INFO    currentMessageInfo;             // One generic message packet.

static TRANS_ID apsTransId;						// Ascending transaction id for APS frames.

//z内部函数
static BOOL APSGetHeader(void);

static void APSPutHeader(NODE_INFO *dest, BYTE frameCON, SHORT_ADDR dest_nwk_addr);

static BOOL SetEPDataReady(BYTE ep);

static void APSFlushEPs(void);

//内部函数（宏定义）
#define APSPut(a)                   NWKPut(a)

#define APSPutArray(v, n)           NWKPutArray(v, n)

// Calls MAC module to check ack status of frame associated with currnt ep.
#define APSIsConfirmed()            NWKFrameIsAcked(pCurrentEP->hFrame)

// Calls MAC module to check timeout status of frame associated with currnt ep.
#define APSIsTimedOut()             NWKFrameIsTimedOut(pCurrentEP->hFrame)

//Calls MAC module to remove frame entry for current ep.
#define APSRemoveFrame()            NWKFrameRemove(pCurrentEP->hFrame)

//send a frame of the current ep.
#define APSSend()               pCurrentEP->hFrame = NWKFlush()

//return the number of bytes remaining to be fetched for this EP.
#define APSGetDataLen()             (NWKPayloadLength)

/*********************************************************************
 * Function:        void APSInit(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Initializes APS variables.
 *
 * Note:            None
 ********************************************************************/
void APSInit(void)
{
    BYTE i;

    // Zero out ep list.
    memset((void*)epList, 0x00, (size_t)(sizeof(epList)));

    // Clear out all aps flags.
    apsCurrentFrame.Flags.Val = 0x00;

    // Disable all endpoints.
    for (i=0; i<NUM_DEFINED_ENDPOINTS; i++)
    {
        epList[i].flags.Val = 0x00;
		
    }

    NWKInit();
}

/*********************************************************************
 * Macro:           void APSDisable(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Forgets about all EP transactions and prepares for
 *                  stack disable mode.
 *
 * Note:            None
 ********************************************************************/
void APSDisable(void)
{
	APSFlushEPs(); 
	apsCurrentFrame.Flags.bits.bIsInProcess = FALSE;
	NWKDisable();
}	

/*********************************************************************
 * Function:        void APSFlushEPs(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Clears out all EPs from any outstanding transmit/
 *                  receive operations.
 *
 * Note:            None
 ********************************************************************/
static void APSFlushEPs(void)
{
    BYTE i;

    for ( i = 0; i < sizeof(epList)/sizeof(epList[0]); i++ )
    {
        epList[i].flags.bits.bIsDataReady = FALSE;
    }

    // Clear out all aps flags.
    apsCurrentFrame.Flags.Val = 0x00;
}

/*********************************************************************
 * Function:        BOOL APSTask(void)
 *
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          TRUE if there is no outstanding states to execute
 *                  FALSE otherwise
 *
 * Side Effects:    None
 *
 * Overview:        This is a main APS task. It must be called
 *                  as many times as possible to keep APS module
 *                  running.
 *
 * Note:            None
 ********************************************************************/
BOOL APSTask(void)
{
	 NWKTask();

     BOOL    destroyFrame = TRUE;
     BYTE    frameCon;
	
    if ( NWKIsGetReady() )
    {
        // Proceed only if we have not yet prcessed this frame.
        if ( apsCurrentFrame.Flags.bits.bIsInProcess )
            return TRUE;

        // APS layer only handles NWK DATA.  Make sure nothing else
        // got through to us.
        if ( !MACIsData() )	//NWKIsData()
        {
            return TRUE;
        }

        // Make sure that this is a valid APS frame.
        if ( APSGetHeader() )
        {
            // Handle APS command frames.
            if (APSIsCommandFrame())
            {
                // No APS commands are defined in this version
            }
            // Handle APS acknowledge frames.
            else if (APSIsAcknowledgeFrame())
            {
                // No APS acknoledge are defined in this version
            }
            // Handle APS data frames.
            else if (APSIsDataFrame())
            {
                if ( apsCurrentFrame.deliveryMode == APS_DELIVERY_DIRECT )
                {
					          
                    if ( SetEPDataReady(apsCurrentFrame.destEP) )
                    {
					   	ConsolePutString(" ep receive  direct data\n");
                        //du 当前版本不支持应用层ACK
                        if (APSAckIsRequested())
                        {
                        }
                        return TRUE;
                    }
                    else
                    {
                        ConsolePutString("Unknown EP data received.\r\n");
                    }
                }
				//du 当前版本不支持应用层广播
				//du 当前版本不支持应用层间接发送
            }
        }

        if (destroyFrame)
        { 
			APSDiscardRx();
        }
    }

    return TRUE;
}

/*********************************************************************
 * Function:        EP_HANDLE APSOpenEP(BYTE endPoint )
 *
 * PreCondition:    APSInit() is called
 *
 * Input:           endPoint    - endpoint number to enable
 *
 * Output:          handle to EP if there was one available EP
 *
 * Side Effects:    None
 *
 * Overview:        Enables the selected endpoint to receive data, and
 *                  returns a handle to the endpoint information.  Also
 *                  updates pCurrentEP to point to the newly enabled
 *                  endpoint.
 *
 * Note:            None
 ********************************************************************/
EP_HANDLE APSOpenEP(BYTE endPoint)
{
    BYTE                    i;
    //NODE_SIMPLE_DESCRIPTOR  simpleDescriptor;

    // Setup pointer to EP list.
    pCurrentEP = (EP*)&epList[0];

    // Find the matching endpoint information
    for ( i=0; i < NUM_DEFINED_ENDPOINTS; i++, pCurrentEP++)
    {
        //ProfileGetSimpleDesc( &simpleDescriptor, i );
        //if ( simpleDescriptor.Endpoint == endPoint )
		if (i == endPoint)
        {
            pCurrentEP->flags.Val = 0x00;
            pCurrentEP->flags.bits.bIsInUse = TRUE;

            return i;
        }
    }

    // No EP entry found.
    return EP_HANDLE_INVALID;
}

/*********************************************************************
 * Function:        TRANS_ID APSDE_DATA_request( MESSAGE_INFO *pMessageInfo )
 *
 * PreCondition:    APSInit() is called
 *
 * Input:           pMessageInfo    - pointer to a message information
 *                                      structure
 *
 * Output:          If successful, the transaction ID of the message is
 *                  returned.  Otherwise, TRANS_ID_INVALID is returned.
 *
 * Side Effects:    None
 *
 * Overview:        This function is used to send a KVP, MSG or other frame.
 *
 * Note:            None
 ********************************************************************/
TRANS_ID APSDE_DATA_request( MESSAGE_INFO *pMessageInfo )
{
    BYTE apsFlags;

    apsCurrentFrame.srcEP = pMessageInfo->srcEP;
    apsCurrentFrame.clusterID = pMessageInfo->clusterID;
    apsCurrentFrame.profileID.Val = pMessageInfo->profileID;
	//du 当前版本不支持应用层广播    
	if (pMessageInfo->destAddress.shortAddr.Val == 0xffff)
		return TRANS_ID_INVALID;

	//du 查找目标节点路由
	struct Route_Table_Entry *route = lookup_route(pMessageInfo->destAddress.shortAddr.Val );

	if (route == NULL || !route->flag.bit.is_route_valid)
	{
		//du 路由查找		
		send_rreq( pMessageInfo->destAddress.shortAddr.Val);
			

		
		if (is_route_discovered(pMessageInfo->destAddress.shortAddr))
		{
			ConsolePutString("route discover finished\n");
			route = lookup_route(pMessageInfo->destAddress.shortAddr.Val );
		}
		else
		{
			ConsolePutString("route discover failed\n");
			return TRANS_ID_INVALID;
		}
	}
	macDestInfo.addrMode = MAC_DST_SHORT_ADDR;
	macDestInfo.panID = macInfo.panID;
	macDestInfo.shortAddr.Val = route->next_ip;
	ConsolePutString("shortAddr:");//test lyj
	ConsolePutInitData(macDestInfo.shortAddr.byte.MSB,macDestInfo.shortAddr.byte.LSB);//test lyj
	
	//du 当前版本不支持应用间接发送
    if ( !pMessageInfo->flags.bits.bModeIsDirect )
    {
		return TRANS_ID_INVALID;
    }
     
	apsFlags = APS_FRAME_DATA | APS_DELIVERY_DIRECT;
    apsCurrentFrame.destEP = pMessageInfo->destEP;
	//du 当前版本不支持应用层应答
    if (pMessageInfo->flags.bits.bAckRequested)
    {
		return TRANS_ID_INVALID;
    }

    APSPutHeader( &macDestInfo, apsFlags , pMessageInfo->destAddress.shortAddr);
    
    if (pMessageInfo->transactionID == TRANS_ID_INVALID)
    {
        apsTransId++;
        if ( apsTransId == TRANS_ID_INVALID )
            apsTransId++;
        APSPut(apsTransId);
    }
    else
    {
        APSPut( pMessageInfo->transactionID );
    }

    if (pMessageInfo->frameType == KVP_FRAME)
    {
        APSPut((BYTE)((BYTE)pMessageInfo->command | (BYTE)pMessageInfo->dataType));
        APSPut(pMessageInfo->attribID.byte.LSB);
        APSPut(pMessageInfo->attribID.byte.MSB);
    }
    else if (pMessageInfo->frameType == MSG_FRAME)
    {
        APSPut(pMessageInfo->dataLength);
    }

    APSPutArray( pMessageInfo->dataPointer, pMessageInfo->dataLength );

    APSSend();
	//du 是否应答
	while (1){
		APSTask();
		if(APSIsConfirmed())
		{
			APSRemoveFrame();
			ConsolePutString("send successfully\n");
			return apsTransId;
		}
		else if( APSIsTimedOut()){
			APSRemoveFrame();
			//du 发送的数据包没有应答则产生路由错误
			gen_rerr(route->next_ip);
			ConsolePutString("send failed\n");
			return TRANS_ID_INVALID;	
		}
	}    
}

/*********************************************************************
 * Function:        static void APSPutHeader(NODE_INFO *dest,
 *                                           BYTE frameCON)
 *
 *
 * PreCondition:    APSIsPutReady() = TRUE
 *
 * Input:           dest        - destination node address info
 *                  frameCON    - APS frame control info
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Construct new APS frame header and load it in
 *                  RF TX buffer.
 *
 * Note:            None
 ********************************************************************/
static void APSPutHeader(NODE_INFO *dest, BYTE frameCON, SHORT_ADDR dest_nwk_addr)
{
    BYTE deliveryMode;

    // Load NWK header.
    NWKPutHeader(dest,
                NWK_FRAME_DATA |
                NWK_PROT_VER,
                0x00,
				dest_nwk_addr);

#ifdef ROUTE_MONITOR
	struct Route_Table_Entry *route = lookup_route(dest_nwk_addr.Val);
	APSPut(1);
	APSPut(macInfo.shortAddr.byte.LSB);
#endif
    // And frame control byte.
    APSPut(frameCON);

    // If this is a command frame, then the APS header contains no
    // more information, and we are done.
    if ( (frameCON & APS_FRAME_TYPE_MASK) == APS_FRAME_COMMAND )
        return;

    // Extract current delivery mode.
    deliveryMode = frameCON & APS_DELIVERY_MODE_MASK;

    #ifdef I_AM_COORDINATOR
        // Packets from the coordinator always have the destination EP.
        APSPut(apsCurrentFrame.destEP);
    #else
    // If we're an end device, only direct and broadcast frames will have
    // a destination EP.
    if ( deliveryMode == APS_DELIVERY_DIRECT ||
         deliveryMode == APS_DELIVERY_BROADCAST )
    {
        APSPut(apsCurrentFrame.destEP);
    }
	#endif

    // Put the cluster ID.
    APSPut(apsCurrentFrame.clusterID);

    // Put the profile ID.
    APSPut(apsCurrentFrame.profileID.byte.LSB);
    APSPut(apsCurrentFrame.profileID.byte.MSB);

    // Put the source EP only if it is present.  It will be present UNLESS the
    // delivery mode is indirect and the destination address is present.
    if (!((deliveryMode == APS_DELIVERY_INDIRECT) &&
          (((APS_FRAME_CON*)&frameCON)->bits.indirectAddressMode == APS_DESTINATION_EP_PRESENT )))
    {
        APSPut(apsCurrentFrame.srcEP);
    }
}

/*********************************************************************
 * Function:        static BOOL SetEPDataReady(BYTE ep)
 *
 *
 * PreCondition:    None
 *
 * Input:           ep          - Endpoint that is to be matched.
 *
 * Output:          TRUE if given EP found and there is no overflow
 *                  FALSE otherwise
 *
 * Side Effects:    None
 *
 * Overview:        This function looks up the list of opened
 *                  end points and tries to find the indicated endpoint.
 *                  If it finds it, it sets the bIsDataReady flag.
 *                  If the broadcast endpoint is passed in, all open
 *                  endpoints except the ZDO endpoint are set to ready.
 *                  If at least one endpoint could accept the data
 *                  without overflow, TRUE is returned.
 *
 * Note:            None
 ********************************************************************/
static BOOL SetEPDataReady( BYTE ep )
{
    BYTE    i;
    BOOL    status = FALSE;
	//NODE_SIMPLE_DESCRIPTOR  simpleDescriptor;
    for (i=0; i<NUM_DEFINED_ENDPOINTS; i++ )
    {
        if (epList[i].flags.bits.bIsInUse)
        {
			
			//ProfileGetSimpleDesc( &simpleDescriptor, i );
			//if ((simpleDescriptor.Endpoint == ep) ||
                //((ep == BROADCAST_EP) && (simpleDescriptor.Endpoint != 0)))
			//不支持应用层广播
			if (i == ep)
            {
                if ( epList[i].flags.bits.bIsDataReady == 1 )
                {
                    ConsolePutString("APS: An EP has overflowed.\r\n");
                    epList[i].flags.bits.bIsOverflow = 1;
                }
                else
                {
                    epList[i].flags.bits.bIsDataReady = 1;
              
					status = TRUE;
                }
            }
        }
    }
    return status;
}

/*********************************************************************
 * Function:        static BOOL APSGetHeader(void)
 *
 *
 * PreCondition:    NWKGetHeader() is already
 *                  called.
 *
 * Input:           None
 *
 * Output:          TRUE RX buffer contains valid APS frame.
 *                  FALSE, otherwise
 *
 * Side Effects:    None
 *
 * Overview:        Fetch rx buffer bytes and check to see if
 *                  it is a valid APS frame header.
 *
 * Note:            None
 ********************************************************************/
static BOOL APSGetHeader(void)
{
    // At this point, there must be some data paylond in the frame.
    if ( APSGetDataLen() < 1 )
        return FALSE;

    // Fetch frame control
    apsCurrentFrame.frameCON.Val = APSGet();

    // Extract and remember delivery mode.
    apsCurrentFrame.deliveryMode = apsCurrentFrame.frameCON.Val & APS_DELIVERY_MODE_MASK;

    // Make sure that deliveryMode is not using reserved mode.
    if (apsCurrentFrame.deliveryMode == APS_DELIVERY_RESERVED)
        return FALSE;

    // Destination EP will be present UNLESS the delivery mode is indirect and the source
    // address is present.
    if (!((apsCurrentFrame.deliveryMode == APS_DELIVERY_INDIRECT) &&
          (apsCurrentFrame.frameCON.bits.indirectAddressMode == APS_SOURCE_EP_PRESENT )))
    {
        apsCurrentFrame.destEP = APSGet();
    }

    // Data and APS acknowledge frames will contain the cluster ID.
    if ( ((apsCurrentFrame.frameCON.Val & APS_FRAME_TYPE_MASK) == APS_FRAME_DATA ) ||
         ((apsCurrentFrame.frameCON.Val & APS_FRAME_TYPE_MASK) == APS_FRAME_ACKNOWLEDGE ) )
    {
        apsCurrentFrame.clusterID = APSGet();
    }

    // Profile ID is now always sent.
    apsCurrentFrame.profileID.byte.LSB = APSGet();
    apsCurrentFrame.profileID.byte.MSB = APSGet();

    // Get source endpoint byte, if present.  If will be present UNLESS the
    // delivery mode is indirect and the destination endpoint is present.
    if (!((apsCurrentFrame.deliveryMode == APS_DELIVERY_INDIRECT) &&
          (apsCurrentFrame.frameCON.bits.indirectAddressMode == APS_DESTINATION_EP_PRESENT )))
    {
        apsCurrentFrame.srcEP = APSGet();
    }

    // Remember that we have already processed this frame.
    apsCurrentFrame.Flags.bits.bIsInProcess = TRUE;

    apsCurrentFrame.payload = NWKPayload;

    return TRUE;
}

/*********************************************************************
 * Function:        void APSDiscard(void)
 *
 * PreCondition:    APSSetEP() is called with valid EP handle.
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Clears internal flag indicating that this EP
 *                  is done processing the current frame.  If all
 *                  endpoints are done with the current frame, the
 *                  frame is released.
 *
 * Note:            None
 ********************************************************************/
void APSDiscardRx( void )
{
    EP      *epPtr;
    BYTE    i;

    pCurrentEP->flags.bits.bIsDataReady = FALSE;
    NWKPayload = apsCurrentFrame.payload;
    NWKPayloadLength = NWKPayloadLengthSave;

    // See if all of the endpoints are done with the frame.  If any
    // endpoint still needs this packet, return.
    epPtr = (EP*)&epList[0];
    for ( i =0; i < NUM_DEFINED_ENDPOINTS; i++, epPtr++)
    {
        if (epPtr->flags.bits.bIsDataReady)
        {
            return;
        }
    }

    // All of the endpoints are done with the frame, so release it.
    NWKDiscardRx();
    apsCurrentFrame.Flags.bits.bIsInProcess = FALSE;
	ConsolePutString("discard\n");
}










