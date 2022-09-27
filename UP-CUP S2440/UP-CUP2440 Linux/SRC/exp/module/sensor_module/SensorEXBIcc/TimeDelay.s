	.module TimeDelay.c
	.area text(rom, con, rel)
	.dbfile C:\DOCUME~1\²ÌÓ±Åô\×ÀÃæ\SensorEXBIcc\TimeDelay.c
	.dbfunc e delay_50us _delay_50us fV
;              j -> R20,R21
;              n -> R16,R17
	.even
_delay_50us::
	rcall push_gset1
	.dbline -1
	.dbline 6
; #include "Generic.h"
; 
; 
; 
; void delay_50us(unsigned int n)
; {
	.dbline 8
; 	unsigned int j;
; 	for(;n>0;n--)
	rjmp L5
L2:
	.dbline 9
; 	{
	.dbline 10
; 		for(j=0;j<70;j++)
	clr R20
	clr R21
	rjmp L9
L6:
	.dbline 11
L7:
	.dbline 10
	subi R20,255  ; offset = 1
	sbci R21,255
L9:
	.dbline 10
	cpi R20,70
	ldi R30,0
	cpc R21,R30
	brlo L6
	.dbline 12
L3:
	.dbline 8
	subi R16,1
	sbci R17,0
L5:
	.dbline 8
	cpi R16,0
	cpc R16,R17
	brne L2
X0:
	.dbline -2
L1:
	rcall pop_gset1
	.dbline 0 ; func end
	ret
	.dbsym r j 20 i
	.dbsym r n 16 i
	.dbend
	.dbfunc e delay_ms _delay_ms fV
;              n -> R20,R21
	.even
_delay_ms::
	rcall push_gset1
	movw R20,R16
	.dbline -1
	.dbline 16
; 			;
; 	}
; }
; 
; void delay_ms(unsigned int n)
; {
	.dbline 17
; 	for(;n>0;n--)
	rjmp L14
L11:
	.dbline 18
	.dbline 19
	ldi R16,20
	ldi R17,0
	rcall _delay_50us
	.dbline 20
L12:
	.dbline 17
	subi R20,1
	sbci R21,0
L14:
	.dbline 17
	cpi R20,0
	cpc R20,R21
	brne L11
X1:
	.dbline -2
L10:
	rcall pop_gset1
	.dbline 0 ; func end
	ret
	.dbsym r n 20 i
	.dbend
	.dbfunc e delay_50ms _delay_50ms fV
;              i -> R20,R21
;              n -> R16,R17
	.even
_delay_50ms::
	rcall push_gset1
	.dbline -1
	.dbline 24
; 	{
; 		delay_50us(20);
; 	}		
; }
; 
; void delay_50ms(unsigned int n)
; {
	.dbline 26
; 	unsigned int i; 
; 	for(;n>0;n--)
	rjmp L19
L16:
	.dbline 27
; 	{
	.dbline 28
; 		for(i=0;i<52642;i++)
	clr R20
	clr R21
	rjmp L23
L20:
	.dbline 29
L21:
	.dbline 28
	subi R20,255  ; offset = 1
	sbci R21,255
L23:
	.dbline 28
	cpi R20,162
	ldi R30,205
	cpc R21,R30
	brlo L20
	.dbline 30
L17:
	.dbline 26
	subi R16,1
	sbci R17,0
L19:
	.dbline 26
	cpi R16,0
	cpc R16,R17
	brne L16
X2:
	.dbline -2
L15:
	rcall pop_gset1
	.dbline 0 ; func end
	ret
	.dbsym r i 20 i
	.dbsym r n 16 i
	.dbend
	.dbfunc e delay_s _delay_s fV
;              n -> R20,R21
	.even
_delay_s::
	rcall push_gset1
	movw R20,R16
	.dbline -1
	.dbline 34
; 		; 
; 	}
; }
; 
; void delay_s(unsigned int n)
; {
	.dbline 35
; 	for(;n>0;n--)
	rjmp L28
L25:
	.dbline 36
	.dbline 37
	ldi R16,20
	ldi R17,0
	rcall _delay_50ms
	.dbline 38
L26:
	.dbline 35
	subi R20,1
	sbci R21,0
L28:
	.dbline 35
	cpi R20,0
	cpc R20,R21
	brne L25
X3:
	.dbline -2
L24:
	rcall pop_gset1
	.dbline 0 ; func end
	ret
	.dbsym r n 20 i
	.dbend
