/* SPDX-License-Identifier: MIT */
/******************************************************************************
 *
 * Name: time.c - Priority-Based Scheduler
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
 * data:    2025-08-15
 */

#include "../inc/time.h"
#include "../inc/lock.h"
#include "../inc/common.h"

extern void start_timer(void);
extern void stop_timer(void);

#define TIMER_COUNTER 249
#define TIMER_COUNT_MAX 0xFFFF

static volatile bool running;
static volatile unsigned int icount;
static volatile enum task_id_number cur_id;

static volatile struct {
  unsigned int count;
  bool* flag;
  bool blocked;
} gtimer[TASK_ID_MAX];

// start/stop the timer
STATIC_INLINE void on(bool b) 
{
  if (b) {
    icount = 0;
    running = true;

    start_timer();  // start the timer
  } else {
    stop_timer();  // stop the timer

    running = false;
  }
}

// adjust the count and find the next 
static enum task_id_number adj_select(unsigned int escape) 
{
  unsigned int min = TIMER_COUNT_MAX;
  enum task_id_number current = TASK_ID_MAX;

  // walk all to find the minimum
  for (char i = 0; i < TASK_ID_MAX; i++) {
    unsigned int count = gtimer[i].count;

    if (count == TIMER_COUNT_MAX)
      continue;  // skip the inactive task

    count = (count <= escape) ? 0 : (count - escape);

    gtimer[i].count = count;

    if (min > count) {
      min = count;
      current = (enum task_id_number)i;
    }
  }

  return current; //add 2025-11-17
}

// marking the id when it is completed, and resuming the task
static void complete_delay(enum task_id_number id) 
{
  gtimer[id].count = TIMER_COUNT_MAX;
  if (gtimer[id].flag) {
    *(gtimer[id].flag) = true;
  }

  if (gtimer[id].blocked) {
    task_setRun(id);
  }
}

STATIC_INLINE void switch_delay (unsigned int escape) // change 2025-11-17
{
  enum task_id_number id;

  LOCKED_BLOCK() {
    id = cur_id;

    // if id is valid, mark it
    if (id < TASK_ID_MAX) {
      complete_delay(id);
    }

    while (1) {
      // select the next
      id = adj_select(escape);

      if (id == TASK_ID_MAX) {
        // if no more, stop the timer
        on(false);
        cur_id = TASK_ID_MAX;
      } else {
        escape = gtimer[id].count;
        // if the selected id‘s count is zero,mark it and select another
        if (!escape) {
          complete_delay(id);
          continue;
        }

        cur_id = id;
      }

      break;
    }
  }
}

void time_init() 
{
  running = false;
  icount = 0;
  cur_id = TASK_ID_MAX;

  for (char i = 0; i < TASK_ID_MAX; i++) {
    gtimer[i].count = TIMER_COUNT_MAX;
    gtimer[i].flag = NULL;
    gtimer[i].blocked = false;
  }
}

void timer_irq_callback(int msecond) 
{
  static volatile bool inISR = false;

  icount += msecond;
  if(!inISR){
    inISR = true;

    unsigned int count = gtimer[cur_id].count;
    if(icount >= count){
      icount = icount - count;
      switch_delay (count);
    }

    inISR = false;
  }
}

int time_create(enum task_id_number id, int msecond, bool* flag, bool blocked) 
{
  if (id >= TASK_ID_MAX || msecond <= 0 || msecond == TIMER_COUNT_MAX) {
    return -1;
  }

  LOCKED_BLOCK() {
    gtimer[id].count = msecond + icount;
    gtimer[id].flag = flag;
    gtimer[id].blocked = blocked;

    enum task_id_number new_id = adj_select(0);

    cur_id = new_id;
  }

  if (blocked) {
    task_setBlock(id);
  }

  if (!running) {
    on(true);
  }
  
  return 0;
}

void time_cancel(enum task_id_number id) 
{
  if (id >= TASK_ID_MAX) {
    return;
  }

  bool is_active = (id == cur_id);
  gtimer[id].count = TIMER_COUNT_MAX;

  if (gtimer[id].blocked) {
    task_setRun(id);
  }

  if (is_active) {
    LOCKED_BLOCK() {
      unsigned int tc = gtimer[id].count;
      enum task_id_number new_id = adj_select(0);
      cur_id = new_id;

      if (new_id == TASK_ID_MAX) {
        on(false);
      }
    }
  }
}