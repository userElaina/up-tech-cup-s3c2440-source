	.module SPI.c
	.area data(ram, con, rel)
_CharacterTable::
	.blkb 2
	.area idata
	.byte 192,249
	.area data(ram, con, rel)
	.blkb 2
	.area idata
	.byte 164,176
	.area data(ram, con, rel)
	.blkb 2
	.area idata
	.byte 153,146
	.area data(ram, con, rel)
	.blkb 2
	.area idata
	.byte 130,248
	.area data(ram, con, rel)
	.blkb 2
	.area idata
	.byte 128,144
	.area data(ram, con, rel)
	.dbfile C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc\SPI.c
	.dbsym e CharacterTable _CharacterTable A[10:10]c
	.area text(rom, con, rel)
	.dbfile C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc\SPI.c
	.dbfunc e SPI_MasterInit _SPI_MasterInit fV
	.even
_SPI_MasterInit::
	.dbline -1
	.dbline 10
; #include "Generic.h"
; #include "math.h"
; //字符表
; unsigned char CharacterTable[10] =
; 	{ 0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90 };
; //0~9,若要显示小数点，则可表示为CharacterTable[i] & 0x7F
; 
; 
; void SPI_MasterInit(void)
; {
	.dbline 12
; 
; 	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);
	ldi R24,81
	out 0xd,R24
	.dbline 15
; 	//使能SPI 主机模式，设置时钟速率为fck/16
; 	
; 	SPSR = 0x00;
	clr R2
	out 0xe,R2
	.dbline -2
L1:
	.dbline 0 ; func end
	ret
	.dbend
	.dbfunc e SPI_MasterTransmitaByte _SPI_MasterTransmitaByte fV
;          cData -> R16
	.even
_SPI_MasterTransmitaByte::
	.dbline -1
	.dbline 19
; }
; 
; void SPI_MasterTransmitaByte(unsigned char cData)
; {
	.dbline 21
; 	//启动数据传输
; 	SPDR = cData;
	out 0xf,R16
L3:
	.dbline 24
L4:
	.dbline 24
; 
; 	//等待传输结束
; 	while(!(SPSR&(1<<SPIF)));
	sbis 0xe,7
	rjmp L3
	.dbline -2
L2:
	.dbline 0 ; func end
	ret
	.dbsym r cData 16 c
	.dbend
	.dbfunc e DisplayData _DisplayData fV
;          dataL -> R22
;          dataH -> R20
;           data -> y+14
	.even
_DisplayData::
	rcall push_arg4
	rcall push_gset3
	sbiw R28,8
	.dbline -1
	.dbline 28
; }
; 
; void DisplayData(float data)
; {
	.dbline 31
; 	unsigned char dataH,dataL;
; 	
; 	data+=0.001;//特殊处理，防止浮点运算导致结果减小0.1
	movw R30,R28
	ldd R2,z+14
	ldd R3,z+15
	ldd R4,z+16
	ldd R5,z+17
	st -y,R5
	st -y,R4
	st -y,R3
	st -y,R2
	ldi R16,<L7
	ldi R17,>L7
	rcall lpm32
	st -y,R19
	st -y,R18
	st -y,R17
	st -y,R16
	rcall add32f
	movw R30,R28
	std z+14,R16
	std z+15,R17
	std z+16,R18
	std z+17,R19
	.dbline 34
; 	
; 	//整数显示
; 	if((data>=10)&&(data<100))
	movw R30,R28
	ldd R2,z+14
	ldd R3,z+15
	ldd R4,z+16
	ldd R5,z+17
	st -y,R5
	st -y,R4
	st -y,R3
	st -y,R2
	ldi R16,<L10
	ldi R17,>L10
	rcall lpm32
	st -y,R19
	st -y,R18
	st -y,R17
	st -y,R16
	rcall cmp32f
	brge X0
	rjmp L8
X0:
	movw R30,R28
	ldd R2,z+14
	ldd R3,z+15
	ldd R4,z+16
	ldd R5,z+17
	st -y,R5
	st -y,R4
	st -y,R3
	st -y,R2
	ldi R16,<L11
	ldi R17,>L11
	rcall lpm32
	st -y,R19
	st -y,R18
	st -y,R17
	st -y,R16
	rcall cmp32f
	brlt X1
	rjmp L8
X1:
	.dbline 35
; 	{
	.dbline 36
; 		dataH = (unsigned char)(data/10);
	movw R30,R28
	ldd R2,z+14
	ldd R3,z+15
	ldd R4,z+16
	ldd R5,z+17
	st -y,R5
	st -y,R4
	st -y,R3
	st -y,R2
	ldi R16,<L10
	ldi R17,>L10
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
	movw R30,R28
	ldd R2,z+0
	ldd R3,z+1
	ldd R4,z+2
	ldd R5,z+3
	st -y,R5
	st -y,R4
	st -y,R3
	st -y,R2
	ldi R16,<L14
	ldi R17,>L14
	rcall lpm32
	st -y,R19
	st -y,R18
	st -y,R17
	st -y,R16
	rcall cmp32f
	brlt L12
	movw R30,R28
	ldd R2,z+0
	ldd R3,z+1
	ldd R4,z+2
	ldd R5,z+3
	st -y,R5
	st -y,R4
	st -y,R3
	st -y,R2
	ldi R16,<L14
	ldi R17,>L14
	rcall lpm32
	st -y,R19
	st -y,R18
	st -y,R17
	st -y,R16
	rcall sub32f
	rcall fp2int
	movw R20,R16
	subi R20,0  ; offset = 32768
	sbci R21,128
	rjmp L13
L12:
	movw R30,R28
	ldd R16,z+0
	ldd R17,z+1
	ldd R18,z+2
	ldd R19,z+3
	rcall fp2int
	movw R20,R16
L13:
	.dbline 37
; 		dataL = (unsigned char)(data-dataH*10);
	movw R30,R28
	ldd R2,z+14
	ldd R3,z+15
	ldd R4,z+16
	ldd R5,z+17
	st -y,R5
	st -y,R4
	st -y,R3
	st -y,R2
	ldi R24,10
	mul R24,R20
	movw R16,R0
	rcall int2fp
	st -y,R19
	st -y,R18
	st -y,R17
	st -y,R16
	rcall sub32f
	movw R30,R28
	std z+4,R16
	std z+5,R17
	std z+6,R18
	std z+7,R19
	movw R30,R28
	ldd R2,z+4
	ldd R3,z+5
	ldd R4,z+6
	ldd R5,z+7
	st -y,R5
	st -y,R4
	st -y,R3
	st -y,R2
	ldi R16,<L14
	ldi R17,>L14
	rcall lpm32
	st -y,R19
	st -y,R18
	st -y,R17
	st -y,R16
	rcall cmp32f
	brlt L15
	movw R30,R28
	ldd R2,z+4
	ldd R3,z+5
	ldd R4,z+6
	ldd R5,z+7
	st -y,R5
	st -y,R4
	st -y,R3
	st -y,R2
	ldi R16,<L14
	ldi R17,>L14
	rcall lpm32
	st -y,R19
	st -y,R18
	st -y,R17
	st -y,R16
	rcall sub32f
	rcall fp2int
	movw R22,R16
	subi R22,0  ; offset = 32768
	sbci R23,128
	rjmp L16
L15:
	movw R30,R28
	ldd R16,z+4
	ldd R17,z+5
	ldd R18,z+6
	ldd R19,z+7
	rcall fp2int
	movw R22,R16
L16:
	.dbline 38
; 		SPI_MasterTransmitaByte(CharacterTable[dataL]);
	ldi R24,<_CharacterTable
	ldi R25,>_CharacterTable
	mov R30,R22
	clr R31
	add R30,R24
	adc R31,R25
	ldd R16,z+0
	rcall _SPI_MasterTransmitaByte
	.dbline 39
; 		SPI_MasterTransmitaByte(CharacterTable[dataH]);
	ldi R24,<_CharacterTable
	ldi R25,>_CharacterTable
	mov R30,R20
	clr R31
	add R30,R24
	adc R31,R25
	ldd R16,z+0
	rcall _SPI_MasterTransmitaByte
	.dbline 40
; 	}
	rjmp L9
L8:
	.dbline 42
; 	//浮点数显示
; 	else if((data>=0)&&(data<10))
	movw R30,R28
	ldd R2,z+14
	ldd R3,z+15
	ldd R4,z+16
	ldd R5,z+17
	st -y,R5
	st -y,R4
	st -y,R3
	st -y,R2
	ldi R16,<L19
	ldi R17,>L19
	rcall lpm32
	st -y,R19
	st -y,R18
	st -y,R17
	st -y,R16
	rcall cmp32f
	brge X2
	rjmp L17
X2:
	movw R30,R28
	ldd R2,z+14
	ldd R3,z+15
	ldd R4,z+16
	ldd R5,z+17
	st -y,R5
	st -y,R4
	st -y,R3
	st -y,R2
	ldi R16,<L10
	ldi R17,>L10
	rcall lpm32
	st -y,R19
	st -y,R18
	st -y,R17
	st -y,R16
	rcall cmp32f
	brlt X3
	rjmp L17
X3:
	.dbline 43
; 	{
	.dbline 44
; 		dataH = (unsigned char)data;
	movw R30,R28
	ldd R2,z+14
	ldd R3,z+15
	ldd R4,z+16
	ldd R5,z+17
	st -y,R5
	st -y,R4
	st -y,R3
	st -y,R2
	ldi R16,<L14
	ldi R17,>L14
	rcall lpm32
	st -y,R19
	st -y,R18
	st -y,R17
	st -y,R16
	rcall cmp32f
	brlt L20
	movw R30,R28
	ldd R2,z+14
	ldd R3,z+15
	ldd R4,z+16
	ldd R5,z+17
	st -y,R5
	st -y,R4
	st -y,R3
	st -y,R2
	ldi R16,<L14
	ldi R17,>L14
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
	movw R10,R24
	rjmp L21
L20:
	movw R30,R28
	ldd R16,z+14
	ldd R17,z+15
	ldd R18,z+16
	ldd R19,z+17
	rcall fp2int
	movw R10,R16
L21:
	mov R20,R10
	.dbline 45
; 		dataL = (unsigned char)(data*10)%10;
	ldi R16,<L10
	ldi R17,>L10
	rcall lpm32
	st -y,R19
	st -y,R18
	st -y,R17
	st -y,R16
	movw R30,R28
 ; stack offset 4
	ldd R2,z+18
	ldd R3,z+19
	ldd R4,z+20
	ldd R5,z+21
	st -y,R5
	st -y,R4
	st -y,R3
	st -y,R2
	rcall empy32f
	movw R30,R28
	std z+0,R16
	std z+1,R17
	std z+2,R18
	std z+3,R19
	movw R30,R28
	ldd R2,z+0
	ldd R3,z+1
	ldd R4,z+2
	ldd R5,z+3
	st -y,R5
	st -y,R4
	st -y,R3
	st -y,R2
	ldi R16,<L14
	ldi R17,>L14
	rcall lpm32
	st -y,R19
	st -y,R18
	st -y,R17
	st -y,R16
	rcall cmp32f
	brlt L22
	movw R30,R28
	ldd R2,z+0
	ldd R3,z+1
	ldd R4,z+2
	ldd R5,z+3
	st -y,R5
	st -y,R4
	st -y,R3
	st -y,R2
	ldi R16,<L14
	ldi R17,>L14
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
	movw R10,R24
	rjmp L23
L22:
	movw R30,R28
	ldd R16,z+0
	ldd R17,z+1
	ldd R18,z+2
	ldd R19,z+3
	rcall fp2int
	movw R10,R16
L23:
	ldi R17,10
	mov R16,R10
	rcall mod8u
	mov R22,R16
	.dbline 46
; 		SPI_MasterTransmitaByte(CharacterTable[dataL]);
	ldi R24,<_CharacterTable
	ldi R25,>_CharacterTable
	mov R30,R22
	clr R31
	add R30,R24
	adc R31,R25
	ldd R16,z+0
	rcall _SPI_MasterTransmitaByte
	.dbline 47
; 		SPI_MasterTransmitaByte(CharacterTable[dataH]&0x7F);
	ldi R24,<_CharacterTable
	ldi R25,>_CharacterTable
	mov R30,R20
	clr R31
	add R30,R24
	adc R31,R25
	ldd R16,z+0
	andi R16,127
	rcall _SPI_MasterTransmitaByte
	.dbline 48
; 	}
	rjmp L18
L17:
	.dbline 50
; 	else
; 	{
	.dbline 51
; 	 	SPI_MasterTransmitaByte(0x8E);//显示F
	ldi R16,142
	rcall _SPI_MasterTransmitaByte
	.dbline 52
; 		SPI_MasterTransmitaByte(0x8E);//显示F
	ldi R16,142
	rcall _SPI_MasterTransmitaByte
	.dbline 53
; 	}
L18:
L9:
	.dbline 55
; 	//将从机的SS拉低
; 	PORTB&=0xFB;
	in R24,0x18
	andi R24,251
	out 0x18,R24
	.dbline 57
; 	//将从机的SS拉高，表明发送完毕
; 	PORTB|=0x04;
	sbi 0x18,2
	.dbline -2
L6:
	adiw R28,8
	rcall pop_gset3
	adiw R28,4
	.dbline 0 ; func end
	ret
	.dbsym r dataL 22 c
	.dbsym r dataH 20 c
	.dbsym l data 14 D
	.dbend
	.area lit(rom, con, rel)
L19:
	.word 0x0,0x0
L14:
	.word 0x0,0x4700
L11:
	.word 0x0,0x42c8
L10:
	.word 0x0,0x4120
L7:
	.word 0x126f,0x3a83
