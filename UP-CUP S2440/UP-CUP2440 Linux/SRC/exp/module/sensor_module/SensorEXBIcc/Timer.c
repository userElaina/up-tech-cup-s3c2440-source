#include "Generic.h"
#include "math.h"

unsigned int HoleCountH = 0;

void timer0_init(void)
{
	//cli();//FOR AVR Studio
	CLI();
	HoleCountH = 0;
	
	TIMSK = 0x01;
 	TCCR0 = 0x00; //stop
 	TCNT0 = 0x00; 
 	TCCR0 = 0x07; //start timer
 	//sei();//FOR AVR Studio
	SEI();
}

/*
//AVR Studio编译环境下使用一下中断函数
SIGNAL(TIMER0_OVF_vect)
{
 	TCNT0 = 0x00;
	HoleCountH++;
}*/
#pragma interrupt_handler timer0_ovf_isr:10
void timer0_ovf_isr(void)
{
 	TCNT0 = 0x00;
 	HoleCountH++;
}

void StartCountHole(void)
{
	timer0_init();	
}

void FinishCountHole(void)
{
 	TCCR0 = 0x00;
}

unsigned int EndCountHole(void)
{
	return(HoleCountH*256+TCNT0);
}

void timer1_init(void)
{
	TCCR1B = 0x00; //stop
	TCNT1H = 0x00;
	TCNT1L = 0x00;
	OCR1AH = 0xFF;
	OCR1AL = 0xFF;
	OCR1BH = 0xFF;
	OCR1BL = 0xFF;
	ICR1H  = 0xFF;
	ICR1L  = 0xFF;
	TCCR1A = 0x00;
	TCCR1B = 0x07; //start Timer
}

unsigned int GetHS1101(void)
{
	unsigned int fout = 0;
	double x1,x2,x3;
	unsigned int RH = 0;
	timer1_init();
	delay_s(1);
	TCCR1B = 0x00; //stop
	fout = TCNT1;//TCNT1H<<8+TCNT1L;//TCNT1H*256+TCNT1L;
	//在ICCAVR编译环境下，使用上述第二、三种计算方法，无论先访问高字节，
	//还是先访问低字节，都会出错，郁闷~~~
	
	//电阻参数需调整
	x1=6216.006216/(double)fout;
	x2=x1*x1;
	x3=x2*x1;
	RH=(unsigned int)(-3.4656*1000*x3+1.0732*10000*x2-1.0457*10000*x1+3.2459*1000);

	if(RH>99)RH=99;
	if(RH<0.01)RH=0;
	return RH;
}
