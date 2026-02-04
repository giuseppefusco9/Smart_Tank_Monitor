#include "WCSTask.h"
#include "config.h"
#include "kernel/logger.h"

WCSTask::WCSTask(ServoMotorImpl* pServo, Lcd* pLcd, ButtonImpl* pButton, Potentiometer* pPot, SerialComm* pSerial)
    : state(AUTOMATIC), justEntered(true),
      pServo(pServo), pLcd(pLcd), pButton(pButton), pPot(pPot), pSerial(pSerial),
      lastValvePercentage(0), lastPotUpdate(0), lastSerialCheck(0) {}

void WCSTask::init(int period) {
    Task::init(period);
    Logger.log("WCSTask initialized");
    
    // Initialize display
    updateLCDDisplay("STARTING", 0);
    
    // Start in AUTOMATIC mode
    setState(AUTOMATIC);
    
    // Initialize potentiometer tracking
    pPot->sync();
    lastPhysicalPotPercentage = mapPotToPercentage(pPot->getValue());
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
        pSerial->sendMessage("mode", 0);
    }
    
    // In AUTOMATIC mode, we just wait for valve commands from CUS
    // The valve is controlled via serial messages
}

void WCSTask::handleManualMode() {
    if (checkAndSetJustEntered()) {
        Logger.log("State: MANUAL");
        updateLCDDisplay("MANUAL", lastValvePercentage);
        
        // Notify CUS of mode change
        pSerial->sendMessage("mode", 1);
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
    
    // Always update LCD to show new percentage
    String modeStr = "UNKNOWN";
    if (state == AUTOMATIC) modeStr = "AUTOMATIC";
    else if (state == MANUAL) modeStr = "MANUAL";
    else if (state == UNCONNECTED) modeStr = "UNCONNECTED";
    
    updateLCDDisplay(modeStr, percentage);
    
    // Send confirmation back to CUS
    pSerial->sendMessage("status", "Valve set to " + String(percentage) + "%");
    
    Logger.log("Valve set to " + String(percentage) + "% (angle: " + String(angle) + ")");
}

void WCSTask::handleDisplayUpdate(const String& value) {
    // Parse composite string: "MODE|VALVE"
    String modeStr = value;
    int valveVal = lastValvePercentage;
    
    int pipeIdx = value.indexOf('|');
    if (pipeIdx != -1) {
        modeStr = value.substring(0, pipeIdx);
        valveVal = value.substring(pipeIdx + 1).toInt();
        lastValvePercentage = valveVal;
    }
    
    if (modeStr == "AUTOMATIC") {
        setState(AUTOMATIC);
    } else if (modeStr == "MANUAL") {
        setState(MANUAL);
    } else if (modeStr == "UNCONNECTED") {
        setState(UNCONNECTED);
    }
    
    // Update servo position to match the synchronized state
    int angle = mapPercentageToAngle(valveVal);
    pServo->setPosition(angle);
    
    // Force LCD update with latest data
    updateLCDDisplay(modeStr, valveVal);
    
    // Send confirmation back to CUS
    pSerial->sendMessage("status", "Display synced: " + modeStr);
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
    
    // Check if the physical dial was actually moved (use threshold to ignore noise/jitter)
    int delta = abs(percentage - lastPhysicalPotPercentage);
    
    if (delta >= 2) { 
        // Movement detected, update tracking
        lastPhysicalPotPercentage = percentage;
        
        // If the dial moved, it takes priority and updates the system VALVE
        if (percentage != lastValvePercentage) {
            // Set servo
            int angle = mapPercentageToAngle(percentage);
            pServo->setPosition(angle);
            lastValvePercentage = percentage;
            
            // Update LCD
            updateLCDDisplay("MANUAL", percentage);
            
            // Send to CUS
            pSerial->sendMessage("valve", percentage);
            
            Logger.log("Manual dial moved: " + String(percentage) + "%");
        }
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