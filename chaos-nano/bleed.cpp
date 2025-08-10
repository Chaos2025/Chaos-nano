/*
 * author:  Ansen
 * data:    2025-07-15
 *
 * description: 
 *  PIN:  
 *      3 <===> Bleed valve (not keep working for more than 3s)
 */

#include <Arduino.h> 
#include "bleed.h"
#include "timer.h"
#include "ffs.h"

#define BLEED_VALVE 3
#define ON    1
#define OFF   0

extern Timer timer;
extern FFS ffsb;

static bool time_flag;
static struct{
  bool running;
  bool cur_state;
  bool time_flag;
} status;

void Bleed::setup()
{
  pinMode(BLEED_VALVE, OUTPUT);

  status.running = false;
  status.cur_state = false;
  status.time_flag = false;
}

void Bleed::on(bool b)
{
  if(b){
    ffsb.set(TASK_ID_BLEED);
    digitalWrite(BLEED_VALVE, ON);
    Serial.println("[Bleed:] on(true)");
    timer.create(TASK_ID_BLEED, 2000, &status.time_flag, true);

    status.running = true;
    status.cur_state = true;
  } else {
    ffsb.clear(TASK_ID_BLEED);
    digitalWrite(BLEED_VALVE, OFF);
    timer.cancel(TASK_ID_BLEED);

    status.running = false;
    status.cur_state = false;
  }
}

void Bleed::loop()
{
  if(status.time_flag){
    status.time_flag = false;
    timer.create(TASK_ID_BLEED, 2000, &status.time_flag, true);
    Serial.println("\n\n\n[Bleed:] loop(true)\n\n\n");
  }else{
    Serial.println("[Bleed:] loop(false)");
    ffsb.clear(TASK_ID_BLEED);
  }
}