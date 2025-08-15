#ifndef _TASK_H_
#define _TASK_H_

#include <Arduino.h>

enum task_id_number{
  // device's task
  TASK_ID_DEV_BLEED,

  // user's task
  TASK_ID_TEST,
  TASK_ID_MAX,
};

#define IDLE_PRI   (-1)

class Task
{
  public:
    void setup();
    int8_t getNextPriority();
    void setBlock(enum task_id_number id);
    void setRun(enum task_id_number id);
    void runOnce(enum task_id_number id);
    void stopOnce(enum task_id_number id);
    void restoreAll();
};

extern Task task;
#endif