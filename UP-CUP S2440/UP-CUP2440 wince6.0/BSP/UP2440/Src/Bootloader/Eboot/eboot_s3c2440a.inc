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
;******************************************************************************
;
;  Header: s3c2440a.inc
;
;  This header file defines only those registers required by the startup
;  code. All addresses are based off the physical addresses (PA) defined
;  in s3c2440a_base_reg.h (s3c2440a_base_reg.inc).
;
;******************************************************************************

; Include the base register definitions

	INCLUDE s3c2440a_base_regs.inc



;   Define MSP type
	GBLA    MSP_VER
MSP_VER         SETA   3 ; 1(MSP1,2), 3(MSP3)

;******************************************************************************
; General CPU constants
;******************************************************************************
Mode_USR         EQU        (0x10)
Mode_FIQ         EQU        (0x11)
Mode_IRQ         EQU        (0x12)
Mode_SVC         EQU        (0x13)
Mode_ABT         EQU        (0x17)
Mode_UND         EQU        (0x1B)
Mode_SYS         EQU        (0x1F)
I_Bit            EQU        (0x80)
F_Bit            EQU        (0x40)
R1_iA            EQU        (1<<31)
R1_nF            EQU        (1<<30)
R1_I			 EQU		(1<<12)
R1_C			 EQU		(1<<2)
R1_A			 EQU		(1<<1)
R1_M  			 EQU		(1)



;******************************************************************************
;  Miscellaneous defines
;******************************************************************************
WORD_SIZE       EQU         (4)
DW8             EQU         (0x0)
DW16            EQU         (0x1)
DW32            EQU         (0x2)
WAIT            EQU         (0x1<<2)
UBLB            EQU         (0x1<<3)

;******************************************************************************
; MMU constants
;******************************************************************************
MMU_CTL_MASK     EQU        (0x3FFF0000)
MMU_TTB_MASK     EQU        (0x00003FFF)
MMU_ID_MASK      EQU        (0xFFFFFFF0)

;******************************************************************************
;  Interrupt Control Registers
;******************************************************************************
INTMSK          EQU         (S3C2440A_BASE_REG_PA_INTR        + 0x08)
INTSUBMSK       EQU         (S3C2440A_BASE_REG_PA_INTR        + 0x1C)
INTMOD          EQU         (S3C2440A_BASE_REG_PA_INTR        + 0x04)

vSRCPND			EQU			0xB0a00000	;Interrupt request status
vINTMOD			EQU			0xB0a00004	;Interrupt mode control
vINTMSK			EQU			0xB0a00008	;Interrupt mask control
vPRIORITY		EQU			0xB0a0000c	;IRQ priority control
vINTPND			EQU			0xB0a00010	;Interrupt request status
vINTOFFSET		EQU			0xB0a00014	;Interruot request source offset
vSUSSRCPND		EQU			0xB0a00018	;Sub source pending
vINTSUBMSK		EQU			0xB0a0001c	;Interrupt sub mask
vINTBASE		EQU			0xB0a00000	;Interrupt request status

oSRCPND			EQU			0x00	        ;Interrupt request status
oINTMSK			EQU			0x08	        ;Interrupt mask control
oINTPND			EQU			0x10	        ;Interrupt request status
oINTSUBMSK		EQU			0x1c	        ;Interrupt sub mask



;******************************************************************************
;  IOPort Control Registers
;******************************************************************************
GPBCON			EQU			(S3C2440A_BASE_REG_PA_IOPORT      + 0x10)
GPBDAT			EQU			(S3C2440A_BASE_REG_PA_IOPORT      + 0x14)
GPBUP			EQU			(S3C2440A_BASE_REG_PA_IOPORT      + 0x18)

GPFCON          EQU         (S3C2440A_BASE_REG_PA_IOPORT      + 0x50)
GPFDAT          EQU         (S3C2440A_BASE_REG_PA_IOPORT      + 0x54)

MISCCR          EQU         (S3C2440A_BASE_REG_PA_IOPORT      + 0x80)
GSTATUS2        EQU         (S3C2440A_BASE_REG_PA_IOPORT      + 0xB4)
GSTATUS3        EQU         (S3C2440A_BASE_REG_PA_IOPORT      + 0xB8)

vGPFDAT			EQU			0xB1600054		;Port F data

vGPIOBASE		EQU			0xB1600000		;Port A control
oGPACON			EQU			0x00
oGPADAT			EQU			0x04            ;Port A data
			  
oGPBCON			EQU			0x10            ;Port B control
oGPBDAT			EQU			0x14            ;Port B data
oGPBUP			EQU			0x18            ;Pull-up control B
			  
oGPCCON			EQU			0x20            ;Port C control
oGPCDAT			EQU			0x24            ;Port C data
oGPCUP			EQU			0x28            ;Pull-up control C
			  
oGPDCON			EQU			0x30            ;Port D control
oGPDDAT			EQU			0x34            ;Port D data
oGPDUP			EQU			0x38            ;Pull-up control D
			  
oGPECON			EQU			0x40            ;Port E control
oGPEDAT			EQU			0x44            ;Port E data
oGPEUP			EQU			0x48            ;Pull-up control E
			  
oGPFCON			EQU			0x50            ;Port F control
oGPFDAT			EQU			0x54            ;Port F data
oGPFUP			EQU			0x58            ;Pull-up control F
			  
oGPGCON			EQU			0x60            ;Port G control
oGPGDAT			EQU			0x64            ;Port G data
oGPGUP			EQU			0x68            ;Pull-up control G
			  
oGPHCON			EQU			0x70            ;Port H control
oGPHDAT			EQU			0x74            ;Port H data
oGPHUP			EQU			0x78            ;Pull-up control H
			  
oMISCCR			EQU			0x80            ;Miscellaneous control
oDCKCON			EQU			0x84            ;DCLK0/1 control
oEXTINT0		EQU			0x88            ;External interrupt control register 0
oEXTINT1		EQU			0x8c            ;External interrupt control register 1
oEXTINT2		EQU			0x90            ;External interrupt control register 2
oEINTFLT0		EQU			0x94            ;Reserved
oEINTFLT1		EQU			0x98            ;Reserved
oEINTFLT2		EQU			0x9c            ;External interrupt filter control register 2
oEINTFLT3		EQU			0xa0            ;External interrupt filter control register 3
oEINTMASK		EQU			0xa4            ;External interrupt mask
oEINTPEND		EQU			0xa8            ;External interrupt pending
oGSTATUS0		EQU			0xac            ;External pin status
oGSTATUS1		EQU			0xb0            ;Chip ID(0x32440000)
oGSTATUS2		EQU			0xb4            ;Reset type
oGSTATUS3		EQU			0xb8            ;Saved data0(32-bit) before entering POWER_OFF mode 
oGSTATUS4		EQU			0xbc            ;Saved data1(32-bit) before entering POWER_OFF mode

vMISCCR			EQU			0xB1600080	;Miscellaneous control

;******************************************************************************
;  Watch Dog Control Registers
;******************************************************************************
WTCON           EQU         (S3C2440A_BASE_REG_PA_WATCHDOG    + 0x00)
vWTCON			EQU			0xB1300000      ;Watch-dog timer mode
vWTDAT			EQU			0xB1300004      ;Watch-dog timer data
vWTCNT			EQU			0xB1300008      ;Eatch-dog timer count

;******************************************************************************
; PWM TIMER
;******************************************************************************
TCFG0		EQU				(S3C2440A_BASE_REG_PA_PWM		+ 0x00)
TCFG1		EQU				(S3C2440A_BASE_REG_PA_PWM		+ 0x04)
TCON		EQU				(S3C2440A_BASE_REG_PA_PWM		+ 0x08)
TCNTB0		EQU				(S3C2440A_BASE_REG_PA_PWM		+ 0x0c)
TCMPB0		EQU				(S3C2440A_BASE_REG_PA_PWM		+ 0x10)
TCNTO0		EQU				(S3C2440A_BASE_REG_PA_PWM		+ 0x14)
TCNTB1		EQU				(S3C2440A_BASE_REG_PA_PWM		+ 0x18)
TCMPB1		EQU				(S3C2440A_BASE_REG_PA_PWM		+ 0x1c)
TCNTO1		EQU				(S3C2440A_BASE_REG_PA_PWM		+ 0x20)
TCNTB2		EQU				(S3C2440A_BASE_REG_PA_PWM		+ 0x24)
TCMPB2		EQU				(S3C2440A_BASE_REG_PA_PWM		+ 0x28)
TCNTO2		EQU				(S3C2440A_BASE_REG_PA_PWM		+ 0x2c)
TCNTB3		EQU				(S3C2440A_BASE_REG_PA_PWM		+ 0x30)
TCMPB3		EQU				(S3C2440A_BASE_REG_PA_PWM		+ 0x34)
TCNTO3		EQU				(S3C2440A_BASE_REG_PA_PWM		+ 0x38)
TCNTB4		EQU				(S3C2440A_BASE_REG_PA_PWM		+ 0x3c)
TCNTO4		EQU				(S3C2440A_BASE_REG_PA_PWM		+ 0x40)

vTCNTB4		EQU				0xB110003c



;******************************************************************************
;  Memory configuration Control Registers
;******************************************************************************
BWSCON          EQU         (S3C2440A_BASE_REG_PA_MEMCTRL     + 0x00)
REFRESH         EQU         (S3C2440A_BASE_REG_PA_MEMCTRL     + 0x24)
vREFRESH		EQU			0xB0800024	;DRAM/SDRAM refresh

;******************************************************************************
;  Clock and Power Control Registers
;******************************************************************************
LOCKTIME        EQU         (S3C2440A_BASE_REG_PA_CLOCK_POWER + 0x00)
MPLLCON         EQU         (S3C2440A_BASE_REG_PA_CLOCK_POWER + 0x04)
UPLLCON         EQU         (S3C2440A_BASE_REG_PA_CLOCK_POWER + 0x08)
CLKCON          EQU         (S3C2440A_BASE_REG_PA_CLOCK_POWER + 0x0C)
CLKSLOW         EQU         (S3C2440A_BASE_REG_PA_CLOCK_POWER + 0x10)
CLKDIVN         EQU         (S3C2440A_BASE_REG_PA_CLOCK_POWER + 0x14)
CAMDIVN         EQU         (S3C2440A_BASE_REG_PA_CLOCK_POWER + 0x18)
vMPLLCON		EQU			0xB0c00004	;MPLL Control
vCLKCON			EQU			0xB0c0000c	;Clock generator control
vCLKDIVN		EQU			0xB0c00014	;Clock divider control
vCAMDIVN		EQU			0xB0c00018	;Camera Clock divider control

;******************************************************************************
; PENDING BIT
;******************************************************************************
BIT_EINT0	EQU	(0x1)
BIT_EINT1	EQU	(0x1<<1)
BIT_EINT2	EQU	(0x1<<2)
BIT_EINT3	EQU	(0x1<<3)
BIT_EINT4_7	EQU	(0x1<<4)
BIT_EINT8_23	EQU	(0x1<<5)
BIT_CAM		EQU	(0x1<<6)		; 030610
BIT_BAT_FLT	EQU	(0x1<<7)
BIT_TICK	EQU	(0x1<<8)
BIT_WDT_AC97	EQU	(0x1<<9)
BIT_TIMER0	EQU	(0x1<<10)
BIT_TIMER1	EQU	(0x1<<11)
BIT_TIMER2	EQU	(0x1<<12)
BIT_TIMER3	EQU	(0x1<<13)
BIT_TIMER4	EQU	(0x1<<14)
BIT_UART2	EQU	(0x1<<15)
BIT_LCD		EQU	(0x1<<16)
BIT_DMA0	EQU	(0x1<<17)
BIT_DMA1	EQU	(0x1<<18)
BIT_DMA2	EQU	(0x1<<19)
BIT_DMA3	EQU	(0x1<<20)
BIT_SDI		EQU	(0x1<<21)
BIT_SPI0	EQU	(0x1<<22)
BIT_UART1	EQU	(0x1<<23)
BIT_NFCON	EQU	(0x1<<24)	; 030610
BIT_USBD	EQU	(0x1<<25)
BIT_USBH	EQU	(0x1<<26)
BIT_IIC		EQU	(0x1<<27)
BIT_UART0	EQU	(0x1<<28)
BIT_SPI1	EQU	(0x1<<29)
BIT_RTC		EQU	(0x1<<30)
BIT_ADC		EQU	(0x1<<31)
BIT_ALLMSK	EQU	(0xffffffff)





  END
 
