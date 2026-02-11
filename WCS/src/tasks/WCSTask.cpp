#include "WCSTask.h"
#include "config.h"

WCSTask::WCSTask(HWPlatform* pHW, SerialComm* pSerial)
    : state(AUTOMATIC), justEntered(true),
      pHW(pHW), pSerial(pSerial),
      lastValvePercentage(0), lastPotUpdate(0), lastSerialCheck(0) {}

void WCSTask::init(int period) {
    Task::init(period); 
    
    updateLCDDisplay("STARTING", 0);
    
    setState(AUTOMATIC);
    
    pHW->getPot()->sync();
    lastPhysicalPotPercentage = mapPotToPercentage(pHW->getPot()->getValue());
}

void WCSTask::tick() {
    unsigned long now = millis();
    
    if (now - lastSerialCheck >= SERIAL_CHECK_INTERVAL) {
        pSerial->update();
        processSerialMessages();
        lastSerialCheck = now;
    }
    
    checkButtonPress();
    
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
        updateLCDDisplay("AUTOMATIC", lastValvePercentage);

        pSerial->sendMessage("mode", 0);
    }
}

void WCSTask::handleManualMode() {
    if (checkAndSetJustEntered()) {
        updateLCDDisplay("MANUAL", lastValvePercentage);
        
        pSerial->sendMessage("mode", 1);
    }
    
    unsigned long now = millis();
    if (now - lastPotUpdate >= MANUAL_UPDATE_INTERVAL) {
        processPotentiometerInput();
        lastPotUpdate = now;
    }
}

void WCSTask::handleUnconnectedMode() {
    if (checkAndSetJustEntered()) {
        updateLCDDisplay("UNCONNECTED", 0);
        
        pHW->getMotor()->setPosition(0);
        lastValvePercentage = 0;
    }
}

void WCSTask::processSerialMessages() {
    String type, value;
    
    while (pSerial->messageAvailable()) {
        if (pSerial->receiveMessage(type, value)) {
            
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
    
    if (percentage < VALVE_MIN || percentage > VALVE_MAX) {
        return;
    }
    
    int angle = mapPercentageToAngle(percentage);
    pHW->getMotor()->setPosition(angle);
    lastValvePercentage = percentage;
    
    String modeStr = "UNKNOWN";
    if (state == AUTOMATIC) modeStr = "AUTOMATIC";
    else if (state == MANUAL) modeStr = "MANUAL";
    else if (state == UNCONNECTED) modeStr = "UNCONNECTED";
    
    updateLCDDisplay(modeStr, percentage);
    
    pSerial->sendMessage("status", "Valve set to " + String(percentage) + "%");
}

void WCSTask::handleDisplayUpdate(const String& value) {
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
    
    int angle = mapPercentageToAngle(valveVal);
    pHW->getMotor()->setPosition(angle);
    
    updateLCDDisplay(modeStr, valveVal);
    
    pSerial->sendMessage("status", "Display synced: " + modeStr);
}

void WCSTask::checkButtonPress() {
    if (pHW->getButton()->isPressed()) {
        if (state == AUTOMATIC) {
            setState(MANUAL);
        } else if (state == MANUAL) {
            setState(AUTOMATIC);
        }
        
        delay(200);
    }
}

void WCSTask::processPotentiometerInput() {
    pHW->getPot()->sync();
    int potValue = pHW->getPot()->getValue();
    
    int percentage = mapPotToPercentage(potValue);
    
    int delta = abs(percentage - lastPhysicalPotPercentage);
    
    if (delta >= 2) { 
        lastPhysicalPotPercentage = percentage;
        
        if (percentage != lastValvePercentage) {
            int angle = mapPercentageToAngle(percentage);
            pHW->getMotor()->setPosition(angle);
            lastValvePercentage = percentage;
            
            updateLCDDisplay("MANUAL", percentage);
            
            pSerial->sendMessage("valve", percentage);
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
    pHW->getLCD()->writeModeMessage(mode);
    
    String valveStr = "Valve: " + String(valve) + "%";
    pHW->getLCD()->writePercMessage(valveStr);
}