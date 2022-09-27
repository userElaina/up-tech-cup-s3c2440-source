	.module Sensor.c
	.area data(ram, con, rel)
_CMD::
	.blkb 1
	.area idata
	.byte 0
	.area data(ram, con, rel)
	.dbfile C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc\Sensor.c
	.dbsym e CMD _CMD c
	.area text(rom, con, rel)
	.dbfile C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc\Sensor.c
	.dbfunc e GetSensorMessage _GetSensorMessage fV
	.area func_lit(rom, con, rel)
L145:
	.word `L6
	.word `L7
	.word `L25
	.word `L43
	.word `L44
	.word `L45
	.word `L46
	.word `L47
	.word `L65
	.word `L83
	.word `L101
	.word `L119
	.word `L124
	.word `L125
	.word `L126
	.word `L143
	.word `L144
	.area text(rom, con, rel)
	.dbfile C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc\Sensor.c
;        resultL -> y+20
;        resultH -> y+19
;     AH3020Flag -> R10
;         result -> y+0
;        MQ2Flag -> R20
;     RE200BFlag -> y+18
;       REEDFlag -> y+17
;       MoveFlag -> y+16
;      TouchFlag -> y+15
;      SoundFlag -> y+14
;              i -> R12,R13
;              j -> R14,R15
;              k -> R22,R23
;          param -> R22
	.even
_GetSensorMessage::
	rcall push_gset5
	mov R22,R16
	sbiw R28,21
	.dbline -1
	.dbline 6
; #include "Generic.h"
; 
; unsigned char CMD = 0x00;
; 
; void GetSensorMessage( unsigned char param )
; {
	.dbline 7
; 	float result = 0;
	ldi R16,<L2
	ldi R17,>L2
	rcall lpm32
	movw R30,R28
	std z+0,R16
	std z+1,R17
	std z+2,R18
	std z+3,R19
	.dbline 8
; 	unsigned char resultH = 0;
	clr R0
	std y+19,R0
	.dbline 9
; 	unsigned char resultL = 0;
	std y+20,R0
	.dbline 11
; 	unsigned int i,j,k;
; 	unsigned char SoundFlag = 0;
	std y+14,R0
	.dbline 12
; 	unsigned char TouchFlag = 0;
	std y+15,R0
	.dbline 13
; 	unsigned char MoveFlag = 0;
	std y+16,R0
	.dbline 14
; 	unsigned char REEDFlag = 0;
	std y+17,R0
	.dbline 15
; 	unsigned char RE200BFlag = 0;
	std y+18,R0
	.dbline 16
; 	unsigned char MQ2Flag = 0;
	clr R20
	.dbline 17
; 	unsigned char AH3020Flag = 0;
	clr R10
	.dbline 19
; 	
; 	switch(param)
	mov R2,R22
	clr R3
	std y+5,R3
	std y+4,R2
	movw R24,R2
	cpi R24,16
	ldi R30,0
	cpc R25,R30
	brge X9
	rjmp L4
X9:
	ldi R24,32
	cp R24,R2
	cpc R25,R3
	brge X10
	rjmp L4
X10:
	movw R18,R2
	ldi R16,2
	ldi R17,0
	rcall empy16s
	movw R30,R16
	ldi R24,<L145-32
	ldi R25,>L145-32
	add R30,R24
	adc R31,R25
	lpm R0,Z+
	lpm R1,Z
	movw R30,R0
	ijmp
X0:
	.dbline 20
; 	{
L6:
	.dbline 23
; 		//18b20温度传感器
; 		case Temperature18b20:
; 			{
	.dbline 24
; 				result = Get_Temperature();
	rcall _Get_Temperature
	movw R30,R28
	std z+0,R16
	std z+1,R17
	std z+2,R18
	std z+3,R19
	.dbline 25
; 				break;
	rjmp L4
L7:
	.dbline 30
; 			}
; 
; 		//MIC声音传感器
; 		case MIC:
; 			{
	.dbline 32
; 				
; 				SoundFlag = 0;
	clr R0
	std y+14,R0
	.dbline 33
; 				for( i = 0;(i<100)&&(SoundFlag == 0);i++ )
	clr R12
	clr R13
	rjmp L11
L8:
	.dbline 34
; 				{
	.dbline 35
; 					for(j = 0;(j<1000)&&(SoundFlag == 0);j++)
	clr R14
	clr R15
	rjmp L15
L12:
	.dbline 36
; 					{
	.dbline 37
; 						if( PINB&0x02 )
	sbis 0x16,1
	rjmp L16
	.dbline 38
; 						{
	.dbline 39
; 							k = 100;
	ldi R22,100
	ldi R23,0
L18:
	.dbline 40
L19:
	.dbline 40
; 							while(k--);
	movw R2,R22
	subi R22,1
	sbci R23,0
	tst R2
	brne L18
	tst R3
	brne L18
X1:
	.dbline 41
; 							if( PINB&0x02 )
	sbis 0x16,1
	rjmp L21
	.dbline 42
; 							{
	.dbline 43
; 								SoundFlag = 1;
	clr R0
	inc R0
	std y+14,R0
	.dbline 44
; 							}
L21:
	.dbline 45
; 						}
L16:
	.dbline 46
L13:
	.dbline 35
	movw R24,R14
	adiw R24,1
	movw R14,R24
L15:
	.dbline 35
	movw R24,R14
	cpi R24,232
	ldi R30,3
	cpc R25,R30
	brsh L23
	ldd R0,y+14
	tst R0
	breq L12
L23:
	.dbline 47
L9:
	.dbline 33
	movw R24,R12
	adiw R24,1
	movw R12,R24
L11:
	.dbline 33
	movw R24,R12
	cpi R24,100
	ldi R30,0
	cpc R25,R30
	brsh L24
	ldd R0,y+14
	tst R0
	breq L8
L24:
	.dbline 48
; 					}
; 				}
; 				result = SoundFlag;
	ldd R16,y+14
	clr R17
	rcall int2fp
	movw R30,R28
	std z+0,R16
	std z+1,R17
	std z+2,R18
	std z+3,R19
	.dbline 50
; 				//1有声，0无声
; 				break;
	rjmp L4
L25:
	.dbline 55
; 			}
; 
; 		//触摸板
; 		case TouchPad:
; 			{
	.dbline 56
; 				TouchFlag = 0;
	clr R0
	std y+15,R0
	.dbline 57
; 				for( i = 0;(i<100)&&(TouchFlag == 0);i++ )
	clr R12
	clr R13
	rjmp L29
L26:
	.dbline 58
; 				{
	.dbline 59
; 					for(j = 0;(j<1000)&&(TouchFlag == 0);j++)
	clr R14
	clr R15
	rjmp L33
L30:
	.dbline 60
; 					{
	.dbline 61
; 						if( PINB&0x10 )
	sbis 0x16,4
	rjmp L34
	.dbline 62
; 						{
	.dbline 63
; 							k = 100;
	ldi R22,100
	ldi R23,0
L36:
	.dbline 64
L37:
	.dbline 64
; 							while(k--);
	movw R2,R22
	subi R22,1
	sbci R23,0
	tst R2
	brne L36
	tst R3
	brne L36
X2:
	.dbline 65
; 							if( PINB&0x10 )
	sbis 0x16,4
	rjmp L39
	.dbline 66
; 							{
	.dbline 67
; 								TouchFlag = 1;
	clr R0
	inc R0
	std y+15,R0
	.dbline 68
; 							}
L39:
	.dbline 69
; 						}
L34:
	.dbline 70
L31:
	.dbline 59
	movw R24,R14
	adiw R24,1
	movw R14,R24
L33:
	.dbline 59
	movw R24,R14
	cpi R24,232
	ldi R30,3
	cpc R25,R30
	brsh L41
	ldd R0,y+15
	tst R0
	breq L30
L41:
	.dbline 71
L27:
	.dbline 57
	movw R24,R12
	adiw R24,1
	movw R12,R24
L29:
	.dbline 57
	movw R24,R12
	cpi R24,100
	ldi R30,0
	cpc R25,R30
	brsh L42
	ldd R0,y+15
	tst R0
	breq L26
L42:
	.dbline 72
; 					}
; 				}
; 				result = TouchFlag;
	ldd R16,y+15
	clr R17
	rcall int2fp
	movw R30,R28
	std z+0,R16
	std z+1,R17
	std z+2,R18
	std z+3,R19
	.dbline 74
; 				//1有接触，0无接触
; 				break;
	rjmp L4
L43:
	.dbline 79
; 			}
; 
; 		//热敏电阻，返回电压值
; 		case TemperatureResister:
; 			{
	.dbline 80
; 				result = StartADC(0);
	clr R16
	rcall _StartADC
	movw R30,R28
	std z+0,R16
	std z+1,R17
	std z+2,R18
	std z+3,R19
	.dbline 81
; 				break;
	rjmp L4
L44:
	.dbline 86
; 			}
; 
; 		//光敏电阻，返回电压值
; 		case LightResister:
; 			{
	.dbline 87
; 				result = StartADC(1);
	ldi R16,1
	rcall _StartADC
	movw R30,R28
	std z+0,R16
	std z+1,R17
	std z+2,R18
	std z+3,R19
	.dbline 88
; 				break;
	rjmp L4
L45:
	.dbline 93
; 			}
; 
; 		//差分电压，返回电压值
; 		case DifferentialVoltage:
; 			{
	.dbline 94
; 				result = StartADC(2);
	ldi R16,2
	rcall _StartADC
	movw R30,R28
	std z+0,R16
	std z+1,R17
	std z+2,R18
	std z+3,R19
	.dbline 95
; 				break;
	rjmp L4
L46:
	.dbline 100
; 			}
; 
; 		//霍尔线性传感器，返回电压值
; 		case AH3503:
; 			{
	.dbline 101
; 				result = StartADC(3);
	ldi R16,3
	rcall _StartADC
	movw R30,R28
	std z+0,R16
	std z+1,R17
	std z+2,R18
	std z+3,R19
	.dbline 102
; 				break;
	rjmp L4
L47:
	.dbline 107
; 			}
; 
; 		//机械抖动开关
; 		case MOVESW:
; 			{
	.dbline 108
; 				MoveFlag = 0;
	clr R0
	std y+16,R0
	.dbline 109
; 				for( i = 0;(i<100)&&(MoveFlag == 0);i++ )
	clr R12
	clr R13
	rjmp L51
L48:
	.dbline 110
; 				{
	.dbline 111
; 					for(j = 0;(j<1000)&&(MoveFlag == 0);j++)
	clr R14
	clr R15
	rjmp L55
L52:
	.dbline 112
; 					{
	.dbline 113
; 						if( !(PINC&0x10) )
	sbic 0x13,4
	rjmp L56
	.dbline 114
; 						{
	.dbline 115
; 							k = 100;
	ldi R22,100
	ldi R23,0
L58:
	.dbline 116
L59:
	.dbline 116
; 							while(k--);
	movw R2,R22
	subi R22,1
	sbci R23,0
	tst R2
	brne L58
	tst R3
	brne L58
X3:
	.dbline 117
; 							if( !(PINC&0x10) )
	sbic 0x13,4
	rjmp L61
	.dbline 118
; 							{
	.dbline 119
; 								MoveFlag = 1;
	clr R0
	inc R0
	std y+16,R0
	.dbline 120
; 							}
L61:
	.dbline 121
; 						}
L56:
	.dbline 122
L53:
	.dbline 111
	movw R24,R14
	adiw R24,1
	movw R14,R24
L55:
	.dbline 111
	movw R24,R14
	cpi R24,232
	ldi R30,3
	cpc R25,R30
	brsh L63
	ldd R0,y+16
	tst R0
	breq L52
L63:
	.dbline 123
L49:
	.dbline 109
	movw R24,R12
	adiw R24,1
	movw R12,R24
L51:
	.dbline 109
	movw R24,R12
	cpi R24,100
	ldi R30,0
	cpc R25,R30
	brsh L64
	ldd R0,y+16
	tst R0
	breq L48
L64:
	.dbline 124
; 					}
; 				}
; 				result = MoveFlag;
	ldd R16,y+16
	clr R17
	rcall int2fp
	movw R30,R28
	std z+0,R16
	std z+1,R17
	std z+2,R18
	std z+3,R19
	.dbline 126
; 				//1有抖动，0无抖动
; 				break;
	rjmp L4
L65:
	.dbline 131
; 			}
; 
; 		//干簧管，低电平有信号
; 		case REEDSW:
; 			{
	.dbline 132
; 				REEDFlag = 0;
	clr R0
	std y+17,R0
	.dbline 133
; 				for( i = 0;(i<100)&&(REEDFlag == 0);i++ )
	clr R12
	clr R13
	rjmp L69
L66:
	.dbline 134
; 				{
	.dbline 135
; 					for(j = 0;(j<1000)&&(REEDFlag == 0);j++)
	clr R14
	clr R15
	rjmp L73
L70:
	.dbline 136
; 					{
	.dbline 137
; 						if( !(PINC&0x20) )
	sbic 0x13,5
	rjmp L74
	.dbline 138
; 						{
	.dbline 139
; 							k = 100;
	ldi R22,100
	ldi R23,0
L76:
	.dbline 140
L77:
	.dbline 140
; 							while(k--);
	movw R2,R22
	subi R22,1
	sbci R23,0
	tst R2
	brne L76
	tst R3
	brne L76
X4:
	.dbline 141
; 							if( !(PINC&0x20) )
	sbic 0x13,5
	rjmp L79
	.dbline 142
; 							{
	.dbline 143
; 								REEDFlag = 1;
	clr R0
	inc R0
	std y+17,R0
	.dbline 144
; 							}
L79:
	.dbline 145
; 						}
L74:
	.dbline 146
L71:
	.dbline 135
	movw R24,R14
	adiw R24,1
	movw R14,R24
L73:
	.dbline 135
	movw R24,R14
	cpi R24,232
	ldi R30,3
	cpc R25,R30
	brsh L81
	ldd R0,y+17
	tst R0
	breq L70
L81:
	.dbline 147
L67:
	.dbline 133
	movw R24,R12
	adiw R24,1
	movw R12,R24
L69:
	.dbline 133
	movw R24,R12
	cpi R24,100
	ldi R30,0
	cpc R25,R30
	brsh L82
	ldd R0,y+17
	tst R0
	breq L66
L82:
	.dbline 148
; 					}
; 				}
; 				result = REEDFlag;
	ldd R16,y+17
	clr R17
	rcall int2fp
	movw R30,R28
	std z+0,R16
	std z+1,R17
	std z+2,R18
	std z+3,R19
	.dbline 150
; 				//1有磁变化，0无磁变化
; 				break;
	rjmp L4
L83:
	.dbline 155
; 			}
; 
; 		//热释红外传感器
; 		case RE200B:
; 			{
	.dbline 156
; 				RE200BFlag = 0;
	clr R0
	std y+18,R0
	.dbline 157
; 				for( i = 0;(i<100)&&(RE200BFlag == 0);i++ )
	clr R12
	clr R13
	rjmp L87
L84:
	.dbline 158
; 				{
	.dbline 159
; 					for(j = 0;(j<1000)&&(RE200BFlag == 0);j++)
	clr R14
	clr R15
	rjmp L91
L88:
	.dbline 160
; 					{
	.dbline 161
; 						if( PIND&0x04 )
	sbis 0x10,2
	rjmp L92
	.dbline 162
; 						{
	.dbline 163
; 							k = 100;
	ldi R22,100
	ldi R23,0
L94:
	.dbline 164
L95:
	.dbline 164
; 							while(k--);
	movw R2,R22
	subi R22,1
	sbci R23,0
	tst R2
	brne L94
	tst R3
	brne L94
X5:
	.dbline 165
; 							if( PIND&0x04 )
	sbis 0x10,2
	rjmp L97
	.dbline 166
; 							{
	.dbline 167
; 								RE200BFlag = 1;
	clr R0
	inc R0
	std y+18,R0
	.dbline 168
; 							}
L97:
	.dbline 170
; 							
; 						}
L92:
	.dbline 171
L89:
	.dbline 159
	movw R24,R14
	adiw R24,1
	movw R14,R24
L91:
	.dbline 159
	movw R24,R14
	cpi R24,232
	ldi R30,3
	cpc R25,R30
	brsh L99
	ldd R0,y+18
	tst R0
	breq L88
L99:
	.dbline 172
L85:
	.dbline 157
	movw R24,R12
	adiw R24,1
	movw R12,R24
L87:
	.dbline 157
	movw R24,R12
	cpi R24,100
	ldi R30,0
	cpc R25,R30
	brsh L100
	ldd R0,y+18
	tst R0
	breq L84
L100:
	.dbline 173
; 					}
; 				}
; 				result = RE200BFlag;
	ldd R16,y+18
	clr R17
	rcall int2fp
	movw R30,R28
	std z+0,R16
	std z+1,R17
	std z+2,R18
	std z+3,R19
	.dbline 175
; 				//1，0
; 				break;
	rjmp L4
L101:
	.dbline 180
; 			}
; 
; 		//广谱气体传感器
; 		case MQ2:
; 			{
	.dbline 181
; 				MQ2Flag = 0;
	clr R20
	.dbline 182
; 				for( i = 0;(i<100)&&(MQ2Flag == 0);i++ )
	clr R12
	clr R13
	rjmp L105
L102:
	.dbline 183
; 				{
	.dbline 184
; 					for(j = 0;(j<1000)&&(MQ2Flag == 0);j++)
	clr R14
	clr R15
	rjmp L109
L106:
	.dbline 185
; 					{
	.dbline 186
; 						if( PIND&0x08 )
	sbis 0x10,3
	rjmp L110
	.dbline 187
; 						{
	.dbline 188
; 							k = 100;
	ldi R22,100
	ldi R23,0
L112:
	.dbline 189
L113:
	.dbline 189
; 							while(k--);
	movw R2,R22
	subi R22,1
	sbci R23,0
	tst R2
	brne L112
	tst R3
	brne L112
X6:
	.dbline 190
; 							if( PIND&0x08 )
	sbis 0x10,3
	rjmp L115
	.dbline 191
; 							{
	.dbline 192
; 								MQ2Flag = 1;
	ldi R20,1
	.dbline 193
; 							}
L115:
	.dbline 194
; 						}
L110:
	.dbline 195
L107:
	.dbline 184
	movw R24,R14
	adiw R24,1
	movw R14,R24
L109:
	.dbline 184
	movw R24,R14
	cpi R24,232
	ldi R30,3
	cpc R25,R30
	brsh L117
	tst R20
	breq L106
L117:
	.dbline 196
L103:
	.dbline 182
	movw R24,R12
	adiw R24,1
	movw R12,R24
L105:
	.dbline 182
	movw R24,R12
	cpi R24,100
	ldi R30,0
	cpc R25,R30
	brsh L118
	tst R20
	breq L102
L118:
	.dbline 197
; 					}
; 				}
; 				result = MQ2Flag;
	mov R16,R20
	clr R17
	rcall int2fp
	movw R30,R28
	std z+0,R16
	std z+1,R17
	std z+2,R18
	std z+3,R19
	.dbline 199
; 				//1，0
; 				break;
	rjmp L4
L119:
	.dbline 204
; 			}
; 
; 		//红外对射开关（打开）
; 		case StartInfraredRadio:
; 			{
	.dbline 205
; 				UCSRB = 0x98;
	ldi R24,152
	out 0xa,R24
	.dbline 207
; 				
; 				StartCountHole();
	rcall _StartCountHole
	rjmp L121
L120:
	.dbline 209
	.dbline 210
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
	ldi R16,<L123
	ldi R17,>L123
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
	rcall add32f
	rcall _DisplayData
	.dbline 212
	in R24,0x18
	andi R24,251
	out 0x18,R24
	.dbline 215
	sbi 0x18,2
	.dbline 216
L121:
	.dbline 208
; 				while(1)
	rjmp L120
X7:
	.dbline 217
; 				{
; 					DisplayData(HoleCountH*256+TCNT0);
; 					//将从机的SS拉低
; 					PORTB&=0xFB;
; 	
; 					//将从机的SS拉高，表明发送完毕
; 					PORTB|=0x04;
; 				}
; 				UCSRB = 0x18;
	ldi R24,24
	out 0xa,R24
	.dbline 218
; 				break;
	rjmp L4
L124:
	.dbline 223
; 			}
; 			
; 		//红外对射开关（关闭）
; 		case FinishInfraredRadio:
; 			{
	.dbline 224
; 				FinishCountHole();
	rcall _FinishCountHole
	.dbline 225
; 				result = HoleCountH*256+TCNT0;
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
	ldi R16,<L123
	ldi R17,>L123
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
	rcall add32f
	movw R30,R28
	std z+0,R16
	std z+1,R17
	std z+2,R18
	std z+3,R19
	.dbline 226
; 				DisplayData(HoleCountH*256+TCNT0);
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
	ldi R16,<L123
	ldi R17,>L123
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
	rcall add32f
	rcall _DisplayData
	.dbline 228
; 				//将从机的SS拉低
; 				PORTB&=0xFB;
	in R24,0x18
	andi R24,251
	out 0x18,R24
	.dbline 230
; 				//将从机的SS拉高，表明发送完毕
; 				PORTB|=0x04;
	sbi 0x18,2
	.dbline 233
; 				
; 				//TCCR0 = 0x00;
; 				break;
	rjmp L4
L125:
	.dbline 238
; 			}
; 
; 		//湿度传感器HS1101
; 		case HS1101:
; 			{
	.dbline 239
; 				result=GetHS1101();
	rcall _GetHS1101
	movw R2,R16
	ldi R16,<L123
	ldi R17,>L123
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
	rcall add32f
	movw R30,R28
	std z+0,R16
	std z+1,R17
	std z+2,R18
	std z+3,R19
	.dbline 240
; 				break;
	rjmp L4
L126:
	.dbline 245
; 			}
; 
; 		//霍尔开关
; 		case AH3020:
; 			{
	.dbline 246
; 				AH3020Flag = 0;
	clr R10
	.dbline 247
; 				for( i = 0;(i<100)&&(AH3020Flag==0);i++ )
	clr R12
	clr R13
	rjmp L130
L127:
	.dbline 248
; 				{
	.dbline 249
; 					for(j = 0;j<1000;j++)
	clr R14
	clr R15
	rjmp L134
L131:
	.dbline 250
; 					{
	.dbline 251
; 						if( !(PIND&0x40) )
	sbic 0x10,6
	rjmp L135
	.dbline 252
; 						{
	.dbline 253
; 							k = 100;
	ldi R22,100
	ldi R23,0
L137:
	.dbline 254
L138:
	.dbline 254
; 							while(k--);
	movw R2,R22
	subi R22,1
	sbci R23,0
	tst R2
	brne L137
	tst R3
	brne L137
X8:
	.dbline 255
; 							if( !(PIND&0x40) )
	sbic 0x10,6
	rjmp L140
	.dbline 256
; 							{
	.dbline 257
; 								AH3020Flag = 1;
	clr R10
	inc R10
	.dbline 258
; 							}
L140:
	.dbline 259
; 						}
L135:
	.dbline 260
L132:
	.dbline 249
	movw R24,R14
	adiw R24,1
	movw R14,R24
L134:
	.dbline 249
	movw R24,R14
	cpi R24,232
	ldi R30,3
	cpc R25,R30
	brlo L131
	.dbline 261
L128:
	.dbline 247
	movw R24,R12
	adiw R24,1
	movw R12,R24
L130:
	.dbline 247
	movw R24,R12
	cpi R24,100
	ldi R30,0
	cpc R25,R30
	brsh L142
	tst R10
	breq L127
L142:
	.dbline 262
; 					}
; 				}
; 				result = AH3020Flag;
	mov R16,R10
	clr R17
	rcall int2fp
	movw R30,R28
	std z+0,R16
	std z+1,R17
	std z+2,R18
	std z+3,R19
	.dbline 264
; 				//1，0
; 				break;
	rjmp L4
L143:
	.dbline 269
; 			}
; 
; 		//电压输入
; 		case VoltageInput:
; 			{
	.dbline 270
; 				result = StartADC(6);
	ldi R16,6
	rcall _StartADC
	movw R30,R28
	std z+0,R16
	std z+1,R17
	std z+2,R18
	std z+3,R19
	.dbline 271
; 				break;
	rjmp L4
L144:
	.dbline 276
; 			}
; 
; 		//电流输入
; 		case CurrentInput:
; 			{
	.dbline 277
; 				result = StartADC(7);
	ldi R16,7
	rcall _StartADC
	movw R30,R28
	std z+0,R16
	std z+1,R17
	std z+2,R18
	std z+3,R19
	.dbline 278
; 				break;
	.dbline 281
; 			}
; 		default:
; 			break;
L4:
	.dbline 283
; 	}
; 	DisplayData(result);
	movw R30,R28
	ldd R16,z+0
	ldd R17,z+1
	ldd R18,z+2
	ldd R19,z+3
	rcall _DisplayData
	.dbline 286
; 	
; 	//整数显示
; 	if((result>=10)&&(result<100))
	movw R30,R28
	ldd R2,z+0
	ldd R3,z+1
	ldd R4,z+2
	ldd R5,z+3
	st -y,R5
	st -y,R4
	st -y,R3
	st -y,R2
	ldi R16,<L149
	ldi R17,>L149
	rcall lpm32
	st -y,R19
	st -y,R18
	st -y,R17
	st -y,R16
	rcall cmp32f
	brge X11
	rjmp L147
X11:
	movw R30,R28
	ldd R2,z+0
	ldd R3,z+1
	ldd R4,z+2
	ldd R5,z+3
	st -y,R5
	st -y,R4
	st -y,R3
	st -y,R2
	ldi R16,<L150
	ldi R17,>L150
	rcall lpm32
	st -y,R19
	st -y,R18
	st -y,R17
	st -y,R16
	rcall cmp32f
	brlt X12
	rjmp L147
X12:
	.dbline 287
; 	{
	.dbline 288
; 		resultH = (unsigned char)(result/10);
	movw R30,R28
	ldd R2,z+0
	ldd R3,z+1
	ldd R4,z+2
	ldd R5,z+3
	st -y,R5
	st -y,R4
	st -y,R3
	st -y,R2
	ldi R16,<L149
	ldi R17,>L149
	rcall lpm32
	st -y,R19
	st -y,R18
	st -y,R17
	st -y,R16
	rcall div32f
	movw R30,R28
	std z+6,R16
	std z+7,R17
	std z+8,R18
	std z+9,R19
	movw R30,R28
	ldd R2,z+6
	ldd R3,z+7
	ldd R4,z+8
	ldd R5,z+9
	st -y,R5
	st -y,R4
	st -y,R3
	st -y,R2
	ldi R16,<L153
	ldi R17,>L153
	rcall lpm32
	st -y,R19
	st -y,R18
	st -y,R17
	st -y,R16
	rcall cmp32f
	brlt L151
	movw R30,R28
	ldd R2,z+6
	ldd R3,z+7
	ldd R4,z+8
	ldd R5,z+9
	st -y,R5
	st -y,R4
	st -y,R3
	st -y,R2
	ldi R16,<L153
	ldi R17,>L153
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
	rjmp L152
L151:
	movw R30,R28
	ldd R16,z+6
	ldd R17,z+7
	ldd R18,z+8
	ldd R19,z+9
	rcall fp2int
	movw R10,R16
L152:
	std y+19,R10
	.dbline 289
; 		resultL = (unsigned char)(result-resultH*10);
	movw R30,R28
	ldd R2,z+0
	ldd R3,z+1
	ldd R4,z+2
	ldd R5,z+3
	st -y,R5
	st -y,R4
	st -y,R3
	st -y,R2
	ldi R24,10
	mov R0,R10
	mul R24,R0
	movw R16,R0
	rcall int2fp
	st -y,R19
	st -y,R18
	st -y,R17
	st -y,R16
	rcall sub32f
	movw R30,R28
	std z+10,R16
	std z+11,R17
	std z+12,R18
	std z+13,R19
	movw R30,R28
	ldd R2,z+10
	ldd R3,z+11
	ldd R4,z+12
	ldd R5,z+13
	st -y,R5
	st -y,R4
	st -y,R3
	st -y,R2
	ldi R16,<L153
	ldi R17,>L153
	rcall lpm32
	st -y,R19
	st -y,R18
	st -y,R17
	st -y,R16
	rcall cmp32f
	brlt L154
	movw R30,R28
	ldd R2,z+10
	ldd R3,z+11
	ldd R4,z+12
	ldd R5,z+13
	st -y,R5
	st -y,R4
	st -y,R3
	st -y,R2
	ldi R16,<L153
	ldi R17,>L153
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
	rjmp L155
L154:
	movw R30,R28
	ldd R16,z+10
	ldd R17,z+11
	ldd R18,z+12
	ldd R19,z+13
	rcall fp2int
	movw R10,R16
L155:
	std y+20,R10
	.dbline 291
; 		
; 		SendBuffer[0] = 0xBB;
	ldi R24,187
	sts _SendBuffer,R24
	.dbline 292
; 		SendBuffer[1] = 0xFF;
	ldi R24,255
	sts _SendBuffer+1,R24
	.dbline 293
; 		SendBuffer[2] = resultH;
	ldd R0,y+19
	sts _SendBuffer+2,R0
	.dbline 294
; 		SendBuffer[3] = resultL;
	mov R0,R10
	sts _SendBuffer+3,R0
	.dbline 295
; 		SendBuffer[4] = 0x00;
	clr R2
	sts _SendBuffer+4,R2
	.dbline 296
; 		SendBuffer[6] = 0xFF;
	sts _SendBuffer+6,R24
	.dbline 297
; 	}
	rjmp L148
L147:
	.dbline 299
; 	//浮点数显示
; 	else if((result>=0)&&(result<10))
	movw R30,R28
	ldd R2,z+0
	ldd R3,z+1
	ldd R4,z+2
	ldd R5,z+3
	st -y,R5
	st -y,R4
	st -y,R3
	st -y,R2
	ldi R16,<L2
	ldi R17,>L2
	rcall lpm32
	st -y,R19
	st -y,R18
	st -y,R17
	st -y,R16
	rcall cmp32f
	brge X13
	rjmp L161
X13:
	movw R30,R28
	ldd R2,z+0
	ldd R3,z+1
	ldd R4,z+2
	ldd R5,z+3
	st -y,R5
	st -y,R4
	st -y,R3
	st -y,R2
	ldi R16,<L149
	ldi R17,>L149
	rcall lpm32
	st -y,R19
	st -y,R18
	st -y,R17
	st -y,R16
	rcall cmp32f
	brlt X14
	rjmp L161
X14:
	.dbline 300
; 	{
	.dbline 301
; 		resultH = (unsigned char)result;
	movw R30,R28
	ldd R2,z+0
	ldd R3,z+1
	ldd R4,z+2
	ldd R5,z+3
	st -y,R5
	st -y,R4
	st -y,R3
	st -y,R2
	ldi R16,<L153
	ldi R17,>L153
	rcall lpm32
	st -y,R19
	st -y,R18
	st -y,R17
	st -y,R16
	rcall cmp32f
	brlt L163
	movw R30,R28
	ldd R2,z+0
	ldd R3,z+1
	ldd R4,z+2
	ldd R5,z+3
	st -y,R5
	st -y,R4
	st -y,R3
	st -y,R2
	ldi R16,<L153
	ldi R17,>L153
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
	rjmp L164
L163:
	movw R30,R28
	ldd R16,z+0
	ldd R17,z+1
	ldd R18,z+2
	ldd R19,z+3
	rcall fp2int
	movw R10,R16
L164:
	std y+19,R10
	.dbline 302
; 		resultL = (unsigned char)(result*10)%10;
	ldi R16,<L149
	ldi R17,>L149
	rcall lpm32
	st -y,R19
	st -y,R18
	st -y,R17
	st -y,R16
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
	std z+6,R16
	std z+7,R17
	std z+8,R18
	std z+9,R19
	movw R30,R28
	ldd R2,z+6
	ldd R3,z+7
	ldd R4,z+8
	ldd R5,z+9
	st -y,R5
	st -y,R4
	st -y,R3
	st -y,R2
	ldi R16,<L153
	ldi R17,>L153
	rcall lpm32
	st -y,R19
	st -y,R18
	st -y,R17
	st -y,R16
	rcall cmp32f
	brlt L165
	movw R30,R28
	ldd R2,z+6
	ldd R3,z+7
	ldd R4,z+8
	ldd R5,z+9
	st -y,R5
	st -y,R4
	st -y,R3
	st -y,R2
	ldi R16,<L153
	ldi R17,>L153
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
	rjmp L166
L165:
	movw R30,R28
	ldd R16,z+6
	ldd R17,z+7
	ldd R18,z+8
	ldd R19,z+9
	rcall fp2int
	movw R10,R16
L166:
	ldi R17,10
	mov R16,R10
	rcall mod8u
	std y+20,R16
	.dbline 303
; 		SendBuffer[0] = 0xBB;
	ldi R24,187
	sts _SendBuffer,R24
	.dbline 304
; 		SendBuffer[1] = 0xFF;
	ldi R24,255
	sts _SendBuffer+1,R24
	.dbline 305
; 		SendBuffer[2] = resultH;
	ldd R0,y+19
	sts _SendBuffer+2,R0
	.dbline 306
; 		SendBuffer[3] = resultL;
	mov R0,R16
	sts _SendBuffer+3,R0
	.dbline 307
; 		SendBuffer[4] = 0x01;
	ldi R24,1
	sts _SendBuffer+4,R24
	.dbline 308
; 		SendBuffer[6] = 0xFF;
	ldi R24,255
	sts _SendBuffer+6,R24
	.dbline 309
; 	}
	rjmp L162
L161:
	.dbline 311
; 	else
; 	{
	.dbline 312
; 	 	SendBuffer[0] = 0xBB;
	ldi R24,187
	sts _SendBuffer,R24
	.dbline 313
; 		SendBuffer[1] = 0xFF;
	ldi R24,255
	sts _SendBuffer+1,R24
	.dbline 314
; 		SendBuffer[2] = 0xFF;
	sts _SendBuffer+2,R24
	.dbline 315
; 		SendBuffer[3] = 0xFF;
	sts _SendBuffer+3,R24
	.dbline 316
; 		SendBuffer[4] = 0x00;
	clr R2
	sts _SendBuffer+4,R2
	.dbline 317
; 		SendBuffer[6] = 0xFF;
	sts _SendBuffer+6,R24
	.dbline 318
; 	}
L162:
L148:
	.dbline 319
; 	UARTSendAMessage(SendBuffer,7);
	ldi R18,7
	ldi R19,0
	ldi R16,<_SendBuffer
	ldi R17,>_SendBuffer
	rcall _UARTSendAMessage
	.dbline -2
L1:
	adiw R28,21
	rcall pop_gset5
	.dbline 0 ; func end
	ret
	.dbsym l resultL 20 c
	.dbsym l resultH 19 c
	.dbsym r AH3020Flag 10 c
	.dbsym l result 0 D
	.dbsym r MQ2Flag 20 c
	.dbsym l RE200BFlag 18 c
	.dbsym l REEDFlag 17 c
	.dbsym l MoveFlag 16 c
	.dbsym l TouchFlag 15 c
	.dbsym l SoundFlag 14 c
	.dbsym r i 12 i
	.dbsym r j 14 i
	.dbsym r k 22 i
	.dbsym r param 22 c
	.dbend
	.area lit(rom, con, rel)
L153:
	.word 0x0,0x4700
L150:
	.word 0x0,0x42c8
L149:
	.word 0x0,0x4120
L123:
	.word 0x0,0x4000
L2:
	.word 0x0,0x0
