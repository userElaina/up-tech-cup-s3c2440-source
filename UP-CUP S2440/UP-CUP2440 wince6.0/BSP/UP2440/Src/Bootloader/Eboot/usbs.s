; Module Name: usb.s                                           

	OPT	2	; disable listing
	INCLUDE kxarm.h
	OPT	1	; reenable listing
	OPT	128	; disable listing of macro expansions


    TEXTAREA
	IMPORT  IsrUsbd
; ---------------------------------------------------------------------
; ---------------------------------------------------------------------        
    LEAF_ENTRY IsrHandler

;	sub	sp,sp,#4	;decrement sp(to store jump address)
	sub lr, lr, #4
	stmfd   sp!, {r0-r12,lr}
	mov	r0, lr
	bl	IsrUsbd
	ldmfd   sp!, {r0-r12,lr}

	movs pc, lr

    ENDP    ; |IsrHandler|

    END    

