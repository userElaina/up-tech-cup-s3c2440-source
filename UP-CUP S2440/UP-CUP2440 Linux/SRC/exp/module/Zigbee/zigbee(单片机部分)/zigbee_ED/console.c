#include "Console.h"

//定义串口的波特率，停止位，校验位
#define baudRate	UART_BAUDRATE_19K2
#define stop_bit	UART_OPT_ONE_STOP_BIT
#define parity		UART_OPT_NO_PARITY
#define bits		UART_OPT_8_BITS_PER_CHAR
//add by lyj
//#define UART_DEBUG
/*
*************************************************************************************************************
*          /myzigbeenewA/uart0/uart0.c
* ATmega128  UART driver
* @auther lyj
* @date 20071101
* @add new API  20071030
*************************************************************************************************************
*/
/////////////////////////////////////////////////
//                uart0驱动                    //
/////////////////////////////////////////////////

//静态全局变量
static unsigned char UART0_RxBuf[UART0_RX_BUFFER_SIZE];
static volatile unsigned char UART0_RxHead;
static volatile unsigned char UART0_RxTail;
static unsigned char UART0_TxBuf[UART0_TX_BUFFER_SIZE];
static volatile unsigned char UART0_TxHead;
static volatile unsigned char UART0_TxTail;
static volatile unsigned char UART0_Rx_flag = 0;
static volatile unsigned char command_valid = 0;
static volatile unsigned char data_FCS = 0;

//串口初始化函数
void UART0_InitUART( unsigned long baudrate )
{
    unsigned char x;

    UBRR0H = (unsigned char)(CPU_CLK_FREQ/(16*baudrate) - 1) >>8;         //设置波特率
    UBRR0L = (unsigned char)(CPU_CLK_FREQ/(16*baudrate) - 1) & 0xFF;

    UCSR0B = ( (1<<RXCIE0) | (1<<RXEN0) | (1<<TXEN0) );  //允许串口接收、发送和允许接收中断
    UCSR0C = ( (1<<UCSZ01) | (1<<UCSZ00) );  //1位停止位,8位数据位
    x = 0;              //初始化数据缓存
    UART0_RxTail = x;
    UART0_RxHead = x;
    UART0_TxTail = x;
    UART0_TxHead = x;
}

//串口接收中断处理函数
SIGNAL(USART0_RX_vect)//18
{
    unsigned char data;
    unsigned char tmphead;
    data = UDR0;     //读接收数据
    tmphead = ( UART0_RxHead + 1 ) & UART0_RX_BUFFER_MASK;    //计算缓存索引
    UART0_RxHead = tmphead;         //保存新的缓存索引
    if ( tmphead == UART0_RxTail )
    {
        return ;//这里加入缓存溢出处理
    }
    UART0_RxBuf[tmphead] = data;   //把接收数据保存到接收缓存
	//置位
	UART0_Rx_flag = 1;
}

//串行发送中断处理函数
//发送中断是要修改的
SIGNAL(USART0_UDRE_vect)//19
{
    unsigned char tmptail;

    if ( UART0_TxHead != UART0_TxTail )  //检查是否发送缓存里的所以数据都已经发送完毕
    {
        tmptail = ( UART0_TxTail + 1 ) & UART0_TX_BUFFER_MASK;       //计算缓存索引
        UART0_TxTail = tmptail;     //保存新的缓存索引
        UDR0 = UART0_TxBuf[tmptail];     //开始发送
    }
    else
    {
        UCSR0B &= ~(1<<UDRIE0);       //关UDRE中断 
    }
}

//从接收缓存里接收一个字节
unsigned char UART0_ReceiveByte( void )
{
    unsigned char tmptail;

    while ( UART0_RxHead == UART0_RxTail ); //等待接收数据
        
    tmptail = ( UART0_RxTail + 1 ) & UART0_RX_BUFFER_MASK;    //计算缓存索引
    UART0_RxTail = tmptail;       //保存新的缓存索引
    return UART0_RxBuf[tmptail]; 
}

//将一个字节放入发送缓存
void UART0_TransmitByte( unsigned char data )
{
//#ifdef DEBUG_UPTECH

    unsigned char tmphead;
    tmphead = ( UART0_TxHead + 1 ) & UART0_TX_BUFFER_MASK;   //计算缓存索引

    while ( tmphead == UART0_TxTail );       //等待有空地缓存空间
        
    UART0_TxBuf[tmphead] = data;     //保存数据到缓存
    UART0_TxHead = tmphead;     //保存新的缓存索引
    UCSR0B |= (1<<UDRIE0); //开 UDRE 中断
//#endif
}
//判断接收缓存是否为空
unsigned char UART0_DataInReceiveBuffer( void )
{
    return ( UART0_RxHead != UART0_RxTail ); //返回0表示接收缓存已空
}
//add end


//串口初始化
void ConsoleInit(void)
{
	UART0_InitUART(115200);
}
//************************************************

void ConsolePut(BYTE c)
{
#if defined(I_AM_END_DEVICE)
	UART0_TransmitByte(c);
#endif
}
//更改是有代价的，一定要把这个问题搞明白
void ConsolePutString(BYTE *s)
{
#ifdef UART_DEBUG
	BYTE c;
    while( (c = *s++) ){
        UART0_TransmitByte(c);
		if(c == '\n')
			UART0_TransmitByte('\r');
    }

#endif
}
//目前还不知道为什么,把这个宏定义加上下面这句就无法编译
#ifdef I_AM_COORDINATOR
	void CoordPut(BYTE c)
	{
		UART0_TransmitByte(c);

	}
	void CoordPutString(BYTE *s)
	{
    BYTE c;
    while( (c = *s++) )
        CoordPut(c);
	}
#endif

void ConsolePutFlashString(PGM_P string_P){
	char *string = malloc(strlen_P(string_P)+1);
	if (string == NULL){
		ConsolePutString("malloc new char[] failed\n");
		return;
	}
	strcpy_P(string,string_P);
	ConsolePutString(string);
	free(string);
}




//方便测试添加打印数据的函数
void ConsolePutInitData(int highval,int lowval)
{
#ifdef UART_DEBUG
  char i;
  BYTE dig;
  long divider;
  int wordvalue;
  wordvalue=highval*256+lowval;
  divider=10000;
  
	for(i=5; divider>wordvalue && i>1 ;i--)
	{
		divider/=10;
		UART0_TransmitByte(' ');
	}
  for(;i>0;i--)
  {

    dig = (wordvalue/divider);
	wordvalue = (wordvalue%divider);
	UART0_TransmitByte(dig+0x30);
		
    divider/=10;
  }   
  UART0_TransmitByte('\r');	
  UART0_TransmitByte('\n');	
#endif
}




//************************************************
BYTE ConsoleGet(void)
{		
   	return UART0_ReceiveByte();
}

BYTE ConsoleGetString(char *buffer, BYTE bufferLen)
{
    BYTE v;
    BYTE count;
    count = 0;

    do
    {
        if ( bufferLen-- == 0 )
            break;
//		UDR1 = 0;
        
		while( !UART0_DataInReceiveBuffer() );
        v = UART0_ReceiveByte();
        if ( v == '\r' || v == '\n' )
            break;
        count++;
        *buffer++ = v;
        *buffer = '\0';
    } while(1);
    return count;
}



