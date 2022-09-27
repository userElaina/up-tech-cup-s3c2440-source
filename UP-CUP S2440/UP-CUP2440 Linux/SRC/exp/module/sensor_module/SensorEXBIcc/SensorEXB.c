#include "generic.h"


void port_init(void)
{
	PORTB = 0x00;
 	DDRB  = 0x2C;//00101100
 	PORTC = 0x00;
 	DDRC  = 0x40;
 	PORTD = 0x80;
 	DDRD  = 0x82;
}

int main(void)
{
	float i = 0;
	port_init();
	USART_Init();
	delay_s(1);
	ADC_Init();
	SPI_MasterInit();
	
	//�ȶ�һ�����ݣ���һ�ε����ݺܴ���ʻ����
	Get_Temperature();
	
	//��ʾ0xAA��ʾ�ȴ�����״̬
	SPI_MasterTransmitaByte(0x88);//��ʾA
	SPI_MasterTransmitaByte(0x88);//��ʾA
		
	PORTB&=0xFB;
	PORTB|=0x04;
	
	
	while(1)
	{
		//�ȴ�����
		while(CMD == 0x00)
		{
	 	 	CMD = WaitForCMD();
		 	//����0��������ʧ�ܣ����µȴ��µ�����
		}
		GetSensorMessage(CMD);
		CMD = 0x00;
	}
	return 0;
}
