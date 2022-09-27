#include "Generic.h"

void ADC_Init(void)
{  
	ADCSRA = 0x00; //disable adc
    ADMUX = 0x40; 
    ACSR  = 0x80;
    ADCSRA = 0x01;
}

float StartADC( char SensorCode )
{  
    float voltage = 0;
	ADCSRA = 0x00; //disable adc
	ADMUX = 0x40|SensorCode;//0~7
	ADCSRA = 0xc1;
	while( ADCSRA&0x40 );
	voltage = (float)ADC*VREF/1024;
	/*
		//fortest
		DisplayData(voltage);
		//���ӻ���SS����
		PORTB&=0xFB;
	
		//���ӻ���SS���ߣ������������
		PORTB|=0x04;
	*/
	return voltage;
} 

