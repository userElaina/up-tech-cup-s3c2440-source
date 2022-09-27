;---------------------------------------------------------------------------------------------------------
;	2004.3.17
;---------------------------------------------------------------------------------------------------------
;改写了OSIntCtxSw函数中栈指针调整的算法，根据：
;	1、IRQStack栈的位置是固定的
;	2、IRQ中断以后，在调用ISR_IrqHandler函数之前，仅仅有
;	STMFD	sp!, {r0-r12, lr}
;	入栈，计算出栈指针调整的位置。避免了因为OSIntExit等函数经过编译器优化入栈不固定的问题
;---------------------------------------------------------------------------------------------------------
;
; File: os_cpu_a.s
;
;            Copyright 2002 by Interniche Technologies Inc.  All rights reserved. 
;
;            ARM Specific code for ChronOS port to Samsung/ARM7 board.
;
;

	AREA	|subr|, CODE, READONLY

	EXPORT	OSIntCtxSw
	EXPORT	OS_TASK_SW
	
	EXPORT   INTS_OFF
	EXPORT   INTS_ON


INTS_OFF
   mrs   r0, cpsr          ; current CSR
   mov   r1, r0            ; make a copy for masking
   orr   r1, r1, #0xC0     ; mask off int bits
   msr   CPSR_cxsf, r1     ; disable ints (IRQ and FIQ)
   and   r0, r0, #0x80     ; return FIQ bit from original CSR
   mov   pc,lr             ; return


INTS_ON
   mrs   r0, cpsr          ; current CSR
   bic   r0, r0, #0xC0     ; mask on ints
   msr   CPSR_cxsf, r0     ; enable ints (IRQ and FIQ)
   mov   pc,lr             ; return


	; External symbols we need the addresses of
			IMPORT	OSTCBCur
addr_OSTCBCur		DCD	OSTCBCur
			IMPORT	OSTCBHighRdy
addr_OSTCBHighRdy	DCD	OSTCBHighRdy
			IMPORT	OSPrioCur
addr_OSPrioCur		DCD	OSPrioCur
			IMPORT	OSPrioHighRdy
addr_OSPrioHighRdy	DCD	OSPrioHighRdy

         IMPORT IRQStack	;FIQ_STACK

OSIntCtxSw
; post FIQ Context switcher. This is called from OSIntExit when a hooked ISR
; wants to return in the context of another task. We load the new tasks context
; (from OSPrioHighRdy) and do the return from interrupt.
;

   ; Get pointer to stack where ISR_FiqHandler saved interrupted context
   ; ISR entry only saves first seven regs and LR.
   ;
	;add      r7, sp, #24   ; save pointer to register file, we must adjust this pointer to the position that just Enter Interrupt
	LDR	sp, =IRQStack	;IRQ_STACK ;test to del it
	sub	r7, sp, #4	;r7 is the position that just Enter Interrupt

	; Change ARM CPU to SVC mode for stack operations.
	; This gets the CPU off the interrupt stack and back to the
	; interrupted task's stack, which is the one we want to alter.
	;
	mrs      r1, SPSR                ; get suspended PSR
	orr      r1, r1, #0xC0           ; disable IRQ, FIQ.
	msr      CPSR_cxsf, r1           ; switch mode (shold be SVC_MODE)

   ; PSR, SP, LR regs are now restored to the interrupted SVC_MODE.
   ; now set up the task's stack frame as OS_TASK_SW does...

;	ldr      r0, [r7, #52]     ; get IRQ's LR (tasks PC) from IRQ stack //r0-r12
	ldr	 r0, [r7]	; get IRQ's LR (tasks PC) from IRQ stack
	sub      r0, r0, #4        ; Actual PC address is (saved_LR - 4)
	STMFD	sp!, {r0}		      ; save task PC
	STMFD	sp!, {lr}		      ; save LR

	sub	lr, r7, #52	;//we save the r0-r12 when we enter IRQ.
;	mov      lr, r7            ; save FIQ stack ptr in LR (going to nuke r7)
	ldmfd    lr!, {r0-r12}      ; get saved registers from FIQ stack
	STMFD	sp!, {r0-r12}	      ; save registers on task stack

   ; save PSR and PSR for task on task's stack
	MRS	r4, CPSR
	bic   r4, r4, #0xC0  ; leave interrupt bits in enabled mode
	STMFD	sp!, {r4}		; save task's current PSR
	MRS	r4, SPSR
	STMFD	sp!, {r4}		; SPSR too

	; OSPrioCur = OSPrioHighRdy   // change the current process
	LDR	r4, addr_OSPrioCur
	LDR	r5, addr_OSPrioHighRdy
	LDRB	r6, [r5]
	STRB	r6, [r4]
	
	; Get preempted tasks's TCB
	LDR	r4, addr_OSTCBCur
	LDR	r5, [r4]
	STR	sp, [r5]		; store sp in preempted tasks's TCB

	; Get new task TCB address
	LDR	r6, addr_OSTCBHighRdy
	LDR	r6, [r6]
	LDR	sp, [r6]		; get new task's stack pointer

	; OSTCBCur = OSTCBHighRdy
	STR	r6, [r4]		   ; set new current task TCB address


	LDMFD	sp!, {r4}
	MSR	SPSR_cxsf, r4
	LDMFD	sp!, {r4}
	BIC   r4, r4, #0xC0         ; we must exit to new task with ints enabled
	MSR	CPSR_cxsf, r4

	LDMFD	sp!, {r0-r12, lr, pc}


;	void OS_TASK_SW(void)
;	
;	Perform a context switch.
;
;	On entry, OSTCBCur and OSPrioCur hold the current TCB and priority
;	and OSTCBHighRdy and OSPrioHighRdy contain the same for the task
;	to be switched to.
;
;	The following code assumes that the virtual memory is directly
;	mapped into  physical memory. If this is not true, the cache must 
;	be flushed at context switch to avoid address aliasing.


OS_TASK_SW
	STMFD	sp!, {lr}		; save pc
	STMFD	sp!, {lr}		; save lr
	STMFD	sp!, {r0-r12}	; save registers and ret address
	MRS	r4, CPSR
	STMFD	sp!, {r4}		; save current PSR
	MRS	r4, SPSR	
	STMFD	sp!, {r4}		; save SPSR

	; OSPrioCur = OSPrioHighRdy
	LDR	r4, addr_OSPrioCur
	LDR	r5, addr_OSPrioHighRdy
	LDRB	r6, [r5]
	STRB	r6, [r4]
	
	; Get current task TCB address
	LDR	r4, addr_OSTCBCur
	LDR	r5, [r4]
	STR	sp, [r5]		; store sp in preempted tasks's TCB

	; Get highest priority task TCB address
	LDR	r6, addr_OSTCBHighRdy
	LDR	r6, [r6]
	LDR	sp, [r6]		; get new task's stack pointer

	; OSTCBCur = OSTCBHighRdy
	STR	r6, [r4]		; set new current task TCB address

   ; restore task's mode regsiters
	LDMFD	sp!, {r4}
	MSR	SPSR_cxsf, r4
	LDMFD	sp!, {r4}
	MSR	CPSR_cxsf, r4

   ; return in new task context
	LDMFD	sp!, {r0-r12, lr, pc}



;	void OSStartHighRdy(void)
;	
;	Start the task with the highest priority;
;
	EXPORT	OSStartHighRdy
OSStartHighRdy
	LDR	r4, addr_OSTCBCur	; Get current task TCB address
	LDR	r5, addr_OSTCBHighRdy	; Get highest priority task TCB address

	LDR	r5, [r5]		; get stack pointer
	LDR	sp, [r5]		; switch to the new stack

	STR	r5, [r4]		; set new current task TCB address

	LDMFD	sp!, {r4}		; YYY
	MSR	SPSR_cxsf, r4
	LDMFD	sp!, {r4}		; get new state from top of the stack
	MSR	CPSR_cxsf, r4		; CPSR should be SVC32Mode
	LDMFD	sp!, {r0-r12, lr, pc }	; start the new task

	END


	
