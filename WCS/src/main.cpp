#include <Arduino.h>
#include "config.h"
#include "kernel/Scheduler.h"
#include "kernel/SerialComm.h"
#include "devices/servoMotorImpl.h"
#include "devices/lcd.h"
#include "devices/buttonimpl.h"
#include "devices/pot.h"
#include "tasks/WCSTask.h"

// Global objects
Scheduler sched;
SerialComm serialComm;

// Hardware components
ServoMotorImpl* pServo;
Lcd* pLcd;
ButtonImpl* pButton;
Potentiometer* pPot;

// Task
WCSTask* wcsTask;

void setup() {
    serialComm.init(SERIAL_BAUD);
    
    pServo = new ServoMotorImpl(SERVO_PIN);
    pServo->on();
    
    pLcd = new Lcd();
    pButton = new ButtonImpl(BUTTON_PIN);
    pPot = new Potentiometer(POT_PIN);
    
    sched.init(50);
    
    wcsTask = new WCSTask(pServo, pLcd, pButton, pPot, &serialComm);
    wcsTask->init(100);
    sched.addTask(wcsTask);
}

void loop() {
    sched.schedule();
    
    delay(10);
}