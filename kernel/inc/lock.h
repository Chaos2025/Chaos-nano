/* SPDX-License-Identifier: MIT */
/******************************************************************************
 *
 * Name: lock.h - Lock operation & definition
 *
 * Copyright (C) 2025 Lijun An
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *****************************************************************************/
/**
 * @file    lock.h
 * @author  Lijun An
 * @date    2025-09-18
 * @brief   Task's definition
 *
 * @details:
 * Defines the critical section of the code block, where operations within 
 * this code block are not affected by other operations.
 */

#ifndef _LOCK_H_
#define _LOCK_H_

extern unsigned int enter_critical_section(void);
extern void exit_critical_section(unsigned int primask);

#ifdef __GNUC__
#define ATOMIC_RESTORESTATE unsigned int sreg_save \
	__attribute__((__cleanup__(exit_critical_section))) = enter_critical_section()

#define LOCKED_BLOCK() \
    for ( ATOMIC_RESTORESTATE, __ToDo = 1; \
	    __ToDo ; __ToDo = 0 )

#else
#define LOCKED_BLOCK()  \
        for(int i = 1, primask = enter_critical_section(); i; i--)  \
            if(!i){ \
                exit_critical_section(primask); \
            }else

#endif /* __GNUC__ */

#endif
