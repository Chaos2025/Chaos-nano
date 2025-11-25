/* SPDX-License-Identifier: MIT */
/******************************************************************************
 *
 * Name: task.h - Task's definition
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
 * @file    scheduler.c
 * @author  Lijun An
 * @date    2025-09-18
 * @brief   Task's definition
 *
 * @details:
 * The task ID identifies the uniqueness of the task. 
 * The system does not have dynamic memory management.So the task ID is used to 
 * represent both the priority of the task and also serves as the sequence number 
 * for task resources in other subsystems.
 * 
 * There are three types task in the system:
 * - user task:
 *  These tasks are for data processing.
 * - device task:
 *  When a interrupt arrived, it wakes up a specified task for device driver.
 *  Or executing the driver when the timer expires.
 * - idle task: 
 *  The idle task is not take any task ID, and its priority is lowest.
 */

#ifndef _TASK_H_
#define _TASK_H_

enum task_id_number
{
  // device's task
  TASK_ID_POWER,
  TASK_ID_KEY,
  TASK_ID_SHT21,
  TASK_ID_TVOC,
  TASK_ID_BAT,

  // user's task
  TASK_ID_DISP,
  TASK_ID_MAX,
};

#define IDLE_PRI (-1)

void task_init(void);
signed char task_getNextPriority(void);
void task_setBlock(enum task_id_number id);
void task_setRun(enum task_id_number id);
void task_runOnce(enum task_id_number id);
void task_stopOnce(enum task_id_number id);
void task_restoreAll(void);

#endif