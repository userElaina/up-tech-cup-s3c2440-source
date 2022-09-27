;
; Copyright (c) Microsoft Corporation.  All rights reserved.
;
;
; Use of this source code is subject to the terms of the Microsoft end-user
; license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
; If you did not accept the terms of the EULA, you are not authorized to use
; this source code. For a copy of the EULA, please see the LICENSE.RTF on your
; install media.
;


		INCLUDE kxarm.h
		INCLUDE armmacros.s
		INCLUDE eboot_s3c2440a.inc

;-------------------------------------------------------------------------------

MemoryMap       EQU     0x2a4
BANK_SIZE       EQU     0x00100000      ; 1MB per bank in MemoryMap array
BANK_SHIFT      EQU     20


;   Define RAM space for the Page Tables:
;
PHYBASE			EQU     0x30000000      ; physical start
PTs			EQU     0x30010000      ; 1st level page table address (PHYBASE + 0x10000)
						; save room for interrupt vectors.

;Start address of each stacks,
_STACK_BASEADDRESS	EQU	0x33ff8000  
_MMUTT_STARTADDRESS	EQU	0x33ff8000  
_ISR_STARTADDRESS	EQU	0x33ffff00
; Pre-defined constants.
;
USERMODE		EQU 	0x10
FIQMODE			EQU 	0x11
IRQMODE			EQU 	0x12
SVCMODE			EQU 	0x13
ABORTMODE		EQU 	0x17
UNDEFMODE		EQU 	0x1b
MODEMASK		EQU 	0x1f
NOINT			EQU 	0xc0

; Stack locations.
;
SVCStack		EQU	(_STACK_BASEADDRESS-0x2800)	; 0x33ff5800 ~
UserStack		EQU	(_STACK_BASEADDRESS-0x3800)	; 0x33ff4800 ~ 
UndefStack		EQU	(_STACK_BASEADDRESS-0x2400)	; 0x33ff5c00 ~
AbortStack		EQU	(_STACK_BASEADDRESS-0x2000)	; 0x33ff6000 ~
IRQStack		EQU	(_STACK_BASEADDRESS-0x1000)	; 0x33ff7000 ~
FIQStack		EQU	(_STACK_BASEADDRESS-0x0)	; 0x33ff8000 ~ 
;virtual address
VirtualSVCStack		EQU	(0x83ff5800)			; 0x83ff5800 ~
VirtualUserStack	EQU	(0x83ff4800)			; 0x83ff4800 ~ 
virtualUndefStack	EQU	(0x83ff5c00)			; 0x83ff5c00 ~
VirtualAbortStack	EQU	(0x83ff6000)			; 0x83ff6000 ~
VirtualIRQStack		EQU	(0x83ff7000)			; 0x83ff7000 ~
VirtualFIQStack		EQU	(0x83ff8000)			; 0x83ff8000 ~ 
;-------------------------------------------------------------------------------

	TEXTAREA

	IMPORT  main

; Set up the MMU and Dcache for bootloader.
;
; This routine will initialize the first-level page table based up the contents
; of the MemoryMap array and enable the MMU and caches.
;
; Copy the image to RAM if it's not already running there.
;
; Include Files 



; Defines

;------------------------------------------------------------------------------
; BUGBUG - ?

PLLVAL      EQU     (((0x5C << 12) + (0x1 << 4) + 0x1))		; 2440A  400MHz, input 12MHz

;------------------------------------------------------------------------------
; Cache Configuration

DCACHE_LINES_PER_SET_BITS	EQU     (6)
DCACHE_LINES_PER_SET		EQU     (64)
DCACHE_NUM_SETS			EQU     (8)
DCACHE_SET_INDEX_BIT		EQU     (32 - DCACHE_LINES_PER_SET_BITS)
DCACHE_LINE_SIZE		EQU     (32)



; External Variables 

; External Functions 

; Global Variables 
 
; Local Variables 
 
; Local Functions 

;-------------------------------------------------------------------------------
;   Function: Startup
;
;   Main entry point for CPU initialization.
;

	STARTUPTEXT
	LEAF_ENTRY      StartUp

	; Jump over power-off code. 
	b       ResetHandler

ResetHandler

	; Make sure that TLB & cache are consistent
	mov     r0, #0
	mcr     p15, 0, r0, c8, c7, 0		; flush both TLB
	mcr     p15, 0, r0, c7, c5, 0		; invalidate instruction cache
	mcr     p15, 0, r0, c7, c6, 0		; invalidate data cache
	
	ldr     r0, = GPBCON
	ldr     r1, = 0x055555      
	str     r1, [r0]

	ldr     r0, = WTCON			; disable watch dog
	ldr     r1, = 0x0	 
	str     r1, [r0]

	ldr     r0, = INTMSK
	ldr     r1, = 0xffffffff		; disable all interrupts
	str     r1, [r0]

	ldr     r0, = INTSUBMSK
	ldr     r1, = 0x7fff			; disable all sub interrupt
	str     r1, [r0]

	ldr     r0, = INTMOD
	mov     r1, #0x0			; set all interrupt as IRQ
	str     r1, [r0]

	ldr     r0, = CLKDIVN
	ldr     r1, = 0x7			; 0x0 = 1:1:1,  0x1 = 1:1:2, 0x2 = 1:2:2,  0x3 = 1:2:4,
;	ldr     r1, = 0x5			; 0x0 = 1:1:1,  0x1 = 1:1:2, 0x2 = 1:2:2,  0x3 = 1:2:4,
						; 0x7 = 1:3:6,  0x8 = 1:4:4
	str     r1, [r0]

;	ands    r1, r1, #0xe			; set AsyncBusMode
	; MMU_SetAsyncBusMode FCLK:HCLK= 1:2
;	ands	r1, r1, #0x2
;	beq     %F10

	mrc     p15, 0, r0, c1, c0, 0
	orr     r0, r0, #R1_nF:OR:R1_iA
	mcr     p15, 0, r0, c1, c0, 0
;10

	ldr     r0, = LOCKTIME			; To reduce PLL lock time, adjust the LOCKTIME register. 
	ldr     r1, = 0xffffff
	str     r1, [r0]
    
	ldr	r0, = CAMDIVN
	ldr	r1, = 0
	str	r1, [r0]

	ldr     r0, = MPLLCON			; Configure MPLL
						; Fin=12MHz, Fout=400MHz
	ldr     r1, = PLLVAL
	str     r1, [r0]

	ldr     r0, = UPLLCON					; Fin=12MHz, Fout=48MHz
;	ldr     r1, = ((0x3c << 12) + (0x4 << 4) + 0x2)		; 16Mhz
	ldr     r1, = ((0x38 << 12) + (0x2 << 4) + 0x2)  	; 12Mhz
	str     r1, [r0]

	mov     r0, #0x2000
20   
	subs    r0, r0, #1
	bne     %B20

;------------------------------------------------------------------------------
;   Add for Power Management 

	ldr     r1, =GSTATUS2			; Determine Booting Mode
	ldr     r10, [r1]

;------------------------------------------------------------------------------
;   Add for Power Management 

	tst     r10, #0x2
	beq     BringUpWinCE			; Normal Mode Booting

;------------------------------------------------------------------------------
;   Add for Power Management ?

BringUpWinCE

;------------------------------------------------------------------------------
;   Initialize memory controller


	add     r0, pc, #MEMCTRLTAB - (. + 8)
	ldr     r1, = BWSCON			; BWSCON Address
	add     r2, r0, #52			; End address of MEMCTRLTAB
40      ldr     r3, [r0], #4    
	str     r3, [r1], #4    
	cmp     r2, r0      
	bne     %B40
    
	ldr     r0, = GPFDAT
	mov     r1, #0x60
	str     r1, [r0]

;------------------------------------------------------------------------------
;   Copy boot loader to memory

	ands    r9, pc, #0xFF000000		; see if we are in flash or in ram
	bne     %f20				; go ahead if we are already in ram

	; This is the loop that perform copying.
	ldr     r0, = 0x38000			; offset into the RAM 
	add     r0, r0, #PHYBASE		; add physical base
	mov     r1, r0				; (r1) copy destination
	ldr     r2, =0x0			; (r2) flash started at physical address 0
	ldr     r3, =0x10000			; counter (0x40000/4)
10      ldr     r4, [r2], #4
	str     r4, [r1], #4
	subs    r3, r3, #1
	bne     %b10

	; Restart from the RAM position after copying.
	mov pc, r0
	nop
	nop
	nop

	; Shouldn't get here.
	b       .

	INCLUDE oemaddrtab_cfg.inc
 

	; Compute physical address of the OEMAddressTable.
20      add     r11, pc, #g_oalAddressTable - (. + 8)
	ldr     r10, =PTs			; (r10) = 1st level page table


	; Setup 1st level page table (using section descriptor)     
	; Fill in first level page table entries to create "un-mapped" regions
	; from the contents of the MemoryMap array.
	;
	;   (r10) = 1st level page table
	;   (r11) = ptr to MemoryMap array

	add     r10, r10, #0x2000		; (r10) = ptr to 1st PTE for "unmapped space"
	mov     r0, #0x0E			; (r0) = PTE for 0: 1MB cachable bufferable
	orr     r0, r0, #0x400			; set kernel r/w permission
25      mov     r1, r11				; (r1) = ptr to MemoryMap array

	
30      ldr     r2, [r1], #4			; (r2) = virtual address to map Bank at
	ldr     r3, [r1], #4			; (r3) = physical address to map from
	ldr     r4, [r1], #4			; (r4) = num MB to map

	cmp     r4, #0				; End of table?
	beq     %f40

	ldr     r5, =0x1FF00000
	and     r2, r2, r5			; VA needs 512MB, 1MB aligned.		

	ldr     r5, =0xFFF00000
	and     r3, r3, r5			; PA needs 4GB, 1MB aligned.

	add     r2, r10, r2, LSR #18
	add     r0, r0, r3			; (r0) = PTE for next physical page

35      str     r0, [r2], #4
	add     r0, r0, #0x00100000		; (r0) = PTE for next physical page
	sub     r4, r4, #1			; Decrement number of MB left 
	cmp     r4, #0
	bne     %b35				; Map next MB

	bic     r0, r0, #0xF0000000		; Clear Section Base Address Field
	bic     r0, r0, #0x0FF00000		; Clear Section Base Address Field
	b       %b30				; Get next element
	
40      tst     r0, #8
	bic     r0, r0, #0x0C			; clear cachable & bufferable bits in PTE
	add     r10, r10, #0x0800		; (r10) = ptr to 1st PTE for "unmapped uncached space"
	bne     %b25				; go setup PTEs for uncached space
	sub     r10, r10, #0x3000		; (r10) = restore address of 1st level page table

	; Setup mmu to map (VA == 0) to (PA == 0x30000000).
	ldr     r0, =PTs			; PTE entry for VA = 0
	ldr     r1, =0x3000040E			; uncache/unbuffer/rw, PA base == 0x30000000
	str     r1, [r0]

	; uncached area.
	add     r0, r0, #0x0800			; PTE entry for VA = 0x0200.0000 , uncached     
	ldr     r1, =0x30000402			; uncache/unbuffer/rw, base == 0x30000000
	str     r1, [r0]
	
	; Comment:
	; The following loop is to direct map RAM VA == PA. i.e. 
	;   VA == 0x30XXXXXX => PA == 0x30XXXXXX for S3C2400
	; Fill in 8 entries to have a direct mapping for DRAM
	;
	ldr     r10, =PTs			; restore address of 1st level page table
	ldr     r0,  =PHYBASE

	add     r10, r10, #(0x3000 / 4)		; (r10) = ptr to 1st PTE for 0x30000000

	add     r0, r0, #0x1E			; 1MB cachable bufferable
	orr     r0, r0, #0x400			; set kernel r/w permission
	mov     r1, #0 
	mov     r3, #64

45      mov     r2, r1				; (r2) = virtual address to map Bank at
	cmp     r2, #0x20000000:SHR:BANK_SHIFT
	add     r2, r10, r2, LSL #BANK_SHIFT-18
	strlo   r0, [r2]
	add     r0, r0, #0x00100000		; (r0) = PTE for next physical page
	subs    r3, r3, #1
	add     r1, r1, #1
	bgt     %b45

	ldr     r10, =PTs			; (r10) = restore address of 1st level page table

	; The page tables and exception vectors are setup.
	; Initialize the MMU and turn it on.
	mov     r1, #1
	mcr     p15, 0, r1, c3, c0, 0		; setup access to domain 0
	mcr     p15, 0, r10, c2, c0, 0

	mcr     p15, 0, r0, c8, c7, 0		; flush I+D TLBs
	mov     r1, #0x0071			; Enable: MMU
	and	r1, r1, #0xC0000000		; [31:30] AsyncBusMode Enable, [12] ICache Disable(UMON set enable)
	orr	r1, r1, #0x0079			; [0] Enable MMU, [6:3] Reserved 1111
	orr     r1, r1, #0x4000			; Round-Robind Cache Replacement Policy
	orr	r1, r1, #0x1000			; Enable ICache (with MMU On)

;	orr     r1, r1, #0x0004			; Enable the cache
	mcr     p15, 0, r1, c1, c0, 0
	; Make AsyncBusMode 
	mrc	p15, 0, r0, c1, c0, 0
	orr	r0, r0, #R1_nF:OR:R1_iA
	mcr	p15, 0, r0, c1, c0, 0


	ldr     r0, =VirtualStart

	cmp     r0, #0				; make sure no stall on "mov pc,r0" below
	mov     pc, r0				;  & jump to new virtual address
	nop

	; MMU & caches now enabled.
	;   (r10) = physcial address of 1st level page table
	;

VirtualStart

	mov     sp, #0x80000000
	add     sp, sp, #0x30000		; arbitrary initial super-page stack pointer

	mrs     r0,cpsr
	bic     r0,r0,#MODEMASK
 
	orr     r1,r0,#UNDEFMODE|NOINT
	msr     cpsr_cxsf,r1		; UndefMode
	ldr     sp,=virtualUndefStack

	orr     r1,r0,#ABORTMODE|NOINT
	msr     cpsr_cxsf,r1		; AbortMode
	ldr     sp,=VirtualAbortStack

	orr     r1,r0,#IRQMODE|NOINT
	msr     cpsr_cxsf,r1		; IRQMode
	ldr     sp,=VirtualIRQStack

	orr     r1,r0,#FIQMODE|NOINT
	msr     cpsr_cxsf,r1		; FIQMode
	ldr     sp,=VirtualFIQStack

	mrs	r0, cpsr
	bic	r0, r0, #MODEMASK|NOINT
	orr	r1, r0, #SVCMODE
	msr	cpsr_cxsf, r1			; SVCMode.
	ldr     sp, =VirtualSVCStack
	b	main

	ENTRY_END
 
	LTORG

;------------------------------------------------------------------------------
; Memory Controller Configuration 
;
;   The below defines are used in the MEMCTRLTAB table
;   defined below to iniatialize the memory controller's
;   register bank.
;
; SDRAM refresh control register configuration

REFEN		EQU     (0x1)		; Refresh enable
TREFMD		EQU     (0x0)		; CBR(CAS before RAS)/Auto refresh
Trp		EQU     (0x1)		; 3clk
Trc		EQU     (0x3)		; 7clk
Tchr		EQU     (0x2)		; 3clk
REFCNT		EQU     (0x4f4)		; period=7.8125us, HCLK=100Mhz, (2048+1-7.8125*100)

; Bank Control 
;
; Bus width and wait status control 

B1_BWSCON   EQU     (DW32)
B2_BWSCON   EQU     (DW16)
B3_BWSCON   EQU     (DW16 + WAIT + UBLB)
B4_BWSCON   EQU     (DW16)
B5_BWSCON   EQU     (DW16)
B6_BWSCON   EQU     (DW32)
B7_BWSCON   EQU     (DW32)

; Bank 0

B0_Tacs     EQU     (0x0)    ; 0clk
B0_Tcos     EQU     (0x0)    ; 0clk
B0_Tacc     EQU     (0x7)    ; 14clk
B0_Tcoh     EQU     (0x0)    ; 0clk
B0_Tah      EQU     (0x0)    ; 0clk
B0_Tacp     EQU     (0x0)    
B0_PMC      EQU     (0x0)    ; normal

; Bank 1

B1_Tacs     EQU     (0x0)    ; 0clk
B1_Tcos     EQU     (0x0)    ; 0clk
B1_Tacc     EQU     (0x7)    ; 14clk
B1_Tcoh     EQU     (0x0)    ; 0clk
B1_Tah      EQU     (0x0)    ; 0clk
B1_Tacp     EQU     (0x0)    
B1_PMC      EQU     (0x0)    ; normal

; Bank 2

B2_Tacs     EQU     (0x0)    ; 0clk
B2_Tcos     EQU     (0x0)    ; 0clk
B2_Tacc     EQU     (0x7)    ; 14clk
B2_Tcoh     EQU     (0x0)    ; 0clk
B2_Tah      EQU     (0x0)    ; 0clk
B2_Tacp     EQU     (0x0)     
B2_PMC      EQU     (0x0)    ; normal

; Bank 3

B3_Tacs     EQU     (0x0)    ; 0clk
B3_Tcos     EQU     (0x0)    ; 0clk
B3_Tacc     EQU     (0x7)    ; 14clk
B3_Tcoh     EQU     (0x0)    ; 0clk
B3_Tah      EQU     (0x0)    ; 0clk
B3_Tacp     EQU     (0x0)    
B3_PMC      EQU     (0x0)    ; normal

; Bank 4

B4_Tacs     EQU     (0x0)    ; 0clk
B4_Tcos     EQU     (0x0)    ; 0clk
B4_Tacc     EQU     (0x7)    ; 14clk
B4_Tcoh     EQU     (0x0)    ; 0clk
B4_Tah      EQU     (0x0)    ; 0clk
B4_Tacp     EQU     (0x0)    
B4_PMC      EQU     (0x0)    ; normal

; Bank 5

B5_Tacs     EQU     (0x0)    ; 0clk
B5_Tcos     EQU     (0x0)    ; 0clk
B5_Tacc     EQU     (0x7)    ; 14clk
B5_Tcoh     EQU     (0x0)    ; 0clk
B5_Tah      EQU     (0x0)    ; 0clk
B5_Tacp     EQU     (0x0)    
B5_PMC      EQU     (0x0)    ; normal

; Bank 6

B6_MT       EQU     (0x3)    ; SDRAM
B6_Trcd     EQU     (0x1)    ; 3clk
B6_SCAN     EQU     (0x1)    ; 9bit

; Bank 7
;
; Note - there is no memory connected to Bank 7

B7_MT       EQU     (0x3)    ; SDRAM
B7_Trcd     EQU     (0x1)    ; 3clk
B7_SCAN     EQU     (0x1)    ; 9bit


;------------------------------------------------------------------------------
;   Memory Controller Configuration Data Table
;
;   This data block is loaded into the memory controller's 
;   registers to configure the platform memory.
;

MEMCTRLTAB DATA
	DCD (0+(B1_BWSCON<<4)+(B2_BWSCON<<8)+(B3_BWSCON<<12)+(B4_BWSCON<<16)+(B5_BWSCON<<20)+(B6_BWSCON<<24)+(B7_BWSCON<<28))
	DCD ((B0_Tacs<<13)+(B0_Tcos<<11)+(B0_Tacc<<8)+(B0_Tcoh<<6)+(B0_Tah<<4)+(B0_Tacp<<2)+(B0_PMC))	; BANKCON0
	DCD ((B1_Tacs<<13)+(B1_Tcos<<11)+(B1_Tacc<<8)+(B1_Tcoh<<6)+(B1_Tah<<4)+(B1_Tacp<<2)+(B1_PMC))	; BANKCON1 
	DCD ((B2_Tacs<<13)+(B2_Tcos<<11)+(B2_Tacc<<8)+(B2_Tcoh<<6)+(B2_Tah<<4)+(B2_Tacp<<2)+(B2_PMC))	; BANKCON2
	DCD ((B3_Tacs<<13)+(B3_Tcos<<11)+(B3_Tacc<<8)+(B3_Tcoh<<6)+(B3_Tah<<4)+(B3_Tacp<<2)+(B3_PMC))	; BANKCON3
	DCD ((B4_Tacs<<13)+(B4_Tcos<<11)+(B4_Tacc<<8)+(B4_Tcoh<<6)+(B4_Tah<<4)+(B4_Tacp<<2)+(B4_PMC))	; BANKCON4
	DCD ((B5_Tacs<<13)+(B5_Tcos<<11)+(B5_Tacc<<8)+(B5_Tcoh<<6)+(B5_Tah<<4)+(B5_Tacp<<2)+(B5_PMC))	; BANKCON5
	DCD ((B6_MT<<15)+(B6_Trcd<<2)+(B6_SCAN))							; BANKCON6
	DCD ((B7_MT<<15)+(B7_Trcd<<2)+(B7_SCAN))							; BANKCON7
	DCD ((REFEN<<23)+(TREFMD<<22)+(Trp<<20)+(Trc<<18)+(Tchr<<16)+REFCNT)				; REFRESH 100MHz
	DCD 0x32											; BANKSIZE
	DCD 0x30											; MRSRB6
	DCD 0x30											; MRSRB7

	END

;-------------------------------------------------------------------------------
