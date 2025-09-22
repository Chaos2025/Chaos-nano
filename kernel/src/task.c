/* SPDX-License-Identifier: MIT */
/******************************************************************************
 *
 * Name: task.c
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
/*
 * author:  Lijun An
 * data:    2025-07-15
 */

#include "../inc/task.h"
#include "../inc/lock.h"
#include "../inc/common.h"

static volatile unsigned char taskPriority;
static const signed char ffsTable[256] = {
    -1, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0};

static volatile struct
{
  bool block : 1;
} status[TASK_ID_MAX];

void task_init(void)
{
  taskPriority = 0;
  for (int i = 0; i < TASK_ID_MAX; i++)
  {
    status[i].block = true;
  }
}

signed char task_getNextPriority(void)
{
  signed char ffs;

  LOCKED_BLOCK()
  {
    ffs = ffsTable[taskPriority];
  }

  return ffs;
}

void task_setBlock(enum task_id_number id)
{
  LOCKED_BLOCK()
  {
    status[id].block = true;
    taskPriority &= ~(1 << id);
  }
}

void task_setRun(enum task_id_number id)
{
  LOCKED_BLOCK()
  {
    status[id].block = false;
    taskPriority |= 1 << id;
  }
}

void task_runOnce(enum task_id_number id)
{
  LOCKED_BLOCK()
  {
    taskPriority |= 1 << id;
  }
}

void task_stopOnce(enum task_id_number id)
{
  LOCKED_BLOCK()
  {
    taskPriority &= ~(1 << id);
  }
}

void task_restoreAll(void)
{
  LOCKED_BLOCK()
  {
    for (int i = 0; i < TASK_ID_MAX; i++)
    {
      if (status[i].block)
      {
        taskPriority &= ~(1 << i);
      }
      else
      {
        taskPriority |= 1 << i;
      }
    }
  }
}
