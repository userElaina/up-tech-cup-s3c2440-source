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
	
	//先读一次数据，第一次的数据很大概率会出错
	Get_Temperature();
	
	//显示0xAA表示等待命令状态
	SPI_MasterTransmitaByte(0x88);//显示A
	SPI_MasterTransmitaByte(0x88);//显示A
		
	PORTB&=0xFB;
	PORTB|=0x04;
	
	
	while(1)
	{
		//等待命令
		while(CMD == 0x00)
		{
	 	 	CMD = WaitForCMD();
		 	//返回0代表命令失败，重新等待新的命令
		}
		GetSensorMessage(CMD);
		CMD = 0x00;
	}
	return 0;
}
