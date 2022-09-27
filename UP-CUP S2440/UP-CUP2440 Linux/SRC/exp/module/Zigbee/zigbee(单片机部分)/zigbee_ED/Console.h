#ifndef  _CONSOLE_H_
#define _CONSOLE_H_

#include "generic.h"
// Baud rate codes for use with the INIT_UART1 macro
#define UART_BAUDRATE_2K4           416
#define UART_BAUDRATE_4K8           207
#define UART_BAUDRATE_9K6           103
#define UART_BAUDRATE_14K4          68
#define UART_BAUDRATE_19K2          51
#define UART_BAUDRATE_28K8          34
#define UART_BAUDRATE_38K4          25
#define UART_BAUDRATE_57K6          16
#define UART_BAUDRATE_76K8          12
#define UART_BAUDRATE_115K2         8
#define UART_BAUDRATE_230K4         3
#define UART_BAUDRATE_250K          3
#define UART_BAUDRATE_500K          1   
#define UART_BAUDRATE_1M            0

// Options for use with the INIT_UART1 macro
#define UART_OPT_ONE_STOP_BIT       0
#define UART_OPT_TWO_STOP_BITS      0x08
#define UART_OPT_NO_PARITY          0
#define UART_OPT_EVEN_PARITY        0x20
#define UART_OPT_ODD_PARITY         0x30
#define UART_OPT_5_BITS_PER_CHAR    0
#define UART_OPT_6_BITS_PER_CHAR    0x02
#define UART_OPT_7_BITS_PER_CHAR    0x04
#define UART_OPT_8_BITS_PER_CHAR    0x06
#define UART_OPT_9_BITS_PER_CHAR    0x0406

//**********************************************
// Enable/disable UART
#define ENABLE_UART1()              (UCSR1B |= (BM(RXEN1) | BM(TXEN1))) 
#define DISABLE_UART1()             (UCSR1B &= ~(BM(RXEN1) | BM(TXEN1)))
//**********************************************

#define INIT_UART1(baudRate_t,stop_bit_t,parity_t,bits_t) \
	do {\
		UBRR1H = ((baudRate_t) >> 8) ;\
		UBRR1L = (baudRate_t);\
		UCSR1C = (stop_bit_t | parity_t | bits_t);\
		if (bits > 0xFF) {\
            UCSR1B |= 0x04;\
        } else {\
            UCSR1B &= ~0x04;\
        }\
		UCSR1A |= BM(U2X1);\
    } while (0)

//**********************************************

/**********************************************
*
* add by lyj_uptech@126.com
* 采用中断加缓冲的方式
*
*
**********************************************/
/////////////////////////////////////////////////
//            MCU时钟频率定义                  //
/////////////////////////////////////////////////
#define  CPU_CLK_FREQ 7372800L     //系统时钟7.3728M
/////////////////////////////////////////////////
//              uart0驱动定义                  //
/////////////////////////////////////////////////

//定义串行接收和发送中断向量
//UART0_RX_interrupt 为RX结束中断
//UART0_TX_interrupt 为USART数据寄存器空中断
//#pragma interrupt_handler UART0_RX_interrupt:19 UART0_TX_interrupt:20

//初始化后默认为关中断
//允许串口0接收中断
#define Enable_UART0_RX()  UCSR0B |= (1<<RXCIE0)  
//禁止串口0接收中断        
#define Disable_UART0_RX() UCSR0B &= ~(1<<RXCIE0)

//串口数据缓存定义
#define UART0_RX_BUFFER_SIZE 256 /* 1,2,4,8,16,32,64,128 or 256 bytes */
#define UART0_RX_BUFFER_MASK ( UART0_RX_BUFFER_SIZE - 1 )
#define UART0_TX_BUFFER_SIZE 128 /* 1,2,4,8,16,32,64,128 or 256 bytes */
#define UART0_TX_BUFFER_MASK ( UART0_TX_BUFFER_SIZE - 1 )

#if ( UART0_RX_BUFFER_SIZE & UART0_RX_BUFFER_MASK )
#error RX0 buffer size is not a power of 2
#endif

#if ( UART0_TX_BUFFER_SIZE & UART0_TX_BUFFER_MASK )
#error TX0 buffer size is not a power of 2
#endif
//add end
void ConsoleInit(void);

#define ConsoleIsPutReady()     ((UCSR1A & BM(UDRE1)))

void ConsolePut(BYTE c);

void ConsolePutString(BYTE *s);

#ifdef I_AM_COORDINATOR
	void CoordPut(BYTE C);

	void CoordPutString(BYTE *s);
#endif

void ConsolePutFlashString(PGM_P src);
void ConsolePutInitData(int highval,int lowval);

#define ConsoleIsGetReady()     ((UCSR1A & BM(RXC1)))

BYTE ConsoleGet(void);

BYTE ConsoleGetString(char *buffer, BYTE bufferLen);
void UART0_TransmitByte(BYTE ch);

#endif 








