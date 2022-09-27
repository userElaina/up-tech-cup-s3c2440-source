#include "Console.h"

//打开该宏定义，就可以通过串口0看到对应的调试信息
//因为现在要将该模块作为一个OEM模块来使用所有要屏蔽掉调试
//信息
//#define UART_DEBUG


//定义串口的波特率，停止位，校验位
#define baudRate	UART_BAUDRATE_19K2
#define stop_bit	UART_OPT_ONE_STOP_BIT
#define parity		UART_OPT_NO_PARITY
#define bits		UART_OPT_8_BITS_PER_CHAR
//add by lyj
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
//是否接收到数据

static volatile unsigned char command_valid = 0;
//解析命令是否有效

static volatile unsigned char data_FCS = 0;     
//是否校验成功

static volatile unsigned char command=0x00;     
//存储解析候的命令

//从串口接收有效数据缓冲区
unsigned char receive_data[125];//全局变量
unsigned char send_data[125];//全局变量

static volatile int receive_length = 0;         
//接收到的有效数据长度

static volatile int receive_count = 0;          
//记录当前帧接收到的数据

static volatile int receive_a_frame = 0;        
//收接收到了完整的一帧数据

static volatile int i=0;




//串口初始化函数
void UART0_InitUART( unsigned long baudrate )
{
    unsigned char x;

    UBRR0H = (unsigned char)(CPU_CLK_FREQ/(16*baudrate) - 1) >>8;         
	//设置波特率
    UBRR0L = (unsigned char)(CPU_CLK_FREQ/(16*baudrate) - 1) & 0xFF;

    UCSR0B = ( (1<<RXCIE0) | (1<<RXEN0) | (1<<TXEN0) );  
	//允许串口接收、发送和允许接收中断
    UCSR0C = ( (1<<UCSZ01) | (1<<UCSZ00) );  
	//1位停止位,8位数据位
    x = 0;              
	//初始化数据缓存
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
    data = UDR0;     
	//读接收数据
	
    tmphead = ( UART0_RxHead + 1 ) & UART0_RX_BUFFER_MASK;    // 1 & 0xFF
	//计算缓存索引
	
    UART0_RxHead = tmphead;// 1         
	//保存新的缓存索引
	
    if ( tmphead == UART0_RxTail )
    {
		//这里加入缓存溢出处理
		return ;
    }
	UART0_RxBuf[tmphead] = data;   
	//把接收数据保存到接收缓存

	//置位
	UART0_Rx_flag = 1;
	receive_count ++;
	if(receive_count >= 10){
		receive_count = 0;
		receive_a_frame = 1;
	}
}

//串行发送中断处理函数
//发送中断是要修改的
SIGNAL(USART0_UDRE_vect)//19
{
    unsigned char tmptail;
	DISABLE_GLOBAL_INT();
	

	//检查是否发送缓存里的所有数据都已经发送完毕
	if ( UART0_TxHead != UART0_TxTail )  
    {
        tmptail = ( UART0_TxTail + 1 ) & UART0_TX_BUFFER_MASK;       
		//计算缓存索引
        UART0_TxTail = tmptail;     
		//保存新的缓存索引
        UDR0 = UART0_TxBuf[tmptail];     
		//开始发送
    }
    else
    {
        UCSR0B &= ~(1<<UDRIE0);       
		//关UDRE中断 
    }
	ENABLE_GLOBAL_INT();
}

//从接收缓存里接收一个字节
unsigned char UART0_ReceiveByte( void )
{
    unsigned char tmptail;

    while ( UART0_RxHead == UART0_RxTail ); 
	//等待接收数据
        
    tmptail = ( UART0_RxTail + 1 ) & UART0_RX_BUFFER_MASK;    
	//计算缓存索引
	
    UART0_RxTail = tmptail;       
	//保存新的缓存索引
	
    return UART0_RxBuf[tmptail]; 
}

//将一个字节放入发送缓存
void UART0_TransmitByte( unsigned char data )
{
    unsigned char tmphead;
    tmphead = ( UART0_TxHead + 1 ) & UART0_TX_BUFFER_MASK;   
	//计算缓存索引

    while ( tmphead == UART0_TxTail );       
	//等待有空地缓存空间

    UART0_TxBuf[tmphead] = data;     
	//保存数据到缓存

    UART0_TxHead = tmphead;     
	//保存新的缓存索引

    UCSR0B |= (1<<UDRIE0); 
	//开 UDRE 中断
}
//判断接收缓存是否为空
unsigned char UART0_DataInReceiveBuffer( void )
{
    return ( UART0_RxHead != UART0_RxTail ); 
	//返回0表示接收缓存已空
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
//add by lyj _uptech@126.com
#ifdef UART_DEBUG
    BYTE c;
    while( (c = *s++) ){
//		ConsolePut(c);
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

/******************************************
*
*  工具函数
*
*
******************************************/
int UART0_read(unsigned char *buf,int nbytes){
	int i = 0;
	for(i = 0;i < nbytes;i ++){
/**
		if(!UART0_DataInReceiveBuffer()){
			halWait(50)!
			if(!UART0_DataInReceiveBuffer())
				//如果等不到数据就错误返回
				return 0;
		}
*/
		buf[i] = UART0_ReceiveByte();
	}
	return i;
}
int UART0_write(unsigned char *buf,int nbytes){
	int i = 0;
	for(i = 0;i < nbytes;i ++){
		UART0_TransmitByte(buf[i]);
	}
	return i;
}
//刷新接收缓存
void UART0_RX_flash(){
    UART0_RxTail = 0;
    UART0_RxHead = 0;
}
//刷新发送缓存
void UART_TX_flash(){
    UART0_TxTail = 0;
    UART0_TxHead = 0;
}
/***********************************************************
*
*帧头   |   帧长度|命令|目标端点|源端点|目标地址|     命令| 参数 |结尾
* AA  FF           06               00              03                 02                MSB LSB                                       FF
* 0      1           2                 3               4                   5                  6    7                8            9           10
*
*
***********************************************************/
int decode_rec(){
	if(!UART0_Rx_flag)
		//还没有收到数据
		return 0;
	//复位异或校验位
	data_FCS = 0;
	//step 0 读取一帧数据
	//先读取帧头
//	if((receive_data[0] = UART0_ReceiveByte()) != 0xAA)
//		return 0;
//		goto DECODE_FAIL;//数据不足溢出
//防止前面还有数据没有读完的情况
	for(;;){
		//遍历所有的数据缓冲区进行匹配
		if(UART0_DataInReceiveBuffer()){
			if((receive_data[0] = UART0_ReceiveByte()) == 0xAA)
				if((receive_data[1] = UART0_ReceiveByte()) == 0xFF)
					{
						//追加一个判断
						break;
					}
		}else
			goto DECODE_FAIL;//数据不足溢出
	}
	//该等待是为了防止前面有数据添加的
	//但是出现了系统等待的问题
//	while((receive_data[0] = UART0_ReceiveByte()) != 0xAA);

	//帧头正确的话，将剩余数据读出
	for(i = 2;i < 11;i ++ ){//将数据更改到从第三个开始读取
		while(!UART0_DataInReceiveBuffer());
		if(UART0_DataInReceiveBuffer()){
			receive_data[i] = UART0_ReceiveByte(); 
		}else
			goto DECODE_FAIL;
			//数据不足溢出
	}
		
	//step1
	//第一个字节
	if(receive_data[0] != 0xAA)
		goto DECODE_FAIL;
	//第二个字节
	if(receive_data[1] != 0xFF)
		goto DECODE_FAIL;
/*
	//step2
	//读取数据
	if(receive_data[3] != 0x01)
		goto DECODE_FAIL;
	if(receive_data[4] != 0xA2)
		goto DECODE_FAIL;

*/
	//step3
	//数据尾
	//本来是一个数据校验位现在改为0xff
	//保留数据校验的功能
	//方便将来扩展
	if(receive_data[10] != 0xFF)
		goto DECODE_FAIL;
	command_valid = 1;
	command=receive_data[3];
	//刷新接收缓存
	UART0_RX_flash();	
	//清零标志位
	UART0_Rx_flag = 0;
	return receive_length;
DECODE_FAIL:
	command_valid = 0;
	//刷新接收缓存
	UART0_RX_flash();	
	//清零标志位
	UART0_Rx_flag = 0;
	for(i=0;i < 11;i ++)
		receive_data[i] = 0;
	return 0;
}
void test_for(){
	UART0_write(receive_data,11);
}

unsigned char if_receive_a_frame(){
	return receive_a_frame ;
}
void Reset_receive_frame(){
	receive_a_frame = 0;
}
unsigned char if_command_valid(){
	return command_valid;
}
unsigned char get_command(){
	return command;
}
void Reset_command_valid(){
	command_valid = 0;
}
unsigned char get_rx_data(int index){
	return receive_data[index];
}


