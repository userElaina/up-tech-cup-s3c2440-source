;******************************************************************************
;*
;* Copyright (c) 2006 Mobile Solution Development, Samsung Electronics, Inc.
;* All rights reserved.
;*
;* This software is the confidential and proprietary information of Samsung 
;* Electronics, Inc("Confidential Information"). You Shall not disclose such 
;* Confidential Information and shall use it only in accordance with the terms 
;* of the license agreement you entered into Samsung.
;*
;******************************************************************************

;   File:  startup.s
;
;   Kernel startup routine for Samsung UP2440 board. Hardware is
;   initialized in boot loader - so there isn't much code at all.
;
;------------------------------------------------------------------------------

		INCLUDE kxarm.h
		INCLUDE armmacros.s
		INCLUDE s3c2440a.inc

		IMPORT  OALClearUTLB
		IMPORT  OALFlushICache
		IMPORT  OALFlushDCache
		IMPORT  KernelStart

		GBLA	LOADERUSE
LOADERUSE	SETA	0   ;If system boot from using Eboot and Step-Loader, You don't need to configure clock setting again.

;---------------------------------------------------------------------------
;	4 LED light function for Physical Map
;	The LEDs are located below AMD Flash ROM

	MACRO
	LED_ON	$data
	LDR		r10, =GPFDAT
	LDR		r11, =$data
	MOV		r11, r11, lsl #4
  	STR		r11, [r10]
	MEND

;---------------------------------------------------------------------------
;	4 LED light function for Virtual Map
;	The LEDs are located below AMD Flash ROM

	MACRO
	VLED_ON	$data
	LDR		r10, =vGPFDAT
	LDR		r11, =$data
	MOV		r11, r11, lsl #4
  	STR		r11, [r10]
	MEND
;---------------------------------------------------------------------------


	[ LOADERUSE = 0
;---------------------------------------------------------------------------
;	Voltage Change function
;	The LEDs are located below AMD Flash ROM

	MACRO
	VOLTAGECHANGE

	ldr r8, = GPBDAT  ; D4
	ldr r9, [r8]
	ldr r10, = 0x77f
	and r9, r9, r10
	ldr r10, = (D4VAL<<7)
	orr r9, r9, r10
	str r9, [r8]

	ldr r8, = GPFDAT  ; D3~0
	ldr r9, [r8]
	ldr r10, = 0x0f
	and r9, r9, r10
	ldr r10, = ((D3VAL<<7)+(D2VAL<<6)+(D1VAL<<5)+(D0VAL<<4))
	orr r9, r9, r10
	str r9, [r8]

	ldr r8, = GPBCON  ; GPB7: Output
	ldr r9, [r8]
	ldr r10, = 0x3f3fff
	and r9, r9, r10
	ldr r10, = (1<<14)
	orr r9, r9, r10
	str r9, [r8]

	;ldr r8, = GPFCON  ; GPF4~7: Output
	;ldr r9, [r8]
	;ldr r10, = 0x00ff
	;and r9, r9, r10
	;ldr r10, = 0x9500
	;orr r9, r9, r10
	;str r9, [r8]

	ldr r8, = GPBDAT  ; Latch enable
	ldr r9, [r8]
	ldr r10, = ~(0<<8)
	and r9, r9, r10
	str r9, [r8]

	ldr r8, = GPBCON  ; GPB8: Output
	ldr r9, [r8]
	ldr r10, = 0x3cffff
	and r9, r9, r10
	ldr r10, = (1<<16)
	orr r9, r9, r10
	str r9, [r8]

	ldr r8, = GPBDAT  ; Output enable
	ldr r9, [r8]
	ldr r10, = (1<<10)
	orr r9, r9, r10
	str r9, [r8]

	ldr r8, = GPBCON  ; GPB10: Output
	ldr r9, [r8]
	ldr r10, = 0x0fffff
	and r9, r9, r10
	ldr r10, = (1<<20)
	orr r9, r9, r10
	str r9, [r8]

	ldr r8, = GPBDAT  ; Latch disable
	ldr r9, [r8]
	ldr r10, = (1<<8)
	orr r9, r9, r10
	str r9, [r8]
 
    MEND
;---------------------------------------------------------------------------
	]

; Data Cache Characteristics.
;
DCACHE_LINES_PER_SET_BITS       EQU     6
DCACHE_LINES_PER_SET		EQU     64
DCACHE_NUM_SETS			EQU     8
DCACHE_SET_INDEX_BIT		EQU     (32 - DCACHE_LINES_PER_SET_BITS)
DCACHE_LINE_SIZE		EQU     32



SLEEPDATA_BASE_VIRTUAL		EQU	0xA0028000		; keep in sync w/ config.bib
SLEEPDATA_BASE_PHYSICAL		EQU	0x30028000

SleepState_Data_Start		EQU     (0)

SleepState_WakeAddr    		EQU     (SleepState_Data_Start		    )
SleepState_MMUCTL		EQU     (SleepState_WakeAddr    + WORD_SIZE )
SleepState_MMUTTB       	EQU     (SleepState_MMUCTL  	+ WORD_SIZE )
SleepState_MMUDOMAIN    	EQU     (SleepState_MMUTTB  	+ WORD_SIZE )
SleepState_SVC_SP       	EQU     (SleepState_MMUDOMAIN   + WORD_SIZE )
SleepState_SVC_SPSR     	EQU     (SleepState_SVC_SP  	+ WORD_SIZE )
SleepState_FIQ_SPSR     	EQU     (SleepState_SVC_SPSR    + WORD_SIZE )
SleepState_FIQ_R8       	EQU     (SleepState_FIQ_SPSR    + WORD_SIZE )
SleepState_FIQ_R9       	EQU     (SleepState_FIQ_R8  	+ WORD_SIZE )
SleepState_FIQ_R10      	EQU     (SleepState_FIQ_R9  	+ WORD_SIZE )
SleepState_FIQ_R11      	EQU     (SleepState_FIQ_R10 	+ WORD_SIZE )
SleepState_FIQ_R12      	EQU     (SleepState_FIQ_R11 	+ WORD_SIZE )
SleepState_FIQ_SP       	EQU     (SleepState_FIQ_R12 	+ WORD_SIZE )
SleepState_FIQ_LR       	EQU     (SleepState_FIQ_SP  	+ WORD_SIZE )
SleepState_ABT_SPSR     	EQU     (SleepState_FIQ_LR  	+ WORD_SIZE )
SleepState_ABT_SP       	EQU     (SleepState_ABT_SPSR    + WORD_SIZE )
SleepState_ABT_LR       	EQU     (SleepState_ABT_SP  	+ WORD_SIZE )
SleepState_IRQ_SPSR     	EQU     (SleepState_ABT_LR  	+ WORD_SIZE )
SleepState_IRQ_SP       	EQU     (SleepState_IRQ_SPSR    + WORD_SIZE )
SleepState_IRQ_LR       	EQU     (SleepState_IRQ_SP  	+ WORD_SIZE )
SleepState_UND_SPSR     	EQU     (SleepState_IRQ_LR  	+ WORD_SIZE )
SleepState_UND_SP       	EQU     (SleepState_UND_SPSR    + WORD_SIZE )
SleepState_UND_LR       	EQU     (SleepState_UND_SP  	+ WORD_SIZE )
SleepState_SYS_SP       	EQU     (SleepState_UND_LR  	+ WORD_SIZE )
SleepState_SYS_LR       	EQU     (SleepState_SYS_SP  	+ WORD_SIZE )

SleepState_Data_End     	EQU     (SleepState_SYS_LR	+ WORD_SIZE )

SLEEPDATA_SIZE		    	EQU     ((SleepState_Data_End - SleepState_Data_Start) / 4)

BIT_SELFREFRESH			EQU	(1<<22)
HandleFIQ			EQU	0x33ffff1c


;**
; * StartUp - Image EntryPoint
; *

	STARTUPTEXT
	LEAF_ENTRY StartUp
1	b		ResetHandler
	b		%B1			;handler for Undefined mode
	b		%B1			;handler for SWI interrupt
	b		%B1			;handler for PAbort
	b		%B1			;handler for DAbort
	b		%B1			;reserved
	b		%B1	  		;handler for IRQ interrupt 
	b		ENTER_POWER_OFF		;handler for FIQ interrupt

SelfRefreshAndPowerOff		; run with Instruction Cache's code
	str		r1, [r0]		; Enable SDRAM self-refresh
	str		r3, [r2]		; MISCCR Setting
	str		r5, [r4]		; Power Off !!
	b		.

ENTER_POWER_OFF	;FIQ Handler
;	LED_ON	0x4

	ldr		r0,=GPFCON
	ldr		r1,=0x9500
	str		r1,[r0]
	ldr		r0,=GPFDAT
	ldr		r1,=0x20
	str		r1,[r0]

	ldr		r5,=REFRESH
	ldr		r6,[r5]
	orr		r6, r6, #BIT_SELFREFRESH

	ldr		r3,=MISCCR
	ldr		r4,[r3]
	orr		r4,r4,#(3<<17)
	bic		r4,r4, #(7 << 20)
	orr		r4,r4, #(6 << 20)

	ldr		r2,=0x7fff8
	ldr		r1,=CLKCON

	mov		r0,#16
	b		_EPO		;This code isn't needed for code on the ROM

	ALIGN	32
_EPO
1	str		r6, [r5]	;Enable SDRAM self-refresh
2	subs		r0,r0,#1	;Wait until self-refresh is issued,which may not be needed.
	bne		%B2
	str		r4,[r3]		;Make sure that SCLK0:SCLK->0, SCLK1:SCLK->0, SCKE=L during boot-up 
	str		r2,[r1]    	;CPU will enter into POWER_OFF mode.
	b		.

	LTORG


ResetHandler 

	LED_ON	0x1
	
	[ LOADERUSE = 0
	
	VOLTAGECHANGE
	
	;ldr		r0, = GPFCON
	;ldr		r1, = 0x95aa  
	;str		r1, [r0]
	
	ldr		r0, =WTCON		; disable the watchdog timer.
	ldr		r1, =0x0
	str		r1, [r0]

	ldr		r0, =INTMSK		; mask all first-level interrupts.
	ldr		r1, =0xffffffff
	str		r1, [r0]

	ldr		r0, =INTSUBMSK		; mask all second-level interrupts.
	ldr		r1, =0x7fff
	str		r1, [r0]
	]
	
	ldr		r0, = INTMSK
	ldr		r1, = ~BIT_BAT_FLT	; all interrupt disable, nBATT_FLT =enabled
	str		r1, [r0]

	ldr		r0, = INTMOD
	ldr		r1, = BIT_BAT_FLT	; set all interrupt as IRQ, BAT_FLT = FIQ
	str		r1, [r0]

	ldr		r1, =MISCCR		; MISCCR's Bit [22:20] -> 100
	ldr		r0, [r1]
	bic		r0, r0, #(7 << 20)
	orr		r0, r0, #(4 << 20)
	str		r0, [r1]

	[ LOADERUSE = 0
	ldr		r0, = CLKDIVN
	ldr		r1, = CLKDIVVAL		;MPLL Clock Divider=1:3:6/1:4:8
	str		r1, [r0]
	]
	
	; Make AsyncBusMode 
	mrc		p15, 0, r0, c1, c0, 0
	orr		r0, r0, #R1_nF:OR:R1_iA
	mcr		p15, 0, r0, c1, c0, 0

	[ LOADERUSE = 0
	ldr		r0, = LOCKTIME		; To reduce PLL lock time, adjust the LOCKTIME register. 
	ldr		r1, = 0xffffff
	str		r1, [r0]
	
	ldr		r0, = UPLLCON		; Fin=12MHz, UPLL=48/96MHz
	ldr		r1, = UPLLVAL		;UPLLVAL1
	str		r1, [r0]

	nop ; Caution: After UPLL setting, at least 7-clocks delay must be inserted for setting hardware be completed.
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	ldr		r0, = UPLLCON		; Fin=12MHz, UPLL=48/96MHz
	ldr		r1, = UPLLVAL
	str		r1, [r0]

	nop ; Caution: After UPLL setting, at least 7-clocks delay must be inserted for setting hardware be completed.
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop

	ldr		r0, = CAMDIVN
	ldr		r1, = 0
	str		r1, [r0]

	ldr		r0, = MPLLCON		; Configure MPLL
						; Fin=12MHz, MPLL= 400MHz
	ldr		r1, = PLLVAL
	str		r1, [r0]

	; delay
	mov		r0, #0x200
4	subs		r0, r0, #1
	bne		%B4
    
;------------------------------------------------------------------------------
;   Initialize memory controller


	add		r0, pc, #MEMCTRLTAB - (. + 8)
	ldr		r1, = BWSCON		; BWSCON Address
	add		r2, r0, #52		; End address of MEMCTRLTAB
10	ldr		r3, [r0], #4
	str		r3, [r1], #4
	cmp		r2, r0
	bne		%B10
	]
	
;	LED_ON		0x6

;; Checking Booting Mode, GSTATUS:1(Power-On Reset), GSTATUS:2(Sleep mode Wakeup), GSTATUS:4(WatchDog Reset)
	ldr		r1, =GSTATUS2		; Determine Booting Mode
	ldr		r10, [r1]
	tst		r10, #0x2		; Power-Off reset Check
	beq		%F5			; if not wakeup from PowerOffmode
						; goto Watchdog reset test.
	b		%F6			; if wakeup from PowerOff mode
						; goto Power-up code.

5
	tst		r10, #0x4		;  reset check
	beq		BringUpWinCE		; Normal Mode Booting
	mov		r0, #4
	str		r0, [r1]		; Clear the GSTATUS2
	b		BringUpWinCE

; Power-OFF Wake-Up Recover Process : Starting Point

; Case of Power-off reset
6
	ldr		r1, =MISCCR			; MISCCR's Bit 17, 18, 19 -> 0
	ldr		r0, [r1]
	bic		r0, r0, #(3 << 17)
	str		r0, [r1]

	ldr		r1, =GSTATUS2
	mov		r0, #2
	str		r0, [r1]			; Clear the GSTATUS2

;  1. Checksum Calculation saved Data

	ldr		r5, =SLEEPDATA_BASE_PHYSICAL	; pointer to physical address of reserved Sleep mode info data structure 

	mov		r3, r5				; pointer for checksum calculation
	mov		r2, #0
	ldr		r0, =SLEEPDATA_SIZE		; get size of data structure to do checksum on
40
	ldr		r1, [r3], #4			; pointer to SLEEPDATA
	and		r1, r1, #0x1
	mov		r1, r1, LSL #31
	orr		r1, r1, r1, LSR #1
	add		r2, r2, r1
	subs		r0, r0, #1			; dec the count
	bne		%b40				; loop till done

	ldr		r0,=GSTATUS3
	ldr		r3, [r0]			; get the Sleep data checksum from the Power Manager Scratch pad register
	teq		r2, r3				; compare to what we saved before going to sleep
	bne		JumpToRAM			; bad news - do a cold boot - If emergency power off case, normal booting.
	b		MMUENABLE
JumpToRAM
	ldr		r2, =0x201000			; offset into the RAM
	ldr		r3, =0x30000000			; add physical base
	add		r2, r2, r3
	mov		pc, r2				;  & jump to StartUp address

MMUENABLE
;  2. MMU Enable

	ldr		r10, [r5, #SleepState_MMUDOMAIN]	; load the MMU domain access info
	ldr		r9,  [r5, #SleepState_MMUTTB]		; load the MMU TTB info	
	ldr		r8,  [r5, #SleepState_MMUCTL]		; load the MMU control info
	ldr		r7,  [r5, #SleepState_WakeAddr]		; load the LR address
	nop			
	nop
	nop
	nop
	nop

; if software reset
	mov		r1, #0x38000000
	teq		r1, r7
	bne		%f1
	bl		BringUpWinCE

; wakeup routine
1
	mcr		p15, 0, r10, c3, c0, 0		; setup access to domain 0
	mcr		p15, 0, r9,  c2, c0, 0		; PT address
	mcr		p15, 0, r0,  c8, c7, 0	   	; flush I+D TLBs
	mcr		p15, 0, r8,  c1, c0, 0		; restore MMU control

;  3. Jump to Kernel Image's fw.s(Awake_address)
	mov		pc, r7				; & jump to new virtual address (back up Power management stack)
	nop

	LED_ON		0xF

BringUpWinCE	
	add		r0, pc, #g_oalAddressTable - (. + 8)
	bl		KernelStart

	LTORG

MEMCTRLTAB DATA
	DCD (0+(B1_BWSCON<<4)+(B2_BWSCON<<8)+(B3_BWSCON<<12)+(B4_BWSCON<<16)+(B5_BWSCON<<20)+(B6_BWSCON<<24)+(B7_BWSCON<<28))
	DCD ((B0_Tacs<<13)+(B0_Tcos<<11)+(B0_Tacc<<8)+(B0_Tcoh<<6)+(B0_Tah<<4)+(B0_Tacp<<2)+(B0_PMC))   ; BANKCON0
	DCD ((B1_Tacs<<13)+(B1_Tcos<<11)+(B1_Tacc<<8)+(B1_Tcoh<<6)+(B1_Tah<<4)+(B1_Tacp<<2)+(B1_PMC))   ; BANKCON1
	DCD ((B2_Tacs<<13)+(B2_Tcos<<11)+(B2_Tacc<<8)+(B2_Tcoh<<6)+(B2_Tah<<4)+(B2_Tacp<<2)+(B2_PMC))   ; BANKCON2
	DCD ((B3_Tacs<<13)+(B3_Tcos<<11)+(B3_Tacc<<8)+(B3_Tcoh<<6)+(B3_Tah<<4)+(B3_Tacp<<2)+(B3_PMC))   ; BANKCON3
	DCD ((B4_Tacs<<13)+(B4_Tcos<<11)+(B4_Tacc<<8)+(B4_Tcoh<<6)+(B4_Tah<<4)+(B4_Tacp<<2)+(B4_PMC))   ; BANKCON4
	DCD ((B5_Tacs<<13)+(B5_Tcos<<11)+(B5_Tacc<<8)+(B5_Tcoh<<6)+(B5_Tah<<4)+(B5_Tacp<<2)+(B5_PMC))   ; BANKCON5
	DCD ((B6_MT<<15)+(B6_Trcd<<2)+(B6_SCAN))							; BANKCON6
	DCD ((B7_MT<<15)+(B7_Trcd<<2)+(B7_SCAN))							; BANKCON7
	DCD ((REFEN<<23)+(TREFMD<<22)+(Trp<<20)+(Trc<<18)+(Tchr<<16)+REFCNT)				; REFRESH 100MHz
	DCD 0x32											; BANKSIZE
	DCD 0x30											; MRSRB6
	DCD 0x30											; MRSRB7

	INCLUDE oemaddrtab_cfg.inc



; * OALCPUPowerOff - OFF button handler(Called from OEMPowerOff() in cfw.c)
; *     This routine is invoked when the OFF button is pressed. It is responsible
; *	for any final power off state and putting the cpu into standby.
; *
; *	Entry	none
; *	Exit	none
; *	Uses	r0-r3
; *

	LEAF_ENTRY OALCPUPowerOff

;   1. Push SVC state onto our stack
	stmdb		sp!, {r4-r12}
	stmdb		sp!, {lr}

;   2. Save MMU & CPU Register to RAM
	ldr		r3, =SLEEPDATA_BASE_VIRTUAL	; base of Sleep mode storage

	ldr		r2, =Awake_address		; store Virtual return address
	str		r2, [r3], #4

	mrc		p15, 0, r2, c1, c0, 0		; load r2 with MMU Control
	ldr		r0, =MMU_CTL_MASK		; mask off the undefined bits
	bic		r2, r2, r0
	str		r2, [r3], #4			; store MMU Control data

	mrc		p15, 0, r2, c2, c0, 0		; load r2 with TTB address.
	ldr		r0, =MMU_TTB_MASK		; mask off the undefined bits
	bic		r2, r2, r0
	str		r2, [r3], #4			; store TTB address

	mrc		p15, 0, r2, c3, c0, 0		; load r2 with domain access control.
	str		r2, [r3], #4			; store domain access control

	str		sp, [r3], #4			; store SVC stack pointer

	mrs		r2, spsr
	str		r2, [r3], #4			; store SVC status register

	mov		r1, #Mode_FIQ:OR:I_Bit:OR:F_Bit	; Enter FIQ mode, no interrupts
	msr		cpsr, r1
	mrs		r2, spsr
	stmia		r3!, {r2, r8-r12, sp, lr}	; store the FIQ mode registers

	mov		r1, #Mode_ABT:OR:I_Bit:OR:F_Bit	; Enter ABT mode, no interrupts
	msr		cpsr, r1
	mrs		r0, spsr
	stmia		r3!, {r0, sp, lr}		; store the ABT mode Registers

	mov		r1, #Mode_IRQ:OR:I_Bit:OR:F_Bit	; Enter IRQ mode, no interrupts
	msr		cpsr, r1
	mrs		r0, spsr
	stmia		r3!, {r0, sp, lr}		; store the IRQ Mode Registers

	mov		r1, #Mode_UND:OR:I_Bit:OR:F_Bit	; Enter UND mode, no interrupts
	msr		cpsr, r1
	mrs		r0, spsr
	stmia		r3!, {r0, sp, lr}		; store the UND mode Registers

	mov		r1, #Mode_SYS:OR:I_Bit:OR:F_Bit	; Enter SYS mode, no interrupts
	msr		cpsr, r1
	stmia		r3!, {sp, lr}			; store the SYS mode Registers

	mov		r1, #Mode_SVC:OR:I_Bit:OR:F_Bit	; Back to SVC mode, no interrupts
	msr		cpsr, r1

;   3. do Checksum on the Sleepdata
	ldr		r3, =SLEEPDATA_BASE_VIRTUAL	; get pointer to SLEEPDATA
	ldr		r2, =0x0
	ldr		r0, =SLEEPDATA_SIZE		; get size of data structure (in words)
30
	ldr		r1, [r3], #4
	and		r1, r1, #0x1
	mov		r1, r1, ROR #31
	add		r2, r2, r1
	subs		r0, r0, #1
	bne		%b30

	ldr		r0, =vGPIOBASE
	str		r2, [r0, #oGSTATUS3]		; Store in Power Manager Scratch pad register

;   4. Interrupt Disable 
	ldr		r0, =vINTBASE
	mvn		r2, #0
	str		r2, [r0, #oINTMSK]
	str		r2, [r0, #oSRCPND]
	str		r2, [r0, #oINTPND]

;;  5. Cache Flush
	bl		OALClearUTLB
	bl		OALFlushICache
	ldr		r0, = (DCACHE_LINES_PER_SET - 1)    
	ldr		r1, = (DCACHE_NUM_SETS - 1)    
	ldr		r2, = DCACHE_SET_INDEX_BIT    
	ldr		r3, = DCACHE_LINE_SIZE
	bl		OALFlushDCache

;   6. Setting Wakeup External Interrupt(EINT0,1,2) Mode
	ldr		r0, =vGPIOBASE

	ldr		r1, =0x950a
	str		r1, [r0, #oGPFCON]

;   7. Go to Power-Off Mode
	ldr		r0, =vMISCCR			; hit the TLB
	ldr		r0, [r0]
	ldr		r0, =vCLKCON
	ldr		r0, [r0]

	ldr		r0, =vREFRESH		
	ldr		r1, [r0]			; r1=rREFRESH	
	orr		r1, r1, #(1 << 22)

	ldr		r2, =vMISCCR
	ldr		r3, [r2]
	orr		r3, r3, #(3<<17)		; Make sure that SCLK0:SCLK->0, SCLK1:SCLK->0, SCKE=L during boot-up 
	bic		r3, r3, #(7<<20)
	orr		r3, r3, #(6<<20)

	ldr		r4, =vCLKCON
	ldr		r5, =0x1ffff8			; Power Off Mode

;	8. Jump to real Power-Off Address
	ldr		r7, =0x90000000			; make address to 0x9200 0020
	add		r7, r7, #0x2000000		; 
	add		r7, r7, #0x20			; 
	mov		pc, r7				; jump to Power off code in ROM

Awake_address

;   1. Recover CPU Registers

	ldr		r3, =SLEEPDATA_BASE_VIRTUAL	; Sleep mode information data structure
	add		r2, r3, #SleepState_FIQ_SPSR
	mov		r1, #Mode_FIQ:OR:I_Bit:OR:F_Bit	; Enter FIQ mode, no interrupts - also FIQ
	msr		cpsr, r1
	ldr		r0,  [r2], #4
	msr		spsr, r0
	ldr		r8,  [r2], #4
	ldr		r9,  [r2], #4
	ldr		r10, [r2], #4
	ldr		r11, [r2], #4
	ldr		r12, [r2], #4
	ldr		sp,  [r2], #4
	ldr		lr,  [r2], #4

	mov		r1, #Mode_ABT:OR:I_Bit		; Enter ABT mode, no interrupts
	msr		cpsr, r1
	ldr		r0, [r2], #4
	msr		spsr, r0
	ldr		sp, [r2], #4
	ldr		lr, [r2], #4

	mov		r1, #Mode_IRQ:OR:I_Bit		; Enter IRQ mode, no interrupts
	msr		cpsr, r1
	ldr		r0, [r2], #4
	msr		spsr, r0
	ldr		sp, [r2], #4
	ldr		lr, [r2], #4

	mov		r1, #Mode_UND:OR:I_Bit		; Enter UND mode, no interrupts
	msr		cpsr, r1
	ldr		r0, [r2], #4
	msr		spsr, r0
	ldr		sp, [r2], #4
	ldr		lr, [r2], #4

	mov		r1, #Mode_SYS:OR:I_Bit		; Enter SYS mode, no interrupts
	msr		cpsr, r1
	ldr		sp, [r2], #4
	ldr		lr, [r2]

	mov		r1, #Mode_SVC:OR:I_Bit		; Enter SVC mode, no interrupts - FIQ is available
	msr		cpsr, r1
	ldr		r0, [r3, #SleepState_SVC_SPSR]
	msr		spsr, r0

;   2. Recover Last mode's REG's, & go back to caller of OALCPUPowerOff()

	ldr		sp, [r3, #SleepState_SVC_SP]
	ldr		lr, [sp], #4
	ldmia   sp!, {r4-r12}
	mov		pc, lr				; and now back to our sponsors



;------------------------------------------------------------------------------
; Clock Division Change funtion for DVS on S3C2440A.
;------------------------------------------------------------------------------

	LEAF_ENTRY CLKDIV124
	ldr		r0, = vCLKDIVN
	ldr		r1, = 0x3			; 0x3 = 1:2:4
	str		r1, [r0]
	mov		pc, lr

	LEAF_ENTRY CLKDIV144
	ldr		r0, = vCLKDIVN
	ldr		r1, = 0x4			; 0x4 = 1:4:4
	str		r1, [r0]
	mov		pc, lr

	LEAF_ENTRY CLKDIV136
	ldr		r0, = vCLKDIVN
	ldr		r1, = 0x7			; 1:6:12
	str		r1, [r0]
	ldr		r0, = vCAMDIVN
	ldr		r1, [r0]
	bic		r1, r1, #(0x3<<8)
	orr		r1, r1, #(0x0<<8)		; 1:3:6
	str		r1, [r0]	
	mov		pc, lr

	LEAF_ENTRY CLKDIV166
	ldr		r0, = vCAMDIVN
	ldr		r1, [r0]
	bic		r1, r1, #(0x3<<8)
	orr		r1, r1, #(0x1<<8)		; 1:6:12
	str		r1, [r0]
	ldr		r0, = vCLKDIVN
	ldr		r1, = 0x6			; 1:6:6
	str		r1, [r0]	
	mov		pc, lr

	LEAF_ENTRY CLKDIV148
	ldr		r0, = vCLKDIVN
	ldr		r1, = 0x5			; 1:8:16
	ldr		r2, = vCAMDIVN
	ldr		r3, [r2]
	bic		r3, r3, #(0x3<<8)
	orr		r3, r3, #(0x0<<8)		; 1:4:8
	str		r1, [r0]
	str		r3, [r2]
	mov		pc, lr

	LEAF_ENTRY CLKDIV188
	ldr		r0, = vCAMDIVN
	ldr		r1, [r0]
	bic		r1, r1, #(0x3<<8)
	orr		r1, r1, #(0x2<<8)		; 1:8:16
	ldr		r2, = vCLKDIVN
	ldr		r3, = 0x4			; 1:8:8
	str		r1, [r0]
	str		r3, [r2]
	mov		pc, lr

	LEAF_ENTRY DVS_ON	
	ldr		r0, = vCAMDIVN
	ldr		r1, [r0]
	orr		r1, r1, #(0x1<<12)		; DVS_EN bit = 1(FCLK = HCLK)
	str		r1, [r0]
	mov		pc, lr

	LEAF_ENTRY DVS_OFF
	ldr		r0, = vCAMDIVN
	ldr		r1, [r0]
	bic		r1, r1, #(0x1<<12)		; DVS_EN bit = 0(FCLK = MPLL clock)
	str		r1, [r0]
	mov		pc, lr
	
	END
