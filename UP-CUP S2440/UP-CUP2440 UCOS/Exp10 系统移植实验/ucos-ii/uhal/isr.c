/***************************************************************************\
	Copyright (c) 2004-2007 threewater@up-tech.com, All rights reserved.
	by threewter	2004.5.12
\***************************************************************************/
/***************************************************************************\
    #说明:中断处理函数
    #接口函数
	----------------------------------  Bug  --------------------------------------

	----------------------------------  TODO list  --------------------------------------
	2004-5-12	对于邋IO口的多中断源的共享
	----------------------------------修正--------------------------------------

	2004-5-12	1、移植
				2、改变了中断函数的定义，添加了一个标志中断号的参数
				3、添加了void ISR_Init(void)函数定义，系统初始化中断的时候调用

\***************************************************************************/
#include "../inc/drv/reg2410.h"
#include "isr.h"
#include "includes.h"
#include <string.h>


typedef void (*mask_func_t)(unsigned int);

typedef struct{
	Interrupt_func_t InterruptHandlers;
	void* data;
	int valid;		//设置中断1=有效0=无效
	mask_func_t mask;
	mask_func_t unmask;
	mask_func_t ack_irq;
}struct_InterruptFunc;

static struct_InterruptFunc InterruptFunc[NR_IRQS]={NULL,};

#define GetISROffsetClr()	rINTOFFSET

#define	ClearPending(x)	do{rSRCPND = (1u << (x));	rINTPND = rINTPND;}while(0)

#define EINT_OFFSET(x)		((x) - NORMAL_IRQ_OFFSET + 4)
#define SUBIRQ_OFFSET(x)	((x) - EXT_IRQ_OFFSET)

#define EXTINT_MASK	0x7

//--------------------------------IRQ for s3c2410---------------------------------//

static int __inline fixup_irq_num(int irq)
{
	if (irq < IRQ_EINT4) return irq;
	else return ((irq + 4) - NORMAL_IRQ_OFFSET);
}

static void __inline set_gpios(int irq, int pullup)
{
	int shift;
	if (irq < 8) {
		shift = 2*irq;
		rGPFCON &= ~(0x3 << shift);
		rGPFCON |= (0x2 << shift);
		//GPFUP &= ~(GRAB_PULLUP(pullup) << irq); modify by threewater
		rGPFUP &= ~(1<< irq);
		rGPFUP |= (GRAB_PULLUP(pullup) << irq);
	} else {
		shift = 2*(irq - 8);
		rGPGCON &= ~(0x3 << shift);
		rGPGCON |= (0x2 << shift);
		//GPGUP &= ~(GRAB_PULLUP(pullup) << (irq - 8));
		rGPGUP &= ~(1<< (irq - 8));
		rGPGUP |= (GRAB_PULLUP(pullup) << (irq - 8));
	} 
}

int set_external_irq(int irq, int edge, int pullup)
{
	int real_irq, reg_ofs, shift;
	volatile U32 *extint = &rEXTINT0;

	if (((irq < IRQ_EINT0) && (irq > IRQ_EINT23)) ||
	    ((irq > IRQ_EINT3) && (irq < IRQ_EINT4)))
		return FAIL;

	real_irq = fixup_irq_num(irq);

	set_gpios(real_irq, pullup);

	reg_ofs = (real_irq / 8);
	shift = 4 * (real_irq - 8 * reg_ofs);
	extint += reg_ofs;

	*extint &= ~(EXTINT_MASK << shift);
	*extint |= (edge << shift);

	if (irq < 4) {
		rSRCPND |= (1 << real_irq);
		rINTPND |= (1 << real_irq);
	} else {
		rEINTPEND |= (1 << real_irq);
	}
	InterruptFunc[irq].valid=1;

	return 0;
}



/*
 * Defined irq handlers
 */
static void ack_irq(unsigned int irq)
{
	rSRCPND = (1 << irq);
	rINTPND = (1 << irq);
}

static void mask_irq(unsigned int irq)
{
	rINTMSK |= (1 << irq);
}

static void unmask_irq(unsigned int irq)
{
	rINTMSK &= ~(1 << irq);
}

/* for EINT? */
static void EINT4_23ack_irq(unsigned int irq)
{
	irq = EINT_OFFSET(irq);
	rEINTPEND = (1 << irq);

	if (irq < EINT_OFFSET(IRQ_EINT8)) {
		ClearPending(SHIFT_EINT4_7);
	} else {
		ClearPending(SHIFT_EINT8_23);
	}
}

static void EINT4_23mask_irq(unsigned int irq)
{
	irq = EINT_OFFSET(irq);
	rEINTMASK |= (1 << irq);
}

static void EINT4_23unmask_irq(unsigned int irq)
{
	rEINTMASK &= ~(1 << EINT_OFFSET(irq));

	if (irq < IRQ_EINT8) {
	  rINTMSK &= ~(1 << SHIFT_EINT4_7);
	} else {
	  rINTMSK &= ~(1 << SHIFT_EINT8_23);
	}
}


/* for sub_IRQ */
static void SUB_ack_irq(unsigned int irq)
{
	rSUBSRCPND = (1 << SUBIRQ_OFFSET(irq));

	if (irq <= IRQ_ERR0){
		ClearPending(IRQ_UART0);
	}else if (irq <= IRQ_ERR1){
		ClearPending(IRQ_UART1);
	}else if (irq <= IRQ_ERR2){
		ClearPending(IRQ_UART2);
	} else {	/* if ( irq <= IRQ_ADC_DONE ) { */
		ClearPending(IRQ_ADCTC);
	}
}

static void SUB_mask_irq(unsigned int irq)
{
	rINTSUBMSK |= (1 << SUBIRQ_OFFSET(irq));
}

static void SUB_unmask_irq(unsigned int irq)
{
	rINTSUBMSK &= ~(1u << SUBIRQ_OFFSET(irq));

	if (irq <= IRQ_ERR0) {
		rINTMSK &= ~(1u << IRQ_UART0); 
        } else if (irq <= IRQ_ERR1) {
		rINTMSK &= ~(1u << IRQ_UART1);
	} else if (irq <= IRQ_ERR2){
	    	rINTMSK &= ~(1u << IRQ_UART2);
        } else {	/* if ( irq <= IRQ_ADC_DONE ) { */
		rINTMSK &= ~(1u << IRQ_ADCTC);
        }
}

__inline unsigned int get_subIRQ(int irq, int begin, int end, int fail_irq) {
	int i;

	for(i=begin; i <= end; i++) {
	  if (irq & (1 << i))
	    return (EXT_IRQ_OFFSET + i);
	}
	return fail_irq;
}

__inline unsigned int get_extIRQ(int irq, int begin, int end, int fail_irq) {
	int i;

	for(i=begin; i <= end; i++) {
	  if (irq & (1 << i))
	    return (NORMAL_IRQ_OFFSET - 4 + i);
	}
	return fail_irq;
}

static unsigned int fixup_irq(int irq) {
    unsigned int ret;
    unsigned long sub_mask, ext_mask;

    switch (irq) {
    case IRQ_UART0:
      sub_mask = rSUBSRCPND & ~rINTSUBMSK;
      ret = get_subIRQ(sub_mask, 0, 2, irq);
      break;
    case IRQ_UART1:
      sub_mask = rSUBSRCPND & ~rINTSUBMSK;
      ret = get_subIRQ(sub_mask, 3, 5, irq);
      break;
    case IRQ_UART2:
      sub_mask = rSUBSRCPND & ~rINTSUBMSK;
      ret = get_subIRQ(sub_mask, 6, 8, irq);
      break;
    case IRQ_ADCTC:
      sub_mask = rSUBSRCPND & ~rINTSUBMSK;
      ret = get_subIRQ(sub_mask, 9, 10, irq);
      break;
    case IRQ_EINT4_7:
      ext_mask = rEINTPEND & ~rEINTMASK;
      ret = get_extIRQ(ext_mask, 4, 7, irq);
      break;
    case IRQ_EINT8_23:
      ext_mask = rEINTPEND & ~rEINTMASK;
      ret = get_extIRQ(ext_mask, 8, 23, irq);
      break;
    default:
      ret = irq;
    }
	
    return ret;
}

void ISR_Init(void)
{
	int irq;

	//FIQ disable and disable all interrupt
	rINTMOD     = 0x0;                     //All=IRQ mode
	rINTMSK     = BIT_ALLMSK;              //All interrupt is masked.
	rINTSUBMSK  = BIT_SUB_ALLMSK;          //All sub-interrupt is masked

	// Define irq handler
	for (irq=0; irq < NORMAL_IRQ_OFFSET; irq++) {
		InterruptFunc[irq].valid = 1;
		InterruptFunc[irq].ack_irq = ack_irq;
		InterruptFunc[irq].mask =  mask_irq;
		InterruptFunc[irq].unmask = unmask_irq;
	}

	InterruptFunc[IRQ_RESERVED6].valid	= 0;
	InterruptFunc[IRQ_RESERVED24].valid	= 0;

	InterruptFunc[IRQ_EINT4_7].valid	= 0;
	InterruptFunc[IRQ_EINT8_23].valid	= 0;

	InterruptFunc[IRQ_EINT0].valid		= 0;
	InterruptFunc[IRQ_EINT1].valid		= 0;
	InterruptFunc[IRQ_EINT2].valid		= 0;
	InterruptFunc[IRQ_EINT3].valid		= 0;

	for (irq=NORMAL_IRQ_OFFSET; irq < EXT_IRQ_OFFSET; irq++) {
		InterruptFunc[irq].valid	= 0;
		InterruptFunc[irq].ack_irq	= EINT4_23ack_irq;
		InterruptFunc[irq].mask	= EINT4_23mask_irq;
		InterruptFunc[irq].unmask	= EINT4_23unmask_irq;
	}

	for (irq=EXT_IRQ_OFFSET; irq < SUB_IRQ_OFFSET; irq++) {
		InterruptFunc[irq].valid	= 1;
		InterruptFunc[irq].ack_irq	= SUB_ack_irq;
		InterruptFunc[irq].mask	= SUB_mask_irq;
		InterruptFunc[irq].unmask	= SUB_unmask_irq;
	}  

}

//--------------------------------IRQ core---------------------------------//

#if 0
int GetISROffsetClr()
{
	//计算中断的偏移地址，高位优先
	int i,ispr=rI_ISPR,tmp=1<<(MAXHNDLRS-1);//temp bit
	for(i=MAXHNDLRS;i>0;i--){
		if(ispr&tmp){
			return i-1;
		}
		tmp>>=1;
	}
	return -1;
}
#endif

int SetISR_Interrupt(int vector, void (*handler)(int, void*), void* data)
{
#if OS_CRITICAL_METHOD == 3
	OS_CPU_SR  cpu_sr;
#endif

	if(vector>NR_IRQS || vector<0)
		return -1;

	if(!InterruptFunc[vector].valid)
		return -1;

	OS_ENTER_CRITICAL();

	InterruptFunc[vector].ack_irq(vector);	//clear pending

	InterruptFunc[vector].InterruptHandlers = handler;
	InterruptFunc[vector].data = data;

	InterruptFunc[vector].unmask(vector);	//enable interrupt

	OS_EXIT_CRITICAL();

	return 0;
}

void ISR_IrqHandler(void)
{
	unsigned int irq=GetISROffsetClr();	//得到中断向量的偏移地址

	irq=fixup_irq(irq);

	if(irq>=NR_IRQS)
		return;
	if(InterruptFunc[irq].InterruptHandlers==NULL){
		InterruptFunc[irq].ack_irq(irq);	//clear pending
		return;
	}

	OSIntEnter();
	 // Call interrupt service routine
	InterruptFunc[irq].InterruptHandlers(irq, InterruptFunc[irq].data);
	InterruptFunc[irq].ack_irq(irq);	//clear pending

	OSIntExit();
}

