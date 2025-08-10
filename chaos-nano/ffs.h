#ifndef _FFS_H_
#define _FFS_H_

#include "task.h"

class FFS
{
  public:
    void setup();
    int8_t get();
    void set(enum task_id_number id);
    void clear(enum task_id_number id);
};
#endif