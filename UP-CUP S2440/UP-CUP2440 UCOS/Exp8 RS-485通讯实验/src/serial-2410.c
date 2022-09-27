/***************************************************************************\
	Copyright (c) 2004-2007 threewater@up-tech.com, All rights reserved.
	by threewter	2004.4.26
\***************************************************************************/
	

/***************************************************************************\
    #说明: s3c2410串口驱动程序for uCOS-II
	----------------------------------  Bug  --------------------------------------

	----------------------------------  TODO list  --------------------------------------

	----------------------------------修正--------------------------------------
	2004-4-26	创建

\***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "../inc/reg2410.h"
#include "../inc/serial.h"
//#include "../inc/io.h"
#include "/uhal/isr.h"
#include "inc/bitfield.h"

#define UTRSTAT_TX_EMPTY	(1 << 2)
#define UTRSTAT_RX_READY	(1 << 0)
#define UART_ERR_MASK		0xF 

#define ULCON_IR		(1 << 6)	/* use Infra-Red mode */

#define fULCON_PAR		Fld(3,3)	/* what parity mode? */
#define ULCON_PAR		FMsk(fULCON_PAR)
#define ULCON_PAR_NONE		FInsrt(0x0, fULCON_PAR) /* No Parity */
#define ULCON_PAR_ODD		FInsrt(0x4, fULCON_PAR) /* Odd Parity */
#define ULCON_PAR_EVEN		FInsrt(0x5, fULCON_PAR) /* Even Parity */
#define ULCON_PAR_1		FInsrt(0x6, fULCON_PAR) /* Parity force/checked as 1 */
#define ULCON_PAR_0		FInsrt(0x7, fULCON_PAR) /* Parity force/checked as 0 */

#define ULCON_STOP		(1 << 2)	/* The number of stop bits */
#define ULCON_ONE_STOP		(0 << 2)	/* 1 stop bit */
#define ULCON_TWO_STOP		(1 << 2)	/* 2 stop bit */

#define fULCON_WL		Fld(2, 0)	/* word length */
#define ULCON_WL		FMsk(fULCON_WL)
#define ULCON_WL5		FInsrt(0x0, fULCON_WL)	/* 5 bits */
#define ULCON_WL6		FInsrt(0x1, fULCON_WL)	/* 6 bits */
#define ULCON_WL7		FInsrt(0x2, fULCON_WL)	/* 7 bits */
#define ULCON_WL8		FInsrt(0x3, fULCON_WL)	/* 8 bits */

#define ULCON_CFGMASK		(ULCON_IR | ULCON_PAR | ULCON_WL)

#define UCON_CLK_SEL		(1 << 10)	/* select clock for UART */
#define UCON_CLK_PCLK		(0 << 10)	/* PCLK for UART baud rate */
#define UCON_CLK_UCLK		(1 << 10)	/* UCLK for UART baud rate */
#define UCON_TX_INT_TYPE	(1 << 9)	/* TX Interrupt request type */
#define UCON_TX_INT_PLS		(0 << 9)	/* Pulse */
#define UCON_TX_INT_LVL		(1 << 9)	/* Level */

#define UCON_RX_INT_TYPE	(1 << 8)	/* RX Interrupt request type */
#define UCON_RX_INT_PLS		(0 << 8)	/* Pulse */
#define UCON_RX_INT_LVL		(1 << 8)	/* Level */

#define UCON_RX_TIMEOUT		(1 << 7)	/* RX timeout enable */
#define UCON_RX_ERR_INT		(1 << 6)	/* RX error status interrupt enable */
#define UCON_LOOPBACK		(1 << 5)	/* to enter the loop-back mode */
#define UCON_BRK_SIG		(1 << 4)	/* to send a break during 1 frame time */

#define fUCON_TX	Fld(2,2)		/* function to write Tx data
						   to the UART Tx buffer */
#define UCON_TX		FMsk(fUCON_TX)
#define UCON_TX_DIS	FInsrt(0x0, fUCON_TX)	/* Disable */
#define UCON_TX_INT	FInsrt(0x1, fUCON_TX)	/* Interrupt or polling */
#define UCON_TX_DMA0	FInsrt(0x2, fUCON_TX)	/* DMA0 request */
#define UCON_TX_DMA1	FInsrt(0x3, fUCON_TX)	/* DMA1 request */

#define fUCON_RX	Fld(2,0)		/* function to read data
						   from UART Rx buffer */
#define UCON_RX		FMsk(fUCON_RX)
#define UCON_RX_DIS	FInsrt(0x0, fUCON_RX)	/* Disable */
#define UCON_RX_INT	FInsrt(0x1, fUCON_RX)	/* Interrupt or polling */
#define UCON_RX_DMA0	FInsrt(0x2, fUCON_RX)	/* DMA0 request */
#define UCON_RX_DMA1	FInsrt(0x3, fUCON_RX)	/* DMA1 request */

#define fUFCON_TX_TR	Fld(2,6)	/* trigger level of transmit FIFO */
#define UFCON_TX_TR	FMsk(fUFCON_TX_TR)
#define UFCON_TX_TR0	FInsrt(0x0, fUFCON_TX_TR)	/* Empty */
#define UFCON_TX_TR4	FInsrt(0x1, fUFCON_TX_TR)	/* 4-byte */
#define UFCON_TX_TR8	FInsrt(0x2, fUFCON_TX_TR)	/* 8-byte */
#define UFCON_TX_TR12	FInsrt(0x3, fUFCON_TX_TR)	/* 12-byte */

#define fUFCON_RX_TR	Fld(2,4)	/* trigger level of receive FIFO */
#define UFCON_RX_TR	FMsk(fUFCON_RX_TR)
#define UFCON_RX_TR4	FInsrt(0x0, fUFCON_RX_TR)	/* 4-byte */
#define UFCON_RX_TR8	FInsrt(0x1, fUFCON_RX_TR)	/* 8-byte */
#define UFCON_RX_TR12	FInsrt(0x2, fUFCON_RX_TR)	/* 12-byte */
#define UFCON_RX_TR16	FInsrt(0x3, fUFCON_RX_TR)	/* 16-byte */

#define UFCON_TX_REQ	(1 << 2)	/* auto-cleared after resetting FIFO */
#define UFCON_RX_REQ	(1 << 1)	/* auto-cleared after resetting FIFO */
#define UFCON_FIFO_EN	(1 << 0)	/* FIFO Enable */

#define UMCON_AFC	(1 << 4)	/* Enable Auto Flow Control */
#define UMCON_SEND	(1 << 0)	/* when not-AFC,
						set nRTS 1:'L' 0:'H' level */

#define UTRSTAT_TR_EMP	(1 << 2)	/* 1: Transmitter buffer &
						shifter register empty */
#define UTRSTAT_TX_EMP	(1 << 1)	/* Transmit buffer reg. is empty */
#define UTRSTAT_RX_RDY	(1 << 0)	/* Receive buffer reg. has data */

#define UERSTAT_BRK	(1 << 3)	/* Break receive */
#define UERSTAT_FRAME	(1 << 2)	/* Frame Error */
#define UERSTAT_PARITY	(1 << 1)	/* Parity Error */
#define UERSTAT_OVERRUN	(1 << 0)	/* Overrun Error */

#define UFSTAT_TX_FULL	(1 << 9)	/* Transmit FIFO is full */
#define UFSTAT_RX_FULL	(1 << 8)	/* Receive FIFO is full */

#define fUFSTAT_TX_CNT	Fld(4,4)	/* Number of data in Tx FIFO */
#define UFSTAT_TX_CNT	FMsk(fUFSTAT_TX_CNT)

#define fUFSTAT_RX_CNT	Fld(4,0)	/* Number of data in Rx FIFO */
#define UFSTAT_RX_CNT	FMsk(fUFSTAT_RX_CNT)

#define UMSTAT_dCTS	(1 << 3)	/* see Page 11-16 */
#define UMSTAT_CTS	(1 << 0)	/* CTS(Clear to Send) signal */

#define UTXH_DATA	0x000000FF	/* Transmit data for UARTn */
#define URXH_DATA	0x000000FF	/* Receive data for UARTn */
#define UBRDIVn		0x0000FFFF	/* Baud rate division value (> 0) */
/* UBRDIVn = (int)(PCLK/(bsp * 16)-1 or	UBRDIVn = (int)(UCLK/(bsp * 16)-1 */

#define UART_TXD0	GPIO_MODE_ALT0|GPIO_H2|GPIO_PULLUP_DIS
#define UART_RXD0	GPIO_MODE_ALT0|GPIO_H3|GPIO_PULLUP_DIS
#define UART_TXD1	GPIO_MODE_ALT0|GPIO_H4|GPIO_PULLUP_DIS
#define UART_RXD1	GPIO_MODE_ALT0|GPIO_H5|GPIO_PULLUP_DIS
#define UART_TXD2	GPIO_MODE_ALT0|GPIO_H6|GPIO_PULLUP_DIS
#define UART_RXD2	GPIO_MODE_ALT0|GPIO_H7|GPIO_PULLUP_DIS

typedef struct {
	unsigned int txio;
	unsigned int rxio;
	unsigned int clkcon;

}type_Uart_def;
static const type_Uart_def Uart_def[]={{UART_TXD0, UART_RXD0, CLKCON_UART0}, 
	{UART_TXD1, UART_RXD1, CLKCON_UART1}, {UART_TXD2, UART_RXD2, CLKCON_UART2}};


#define Serial_Isr(n)		(IRQ_RXD##n)	//串口中断号

__inline static int serial_init(int ndev, int baudrate)
{
	int div=s3c2410_get_bus_clk(GET_PCLK);
	div=( (int)(div/16/baudrate) -1 );	//Baud rate divisior register

	//configure io for uart
	set_gpio_ctrl(Uart_def[ndev].txio);
	set_gpio_ctrl(Uart_def[ndev].rxio);

	//setup uart clock
	rCLKCON |= Uart_def[ndev].clkcon;
	bUART(ndev, oUBRDIV) = div;
	bUART(ndev, oUMCON) = 0;

	//reset and configure fifo
	bUART(ndev, oUFCON) = UFCON_TX_REQ | UFCON_RX_REQ;
	bUART(ndev, oUFCON) = UFCON_FIFO_EN | UFCON_TX_TR8 | UFCON_RX_TR4;
//	bUART(ndev, oUFCON) = 0;

	//enable uart
	bUART(ndev, oULCON) = ULCON_WL8;
	bUART(ndev, oUCON) = UCON_CLK_PCLK | UCON_TX_INT_PLS | 
			UCON_RX_INT_PLS | UCON_TX_INT | UCON_RX_INT |UCON_RX_TIMEOUT;
	return 0;
}

__inline static int serial_read(int ndev)
{
	return bUART(ndev, oURXHL);
}

__inline static int serial_write(int ndev, int ch)
{
//	while(!(bUART(ndev, oUTRSTAT) & 0x2));   //Wait until THR is empty.

	while(bUART(ndev, oUFSTAT) & UFSTAT_TX_FULL);	// wait when tx fifo full
	bUART(ndev, oUTXHL) = ch;

	return 0;
}

__inline static int serial_poll(int ndev)
{
	return ((bUART(ndev, oUFSTAT) & UFSTAT_RX_CNT) != 0);
}

__inline static int serial_flush_input(int ndev)
{
	int tmp;
	while(bUART(ndev, oUFSTAT) & UFSTAT_RX_CNT)
		tmp=bUART(ndev, oURXHL);
	
	//reset rx fifo
	bUART(ndev, oUFCON) |= UFCON_RX_REQ;
	return 0;
}

__inline static int serial_flush_output(int ndev)
{
	while(!(bUART(ndev, oUTRSTAT) & UTRSTAT_TR_EMP)); //Wait until tx shifter is empty.

	return 0;
}

#define DefSerialN(n)	static int s3c2410_serial##n##_init(int baud){return serial_init(n,baud);}\
					static int s3c2410_serial##n##_read(void){return serial_read(n);}\
					static int s3c2410_serial##n##_write(int ch){return serial_write(n, ch);}\
					static int s3c2410_serial##n##_poll(void){return serial_poll(n);}\
					static int s3c2410_serial##n##_flush_input(void){return serial_flush_input(n);}\
					static int s3c2410_serial##n##_flush_output(void){return serial_flush_output(n);}

/* export serial driver */
#define ExportSerialN(n)	serial_driver_t s3c2410_serial##n##_driver = {	\
							s3c2410_serial##n##_init,	\
							s3c2410_serial##n##_read,\
							s3c2410_serial##n##_write,\
							s3c2410_serial##n##_poll,\
							s3c2410_serial##n##_flush_input,\
							s3c2410_serial##n##_flush_output,\
							Serial_Isr(n)}
DefSerialN(0)
DefSerialN(1)
DefSerialN(2)


ExportSerialN(0);
ExportSerialN(1);
ExportSerialN(2);

