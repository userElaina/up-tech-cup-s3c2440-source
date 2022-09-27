#include "inc/lib.h"
#include"led.h"

void set_lednum(void)
{
	int i,j;
	          //   0    1   2    3    4    5    6    7    8    9    a    b    c    d    e    f    -    .    0ff
	int num[19]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0x88,0x83,0xc6,0xa1,0x86,0x8e,0xbf,0x7f,0xff};
	lednum2con=num[0];
	lednum1con=num[0];
	for(j=1;j<16;j++)
	
	{
		for(i=0;i<=15;i++)
			{
				lednum2con=num[i];
				hudelay(1000);
			}
		
		lednum1con=num[j];
		
		//hudelay(3500);
	}
}

void set_1_led(void)
{
	led1con=0x0;
	led2con=0x0;
	led3con=0x0;
	led4con=0x18;
	led5con=0x18;
	led6con=0x0;
	led7con=0x0;
	led8con=0x0;
	
}
void set_2_led(void)
{
	led1con=0x0;
	led2con=0x0;
	led3con=0x3c;
	led4con=0x24;
	led5con=0x24;
	led6con=0x3c;
	led7con=0x0;
	led8con=0x0;
}
void set_3_led(void)
{
	led1con=0x0;
	led2con=0x7e;
	led3con=0x42;
	led4con=0x42;
	led5con=0x42;
	led6con=0x42;
	led7con=0x7e;
	led8con=0x0;
}
void set_4_led(void)
{
	led1con=0xff;
	led2con=0x81;
	led3con=0x81;
	led4con=0x81;
	led5con=0x81;
	led6con=0x81;
	led7con=0x81;
	led8con=0xff;
}

void test_led(void)
{
	
while(1)
	{
	//set_lednum();
	set_1_led();
	hudelay(1000);
	set_2_led();
	hudelay(1000);
	set_3_led();
	hudelay(1000);
	set_4_led();
	hudelay(1000);
	}

} 








