/*
 * author:  Ansen
 * data:    2025-07-15
 *
 * description: 优化后的定时器库
 * 1. 修正了定时器分频配置
 * 2. 优化了原子操作范围
 * 3. 改进了任务调度逻辑
 * 4. 添加了错误处理和状态检查
 */

#include <Arduino.h> 
#include "timer.h"
#include <util/atomic.h>

// 修正分频配置，使用8分频（CS11=1）
#define TIMER_PRESCALER (1 << CS11)  // 8分频，对应16MHz/8=2MHz
#define TIMER_COUNTER 63035          // 65536-2MHz/1kHz = 63036
#define TIMER_COUNT_MAX 0xFFFF

extern FFS ffsb;

static volatile bool running;
static volatile unsigned int tcount;
static volatile enum task_id_number cur_id;

static volatile struct {
  unsigned int count;
  bool* flag;
  bool blocked;
} timer[TASK_ID_MAX];

// 启动/停止定时器
static void on(bool b)
{
  if(b){
    TCCR1B |= TIMER_PRESCALER;      // 设置8分频
    running = true;
  }
  else{
    TCCR1B &= ~(TIMER_PRESCALER);   // 停止定时器
    running = false;
  }
}

// 调整并选择下一个任务
static enum task_id_number adj_select(unsigned int escape)
{
  unsigned int min = TIMER_COUNT_MAX;
  enum task_id_number current = TASK_ID_MAX;

  // 遍历所有任务，寻找最小的有效计数
  for(int8_t i = 0; i < TASK_ID_MAX; i++){
    unsigned int count = timer[i].count;
    
    if(count == TIMER_COUNT_MAX)
      continue;  // 跳过未激活的任务
      
    if(count < escape) {
      Serial.print("[timer:] Warning: count < escape (");
      Serial.print(count);
      Serial.print(" < ");
      Serial.print(escape);
      Serial.println(")");
      count = 0;  // 防止计数下溢
    } else {
      count -= escape;
    }
    
    timer[i].count = count;
    
    if(min > count){
      min = count;
      current = i;
    }
  }

  return current;
}

void Timer::setup()
{
  running = false;
  tcount = 0;
  cur_id = TASK_ID_MAX;

  // 初始化所有定时器任务
  for(int8_t i = 0; i < TASK_ID_MAX; i++){
    timer[i].count = TIMER_COUNT_MAX;
    timer[i].flag = NULL;
    timer[i].blocked = false;
  }

  // 配置Timer1为普通模式
  TCCR1A = 0;
  TCCR1B = 0;

  TCNT1 = TIMER_COUNTER;        // 预加载定时器
  TIMSK1 |= (1 << TOIE1);        // 启用定时器溢出中断
}

ISR(TIMER1_OVF_vect) {          // 定时器溢出中断服务函数
  TCNT1 = TIMER_COUNTER;        // 重新加载定时器值
  
  // 减少全局计数器
  if(tcount > 0) {
    tcount--;
  } else {
    // 防止计数器下溢
    Serial.println("[timer:] Error: tcount underflow");
    return;
  }

  // 如果计数器归零，处理任务切换
  if (!tcount) {
    enum task_id_number id;
    unsigned int count;
    
    // 最小化原子块范围
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      id = cur_id;
      count = timer[id].count;
      
      // 检查任务是否有效
      if(id < TASK_ID_MAX && timer[id].count != TIMER_COUNT_MAX) {
        // 标记任务完成
        timer[id].count = TIMER_COUNT_MAX;
        if(timer[id].flag) {
          *(timer[id].flag) = true;
        }
        
        // 如果任务是阻塞的，通知FFS
        if(timer[id].blocked) {
          ffsb.set(id);
        }
      }
      
      // 选择下一个任务
      id = adj_select(count);
      cur_id = id;
      
      if (id == TASK_ID_MAX) {
        // 没有更多任务，停止定时器
        on(false);
      } else {
        tcount = timer[id].count;
      }
    }
  } 
}

void Timer::create(enum task_id_number id, int msecond, bool* flag, bool blocked)
{
  // 检查参数有效性
  if(id >= TASK_ID_MAX || msecond <= 0) {
    Serial.println("[timer:] Error: invalid parameters");
    return;
  }

  Serial.print("\n\n[timer:] create msecond(");
  Serial.print(msecond);
  Serial.println(")");

  unsigned int count = msecond;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    // 计算任务到期时间
    unsigned int tc =  timer[cur_id].count;
    unsigned int escape = tc - tcount;

    Serial.print("[timer:] create tc(");
    Serial.print(tc);
    Serial.println(")");
    
    // 如果当前有活动任务，基于当前时间计算
    if(cur_id != TASK_ID_MAX && tc != TIMER_COUNT_MAX) {
      count += escape;
    }
    
    // 设置新任务
    timer[id].count = count;
    timer[id].flag = flag;
    timer[id].blocked = blocked;

    Serial.print("[timer:] create count(");
    Serial.print(count);
    Serial.println(")\n\n");
    
    // 重新选择最优任务
    enum task_id_number new_id = adj_select(escape);

    Serial.print("[timer:] create TASK_ID_BLEED(");
    Serial.print(timer[id].count);
    Serial.println(")");

    cur_id = new_id;
    tcount = timer[new_id].count;
    
    // 如果任务是阻塞的，通知FFS
    if(blocked) {
      ffsb.clear(id);
    }
  }
  
  // 如果定时器未运行，启动它
  if(!running) {
    on(true);
  }
}

void Timer::cancel(enum task_id_number id)
{
  // 检查参数有效性
  if(id >= TASK_ID_MAX) {
    return;
  }

  // 标记任务为无效
  bool was_active = (id == cur_id);
  timer[id].count = TIMER_COUNT_MAX;
  
  // 如果任务是阻塞的，通知FFS
  if(timer[id].blocked) {
    ffsb.set(id);
  }

  // 如果取消的是当前活动任务，需要重新选择
  if(was_active) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      enum task_id_number new_id = adj_select(tcount);
      cur_id = new_id;
      tcount = timer[new_id].count;
    }
    
    // 如果没有更多任务，停止定时器
    if(cur_id == TASK_ID_MAX) {
      on(false);
    }
  }
}