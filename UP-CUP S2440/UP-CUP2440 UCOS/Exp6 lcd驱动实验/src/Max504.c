//#include "44b.h"
#include "inc/Exio.h"
//#include "uhal.h"
#include "inc/spi.h"
#include "inc/lib.h"

//#define MAX504_CS			0x2		//EXIO1
//#define MAX504_CLR			0x1		//EXIO0
#define MAX504_CS			0x8		//EXIO3
#define MAX504_CLR			0x4		//EXIO2


#define SIOLSB				0x40

#define MAX504_ENABLE()			do{CLREXIOBIT(MAX504_CS);}while(0)
#define MAX504_DISABLE()		do{SETEXIOBIT(MAX504_CS);}while(0)
#define MAX504_CLEAR()			do{CLREXIOBIT(MAX504_CLR);	hudelay(1); SETEXIOBIT(MAX504_CLR);}while(0)

#define Max504_FULL			4.096f

void Max504_SetDA(int value)
{
	value<<=2;//左移两位

	MAX504_CLEAR();//将DAC寄存器设为0
	hudelay(1);
	MAX504_ENABLE();//开启MAX504

    SPISend ( value>>8, 0);
    SPISend ( value, 0);

	
	
	MAX504_DISABLE();//关闭MAX504
}


void DA_Test()
{
	int i;
	float v;

	Uart_Printf(0,"\nAD begin output\n");
	for(i=0;i<1024;i+=8){
		Max504_SetDA(i);
		v=((float)i)*Max504_FULL/1024.0f;
		Uart_Printf(0,"Current Voltage=%1.2fV\r",v);
		//hudelay(10);
	}
}
