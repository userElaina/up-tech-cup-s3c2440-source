#include "generic.h"

#define FOSC 7372800
#define BAUDRATE                115200
#define UART_WaitForComplete()  while(!(UCSRA&(1<<UDRE)))

unsigned char SendBuffer[7];

void USART_Init(void)
{
	UCSRB = 0x00;//disable while setting baud rate
	UCSRA = 0x00;
	//UCSRC = 0x86;
 	UCSRC = BIT(URSEL) | 0x06;
	//UBRR = (unsigned int)( FOSC/(16*BAUDRATE) - 1 );
	// 设置波特率
	UBRRL = 0x03;//set baud rate lo
 	UBRRH = 0x00;//set baud rate hi
 	UCSRB = 0x18;
}

void UARTSendaByte(unsigned char cTransmitValue)
{
	UART_WaitForComplete();
	UDR = cTransmitValue;
}

void UARTSendAMessage(unsigned char *SendBuffer, unsigned int num)
{
 	unsigned int i;
	SendBuffer[5] = 0x00;
	UARTSendaByte(SendBuffer[0]);
	UARTSendaByte(SendBuffer[1]);
	for( i = 2; i < num-2; i++ )
	{
	 	UARTSendaByte(SendBuffer[i]);
		SendBuffer[num-2]^=SendBuffer[i];
	}
	UARTSendaByte(SendBuffer[num-2]);
	UARTSendaByte(SendBuffer[num-1]);
}


unsigned char UARTRcvaByte(void)
{
   // 等待接收数据
   while ( !(UCSRA & (1<<RXC)) );
   // 从缓冲器中获取并返回数据
   return UDR;
}

unsigned char WaitForCMD(void)
{
 	unsigned char rcvByte = 0x00;
	unsigned int k = 0;
	unsigned CMD = 0x00;
	/*
	//显示0xAA表示等待命令状态
	SPI_MasterTransmitaByte(0x88);//显示A
	SPI_MasterTransmitaByte(0x88);//显示A
		
	PORTB&=0xFB;
	PORTB|=0x04;
	*/
	//等待帧头
	while(1)
	{
		rcvByte = UARTRcvaByte();
		{
		 	if( rcvByte == 0xAA )
			{
			 	rcvByte = UARTRcvaByte();
				{
				 	if( rcvByte == 0xFF )
					{
						/*SPI_MasterTransmitaByte(0x86);//显示E
						SPI_MasterTransmitaByte(0x86);//显示E
						PORTB&=0xFB;
						PORTB|=0x04;*/
						break;
					}
				}
			}
		}
	}
	
	//当前有效命令范围0x10~0x20
	rcvByte = UARTRcvaByte();
	CMD = rcvByte;
	
	//无效命令
	if(( CMD < 0x10 )||( CMD > 0x20 ))
	{
	 	//显示0xbb表示命令无效状态
		SPI_MasterTransmitaByte(0x82);//显示b
		SPI_MasterTransmitaByte(0x82);//显示b
		PORTB&=0xFB;
		PORTB|=0x04;
		
		SendBuffer[0] = 0xBB;
		SendBuffer[1] = 0xFF;
		SendBuffer[2] = 0x00;
		SendBuffer[3] = 0x00;
		SendBuffer[4] = 0x00;
		SendBuffer[6] = 0xFF;
		UARTSendAMessage(SendBuffer,7);
		
		return (0);
	}
	else
	{
	 	//检测校验码
		rcvByte = UARTRcvaByte();
		if( (CMD + rcvByte) != 0xFF )
		{
		 	//校验码不匹配，显示0xcc表示传输出错
			SPI_MasterTransmitaByte(0xc6);//显示c
			SPI_MasterTransmitaByte(0xc6);//显示c
			PORTB&=0xFB;
			PORTB|=0x04;
			
			SendBuffer[0] = 0xBB;
			SendBuffer[1] = 0xFF;
			SendBuffer[2] = 0x00;
			SendBuffer[3] = 0x00;
			SendBuffer[4] = 0x00;
			SendBuffer[6] = 0xFF;
			UARTSendAMessage(SendBuffer,7);
			
			return (0);	
		}
		else
		{
		 	rcvByte = UARTRcvaByte();
			if( rcvByte != 0xFF )
			{
			 	//显示0xdd表明帧结束标志错误
				SPI_MasterTransmitaByte(0xa1);//显示d
				SPI_MasterTransmitaByte(0xa1);//显示d
				PORTB&=0xFB;
				PORTB|=0x04;
				
				SendBuffer[0] = 0xBB;
				SendBuffer[1] = 0xFF;
				SendBuffer[2] = 0x00;
				SendBuffer[3] = 0x00;
				SendBuffer[4] = 0x00;
				SendBuffer[6] = 0xFF;
				UARTSendAMessage(SendBuffer,7);
				
				return (0);	
			}
			else
			{
			 	return (CMD);
			}
		}
	}
}
