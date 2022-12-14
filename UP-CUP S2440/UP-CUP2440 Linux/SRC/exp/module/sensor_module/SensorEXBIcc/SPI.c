#include "Generic.h"
#include "math.h"
//字符表
unsigned char CharacterTable[10] =
	{ 0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90 };
//0~9,若要显示小数点，则可表示为CharacterTable[i] & 0x7F


void SPI_MasterInit(void)
{

	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);
	//使能SPI 主机模式，设置时钟速率为fck/16
	
	SPSR = 0x00;
}

void SPI_MasterTransmitaByte(unsigned char cData)
{
	//启动数据传输
	SPDR = cData;

	//等待传输结束
	while(!(SPSR&(1<<SPIF)));
}

void DisplayData(float data)
{
	unsigned char dataH,dataL;
	
	data+=0.001;//特殊处理，防止浮点运算导致结果减小0.1
	
	//整数显示
	if((data>=10)&&(data<100))
	{
		dataH = (unsigned char)(data/10);
		dataL = (unsigned char)(data-dataH*10);
		SPI_MasterTransmitaByte(CharacterTable[dataL]);
		SPI_MasterTransmitaByte(CharacterTable[dataH]);
	}
	//浮点数显示
	else if((data>=0)&&(data<10))
	{
		dataH = (unsigned char)data;
		dataL = (unsigned char)(data*10)%10;
		SPI_MasterTransmitaByte(CharacterTable[dataL]);
		SPI_MasterTransmitaByte(CharacterTable[dataH]&0x7F);
	}
	else
	{
	 	SPI_MasterTransmitaByte(0x8E);//显示F
		SPI_MasterTransmitaByte(0x8E);//显示F
	}
	//将从机的SS拉低
	PORTB&=0xFB;
	//将从机的SS拉高，表明发送完毕
	PORTB|=0x04;
}

