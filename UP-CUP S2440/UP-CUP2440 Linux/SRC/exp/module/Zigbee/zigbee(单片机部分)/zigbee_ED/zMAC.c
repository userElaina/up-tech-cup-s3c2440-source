#include "zMAC.h"


#if defined(I_AM_END_DEVICE)
	static SM_ASSOCIATION   smAssociation;

	// Running count of transmit retries used to association/disassociation
	BYTE                    macFrameRetryCount;
#endif

// MAC Module state info.
MAC_STATE               macState;

// Potential coordinator description - used when Associate process is started.
PAN_DESC                PANDesc;

// Number of coordinators found operating in radius.
BYTE                    PANDescCount;

// Current channel of operation.
BYTE                    macCurrentChannel;

// Running counter of MAC frame DSN.
BYTE                    macDSN;

BYTE                    lastMACDSN;

// Length of transmit packet that is being current loaded.
BYTE                    macPacketLen;

// MAC address info.
NODE_INFO               macInfo;

// Coordinator address info.
NODE_INFO               macCoordInfo;

// A temp NODE_INFO to store out going commands from the MAC layer
NODE_INFO               macDestInfo;

// RF channel energy as of last energy scan.
BYTE                    macCurrentEnergy;

// Current MAC frame header.
MAC_HEADER               macCurrentFrame;

// Array of recently transmitted frames that are yet to be acknowledged
// by remote nodes.
MAC_FRAME_STATUS        macFrameStatusQ[MAX_MAC_FRAME_STATUS];

BYTE                    macFrameStatusQLen;

// Tick value used to calculate timeout conditions.
TICK                    macStartTick;

// Handle to a frame - used by many command state machines
HFRAME                  hFrame;

#define PHYBeginTxFIFOAccess()      { PHYBegin(); PHYSelectTxFIFO(); }
#define PHYEndTxFIFOAccess()        PHYEnd()
#define PHYPutTxData(v)             PHYPut(v)

static HFRAME   AddDSNInQueue(BYTE dsn);

static void     UpdateQueue(BYTE dsn);

static void     TransmitIt(void);

static BOOL     MACGetHeader(void);

static void     MACSendBeaconReq(void);

static void     MACSendBeacon(void);

static BOOL     MACProcessBeacon(void);

static void     MACSendAck(ACK_TYPE ack);

// Only coordinator will process orphan notice.
// A orphan notice is said to be valid if there is no data bytes in addition to
// the command identifier itself.
#define MACProcessOrphanNotice()        (macCurrentFrame.frameLength == 0)

/*********************************************************************
 * Function:        void MACInit(void)
 *
 * PreCondition:    macInfo.longAddr must be initialized with
 *                  desired MAC address.
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
void MACInit(void)
{
    
    memset((void*)macFrameStatusQ, 0x00, sizeof(macFrameStatusQ));
    macFrameStatusQLen = 0;

    macCurrentFrame.Flags.Val = 0x00;
    macState.Val = 0x00;
	
	//du 读取地址	
	GetMACShortAddress(&macInfo.shortAddr);
	GetMACAddress(&macInfo.longAddr);

    //du 初始PAN ID = 0xffff
    MACSetPANIdLSB(0xff);
    MACSetPANIdMSB(0xff);

}

BOOL MACIsIdle()
{
	return (MACIsPutReady() && (macFrameStatusQLen == 0));
} 

/*********************************************************************
 * Function:        void MACEnable(void)
 *
 * PreCondition:    macInfo.longAddr must be initialized with
 *                  desired MAC address.
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
void MACEnable(void)
{
    // Power up the PHY's internal regulator
    PHYEnable();

    // Initialize the PHY's registers/wait for it's oscillator to
    // become stable.  In the event the PHY malfunctions for too long
    // the watchdog timer will cause a reset to occur here.
    while( !PHYInit() );

    MACUpdateAddressInfo();

    PHYSetChannel(PHYGetChannel());

    macState.bits.bIsEnabled = TRUE;
}

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
void MACISR(void)
{  
	if ( macState.bits.bIsTxBusy )
         macState.bits.bIsTxBusy = FALSE;
 }   

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
BYTE MACGet(void)
{
    BYTE v;

    // Start PHY RX buffer access.
    PHYBegin();
    PHYSelectRxFIFO();
    v = PHYGet();
    PHYEnd();

    // Update current frame length.
    macCurrentFrame.frameLength--;

    // Now return it.
    return v;
}

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
void MACPutArray(BYTE *v, BYTE len)
{
    // Start RX TX buffer access.
    PHYBeginTxFIFOAccess();

    // Copy all given bytes into TX buffer.
    while( len-- )
    {
        PHYPutTxData(*v);

        v++;

        // Keep count of current TX frame length.
        macPacketLen++;
    }

    // Conclude RX TX buffer access.
    PHYEndTxFIFOAccess();
}

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
BOOL MACIsPutReady(void)
{
    // The transmitter is said to be ready when
    // 1. It has finished previous transmission,
    // 2. We are part of a network
    // 3. Enabled.
    // 4. Ack queue is not full.
    return (macState.bits.bIsTxBusy == FALSE    &&
            macState.bits.bIsAssociated == TRUE &&
            macState.bits.bIsEnabled == TRUE &&
            macFrameStatusQLen < MAX_MAC_FRAME_STATUS);
}

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
void MACPut(BYTE v)
{
    // Put given byte into TXFIFO
    PHYBeginTxFIFOAccess();
    PHYPutTxData(v);
    PHYEndTxFIFOAccess();

    // Maintain length so that we update header.
    macPacketLen++;
}

/*********************************************************************
 * Function:        HFRAME MACFlush(void)
 *
 * PreCondition:    MACIsPutReady() == TRUE and
 *                  a valid TX frame is constructured using
 *                  MACPutHeader() and MACPut()
 *
 * Input:           None
 *
 * Output:          HFRAME_INVALID if an Ack wasn't requested or the
 *                  DSN queue was full. Otherwise, the handle to the
 *                  DSN queue entry is returned which can later be
 *                  used when the Ack frame arrives.
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
HFRAME MACFlush(void)
{
    hFrame = HFRAME_INVALID;

    // Remember that current message is in transmit buffer.
    macState.bits.bIsTxBusy = TRUE;

    if ( macCurrentFrame.Flags.bits.bToBeQueued )
        hFrame = AddDSNInQueue(macDSN);

    macCurrentFrame.Flags.bits.bToBeQueued = FALSE;

    // First, update the packet length.
    PHYBegin();

    PHYSelectTxRAM();

    PHYPut(macPacketLen);

    // Terminate RAM access
    PHYEnd();

    // Now transmit it.
    TransmitIt();

    // Forget packet length.
    macPacketLen = 0;

    return hFrame;
}

static void TransmitIt(void)
{
    BYTE v;
    BYTE backOffCycles;

    // Wait until transceiver is idle
    //while( !PHYIsIdle() );

    // To creat randomness in initial backoff, we will use
    // LSB of current tick counter.
    backOffCycles = (BYTE)TickGet();

    // Remember that a message is being transmitted.
    macState.bits.bIsTxBusy = TRUE;

    // Turn on RX
    PHYSetTRXState(PHY_TRX_RX_ON);
	
    // Wait for RSSI to become valid
    PHYBegin();
    do
    {
        PHYPut(STROBE_SNOP);
        v = PHYGet();
        if ( v & 0x02 )
            break;
    } while(1);
    PHYEnd();

    PHYBegin();
    do
    {
        PHYTx();
        v = PHYGet();


        PHYPut(STROBE_SNOP);
        v = PHYGet();


        // If transmission was started, break out of this loop.
        if ( v & 0x08 )
            break;


        //TODO(DF9): CSMA-CA back-off needed
        // Else, need to back-off - not quite IEEE compliant.
        backOffCycles <<= 1;

        // Reinitialize backoff time if we reach 0 or 0xff value.
        if ( backOffCycles == 0x00 || backOffCycles == 0xff )
            backOffCycles = (BYTE)TickGet();

        // Now wait until backoff cycles expire.
        v = backOffCycles;
        while( v-- );

    } while( 1 );
    PHYEnd();


    // Tell application that we have just transmitted a frame.
    AppMACFrameTransmitted();
}

/*********************************************************************
 * Function:        void MACUpdateTimeoutFlags( void )
 *
 * PreCondition:    MACInit() is called
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    Frame timeout flags are updated
 *
 * Overview:        This function goes through the frame queue and
 *                  checks each frame that is currently waiting for
 *                  an acknowledge to see if it has timed out.  It
 *                  then calls the application call-back.
 *
 * Note:            None
 ********************************************************************/
void MACUpdateTimeoutFlags( void )
{
    BYTE i;
    MAC_FRAME_STATUS *pMACFrameStatus;

    pMACFrameStatus = macFrameStatusQ;
    for ( i = 0; i < sizeof(macFrameStatusQ)/sizeof(macFrameStatusQ[0]); i++, pMACFrameStatus++ )
    {
        if ( pMACFrameStatus->Flags.bits.bIsInUse )
        {
            if ( !pMACFrameStatus->Flags.bits.bIsConfirmed )
            {
                if ( TickGetDiff(TickGet(), pMACFrameStatus->lastTick) >= MAC_ACK_WAIT_DURATION )
                {
                    pMACFrameStatus->Flags.bits.bIsTimedOut = TRUE;

                    //Notify the application of a timeout condition.
					//ConsolePutString("time out and remove\n");
					//du 直接从DSN中移除   
					//MACFrameRemove(i);
				
                    #if 0
                    if(pMACFrameStatus->retryCount == 0)
                    {
                        //DumpMacFrameStatusQ();
                        //MACRemoveFrame(i);
                    }
                    #endif
                }
            }
        }
    }
}

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
BOOL MACTask(void)
{	
//	ConsolePutString(__FUNCTION__);
//	ConsolePutString("\r\n");
	 MACUpdateTimeoutFlags();

    // If there was an overflow, handle it.
    // Since this is a half-duplex and the way CC2420 indicates RX FIFO OVERFLOW,
    // check for overflow condition only when RF is not actively transmitting a packet.
    if ( macState.bits.bIsTxBusy == FALSE )
    {
        if ( PHYProcessRxOverflow() )
        {
            ConsolePutString("MAC: A receive overflow was detected and removed.\r\n");
        }
    }

    //du 当过去的数据被处理完成后，才接受新的数据
	if ( MACIsGetReady() ){
		
		ConsolePutString("mac not handled\n");
		return TRUE;
	}

    if ( !PHYIsGetReady() )
    {
		
        return TRUE;
    }

    if ( !MACGetHeader() )
    {
        ConsolePutString("mac header wrong\n");
		return TRUE;
    }

    ConsolePutString("mac receive right frame\n");
    macCurrentFrame.Flags.bits.bIsGetReady = TRUE;
	
    // Send ack back if requested.
    if ( macCurrentFrame.frameCONLSB.bits.AckRequest )
    {
        if((MACIsCommand()) && (MACIsDataReq()))
        {
           MACSendAck(ACK_DATA_PENDING);
           MACSendAck(ACK_NO_DATA);
        }
        else
        {
            MACSendAck(ACK_NO_DATA);
        }
		ConsolePutString("AckRequest\n");//add  by lyj
    }

    if (MACIsAck())
    {
        UpdateQueue(macCurrentFrame.macDSN);   
        MACDiscardRx();
		ConsolePutString("is ack\n"); //add by lyj
    }
    else if (MACIsCommand())
    {
    	ConsolePutString("is command\n");//add by lyj
        if (MACIsBeaconRequest())
        {
        	ConsolePutString("    Is BeaconRequest\n");//add by lyj
            if (macState.bits.bIsAssociated)
            { 
                MACSendBeacon();
            }
			MACDiscardRx();		
        }

        // New associations are allowed only if it is permitted by application.
        else if (MACIsAssocRequest())
        {            
        	ConsolePutString("    Is AssocRequest\n");//add by lyj
            if ( !MACProcessAssociation() )
            {
                MACDiscardRx();
				
			}
			else {
				MACDiscardRx();
				//du macInfo.shortAddr接收被忽略
				MACSendAssociateResponse(&macInfo.shortAddr, MAC_ASSOCIATE_SUCCESS);
			}        
        }
		        // Orphan can be rejoined only after we have established new network.
        else if ( MACIsOrphanNotice() )
        {
        	ConsolePutString("    Is OrphanNotice\n");//add by lyj
            if ( macState.bits.bIsAssociated )
            {
                if ( !MACProcessOrphanNotice() )
                    MACDiscardRx();
				else {
					MACDiscardRx();
					//du macInfo.shortAddr接收被忽略
					MACSendRealignment(&macInfo.shortAddr, FALSE);
				}
					
            }
            else
                MACDiscardRx();
        }

      
        else if ( MACIsDisassocNotice() )
        {
        	ConsolePutString("    Is DisassocNotice\n");//add by lyj
#ifdef I_AM_COORDINATOR
            if ( !MACProcessDisassociation() )
                MACDiscardRx();
			else {
				 MACDiscardRx();
			}
#endif 
#ifdef I_AM_END_DEVICE
			            //The coordinator or router I am associated to has asked
            //me to leave.
            //MLME-DISASSOCIATION.indication
            macState.bits.bIsAssociated = FALSE;

            // Forget PAN ID.
            MACSetPANIdLSB(0xff);
            MACSetPANIdMSB(0xff);

            // Update the transceiver
            MACUpdateAddressInfo();
			MACDiscardRx();
#endif

        }
		else{
			ConsolePutString("    Is Error command\n");//add by lyj
//			MACDiscardRx();
		}
    }
	else if(MACIsData()){
    
		ConsolePutString("is data\n");
	}
	else if(MACIsBeacon()){
		if(ZIGBEE_RUN_ENDPOINT){
			MACDiscardRx();
			ConsolePutString("ZIGBEE_RUN_ENDPOINT is true\n");
		}
		ConsolePutString("is Beacon\n");
	}

    return TRUE;
}
#if 1
/*********************************************************************
 * Function:        static BOOL MACGetHeader(void)
 *
 * PreCondition:    _MACIsGetReady() == TRUE
 *
 * Input:           None
 *
 * Output:          TRUE        - if current frame has valid header
 *                  FALSE       - if current frame does not have
 *                                valid header
 *
 * Side Effects:    None
 *
 * Overview:        Fetches rx frame one byte at a time and validates
 *                  as per 802.15.4 spec.
 *
 * Note:            This is private function, called MAC module only.
 ********************************************************************/
static BOOL MACGetHeader(void)
{
    BYTE_VAL temp;
    BYTE frameType;
    BYTE_VAL length;

    PHYBegin();

    // Start reading RX FIFO.
    PHYSelectRxFIFO();

    // First byte is length
    length.Val = PHYGet();

    // Make sure that this is a valid length
    if ( length.Val == 0 || length.bits.b7 == 1 )
    {
        goto DiscardAndReturn;
    }

    // Save received packet length minus 2 bytes of CRC/(RSSI + CORRELATION for CC2420)
    macCurrentFrame.frameLength = length.Val - 2;

    // Get frame control field - LSB.
    temp.Val = PHYGet();
    // One less byte from packet count.
    macCurrentFrame.frameLength--;

    // save frame LSB
    macCurrentFrame.frameCONLSB.Val = temp.Val;

    // Extract and save frame type
    macCurrentFrame.type = temp.Val & 0x07;

    // Reject "Reserved" frame type.
    frameType = macCurrentFrame.type;
    if ( frameType > 0x03 )
    {
        goto DiscardAndReturn;
    }

    // Other bits will be processed later...

    // Now get MSB
    temp.Val = PHYGet();
    // One less byte from packet count.
    macCurrentFrame.frameLength--;

    // Save MSB
    macCurrentFrame.frameCONMSB.Val = temp.Val;

    // Save frame MSB.
    macCurrentFrame.dst.addrMode = temp.Val & 0x0c;
    macCurrentFrame.src.addrMode = temp.Val & 0xc0;

    // Get Sequence number
    macCurrentFrame.macDSN = PHYGet();

    macCurrentFrame.frameLength--;

    // Mode value of non-zero means that destination PAN identifier is present.
    if ( macCurrentFrame.dst.addrMode == MAC_DST_ADDR_RESERVED )
    {
        goto DiscardAndReturn;
    }

    // If the destination addressing mode field is non-zero, there is a dst PAN ID.
    if ( macCurrentFrame.dst.addrMode )
    {
        macCurrentFrame.dst.panID.byte.LSB = PHYGet();
        macCurrentFrame.dst.panID.byte.MSB = PHYGet();

		ConsolePutString("macCurrentFrame.dst.panID:");//test lyj
		ConsolePutInitData(macCurrentFrame.dst.panID.byte.MSB,macCurrentFrame.dst.panID.byte.LSB);//test lyj


        macCurrentFrame.frameLength -= 2;
    }

    // Fetch destination address according to its type
    if ( macCurrentFrame.dst.addrMode == MAC_DST_SHORT_ADDR )
    {
        macCurrentFrame.dst.shortAddr.byte.LSB = PHYGet();
        macCurrentFrame.dst.shortAddr.byte.MSB = PHYGet();

		ConsolePutString("macCurrentFrame.dst.shortAddr:");//test lyj
		ConsolePutInitData(macCurrentFrame.dst.shortAddr.byte.MSB,macCurrentFrame.dst.shortAddr.byte.LSB);//test lyj


        macCurrentFrame.frameLength -= 2;
    }
    else if ( macCurrentFrame.dst.addrMode == MAC_DST_LONG_ADDR )
    {
        macCurrentFrame.dst.longAddr.v[0] = PHYGet();
        macCurrentFrame.dst.longAddr.v[1] = PHYGet();
        macCurrentFrame.dst.longAddr.v[2] = PHYGet();
        macCurrentFrame.dst.longAddr.v[3] = PHYGet();
        macCurrentFrame.dst.longAddr.v[4] = PHYGet();
        macCurrentFrame.dst.longAddr.v[5] = PHYGet();
        macCurrentFrame.dst.longAddr.v[6] = PHYGet();
        macCurrentFrame.dst.longAddr.v[7] = PHYGet();

        macCurrentFrame.frameLength -= 8;
    }

    // Mode value of non-zero means that source PAN identifier is present.
    if ( macCurrentFrame.src.addrMode == MAC_SRC_ADDR_RESERVED )
        goto DiscardAndReturn;

    // If the source addressing mode field is non-zero and the IntraPAN flag is clear,
    // the IEEE spec says there will be a src PAN ID.
    if ( macCurrentFrame.src.addrMode &&
         macCurrentFrame.frameCONLSB.bits.IntraPAN == 0 )
    {
        macCurrentFrame.src.panID.byte.LSB = PHYGet();
        macCurrentFrame.src.panID.byte.MSB = PHYGet();

		//ConsolePutString("macCurrentFrame.src.panID:");//test lyj
		//ConsolePutInitData(macCurrentFrame.src.panID.byte.MSB,macCurrentFrame.src.panID.byte.LSB);//test lyj

        macCurrentFrame.frameLength -= 2;
    }


    // Fetch source address according to its type
    if ( macCurrentFrame.src.addrMode == MAC_SRC_SHORT_ADDR )
    {
        macCurrentFrame.src.shortAddr.byte.LSB = PHYGet();
        macCurrentFrame.src.shortAddr.byte.MSB = PHYGet();
		ConsolePutString("macCurrentFrame.src.shortAddr:");//test lyj
		ConsolePutInitData(macCurrentFrame.src.shortAddr.byte.MSB,macCurrentFrame.src.shortAddr.byte.LSB);//test lyj

        macCurrentFrame.frameLength -= 2;
    }
    else if ( macCurrentFrame.src.addrMode == MAC_SRC_LONG_ADDR )
    {
        macCurrentFrame.src.longAddr.v[0] = PHYGet();
        macCurrentFrame.src.longAddr.v[1] = PHYGet();
        macCurrentFrame.src.longAddr.v[2] = PHYGet();
        macCurrentFrame.src.longAddr.v[3] = PHYGet();
        macCurrentFrame.src.longAddr.v[4] = PHYGet();
        macCurrentFrame.src.longAddr.v[5] = PHYGet();
        macCurrentFrame.src.longAddr.v[6] = PHYGet();
        macCurrentFrame.src.longAddr.v[7] = PHYGet();

        macCurrentFrame.frameLength -= 8;
    }
#if 0
	if ( macCurrentFrame.type == MAC_FRAME_CMD )
		ConsolePutString("this is a command\n");
	else
		ConsolePutString("this is not a command\n");
#endif
    // If this is a command frame, extract and save command id.
    if ( macCurrentFrame.type == MAC_FRAME_CMD )
    {
        macCurrentFrame.frameLength--;
        macCurrentFrame.cmd = PHYGet();
		//ConsolePutString("this is a command\n");//add by lyj
		//ConsolePutInitData(0,macCurrentFrame.cmd);//add by lyj
    }
#if defined(I_AM_END_DEVICE)
    else if ( macCurrentFrame.type == MAC_FRAME_DATA )
        macState.bits.bIsPollDone = TRUE;
#endif

    PHYEnd();

    return TRUE;


DiscardAndReturn:
    PHYEnd();

    PHYBegin();
    PHYFlushRx();
    PHYEnd();
    return FALSE;
}

#endif
#if 0
/*********************************************************************
 * Function:        static BOOL MACGetHeader(void)
 *
 * PreCondition:    _MACIsGetReady() == TRUE
 *
 * Input:           None
 *
 * Output:          TRUE        - if current frame has valid header
 *                  FALSE       - if current frame does not have
 *                                valid header
 *
 * Side Effects:    None
 *
 * Overview:        Fetches rx frame one byte at a time and validates
 *                  as per 802.15.4 spec.
 *
 * Note:            This is private function, called MAC module only.
 ********************************************************************/
static BOOL MACGetHeader(void)
{
    BYTE_VAL temp;
    BYTE frameType;
    BYTE_VAL length;

    PHYBegin();

    // Start reading RX FIFO.
    PHYSelectRxFIFO();

    // First byte is length
    length.Val = PHYGet();

    // Make sure that this is a valid length
    if ( length.Val == 0 || length.bits.b7 == 1 )
    {
        goto DiscardAndReturn;
    }

    // Save received packet length minus 2 bytes of CRC/(RSSI + CORRELATION for CC2420)
    macCurrentFrame.frameLength = length.Val - 2;

    // Get frame control field - LSB.
    temp.Val = PHYGet();
    // One less byte from packet count.
    macCurrentFrame.frameLength--;

    // save frame LSB
    macCurrentFrame.frameCONLSB.Val = temp.Val;

    // Extract and save frame type
    macCurrentFrame.type = temp.Val & 0x07;

    // Reject "Reserved" frame type.
    frameType = macCurrentFrame.type;
    if ( frameType > 0x03 )
    {
        goto DiscardAndReturn;
    }

    // Other bits will be processed later...

    // Now get MSB
    temp.Val = PHYGet();
    // One less byte from packet count.
    macCurrentFrame.frameLength--;

    // Save MSB
    macCurrentFrame.frameCONMSB.Val = temp.Val;

    // Save frame MSB.
    macCurrentFrame.dst.addrMode = temp.Val & 0x0c;
    macCurrentFrame.src.addrMode = temp.Val & 0xc0;

    // Get Sequence number
    macCurrentFrame.macDSN = PHYGet();

    macCurrentFrame.frameLength--;

    // Mode value of non-zero means that destination PAN identifier is present.
    if ( macCurrentFrame.dst.addrMode == MAC_DST_ADDR_RESERVED )
    {
        goto DiscardAndReturn;
    }

    // If the destination addressing mode field is non-zero, there is a dst PAN ID.
    if ( macCurrentFrame.dst.addrMode )
    {
        macCurrentFrame.dst.panID.byte.LSB = PHYGet();
        macCurrentFrame.dst.panID.byte.MSB = PHYGet();

        macCurrentFrame.frameLength -= 2;
    }

    // Fetch destination address according to its type
    if ( macCurrentFrame.dst.addrMode == MAC_DST_SHORT_ADDR )
    {
        macCurrentFrame.dst.shortAddr.byte.LSB = PHYGet();
        macCurrentFrame.dst.shortAddr.byte.MSB = PHYGet();

        macCurrentFrame.frameLength -= 2;
    }
    else if ( macCurrentFrame.dst.addrMode == MAC_DST_LONG_ADDR )
    {
        macCurrentFrame.dst.longAddr.v[0] = PHYGet();
        macCurrentFrame.dst.longAddr.v[1] = PHYGet();
        macCurrentFrame.dst.longAddr.v[2] = PHYGet();
        macCurrentFrame.dst.longAddr.v[3] = PHYGet();
        macCurrentFrame.dst.longAddr.v[4] = PHYGet();
        macCurrentFrame.dst.longAddr.v[5] = PHYGet();
        macCurrentFrame.dst.longAddr.v[6] = PHYGet();
        macCurrentFrame.dst.longAddr.v[7] = PHYGet();

        macCurrentFrame.frameLength -= 8;
    }

    // Mode value of non-zero means that source PAN identifier is present.
    if ( macCurrentFrame.src.addrMode == MAC_SRC_ADDR_RESERVED )
        goto DiscardAndReturn;

    // If the source addressing mode field is non-zero and the IntraPAN flag is clear,
    // the IEEE spec says there will be a src PAN ID.
    if ( macCurrentFrame.src.addrMode &&
         macCurrentFrame.frameCONLSB.bits.IntraPAN == 0 )
    {
        macCurrentFrame.src.panID.byte.LSB = PHYGet();
        macCurrentFrame.src.panID.byte.MSB = PHYGet();

        macCurrentFrame.frameLength -= 2;
    }


    // Fetch source address according to its type
    if ( macCurrentFrame.src.addrMode == MAC_SRC_SHORT_ADDR )
    {
        macCurrentFrame.src.shortAddr.byte.LSB = PHYGet();
        macCurrentFrame.src.shortAddr.byte.MSB = PHYGet();

        macCurrentFrame.frameLength -= 2;
    }
    else if ( macCurrentFrame.src.addrMode == MAC_SRC_LONG_ADDR )
    {
        macCurrentFrame.src.longAddr.v[0] = PHYGet();
        macCurrentFrame.src.longAddr.v[1] = PHYGet();
        macCurrentFrame.src.longAddr.v[2] = PHYGet();
        macCurrentFrame.src.longAddr.v[3] = PHYGet();
        macCurrentFrame.src.longAddr.v[4] = PHYGet();
        macCurrentFrame.src.longAddr.v[5] = PHYGet();
        macCurrentFrame.src.longAddr.v[6] = PHYGet();
        macCurrentFrame.src.longAddr.v[7] = PHYGet();

        macCurrentFrame.frameLength -= 8;
    }

    // If this is a command frame, extract and save command id.
    if ( macCurrentFrame.type == MAC_FRAME_CMD )
    {
        macCurrentFrame.frameLength--;
        macCurrentFrame.cmd = PHYGet();
    }
#if defined(I_AM_END_DEVICE)
    else if ( macCurrentFrame.type == MAC_FRAME_DATA )
        macState.bits.bIsPollDone = TRUE;
#endif

    PHYEnd();

    return TRUE;


DiscardAndReturn:
    PHYEnd();

    PHYBegin();
    PHYFlushRx();
    PHYEnd();
    return FALSE;
}
#endif
#if defined(I_AM_END_DEVICE)
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
	void MACStartAssociation(void)
	{
		// Forget about previous association.
		macState.bits.bIsAssociated = FALSE;

		macFrameRetryCount = MAC_MAX_FRAME_RETRIES;

		// Reset the state machine.
		smAssociation = SM_SEND_ASSOCIATE_REQ;

	}

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
	BOOL MACIsAssociationComplete(void)
	{
		BOOL result = FALSE;
		MAC_ASSOCIATE_STATUS    macAssociationStatus;
		int i;

	   
		switch(smAssociation)
		{
		case SM_SEND_ASSOCIATE_REQ:
			macFrameRetryCount--;

			// When associating, provide long address and no pan id.
			macInfo.addrMode    = MAC_SRC_LONG_ADDR;
			macInfo.panID.Val   = 0xffff;
			//du macCoordInfo 不一定是真正的Coord
			MACPutHeader(&macCoordInfo,            
						MAC_FRAME_CMD |           
						MAC_SECURITY |
						MAC_FRAME_PENDING_NO |
						MAC_ACK_YES|
						MAC_INTRA_PAN_NO);



			PHYBeginTxFIFOAccess();
			PHYPutTxData(MAC_CMD_ASSOCIATE_REQ);
			PHYPutTxData(0x00);	//MAC_CAP_INFO，接收忽略
			PHYEndTxFIFOAccess();

	        
			macPacketLen += 2;

			hFrame = MACFlush();

			ConsolePutString("MAC: MACSendAssociateRequest(): Sent association request...\r\n");
			
			macStartTick = TickGet();
			smAssociation = SM_WAIT_FOR_ACK;

			break;
			

		case SM_WAIT_FOR_ACK:
			if ( MACFrameIsAcked(hFrame) )
			{
				MACFrameRemove(hFrame);

				ConsolePutString("MAC: MACIsAssociated(): Valid ACK received, asking for response...\r\n");
				smAssociation = SM_WAIT_FOR_ASSOC_RESP;

			}
			else if ( TickGetDiff(TickGet(), macStartTick) >= MAC_ACK_WAIT_DURATION )
			{
				// Remove current frame and try again, if needed.
				MACFrameRemove(hFrame);

				if ( macFrameRetryCount == 0 )
				{
					ConsolePutString("MAC: MACIsAssociated(): No ACK.\r\n");
	               
					result = TRUE;
				}
				else
				{
					ConsolePutString("MAC: MACIsAssociated(): ACK Timeout, retrying...\r\n");
					smAssociation = SM_SEND_ASSOCIATE_REQ;
				}
			}

			break;


		case SM_WAIT_FOR_ASSOC_RESP:
			if( MACIsGetReady() )
			{
	            
				if (MACIsAssocResponse())
				{

	#if defined(MAC_USE_SECURITY)
					if ( (macCurrentFrame.frameCONLSB.Val & 0x2B == 0x2B) &&
						 (macCurrentFrame.frameCONMSB.Val & 0xCC == 0xCC) )
	#else
					if ( ((BYTE)(macCurrentFrame.frameCONLSB.Val & 0x23) == (BYTE)0x23) &&
						 ((BYTE)(macCurrentFrame.frameCONMSB.Val & 0xCC) == (BYTE)0xCC) )
	#endif
					{
	                   
							//du 抛弃掉分配的short address
							MACGet();
							MACGet();

							macAssociationStatus = MACGet();

							if ( macAssociationStatus  != MAC_ASSOCIATE_SUCCESS )
							{
								ConsolePutString("MAC: MACIsAssociated(): Association attempt has failed.\r\n");
								result = TRUE;
							}

							else
							{

	                        

								ConsolePutString("MAC: MACIsAssociated(): Association successful.\r\n");

								macState.bits.bIsAssociated = TRUE;

								result = TRUE;

								PANDesc.CoordAddress.longAddr.v[0] = macCurrentFrame.src.longAddr.v[0];
								PANDesc.CoordAddress.longAddr.v[1] = macCurrentFrame.src.longAddr.v[1];
								PANDesc.CoordAddress.longAddr.v[2] = macCurrentFrame.src.longAddr.v[2];
								PANDesc.CoordAddress.longAddr.v[3] = macCurrentFrame.src.longAddr.v[3];
								PANDesc.CoordAddress.longAddr.v[4] = macCurrentFrame.src.longAddr.v[4];
								PANDesc.CoordAddress.longAddr.v[5] = macCurrentFrame.src.longAddr.v[5];
								PANDesc.CoordAddress.longAddr.v[6] = macCurrentFrame.src.longAddr.v[6];
								PANDesc.CoordAddress.longAddr.v[7] = macCurrentFrame.src.longAddr.v[7];

								MACSetPANIdLSB(macCoordInfo.panID.byte.LSB);
								MACSetPANIdMSB(macCoordInfo.panID.byte.MSB);

								MACUpdateAddressInfo();
								ConsolePutString("PANDesc.CoordAddress.longAddr:");
								for(i = 0;i < 8;i ++)
									ConsolePut(PANDesc.CoordAddress.longAddr.v[i]);
								
							}
	                    
					}
				}
				MACDiscardRx();
			}
			else if ( TickGetDiff(TickGet(), macStartTick) >= MAC_RESPONSE_WAIT_TIME )
			{
				if ( macFrameRetryCount == 0 )
				{
					ConsolePutString("MAC: MACIsAssociated(): No DATA REQ reply.\r\n");
	                
					result = TRUE;
				}
				else
				{
					ConsolePutString("MAC: MACIsAssociated(): DATA REQ reply timeout, retrying...\r\n");
					smAssociation = SM_SEND_ASSOCIATE_REQ;
				}
			}

			break;

		}
		return result;
	}

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
	void MACStartOrphanNotification(void)
	{
		// Once we start the orphan process, we assume that we are no
		// longer associated with network - we are trying to find our
		// previous network.
		macState.bits.bIsAssociated = FALSE;

		// This is how many times we should retry before giving out.
		macFrameRetryCount = MAC_MAX_FRAME_RETRIES;

		// Start with sending the notice.
		smAssociation = SM_SEND_ORPHAN_NOTICE;
	}

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
	BOOL MACIsOrphanNotificationComplete(void)
	{
		BOOL result;

		// Begin with not ready return value.
		result = FALSE;

		switch(smAssociation)
		{
		case SM_SEND_ORPHAN_NOTICE:
			ConsolePutString("MAC: MACIsOrphanNotificationComplete(): Sending an orphan request...\r\n");

			// This is a retry
			macFrameRetryCount--;

			// macInfo is our address - source address in this case.
			// As per spec, source address would be 64-bit address.
			macInfo.addrMode = MAC_SRC_LONG_ADDR;
			// And no PAN id.
			macInfo.panID.Val = 0xffff;

			// Destination will use not yet known short address with no PAN id.
			macCoordInfo.addrMode = MAC_DST_SHORT_ADDR;
			macCoordInfo.shortAddr.Val = 0xffff;
			macCoordInfo.panID.Val = 0xffff;

			// Prepare the header.
			MACPutHeader(&macCoordInfo,
						 MAC_FRAME_CMD |
						 MAC_SECURITY |
						 MAC_FRAME_PENDING_NO |
						 MAC_ACK_NO |
						 MAC_INTRA_PAN_NO);

			// Copy orphan norification specific bytes in payload area.
			PHYBeginTxFIFOAccess();
			PHYPut(MAC_CMD_ORPHAN_NOTICE);
			PHYEndTxFIFOAccess();

			// Update transmit frame length to account for one byte
			// that we added manually.
			macPacketLen++;

			// Mark this frame as ready to be transmitted.
			MACFlush();

			// Remember when we sent the frame.
			macStartTick = TickGet();

			// We will now wait for ACK from coordinator.
			smAssociation = SM_WAIT_FOR_COORD_ALIGNMENT;
			break;

		case SM_WAIT_FOR_COORD_ALIGNMENT:
			// See if we received any thing...
			if( MACIsGetReady() )
			{
				// The coordinator alignment frame has 7 data bytes in addition to command identifier.
				if ( macCurrentFrame.frameLength == 7 )
				{
					// Make sure that this is a coordinator realignment frame
					if ( MACIsCommand() )
					{
						// Is this coordinator reliang command?
						// Coordinator will respond with coordinator realignment frame
						// to our orphan notice command.
						if ( MACIsCoordRealign() )
						{
							// Now make sure that the frame header is correct according to realignment frame.
							// TBD

							// If we receive realignment frame, it means that we are in fact
							// orphan to this coordinator. In that case, this response will
							// contain our previously assigned short address.
							// Fetch PAN id.
							macInfo.panID.byte.LSB = MACGet();
							macInfo.panID.byte.MSB = MACGet();

							// Coordinator's PAN address is same as what is given to us
							macCoordInfo.panID.Val = macInfo.panID.Val;


							// Fetch coordinator short address.
							macCoordInfo.shortAddr.byte.LSB = MACGet();
							macCoordInfo.shortAddr.byte.MSB = MACGet();

							// Remember the channel.
							macCurrentChannel = MACGet();

							//du 忽略short address
							MACGet();
							MACGet();

							// Now that we have our PAN id, update transciever buffer.
							MACUpdateAddressInfo();

							// Indicate that this process was successful.
	                       

							// Remember that we are now associated.
							macState.bits.bIsAssociated = TRUE;

							// Return value.
							result = TRUE;
						}
					}
				}

				// Discard now processed frame.
				MACDiscardRx();

			}

			// Keep track of timeout so that we do not wait forever for the response.
			else if ( TickGetDiff(TickGet(), macStartTick) >= MAC_RESPONSE_WAIT_TIME )
			{
				ConsolePutString("MAC: MACIsOrphanNotificationComplete(): ORPHAN Response timeout.\r\n");

				// For every timeout, also make sure that we retry predefined number
				// of times.
				if ( macFrameRetryCount == 0 )
				{
					ConsolePutString("MAC: MACIsOrphanNotificationComplete(): ORPHAN Response retry timeout.\r\n");

					// We exhausted retry count, set an error - NO_ACK
					SetZError(ZCODE_NO_ACK);

					// Done.
					result = TRUE;
				}
				else
				{
					ConsolePutString("MAC: MACIsOrphanNotificationComplete(): Trying ORPHAN scan again...\r\n");

					// We still have not exhausted retry count, try again...
					smAssociation = SM_SEND_ORPHAN_NOTICE;
				}
			}
		}

		return result;
	}

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
void MACAcceptCurrentPAN(void)
{
    // Coordinator info will be same as what was received in beacon.
    if ( PANDesc.Flags.bits.CoordAddrMode == TRUE )
    {
        macCoordInfo.addrMode = MAC_DST_LONG_ADDR;
        macCoordInfo.longAddr.v[0] = PANDesc.CoordAddress.longAddr.v[0];
        macCoordInfo.longAddr.v[1] = PANDesc.CoordAddress.longAddr.v[1];
        macCoordInfo.longAddr.v[2] = PANDesc.CoordAddress.longAddr.v[2];
        macCoordInfo.longAddr.v[3] = PANDesc.CoordAddress.longAddr.v[3];
        macCoordInfo.longAddr.v[4] = PANDesc.CoordAddress.longAddr.v[4];
        macCoordInfo.longAddr.v[5] = PANDesc.CoordAddress.longAddr.v[5];
        macCoordInfo.longAddr.v[6] = PANDesc.CoordAddress.longAddr.v[6];
        macCoordInfo.longAddr.v[7] = PANDesc.CoordAddress.longAddr.v[7];
    }

    else
    {
        macCoordInfo.shortAddr = PANDesc.CoordAddress.shortAddr;
        macCoordInfo.addrMode = MAC_DST_SHORT_ADDR;
    }

    macCoordInfo.panID.Val = PANDesc.CoordPANId.Val;

    MACSetPANIdLSB(macCoordInfo.panID.byte.LSB);
    MACSetPANIdMSB(macCoordInfo.panID.byte.MSB);

    MACUpdateAddressInfo();

    
}

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
void MACStartDisassociation(void)
{
    macFrameRetryCount = MAC_MAX_FRAME_RETRIES;
    smAssociation = SM_SEND_DISASSOCIATE_REQ;
}

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
BOOL MACIsDisassociationComplete(void)
{
    BOOL result;

    result = FALSE;
    switch(smAssociation)
    {
    case SM_SEND_DISASSOCIATE_REQ:
SM_SEND_DISASSOCIATE_REQ_LABEL:
        macFrameRetryCount--;

        // When disassociating, provide long address.
        macInfo.addrMode    = MAC_SRC_LONG_ADDR;

        MACPutHeader(&macCoordInfo,            // Dest
                      MAC_FRAME_CMD |           // Frame Control LSB
                      MAC_SECURITY |
                      MAC_FRAME_PENDING_NO |
                      MAC_ACK_YES |
                      MAC_INTRA_PAN_NO);



        // Load command specific data
        PHYBeginTxFIFOAccess();

        // Actual command code
        PHYPutTxData(MAC_CMD_DISASSOCIATE_NOTICE);

        // Leave reason
        PHYPutTxData(0x02);                         // Device wants to leave the PAN.

        // End of command specific data.
        PHYEndTxFIFOAccess();

        // Manually update packet length here instead of calling MACPut() and adding more code
        // and time.
        macPacketLen += 2;

        MACFlush();

        ConsolePutString("MAC: MACIsDisassociationComplete(): Sent Disassociation notice...\r\n");

        // Start the timeout tick.
        macStartTick = TickGet();
        smAssociation = SM_WAIT_FOR_ACK;
        break;

    case SM_WAIT_FOR_ACK:
        if ( MACFrameIsAcked(hFrame) )
        {
            MACFrameRemove(hFrame);

            ConsolePutString("MAC: MACIsDisassociationComplete(): ACK Received.\r\n");

            // Forget PAN ID.
            MACSetPANIdLSB(0xff);
            MACSetPANIdMSB(0xff);

            // Update the transceiver
            MACUpdateAddressInfo();

            // Function is done.
            result = TRUE;
        }

        else if ( TickGetDiff(TickGet(), macStartTick) >= MAC_ACK_WAIT_DURATION )
        {
            // Remove current frame and try again, if needed
            MACFrameRemove(hFrame);

            // If retries are exhausted, set error code and finish.
            if ( macFrameRetryCount == 0 )
            {
                SetZError(ZCODE_NO_ACK);
                result = TRUE;
            }

            ConsolePutString("MAC: MACIsDisassociationComplete(): ACK timeout.\r\n");

            // Try again.
            smAssociation = SM_SEND_DISASSOCIATE_REQ;

        }
    }


    return result;
}

#endif

/*********************************************************************
 * Function:        static void MACSendAck(ACK_TYPE typeOfAck)
 *
 * PreCondition:    A valid frame has just arrived.
 *
 * Input:           typeOfAck - either ACK_NO_DATA or ACK_NO_DATA_PENDING
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Sends ACK frame in response to just received
 *                  frame.
 *
 * Note:            This is a private function
 ********************************************************************/
static void MACSendAck(ACK_TYPE ack)
{
    // First, update the packet length.
    PHYBegin();

    PHYSendAck(ack);

    // Terminate RAM access
    PHYEnd();

    //TODO(DF16) - Investigate error.  Without the following delay the
    //FCS value of the ACK packet gets corrupted/ miscalculated.
    {BYTE i = 0x40; while(++i){nop();}}
}

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
BOOL MACProcessAssociation(void)
{
    BOOL lbResult = FALSE;

   

    // There must be exactly two bytes of data in association request frame.
    if( macCurrentFrame.frameLength == 1 )
    {
        // Make sure that current frame is indeed association request.
        // LSB = Mask: 0b00100111 Value: 0b00100011,

        if ( ((BYTE)(macCurrentFrame.frameCONLSB.Val & 0x27) == (BYTE)0x23))
        {
            macCurrentFrame.capInfo.Val = MACGet();

            return TRUE;
        }
    }

    return FALSE;
}

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
                              MAC_ASSOCIATE_STATUS status)
{
ConsolePutString("send response\n");
    macInfo.addrMode = MAC_SRC_LONG_ADDR;

    // Use information from macCurrentFrame to build response frame.
    // If we arrive at this point, macCurrentFrame must be a valid
    // Associate Request and as a result, we will only change those fields
    // that needs to changed as part of response frame format.
    macCurrentFrame.src.addrMode = MAC_DST_LONG_ADDR;

    // Dest panID will be same as ours.
    macCurrentFrame.src.panID.Val = macInfo.panID.Val;

    MACPutHeader(&macCurrentFrame.src,         // Dest
                  MAC_FRAME_CMD |           // Frame Control LSB
                  MAC_SECURITY |
                  MAC_FRAME_PENDING_NO |
                  MAC_ACK_YES |
                  MAC_INTRA_PAN_NO);



    PHYBeginTxFIFOAccess();
    PHYPutTxData(MAC_CMD_ASSOCIATE_RPLY);
    PHYPutTxData(assocShortAddr->byte.LSB);
    PHYPutTxData(assocShortAddr->byte.MSB);
    PHYPutTxData((BYTE)status);
    PHYEndTxFIFOAccess();

    // Manually update packet length here instead of calling MACPut() and adding more code
    // and time.
    macPacketLen += 4;

    return MACFlush();

}

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
HFRAME MACSendRealignment(SHORT_ADDR *destShortAddr, BOOL bBroadcast)
{
    BYTE frameFlags;

    macInfo.addrMode = MAC_SRC_LONG_ADDR;
    // Other information is already set.

    // If this is a unicast, we need to use LONG address.
    // Since we already have remote device's address information
    // in current frame, we will use current frame's source as
    // destination for this frame.
    if ( bBroadcast == FALSE )
    {
        macCurrentFrame.src.addrMode = MAC_DST_LONG_ADDR;

        // For unicast frame, request ack.
        frameFlags = MAC_FRAME_CMD |
                     MAC_SECURITY |
                     MAC_FRAME_PENDING_NO |
                     MAC_ACK_YES |
                     MAC_INTRA_PAN_NO;
    }

    else
    {
        macCurrentFrame.src.addrMode = MAC_DST_SHORT_ADDR;
        macCurrentFrame.src.shortAddr.Val = 0xffff;

        // For broadcast, do not request ack.
        frameFlags = MAC_FRAME_CMD |
                     MAC_SECURITY |
                     MAC_FRAME_PENDING_NO |
                     MAC_ACK_NO |
                     MAC_INTRA_PAN_NO;
    }

    // The destination PAN will be 0xFFFF
    macCurrentFrame.src.panID.Val = 0xffff;

    MACPutHeader(&macCurrentFrame.src, frameFlags);

    // Now load coord realignment specific payload.
    PHYBegin();
    PHYSelectTxFIFO();
    PHYPut(MAC_CMD_COORD_REALIGNMENT);

    // Current PAN ID.
    PHYPut(macInfo.panID.byte.LSB);
    PHYPut(macInfo.panID.byte.MSB);

    //du 不一定是真正的coord
    PHYPut(macInfo.shortAddr.byte.LSB);
    PHYPut(macInfo.shortAddr.byte.MSB);

    // Logical channel
    PHYPut(macCurrentChannel);

    // If this is a unicast, supply dest short addr.
    if ( bBroadcast == FALSE )
    {	
		//du 收到后被忽略
        PHYPut(destShortAddr->byte.LSB);
        PHYPut(destShortAddr->byte.MSB);
    }
    else
    {
        // For broadcast, this will be 0xffff.
        PHYPut(0xff);
        PHYPut(0xff);
    }

    PHYEnd();

    // Update MACPacketLen manually.
    macPacketLen += 8;

    return MACFlush();

}

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
#if defined(I_AM_COORDINATOR)
BOOL MACProcessDisassociation(void)
{
    // There must be exactly two bytes of data in association request frame.
    if( macCurrentFrame.frameLength == 1 )
    {
        // A disassociation request must have
        // 1. Both source and destination addressing mode = 64-bit addresses.
        // 2. Frame pending field set to 0.
        // 3. Acknowledge field set to 1
        //
        // Make sure that current frame is indeed association request.
        // LSB = Mask: 0b00100111 Value: 0b00100011,
        // MSB = Mask: 0b11001100 Value: 0b11001100
        if ( ((BYTE)(macCurrentFrame.frameCONLSB.Val & 0x27) == (BYTE)0x23) &&
             ((BYTE)(macCurrentFrame.frameCONMSB.Val & 0xCC) == (BYTE)0xCC) )
        {
            // Fetch and discard disassociation reason.
            MACGet();

            return TRUE;
        }
    }

    return FALSE;
}
#endif

/*********************************************************************
 * Function:        static void MACSendBeaconReq(void)
 *
 * PreCondition:    MACIsPutready() = TRUE
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Sends out beacon request
 *
 * Note:            None
 ********************************************************************/
static void MACSendBeaconReq(void)
{
    // Now that we are trying to associate, forget previous PAN ID
    macInfo.addrMode    = MAC_SRC_NO_ADDR;

    // Coordinator info will be same as what was received in beacon.
    // But for time being use hardcoded value to test out...
    macCoordInfo.addrMode       = MAC_DST_SHORT_ADDR;
    macCoordInfo.shortAddr.Val  = 0xffff;
    macCoordInfo.panID.Val      = 0xffff;

    MACPutHeader(&macCoordInfo,             // Dest
                  MAC_FRAME_CMD |           // Frame Control LSB
                  MAC_SECURITY |
                  MAC_FRAME_PENDING_NO |
                  MAC_ACK_NO |
                  MAC_INTRA_PAN_NO);


    // Load beacon req specific bytes in payload area.
    PHYBeginTxFIFOAccess();
    PHYPutTxData(MAC_CMD_BEACON_REQ);
    PHYEndTxFIFOAccess();

    // Manually update packet length here instead of calling MACPut() and adding more code
    // and time.
    macPacketLen++;

    // Mark it as ready to transmit
    MACFlush();


}

/*********************************************************************
 * Function:        static void MACSendBeacon(void)
 *
 * PreCondition:    MACIsPutready() = TRUE
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Sends out beacon
 *
 * Note:            Available to coordinator only
 ********************************************************************/
static void MACSendBeacon(void)
{

    macInfo.addrMode = MAC_SRC_SHORT_ADDR;

    //du beacon frame没有目的地址
    macDestInfo.addrMode = MAC_DST_NO_ADDR;

    MACPutHeader(&macDestInfo,                 // Dest
                  MAC_FRAME_BEACON |            // Frame Control LSB
                  MAC_SECURITY |
                  MAC_FRAME_PENDING_NO |
                  MAC_ACK_NO |
                  MAC_INTRA_PAN_NO);


    // Load beacon specific bytes into payload area.
    PHYBeginTxFIFOAccess();
    PHYPutTxData(SUPERFRAME_SPEC_LSB);
    PHYPutTxData(SUPERFRAME_SPEC_MSB);
    PHYPutTxData(GTS_FIELD_VAL);

    // Pending address field - TBD
    PHYPutTxData(0x00);

    //Protocol ID
    //0x00 as specified in table 45 NWK spec
    PHYPutTxData(0x00);

    //Stack profile (8-11) and nwkcProtocolVersion (12-15)
    //nwkcProtocolVersion = 0x01 because ZigBee v1.0
    PHYPutTxData(0x10);

    //nwkSecurityLevel (16-18)
        //0x00 - no security
    //Device router capacity (19)
        //FALSE because we only do star networks right now (no routers)
    //Device depth (20-22)
        //0x00 because we are coordinator
    //device capacity (23)
        //TRUE - we have room
    PHYPutTxData(0x80);

    //TxOffset
    //only included in multihop beaconing networks
    //PHYPutTxData(TxOffset);

    PHYEndTxFIFOAccess();

    // Manually update packet length here instead of calling MACPut() and adding more code
    // and time.
    //macPacketLen += 4;
    macPacketLen += 7;

    MACFlush();
	ConsolePutString("send beacon successfully\n");
    //while(1){wdt_reset();}
}

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
void MACStartScan(BOOL bActiveScan)
{
    // Send beacon if this is active scan
    if ( bActiveScan )
    {
        // First send out a beacon
        MACSendBeaconReq();
	
    }


    // Remember time so that we can know when to abort.
    macStartTick = TickGet();

    // Zero out number of discovered coordinators
    PANDescCount = 0;

}

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
BOOL MACIsScanComplete(void)
{
    BOOL result = FALSE;

   

    if ( MACIsGetReady() )
    {
		
        if ( MACIsBeacon() )
        {
			ConsolePutString("receive Beacon\n");
            if ( MACProcessBeacon() )
            {
                result = TRUE;
            }
        }

        MACDiscardRx();
    }

    else if ( TickGetDiff(TickGet(), macStartTick) >= MAC_ACTIVE_SCAN_PERIOD )
        result = TRUE;




    return result;
}

/*********************************************************************
 * Function:        static BOOL MACProcessBeacon(void)
 *
 * PreCondition:    MACGetHeader() is called and a valid beacon
 *                  frame was detected.
 *
 * Input:           None
 *
 * Output:          TRUE if this is a valid beacon frame
 *                  FALSE otherwise.
 *
 * Side Effects:    None
 *
 * Overview:        Fetches rest of the frame and validates it as per
 *                  IEEE spec.
 *
 * Note:            None
 ********************************************************************/
static BOOL MACProcessBeacon(void)
{
    BOOL lbResult;

    lbResult = FALSE;

    // There must be at least 4 bytes in MAC payload.
    if( macCurrentFrame.frameLength >= 4 )
    {
        // Clear PAN Desc flags.
        PANDesc.Flags.Val = 0x00;

        //du 当前版本存贮short address（不一定真正coord）
        if ( macCurrentFrame.src.addrMode == MAC_SRC_LONG_ADDR )
        {
            PANDesc.Flags.bits.CoordAddrMode = 1;
            PANDesc.CoordAddress.longAddr.v[0] = macCurrentFrame.src.longAddr.v[0];
            PANDesc.CoordAddress.longAddr.v[1] = macCurrentFrame.src.longAddr.v[1];
            PANDesc.CoordAddress.longAddr.v[2] = macCurrentFrame.src.longAddr.v[2];
            PANDesc.CoordAddress.longAddr.v[3] = macCurrentFrame.src.longAddr.v[3];
            PANDesc.CoordAddress.longAddr.v[4] = macCurrentFrame.src.longAddr.v[4];
            PANDesc.CoordAddress.longAddr.v[5] = macCurrentFrame.src.longAddr.v[5];
            PANDesc.CoordAddress.longAddr.v[6] = macCurrentFrame.src.longAddr.v[6];
            PANDesc.CoordAddress.longAddr.v[7] = macCurrentFrame.src.longAddr.v[7];
        }
        else
        {
            PANDesc.CoordAddress.shortAddr.Val = macCurrentFrame.src.shortAddr.Val;
        }

        PANDesc.CoordPANId.Val = macCurrentFrame.src.panID.Val;

        PANDesc.LogicalChannel = macCurrentChannel;

        // Now fetch data from  Beacon frame and save it to PANDesc.
        PANDesc.SuperFrameSpec.byte.LSB = MACGet();
        PANDesc.SuperFrameSpec.byte.MSB = MACGet();

        PANDesc.TimeStamp.Val = TickGet();

        // per TABLE 41 NWK spec.
        PANDesc.ACLEntry = 0x08;

        // To be added - process GTS fields, Pending address fields and beacon payload.

        PANDescCount++;
        lbResult = TRUE;

    }

    return lbResult;
}

/*********************************************************************
 * Function:        void MACStartED(void)
 *
 * PreCondition:    MACGetHeader() is called and a valid beacon
 *                  frame was detected.
 *
 * Input:           None
 *
 * Output:          TRUE if this is a valid beacon frame
 *                  FALSE otherwise.
 *
 * Side Effects:    None
 *
 * Overview:        Fetches rest of the frame and validates it as per
 *                  IEEE spec.
 *
 * Note:            None
 ********************************************************************/
void MACStartED(void)
{
    macStartTick = TickGet();
    PHYSetTRXState(PHY_TRX_RX_ON);
}

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
BOOL MACIsEDComplete(void)
{

	return ( TickGetDiff(TickGet(), macStartTick) >= MAC_ED_SCAN_PERIOD );

}

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
void MACDiscardRx(void)
{
    // Account for CRC/(RSSI + CORRELATION for CC2420)
    macCurrentFrame.frameLength += 2;

    
    if ( macCurrentFrame.frameLength != 2 )
        macCurrentFrame.frameLength = macCurrentFrame.frameLength;

    while( macCurrentFrame.frameLength )
    {
        MACGet();
    }

    // Mark this frame as empty (macCurrentFrame.Flags.bits.bIsGetReady = FALSE)
    macCurrentFrame.Flags.Val = 0x00;
	
	

}

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
void MACUpdateAddressInfo(void)
{
    PHYBegin();

    PHYSelectIEEEAddrWrite();
    PHYPut(macInfo.longAddr.v[0]);
    PHYPut(macInfo.longAddr.v[1]);
    PHYPut(macInfo.longAddr.v[2]);
    PHYPut(macInfo.longAddr.v[3]);
    PHYPut(macInfo.longAddr.v[4]);
    PHYPut(macInfo.longAddr.v[5]);
    PHYPut(macInfo.longAddr.v[6]);
    PHYPut(macInfo.longAddr.v[7]);
    PHYEnd();

    PHYBegin();
    PHYSelectPANAddrWrite();
    PHYPut(macInfo.panID.byte.LSB);
    PHYPut(macInfo.panID.byte.MSB);
    PHYEnd();

    PHYBegin();
    PHYSelectShortAddrWrite();
    PHYPut(macInfo.shortAddr.byte.LSB);
    PHYPut(macInfo.shortAddr.byte.MSB);
    PHYEnd();

}

/*********************************************************************
 * Function:        static HFRAME AddDSNInQueue(BYTE dsn)
 *
 * PreCondition:    MACInit() is called
 *
 * Input:           dsn     - Data Sequence Number
 *
 * Output:          index to queue where given dsn was stored.
 *                  HFRAME_INVALID if queue is full.
 *
 * Side Effects:    None
 *
 * Overview:        Searches DSN queue for empty entry and places given
 *                  dsn into that entry.
 *
 * Note:            None
 ********************************************************************/
static HFRAME AddDSNInQueue(BYTE dsn)
{
    BYTE i;
    MAC_FRAME_STATUS *pMACFrameStatus;

    pMACFrameStatus = macFrameStatusQ;
    for ( i = 0; i < sizeof(macFrameStatusQ)/sizeof(macFrameStatusQ[0]); i++ )
    {
        if ( !pMACFrameStatus->Flags.bits.bIsInUse )
        {
            pMACFrameStatus->Flags.bits.bIsInUse = TRUE;
            pMACFrameStatus->Flags.bits.bIsConfirmed = FALSE;
            pMACFrameStatus->macDSN = dsn;
            pMACFrameStatus->lastTick = TickGet();
            pMACFrameStatus->retryCount = MAC_MAX_FRAME_RETRIES;
            macFrameStatusQLen++;
			ConsolePut(i+48);
            ConsolePutString("MAC: Added one frame to queue.\r\n");

            return (HFRAME)i;
        }
        else
        {
            pMACFrameStatus++;
        }
    }
	ConsolePutString("MAC:not added one frame to queue\n");
    return HFRAME_INVALID;
}

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
void MACFrameRemove(HFRAME h)
{
		
    if ( macFrameStatusQ[h].Flags.bits.bIsInUse )
    {
        macFrameStatusQ[h].Flags.Val = 0x00;
        macFrameStatusQLen--;
		ConsolePut(h+48);
        ConsolePutString("MAC: Removed one frame from queue.\r\n");
    }
}

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
//TODO(DF13): possible code reduction (minor)
BOOL MACFrameIsAcked(HFRAME h)
{
    BOOL result;
	ConsolePut(h+48);
    // If given frame does not exist in queue, it is said to be ack'ed.
    if ( h == HFRAME_INVALID )
    {
        ConsolePutString("invalid hfame\n");
		return FALSE;
    }

    // Save the ack result.
    if ( macFrameStatusQ[h].Flags.bits.bIsInUse )
    {	
		
        result = macFrameStatusQ[h].Flags.bits.bIsConfirmed;
		if (result)
			 ConsolePutString("frame is confirmed\n");
#if 0
		else
			 ConsolePutString("frame is not confirmed\n");
#endif
    }
    else
    {
        ConsolePutString("frame is not in use\n");
		result = FALSE;
    }

    return result;
}

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
 *                  DSN queue so that future ack can be matched
 *                  against this frame.
 *
 * Note:            None
 ********************************************************************/
void MACPutHeader(NODE_INFO *dest, BYTE frameCON)
{
    BYTE destAddrMode;
    BYTE srcAddrMode;
    BYTE frameCONMSB;

    // If ack is requested, remember the DSN so that we can match
    // ack with right frame.
    if ( frameCON & MAC_ACK_YES )
    {
        
		macCurrentFrame.Flags.bits.bToBeQueued = TRUE;
    }
    else
    {
        macCurrentFrame.Flags.bits.bToBeQueued = FALSE;
    }

    PHYBegin();
    // Clear TXFIFO
    PHYFlushTx();
    PHYEnd();
	ConsolePut(0x01);
	ConsolePut(0x02);
	ConsolePut(0x03);
	ConsolePut(0x04);
	ConsolePut(0x05);
    // Now start writing packet header - actual write will be to either
    // RF chip buffer or RAM tx indirect buffer.
    PHYBeginTxFIFOAccess();
	
    // Write length byte
    PHYPutTxData(0x00);
	
    // Extract and save src and dest address mode quick access
//#if defined(WIN32)
    if ( (frameCON & MAC_FRAME_TYPE_MASK) != MAC_FRAME_ACK)

    {
        destAddrMode = dest->addrMode;
        srcAddrMode = macInfo.addrMode;
    }
    else
    {
		
        destAddrMode = 0x00;
        srcAddrMode = 0x00;
    }


    // Create Frame CON MSB.
    frameCONMSB = destAddrMode | srcAddrMode;

    // Put frame control LSB
    PHYPutTxData(frameCON);
	ConsolePut(frameCON);
    // Now MSB
    PHYPutTxData(frameCONMSB);
	ConsolePut(frameCONMSB);
    // Sequence number

    if ( (frameCON & MAC_FRAME_TYPE_MASK) == MAC_FRAME_ACK )
    {
        PHYPutTxData(macCurrentFrame.macDSN);ConsolePut(macCurrentFrame.macDSN);
    }
    else
    {
        // For every non-ACK type frame, we will increment DSN.
        macDSN++;

        // Write it.
        PHYPutTxData(macDSN);
		ConsolePut(macDSN);
        // Remember it so that we can match its ACK frame,
        lastMACDSN = macDSN;
    }



    // As we build the header, keep track of the total packet length.
    // 5 bytes so far.
    macPacketLen = 5;

    // Include Destination PAN identifier only if dest addr mode is non-zero.
    if ( destAddrMode )
    {
		
        PHYPutTxData(dest->panID.byte.LSB);
        PHYPutTxData(dest->panID.byte.MSB);
		ConsolePut(dest->panID.byte.MSB);
		ConsolePut(dest->panID.byte.LSB);
        // Two more bytes...
        macPacketLen += 2;
    }


    // Include destination address as per frame control MSB
    // By default assume short address.
    if ( destAddrMode == MAC_DST_SHORT_ADDR )
    {
        PHYPutTxData(dest->shortAddr.byte.LSB);ConsolePut(dest->shortAddr.byte.LSB);
        PHYPutTxData(dest->shortAddr.byte.MSB);ConsolePut(dest->shortAddr.byte.MSB);
		ConsolePut(dest->shortAddr.byte.MSB);
		ConsolePut(dest->shortAddr.byte.LSB);


        macPacketLen += 2;
    }
    else if ( destAddrMode == MAC_DST_LONG_ADDR )
    {
		
        PHYPutTxData(dest->longAddr.v[0]);ConsolePut(dest->longAddr.v[0]);
        PHYPutTxData(dest->longAddr.v[1]);ConsolePut(dest->longAddr.v[1]);
        PHYPutTxData(dest->longAddr.v[2]);ConsolePut(dest->longAddr.v[2]);
        PHYPutTxData(dest->longAddr.v[3]);ConsolePut(dest->longAddr.v[3]);
        PHYPutTxData(dest->longAddr.v[4]);ConsolePut(dest->longAddr.v[4]);
        PHYPutTxData(dest->longAddr.v[5]);ConsolePut(dest->longAddr.v[5]);
        PHYPutTxData(dest->longAddr.v[6]);ConsolePut(dest->longAddr.v[6]);
        PHYPutTxData(dest->longAddr.v[7]);ConsolePut(dest->longAddr.v[7]);

        // 8 more bytes
        macPacketLen += 8;
    }




    // Include Source PAN identifier only if src addr mode is non-zero AND
    // IntraPAN is '0'.
    if ( (((BYTE_VAL*)(&frameCON))->bits.b6 == 0) && srcAddrMode )
    {
		
        PHYPutTxData(macInfo.panID.byte.LSB);
		ConsolePut(macInfo.panID.byte.LSB);
        PHYPutTxData(macInfo.panID.byte.MSB);
		ConsolePut(macInfo.panID.byte.MSB);

        // Two more bytes
        macPacketLen += 2;
    }

    if ( srcAddrMode == MAC_SRC_SHORT_ADDR )
    {
        PHYPutTxData(macInfo.shortAddr.byte.LSB);
		ConsolePut(macInfo.shortAddr.byte.LSB);
        PHYPutTxData(macInfo.shortAddr.byte.MSB);
		ConsolePut(macInfo.shortAddr.byte.MSB);

        // Two more bytes...
        macPacketLen += 2;
    }
    else if ( srcAddrMode == MAC_SRC_LONG_ADDR )
    {
        PHYPutTxData(macInfo.longAddr.v[0]);
		ConsolePut(macInfo.longAddr.v[0]);
        PHYPutTxData(macInfo.longAddr.v[1]);
		ConsolePut(macInfo.longAddr.v[1]);
        PHYPutTxData(macInfo.longAddr.v[2]);
		ConsolePut(macInfo.longAddr.v[2]);
        PHYPutTxData(macInfo.longAddr.v[3]);
		ConsolePut(macInfo.longAddr.v[3]);
        PHYPutTxData(macInfo.longAddr.v[4]);
		ConsolePut(macInfo.longAddr.v[4]);
        PHYPutTxData(macInfo.longAddr.v[5]);
		ConsolePut(macInfo.longAddr.v[5]);
        PHYPutTxData(macInfo.longAddr.v[6]);
		ConsolePut(macInfo.longAddr.v[6]);
        PHYPutTxData(macInfo.longAddr.v[7]);
		ConsolePut(macInfo.longAddr.v[7]);

        // 8 more bytes
        macPacketLen += 8;
    }

    PHYEndTxFIFOAccess();

}

/*********************************************************************
 * Function:        static void UpdateQueue(BYTE dsn)
 *
 * PreCondition:    None
 *
 * Input:           dsn     - DSN that is to checked
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Searches DSN queue for matching dsn.
 *                  If matching entry is found, that DSN is said
 *                  to have been ack'ed.
 *
 * Note:            None
 ********************************************************************/
static void UpdateQueue(BYTE dsn)
{
    BYTE i;
    MAC_FRAME_STATUS *pMACFrameStatus;

    pMACFrameStatus = macFrameStatusQ;
    for ( i = 0; i < sizeof(macFrameStatusQ)/sizeof(macFrameStatusQ[0]); i++, pMACFrameStatus++ )
    {
        if ( pMACFrameStatus->Flags.bits.bIsInUse )
        {
            if ( pMACFrameStatus->macDSN == dsn )
            {
                pMACFrameStatus->Flags.bits.bIsConfirmed = TRUE;
                return;
            }
        }
    }
}



void MACPutLongAddress(LONG_ADDR *addr)
{
	PutMACAddress(addr);
	memcpy((void*)&macInfo.longAddr, (void *)addr, (size_t)sizeof(macInfo.longAddr));
}
void MACPutShortAddress(SHORT_ADDR *addr)
{
	PutMACShortAddress(addr);
	MACSetShortAddrLSB(addr->byte.LSB);
	MACSetShortAddrMSB(addr->byte.MSB);
}
BOOL MACIsAddressAssigned(void)
{
    // Read MAC long address from app-specific nonvolatile memory.
    GetMACAddress( &macInfo.longAddr );

    // An address is said to be invalid if its OUI does not match with our preset OUI
    if ( macInfo.longAddr.v[7] != MAC_LONG_ADDR_BYTE7 ||
         macInfo.longAddr.v[6] != MAC_LONG_ADDR_BYTE6 ||
         macInfo.longAddr.v[5] != MAC_LONG_ADDR_BYTE5 )
         return FALSE;

    else
        return TRUE;
}

