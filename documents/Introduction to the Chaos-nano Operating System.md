# `Chaos-nano`: A Lightweight Cooperative Asynchronous Operating System Designed for Resource-Constrained Microcontrollers

## Project Background

The birth of `Chaos-nano` originated from a challenging embedded development practice: I planned to develop a prototype of a control project based on the `Arduino Pro Mini`, whose core chip is the `Atmega328p` from `Microchip`. This classic 8-bit `MCU` is widely used in the embedded field, but its hardware resources are extremely limited — equipped with only 32KB of Flash storage and 2KB of RAM, a configuration that can be called a "resource-constrained scenario" in embedded development.

A sharp contradiction soon emerged between the project's core requirements and the hardware resources: on one hand, the prototype needed to achieve precise delay control for multiple tasks through timers. For example, the sensor data collection interval and actuator action timing all had clear requirements for time accuracy. On the other hand, traditional blocking delays (such as the `delay()` function) would put the CPU into an idle loop during waiting, not only wasting valuable computing resources but also continuously consuming power, which was completely inconsistent with the project's "low power consumption" design goal. More critically, due to resource constraints, 8-bit microcontrollers have long struggled to truly apply the concept of an "operating system" to practical development. Most embedded `OS` on the market, even those claiming to be "lightweight," require at least several KB of RAM and more than ten KB of Flash support, far exceeding the carrying capacity of the `Atmega328p`. This left me in a dilemma where "needs could not be met by existing tools."

What drove me to break through this dilemma was a long-standing technical obsession. As early as 2009, I had attempted to develop an extremely simple small operating system on a 51 microcontroller. At that time, there was no mature reference framework, so I could only write task scheduling logic manually. The final system could only support a preemptive operating system for 2 tasks. Although the function was simple, it made me feel the joy of "controlling hardware resources through software logic" for the first time, and also planted the enthusiasm for exploring the principles of embedded scheduling. For me, "designing an operating system that fully meets my own needs" was never just a technical show-off, but the practice of the concept of "full-stack control from hardware bottom layer to software architecture." Since existing `OS` could not adapt to the resource constraints of the `Atmega328p`, I might as well start from scratch and build a system "tailor-made" for 8-bit resource-constrained MCUs.

Thus, the development goal of `Chaos-nano` gradually became clear: within the hardware limits of the `Atmega328p` (only 32KB Flash and 2KB RAM), solve the core contradiction between "precise timing" and "low power consumption, high resource utilization." At the same time, break the inherent perception that "8-bit microcontrollers are difficult to apply operating systems," and truly implement lightweight task scheduling capabilities in practical projects. The interweaving of this "solving practical problems" demand and the enthusiasm for "practicing technical obsessions" became the initial driving force for the birth of `Chaos-nano`.

## System Overview

The `Chaos-nano` operating system is a lightweight embedded operating system with the `libos` design philosophy as its core architecture, tailor-made for low-memory microcontroller (`MCU`) scenarios. Its design concept focuses on efficient resource utilization and development convenience. By streamlining core functions and optimizing execution logic, it achieves stable and reliable task management under limited hardware resources. As a typical cooperative asynchronous operating system, `Chaos-nano` does not rely on hardware interrupts for task preemption. Instead, it completes scheduling through tasks voluntarily yielding execution rights. It is very suitable for embedded projects with limited memory capacity (usually KB-level) and low real-time requirements, such as small sensor nodes, simple control modules, household electronic devices and other scenarios.

### System Design Tenets

The design of `Chaos-nano` always revolves around four core tenets, and all functional features serve this underlying logic:

1. **Extreme Lightweight**: Adhere to the principle of "just enough," eliminating all unnecessary functional modules. The system core only retains core logic such as task scheduling, timing control, and basic state management, avoiding redundant code occupying hardware resources, and ensuring adaptability to the extremely small memory environment of 8-bit MCUs.
2. **Low Resource Occupation**: Through designs such as static resource allocation, shared stack, and bit-level state management, the Flash occupation is compressed to several KB, and the RAM occupation is controlled at the level of hundreds of bytes. It does not rely on hardware MMU (Memory Management Unit), fully matching the hardware characteristics of resource-constrained MCUs.
3. **Controllable Real-Time Performance**: Under the cooperative scheduling framework, the response efficiency of high-priority tasks is guaranteed through a priority mechanism. Combined with the state machine task design pattern, key operations (such as sensor data processing and actuator control) can be completed within the expected time, meeting the basic real-time requirements of small embedded projects.
4. **High System Throughput**: Reduce CPU idle time through a blocking mechanism, allowing idle resources to be quickly allocated to ready tasks. At the same time, simplify task switching logic, reduce scheduling overhead (additional costs), and ensure that limited computing resources can be efficiently converted into actual business processing capabilities.

## System Features

### Static Resource Management Without Dynamic Memory Dependencies

`Chaos-nano` completely abandons dynamic memory operation functions such as `malloc`/`free` from the underlying design, and instead uses static arrays as the carrier for all resources. This design decision stems from the hardware characteristics of low-memory MCUs — dynamic memory allocation may lead to uncontrollable problems such as memory fragmentation, allocation failure, and high resource occupation. Static arrays determine the memory layout at compile time, which can fundamentally avoid such risks. For example, core components such as task control blocks and timer structures in the system are pre-allocated through static arrays, thereby reducing the risks and performance losses caused by dynamic memory operations.

During the development phase, developers need to plan resource capacity in advance according to project requirements: the array length must match indicators such as the maximum number of tasks and the maximum number of timed tasks in the project. Taking the timer module as an example, each task corresponds to a predefined static structure with parameters such as delay time and whether to block. When a task triggers a delay operation, the system directly reuses the structure resources in the array through the task's `ID` as an index, without the need for dynamic memory application. This not only ensures the predictability of memory usage (memory occupation can be determined at compile time) but also reduces the resource overhead during system operation (saving the computational cost of dynamic memory allocation).

### Priority-Based Scheduling Mechanism

`Chaos-nano` has a built-in priority-based task scheduler, which realizes efficient ready state management through a single-byte (8 bits) priority summary flag byte. Each bit of the flag byte corresponds to a priority (supporting levels 0-7 in total, with higher values indicating lower priority). When there are ready tasks at a certain priority level, the corresponding bit is set to 1. Each time the scheduler runs, it selects the highest-priority task from the set priorities for execution, ensuring that high-priority tasks obtain `CPU` time first and guaranteeing the response efficiency of key operations.

Designed for low-memory MCUs, the system does not allocate independent stacks for each task, so all tasks share the same stack — task scheduling only occurs when the current task voluntarily returns (such as completing the current phase of logic, triggering a delay, etc.). This feature requires developers to adopt a state machine mode in task design: split complex tasks into multiple independent small phases, and voluntarily yield the CPU (return to the scheduler) after each phase is completed. For example, a data collection task can be split into phases such as "initializing the sensor," "waiting for sampling completion," and "processing data." After each phase ends, it returns to the scheduler, so that higher-priority tasks can be executed in a timely manner, meeting basic real-time requirements in cooperative mode.

### Lightweight and Simplified Task Management Model

The task management module of `Chaos-nano` adheres to the principle of "streamlined and sufficient," and flexibly controls the state through a `block` flag bit and a temporary shutdown mechanism, balancing resource efficiency and scheduling fairness.

- **Basic Blocking Management**: Tasks are in a blocked state by default when created. When a task triggers a blocking operation (such as delay, waiting for an event), the `block` flag bit is set, and the corresponding priority bit of the task in the scheduling summary flag bit is cleared. When the blocking condition is lifted (such as delay expiration, event triggering), the `block` flag bit is cleared, and the task re-enters the ready state waiting for scheduling. This minimalist design greatly reduces the memory overhead of task management (each task only needs 1 bit to store the state), and simplifies the usage cost for developers — there is no need to pay attention to complex task state transition logic, and basic task scheduling requirements can be achieved only by controlling the blocking state through system APIs.
- **Temporary Shutdown and Fair Scheduling Mechanism**: The task manager additionally provides `stopOnce()`/`restoreAll()` functions, supporting setting tasks to a "temporarily closed" state. In this state, tasks will not participate in scheduling. However, unlike the blocked state, when the system enters the `idle` task (no ready tasks), all tasks closed by `stopOnce()` will be automatically restored to the ready state through the `restoreAll()` function. This mechanism cleverly simulates the core idea of CFS (Completely Fair Scheduler): by forcibly waking up low-priority tasks when the system is idle, avoiding them being "starved to death" due to high-priority tasks continuously occupying the CPU, and achieving basic fairness among tasks under the cooperative scheduling framework.

The flexible design of task blocking and temporary shutdown management can also simplify interrupt service routines and reduce the use of interrupt nesting:

- Only short operations of "starting device tasks" can be performed in device interrupts, and complex logic is handed over to task processing to shorten interrupt service time.
- Due to the simplified interrupt code, the interrupt priority of some devices can be converted into the priority of corresponding tasks, reducing the number of interrupt nesting layers and lowering the risk of stack overflow caused by excessive nesting.

In addition, the flexible design of task states enables the CPU to avoid idling for blocked or temporarily closed tasks and quickly switch to executing other ready tasks, significantly improving CPU utilization and supporting the system to achieve an asynchronous working mode.

### Efficient and Controllable Timing Function

The system provides flexible delay function interfaces, supporting tasks to perform blocking or non-blocking delay operations to optimize the resource utilization of the `MCU`.

- **Blocking Delay**: When a task calls a blocking delay function, the system records the delay time and sets the `block` flag bit. The task switches from the ready state to the blocked state and temporarily exits scheduling. During the delay period, the `CPU` can be occupied by other ready tasks, avoiding resource waste caused by idle loop waiting. When the delay time expires, the system automatically clears the `block` flag bit, and the task re-enters the ready queue waiting for scheduling.
- **Non-Blocking Delay**: Suitable for scenarios where periodic state checks are required. After a task calls a non-blocking delay function, it will not block immediately. Instead, it returns the remaining delay time, allowing the task to perform other lightweight operations (such as state query, simple calculation) during the waiting period until the delay ends.

The implementation of the timing function relies on the system reference clock (usually provided by the `MCU`'s timer interrupt, such as triggering once every 1ms). Each clock interrupt updates the remaining time count of all active delayed tasks, ensuring that the delay accuracy is synchronized with the system clock and meeting millisecond-level timing requirements.

Since the timer allocates a structure for each task, all tasks can perform delay operations simultaneously and be awakened in a timely manner after the delay ends.

### Simple Power Management

When the scheduler detects no ready tasks, the system will automatically enter the `idle` (idle) task. In the `idle` task, developers can configure the `MCU` to switch to a low-power mode (such as sleep mode, stop mode, etc.) through settings, thereby reducing the system's standby power consumption and extending the battery life of battery-powered devices. This design can adapt to low-power scenario requirements without complex power management logic.

### Extremely Low Resource Occupation Rate

Through the collaborative optimization of the above features, `Chaos-nano` achieves extremely low resource occupation: the core code usually only requires several KB of Flash storage space and hundreds of bytes of RAM running memory, perfectly adapting to 8-bit or 16-bit resource-constrained `MCUs` (such as AVR, STM8 and other series). Its stable and predictable task scheduling and management capabilities make it an efficient solution for small embedded projects, especially suitable for resource-constrained scenarios that need to balance functions and costs.

## Testing

Test Platform: ATMEGA328p

Test Code: `examples/arduino_pro_mini/chaos-nano`

Currently, testing is performed by creating "test" and "bleed" tasks in the system, and printing a message after a certain timer delay in each task.

```shell
15:40:54.436 -> [test:] loop(true)
15:40:55.693 -> [test:] loop(true)
15:40:55.693 -> [Bleed:] loop(true)
15:40:56.954 -> [test:] loop(true)
15:40:58.210 -> [test:] loop(true)
15:40:58.211 -> [Bleed:] loop(true)
15:40:59.470 -> [test:] loop(true)
15:41:00.727 -> [test:] loop(true)
15:41:00.727 -> [Bleed:] loop(true)
15:41:01.953 -> [test:] loop(true)
15:41:03.211 -> [test:] loop(true)
15:41:03.211 -> [Bleed:] loop(true)
15:41:04.464 -> [test:] loop(true)
15:41:05.719 -> [test:] loop(true)
15:41:05.719 -> [Bleed:] loop(true)
15:41:06.978 -> [test:] loop(true)
```