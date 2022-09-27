	.module ADC.c
	.area text(rom, con, rel)
	.dbfile C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc\ADC.c
	.dbfunc e ADC_Init _ADC_Init fV
	.even
_ADC_Init::
	.dbline -1
	.dbline 4
; #include "Generic.h"
; 
; void ADC_Init(void)
; {  
	.dbline 5
; 	ADCSRA = 0x00; //disable adc
	clr R2
	out 0x6,R2
	.dbline 6
;     ADMUX = 0x40; 
	ldi R24,64
	out 0x7,R24
	.dbline 7
;     ACSR  = 0x80;
	ldi R24,128
	out 0x8,R24
	.dbline 8
;     ADCSRA = 0x01;
	ldi R24,1
	out 0x6,R24
	.dbline -2
L1:
	.dbline 0 ; func end
	ret
	.dbend
	.dbfunc e StartADC _StartADC fD
;        voltage -> y+0
;     SensorCode -> R20
	.even
_StartADC::
	rcall push_gset1
	mov R20,R16
	sbiw R28,4
	.dbline -1
	.dbline 12
; }
; 
; float StartADC( char SensorCode )
; {  
	.dbline 13
;     float voltage = 0;
	ldi R16,<L3
	ldi R17,>L3
	rcall lpm32
	movw R30,R28
	std z+0,R16
	std z+1,R17
	std z+2,R18
	std z+3,R19
	.dbline 14
; 	ADCSRA = 0x00; //disable adc
	clr R2
	out 0x6,R2
	.dbline 15
; 	ADMUX = 0x40|SensorCode;//0~7
	mov R24,R20
	ori R24,64
	out 0x7,R24
	.dbline 16
; 	ADCSRA = 0xc1;
	ldi R24,193
	out 0x6,R24
L4:
	.dbline 17
L5:
	.dbline 17
; 	while( ADCSRA&0x40 );
	sbic 0x6,6
	rjmp L4
	.dbline 18
; 	voltage = (float)ADC*VREF/1024;
	in R2,0x4
	in R3,0x5
	ldi R16,<L7
	ldi R17,>L7
	rcall lpm32
	st -y,R19
	st -y,R18
	st -y,R17
	st -y,R16
	ldi R16,<L8
	ldi R17,>L8
	rcall lpm32
	st -y,R19
	st -y,R18
	st -y,R17
	st -y,R16
	movw R16,R2
	lsr R17
	ror R16
	rcall int2fp
	st -y,R19
	st -y,R18
	st -y,R17
	st -y,R16
	rcall empy32fs
	movw R16,R2
	andi R16,1
	andi R17,0
	rcall int2fp
	st -y,R19
	st -y,R18
	st -y,R17
	st -y,R16
	rcall add32fs
	rcall empy32fs
	ldi R16,<L9
	ldi R17,>L9
	rcall lpm32
	st -y,R19
	st -y,R18
	st -y,R17
	st -y,R16
	rcall div32f
	movw R30,R28
	std z+0,R16
	std z+1,R17
	std z+2,R18
	std z+3,R19
	.dbline 28
; 	/*
; 		//fortest
; 		DisplayData(voltage);
; 		//将从机的SS拉低
; 		PORTB&=0xFB;
; 	
; 		//将从机的SS拉高，表明发送完毕
; 		PORTB|=0x04;
; 	*/
; 	return voltage;
	movw R30,R28
	ldd R16,z+0
	ldd R17,z+1
	ldd R18,z+2
	ldd R19,z+3
	.dbline -2
L2:
	adiw R28,4
	rcall pop_gset1
	.dbline 0 ; func end
	ret
	.dbsym l voltage 0 D
	.dbsym r SensorCode 20 c
	.dbend
	.area lit(rom, con, rel)
L9:
	.word 0x0,0x4480
L8:
	.word 0x0,0x4000
L7:
	.word 0x0,0x40a0
L3:
	.word 0x0,0x0
