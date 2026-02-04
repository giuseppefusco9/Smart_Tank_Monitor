#include <Arduino.h>
#include "config.h"
#include "kernel/Scheduler.h"
#include "kernel/logger.h"
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
    // Initialize serial communication
    serialComm.init(SERIAL_BAUD);
    
    // Initialize logger
    Logger.log("========================================");
    Logger.log("    Water Channel Subsystem (WCS)");
    Logger.log("    Smart Tank Monitoring System");
    Logger.log("========================================");
    
    // Initialize hardware
    Logger.log("Initializing hardware...");
    pServo = new ServoMotorImpl(SERVO_PIN);
    pServo->on();  // Enable servo
    
    pLcd = new Lcd();
    pButton = new ButtonImpl(BUTTON_PIN);
    pPot = new Potentiometer(POT_PIN);
    
    Logger.log("Hardware initialized");
    
    // Initialize scheduler
    sched.init(50);  // 50ms base period
    Logger.log("Scheduler initialized");
    
    // Create and register WCS task
    wcsTask = new WCSTask(pServo, pLcd, pButton, pPot, &serialComm);
    wcsTask->init(100);  // 100ms task period
    sched.addTask(wcsTask);
    
    Logger.log("WCS Task registered");
    Logger.log("========================================");
    Logger.log("System ready!");
    Logger.log("========================================");
}

void loop() {
    // Run scheduler
    sched.schedule();
    
    // Small delay
    delay(10);
}