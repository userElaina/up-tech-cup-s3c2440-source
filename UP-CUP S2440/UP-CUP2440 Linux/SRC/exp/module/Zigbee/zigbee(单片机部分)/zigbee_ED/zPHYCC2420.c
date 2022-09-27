#include "zPHY.h"

/*
 *  PA_LEVEL determiens output power of transciever
 *          According to Table 9 of CC2420 datasheet
 *
 *          PA_LEVEL (TXCTRL.LSB)       Output Power (dBm)  Current Consumtiion
 *          ====================================================================
 *          0xFF                        0                   17.4 mA
 *          0xFB                        -1                  16.5 mA
 *          0xF7                        -3                  15.2 mA
 *          0xF3                        -5                  13.9 mA
 *          0xEF                        -7                  12.5 mA
 *          0xEB                        -10                 11.2 mA
 *          0xE7                        -15                 9.9 mA
 *          0xE3                        -25                 8.5 mA
 */
#define PA_LEVEL        (0xFF)      // Select one of the above as per your requirement.

BYTE currentChannel;

/*********************************************************************
 * Function:        BOOL PHYInit(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          TRUE if the PHY was successfully initialized
 *                  FALSE if the PHY malfunctioned.  Power cycling
 *                        the PHY and calling PHYInit again might be
 *                        worthwhile.
 *
 * Side Effects:    None
 *
 * Overview:        PHY is initialized
 *
 * Note:            None
 ********************************************************************/
void Delay(WORD i)
{
    while( i-- );
}

//add by lyj 20080417 硬件重新定义
#define PHY_INIT() \
    do { \
	DDRB  |= BM(PHY_SCK) | BM(PHY_MOSI) | BM(PHY_CSn);\
	DDRB  &= ~(BM(PHY_MOSO) | BM(PHY_FIFO));\
	PORTB |= BM(PHY_SCK) | BM(PHY_MOSI) | BM(PHY_CSn);\
	DDRD  &= ~(BM(PHY_CCA) | BM(PHY_SFD_IN));\
	DDRA  |= BM(PHY_RESETn) | BM(PHY_VREG_EN);\
	PORTA |= BM(PHY_RESETn);\
	DDRE  &= ~(BM(PHY_FIFOP) | BM(PHY_SFD));\
    } while (0)



BOOL PHYInit(void)
{
	DDRB  |= BM(PHY_SCK) | BM(PHY_MOSI) | BM(PHY_CSn) | BM(PHY_VREG_EN);
	PORTB |= BM(PHY_SCK) | BM(PHY_MOSI) | BM(PHY_CSn);
	DDRA  |= BM(PHY_VREG_EN) | BM(PHY_RESETn);
	PORTA |= BM(PHY_RESETn);
	PHY_INIT();//add
    typedef union _PHY_STATUS
    {
        struct
        {
            unsigned int :1;
            unsigned int RSSI_VALID : 1;
            unsigned int LOCK : 1;
            unsigned int TX_ACTIVE : 1;
            unsigned int ENC_BUSY : 1;
            unsigned int TX_UNDERFLOW : 1;
            unsigned int XOSC16M_STABLE : 1;
        } bits;
        BYTE Val;
    } PHY_STATUS;

    PHY_STATUS phyStatus;
    BYTE AttemptCount;

    /*
     * Do soft reset of the the chip
     */
    // TODO: Use optimum delay.
    PHY_RESET(0);
    Delay(1000);
    PHY_RESET(1);
    Delay(500);
	

    // Switch oscillator ON here and do other things in parallel
    // while oscillator stabilizes.
	
    PHYBegin();
    PHYPut(STROBE_SXOSCON);
    PHYEnd();
	

    PHYBegin();

    /*
     * For MDMCTRL0, we will accept the default settings of most bits except
     * that we need to do AutoACK
     */
    PHYPut(REG_MDMCTRL0);

    // Set AUTOACK bit in MDMCTRL0 register.
    // 15:14 = '00' = Reserved
    // 13    = '0'  = Reserved frames are rejected
    // 12    = '?'  = '1' if this is coordinator, '0' if otherwise
    // 11    = '1'  = Address decoding is enabled
    // 10:8  = '010'    = CCA Hysteresis in DB - default value
    // 7:6   = '11' = CCA = 1, when RSSI_VAL < CCA_THR - CCA_HYST and not receiving valid IEEE 802.15.4 data
    // 5     = '1'  = Auto CRC
    // 4     = '0'  = Auto ACK
    // 3:0   = '0010' = 3 leading zero bytes - IEEE compliant.

#if defined(I_AM_COORDINATOR)
    // MSB = 0b0001 1010
    PHYPut(0x1A);
	//PHYPut(0x12);
#else
    // MSB = 0b0000 1010
    PHYPut(0x0A);
	//PHYPut(0x02);
#endif

    // LSB = 0b1110 0010
    PHYPut(0xE2);
    PHYEnd();

    /*
     *
     */


    /*
     * For MDMCTRL1, set CORR_THR to 20.
     */

    PHYBegin();
    PHYPut(REG_MDMCTRL1);
    PHYPut(0x05);            // MSB
    PHYPut(0x00);            // LSB
    PHYEnd();

    /*
     *
     */

    PHYBegin();           // Disable MAC security
    PHYPut(REG_SECCTRL0);
    PHYPut(0x01);            // MSB
    PHYPut(0xC4);            // LSB
    PHYEnd();

    // Select desired TX output power level
    PHYBegin();
    PHYPut(REG_TXCTRL);
    // As defined by Table 9 of CC2420 datasheet.
    PHYPut(0xA0);             // MSB
    PHYPut(PA_LEVEL);        // LSB
    PHYEnd();

    // Set FIFOP threshold to full RXFIFO buffer
    PHYBegin();
    PHYPut(REG_IOCFG0);
    PHYPut(0x08);
    PHYPut(0x7F);
    PHYEnd();
	
    // Turn on oscillator
    PHYBegin();

    AttemptCount = 100;
    do                      // Wait for stable oscillator.
    {
        // Execute NOP to read the status.
        PHYPut(STROBE_SNOP);

        phyStatus.Val = PHYGet();
		

    } while( phyStatus.bits.XOSC16M_STABLE == 0 && --AttemptCount);
	
    PHYPut(STROBE_SFLUSHTX);
    PHYPut(STROBE_SFLUSHRX);

    PHYEnd();
	
    return AttemptCount != 0;
}

/*********************************************************************
 * Function:        void PHYSetTxPower(BYTE val)
 *
 * PreCondition:    None
 *
 * Input:           val - Power level as described above
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Changes transmit output power as per given value
 *
 * Note:            None
 ********************************************************************/
void PHYSetTxPower(BYTE val)
{
    // Select desired TX output power level
    PHYBegin();
    PHYPut(REG_TXCTRL);
    // As defined by Table 9 of CC2420 datasheet.
    PHYPut(0xA);                // MSB
    PHYPut(val);                // LSB
    PHYEnd();
}

/*********************************************************************
 * Function:        BOOL PHYProcessRxOverflow(void)
 *
 * PreCondition:    PHYInit() is called
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Chedks for receive overflow and clears as per
 *                  RF chip specific procedure
 *
 * Note:            None
 ********************************************************************/
BOOL PHYProcessRxOverflow(void)
{
    BYTE v;
	char fifo = PINB & BM(PHY_FIFO);
	char fifop = PINE & BM(PHY_FIFOP);

    // Check to see if there was an overflow.
    if (  fifo== 0 &&  fifop!= 0 )
    {
        // As per CC2420, overflow would be handled by
        // reading at least one RX byte and then issuing
        // RX FLUSH command.
        PHYBegin();

        // Select RX FIFO.
        PHYSelectRxFIFO();

        // Read one byte.
        v = PHYGet();

        // End of dummy read sequence.
        PHYEnd();

        // Start RX flush sequence
        PHYBegin();

        // Now issue SFLUSHRX command
        PHYFlushRx();

        PHYEnd();

        return TRUE;
    }

    return FALSE;
}

/*********************************************************************
 * Function:        BYTE CC2420GetStatus(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          Status byte of CC2420
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            None
 ********************************************************************/
BYTE CC2420GetStatus(void)
{
    BYTE v;

    PHYBegin();

    v = PHYGet();

    PHYEnd();

    return v;
}

/*********************************************************************
 * Function:        BOOL PHYSetNextChannel(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          TRUE if next channel was selected
 *                  FALSE, if top channel is reached.
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            None
 ********************************************************************/
BOOL PHYSetNextChannel(void)
{
    // If we reach upper end of available channels, tell app about it.
    if ( currentChannel == 26 )
        return FALSE;

    // We can still go up next channel.
    currentChannel++;

    PHYSetChannel(currentChannel);

    return TRUE;
}

/*********************************************************************
 * Function:        void PHYSetChannel(BYTE channel)
 *
 * PreCondition:    None
 *
 * Input:           channel     - Channel number to set (Must be 11 - 26)
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            None
 ********************************************************************/
void PHYSetChannel(BYTE channel)
{
    currentChannel = channel;

    PHYBegin();
    PHYPut(REG_FSCTRL);
    PHYPut(0x41);            // LOCK_THR = 1 as recommended, with bit 8 = 1 for MSb of FREQ value
    PHYPut((channel-11)*5+101);      // Create raw LSB for given channel
    PHYEnd();
}

/*********************************************************************
 * Function:        BYTE PHYGetED(void)
 *
 * PreCondition:    TXON = TRUE
 *
 * Input:           None
 *
 * Output:          0x00 - 0xff energy level on current channel
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            None
 ********************************************************************/
BYTE PHYGetED(void)
{
    BYTE val;

    PHYBegin();
    PHYPut(REG_RSSI | CMD_READ);

    // Read the value
    val = PHYGet();

    PHYEnd();

    return val;
}

/*********************************************************************
 * Function:        void PHYSetTRXState(PHY_TRX_STATE state)
 *
 * PreCondition:    PHYInit() is called.
 *
 * Input:           state   - State to set
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            None
 ********************************************************************/
void PHYSetTRXState(PHY_TRX_STATE state)
{
    PHYBegin();

    if ( state == PHY_TRX_RX_ON )
    {
        PHYPut(STROBE_SRXON);
    }

    else if ( state == PHY_TRX_OFF || state == PHY_TRX_FORCE_OFF )
    {
        PHYPut(STROBE_SRFOFF);
    }

    else if ( state == PHY_TRX_TX_ON )
    {
        PHYPut(STROBE_STXON);
    }

    PHYEnd();

}

BOOL PHYIsGetReady(){
	char fifo = PINB&BM(PHY_FIFO);
	char sfd = PINE&BM(PHY_SFD);
	return (fifo!=0 && sfd == 0);
}

BOOL  PHYIsIdle(void){
	char sfd = PINE&BM(PHY_SFD);
	return sfd == 0;
}            

BOOL PHYIsRxBusy(void){
	char sfd = PINE&BM(PHY_SFD);
	return sfd != 0;
}    

