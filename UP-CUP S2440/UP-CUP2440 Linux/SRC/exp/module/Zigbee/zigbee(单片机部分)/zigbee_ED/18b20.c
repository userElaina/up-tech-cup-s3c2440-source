/*  m818b20.c   
  本程序为采用mega8 和18b20的温度采集程序 
  选用mega8内部8M RC震荡，18b20 数据线接pd6,数据线和vcc间接一4.7k上拉电阻 */ 

#include "SensorProcess.h"
#include "zigbee.h"		//Zigbee定义
#include "zAPL.h"	//Zigbee应用层
#include "18b20.h"

void init_1820(void); 
void write_1820(unsigned char x); 
unsigned char read_1820(void); 
float count; //温度


/*********************************************************************
 * Function:        void Get_Temperature(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          Temperature
 *
 * Side Effects:    None
 *
 * Overview:      返回当前温度
 *
 * Note:            None
 ********************************************************************/
float Get_Temperature(void)  
{ 
	unsigned char temh,teml;
	int i; 
    init_1820();        //复位18b20 
    write_1820(0xcc);   // 发出转换命令 
    write_1820(0x44); 
	i = 400;
	while(i--); 
    init_1820(); 
    write_1820(0xcc);  //发出读命令 
    write_1820(0xbe); 
    teml=read_1820();  //读数据 
    temh=read_1820(); 
    //count=(temh*256+teml)*6.25;  //计算具体温度 
    count=(temh*256+teml)*0.0625;  //计算具体温度 
    return count;
} 

void Test_1820(void)
{
	long TT=0;
	int i =0;
	unsigned int k=0;
	while(1)
	{
		TT=Get_Temperature();
		k=(unsigned int)TT%10-6;// 4
		switch(k)
		{
			case 0:
				{
					break;
				}
			case 1:
				{
					SET_SensorLED1();
					break;
				}
			case 2:
				{
					SET_SensorLED2();
					break;
				}
			case 3:
				{
					SET_SensorLED3();
					break;
				}
			case 4:
				{
					SET_SensorLED4();
					break;
				}
			default:
				{
					SET_SensorLED1();
					SET_SensorLED2();
					SET_SensorLED3();
					SET_SensorLED4();
					break;
				}
		}
		i=5000;
		while(i--);
		CLR_SensorLED4_EN();
 		CLR_SensorLED3_EN();
		CLR_SensorLED2_EN();
		CLR_SensorLED1_EN();
		
	}
}

void init_1820(void) 
{ 
	int i;
	DDRA |= 0x80;
	PORTA |= 0x80;
	PORTA &= ~0x80;
	i = 3000;
	while(i--);//480us以上 
	PORTA |= 0x80;
	DDRA &= ~0x80;
	i = 40;
	while(i--);//15~60us
	while(PINA & 0x80);
    DDRA |= 0x80;
	PORTA |= 0x80;
	i = 150;
	while(i--);//60~240us  
} 

void write_1820(unsigned char x) 
{    
    unsigned char m;
	int i;
    for(m=0;m<8;m++) 
    { 
		PORTA &= ~0x80;
        if(x&(1<<m))    //写数据，从低位开始 
		PORTA |= 0x80;
        else
		PORTA &= ~0x80;
		i = 40;
        while(i--);   //15~60us 
		PORTA |= 0x80;
    } 
    PORTA |= 0x80; 
} 


unsigned char read_1820(void) 
{     
	unsigned char temp,k,n,i; 
    temp=0; 
    for(n=0;n<8;n++) 
    { 
		PORTA &= ~0x80;
		PORTA |= 0x80;
		DDRA &= ~0x80;
		k = (PINA&0x80);  //读数据,从低位开始 
        if(k) 
        temp|=(1<<n); 
        else 
        temp&=~(1<<n); 
		i = 50;
		while(i--); //60~120us     
		DDRA |= 0x80;
    } 
    return (temp);
}  


