#include "MSPI.h"

//��ʼ��SPI, ����ģʽ,ʼ��Ƶ��FCK/2, SPIģʽ0
void SPI_Init(void)
{
		SPCR = (BM(SPE) | BM(MSTR)); 
		SPSR = BM(SPI2X); 
}

//SPI���ͺ���
void SPIPut(BYTE v)
{
    // BYTE temp;
	
	//temp = SPSR;   
	//temp = SPDR;   	//���SPI�����жϱ�־��ʹSPI����  
    SPDR = v;			//�������ݴ���
    while( !(SPSR & BM(SPIF)) )		//�ȴ��������
    ;
}

//SPI���պ���
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

//SPI���鷢�ͺ���
void SPIPutArray(BYTE *buffer, BYTE len)
{
    while( len-- )
        SPIPut(*buffer++);
}




