/*
 * author:  Ansen
 * data:    2025-08-15
 *
 * description: 
 *  The delay function is implemented, and up to 8 tasks can be delayed at the same time.
 *  using timer1:
 *    CTC mode，1ms
 *    16MHz，64 prescale
 *
 * history
 *  1. Timer mode changed
 *  2. Use interrupt flag to mark the interrupt expiration handler
 */

#include <Arduino.h>
#include "timer.h"
#include <util/atomic.h>

#define TIMER_PRESCALER ((1 << CS11) | (1 << CS10))  
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

// start/stop the timer1
static void on(bool b) {
  if (b) {
    icount = 0;
    running = true;

    TCNT1 = 0;
    TCCR1B |= TIMER_PRESCALER;  // start
  } else {
    TCCR1B &= ~(TIMER_PRESCALER);  // stop

    running = false;
  }
}

// adjust the count and find the next 
static enum task_id_number adj_select(unsigned int escape) {
  unsigned int min = TIMER_COUNT_MAX;
  enum task_id_number current = TASK_ID_MAX;

  // walk all to find the minimum
  for (int8_t i = 0; i < TASK_ID_MAX; i++) {
    unsigned int count = gtimer[i].count;

    if (count == TIMER_COUNT_MAX)
      continue;  // 跳过未激活的任务

    count = (count <= escape) ? 0 : (count - escape);

    gtimer[i].count = count;

    if (min > count) {
      min = count;
      current = i;
    }
  }

  return current;
}

// marking the id when it is completed, and resuming the task
static void complete_delay(enum task_id_number id) {
  gtimer[id].count = TIMER_COUNT_MAX;
  if (gtimer[id].flag) {
    *(gtimer[id].flag) = true;
  }

  if (gtimer[id].blocked) {
    task.setRun(id);
  }
}

static unsigned int switch_delay (unsigned int escape) {
  enum task_id_number id;

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
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

void Timer::setup() {
  running = false;
  icount = 0;
  cur_id = TASK_ID_MAX;

  for (int8_t i = 0; i < TASK_ID_MAX; i++) {
    gtimer[i].count = TIMER_COUNT_MAX;
    gtimer[i].flag = NULL;
    gtimer[i].blocked = false;
  }

  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;

  TCCR1B |= (1 << WGM12);  // CTC mode
  OCR1A = TIMER_COUNTER;

  TIMSK1 |= (1 << OCIE1A);
}

ISR(TIMER1_COMPA_vect) {
  static volatile bool inISR = false;

  icount++;
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

void Timer::create(enum task_id_number id, int msecond, bool* flag, bool blocked) {
  if (id >= TASK_ID_MAX || msecond <= 0 || msecond == TIMER_COUNT_MAX) {
    Serial.println("[gtimer:] Error: invalid parameters");
    return;
  }

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    gtimer[id].count = msecond + icount;
    gtimer[id].flag = flag;
    gtimer[id].blocked = blocked;

    enum task_id_number new_id = adj_select(0);

    cur_id = new_id;
  }

  if (blocked) {
    task.setBlock(id);
  }

  if (!running) {
    on(true);
  }
}

void Timer::cancel(enum task_id_number id) {
  if (id >= TASK_ID_MAX) {
    return;
  }

  bool is_active = (id == cur_id);
  gtimer[id].count = TIMER_COUNT_MAX;

  if (gtimer[id].blocked) {
    task.setRun(id);
  }

  if (is_active) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      unsigned int tc = gtimer[id].count;
      enum task_id_number new_id = adj_select(0);
      cur_id = new_id;

      if (new_id == TASK_ID_MAX) {
        on(false);
      }
    }
  }
}