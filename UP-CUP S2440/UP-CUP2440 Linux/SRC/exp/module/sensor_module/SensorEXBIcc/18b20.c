#include "Generic.h"


float count; 
//温度


void init_1820(void) 
{ 
	int i;

	DDRB |= 0x01;
	PORTB |= 0x01;
	PORTB &= ~0x01;
	
	i = 3000;
	while(i--);//480us以上 

	PORTB |= 0x01;
	DDRB &= ~0x01;
	
	i = 40;
	while(i--);//15~60us
	while(PINB & 0x01);
    DDRB |= 0x01;
	PORTB |= 0x01;
	i = 150;
	while(i--);//60~240us  
} 

void write_1820(unsigned char x) 
{    
    unsigned char m;
	int i;
    for(m=0;m<8;m++) 
    { 
		PORTB &= ~0x01;
        if(x&(1<<m))    //写数据，从低位开始 
		PORTB |= 0x01;
        else
		PORTB &= ~0x01;
		i = 40;
        while(i--);   //15~60us 
		PORTB |= 0x01;
    } 
    PORTB |= 0x01; 
} 


unsigned char read_1820(void) 
{     
	unsigned char temp,k,n,i; 
    temp=0; 
    for(n=0;n<8;n++) 
    { 
		PORTB &= ~0x01;
		PORTB |= 0x01;
		DDRB &= ~0x01;
		k = (PINB&0x01);
		//读数据,从低位开始 
        if(k) 
        temp|=(1<<n); 
        else 
        temp&=~(1<<n); 
		i = 50;
		while(i--); //60~120us     
		DDRB |= 0x01;
    } 
    return (temp);
}  


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

	//发出读命令 
	write_1820(0xcc);  
    write_1820(0xbe); 

	//读数据 
	teml=read_1820();
    temh=read_1820(); 

	//计算具体温度 
	count=(temh*256+teml)*0.0625;
    return count;
} 



