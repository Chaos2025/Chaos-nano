/*
 * author:  Ansen
 * data:    2025-07-15
 *
 * description: 
 *  
 */

#include <Arduino.h>
#include "task.h"
#include <util/atomic.h>

static volatile uint8_t taskPriority;
static const int8_t ffsTable[256] = {
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
  4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0
};

static struct {
  bool block : 1;
} status[TASK_ID_MAX];

void Task::setup() {
  taskPriority = 0;
  for(int i = 0; i < TASK_ID_MAX; i++){
    status[i].block = true;
  }
}

int8_t Task::getNextPriority() {
  uint8_t ffs;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    ffs = ffsTable[taskPriority];
  }
  return ffs;
}

void Task::setBlock(enum task_id_number id) {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    status[id].block = true;
    taskPriority &= ~(1 << id);
  }
}

void Task::setRun(enum task_id_number id) {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    status[id].block = false;
    taskPriority |= 1 << id;
  }
}

void Task::runOnce(enum task_id_number id) {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    taskPriority |= 1 << id;
  }
}

void Task::stopOnce(enum task_id_number id) {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    taskPriority &= ~(1 << id);
  }
}

void Task::restoreAll() {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    for (int i = 0; i < TASK_ID_MAX; i++) {
        if(status[i].block){
          taskPriority &= ~(1 << i);
        }else{
          taskPriority |= 1 << i;
        }
    }
  }
}