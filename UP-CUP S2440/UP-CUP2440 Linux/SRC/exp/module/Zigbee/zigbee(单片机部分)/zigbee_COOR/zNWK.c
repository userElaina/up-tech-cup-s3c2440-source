#include "zNWK.h"

SM_NWK      smNWK;                  // NWK state machine state

NWK_HEADER   nwkCurrentFrame;        // Current NWK frame header

NWK_FLAGS   _NWKFlags;              // NWK module flags

BYTE* NWKPayload;

BYTE* NWKPayloadHead;

BYTE NWKPayloadLength;

BYTE NWKPayloadLengthSave;

////////////////////////////////////
//路由信息
PATH_INFO path_info[MAX_NODE];

PATH_INFO nwk_get_path_info(BYTE dst)
{
	return path_info[dst];
}
////////////////////////////////////

static BOOL NWKGetHeader(void);

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
void NWKInit(void)
{
	
	_NWKFlags.Val = 0x00;
    // Clear out all NWK flags.
    nwkCurrentFrame.Flags.Val = 0x00;


    NWKPayload = NULL;
	//初始化路由
	init_route();
		   
    MACInit(); 
}

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
BOOL NWKTask(void)
{
//	ConsolePutString(__FUNCTION__);
//	ConsolePutString("\r\n");
	MACTask();

    if ( !MACIsGetReady() )
    {
        return TRUE;
    }

    
	if ( nwkCurrentFrame.Flags.bits.bIsInProcess ){
        ConsolePutString("nwk data not handled\n");
		return TRUE;
	}

    
	 if ( MACIsData() )
    {
        if ( macState.bits.bIsAssociated )
        {
            if ( NWKGetHeader() )
            {
				if (NWKIsCommand())
				{
					BYTE command = NWKGet();
					
					switch (command){
						case NWK_CMD_ROUTE_REQUEST:
							#ifdef route_debug
								if (macCurrentFrame.src.shortAddr.Val == 2 && macInfo.shortAddr.Val == 0){
									ConsolePutString("filter route request\n");
									break;
								}
							#endif
							nwk_proc_rreq( macCurrentFrame.src.shortAddr.Val, (RREQ*) NWKPayload, NWKPayloadLength);
							break;
						case NWK_CMD_ROUTE_REPLY:
							nwk_proc_rrep( macCurrentFrame.src.shortAddr.Val, (RREP*) NWKPayload, NWKPayloadLength);
							break;
						case NWK_CMD_ROUTE_ERROR:nwk_proc_rerr( macCurrentFrame.src.shortAddr.Val, (RERR*) NWKPayload, NWKPayloadLength);
							break;
						case NWK_CMD_ROUTE_ACK:nwk_proc_rrep_ack( macCurrentFrame.src.shortAddr.Val, (RREP_ACK*) NWKPayload, NWKPayloadLength);
							break;
						default:
							break;
					}
					 NWKDiscardRx();
				}

				else if (NWKIsData()){
					if (nwkCurrentFrame.destAddr.Val == macInfo.shortAddr.Val){
#ifdef ROUTE_MONITOR
						/*
					//ConsolePutString("alex");
					//ConsolePut(0);
					//ConsolePut((BYTE)nwkCurrentFrame.destAddr.byte.MSB+48);
					ConsolePut((BYTE)nwkCurrentFrame.destAddr.byte.LSB);
					//ConsolePut((BYTE)nwkCurrentFrame.srcAddr.byte.MSB+48);
					ConsolePut((BYTE)nwkCurrentFrame.srcAddr.byte.LSB);
					BYTE hop_count = NWKGet();
					ConsolePut(hop_count);
					while(hop_count){
						ConsolePut(NWKGet());
						hop_count--;
					}

					ConsolePutString("end");
					ConsolePut(0);
					*/
					ConsolePut(01);
					ConsolePut(2);
				
					ConsolePut(03);
					ConsolePut(04);
					ConsolePut(05);
					BYTE metric = NWKGet();
					BYTE index = NWKGet();
							
					path_info[index].metric = metric-1;
					path_info[index].src = index;
					ConsolePut(path_info[index].src);
					path_info[index].dst = nwkCurrentFrame.destAddr.byte.LSB;
					ConsolePut(nwkCurrentFrame.destAddr.byte.LSB);
					
				    metric--;
					ConsolePut(metric);
					while (metric){
						path_info[index].path[path_info[index].metric - metric] = NWKGet();
						ConsolePut(path_info[index].path[path_info[index].metric - metric]);
						metric--;
						
						ConsolePut(path_info[index].path[path_info[index].metric - metric]);
					}
									ConsolePut(06);
					ConsolePut(7);
				
					ConsolePut(8);
					ConsolePut(9);
					ConsolePut(10);
					
#endif
						//du 到达数据，由应用层处理
						return TRUE;
					}
					else
					{	
						//du 转发数据
						struct Route_Table_Entry *route = lookup_route( nwkCurrentFrame.destAddr.Val);
						NWKDiscardRx();
						if (route == NULL || !route->flag.bit.is_route_valid)
						{
							//  不用产生路由错误
						
						}
						else
						{	
							
							macDestInfo.addrMode = MAC_DST_SHORT_ADDR;
							macDestInfo.panID = macInfo.panID;
							macDestInfo.shortAddr.Val = route->next_ip;

							NWKPutHeader(&macDestInfo, NWK_FRAME_DATA | NWK_PROT_VER, 0x00, nwkCurrentFrame.destAddr);
#ifdef ROUTE_MONITOR
							BYTE hop_count = NWKGet();
							  ConsolePut(1);
							  ConsolePut(2);
							  ConsolePut(3);
							  ConsolePut(hop_count);

							NWKPut(hop_count+1);
							BYTE temp;
							while (hop_count){
								temp = NWKGet();
								NWKPut(temp);
								ConsolePut(temp);
								hop_count--;
							}
							NWKPut(macInfo.shortAddr.byte.LSB);
							ConsolePut(macInfo.shortAddr.byte.LSB);
							  ConsolePut(1);
							  ConsolePut(2);
							  ConsolePut(3);
#endif
							NWKPutArray(NWKPayload, NWKPayloadLength);
							HFRAME forward_frame = NWKFlush();
							ConsolePut(48+forward_frame);
							ConsolePutString("forward data\n");
							while (1){
								MACTask();
								if(MACFrameIsAcked(forward_frame)){
									MACFrameRemove(forward_frame);
									ConsolePutString("forward send successfully\n");
									break;
								}
								else if(MACFrameIsTimedOut(forward_frame)){
									//du 产生路由错误
									ConsolePutString("forward send failed\n");
									gen_rerr(route->next_ip);
									MACFrameRemove(forward_frame);
									break;	
								}
							}
						}
						
					}
				}
            
			}
        } 
    }

    return TRUE;
}

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
void NWKStartDiscovery(void)
{
    MACSetFirstChannel();

    smNWK = SM_NWK_ENERGY_SCAN;

    _NWKFlags.bits.bIsNWKDiscovered = FALSE;
    ConsolePutString("NWK: NWKStartDiscovery() \n");

}

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
BOOL NWKIsDiscoveryComplete(void)
{
    BYTE energy;

    switch(smNWK)
    {
    case SM_NWK_ENERGY_SCAN:
        // Check with application to see if this channel should be used.
        // If application does not want to use this channel, we will stay in this state
        // and next channel will be selected.
      if ( !AppOkayToUseChannel(MACGetChannel()) )
        {
            smNWK = SM_NWK_NEXT_CHANNEL;
            break;
        }

        MACStartED();
        smNWK = SM_NWK_ENERGY_SCAN_WAIT;
        break;

	case SM_NWK_NEXT_CHANNEL:
        if ( !MACSetNextChannel() )
        {
            ConsolePutString("NWK: NWKIsDiscoveryComplete(): No channel is free.\r\n");
            SetZError(ZCODE_CHANNEL_BUSY);
            PHYSetTRXState(PHY_TRX_OFF);
            return TRUE;
        }

        smNWK = SM_NWK_ENERGY_SCAN;

        break;


    case SM_NWK_ENERGY_SCAN_WAIT:
        if ( MACIsEDComplete() )
        {
           
            energy = MACGetEDValue();
			

            MACStartScan(TRUE);

			
			

			
            smNWK = SM_NWK_ACTIVE_SCAN_WAIT;

        }
        break;



    case SM_NWK_ACTIVE_SCAN_WAIT:
        if ( MACIsScanComplete() )
        {
           
            if ( MACIsPANAvailable() )
            {
                ConsolePutString("NWK: NWKIsDiscoveryComplete(): A network is found.\r\n");
                _NWKFlags.bits.bIsNWKDiscovered = TRUE;
            }

            else
                _NWKFlags.bits.bIsNWKDiscovered = FALSE;

            // Depending on whether this is coordinator or end device,
            // this function may get called to continue scanning next available channel.

            // A coordinator would want to scan for empty channel. If no network is discovered,
            // coordinator may form network and may not call this function again.

            // An end device needs to find an occupied channel. If no network is discovered,
            // end device would continue to scan and see if it can find a busy channel. In
            // that case, this function will be called again.

            // In any case, set the next state to NEXT_CHANNEL so that if this function is called
            // agian, it will switch to next channel and continue the discovery.

            SetZError(ZCODE_NO_ERROR);
            smNWK = SM_NWK_NEXT_CHANNEL;
            PHYSetTRXState(PHY_TRX_OFF);
            return TRUE;

        }
        break;

    }

    return FALSE;
}

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
void NWKPutHeader(NODE_INFO *dest,
                  BYTE frameCONLSB,
                  BYTE frameCONMSB,
				  SHORT_ADDR dest_nwk_addr)
{
    macInfo.addrMode = MAC_SRC_SHORT_ADDR;
	//du 如果是广播
    if(dest->shortAddr.Val==0xFFFF)
    {
            MACPutHeader(dest,
                     MAC_FRAME_DATA |
                     MAC_SECURITY |
                     MAC_FRAME_PENDING_NO |
                     MAC_ACK_NO |
                     MAC_INTRA_PAN_YES);
    }
    else
    {
      MACPutHeader(dest,
                 MAC_FRAME_DATA |
                 MAC_SECURITY |
                 MAC_FRAME_PENDING_NO |
                 MAC_ACK_YES |
                 MAC_INTRA_PAN_YES);
    }

    // Write Network Frame Control field
    // The frameCONLSB must include NWK_PROT_VER by the caller
    // (APSPutHeader)
    MACPut(frameCONLSB);
    MACPut(frameCONMSB);

    // Write 16 bit destination and 16 bit source network addresses
    // (same as 16 bit short IEEE 802.15.4 addresses)
	MACPut(dest_nwk_addr.byte.LSB);
    MACPut(dest_nwk_addr.byte.MSB);
    MACPut(MACGetShortAddrLSB());
    MACPut(MACGetShortAddrMSB());

    // Write broadcast radius and broadcast sequence number fields.
    // These are mandatory for all addresses now.
    MACPut(nwkCurrentFrame.broadcastRadius);
    MACPut(nwkCurrentFrame.broadcastSequence);
}

/*********************************************************************
 * Function:        BOOL NWKGetHeader(void)
 *
 * PreCondition:    NWKIsGetReady() = TRUE
 *
 * Input:           None
 *
 * Output:          TRUE if a valid NWK frame was received
 *
 * Side Effects:    None
 *
 * Overview:        Fetches and validates already received frame
 *                  against NWK frame rules.
 *
 * Note:            None
 ********************************************************************/
static BOOL NWKGetHeader(void)
{
    // A valid NWK frame must have at least six bytes (2 frame control,
    // 2 destination address, 2 source address) in its payload.
    if ( macCurrentFrame.frameLength < 6 )
        return FALSE;

    nwkCurrentFrame.frameCONLSB.Val = MACGet();
    nwkCurrentFrame.frameCONMSB.Val = MACGet();

    // Make sure that this frame version is supposed.
    if ( (nwkCurrentFrame.frameCONLSB.Val & NWK_PROT_VER_MASK) != NWK_PROT_VER)
        return FALSE;

    // TODO: Implement security capabilities so we don't have to
    // discard all frames with security enabled.
    if ( nwkCurrentFrame.frameCONMSB.bits.security )
        return FALSE;

    // Get 16 bit destination NWK address.  In ZigBee 1.0, this is
    // always the same as the IEEE 802.15.4 short address.
    nwkCurrentFrame.destAddr.byte.LSB = MACGet();
    nwkCurrentFrame.destAddr.byte.MSB = MACGet();

    // Get 16 bit source NWK address.  In ZigBee 1.0, this is
    // always the same as the IEEE 802.15.4 short address.
    nwkCurrentFrame.srcAddr.byte.LSB = MACGet();
    nwkCurrentFrame.srcAddr.byte.MSB = MACGet();

    // Retreive the broadcast radius and broadcast sequence number
    // fields for all destination addresses.
    // Retrieve broadcast radius
    nwkCurrentFrame.broadcastRadius = MACGet();

    // Retrieve broadcast sequence number
    nwkCurrentFrame.broadcastSequence = MACGet();

    // Remember that we have are in process of this frame.
    nwkCurrentFrame.Flags.bits.bIsInProcess = TRUE;

    if(NWKPayload != NULL)
    {
       free(NWKPayloadHead);
        NWKPayloadLength = 0;
    }
    NWKPayload = (BYTE *)malloc(macCurrentFrame.frameLength);
    if(NWKPayload == NULL)
    {
        //TODO(DF20): if null then destroy packet, mark as discarded and exit
    }
    NWKPayloadHead = NWKPayload;
    NWKPayloadLength = macCurrentFrame.frameLength;
    NWKPayloadLengthSave = macCurrentFrame.frameLength;
    while(macCurrentFrame.frameLength)
    {
        *NWKPayload++=MACGet();
    }

    // Account for CRC/(RSSI + CORRELATION for CC2420)
    MACGet();
    MACGet();

    if(nwkCurrentFrame.destAddr.Val == 0xFFFF)
    {
    }
    NWKPayload = NWKPayloadHead;
    return TRUE;
}

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
BYTE NWKGet(void){
    NWKPayloadLength--;
    return *NWKPayload++;
}

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
BYTE NWKGetArray(BYTE *v, BYTE n){
    BYTE count = 0;
    while(((n--)>0)&&(NWKPayloadLength>0))
    {
        *v++=*NWKPayload++;
        NWKPayloadLength--;
        count++;
    }
    return count;
}
#if defined(I_AM_COORDINATOR)
	/*********************************************************************
	 * Function:        BOOL NWKIsCoordInitComplete(void)
	 *
	 * PreCondition:    None
	 *
	 * Input:           TRUE if NWK Coordinator is complete
	 *                  FALSE, otherwise
	 *
	 * Output:          None
	 *
	 * Side Effects:    None
	 *
	 * Overview:        Performs coordinaor initialization process.
	 *
	 * Note:            Available for coorindator only.
	 ********************************************************************/
	BOOL NWKIsCoordInitComplete(void)
	{
		// Check to see if discovery is complete.
		if ( NWKIsDiscoveryComplete()){  
			// Terminate this function if there was an error.
			if ( GetLastZError() != ZCODE_NO_ERROR )
				return TRUE;

			// There was no error. Now check to see if
			// any coordinator was discovered.
			else if (  !NWKIsDiscovered() )
			{
				// Terminate this function.
				return TRUE;
			}
		}

		// we are still not done.
		return FALSE;
	}

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
	void NWKForm(void)
	{
		// There was no coordinator in current channel.
		// We can establish network in this channel.

		// Select unique random network
		MACSetPANIdLSB(RANDOM_LSB);
		MACSetPANIdMSB(RANDOM_MSB&0x3F);    //PAN ID only allowed 0x0000-0x3FFF per NWK tables 10,11,17 and 20
		//alex for debug
		MACSetPANIdLSB(0x55);
		MACSetPANIdMSB(0x55);

		// Coordinator gets short address of 0x00.
		MACSetShortAddrLSB(0x00);
		MACSetShortAddrMSB(0x00);

		MACUpdateAddressInfo();

	   

		// Mark that the network is formed.
		MACStart();
		MACFormNetwork();

	}

#endif

//路由
struct Route_Table_Entry *route_table;
struct REQ_ID *req_id_queue;

void init_route()
{
	
		init_route_table();
		init_req_id_queue();
	
}

BOOL is_route_discovered(SHORT_ADDR dest){
	TICK route_tick = TickGet();				
	while(1){
		NWKTask();
		if (TickGetDiff(TickGet(), route_tick) >= NWK_ROUTE_DISCOVER_PERIOD){
			break;
		}
	}
	
	struct Route_Table_Entry *route = lookup_route(dest.Val);
	if (route != NULL && route->flag.bit.is_route_valid){
		return TRUE;	
	}
	else {
		return FALSE;
	}

}

void nwk_proc_rreq( WORD prev_ip, RREQ* request, unsigned int length){	

	//todo: 建立到上一跳节点的无效序列号路由
	
	
	if (!find_req_id( request->source_ip, request->req_id)){
		ConsolePutString("receive route request\n");
		insert_req_id( request->source_ip, request->dest_ip, request->req_id);

		request->hopCount++;

		update_route( request->source_ip, prev_ip, request->sourceSeq, request->hopCount);
		
		if (macInfo.shortAddr.Val == request->dest_ip){
			//目标节点应答
			send_rrep( request);

		}
		/*else if(have_valid_dest_route(route_table, request->dest_ip,
			request->dest_seq) != NULL && !request->flag.bit.d){
			//中间结点应答
			
		}*/
		else{
			//转发RREQ
			struct	Route_Table_Entry * dest_route = lookup_route( request->dest_ip);
			if (dest_route!=NULL && dest_route->flag.bit.is_route_seq_valid){
				if (seq_greater(dest_route->dest_seq, request->dest_seq)){
					request->dest_seq = dest_route->dest_seq;
				}
			}
			forward_rreq( request);
		}
	}
}

void nwk_proc_rrep( WORD prev_ip, RREP *reply, unsigned int length)
{
	//todo:建立到上一跳的无效序列号路由
	ConsolePutString("receive route reply\n");
	
	if (reply->flag.bit.a){
		send_rrep_ack( prev_ip);
	}
	reply->hopCount++;
	update_route( reply->dest_ip, prev_ip, reply->dest_seq, reply->hopCount);

	if (macInfo.shortAddr.Val == reply->source_ip){
		//du 路有查找成功，通知上层应用
		ConsolePutString("we have valid route\n");
	}
	else
	{
		forward_rrep( reply);
	}
}

void nwk_proc_rerr( WORD prev_ip, RERR* error, unsigned int length){
	struct Route_Table_Entry *tmp_route = NULL;
  
	struct Err_Dest *tmp_rerr_dst = NULL;
	struct Err_Dest_List *outgoing_rerr_routes = NULL;
	struct Err_Dest_List *tmp_rerr_route = NULL;
    int num_outgoing_routes = 0;
    
	int i;
    ConsolePutString("receive route error\n");
    tmp_rerr_dst = (struct Err_Dest *) (error + 1);
   
	for ( i = 0; i<error->dest_count; i++)
    {

		tmp_route = lookup_route( tmp_rerr_dst->unreach_dest_ip);
		if (tmp_route && !tmp_route->flag.bit.is_self_route && tmp_route->flag.bit.is_route_valid)
        {
			tmp_route->flag.bit.is_route_valid = FALSE;
			tmp_route->dest_seq = tmp_rerr_dst->unreach_dest_seq; //convert to host format for local storage

							//create a temporary rerr route to use for the outgoing RERR
            if ((tmp_rerr_route = (struct Err_Dest_List *) malloc(sizeof(struct Err_Dest_List))) == NULL)
            {
                //ConsolePutString("malloc new Err_Dest_List error\n");
                return;
            }

            
            tmp_rerr_route->next = outgoing_rerr_routes;
            outgoing_rerr_routes = tmp_rerr_route;
			tmp_rerr_route->err_dest.unreach_dest_ip = tmp_rerr_dst->unreach_dest_ip;
			tmp_rerr_route->err_dest.unreach_dest_seq = tmp_rerr_dst->unreach_dest_seq; //Already in network format...
            num_outgoing_routes++;

        }
        tmp_rerr_dst = tmp_rerr_dst + 1;
     
    }
    if (num_outgoing_routes)
    {
        send_rerr( outgoing_rerr_routes, num_outgoing_routes);
    }
	//ConsolePutString("node%d forward RERR complete\n", node_index+1);
}

void nwk_proc_rrep_ack( WORD prev_ip, RREP_ACK* ack, unsigned int length){
	ConsolePutString("receive route reply ack\n");
}

void send_rreq( WORD dest_ip)
{
	RREQ * request = (RREQ *)malloc(sizeof(RREQ));
	if (request == NULL){
		//ConsolePutString("malloc new RREQ failed\n");
		return;
	}

	struct	Route_Table_Entry *dest_route = lookup_route( dest_ip);
	if (dest_route != NULL && dest_route->flag.bit.is_route_seq_valid){
		//如果存在到目标节点路由，并且序列号有效
		request->dest_seq = dest_route->dest_seq;
		request->flag.bit.u = TRUE;
	}else {
		request->dest_seq = 0;
		request->flag.bit.u = FALSE;
	}
	request->dest_ip  = dest_ip;
	request->type = AODV_RREQ;
	request->reserved = 0;
	request->flag.bit.j = FALSE;
	request->flag.bit.r = FALSE;
	request->flag.bit.d = TRUE;
	request->flag.bit.g = FALSE;
	request->hopCount = 0;

	struct	Route_Table_Entry * src_route = lookup_route( macInfo.shortAddr.Val);
	if (src_route == NULL){
		//ConsolePutString("send rreq failed because of no src_route");
		return;
	}
	request->source_ip = src_route->dest_ip;
	request->req_id = ++src_route->req_id;
	request->sourceSeq = ++src_route->dest_seq;
	//将req_id加入请求表，路由表的第一项是到本地的路由信息
	insert_req_id( macInfo.shortAddr.Val, request->dest_ip, request->req_id);

	//du 发送route_request包
	macDestInfo.addrMode = MAC_DST_SHORT_ADDR;
	macDestInfo.panID = macInfo.panID;
	macDestInfo.shortAddr.Val = 0xffff;
		
    NWKPutHeader(&macDestInfo,
            NWK_FRAME_CMD |
            NWK_PROT_VER,
            0x00,
			macDestInfo.shortAddr);
	NWKPut(NWK_CMD_ROUTE_REQUEST);
	NWKPutArray((BYTE *)request, (BYTE)sizeof(RREQ));
	NWKFlush();
	ConsolePutString("send route request\n");
}

void forward_rreq( RREQ *request){

	//du 转发route_request包
	macDestInfo.addrMode = MAC_DST_SHORT_ADDR;
	macDestInfo.panID = macInfo.panID;
	macDestInfo.shortAddr.Val = 0xffff;
		
    NWKPutHeader(&macDestInfo,
            NWK_FRAME_CMD |
            NWK_PROT_VER,
            0x00,
			macDestInfo.shortAddr);
	
    NWKPut(NWK_CMD_ROUTE_REQUEST);
	MACPutArray((BYTE *)request, (BYTE)sizeof(RREQ));
	NWKFlush();

	ConsolePutString("forward route request\n");
}

void send_rrep( RREQ * request)
{
	RREP *reply = (RREP*)malloc(sizeof(RREP));
	if (reply == NULL){
		//ConsolePutString("malloc new RREP failed\n");
		return;
	}
	reply->type = AODV_RREP;
	reply->flag.bit.a = FALSE;
	reply->flag.bit.r = FALSE;
	reply->reserved = 0;
	reply->prefix = 0;
	reply->dest_ip = request->dest_ip;
	struct	Route_Table_Entry *dest_route = lookup_route( request->dest_ip);
	if (seq_greater(request->dest_seq, dest_route->dest_seq)){
		dest_route->dest_seq = request->dest_seq+1;
	}
	reply->dest_seq = dest_route->dest_seq;
	reply->hopCount = 0;
	reply->source_ip = request->source_ip;
	reply->life_time = 0;//生存时间没定义


	macDestInfo.addrMode = MAC_DST_SHORT_ADDR;
	macDestInfo.panID = macInfo.panID;
	//du 到请求发起节点路径的下一跳
	macDestInfo.shortAddr.Val = lookup_route( request->source_ip)->next_ip;
	NWKPutHeader(&macDestInfo,
                NWK_FRAME_CMD |
                NWK_PROT_VER,
                0x00,
				macDestInfo.shortAddr);
	NWKPut(NWK_CMD_ROUTE_REPLY);
	NWKPutArray((BYTE *)reply, (BYTE)sizeof(RREP));
	HFRAME frame = NWKFlush();
	//du mac ask如何处理
							while (1){
								MACTask();
								if(MACFrameIsAcked(frame)){
									MACFrameRemove(frame);
									
									break;
								}
								else if(MACFrameIsTimedOut(frame)){
									//du 产生路由错误
									
									MACFrameRemove(frame);
									break;	
								}
							}			
	ConsolePutString("send route reply\n");
	
}

void forward_rrep(RREP *reply){
	macDestInfo.addrMode = MAC_DST_SHORT_ADDR;
	macDestInfo.panID = macInfo.panID;
	//du 到请求发起节点路径的下一跳
	macDestInfo.shortAddr.Val = lookup_route( reply->source_ip)->next_ip;
	
	NWKPutHeader(&macDestInfo,
                NWK_FRAME_CMD |
                NWK_PROT_VER,
                0x00,
				macDestInfo.shortAddr);
	NWKPut(NWK_CMD_ROUTE_REPLY);
	NWKPutArray((BYTE *)reply, (BYTE)sizeof(RREP));
	HFRAME frame = NWKFlush();
	//du mac ask如何处理
							while (1){
								MACTask();
								if(MACFrameIsAcked(frame)){
									MACFrameRemove(frame);
									
									break;
								}
								else if(MACFrameIsTimedOut(frame)){
									//du 产生路由错误
									
									MACFrameRemove(frame);
									break;	
								}
							}
	//du mac ask如何处理
	ConsolePutString("forward route reply\n");
}

void send_rrep_ack( WORD dest_ip){
	RREP_ACK *ack = (RREP_ACK *) malloc(sizeof(RREP_ACK));
	ack->type = AODV_RREP_ACK;
	ack->reserved = 0;
	
	macDestInfo.addrMode = MAC_DST_SHORT_ADDR;
	macDestInfo.panID = macInfo.panID;
	//du 到请求发起节点路径的下一跳
	macDestInfo.shortAddr.Val = dest_ip; 
	NWKPutHeader(&macDestInfo,
                NWK_FRAME_CMD |
                NWK_PROT_VER,
                0x00,
				macDestInfo.shortAddr);
	NWKPut(NWK_CMD_ROUTE_ACK);
	NWKPutArray((BYTE *)ack, (BYTE)sizeof(RREP_ACK));
	NWKFlush();
	//du mac ask如何处理
	ConsolePutString("send route ack\n");


}

void gen_rerr( WORD break_ip){
struct	Route_Table_Entry * tmp_route = route_table;
struct	Err_Dest_List * err_list = NULL;
struct	Err_Dest_List * tmp_list = NULL;
	WORD err_count = 0;
	while (tmp_route){
		if (tmp_route->next_ip == break_ip && !tmp_route->flag.bit.is_self_route && tmp_route->flag.bit.is_route_valid){
			tmp_route->flag.bit.is_route_valid = FALSE;
			tmp_route->dest_seq++;
			tmp_list = (struct Err_Dest_List *)malloc(sizeof(struct Err_Dest_List));
			if (tmp_list == NULL){
				//ConsolePutString("malloc new Err_Dest_List error\n");
				return;
			}
			tmp_list->err_dest.unreach_dest_ip = tmp_route->dest_ip;
			tmp_list->err_dest.unreach_dest_seq = tmp_route->dest_seq;
			tmp_list->next = err_list;
			err_list = tmp_list;
			
				
	
			err_count++;
		}
		tmp_route = tmp_route->next;
	}
	if (err_count)
		send_rerr( err_list, err_count);
}

void send_rerr(struct Err_Dest_List *err_list,  WORD dest_count){
struct Err_Dest * err_dest = NULL;
struct Err_Dest_List *dead_list = NULL;
WORD length = sizeof(RERR) + (sizeof(struct Err_Dest))*dest_count;
char * data = NULL;
	if (dest_count){
		data = (char *)malloc(length);
		if (data == NULL){
			//ConsolePutString("malloc new RERR error\n");
			return;
		}
		RERR * rerr = rerr = (RERR *)data;
		
		rerr->type = AODV_RERR;
		rerr->flag.bit.n = FALSE;
		rerr->reserved = 0;
		rerr->dest_count = dest_count;
		err_dest = (struct Err_Dest *)(data + sizeof(RERR));
		while (dest_count){
			//ConsolePutString("node%d failed route\n", node_index+1);

			err_dest->unreach_dest_ip = err_list->err_dest.unreach_dest_ip;
			//ConsolePutString("route dest_ip = %d\n", err_dest->unreach_dest_ip);
			err_dest->unreach_dest_seq = err_list->err_dest.unreach_dest_seq;
			//ConsolePutString("route dest_ip_seq = %d\n", err_dest->unreach_dest_seq);
			dead_list = err_list;
			err_list = err_list->next;
			err_dest = err_dest+1;
			dest_count--;
			free(dead_list);
		}
		macDestInfo.addrMode = MAC_DST_SHORT_ADDR;
		macDestInfo.panID = macInfo.panID;
		macDestInfo.shortAddr.Val = 0xffff;
		
	    NWKPutHeader(&macDestInfo,
                NWK_FRAME_CMD |
                NWK_PROT_VER,
                0x00,
				macDestInfo.shortAddr);
		NWKPut(NWK_CMD_ROUTE_ERROR);
		NWKPutArray((BYTE *)data, (BYTE)length);
		NWKFlush();
		ConsolePutString("send route error");
	}
}

void init_route_table(){
	route_table = NULL;
struct	Route_Table_Entry *tmp_route = create_route( macInfo.shortAddr.Val);
	tmp_route->flag.bit.is_self_route = TRUE;
	tmp_route->flag.bit.is_route_valid = TRUE;
	tmp_route->flag.bit.is_route_seq_valid = TRUE;
	tmp_route->next_ip = macInfo.shortAddr.Val;
}

struct Route_Table_Entry *create_route( WORD ip){
struct	Route_Table_Entry * new_route = (struct Route_Table_Entry *)malloc(sizeof(struct Route_Table_Entry ));
	if (new_route == NULL){
		//ConsolePutString("create route failer\n");
		return NULL;
	}
	new_route->dest_ip = ip;
	new_route->dest_seq = 0;
	new_route->hopCount = 0;
	new_route->life_time = 0;
	new_route->next_ip = 0;
	new_route->req_id = 0;
	new_route->flag.bit.is_self_route = FALSE;
	new_route->flag.bit.is_route_valid = FALSE;
	new_route->flag.bit.is_route_seq_valid = FALSE;
	new_route->next = NULL;
	new_route->prev = NULL;
	insert_route( new_route);
	return new_route;
}

void insert_route(struct Route_Table_Entry * new_route){

	if (route_table == NULL){
		route_table = new_route;
		return;
	}
struct	Route_Table_Entry *tmp_route = route_table;
struct	Route_Table_Entry *prev_route = NULL;

	while (tmp_route && tmp_route->dest_ip < new_route->dest_ip){
		prev_route = tmp_route;
		tmp_route = tmp_route->next;
	}
	if (tmp_route && tmp_route == route_table){
		tmp_route->prev = new_route;
		new_route->next = tmp_route;
		route_table = new_route;
	
	}else if (tmp_route == NULL){
		prev_route->next = new_route;
		new_route->prev = prev_route;
	}else{
		prev_route->next = new_route;
		new_route->prev = prev_route;
		new_route->next = tmp_route;
		tmp_route->prev = new_route;
	}
}

struct Route_Table_Entry *lookup_route(WORD dest_ip){
struct	Route_Table_Entry * tmp_route = route_table;
struct	Route_Table_Entry * dest_route = NULL;
	while (tmp_route && tmp_route->dest_ip <= dest_ip){
		if (tmp_route->dest_ip == dest_ip){
			dest_route = tmp_route;
		}
		tmp_route = tmp_route->next;
	}
	return dest_route;
}

void cleanup_route_table(){
struct	Route_Table_Entry * tmp_route = route_table;
	while (tmp_route){
		tmp_route = tmp_route->next;
		free(tmp_route->prev);
	}
	route_table = NULL;
}

void delete_route( WORD dest_ip){
struct	Route_Table_Entry * tmp_route = route_table;
struct	Route_Table_Entry * del_route = NULL;
	while(tmp_route && tmp_route->dest_ip <= dest_ip){
		if (tmp_route->dest_ip == dest_ip)
			del_route = tmp_route;
		tmp_route = tmp_route->next;
	}
	if (del_route){
		if (del_route == route_table){
			route_table = route_table->next;
			route_table->prev = NULL;
		}
		if (del_route->prev){
			del_route->prev->next = del_route->next;
		}
		if (del_route->next){
			del_route->next->prev = del_route->prev;
		}
		free(del_route);
	}

	
}

void update_route( WORD dest_ip, WORD next_ip, WORD dest_seq, BYTE hopCount)
{
struct Route_Table_Entry * tmp_route = NULL;
tmp_route = lookup_route( dest_ip);
if (tmp_route == NULL)
{
	tmp_route = create_route( dest_ip);
}
if (!tmp_route->flag.bit.is_route_seq_valid);
else if (seq_greater(dest_seq, tmp_route->dest_seq));
else if (dest_seq == tmp_route->dest_seq && (!tmp_route->flag.bit.is_route_valid || tmp_route->hopCount > hopCount));
else
	return;
tmp_route->dest_ip = dest_ip;
tmp_route->dest_seq = dest_seq;
tmp_route->hopCount = hopCount;
tmp_route->next_ip = next_ip;
tmp_route->precuror = NULL;
tmp_route->life_time = 0;
tmp_route->flag.bit.is_route_seq_valid = TRUE;
tmp_route->flag.bit.is_route_valid = TRUE;
}

void init_req_id_queue(){
req_id_queue = NULL;
}
void insert_req_id( WORD src_ip, WORD dst_ip, WORD id){
flush_req_id_queue();
struct REQ_ID * req_id = (struct REQ_ID *)malloc(sizeof(struct REQ_ID));
if (req_id == NULL){
	//ConsolePutString("malloc new req_id failed");
	return;
}
req_id->dest_ip = dst_ip;
req_id->life_time = TickGet();//du 记录请求时间
req_id->req_id = id;
req_id->source_ip = src_ip;
req_id->next = req_id_queue;
req_id_queue = req_id;
}
struct REQ_ID *find_req_id( WORD src_ip, WORD id){
struct REQ_ID * tmp_req_id = req_id_queue;
while (tmp_req_id != NULL){
	if (tmp_req_id->source_ip == src_ip && tmp_req_id->req_id == id)
		return tmp_req_id;
	tmp_req_id = tmp_req_id->next;
}
return NULL;
}

void cleanup_req_id_queue(){
struct REQ_ID * tmp_req_id = req_id_queue;
struct REQ_ID * req_id = NULL;
while (tmp_req_id != NULL){
	req_id = tmp_req_id;
	tmp_req_id = tmp_req_id->next;
	free(req_id);
}
req_id_queue = NULL;
}

//du 一次只删除一个req_id
void flush_req_id_queue(){
struct REQ_ID * tmp_req_id = req_id_queue;
struct REQ_ID * prev_req_id = NULL;
struct REQ_ID * req_id = NULL;

while (tmp_req_id != NULL){
	if ((TickGet()-tmp_req_id->life_time) >= NWK_ROUTE_REQUEST_ID_PERIOD){
		req_id = tmp_req_id;
		
		if (tmp_req_id == req_id_queue)
			req_id_queue = tmp_req_id->next;
		else{
			prev_req_id->next = tmp_req_id->next;
			
		}
		free(req_id);
		ConsolePutString("remove req id\n");
		return;
	}
		prev_req_id = tmp_req_id;
		tmp_req_id = tmp_req_id->next;
	
	
}
ConsolePutString("no req id expire\n");
return;
}

BOOL seq_less_or_equal(WORD seq_one, WORD seq_two)
{
    SWORD *comp_seq_one = (SWORD *)&seq_one;
    SWORD *comp_seq_two = (SWORD *)&seq_two;

    if (  ( *comp_seq_one - *comp_seq_two ) > 0 )
    {
        return FALSE;
    }
    else
        return TRUE;
}

BOOL seq_greater(WORD seq_one,WORD seq_two)
{
    SWORD *comp_seq_one = (SWORD *)&seq_one;
   SWORD *comp_seq_two = (SWORD *)&seq_two;

    if (  ( *comp_seq_one - *comp_seq_two ) <= 0 )
        return FALSE;
    else
        return TRUE;
}
