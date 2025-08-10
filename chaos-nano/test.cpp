#include <Arduino.h> 
#include "test.h"
#include "timer.h"
#include "ffs.h"

#define LED_PIN 13

static bool flag;
extern Timer timer;
extern FFS ffsb;

void Test::setup()
{
  pinMode(LED_PIN, OUTPUT);
  flag = false;
}

void Test::on(bool b)
{
  digitalWrite(LED_PIN, b);
  timer.create(TASK_ID_TEST, 1000, &flag, true);
}

void Test::loop()
{
  if(flag){
  //   flag = false;
  //   digitalWrite(LED_PIN, digitalRead(LED_PIN) ^ 1);
    timer.create(TASK_ID_TEST, 1000, &flag, true);
    Serial.println("[test:] loop(true)");
  // }else{
    // Serial.println("[test:] loop()");
    // ffsb.clear(TASK_ID_TEST);
  }
}