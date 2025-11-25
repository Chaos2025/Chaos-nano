/* SPDX-License-Identifier: MIT */
/******************************************************************************
 *
 * Name: time.h - Delay with/without block
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
 * @file    time.h
 * @author  Lijun An
 * @date    2025-09-18
 * @brief   Task's definition
 *
 * @details:
 * Each task has a corresponding set of resources in time struct, 
 * which are identified by the task ID. So all tasks can be delayed at the same
 * time, but each task only can be delayed once at a time. If multiple functions
 * in a task and they need a delay for themself, there is a special handling to 
 * integrate the delays into a single timer.
 * 
 * When create a time, the parameter 'blocked' means block the task or not.
 * If 'blocked' is true, when the task returns to scheduler it will not be
 * scheduled any more, until the delay expires. The parameter 'flag' will return
 * true if the delay expires. It can keep tasks away from unexpected scheduling.
 * 
 * The function 'timer_irq_callback()' can be called by timer's interrupt. 
 * It requires a delay of 1 millisecond or a multiple of 1 millisecond.
 */

#ifndef _TIME_H_
#define _TIME_H_

#include "../inc/common.h"
#include "../inc/task.h"

void time_init(void);
int time_create(enum task_id_number id, int msecond, bool *flag, bool blocked);
void time_cancel(enum task_id_number id);

void timer_irq_callback(int msecond);

#endif