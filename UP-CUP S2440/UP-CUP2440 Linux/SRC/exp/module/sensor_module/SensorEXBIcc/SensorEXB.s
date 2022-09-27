	.module SensorEXB.c
	.area text(rom, con, rel)
	.dbfile C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc\SensorEXB.c
	.dbfunc e port_init _port_init fV
	.even
_port_init::
	.dbline -1
	.dbline 5
; #include "generic.h"
; 
; 
; void port_init(void)
; {
	.dbline 6
; 	PORTB = 0x00;
	clr R2
	out 0x18,R2
	.dbline 7
;  	DDRB  = 0x2C;//00101100
	ldi R24,44
	out 0x17,R24
	.dbline 8
;  	PORTC = 0x00;
	out 0x15,R2
	.dbline 9
;  	DDRC  = 0x40;
	ldi R24,64
	out 0x14,R24
	.dbline 10
;  	PORTD = 0x80;
	ldi R24,128
	out 0x12,R24
	.dbline 11
;  	DDRD  = 0x82;
	ldi R24,130
	out 0x11,R24
	.dbline -2
L1:
	.dbline 0 ; func end
	ret
	.dbend
	.dbfunc e main _main fI
;              i -> y+0
	.even
_main::
	sbiw R28,4
	.dbline -1
	.dbline 15
; }
; 
; int main(void)
; {
	.dbline 16
; 	float i = 0;
	ldi R16,<L3
	ldi R17,>L3
	rcall lpm32
	movw R30,R28
	std z+0,R16
	std z+1,R17
	std z+2,R18
	std z+3,R19
	.dbline 17
; 	port_init();
	rcall _port_init
	.dbline 18
; 	USART_Init();
	rcall _USART_Init
	.dbline 19
; 	delay_s(1);
	ldi R16,1
	ldi R17,0
	rcall _delay_s
	.dbline 20
; 	ADC_Init();
	rcall _ADC_Init
	.dbline 21
; 	SPI_MasterInit();
	rcall _SPI_MasterInit
	.dbline 24
; 	
; 	//先读一次数据，第一次的数据很大概率会出错
; 	Get_Temperature();
	rcall _Get_Temperature
	.dbline 27
; 	
; 	//显示0xAA表示等待命令状态
; 	SPI_MasterTransmitaByte(0x88);//显示A
	ldi R16,136
	rcall _SPI_MasterTransmitaByte
	.dbline 28
; 	SPI_MasterTransmitaByte(0x88);//显示A
	ldi R16,136
	rcall _SPI_MasterTransmitaByte
	.dbline 30
; 		
; 	PORTB&=0xFB;
	in R24,0x18
	andi R24,251
	out 0x18,R24
	.dbline 31
; 	PORTB|=0x04;
	sbi 0x18,2
	rjmp L5
X0:
	.dbline 35
; 	
; 	
; 	while(1)
; 	{
L7:
	.dbline 38
	.dbline 39
	rcall _WaitForCMD
	sts _CMD,R16
	.dbline 41
L8:
	.dbline 37
	lds R2,_CMD
	tst R2
	breq L7
	.dbline 42
	mov R16,R2
	rcall _GetSensorMessage
	.dbline 43
	clr R2
	sts _CMD,R2
	.dbline 44
L5:
	.dbline 34
	rjmp L8
X1:
	.dbline 45
; 		//等待命令
; 		while(CMD == 0x00)
; 		{
; 	 	 	CMD = WaitForCMD();
; 		 	//返回0代表命令失败，重新等待新的命令
; 		}
; 		GetSensorMessage(CMD);
; 		CMD = 0x00;
; 	}
; 	return 0;
	clr R16
	clr R17
	.dbline -2
L2:
	adiw R28,4
	.dbline 0 ; func end
	ret
	.dbsym l i 0 D
	.dbend
	.area lit(rom, con, rel)
L3:
	.word 0x0,0x0
