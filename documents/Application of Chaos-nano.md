# Application of Chaos-nano in Pressure Vessel Monitoring System

## I. Analysis of Overall System Architecture and Core Components

The Chaos-nano pressure vessel monitoring system takes the **ATMEGA328p microcontroller** as the control core, integrating three modules: hardware execution, environmental perception, and human-computer interaction to form a closed-loop pressure and environmental monitoring capability for pressure vessels. The functions and roles of each core component are as follows:

| Component Category         | Specific Equipment          | Core Function                                                | Technical Parameters & Connection Method                     |
| -------------------------- | --------------------------- | ------------------------------------------------------------ | ------------------------------------------------------------ |
| Execution Equipment        | Air Pump (Compressor)       | Supplement air to the pressure vessel to maintain stable pressure | Controlled by COMPRESSOR_PIN (Pin 4), supporting on/off switching via high/low level, with a single continuous operation not exceeding 30 seconds |
| Execution Equipment        | Pressure Relief Valve       | Release excess gas when pressure exceeds the limit to avoid safety risks | Controlled by BLEED_PIN (Pin 3), supporting on/off switching via high/low level, with a single continuous operation not exceeding 2 seconds |
| Sensors                    | Barometric Pressure Sensor  | Real-time collection of internal pressure data of the pressure vessel | Outputs analog/digital signals, transmits data via the microcontroller's AD conversion or dedicated interface, with adjustable sampling period via system settings |
| Sensors                    | Temperature-Humidity Sensor | Monitor the ambient temperature and humidity around the pressure vessel to avoid environmental impact risks | Adopts single-bus protocol for periodic sampling, with data accuracy of ±2℃ (temperature) and ±5% RH (humidity), triggering an alarm when exceeding the limit |
| Human-Computer Interaction | Serial Screen               | Display real-time data (pressure, temperature, humidity) + support parameter setting | Communicates with ATMEGA328p via serial port, allowing setting of pressure upper/lower limits, temperature-humidity alarm thresholds, and sensor sampling period |
| Alarm Equipment            | Buzzer                      | Issue an audible alarm when temperature or humidity exceeds the limit to remind staff to intervene | Low-power active buzzer, directly controlled by the microcontroller's IO port, with an alarm volume ≥60dB |

![系统框图](../pic/框图.png)

## II. Detailed System Workflow

With "stable pressure control" and "environmental risk early warning" as core goals, the system workflow is divided into four phases: **data collection - logical judgment - execution control - interactive feedback**, with specific steps as follows:

### 1. Data Collection Phase

- Barometric Pressure Sensor: Collects internal pressure data of the pressure vessel at a preset cycle (e.g., 1 time/second), transmits raw data to the microcontroller buffer, and marks the "pressure data ready" state.
- Temperature-Humidity Sensor: Synchronously collects ambient temperature and humidity periodically (e.g., 2 times/second), transmits data to the buffer, and marks the "temperature-humidity data ready" state.
- Data Trigger Mechanism: Immediately wake up the "sensor data processing task (TASK_ID_SENSOR_HANDLE)" after any sensor completes collection and marks readiness to avoid data accumulation.

### 2. Logical Judgment Phase

- Pressure Data Processing: The sensor data processing task reads pressure data and compares it with the "pressure upper limit" and "pressure lower limit" preset on the serial screen.
  - If pressure < lower limit: Determine as "insufficient pressure" and trigger the air pump start logic.
  - If pressure > upper limit: Determine as "excessive pressure" and trigger the pressure relief valve start logic.
  - If pressure is between upper and lower limits: Determine as "normal pressure" and do not perform any execution equipment operations.
- Temperature-Humidity Data Processing: Reads temperature and humidity data and compares it with the preset "temperature alarm threshold" and "humidity alarm threshold".
  - If temperature > upper limit or humidity > upper limit: Determine as "environmental excess", trigger the buzzer alarm, and continue until data returns to normal or manual intervention.
  - If both temperature and humidity are within normal ranges: Determine as "normal environment" and turn off the buzzer (if alarmed previously).

### 3. Execution Control Phase

- Air Pump Control: After startup, the system synchronously starts the "compressor control task (TASK_ID_DEV_COMPRESSOR)", which controls the air pump's working duration via a timer (DELAY_MS=30 seconds).
  - The timer triggers a state reversal every 30 seconds: If the air pump is currently on, turn it off; if off, turn it on (to avoid overheating from continuous operation).
  - When the barometric pressure sensor detects that the pressure has recovered to the normal range, the system calls Compressor::on(false) to turn off the compressor, cancel the compressor timer, and block the compressor control task.
- Pressure Relief Valve Control: After startup, the "pressure relief valve control task (TASK_ID_DEV_BLEED)" controls the valve's working duration via a timer (DELAY_MS=2 seconds).
  - The timer triggers a state reversal every 2 seconds: If the relief valve is currently on, turn it off; if off, turn it on (to avoid overheating from continuous operation).
  - When the barometric pressure sensor detects that the pressure has recovered to the normal range, the system calls Temp_Humd::on(false) to turn off the relief valve, cancel the relief valve timer, and block the relief valve control task.

### 4. Interactive Feedback Phase

- Data Display: The serial screen updates and displays pressure (unit: kPa/PSI, switchable via settings), temperature (unit: ℃/℉), humidity (unit: % RH) in real time, as well as the current state of each execution device (air pump: on/off; relief valve: on/off; buzzer: alarm/normal).
- Parameter Setting: Users input new "pressure upper/lower limits", "temperature-humidity thresholds", and "sampling periods" via the serial screen buttons. Data is transmitted to the microcontroller via the serial port to update system control parameters without the need for re-burning the program.

## III. Software Architecture and Task Scheduling Mechanism

Based on the Arduino development environment and equipped with the Chaos-nano operating system, the system adopts a "task-oriented" design, decomposing functions into 6 independent tasks that achieve efficient collaboration through priority scheduling.

### 3.1 Task Classification and Priority Settings

Chaos-nano classifies tasks into "device tasks" and "user tasks", with device tasks having higher priority than user tasks to ensure fast response for execution equipment (air pump, pressure relief valve) control.

| Task ID                   | Task Name                           | Task Type   | Core Function                                                | Priority | Trigger Method                                               |
| ------------------------- | ----------------------------------- | ----------- | ------------------------------------------------------------ | -------- | ------------------------------------------------------------ |
| TASK_ID_DEV_BLEED         | Pressure Relief Valve Control Task  | Device Task | Control the on/off of the relief valve via timer to prevent overheating | 0        | Triggered when the relief valve is turned on (works for 2 seconds, rests for 2 seconds) |
| TASK_ID_DEV_COMPRESSOR    | Compressor Control Task             | Device Task | Control the on/off of the compressor via timer to prevent overheating | 1        | Triggered when the compressor is turned on (works for 30 seconds, rests for 30 seconds) |
| TASK_ID_SENSOR_TEMP_HUMD  | Temperature-Humidity Detection Task | Device Task | Periodically collect temperature and humidity data and store it in the buffer | 2        | Periodically triggered by the system timer (e.g., 2 times/second) |
| TASK_ID_SENSOR_BAROMETRIC | Barometric Pressure Detection Task  | Device Task | Periodically collect pressure data and store it in the buffer | 3        | Periodically triggered by the system timer (e.g., 0.5 times/second) |
| TASK_ID_SENSOR_HANDLE     | Sensor Data Processing Task         | User Task   | Read buffer data and determine whether to trigger execution logic | 4        | Awakened when sensor data is ready                           |
| TASK_ID_USER_DISP         | Screen Interaction Task             | User Task   | Process serial screen data display and parameter setting commands | 5        | Triggered by serial interrupt when there is data input from the serial screen |

### 3.2 Analysis of Core Task Code Logic

#### (1) Main Task Scheduling Logic (loop Function)

The main function implements task scheduling through a loop of "obtain the next high-priority task → execute the task → reset state", with the core logic as follows:

```cpp
void loop() {
  // 1. Get the task ID of the current highest priority
  int8_t pri = task.getNextPriority();
  // 2. Execute the corresponding logic according to the task ID
  switch (pri) {
    case TASK_ID_DEV_BLEED:
      // Execute relief valve control logic (prevent relief valve damage from overheating)
      bleed.loop();
      break;
    case TASK_ID_DEV_COMPRESSOR:
      // Execute air pump control logic (prevent compressor damage from overheating)
      compressor.loop();
      break;
    case TASK_ID_SENSOR_TEMP_HUMD:
      // Collect temperature and humidity data; wake up the data processing task if collection is successful
      sensor_th = th.loop();
      if (sensor_th) {
        task.runOnce(TASK_ID_SENSOR_HANDLE);
      }
      break;
    case TASK_ID_SENSOR_BAROMETRIC:
      // Collect pressure data; wake up the data processing task if collection is successful
      sensor_pressure = barometric.loop();
      if (sensor_pressure) {
        task.runOnce(TASK_ID_SENSOR_HANDLE);
      }
      break;
    case TASK_ID_SENSOR_HANDLE:
      // The data processing task processes all data at once, so it only needs to be executed once after new data arrives
      task.stopOnce(TASK_ID_SENSOR_HANDLE);
      // Process pressure data: determine whether to start/stop the air pump/relief valve
      if (sensor_pressure) {
        float pressure = barometric.get();
        sensor_pressure = false;
        Serial.print("\n[main:] pressure = ");
        Serial.println(pressure);
        sensor_handle_pressure(pressure); // Pressure control logic function
      }
      // Process temperature and humidity data: determine whether to trigger the buzzer alarm
      if (sensor_th) {
        sensor_th = false;
        float temp = th.get_temp();
        float humd = th.get_humd();
        Serial.print("\n[main:] temp = ");
        Serial.println(temp);
        Serial.print("\n[main:] humd = ");
        Serial.println(humd);
        sensor_handle_temp_humd(temp, humd); // Temperature-humidity alarm logic function
      }
      break;
    case TASK_ID_USER_DISP:
      // Process serial screen interaction: read user-input parameters and update system settings
      Serial.println("[main:] TASK_ID_USER_DISP");
      char inChar;
      while (Serial.available()) {
        inChar = (char)Serial.read();
        Serial.print("revc: ");
        Serial.println(inChar);
        disp_handle(&inChar); // Serial screen data processing function (e.g., parse parameters, update display)
        task.stopOnce(TASK_ID_USER_DISP); // Stop the task after processing is completed
        break;
      }
      break;
    default:
      // Restore the default state of non-blocked tasks after all tasks are executed once;
      // Simulate CFS scheduling to prevent low-priority tasks from being starved
      task.restoreAll();
      // If there are no executable tasks, the system enters low-power mode
      if (IDLE_PRI != task.getNextPriority()) {
        sleep_cpu();
      }
      break;
  }
}
```

#### (2) Air Pump Control Code Logic (compressor.h/.cpp)

The air pump control class (Compressor) encapsulates pin initialization, on/off control, and state monitoring functions. The core logic relies on a timer to implement "30-second state reversal" to avoid equipment damage from continuous operation.

- compressor.h: Defines the class interface, including initialization (setup), on/off control (on), state acquisition (state), and loop execution (loop) functions, ensuring other modules can call the air pump control functions.
- compressor.cpp: Implements the class functions with key logic as follows:
  1. Initialization (setup): Sets Pin 4 as output mode, initial state as off (OFF), and resets flags (flag, mState).
  2. On/off Control (on):
     - Turn on (b=true): Sets the air pump pin to high level (ON), starts the compressor control task, creates a 30-second timer, and marks the current state as on (mState=true).
     - Turn off (b=false): Cancels the timer, sets the pin to low level (OFF), blocks the compressor control task, and marks the state as off (mState=false).
  3. Loop Execution (loop): The timer sets the flag to true every 30 seconds. After triggering, it reverses the air pump state (on→off / off→on) and recreates the timer to achieve periodic state switching.
  4. Timer End Flag (flag): When the timer expires, the delay function sets this flag to true, which the task uses to determine if it was awakened by the timer.

## IV. Key System Features and Advantages

1. **High Reliability**: High-priority device tasks (air pump, relief valve control) ensure fast response of execution equipment, avoiding risks caused by equipment damage from overheating. The temperature-humidity alarm function can pre-empt environmental impacts on pressure vessels. The high detection frequency of the pressure sensor and comprehensive consideration of the air pump's flow rate allow appropriate reduction of the pressure detection priority without affecting the overall system safety.
2. **Low-Power Design**: When there are no executable tasks, the CPU enters sleep mode (sleep_cpu()), reducing overall system power consumption and making it suitable for long-term unattended scenarios.
3. **Flexible Configurability**: Users can modify pressure upper/lower limits, temperature-humidity thresholds, and sampling periods in real time via the serial screen without connecting to a computer for program re-burning, adapting to monitoring needs of pressure vessels of different specifications.
4. **Fault Self-Protection**: The air pump and relief valve periodically reverse their states via timers to avoid overheating damage from continuous operation. If sensor data collection fails, the system will not falsely trigger execution logic, improving operational stability.

## V. Application Video

<div align="center">
  <img src="../pic/product_show.gif">

</div>
