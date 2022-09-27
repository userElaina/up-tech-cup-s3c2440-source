	.module _18b20.c
	.area text(rom, con, rel)
	.dbfile C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc\18b20.c
	.dbfunc e init_1820 _init_1820 fV
;              i -> R16,R17
	.even
_init_1820::
	.dbline -1
	.dbline 9
; #include "Generic.h"
; 
; 
; float count; 
; //温度
; 
; 
; void init_1820(void) 
; { 
	.dbline 12
; 	int i;
; 
; 	DDRB |= 0x01;
	sbi 0x17,0
	.dbline 13
; 	PORTB |= 0x01;
	sbi 0x18,0
	.dbline 14
; 	PORTB &= ~0x01;
	cbi 0x18,0
	.dbline 16
; 	
; 	i = 3000;
	ldi R16,3000
	ldi R17,11
L2:
	.dbline 17
L3:
	.dbline 17
; 	while(i--);//480us以上 
	movw R2,R16
	subi R16,1
	sbci R17,0
	tst R2
	brne L2
	tst R3
	brne L2
X0:
	.dbline 19
; 
; 	PORTB |= 0x01;
	sbi 0x18,0
	.dbline 20
; 	DDRB &= ~0x01;
	cbi 0x17,0
	.dbline 22
; 	
; 	i = 40;
	ldi R16,40
	ldi R17,0
L5:
	.dbline 23
L6:
	.dbline 23
; 	while(i--);//15~60us
	movw R2,R16
	subi R16,1
	sbci R17,0
	tst R2
	brne L5
	tst R3
	brne L5
X1:
L8:
	.dbline 24
L9:
	.dbline 24
; 	while(PINB & 0x01);
	sbic 0x16,0
	rjmp L8
	.dbline 25
;     DDRB |= 0x01;
	sbi 0x17,0
	.dbline 26
; 	PORTB |= 0x01;
	sbi 0x18,0
	.dbline 27
; 	i = 150;
	ldi R16,150
	ldi R17,0
L11:
	.dbline 28
L12:
	.dbline 28
; 	while(i--);//60~240us  
	movw R2,R16
	subi R16,1
	sbci R17,0
	tst R2
	brne L11
	tst R3
	brne L11
X2:
	.dbline -2
L1:
	.dbline 0 ; func end
	ret
	.dbsym r i 16 I
	.dbend
	.dbfunc e write_1820 _write_1820 fV
;              m -> R20
;              i -> R10,R11
;              x -> R22
	.even
_write_1820::
	rcall push_gset3
	mov R22,R16
	.dbline -1
	.dbline 32
; } 
; 
; void write_1820(unsigned char x) 
; {    
	.dbline 35
;     unsigned char m;
; 	int i;
;     for(m=0;m<8;m++) 
	clr R20
	rjmp L18
L15:
	.dbline 36
;     { 
	.dbline 37
; 		PORTB &= ~0x01;
	cbi 0x18,0
	.dbline 38
;         if(x&(1<<m))    //写数据，从低位开始 
	ldi R16,1
	mov R17,R20
	rcall lsl8
	mov R2,R22
	and R2,R16
	breq L19
	.dbline 39
; 		PORTB |= 0x01;
	sbi 0x18,0
	rjmp L20
L19:
	.dbline 41
;         else
; 		PORTB &= ~0x01;
	cbi 0x18,0
L20:
	.dbline 42
; 		i = 40;
	ldi R24,40
	ldi R25,0
	movw R10,R24
L21:
	.dbline 43
L22:
	.dbline 43
	movw R2,R10
	movw R24,R2
	sbiw R24,1
	movw R10,R24
	tst R2
	brne L21
	tst R3
	brne L21
X3:
	.dbline 44
	sbi 0x18,0
	.dbline 45
L16:
	.dbline 35
	inc R20
L18:
	.dbline 35
	cpi R20,8
	brlo L15
	.dbline 46
;         while(i--);   //15~60us 
; 		PORTB |= 0x01;
;     } 
;     PORTB |= 0x01; 
	sbi 0x18,0
	.dbline -2
L14:
	rcall pop_gset3
	.dbline 0 ; func end
	ret
	.dbsym r m 20 c
	.dbsym r i 10 I
	.dbsym r x 22 c
	.dbend
	.dbfunc e read_1820 _read_1820 fc
;           temp -> R20
;              k -> R22
;              n -> R10
;              i -> R12
	.even
_read_1820::
	rcall push_gset4
	.dbline -1
	.dbline 51
; } 
; 
; 
; unsigned char read_1820(void) 
; {     
	.dbline 53
; 	unsigned char temp,k,n,i; 
;     temp=0; 
	clr R20
	.dbline 54
;     for(n=0;n<8;n++) 
	clr R10
	rjmp L28
L25:
	.dbline 55
;     { 
	.dbline 56
; 		PORTB &= ~0x01;
	cbi 0x18,0
	.dbline 57
; 		PORTB |= 0x01;
	sbi 0x18,0
	.dbline 58
; 		DDRB &= ~0x01;
	cbi 0x17,0
	.dbline 59
; 		k = (PINB&0x01);
	in R22,0x16
	andi R22,1
	.dbline 61
; 		//读数据,从低位开始 
;         if(k) 
	breq L29
	.dbline 62
;         temp|=(1<<n); 
	ldi R16,1
	mov R17,R10
	rcall lsl8
	or R20,R16
	rjmp L30
L29:
	.dbline 64
;         else 
;         temp&=~(1<<n); 
	ldi R16,1
	mov R17,R10
	rcall lsl8
	mov R2,R16
	com R2
	and R20,R2
L30:
	.dbline 65
; 		i = 50;
	ldi R24,50
	mov R12,R24
L31:
	.dbline 66
L32:
	.dbline 66
	mov R2,R12
	clr R3
	mov R24,R2
	subi R24,1
	mov R12,R24
	tst R2
	brne L31
	.dbline 67
	sbi 0x17,0
	.dbline 68
L26:
	.dbline 54
	inc R10
L28:
	.dbline 54
	mov R24,R10
	cpi R24,8
	brlo L25
	.dbline 69
; 		while(i--); //60~120us     
; 		DDRB |= 0x01;
;     } 
;     return (temp);
	mov R16,R20
	.dbline -2
L24:
	rcall pop_gset4
	.dbline 0 ; func end
	ret
	.dbsym r temp 20 c
	.dbsym r k 22 c
	.dbsym r n 10 c
	.dbsym r i 12 c
	.dbend
	.dbfunc e Get_Temperature _Get_Temperature fD
;           teml -> R20
;           temh -> R22
;              i -> R10,R11
	.even
_Get_Temperature::
	rcall push_gset3
	.dbline -1
	.dbline 74
; }  
; 
; 
; float Get_Temperature(void)  
; { 
	.dbline 77
; 	unsigned char temh,teml;
; 	int i; 
;     init_1820();        //复位18b20 
	rcall _init_1820
	.dbline 78
;     write_1820(0xcc);   // 发出转换命令 
	ldi R16,204
	rcall _write_1820
	.dbline 79
;     write_1820(0x44); 
	ldi R16,68
	rcall _write_1820
	.dbline 80
; 	i = 400;
	ldi R24,400
	ldi R25,1
	movw R10,R24
L35:
	.dbline 81
L36:
	.dbline 81
; 	while(i--); 
	movw R2,R10
	movw R24,R2
	sbiw R24,1
	movw R10,R24
	tst R2
	brne L35
	tst R3
	brne L35
X4:
	.dbline 82
;     init_1820(); 
	rcall _init_1820
	.dbline 85
; 
; 	//发出读命令 
; 	write_1820(0xcc);  
	ldi R16,204
	rcall _write_1820
	.dbline 86
;     write_1820(0xbe); 
	ldi R16,190
	rcall _write_1820
	.dbline 89
; 
; 	//读数据 
; 	teml=read_1820();
	rcall _read_1820
	mov R20,R16
	.dbline 90
;     temh=read_1820(); 
	rcall _read_1820
	mov R22,R16
	.dbline 93
; 
; 	//计算具体温度 
; 	count=(temh*256+teml)*0.0625;
	ldi R16,<L38
	ldi R17,>L38
	rcall lpm32
	st -y,R19
	st -y,R18
	st -y,R17
	st -y,R16
	mov R18,R22
	clr R19
	ldi R16,256
	ldi R17,1
	rcall empy16s
	movw R2,R16
	mov R4,R20
	clr R5
	add R2,R4
	adc R3,R5
	movw R16,R2
	rcall int2fp
	st -y,R19
	st -y,R18
	st -y,R17
	st -y,R16
	rcall empy32f
	sts _count+1,R17
	sts _count,R16
	sts _count+2+1,R19
	sts _count+2,R18
	.dbline 94
;     return count;
	.dbline -2
L34:
	rcall pop_gset3
	.dbline 0 ; func end
	ret
	.dbsym r teml 20 c
	.dbsym r temh 22 c
	.dbsym r i 10 I
	.dbend
	.area bss(ram, con, rel)
	.dbfile C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc\18b20.c
_count::
	.blkb 4
	.dbsym e count _count D
	.area lit(rom, con, rel)
L38:
	.word 0x0,0x3d80
