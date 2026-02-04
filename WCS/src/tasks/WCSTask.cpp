#include "WCSTask.h"
#include "config.h"
#include "kernel/logger.h"

WCSTask::WCSTask(ServoMotorImpl* pServo, Lcd* pLcd, ButtonImpl* pButton, Potentiometer* pPot, SerialComm* pSerial)
    : state(AUTOMATIC), justEntered(true),
      pServo(pServo), pLcd(pLcd), pButton(pButton), pPot(pPot), pSerial(pSerial),
      lastValvePercentage(-1), lastPotUpdate(0), lastSerialCheck(0) {}

void WCSTask::init(int period) {
    Task::init(period);
    Logger.log("WCSTask initialized");
    
    // Initialize display
    updateLCDDisplay("STARTING", 0);
    
    // Initialize servo to closed position
    pServo->setPosition(0);
    
    // Start in AUTOMATIC mode
    setState(AUTOMATIC);
}

void WCSTask::tick() {
    unsigned long now = millis();
    
    // Check serial messages periodically
    if (now - lastSerialCheck >= SERIAL_CHECK_INTERVAL) {
        pSerial->update();
        processSerialMessages();
        lastSerialCheck = now;
    }
    
    // Check button press
    checkButtonPress();
    
    // Execute state-specific logic
    switch (state) {
        case AUTOMATIC:
            handleAutomaticMode();
            break;
            
        case MANUAL:
            handleManualMode();
            break;
            
        case UNCONNECTED:
            handleUnconnectedMode();
            break;
    }
}

void WCSTask::handleAutomaticMode() {
    if (checkAndSetJustEntered()) {
        Logger.log("State: AUTOMATIC");
        updateLCDDisplay("AUTOMATIC", lastValvePercentage);
        
        // Notify CUS of mode change
        pSerial->sendMessage("mode", "AUTOMATIC");
    }
    
    // In AUTOMATIC mode, we just wait for valve commands from CUS
    // The valve is controlled via serial messages
}

void WCSTask::handleManualMode() {
    if (checkAndSetJustEntered()) {
        Logger.log("State: MANUAL");
        updateLCDDisplay("MANUAL", lastValvePercentage);
        
        // Notify CUS of mode change
        pSerial->sendMessage("mode", "MANUAL");
    }
    
    // Process potentiometer input
    unsigned long now = millis();
    if (now - lastPotUpdate >= MANUAL_UPDATE_INTERVAL) {
        processPotentiometerInput();
        lastPotUpdate = now;
    }
}

void WCSTask::handleUnconnectedMode() {
    if (checkAndSetJustEntered()) {
        Logger.log("State: UNCONNECTED");
        updateLCDDisplay("UNCONNECTED", 0);
        
        // Close valve for safety
        pServo->setPosition(0);
        lastValvePercentage = 0;
    }
    
    // Wait for CUS to send a display update (reconnection signal)
    // State will change when CUS sends a message
}

void WCSTask::processSerialMessages() {
    String type, value;
    
    while (pSerial->messageAvailable()) {
        if (pSerial->receiveMessage(type, value)) {
            Logger.log("Received: " + type + " = " + value);
            
            if (type == "valve") {
                handleValveCommand(value);
            } else if (type == "display") {
                handleDisplayUpdate(value);
            }
        }
    }
}

void WCSTask::handleValveCommand(const String& value) {
    int percentage = value.toInt();
    
    // Validate percentage
    if (percentage < VALVE_MIN || percentage > VALVE_MAX) {
        Logger.log("Invalid valve percentage: " + String(percentage));
        return;
    }
    
    // Set servo angle
    int angle = mapPercentageToAngle(percentage);
    pServo->setPosition(angle);
    lastValvePercentage = percentage;
    
    // Update LCD
    if (state == AUTOMATIC) {
        updateLCDDisplay("AUTOMATIC", percentage);
    }
    
    Logger.log("Valve set to " + String(percentage) + "% (angle: " + String(angle) + ")");
}

void WCSTask::handleDisplayUpdate(const String& value) {
    // Parse display update message
    // Expected format from CUS: JSON with mode and valve fields
    // For simplicity, we'll parse it as a mode name
    
    if (value == "AUTOMATIC") {
        if (state == UNCONNECTED) {
            setState(AUTOMATIC);
        }
    } else if (value == "MANUAL") {
        if (state == UNCONNECTED) {
            setState(MANUAL);
        }
    } else if (value == "UNCONNECTED") {
        setState(UNCONNECTED);
    }
}

void WCSTask::checkButtonPress() {
    if (pButton->isPressed()) {
        Logger.log("Button pressed!");
        
        // Toggle between AUTOMATIC and MANUAL
        if (state == AUTOMATIC) {
            setState(MANUAL);
        } else if (state == MANUAL) {
            setState(AUTOMATIC);
        }
        // Ignore button press in UNCONNECTED state
        
        // Debounce delay
        delay(200);
    }
}

void WCSTask::processPotentiometerInput() {
    pPot->sync();
    int potValue = pPot->getValue();
    
    // Map potentiometer to percentage
    int percentage = mapPotToPercentage(potValue);
    
    // Only update if changed
    if (percentage != lastValvePercentage) {
        // Set servo
        int angle = mapPercentageToAngle(percentage);
        pServo->setPosition(angle);
        lastValvePercentage = percentage;
        
        // Update LCD
        updateLCDDisplay("MANUAL", percentage);
        
        // Send to CUS
        pSerial->sendMessage("valve", percentage);
        
        Logger.log("Manual valve: " + String(percentage) + "%");
    }
}

void WCSTask::setState(WCSState newState) {
    state = newState;
    justEntered = true;
}

bool WCSTask::checkAndSetJustEntered() {
    bool wasJustEntered = justEntered;
    justEntered = false;
    return wasJustEntered;
}

int WCSTask::mapPercentageToAngle(int percentage) {
    return map(percentage, VALVE_MIN, VALVE_MAX, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
}

int WCSTask::mapPotToPercentage(int potValue) {
    return map(potValue, POT_MIN, POT_MAX, VALVE_MIN, VALVE_MAX);
}

void WCSTask::updateLCDDisplay(const String& mode, int valve) {
    // Line 1: Mode
    pLcd->writeModeMessage(mode);
    
    // Line 2: Valve percentage
    String valveStr = "Valve: " + String(valve) + "%";
    pLcd->writePercMessage(valveStr);
}