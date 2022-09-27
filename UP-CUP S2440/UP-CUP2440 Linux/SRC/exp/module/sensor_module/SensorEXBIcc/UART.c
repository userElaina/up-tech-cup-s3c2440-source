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
	// ���ò�����
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
   // �ȴ���������
   while ( !(UCSRA & (1<<RXC)) );
   // �ӻ������л�ȡ����������
   return UDR;
}

unsigned char WaitForCMD(void)
{
 	unsigned char rcvByte = 0x00;
	unsigned int k = 0;
	unsigned CMD = 0x00;
	/*
	//��ʾ0xAA��ʾ�ȴ�����״̬
	SPI_MasterTransmitaByte(0x88);//��ʾA
	SPI_MasterTransmitaByte(0x88);//��ʾA
		
	PORTB&=0xFB;
	PORTB|=0x04;
	*/
	//�ȴ�֡ͷ
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
						/*SPI_MasterTransmitaByte(0x86);//��ʾE
						SPI_MasterTransmitaByte(0x86);//��ʾE
						PORTB&=0xFB;
						PORTB|=0x04;*/
						break;
					}
				}
			}
		}
	}
	
	//��ǰ��Ч���Χ0x10~0x20
	rcvByte = UARTRcvaByte();
	CMD = rcvByte;
	
	//��Ч����
	if(( CMD < 0x10 )||( CMD > 0x20 ))
	{
	 	//��ʾ0xbb��ʾ������Ч״̬
		SPI_MasterTransmitaByte(0x82);//��ʾb
		SPI_MasterTransmitaByte(0x82);//��ʾb
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
	 	//���У����
		rcvByte = UARTRcvaByte();
		if( (CMD + rcvByte) != 0xFF )
		{
		 	//У���벻ƥ�䣬��ʾ0xcc��ʾ�������
			SPI_MasterTransmitaByte(0xc6);//��ʾc
			SPI_MasterTransmitaByte(0xc6);//��ʾc
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
			 	//��ʾ0xdd����֡������־����
				SPI_MasterTransmitaByte(0xa1);//��ʾd
				SPI_MasterTransmitaByte(0xa1);//��ʾd
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
