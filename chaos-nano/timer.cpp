/*
 * author:  Ansen
 * data:    2025-07-15
 *
 * description: 优化后的定时器库
 *  timer1:
 *    CTC模式，1ms定时
 *    16MHz时钟，64分频，1ms定时
 * history:
 * 1. 修正了定时器分频配置
 * 2. 优化了原子操作范围
 * 3. 改进了任务调度逻辑
 * 4. 添加了错误处理和状态检查
 */

#include <Arduino.h>
#include "timer.h"
#include <util/atomic.h>

// 修正分频配置，使用64分频（CS11=1）
#define TIMER_PRESCALER ((1 << CS11) | (1 << CS10))  // 16MHz时钟，64分频
#define TIMER_COUNTER 249                            // 1 ms 定时
#define TIMER_COUNT_MAX 0xFFFF

static volatile bool running;
static volatile unsigned int icount;
static volatile enum task_id_number cur_id;

static volatile struct {
  unsigned int count;
  bool* flag;
  bool blocked;
} gtimer[TASK_ID_MAX];

// 启动/停止定时器
static void on(bool b) {
  if (b) {
    icount = 0;
    running = true;

    TCNT1 = 0;
    TCCR1B |= TIMER_PRESCALER;  // 启动定时器
  } else {
    TCCR1B &= ~(TIMER_PRESCALER);  // 停止定时器

    running = false;
  }
}

// 调整,并选择下一个任务
static enum task_id_number adj_select(unsigned int escape) {
  unsigned int min = TIMER_COUNT_MAX;
  enum task_id_number current = TASK_ID_MAX;

  // 遍历所有任务，寻找最小的有效计数
  for (int8_t i = 0; i < TASK_ID_MAX; i++) {
    unsigned int count = gtimer[i].count;

    if (count == TIMER_COUNT_MAX)
      continue;  // 跳过未激活的任务

    count = (count <= escape) ? 0 : (count - escape);

    gtimer[i].count = count;

    if (min > count) {
      min = count;
      current = i;
    }
  }

  return current;
}

// 标记 id 的延迟已经完成，并根据标志位设置该任务的状态
static void complete_delay(enum task_id_number id) {
  // 标记任务完成
  gtimer[id].count = TIMER_COUNT_MAX;
  if (gtimer[id].flag) {
    *(gtimer[id].flag) = true;
  }

  // 如果任务是阻塞的，通知FFS
  if (gtimer[id].blocked) {
    task.setRun(id);
  }
}

static unsigned int switch_delay (unsigned int escape) {
  // 处理任务切换
  enum task_id_number id;

  // 最小化原子块范围
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    id = cur_id;

    // 检查当前任务延迟是否有效，如果有效标记该延迟完成
    if (id < TASK_ID_MAX) {
      // 标记任务完成
      complete_delay(id);
    }

    while (1) {
      // 选择下一个任务
      id = adj_select(escape);

      if (id == TASK_ID_MAX) {
        // 没有更多任务，停止定时器
        on(false);
        cur_id = TASK_ID_MAX;
      } else {
        escape = gtimer[id].count;
        // 如果当前被选择的延迟也到期，则标记选择的延迟并重新选择
        if (!escape) {
          complete_delay(id);
          continue;
        }

        // 否则设置选取的延迟为当前延迟
        cur_id = id;
      }

      break;
    }
  }
}

void Timer::setup() {
  running = false;
  icount = 0;
  cur_id = TASK_ID_MAX;

  // 初始化所有定时器任务
  for (int8_t i = 0; i < TASK_ID_MAX; i++) {
    gtimer[i].count = TIMER_COUNT_MAX;
    gtimer[i].flag = NULL;
    gtimer[i].blocked = false;
  }

  // 配置Timer1为普通模式
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;

  TCCR1B |= (1 << WGM12);  // 设置CTC模式
  OCR1A = TIMER_COUNTER;

  TIMSK1 |= (1 << OCIE1A);  // 启用定时器溢出中断
}

ISR(TIMER1_COMPA_vect) {  // 定时器溢出中断服务函数
  static volatile bool inISR = false;

  icount++;
  if(!inISR){
    inISR = true;

    unsigned int count = gtimer[cur_id].count;
    if(icount >= count){
      icount = icount - count;
      switch_delay (count);
    }

    inISR = false;
  }
}

void Timer::create(enum task_id_number id, int msecond, bool* flag, bool blocked) {
  // 检查参数有效性
  if (id >= TASK_ID_MAX || msecond <= 0 || msecond == TIMER_COUNT_MAX) {
    Serial.println("[gtimer:] Error: invalid parameters");
    return;
  }

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    // 设置新任务
    gtimer[id].count = msecond + icount;
    gtimer[id].flag = flag;
    gtimer[id].blocked = blocked;

    // 重新选择最优任务
    enum task_id_number new_id = adj_select(0);

    cur_id = new_id;
  }

  // 如果任务是阻塞的，通知FFS
  if (blocked) {
    task.setBlock(id);
  }

  // 如果定时器未运行，启动它
  if (!running) {
    on(true);
  }
}

void Timer::cancel(enum task_id_number id) {
  // 检查参数有效性
  if (id >= TASK_ID_MAX) {
    return;
  }

  // 标记任务为无效
  bool is_active = (id == cur_id);
  gtimer[id].count = TIMER_COUNT_MAX;

  // 如果任务是阻塞的，通知FFS
  if (gtimer[id].blocked) {
    task.setRun(id);
  }

  // 如果取消的是当前活动任务，需要重新选择
  if (is_active) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      unsigned int tc = gtimer[id].count;
      enum task_id_number new_id = adj_select(0);
      cur_id = new_id;
      // 如果没有更多任务，停止定时器
      if (new_id == TASK_ID_MAX) {
        on(false);
      }
    }
  }
}