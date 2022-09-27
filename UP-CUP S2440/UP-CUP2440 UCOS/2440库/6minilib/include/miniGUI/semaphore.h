/**
 * \file semaphore.h
 * \author Wei Yongming <ymwei@minigui.org>
 * \date 2004/02/03
 * 
 * semaphore.h: This header contains the pthread semaphore definitions 
 *              needed to support MiniGUI under uC/OS-II. 
 *
 \verbatim
    Copyright (C) 2004 Feynman Software.

    This file is part of MiniGUI, a lightweight Graphics User Interface 
    support library for real-time embedded Linux.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 \endverbatim
 */

/*
 * $Id: ucos2_semaphore.h,v 1.2 2004/02/25 02:34:49 weiym Exp $
 *
 *             MiniGUI for Linux, uClinux, eCos, and uC/OS-II version 1.5.x
 *             Copyright (C) 1998-2002 Wei Yongming.
 *             Copyright (C) 2002-2004 Feynman Software.
 */

#ifndef UCOSII_SEMAPHORE_H
#define UCOSII_SEMAPHORE_H

#include "common.h"

#ifdef __UCOSII__

#define sem_t ucos2_sem_t

#define SEM_VALUE_MAX	USHRT_MAX

//-----------------------------------------------------------------------------
// Semaphore object definition

typedef struct
{
    void* os_sema;
} sem_t;

//-----------------------------------------------------------------------------
// Semaphore functions

// Initialize semaphore to value.
// pshared is not supported under uC/OS-II.
int sem_init  (sem_t *sem, int pshared, unsigned int value);

// Destroy the semaphore.
int sem_destroy  (sem_t *sem);

// Decrement value if >0 or wait for a post.
int sem_wait  (sem_t *sem);

// Decrement value if >0, return -1 if not.
int sem_trywait  (sem_t *sem);

// Increment value and wake a waiter if one is present.
int sem_post  (sem_t *sem);

// Get current value
int sem_getvalue  (sem_t *sem, int *sval);

#if 0 /* We do not implement the named semaphore functions */

//-----------------------------------------------------------------------------
// Named semaphore functions

// Open an existing named semaphore, or create it.
sem_t *sem_open  (const char *name, int oflag, ...);

// Close descriptor for semaphore.
int sem_close  (sem_t *sem);

// Remove named semaphore
int sem_unlink  (const char *name);

//-----------------------------------------------------------------------------
// Special return value for sem_open()

#define SEM_FAILED      ((sem_t *)NULL)

#endif /* We do not implement the named semaphore functions */

#endif /* __UCOSII__ */
#endif /* UCOSII_SEMAPHORE_H */

