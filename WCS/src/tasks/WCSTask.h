#ifndef __WCS_TASK__
#define __WCS_TASK__

#include "kernel/Task.h"
#include "kernel/SerialComm.h"
#include "devices/servoMotorImpl.h"
#include "devices/lcd.h"
#include "devices/buttonimpl.h"
#include "devices/pot.h"
#include <Arduino.h>

/**
 * Water Channel Subsystem Task
 * Manages valve control, LCD display, and communication with CUS
 */
class WCSTask : public Task {
public:
    WCSTask(ServoMotorImpl* pServo, Lcd* pLcd, ButtonImpl* pButton, Potentiometer* pPot, SerialComm* pSerial);
    
    void init(int period) override;
    void tick() override;

private:
    // FSM States
    enum WCSState {
        AUTOMATIC,      // CUS controls valve automatically
        MANUAL,         // User controls valve via potentiometer
        UNCONNECTED     // CUS disconnected
    };
    
    WCSState state;
    bool justEntered;
    
    // Hardware components
    ServoMotorImpl* pServo;
    Lcd* pLcd;
    ButtonImpl* pButton;
    Potentiometer* pPot;
    SerialComm* pSerial;
    
    // State tracking
    int lastValvePercentage;
    int lastPhysicalPotPercentage;
    unsigned long lastPotUpdate;
    unsigned long lastSerialCheck;
    
    // State machine methods
    void setState(WCSState newState);
    bool checkAndSetJustEntered();
    
    // Message handling
    void processSerialMessages();
    void handleValveCommand(const String& value);
    void handleDisplayUpdate(const String& value);
    
    // Mode-specific logic
    void handleAutomaticMode();
    void handleManualMode();
    void handleUnconnectedMode();
    
    // Button handling
    void checkButtonPress();
    
    // Potentiometer handling
    void processPotentiometerInput();
    
    // Utilities
    int mapPercentageToAngle(int percentage);
    int mapPotToPercentage(int potValue);
    void updateLCDDisplay(const String& mode, int valve);
};

#endif