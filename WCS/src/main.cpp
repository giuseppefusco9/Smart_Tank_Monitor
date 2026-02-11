#include <Arduino.h>
#include "config.h"
#include "kernel/Scheduler.h"
#include "kernel/SerialComm.h"
#include "model/HWPlatform.h"
#include "tasks/WCSTask.h"

// Global objects
Scheduler* sched;
SerialComm* serialComm;
HWPlatform* hw;

// Task
WCSTask* wcsTask;

void setup() {
    serialComm = new SerialComm();
    hw = new HWPlatform();
    sched = new Scheduler();

    serialComm->init(SERIAL_BAUD);
    sched->init(50);
    
    wcsTask = new WCSTask(hw, serialComm);
    wcsTask->init(100);
    sched->addTask(wcsTask);
}

void loop() {
    sched->schedule();
}