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
;                                                                     
; Module Name: nand.s                                           
; 
; Assembly code to speed up NAND read and write. 
;
;      
;*************************************************************
; NAME : NAND FLASH Subroutine for a410 bootstrap
; DATE : 18.FEB.2002
; DESC :
;  02.18.2002:purnnamu: modified for A410
;  04.24.2002:purnnamu: optimized for NAND flash bootstrap 
;*************************************************************
	OPT	2	; disable listing
	INCLUDE kxarm.h
	OPT	1	; reenable listing
	OPT	128	; disable listing of macro expansions

;; Macro LDR4STR1
;;    
	MACRO
	LDR4STR1 $src,$tmp1,$tmp2	
	ldr		$tmp1,[$src]
	MEND

;; Macro LDR1STR4
;;    
    MACRO
    LDR1STR4 $dest, $tmp1, $tmp2
	str		$tmp1, [$dest]
    MEND
    

    TEXTAREA

; ---------------------------------------------------------------------
;   void ReadPage512(BYTE *pBuf, PUSHORT pNFData)
;
;   pBuf (r0):       a buffer big enough to hold 512 bytes
;   pNFData (r1):    the DATA register on the NAND controller
;
; ---------------------------------------------------------------------        
    LEAF_ENTRY ReadPage512
    
    stmfd     sp!, {r2-r11} 
    
	mov	r2, #0x200
10	
	LDR4STR1 r1,r4,r3
	LDR4STR1 r1,r5,r3
	LDR4STR1 r1,r6,r3
	LDR4STR1 r1,r7,r3
	LDR4STR1 r1,r8,r3
	LDR4STR1 r1,r9,r3
	LDR4STR1 r1,r10,r3
	LDR4STR1 r1,r11,r3
	stmia	r0!,{r4-r11}
	subs	r2,r2,#32
	bne	%B10

	ldmfd	sp!,{r2-r11}
	mov	pc,lr
    
    ENDP    ; |ReadPage512|
    
; ---------------------------------------------------------------------
;   void WritePage512(BYTE *pBuf, PUSHORT pNFData)
;
;   pBuf (r0):       the data buffer
;   pNFData (r1):    the DATA register on the NAND controller
;
; ---------------------------------------------------------------------        
    
    LEAF_ENTRY WritePage512
    
    stmfd   sp!, {r2-r12}

;;; -- We use r12 as the mask     
    mov r12, #0xFF                        
    mov r2, #0x200
10
    ldmia   r0!, {r4-r11}
    LDR1STR4 r1, r4, r3    
    LDR1STR4 r1, r5, r3    
    LDR1STR4 r1, r6, r3    
    LDR1STR4 r1, r7, r3    
    LDR1STR4 r1, r8, r3    
    LDR1STR4 r1, r9, r3    
    LDR1STR4 r1, r10, r3    
    LDR1STR4 r1, r11, r3
    subs    r2, r2, #32
    bne %B10
    
    ldmfd   sp!, {r2-r12}
    mov     pc, lr
    
    ENDP    ; |WritePage512|    
    
    END    
