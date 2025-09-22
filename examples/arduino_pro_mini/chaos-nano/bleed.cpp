/*
 * author:  Ansen
 * data:    2025-07-15
 *
 * description: 
 *  This device prints a string for each 2 second
 */

#include <Arduino.h> 
#include "bleed.h"
#include "timer.h"
#include "task.h"

#define DELAY_MS        (1000 * 2)
#define ON    1
#define OFF   0

static bool flag;


void Bleed::setup()
{
  flag = false;
  Serial.println("[BLEED:] INIT");
}

void Bleed::on(bool b)
{
  if(b){
    task.setRun(TASK_ID_DEV_BLEED);
    timer.create(TASK_ID_DEV_BLEED, DELAY_MS, &flag, true);
  } else {
    timer.cancel(TASK_ID_DEV_BLEED);
    task.setBlock(TASK_ID_DEV_BLEED);
  }
}

void Bleed::loop()
{
  if(flag){
    flag = false;
    timer.create(TASK_ID_DEV_BLEED, DELAY_MS, &flag, true);
    Serial.println("[bleed:] loop(true)");
  }
}