/*
 * author:  Ansen
 * data:    2025-07-15
 *
 * description: 
 *  This is the main file. 
 *  It implemented the main loop of Chaos-nano
 */

#include <util/atomic.h>
#include <avr/sleep.h>
#include "timer.h"
#include "test.h"
#include "task.h"
#include "bleed.h"

// system define
Timer timer;
Task task;

//device define
Bleed bleed;
Test test;

#if TASK_ID_MAX >= 8
#error "the number of task's id is too large!"
#endif

void setup() {
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
  noInterrupts();  //Close all interruptions

  // system
  task.setup();
  timer.setup();

  // device or user's task
  bleed.setup();
  test.setup();

  interrupts();  //Open all interruptions
}

void device_on(void) {
  bleed.on(true);
  test.on(true);
}