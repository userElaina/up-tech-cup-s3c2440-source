#ifndef _ZPHY_H_
#define _ZPHY_H_

#include "zigbee.h"
#include "zPHYCC2420.h"

// Following macros are specific to CC2420 and are used by MAC
// private functions only.

    //CSn       (Output - to select CC2420 SPI slave)
    //SFD       (Input - Generates interrupt on falling edge)
    //FIFOP     (Input - Used to detect overflow)
    //SCK      (Output - SPI Clock to CC2420)
    //SO        (Input - SPI data from CC2420)
    //SI        (Output - SPI data to CC2420)
    //FIFO      (Input)
    //CCA       (Input - Not used in this version of stack)
    //VREG_EN   (Output - to enable CC2420 voltage regulator)
    //RESETn    (Output - to reset CC2420)
    
//************************
/**
//PORTE
#define PHY_FIFOP       6	//I
#define PHY_SFD         7	//I
//PORTB
#define PHY_SS			0	//O

#define	PHY_SCK			1	//O
#define	PHY_MOSI		2	//O
#define	PHY_MOSO		3	//I
#define PHY_CSn         4	//O
#define PHY_FIFO		5	//I
#define PHY_CCA	        6	//I
#define PHY_VREG_EN     7	//O
//PORTG	    
#define PHY_RESETn      3	//O				
*/
/**********************************************/
//实际物理定义
//PHY_SS 没有定义，估计与cc2420没有什么必然的联系
//PORTA
#define PHY_VREG_EN 	5   //O
#define PHY_RESETn		6   //O
//PORTB
#define PHY_CSn			0	//O
#define	PHY_SCK			1	//O
#define	PHY_MOSI		2	//O
#define	PHY_MOSO		3	//I
#define PHY_FIFO		7	//I
//PORTD	    
//#define PHY_SFD		    3	//I		
#define PHY_SFD_IN		    3	//I	
#define PHY_CCA	        6	//I
//PORTE
#define PHY_FIFOP       6	//I
#define PHY_SFD         7	//I



// This is how PHY chip is selected
#define PHYSelect()     (PORTB &= ~BM(PHY_CSn)) 
#define PHYDeselect()   (PORTB |= BM(PHY_CSn))

#define PHYBegin()      PHYSelect()			//cc2420 CSn为低
#define PHYEnd()        PHYDeselect()		//cc2420 CSn为高

//复位，改变引脚电平
#define PHY_RESET(a)    {\
						if (a==1)\
						PORTA |= BM(PHY_RESETn) ;\
						else	\
						PORTA &= ~BM(PHY_RESETn);\
						}

#define PHYEnable()     { \
                          WORD i = 0xE000;\
                          PORTA |= BM(PHY_VREG_EN);\
                          while(++i);\
                         }
#define PHYDisable()    PORTA &= ~BM(PHY_VREG_EN)


// CC2420 specific command bits.
#define PHY_WRITE           CMD_RAM_RW
#define PHY_READ            CMD_RAM_R

#define PHYSelectIEEEAddrWrite()         \
    PHYPut(RAM_IEEEADR);                 \
    PHYPut((RAM_IEEEADR_BANK) | CMD_RAM_RW);

#define PHYSelectPANAddrWrite()          \
    PHYPut(RAM_PANID);                   \
    PHYPut((RAM_PANID_BANK) | CMD_RAM_RW);

#define PHYSelectShortAddrWrite()        \
    PHYPut(RAM_SHORTADR);                \
    PHYPut((RAM_SHORTADR_BANK) | CMD_RAM_RW);

#define PHYTx()             PHYPut(STROBE_STXONCCA)

#define PHYIsTxActive()     PHYCC2420IsTxActive()

BOOL PHYIsRxBusy(void);      

#define PHYGetStatus()      CC2420GetStatus()

#define PHYSelectTxFIFO()   PHYPut(REG_TXFIFO)

#define PHYSelectTxRAM()                    \
    PHYPut(RAM_TXFIFO);                     \
    PHYPut(RAM_TXFIFO_BANK | CMD_RAM_RW);

#define PHYBeginTxFIFOAccess()      { PHYBegin(); PHYSelectTxFIFO(); }
#define PHYEndTxFIFOAccess()        PHYEnd()
#define PHYPutTxData(v)             PHYPut(v)
#define PHYSelectRxFIFO()   PHYPut(REG_RXFIFO | CMD_READ)
#define PHYRxEnable()       PHYPut(STROBE_SRXON)

typedef enum _ACK_TYPE
{
ACK_NO_DATA = STROBE_SACK,
ACK_DATA_PENDING = STROBE_SACKPEND
} ACK_TYPE;

#define PHYSendAck(ack)        PHYPut(ack)

typedef enum PHY_TRX_STATE
{
    PHY_TRX_RX_ON,
    PHY_TRX_OFF,
    PHY_TRX_FORCE_OFF,
    PHY_TRX_TX_ON
}PHY_TRX_STATE;

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
 BOOL PHYInit(void);

/*********************************************************************
 * Function:        BOOL PHYIsGetReady(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          TRUE if PHY receive buffer contains any data
 *                  FALSE, otherwise
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            None
 ********************************************************************/
BOOL PHYIsGetReady(void);     

/*********************************************************************
 * Function:        BYTE PHYGet(void)
 *
 * PreCondition:    PHYIsGetReady() = TRUE
 *
 * Input:           None
 *
 * Output:          Byte from PHY receive buffer
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            None
 ********************************************************************/
#define PHYGet()       SPIGet()

/*********************************************************************
 * Function:        void PHYGetArray(BYTE *buffer, BYTE len)
 *
 * PreCondition:    PHYIsGetReady() = TRUE, and PHY contains at least
 *                  len bytes of data.
 *
 * Input:           buffer  - Buffer to contain the received data
 *                  len     - Number of bytes to get
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Fetches len number of bytes from PHY receive
 *                  buffer. It does not check for underflow conditions
 *                  Caller must make sure that there are at least
 *                  len number of bytes before calling this function.
 *
 * Note:            None
 ********************************************************************/
#define PHYGetArray(b, len)       SPIGetArray(b, len)

/*********************************************************************
 * Function:        void PHYPut(BYTE v)
 *
 * PreCondition:    PHYBegin() is already called on CC2420
 *                  PHYInit() has been called on ZMD44101
 *
 * Input:           v   - Byte to put
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            None
 ********************************************************************/
#define PHYPut(v)           SPIPut(v)

/*********************************************************************
 * Function:        void PHYPutArray(BYTE* buffer, BYTE len)
 *
 * PreCondition:    PHYBegin() is already called and there is at least
 *                  len byte of space left in transmit buffer
 *
 * Input:           buffer  - Buffer that needs to be put
 *                  len     - Number of bytes that are to be put
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Loads given buffer into PHY transmit buffer.
 *
 * Note ZMD:        An arbitrary length put is not supported by the
 *                  ZMD44101.  You must give the length at the
 *                  beginning of the SPI transaction.  Use
 *                  PHYWriteArray() instead.
 ********************************************************************/
#define PHYPutArray(b, len)           SPIPutArray(b, len)

/*********************************************************************
 * Function:        BYTE PHYGetEnergy(void)
 *
 * PreCondition:    PHYRxState(ON).
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
BYTE PHYGetED(void);

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
void PHYSetTRXState(PHY_TRX_STATE state);

/*********************************************************************
 * Macro:           void PHYFlushTx(void)
 *
 * PreCondition:    PHYInit() is called
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Removes all data from the Tx FIFO
 *
 * Note ZMD:        TODO: Operation not possible with current
 *                        ZMD44101 CR2 silicon.
 ********************************************************************/
#define PHYFlushTx()        PHYPut(STROBE_SFLUSHTX)

/*********************************************************************
 * Macro:           void PHYFlushRx(void)
 *
 * PreCondition:    PHYInit() is called
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Clears receive FIFO
 *
 * Note:            None
 ********************************************************************/
#define PHYFlushRx()        PHYPut(STROBE_SFLUSHRX); PHYPut(STROBE_SFLUSHRX);

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
void PHYSetTxPower(BYTE val);

/*********************************************************************
 * Function:        BOOL PHYProcessRxOverflow(void)
 *
 * PreCondition:    PHYInit() is called
 *
 * Input:           None
 *
 * Output:          TRUE if a RX overflow was detected and removed
 *                  FALSE if no overflow occured
 *
 * Side Effects:    None
 *
 * Overview:        Checks for receive overflow and clears as per
 *                  RF chip specific procedure
 *
 * Note:            None
 ********************************************************************/
BOOL PHYProcessRxOverflow(void);

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
 * Note:            The channel number must fall within current frequency
 *                  band. (e.g. for 2.4GHz, it must be between 11-26)
 ********************************************************************/
void PHYSetChannel(BYTE channel);

/*********************************************************************
 * Macro:           void PHYSetFirstChannel(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview CC2420: Selects very first channel in current frequency
 *                  band (i.e. channel 11 for 2.4GHz band)
 * Overview ZMD:    Selects very first channel in current frequency
 *                  band (i.e. channel 1 for 900MHz band or 0 for
 *                  868.3MHz band)
 *
 * Note:            This macro is designed to allow application to
 *                  browse through all channels irrespective of
 *                  frequency band in use.
 ********************************************************************/
#define PHYSetFirstChannel()    CC2420SetFirstChannel()

/*********************************************************************
 * Function:        BOOL PHYSetNextChannel(void)
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
BOOL PHYSetNextChannel(void);

BOOL  PHYIsIdle(void);             

/*********************************************************************
 * Macro:           BYTE PHYGetChannel(void)
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
#define PHYGetChannel()         (currentChannel)
    extern BYTE currentChannel;

#endif





