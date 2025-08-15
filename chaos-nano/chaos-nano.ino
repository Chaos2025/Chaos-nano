/*
 * author:  Ansen
 * data:    2025-07-15
 *
 * description: 
 *  
 */

#include <util/atomic.h>
#include <avr/sleep.h>
#include "timer.h"
#include "test.h"
#include "task.h"
#include "bleed.h"

// system init
Timer timer;
Task task;

//device
Bleed bleed;
Test test;

#if TASK_ID_MAX >= 8
#error "the number of task's id is too large!"
#endif

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  board_init();
  device_on();

  set_sleep_mode(SLEEP_MODE_IDLE);
  sleep_enable();
}

void loop() {
   switch (task.getNextPriority()) {
    case TASK_ID_DEV_BLEED:
      bleed.loop();
      break;

    case TASK_ID_TEST:
      test.loop();
      break;

    default:
      task.restoreAll();
      if (IDLE_PRI != task.getNextPriority()) {
        sleep_cpu();
      }

      break;
  }
}

void board_init(void) {
  noInterrupts();  //禁止所有中断

  // system
  task.setup();
  timer.setup();

  // device or user's task
  bleed.setup();
  test.setup();

  interrupts();  //允许所有中断
}

void device_on(void) {
  bleed.on(true);
  test.on(true);
}