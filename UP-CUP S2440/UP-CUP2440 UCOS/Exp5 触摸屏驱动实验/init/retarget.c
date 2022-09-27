/*
** Copyright (C) ARM Limited, 2001. All rights reserved.
*/


/*
** This implements a 'retarget' layer for low-level IO.  Typically, this
** would contain your own target-dependent implementations of fputc(),
** ferror(), etc.
** 
** This example provides implementations of fputc(), ferror(),
** _sys_exit(), _ttywrch() and __user_initial_stackheap().
**
** Here, semihosting SWIs are used to display text onto the console 
** of the host debugger.  This mechanism is portable across ARMulator,
** Angel, Multi-ICE and EmbeddedICE.
**
** Alternatively, to output characters from the serial port of an 
** ARM Integrator Board (see serial.c), use:
**
**     #define USE_SERIAL_PORT
**
** or compile with 
**
**     -DUSE_SERIAL_PORT
*/

#include <stdio.h>
#include <string.h>
#include <rt_misc.h>
#include <time.h>
#include "../inc/lib.h"

//#undef DEBUG
//#define DEBUG
#ifdef DEBUG
#define DPRINTF	printf
#else
#define DPRINTF(...)
#endif



struct __FILE { int handle;   /* Add whatever you need here */};



FILE __stdin, __stdout, __stderr;


extern unsigned int bottom_of_heap;     /* defined in heap.s */

int fputc(int ch, FILE *f)
{
	/* Place your implementation of fputc here     */
	/* e.g. write a character to a UART, or to the */
	/* debugger console with SWI WriteC            */
	//if(f == &__stdout)	//for console out put e.g. printf
	//	CONSOLE_PUTC(ch);
     return(0);
	//return ch;
}


int ferror(FILE *f)
{   /* Your implementation of ferror */
    return EOF;
}


void _sys_exit(int return_code)
{
	for(;;);
}


int __raise(int signal, int argument)//void _ttywrch(int ch)
{
	return 0;
}


__value_in_regs struct __initial_stackheap __user_initial_stackheap(
        unsigned R0, unsigned SP, unsigned R2, unsigned SL)
{
    struct __initial_stackheap config;
    
    config.heap_base = (unsigned int)&bottom_of_heap; // defined in heap.s
                                                      // placed by scatterfile   
    config.stack_base = SP;   // inherit SP from the execution environment

    return config;
}

