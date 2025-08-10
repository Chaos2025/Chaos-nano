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
#include "ffs.h"
#include "bleed.h"

// Compressor compressor;
Timer timer;
Test test;
FFS ffsb;
Bleed bleed;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  board_init();
  test.on(true);
  bleed.on(true);

  set_sleep_mode(SLEEP_MODE_IDLE);
  sleep_enable();
}

void loop() {
  // put your main code here, to run repeatedly:
  switch(ffsb.get()){
    case TASK_ID_BLEED:
      // Serial.println("[main:] loop(TASK_ID_BLEED)");
      bleed.loop();
      break;

    case TASK_ID_TEST:
      // Serial.println("[main:] loop(TASK_ID_TEST)");
      test.loop();
      break;

    default:
      // Serial.println("[main:] loop(sleep)");
      sleep_cpu();
      break;
  }

}
 
void board_init(void)
{
  noInterrupts(); //禁止所有中断

  ffsb.setup();
  timer.setup();
  // compressor.setup();
  test.setup();
  bleed.setup();

  interrupts();               //允许所有中断
  // test.on(true);
}