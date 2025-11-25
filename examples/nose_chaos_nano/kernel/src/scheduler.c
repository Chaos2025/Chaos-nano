/* SPDX-License-Identifier: MIT */
/******************************************************************************
*
* Name: scheduler.c - Priority-Based Scheduler
*
* Copyright (C) 2025 Lijun An
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*****************************************************************************/

/**
* @file scheduler.c
* @author Lijun An
* @date 2025-09-18
* @brief Priority-Based Scheduler
*
* @details:
* This is the main function of the whole system.
* When the chip resets or starts up, it run from the function 'start_kernel()'.
* Within the main function, hardware initialization is performed,
* and the hardware devices that need to be activated before the scheduler runs are started.
* 
* If the function 'start_kernel()' is called externally, 
* the board initialization can also be placed before the 'start_kernel()' function is called.
*/

#include "../inc/task.h"
#include "../inc/time.h"

#include "oled.h"
#include "bat.h"
#include "stm8_i2c_sht21.h"
#include "tvoc.h"
#include "handle.h"

extern void board_init(void);
extern void device_on(void);
extern void sleep_cpu(void);

void start_kernel(void)
{
  board_init();
  
  task_init();
  time_init();
  
  device_on();
  
  while (1)
  {
    switch (task_getNextPriority())
    {
    case TASK_ID_POWER:
      powerOnHandle();
      break;
      
    case TASK_ID_KEY:
      keyHandle();
      break;
      
    case TASK_ID_SHT21:
      Get_TempHum();
      break;
      
    case TASK_ID_TVOC:
      Get_Tvoc();
      break;
      
    case TASK_ID_BAT:
      Check_Charge_Bat();
      break;
      
    case TASK_ID_DISP:
      //bat_pic(3,50, pow_bat[1]);
      break;
      
    default:
      task_restoreAll();
      if (IDLE_PRI == task_getNextPriority())
      {
        sleep_cpu();
      }
      break;
    }
  }
}
