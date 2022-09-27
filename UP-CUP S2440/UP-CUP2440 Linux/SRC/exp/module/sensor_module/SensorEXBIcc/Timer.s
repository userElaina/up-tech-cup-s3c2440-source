	.module Timer.c
	.area data(ram, con, rel)
_HoleCountH::
	.blkb 2
	.area idata
	.word 0
	.area data(ram, con, rel)
	.dbfile C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc\Timer.c
	.dbsym e HoleCountH _HoleCountH i
	.area text(rom, con, rel)
	.dbfile C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc\Timer.c
	.dbfunc e timer0_init _timer0_init fV
	.even
_timer0_init::
	.dbline -1
	.dbline 7
; #include "Generic.h"
; #include "math.h"
; 
; unsigned int HoleCountH = 0;
; 
; void timer0_init(void)
; {
	.dbline 9
; 	//cli();//FOR AVR Studio
; 	CLI();
	cli
	.dbline 10
; 	HoleCountH = 0;
	clr R2
	clr R3
	sts _HoleCountH+1,R3
	sts _HoleCountH,R2
	.dbline 12
; 	
; 	TIMSK = 0x01;
	ldi R24,1
	out 0x39,R24
	.dbline 13
;  	TCCR0 = 0x00; //stop
	out 0x33,R2
	.dbline 14
;  	TCNT0 = 0x00; 
	out 0x32,R2
	.dbline 15
;  	TCCR0 = 0x07; //start timer
	ldi R24,7
	out 0x33,R24
	.dbline 17
;  	//sei();//FOR AVR Studio
; 	SEI();
	sei
	.dbline -2
L1:
	.dbline 0 ; func end
	ret
	.dbend
	.area vector(rom, abs)
	.org 18
	rjmp _timer0_ovf_isr
	.area text(rom, con, rel)
	.dbfile C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc\Timer.c
	.dbfunc e timer0_ovf_isr _timer0_ovf_isr fV
	.even
_timer0_ovf_isr::
	st -y,R2
	st -y,R24
	st -y,R25
	in R2,0x3f
	st -y,R2
	.dbline -1
	.dbline 29
; }
; 
; /*
; //AVR Studio编译环境下使用一下中断函数
; SIGNAL(TIMER0_OVF_vect)
; {
;  	TCNT0 = 0x00;
; 	HoleCountH++;
; }*/
; #pragma interrupt_handler timer0_ovf_isr:10
; void timer0_ovf_isr(void)
; {
	.dbline 30
;  	TCNT0 = 0x00;
	clr R2
	out 0x32,R2
	.dbline 31
;  	HoleCountH++;
	lds R24,_HoleCountH
	lds R25,_HoleCountH+1
	adiw R24,1
	sts _HoleCountH+1,R25
	sts _HoleCountH,R24
	.dbline -2
L2:
	ld R2,y+
	out 0x3f,R2
	ld R25,y+
	ld R24,y+
	ld R2,y+
	.dbline 0 ; func end
	reti
	.dbend
	.dbfunc e StartCountHole _StartCountHole fV
	.even
_StartCountHole::
	.dbline -1
	.dbline 35
; }
; 
; void StartCountHole(void)
; {
	.dbline 36
; 	timer0_init();	
	rcall _timer0_init
	.dbline -2
L3:
	.dbline 0 ; func end
	ret
	.dbend
	.dbfunc e FinishCountHole _FinishCountHole fV
	.even
_FinishCountHole::
	.dbline -1
	.dbline 40
; }
; 
; void FinishCountHole(void)
; {
	.dbline 41
;  	TCCR0 = 0x00;
	clr R2
	out 0x33,R2
	.dbline -2
L4:
	.dbline 0 ; func end
	ret
	.dbend
	.dbfunc e EndCountHole _EndCountHole fi
	.even
_EndCountHole::
	.dbline -1
	.dbline 45
; }
; 
; unsigned int EndCountHole(void)
; {
	.dbline 46
; 	return(HoleCountH*256+TCNT0);
	lds R18,_HoleCountH
	lds R19,_HoleCountH+1
	ldi R16,256
	ldi R17,1
	rcall empy16s
	movw R2,R16
	in R4,0x32
	clr R5
	add R2,R4
	adc R3,R5
	movw R16,R2
	.dbline -2
L5:
	.dbline 0 ; func end
	ret
	.dbend
	.dbfunc e timer1_init _timer1_init fV
	.even
_timer1_init::
	.dbline -1
	.dbline 50
; }
; 
; void timer1_init(void)
; {
	.dbline 51
; 	TCCR1B = 0x00; //stop
	clr R2
	out 0x2e,R2
	.dbline 52
; 	TCNT1H = 0x00;
	out 0x2d,R2
	.dbline 53
; 	TCNT1L = 0x00;
	out 0x2c,R2
	.dbline 54
; 	OCR1AH = 0xFF;
	ldi R24,255
	out 0x2b,R24
	.dbline 55
; 	OCR1AL = 0xFF;
	out 0x2a,R24
	.dbline 56
; 	OCR1BH = 0xFF;
	out 0x29,R24
	.dbline 57
; 	OCR1BL = 0xFF;
	out 0x28,R24
	.dbline 58
; 	ICR1H  = 0xFF;
	out 0x27,R24
	.dbline 59
; 	ICR1L  = 0xFF;
	out 0x26,R24
	.dbline 60
; 	TCCR1A = 0x00;
	out 0x2f,R2
	.dbline 61
; 	TCCR1B = 0x07; //start Timer
	ldi R24,7
	out 0x2e,R24
	.dbline -2
L6:
	.dbline 0 ; func end
	ret
	.dbend
	.dbfunc e GetHS1101 _GetHS1101 fi
;             x3 -> y+8
;             x2 -> y+4
;           fout -> R12,R13
;             x1 -> y+0
;             RH -> R10,R11
	.even
_GetHS1101::
	rcall push_gset4
	sbiw R28,16
	.dbline -1
	.dbline 65
; }
; 
; unsigned int GetHS1101(void)
; {
	.dbline 66
; 	unsigned int fout = 0;
	clr R12
	clr R13
	.dbline 68
; 	double x1,x2,x3;
; 	unsigned int RH = 0;
	clr R10
	clr R11
	.dbline 69
; 	timer1_init();
	rcall _timer1_init
	.dbline 70
; 	delay_s(1);
	ldi R16,1
	ldi R17,0
	rcall _delay_s
	.dbline 71
; 	TCCR1B = 0x00; //stop
	clr R2
	out 0x2e,R2
	.dbline 72
; 	fout = TCNT1;//TCNT1H<<8+TCNT1L;//TCNT1H*256+TCNT1L;
	in R12,0x2c
	in R13,0x2d
	.dbline 77
; 	//在ICCAVR编译环境下，使用上述第二、三种计算方法，无论先访问高字节，
; 	//还是先访问低字节，都会出错，郁闷~~~
; 	
; 	//电阻参数需调整
; 	x1=6216.006216/(double)fout;
	ldi R16,<L8
	ldi R17,>L8
	rcall lpm32
	st -y,R19
	st -y,R18
	st -y,R17
	st -y,R16
	ldi R16,<L9
	ldi R17,>L9
	rcall lpm32
	st -y,R19
	st -y,R18
	st -y,R17
	st -y,R16
	movw R16,R12
	lsr R17
	ror R16
	rcall int2fp
	st -y,R19
	st -y,R18
	st -y,R17
	st -y,R16
	rcall empy32fs
	movw R16,R12
	andi R16,1
	andi R17,0
	rcall int2fp
	st -y,R19
	st -y,R18
	st -y,R17
	st -y,R16
	rcall add32fs
	rcall div32f
	movw R30,R28
	std z+0,R16
	std z+1,R17
	std z+2,R18
	std z+3,R19
	.dbline 78
; 	x2=x1*x1;
	movw R30,R28
	ldd R2,z+0
	ldd R3,z+1
	ldd R4,z+2
	ldd R5,z+3
	st -y,R5
	st -y,R4
	st -y,R3
	st -y,R2
	movw R30,R28
 ; stack offset 4
	ldd R2,z+4
	ldd R3,z+5
	ldd R4,z+6
	ldd R5,z+7
	st -y,R5
	st -y,R4
	st -y,R3
	st -y,R2
	rcall empy32f
	movw R30,R28
	std z+4,R16
	std z+5,R17
	std z+6,R18
	std z+7,R19
	.dbline 79
; 	x3=x2*x1;
	movw R30,R28
	ldd R2,z+4
	ldd R3,z+5
	ldd R4,z+6
	ldd R5,z+7
	st -y,R5
	st -y,R4
	st -y,R3
	st -y,R2
	movw R30,R28
 ; stack offset 4
	ldd R2,z+4
	ldd R3,z+5
	ldd R4,z+6
	ldd R5,z+7
	st -y,R5
	st -y,R4
	st -y,R3
	st -y,R2
	rcall empy32f
	movw R30,R28
	std z+8,R16
	std z+9,R17
	std z+10,R18
	std z+11,R19
	.dbline 80
; 	RH=(unsigned int)(-3.4656*1000*x3+1.0732*10000*x2-1.0457*10000*x1+3.2459*1000);
	ldi R16,<L12
	ldi R17,>L12
	rcall lpm32
	st -y,R19
	st -y,R18
	st -y,R17
	st -y,R16
	movw R30,R28
 ; stack offset 4
	ldd R2,z+12
	ldd R3,z+13
	ldd R4,z+14
	ldd R5,z+15
	st -y,R5
	st -y,R4
	st -y,R3
	st -y,R2
	rcall empy32fs
	ldi R16,<L13
	ldi R17,>L13
	rcall lpm32
	st -y,R19
	st -y,R18
	st -y,R17
	st -y,R16
	movw R30,R28
 ; stack offset 8
	ldd R2,z+12
	ldd R3,z+13
	ldd R4,z+14
	ldd R5,z+15
	st -y,R5
	st -y,R4
	st -y,R3
	st -y,R2
	rcall empy32fs
	rcall add32fs
	ldi R16,<L14
	ldi R17,>L14
	rcall lpm32
	st -y,R19
	st -y,R18
	st -y,R17
	st -y,R16
	movw R30,R28
 ; stack offset 8
	ldd R2,z+8
	ldd R3,z+9
	ldd R4,z+10
	ldd R5,z+11
	st -y,R5
	st -y,R4
	st -y,R3
	st -y,R2
	rcall empy32fs
	rcall sub32fs
	ldi R16,<L15
	ldi R17,>L15
	rcall lpm32
	st -y,R19
	st -y,R18
	st -y,R17
	st -y,R16
	rcall add32f
	movw R30,R28
	std z+12,R16
	std z+13,R17
	std z+14,R18
	std z+15,R19
	movw R30,R28
	ldd R2,z+12
	ldd R3,z+13
	ldd R4,z+14
	ldd R5,z+15
	st -y,R5
	st -y,R4
	st -y,R3
	st -y,R2
	ldi R16,<L16
	ldi R17,>L16
	rcall lpm32
	st -y,R19
	st -y,R18
	st -y,R17
	st -y,R16
	rcall cmp32f
	brlt L10
	movw R30,R28
	ldd R2,z+12
	ldd R3,z+13
	ldd R4,z+14
	ldd R5,z+15
	st -y,R5
	st -y,R4
	st -y,R3
	st -y,R2
	ldi R16,<L16
	ldi R17,>L16
	rcall lpm32
	st -y,R19
	st -y,R18
	st -y,R17
	st -y,R16
	rcall sub32f
	rcall fp2int
	movw R24,R16
	subi R24,0  ; offset = 32768
	sbci R25,128
	movw R12,R24
	rjmp L11
L10:
	movw R30,R28
	ldd R16,z+12
	ldd R17,z+13
	ldd R18,z+14
	ldd R19,z+15
	rcall fp2int
	movw R12,R16
L11:
	movw R10,R12
	.dbline 82
; 
; 	if(RH>99)RH=99;
	ldi R24,99
	ldi R25,0
	cp R24,R12
	cpc R25,R13
	brsh L17
	.dbline 82
	movw R10,R24
L17:
	.dbline 83
; 	if(RH<0.01)RH=0;
	ldi R16,<L9
	ldi R17,>L9
	rcall lpm32
	st -y,R19
	st -y,R18
	st -y,R17
	st -y,R16
	movw R16,R10
	lsr R17
	ror R16
	rcall int2fp
	st -y,R19
	st -y,R18
	st -y,R17
	st -y,R16
	rcall empy32fs
	movw R16,R10
	andi R16,1
	andi R17,0
	rcall int2fp
	st -y,R19
	st -y,R18
	st -y,R17
	st -y,R16
	rcall add32fs
	ldi R16,<L21
	ldi R17,>L21
	rcall lpm32
	st -y,R19
	st -y,R18
	st -y,R17
	st -y,R16
	rcall cmp32f
	brge L19
	.dbline 83
	clr R10
	clr R11
L19:
	.dbline 84
; 	return RH;
	movw R16,R10
	.dbline -2
L7:
	adiw R28,16
	rcall pop_gset4
	.dbline 0 ; func end
	ret
	.dbsym l x3 8 D
	.dbsym l x2 4 D
	.dbsym r fout 12 i
	.dbsym l x1 0 D
	.dbsym r RH 10 i
	.dbend
	.area lit(rom, con, rel)
L21:
	.word 0xd70a,0x3c23
L16:
	.word 0x0,0x4700
L15:
	.word 0xde66,0x454a
L14:
	.word 0x6400,0x4623
L13:
	.word 0xb000,0x4627
L12:
	.word 0x999a,0xc558
L9:
	.word 0x0,0x4000
L8:
	.word 0x400d,0x45c2
