#include <Arduino.h> 
#include "test.h"
#include "timer.h"
#include "task.h"

static bool flag;

void Test::setup()
{
  flag = false;
  Serial.println("[TEST:] INIT");
}

void Test::on(bool b)
{
  timer.create(TASK_ID_TEST, 1000, &flag, true);
}

void Test::loop()
{
  if(flag){
    timer.create(TASK_ID_TEST, 1000, &flag, true);
    Serial.println("[test:] loop(true)");
  }
}