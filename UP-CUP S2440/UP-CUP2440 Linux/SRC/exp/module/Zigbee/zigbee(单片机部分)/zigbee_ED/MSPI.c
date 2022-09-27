#include "MSPI.h"

//初始化SPI, 主机模式,始终频率FCK/2, SPI模式0
void SPI_Init(void)
{
		SPCR = (BM(SPE) | BM(MSTR)); 
		SPSR = BM(SPI2X); 
}

//SPI发送函数
void SPIPut(BYTE v)
{
    // BYTE temp;
	
	//temp = SPSR;   
	//temp = SPDR;   	//清空SPI，和中断标志，使SPI空闲  
    SPDR = v;			//启动数据传输
    while( !(SPSR & BM(SPIF)) )		//等待传输结束
    ;
}

//SPI接收函数
BYTE SPIGet(void)
{		
    SPIPut(0x00);
    return SPDR;
}

void SPIGetArray(BYTE *buffer, BYTE len)
{
    while( len-- )
        *buffer++ = SPIGet();
}

//SPI数组发送函数
void SPIPutArray(BYTE *buffer, BYTE len)
{
    while( len-- )
        SPIPut(*buffer++);
}




