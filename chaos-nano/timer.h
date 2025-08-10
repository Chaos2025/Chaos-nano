#ifndef _TIMER_H_
#define _TIMER_H_

#include "task.h"
#include "ffs.h"

typedef void (*timer_callback)();

class Timer
{
  public:
    void setup(); // 初始化引脚
    void create(enum task_id_number id, int msecond, bool* flag, bool blocked);
    void cancel(enum task_id_number id);
};
#endif