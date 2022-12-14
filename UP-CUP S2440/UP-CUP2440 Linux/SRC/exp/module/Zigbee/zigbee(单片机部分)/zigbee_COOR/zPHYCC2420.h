#if !defined(_ZPHYCC2420_H_)
#define _ZPHYCC2420_H_

#define CC2420SetFirstChannel()     PHYSetChannel(11)

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
BYTE CC2420GetStatus(void);

#define PHYCC2420IsTxActive()       (CC2420GetStatus() & 0x08)

// CC2420 specific command set
#define STROBE_SNOP     (0x00)
#define STROBE_SXOSCON  (0x01)
#define STROBE_STXCAL   (0x02)
#define STROBE_SRXON    (0x03)
#define STROBE_STXON    (0x04)
#define STROBE_STXONCCA (0x05)
#define STROBE_SRFOFF   (0x06)
#define STROBE_SXOSCOFF (0x07)
#define STROBE_SFLUSHRX (0x08)
#define STROBE_SFLUSHTX (0x09)
#define STROBE_SACK     (0x0a)
#define STROBE_SACKPEND (0x0b)
#define STROBE_SRXDEC   (0x0c)
#define STROBE_STXENC   (0x0d)
#define STROBE_SAES     (0x0e)

// CC2420 specific registers
#define REG_MAIN        (0x10)
#define REG_MDMCTRL0    (0x11)
#define REG_MDMCTRL1    (0x12)
#define REG_RSSI        (0x13)
#define REG_SYNCWORD    (0x14)
#define REG_TXCTRL      (0x15)
#define REG_RXCTRL0     (0x16)
#define REG_RXCTRL1     (0x17)
#define REG_FSCTRL      (0x18)
#define REG_SECCTRL0    (0x19)
#define REG_SECCTRL1    (0x1a)
#define REG_BATTMON     (0x1b)
#define REG_IOCFG0      (0x1c)
#define REG_IOCFG1      (0x1d)
#define REG_MANFIDL     (0x1e)
#define REG_MANFIDH     (0x1f)
#define REG_FSMTC       (0x20)
#define REG_MANAND      (0x21)
#define REG_MANOR       (0x22)
#define REG_AGCCTRL     (0x23)
#define REG_AGCTST0     (0x24)
#define REG_AGCTST1     (0x25)
#define REG_AGCTST2     (0x26)
#define REG_FSTST0      (0x27)
#define REG_FSTST1      (0x28)
#define REG_FSTST2      (0x29)
#define REG_FSTST3      (0x2a)
#define REG_RXBPFTST    (0x2b)
#define REG_FSMSTATE    (0x2c)
#define REG_ADCTST      (0x2d)
#define REG_DACTST      (0x2e)
#define REG_TOPTST      (0x2f)
#define REG_TXFIFO      (0x3e)
#define REG_RXFIFO      (0x3f)

#define CMD_WRITE       (0)
#define CMD_READ        (0x40)

#define CMD_RAM_RW      (0x00)
#define CMD_RAM_R       (0x20)

// RAM registers are accessed using two bytes - 7-bit address and 2-bit bank address.
// 0x80 sets MSb to indicate RAM access.
#define RAM_ADDR_LSB(a) (((a) & 0xff) | 0x80)
#define RAM_ADDR_BANK(a) (((a) >> 1) & 0xc0)

#define RAM_TXFIFO      RAM_ADDR_LSB(0x000)
#define RAM_TXFIFO_BANK RAM_ADDR_BANK(0x000)

#define RAM_RXFIFO      RAM_ADDR_LSB(0x080)
#define RAM_RXFIFO_BANK RAM_ADDR_BANK(0x080)

#define RAM_KEY0        ((0x100 & 0xff) | 0x80)
#define RAM_KEY0_BANK   (0x100 >> 1)

#define RAM_RXNONCE     ((0x110 & 0xff) | 0x80)
#define RAM_RXNONCE_BANK (0x110 >> 1)

#define RAM_SABUF       RAM_ADDR_LSB(0x120)     // to 0x12F
#define RAM_SABUF_BANK  RAM_ADDR_BANK(0x120)

#define RAM_KEY1        RAM_ADDR_LSB(0x130)     // to 0x13f
#define RAM_KEY1_BANK   RAM_ADDR_BANK(0x130)

#define RAM_TXNONCE     RAM_ADDR_LSB(0x140)     // to 0x14f
#define RAM_TXNONCE_BANK RAM_ADDR_BANK(0x140)

#define RAM_CBCSTATE    RAM_ADDR_LSB(0x150)     // to 0x15f
#define RAM_CBCSTATE_BANK RAM_ADDR_BANK(0x150)

#define RAM_IEEEADR     RAM_ADDR_LSB(0x160)     // to 0x167
#define RAM_IEEEADR_BANK RAM_ADDR_BANK(0x160)

#define RAM_PANID       RAM_ADDR_LSB(0x168)     // to 0x169
#define RAM_PANID_BANK  RAM_ADDR_BANK(0x168)


#define RAM_SHORTADR    RAM_ADDR_LSB(0x16a)     // to 0x16b
#define RAM_SHORTADR_BANK RAM_ADDR_BANK(0x16a)


#endif

