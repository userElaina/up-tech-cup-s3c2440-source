//#include "..\startup\44b.h"
#include "inc/MotorCtrl.h"
#define PCLK (50700000)
#define MOTOR_SEVER_FRE		1000		//20kHz

#define MOTOR_CONT				(PCLK/2/2/MOTOR_SEVER_FRE)
#define MOTOR_MID				(MOTOR_CONT/2)
#define rTCFG0  (*(volatile unsigned *)0x51000000)
#define rTCFG1  (*(volatile unsigned *)0x51000004)
#define rTCNTB0  (*(volatile unsigned *)0x5100000C)
#define rTCMPB0  (*(volatile unsigned *)0x51000010)
#define rTCON  (*(volatile unsigned *)0x51000008)
#define rGPBCON  (*(volatile unsigned *)0x56000010)
#define rGPBUP  (*(volatile unsigned *)0x56000018)
#define rGPBDAT  (*(volatile unsigned *)0x56000014)

void init_MotorPort()
{   rGPBCON=rGPBCON&0x3ffff0|0xa;
   
   
    
	//Dead Zone=24, PreScalero1=2;
	rTCFG0=(0<<16)|2;

	//divider timer0=1/2;
	rTCFG1=0;

	rTCNTB0= MOTOR_CONT;
	rTCMPB0= MOTOR_MID;
		
	rTCON=0x2;		//update mode for TCNTB0 and TCMPB0.
	rTCON=0x19;		//timer0 = auto reload, start. Dead Zone
}

void SetPWM(int value)
{
	rTCMPB0= MOTOR_MID+value;
	
}
