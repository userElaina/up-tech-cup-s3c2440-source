/*
 * Filename: os_cpu_c.c
 *
 * Porting file for Interniche ChronOS RTOS
 *
 * Copyright 2002 by InterNiche Technologies Inc. All rights reserved.
 * www.iniche.com
 * support@iniche.com
 *
 */

#include "includes.h"      /* from uCOS directory */

//#include "ipport.h"        /* from Interniche directory */

/* ARM CPU "supervisor" mode for PSR registers, with INTs enabled */
#define       SUPMODE       0x13


/*
 * INITIALIZE A TASK'S STACK
 *
 * Description: This function is called by either OSTaskCreate() or OSTaskCreateExt() to initialize the
 *              stack frame of the task being created.  This function is highly processor specific.
 *
 * Arguments  : task          is a pointer to the task code
 *
 *              pdata         is a pointer to a user supplied data area that will be passed to the task
 *                            when the task first executes.
 *
 *              ptos          is a pointer to the top of stack.  It is assumed that 'ptos' points to
 *                            a 'free' entry on the task stack.  If OS_STK_GROWTH is set to 1 then 
 *                            'ptos' will contain the HIGHEST valid address of the stack.  Similarly, if
 *                            OS_STK_GROWTH is set to 0, the 'ptos' will contains the LOWEST valid address
 *                            of the stack.
 *
 *              opt           specifies options that can be used to alter the behavior of OSTaskStkInit().
 *                            (see uCOS_II.H for OS_TASK_OPT_???).
 *
 * Returns    : Always returns the location of the new top-of-stack' once the processor registers have
 *              been placed on the stack in the proper order.
 *
 * Note(s)    : Interrupts are enabled when your task starts executing. You can change this by setting the
 *              PSW to 0x0002 instead.  In this case, interrupts would be disabled upon task startup.  The
 *              application code would be responsible for enabling interrupts at the beginning of the task
 *              code.  You will need to modify OSTaskIdle() and OSTaskStat() so that they enable 
 *              interrupts.  Failure to do this will make your system crash!
 */

OS_STK * OSTaskStkInit (void (*task)(void *pd), void *pdata, OS_STK *ptos, INT16U opt)
{
    unsigned int * stk;

    stk    = (unsigned int *)ptos;          /* Load stack pointer */
    //USE_ARG(opt);
    opt++;

    /* build a stack for the new task */
    *--stk = (unsigned int) task;      /* pc */
    *--stk = (unsigned int) task;      /* lr */
    *--stk = 12;                       /* r12 */
    *--stk = 11;                       /* r11 */
    *--stk = 10;                       /* r10 */
    *--stk = 9;                        /* r9 */
    *--stk = 8;                        /* r8 */
    *--stk = 7;                        /* r7 */
    *--stk = 6;                        /* r6 */
    *--stk = 5;                        /* r5 */
    *--stk = 4;                        /* r4 */
    *--stk = 3;                        /* r3 */
    *--stk = 2;                        /* r2 */
    *--stk = 1;                        /* r1 */
    *--stk = (unsigned int) pdata;     /* r0 */
    *--stk = (SUPMODE);			         /* cpsr */
    *--stk = (SUPMODE);			         /* spsr */

    return ((OS_STK *)stk);
}

#if OS_CPU_HOOKS_EN
/*
*********************************************************************************************************
*                                          TASK CREATION HOOK
*
* Description: This function is called when a task is created.
*
* Arguments  : ptcb   is a pointer to the task control block of the task being created.
*
* Note(s)    : 1) Interrupts are disabled during this call.
*********************************************************************************************************
*/
void OSTaskCreateHook (OS_TCB *ptcb)
{
    ptcb = ptcb;                       /* Prevent compiler warning                                     */
}


/*
*********************************************************************************************************
*                                           TASK DELETION HOOK
*
* Description: This function is called when a task is deleted.
*
* Arguments  : ptcb   is a pointer to the task control block of the task being deleted.
*
* Note(s)    : 1) Interrupts are disabled during this call.
*********************************************************************************************************
*/
void OSTaskDelHook (OS_TCB *ptcb)
{
    ptcb = ptcb;                       /* Prevent compiler warning                                     */
}

/*
*********************************************************************************************************
*                                           TASK SWITCH HOOK
*
* Description: This function is called when a task switch is performed.  This allows you to perform other
*              operations during a context switch.
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts are disabled during this call.
*              2) It is assumed that the global pointer 'OSTCBHighRdy' points to the TCB of the task that
*                 will be 'switched in' (i.e. the highest priority task) and, 'OSTCBCur' points to the 
*                 task being switched out (i.e. the preempted task).
*********************************************************************************************************
*/
void OSTaskSwHook (void)
{
}

/*
*********************************************************************************************************
*                                           STATISTIC TASK HOOK
*
* Description: This function is called every second by uC/OS-II's statistics task.  This allows your 
*              application to add functionality to the statistics task.
*
* Arguments  : none
*********************************************************************************************************
*/
void OSTaskStatHook (void)
{
}

/*
*********************************************************************************************************
*                                               TICK HOOK
*
* Description: This function is called every tick.
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts may or may not be ENABLED during this call.
*********************************************************************************************************
*/

void OSTimeTickHook (void)
{
}

void OSTCBInitHook (OS_TCB *ptcb)
{
    ptcb = ptcb;     /* Prevent Compiler warning */
}

void OSTaskIdleHook (void)
{
}
void OSInitHookEnd (void)
{
}
void OSInitHookBegin (void)
{
}
#endif

