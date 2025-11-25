# Chaos-nano
`Chaos` is an operating system designed based on the concept of `libos`. And `Chaos-nano` is a lightweight and easy-to-use operating system specially designed for `MCUs` with less memory.

# Introduction

***Without dynamic memory management*** 

It is for `MCUs` with less memory. So no functions with dynamic memory operations such as `malloc/free` are used in the system, and static arrays are used as an alternative. 

When planning in development, it is necessary to have a enough array capacity to suit the project. For example, a array is assigned to tasks in a timer; When a task is about to be delayed, the timer uses one in the array to save the relevant parameters.

***Simple task management***

Since the system uses static arrays instead of dynamic memory management,  task's `IDs` identify various resources of the task, such as the task `ID` to find the timer resources , and to determine the task priority.

The status of a task in the system currently has only one `block` flag for scheduling with the scheduler: tasks are not scheduled by the scheduler when `block` is set to 1, or tasks are scheduled according to priority when `block` is 0.

Tasks in the system are divided into user tasks and device tasks:

+ User tasks : a series of operations or processes that need to be completed to achieve a specific goal;
+ Device tasks : tasks automatically initiated and executed by devices or systems, usually to support user tasks or realize the functions of the device itself (such as maintenance and monitoring). It focuses on the specific operation logic of the device without direct user intervention;

***Simple Priority-Based Scheduler***

The system comes with a priority summary of a single byte (8 bits) in length - each bit of which corresponds to a priority, up to a maximum of 8 priorities. When a priority is set, it means that the task corresponding to the priority is ready; and the task with the highest priority will be selected to run.

The system is for `MCUs` with less memory, so the system is not allocate a stack for each task, there is only one stack for all tasks. So although the system has a priority-based scheduler, it is not a preemptive operating system - scheduling occurs after a task returns/exits; Therefore, it is recommended to use the state machine mode in task design: decompose the task into multiple small stages; When each small stage is over, the scheduler is returned, so that certain real-time requirements can be met.

***Timing Function***

A delay function is provided in the system. Tasks can perform blocking/non-blocking delay operations for themselves in delay functions. When the delay is blocked, if the task returns to the schedule, the task will be in the blocked state and will no longer be scheduled. After the delay is over, it unblocks itself and is rescheduled - this increases the utilization of the `MCU`.

The timing accuracy is 1ms.

***Simple Power Management***

There is no idle status, but `MCU` can be into a low-power mode.

When the scheduler does not acquire any ready tasks, it goes into a low-power state. By modifying the sleep flag (for example: "SLEEP_MODE_IDLE"), you can change the level to enter low power.

## Application Video

+ Pressure Vessel Monitoring System

<div align="center">
  <img src="pic/product_show.gif">
</div>

+ Handheld VOC Detection Devices

<div align="center">
  <img src="pic/voc.gif">
</div>
