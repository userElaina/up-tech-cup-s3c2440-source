	.module UART.c
	.area text(rom, con, rel)
	.dbfile C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc\UART.c
	.dbfunc e USART_Init _USART_Init fV
	.even
_USART_Init::
	.dbline -1
	.dbline 10
; #include "generic.h"
; 
; #define FOSC 7372800
; #define BAUDRATE                115200
; #define UART_WaitForComplete()  while(!(UCSRA&(1<<UDRE)))
; 
; unsigned char SendBuffer[7];
; 
; void USART_Init(void)
; {
	.dbline 11
; 	UCSRB = 0x00;//disable while setting baud rate
	clr R2
	out 0xa,R2
	.dbline 12
; 	UCSRA = 0x00;
	out 0xb,R2
	.dbline 14
; 	//UCSRC = 0x86;
;  	UCSRC = BIT(URSEL) | 0x06;
	ldi R24,134
	out 0x20,R24
	.dbline 17
; 	//UBRR = (unsigned int)( FOSC/(16*BAUDRATE) - 1 );
; 	// 设置波特率
; 	UBRRL = 0x03;//set baud rate lo
	ldi R24,3
	out 0x9,R24
	.dbline 18
;  	UBRRH = 0x00;//set baud rate hi
	out 0x20,R2
	.dbline 19
;  	UCSRB = 0x18;
	ldi R24,24
	out 0xa,R24
	.dbline -2
L1:
	.dbline 0 ; func end
	ret
	.dbend
	.dbfunc e UARTSendaByte _UARTSendaByte fV
; cTransmitValue -> R16
	.even
_UARTSendaByte::
	.dbline -1
	.dbline 23
; }
; 
; void UARTSendaByte(unsigned char cTransmitValue)
; {
L3:
	.dbline 24
L4:
	.dbline 24
; 	UART_WaitForComplete();
	sbis 0xb,5
	rjmp L3
	.dbline 25
; 	UDR = cTransmitValue;
	out 0xc,R16
	.dbline -2
L2:
	.dbline 0 ; func end
	ret
	.dbsym r cTransmitValue 16 c
	.dbend
	.dbfunc e UARTSendAMessage _UARTSendAMessage fV
;              i -> R20,R21
;            num -> R22,R23
;     SendBuffer -> R10,R11
	.even
_UARTSendAMessage::
	rcall push_gset3
	movw R22,R18
	movw R10,R16
	.dbline -1
	.dbline 29
; }
; 
; void UARTSendAMessage(unsigned char *SendBuffer, unsigned int num)
; {
	.dbline 31
;  	unsigned int i;
; 	SendBuffer[5] = 0x00;
	clr R2
	movw R30,R10
	std z+5,R2
	.dbline 32
; 	UARTSendaByte(SendBuffer[0]);
	movw R30,R10
	ldd R16,z+0
	rcall _UARTSendaByte
	.dbline 33
; 	UARTSendaByte(SendBuffer[1]);
	movw R30,R10
	ldd R16,z+1
	rcall _UARTSendaByte
	.dbline 34
; 	for( i = 2; i < num-2; i++ )
	ldi R20,2
	ldi R21,0
	rjmp L10
L7:
	.dbline 35
	.dbline 36
	movw R30,R20
	add R30,R10
	adc R31,R11
	ldd R16,z+0
	rcall _UARTSendaByte
	.dbline 37
	movw R24,R22
	sbiw R24,2
	add R24,R10
	adc R25,R11
	movw R30,R20
	add R30,R10
	adc R31,R11
	ldd R4,z+0
	movw R30,R24
	ldd R5,z+0
	eor R5,R4
	std z+0,R5
	.dbline 38
L8:
	.dbline 34
	subi R20,255  ; offset = 1
	sbci R21,255
L10:
	.dbline 34
	movw R24,R22
	sbiw R24,2
	cp R20,R24
	cpc R21,R25
	brlo L7
	.dbline 39
; 	{
; 	 	UARTSendaByte(SendBuffer[i]);
; 		SendBuffer[num-2]^=SendBuffer[i];
; 	}
; 	UARTSendaByte(SendBuffer[num-2]);
	movw R30,R22
	sbiw R30,2
	add R30,R10
	adc R31,R11
	ldd R16,z+0
	rcall _UARTSendaByte
	.dbline 40
; 	UARTSendaByte(SendBuffer[num-1]);
	movw R30,R22
	sbiw R30,1
	add R30,R10
	adc R31,R11
	ldd R16,z+0
	rcall _UARTSendaByte
	.dbline -2
L6:
	rcall pop_gset3
	.dbline 0 ; func end
	ret
	.dbsym r i 20 i
	.dbsym r num 22 i
	.dbsym r SendBuffer 10 pc
	.dbend
	.dbfunc e UARTRcvaByte _UARTRcvaByte fc
	.even
_UARTRcvaByte::
	.dbline -1
	.dbline 45
; }
; 
; 
; unsigned char UARTRcvaByte(void)
; {
L12:
	.dbline 47
L13:
	.dbline 47
;    // 等待接收数据
;    while ( !(UCSRA & (1<<RXC)) );
	sbis 0xb,7
	rjmp L12
	.dbline 49
;    // 从缓冲器中获取并返回数据
;    return UDR;
	in R16,0xc
	.dbline -2
L11:
	.dbline 0 ; func end
	ret
	.dbend
	.dbfunc e WaitForCMD _WaitForCMD fc
;              k -> R20,R21
;            CMD -> R20,R21
;        rcvByte -> R22
	.even
_WaitForCMD::
	rcall push_gset2
	.dbline -1
	.dbline 53
; }
; 
; unsigned char WaitForCMD(void)
; {
	.dbline 54
;  	unsigned char rcvByte = 0x00;
	clr R22
	.dbline 55
; 	unsigned int k = 0;
	clr R20
	clr R21
	.dbline 56
; 	unsigned CMD = 0x00;
	rjmp L17
L16:
	.dbline 67
; 	/*
; 	//显示0xAA表示等待命令状态
; 	SPI_MasterTransmitaByte(0x88);//显示A
; 	SPI_MasterTransmitaByte(0x88);//显示A
; 		
; 	PORTB&=0xFB;
; 	PORTB|=0x04;
; 	*/
; 	//等待帧头
; 	while(1)
; 	{
	.dbline 68
; 		rcvByte = UARTRcvaByte();
	rcall _UARTRcvaByte
	mov R22,R16
	.dbline 69
; 		{
	.dbline 70
; 		 	if( rcvByte == 0xAA )
	cpi R16,170
	brne L19
	.dbline 71
; 			{
	.dbline 72
; 			 	rcvByte = UARTRcvaByte();
	rcall _UARTRcvaByte
	mov R22,R16
	.dbline 73
; 				{
	.dbline 74
; 				 	if( rcvByte == 0xFF )
	cpi R16,255
	brne L21
	.dbline 75
; 					{
	.dbline 80
; 						/*SPI_MasterTransmitaByte(0x86);//显示E
; 						SPI_MasterTransmitaByte(0x86);//显示E
; 						PORTB&=0xFB;
; 						PORTB|=0x04;*/
; 						break;
	rjmp L18
L21:
	.dbline 82
; 					}
; 				}
	.dbline 83
; 			}
L19:
	.dbline 84
	.dbline 85
L17:
	.dbline 66
	rjmp L16
L18:
	.dbline 88
; 		}
; 	}
; 	
; 	//当前有效命令范围0x10~0x20
; 	rcvByte = UARTRcvaByte();
	rcall _UARTRcvaByte
	mov R22,R16
	.dbline 89
; 	CMD = rcvByte;
	mov R20,R22
	clr R21
	.dbline 92
; 	
; 	//无效命令
; 	if(( CMD < 0x10 )||( CMD > 0x20 ))
	cpi R20,16
	ldi R30,0
	cpc R21,R30
	brlo L25
	ldi R24,32
	ldi R25,0
	cp R24,R20
	cpc R25,R21
	brsh L23
L25:
	.dbline 93
; 	{
	.dbline 95
; 	 	//显示0xbb表示命令无效状态
; 		SPI_MasterTransmitaByte(0x82);//显示b
	ldi R16,130
	rcall _SPI_MasterTransmitaByte
	.dbline 96
; 		SPI_MasterTransmitaByte(0x82);//显示b
	ldi R16,130
	rcall _SPI_MasterTransmitaByte
	.dbline 97
; 		PORTB&=0xFB;
	in R24,0x18
	andi R24,251
	out 0x18,R24
	.dbline 98
; 		PORTB|=0x04;
	sbi 0x18,2
	.dbline 100
; 		
; 		SendBuffer[0] = 0xBB;
	ldi R24,187
	sts _SendBuffer,R24
	.dbline 101
; 		SendBuffer[1] = 0xFF;
	ldi R24,255
	sts _SendBuffer+1,R24
	.dbline 102
; 		SendBuffer[2] = 0x00;
	clr R2
	sts _SendBuffer+2,R2
	.dbline 103
; 		SendBuffer[3] = 0x00;
	sts _SendBuffer+3,R2
	.dbline 104
; 		SendBuffer[4] = 0x00;
	sts _SendBuffer+4,R2
	.dbline 105
; 		SendBuffer[6] = 0xFF;
	sts _SendBuffer+6,R24
	.dbline 106
; 		UARTSendAMessage(SendBuffer,7);
	ldi R18,7
	ldi R19,0
	ldi R16,<_SendBuffer
	ldi R17,>_SendBuffer
	rcall _UARTSendAMessage
	.dbline 108
; 		
; 		return (0);
	clr R16
	rjmp L15
L23:
	.dbline 111
; 	}
; 	else
; 	{
	.dbline 113
; 	 	//检测校验码
; 		rcvByte = UARTRcvaByte();
	rcall _UARTRcvaByte
	mov R22,R16
	.dbline 114
; 		if( (CMD + rcvByte) != 0xFF )
	mov R2,R22
	clr R3
	movw R24,R20
	add R24,R2
	adc R25,R3
	cpi R24,255
	ldi R30,0
	cpc R25,R30
	breq L31
	.dbline 115
; 		{
	.dbline 117
; 		 	//校验码不匹配，显示0xcc表示传输出错
; 			SPI_MasterTransmitaByte(0xc6);//显示c
	ldi R16,198
	rcall _SPI_MasterTransmitaByte
	.dbline 118
; 			SPI_MasterTransmitaByte(0xc6);//显示c
	ldi R16,198
	rcall _SPI_MasterTransmitaByte
	.dbline 119
; 			PORTB&=0xFB;
	in R24,0x18
	andi R24,251
	out 0x18,R24
	.dbline 120
; 			PORTB|=0x04;
	sbi 0x18,2
	.dbline 122
; 			
; 			SendBuffer[0] = 0xBB;
	ldi R24,187
	sts _SendBuffer,R24
	.dbline 123
; 			SendBuffer[1] = 0xFF;
	ldi R24,255
	sts _SendBuffer+1,R24
	.dbline 124
; 			SendBuffer[2] = 0x00;
	clr R2
	sts _SendBuffer+2,R2
	.dbline 125
; 			SendBuffer[3] = 0x00;
	sts _SendBuffer+3,R2
	.dbline 126
; 			SendBuffer[4] = 0x00;
	sts _SendBuffer+4,R2
	.dbline 127
; 			SendBuffer[6] = 0xFF;
	sts _SendBuffer+6,R24
	.dbline 128
; 			UARTSendAMessage(SendBuffer,7);
	ldi R18,7
	ldi R19,0
	ldi R16,<_SendBuffer
	ldi R17,>_SendBuffer
	rcall _UARTSendAMessage
	.dbline 130
; 			
; 			return (0);	
	clr R16
	rjmp L15
L31:
	.dbline 133
; 		}
; 		else
; 		{
	.dbline 134
; 		 	rcvByte = UARTRcvaByte();
	rcall _UARTRcvaByte
	mov R22,R16
	.dbline 135
; 			if( rcvByte != 0xFF )
	cpi R16,255
	breq L38
	.dbline 136
; 			{
	.dbline 138
; 			 	//显示0xdd表明帧结束标志错误
; 				SPI_MasterTransmitaByte(0xa1);//显示d
	ldi R16,161
	rcall _SPI_MasterTransmitaByte
	.dbline 139
; 				SPI_MasterTransmitaByte(0xa1);//显示d
	ldi R16,161
	rcall _SPI_MasterTransmitaByte
	.dbline 140
; 				PORTB&=0xFB;
	in R24,0x18
	andi R24,251
	out 0x18,R24
	.dbline 141
; 				PORTB|=0x04;
	sbi 0x18,2
	.dbline 143
; 				
; 				SendBuffer[0] = 0xBB;
	ldi R24,187
	sts _SendBuffer,R24
	.dbline 144
; 				SendBuffer[1] = 0xFF;
	ldi R24,255
	sts _SendBuffer+1,R24
	.dbline 145
; 				SendBuffer[2] = 0x00;
	clr R2
	sts _SendBuffer+2,R2
	.dbline 146
; 				SendBuffer[3] = 0x00;
	sts _SendBuffer+3,R2
	.dbline 147
; 				SendBuffer[4] = 0x00;
	sts _SendBuffer+4,R2
	.dbline 148
; 				SendBuffer[6] = 0xFF;
	sts _SendBuffer+6,R24
	.dbline 149
; 				UARTSendAMessage(SendBuffer,7);
	ldi R18,7
	ldi R19,0
	ldi R16,<_SendBuffer
	ldi R17,>_SendBuffer
	rcall _UARTSendAMessage
	.dbline 151
; 				
; 				return (0);	
	clr R16
	rjmp L15
L38:
	.dbline 154
; 			}
; 			else
; 			{
	.dbline 155
; 			 	return (CMD);
	mov R16,R20
	.dbline -2
L15:
	rcall pop_gset2
	.dbline 0 ; func end
	ret
	.dbsym r k 20 i
	.dbsym r CMD 20 i
	.dbsym r rcvByte 22 c
	.dbend
	.area bss(ram, con, rel)
	.dbfile C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc\UART.c
_SendBuffer::
	.blkb 7
	.dbsym e SendBuffer _SendBuffer A[7:7]c
