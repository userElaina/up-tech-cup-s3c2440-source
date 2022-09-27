/***************************************************************************\
	Copyright (c) 2004-2007 threewater@up-tech.com, All rights reserved.
	by threewter	2004.4.26
\***************************************************************************/
	

/***************************************************************************\
    #说明: s3c44b0 AD接口程序
	----------------------------------  Bug  --------------------------------------

	----------------------------------  TODO list  --------------------------------------

	----------------------------------修正--------------------------------------
	2004-11-10	创建

\***************************************************************************/
#include "../inc/sys/lib.h"
#include "../inc/drivers.h"
#include <includes.h>
#include <stdio.h>

#define TLC5617_CS		(GPIO_ENABLE  | GPIO_MODE_DISINT |GPIO_MODE_OUT | GPIO_B8)
#define TLC5617_DI		(GPIO_ENABLE  | GPIO_MODE_DISINT |GPIO_MODE_OUT | GPIO_E19)
#define TLC5617_CLK		(GPIO_ENABLE  | GPIO_MODE_DISINT |GPIO_MODE_OUT | GPIO_E21)

#define REF_VOLTAGE		2.5f		// 2.5v

#define TLC5617CON_WRBUF			(0<<12)	// Write to double buffer latch only
#define TLC5617CON_LATCH			(0<<12)	
#define TLC5617CON_POWERUP		(0<<13)
#define TLC5617CON_POWERDOWN		(1<<13)
#define TLC5617CON_12us			(1<<14)	// 12.5 us
#define TLC5617CON_2us				(0<<14)	// 2.5 us
#define TLC5617CON_CHA				(1<<15)	// channel A
#define TLC5617CON_CHB				(0<<15)	// channel B

#define TLC5617_DEFAULT_CONTOL	(TLC5617CON_2us|TLC5617CON_POWERUP|TLC5617CON_LATCH)

void TLC5617_WriteData(U16 data)
{
#if OS_CRITICAL_METHOD == 3                  /* Allocate storage for CPU status register               */
    OS_CPU_SR  cpu_sr;
#endif    

	int i;

	OS_ENTER_CRITICAL();

	set_gpio_bit(TLC5617_CLK);
	clear_gpio_bit(TLC5617_CS);

	//移位输出
	for(i=15; i>=0; i--){
		write_gpio_bit(TLC5617_DI, (data>>i));
		clear_gpio_bit(TLC5617_CLK);
		set_gpio_bit(TLC5617_CLK);
	}

	set_gpio_bit(TLC5617_CS);
	clear_gpio_bit(TLC5617_CLK);

	OS_EXIT_CRITICAL();
}

void init_DAdevice()
{
	set_gpio_ctrl(TLC5617_CLK);
	set_gpio_ctrl(TLC5617_CS);
	set_gpio_ctrl(TLC5617_DI);

	set_gpio_bit(TLC5617_CS);
	clear_gpio_bit(TLC5617_CLK);
}

/***************************************************************************\
	设置DA数值, 
	channel = 0 	A通道
	channel = 1 	B通道
\***************************************************************************/
void DA_Set(int channel, int value)
{
	if(value>1023){
		printk("DA channel %d, out of range value=%d\n", channel, value);
		value=0x3ff;
	}

	TLC5617_WriteData((((~channel&0x1))<<15)|TLC5617_DEFAULT_CONTOL|((value&0x3ff)<<2));
}

void DA_test(void)
{
	int i;
	char string[100];

	init_DAdevice();
	printf("\n");
	for(;;){
		float v;
		for(i=0;i<2;i++){
			printf("Channel %c (x.x)V: ", (i==0)?'A':'B');
			GetCommand(string);
			sscanf(string, "%f", &v);
			putchar('\n');

			v=v*1024/(REF_VOLTAGE*2);
			DA_Set(i, (int)v);
		}
	}
}

