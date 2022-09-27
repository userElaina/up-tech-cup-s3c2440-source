#include "Generic.h"



void delay_50us(unsigned int n)
{
	unsigned int j;
	for(;n>0;n--)
	{
		for(j=0;j<70;j++)
			;
	}
}

void delay_ms(unsigned int n)
{
	for(;n>0;n--)
	{
		delay_50us(20);
	}		
}

void delay_50ms(unsigned int n)
{
	unsigned int i; 
	for(;n>0;n--)
	{
		for(i=0;i<52642;i++)
		; 
	}
}

void delay_s(unsigned int n)
{
	for(;n>0;n--)
	{
		delay_50ms(20);
	}
}
