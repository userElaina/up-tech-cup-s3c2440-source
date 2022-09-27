/*  m818b20.c   
  ������Ϊ����mega8 ��18b20���¶Ȳɼ����� 
  ѡ��mega8�ڲ�8M RC�𵴣�18b20 �����߽�pd6,�����ߺ�vcc���һ4.7k�������� */ 

#include "SensorProcess.h"
#include "zigbee.h"		//Zigbee����
#include "zAPL.h"	//ZigbeeӦ�ò�
#include "18b20.h"

void init_1820(void); 
void write_1820(unsigned char x); 
unsigned char read_1820(void); 
float count; //�¶�


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
 * Overview:      ���ص�ǰ�¶�
 *
 * Note:            None
 ********************************************************************/
float Get_Temperature(void)  
{ 
	unsigned char temh,teml;
	int i; 
    init_1820();        //��λ18b20 
    write_1820(0xcc);   // ����ת������ 
    write_1820(0x44); 
	i = 400;
	while(i--); 
    init_1820(); 
    write_1820(0xcc);  //���������� 
    write_1820(0xbe); 
    teml=read_1820();  //������ 
    temh=read_1820(); 
    //count=(temh*256+teml)*6.25;  //��������¶� 
    count=(temh*256+teml)*0.0625;  //��������¶� 
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
	while(i--);//480us���� 
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
        if(x&(1<<m))    //д���ݣ��ӵ�λ��ʼ 
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
		k = (PINA&0x80);  //������,�ӵ�λ��ʼ 
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


