#include "Arduino.h"
#include "LEDTask.h"

LEDTask::LEDTask(HWPlatform* hw, StateManager* stateManager) 
  : hw(hw), stateManager(stateManager),
    blinkState(false), lastBlinkTime(0) {
}

void LEDTask::init(int period) {
  Task::init(period);
  DEBUG_PRINTLN("LEDTask initialized");
  hw->getGreenLed()->switchOff();
  hw->getRedLed()->switchOff();
}

void LEDTask::tick() {
  TMSState currentState = stateManager->getState();
  unsigned long now = millis();

  // Debug: Print state periodically
  static unsigned long lastDebug = 0;
  if (now - lastDebug >= 5000) {
    DEBUG_PRINT("[LED] State: ");
    DEBUG_PRINTLN(stateToString(currentState));
    lastDebug = now;
  }

  switch (currentState) {
    case INIT:
      if (now - lastBlinkTime >= LED_BLINK_PERIOD) {
        blinkState = !blinkState;
        if (blinkState) {
          hw->getGreenLed()->switchOn();
          hw->getRedLed()->switchOn();
        } else {
          hw->getGreenLed()->switchOff();
          hw->getRedLed()->switchOff();
        }
        lastBlinkTime = now;
      }
      break;

    case CONNECTING:
      hw->getGreenLed()->switchOff();
      hw->getRedLed()->switchOn();
      break;

    case CONNECTED:
      hw->getGreenLed()->switchOn();
      hw->getRedLed()->switchOff();
      break;

    case MONITORING:
      hw->getGreenLed()->switchOn();
      hw->getRedLed()->switchOff();
      break;

    case DISCONNECTED:
      hw->getGreenLed()->switchOff();
      hw->getRedLed()->switchOn();
      break;

    default:
      hw->getGreenLed()->switchOff();
      hw->getRedLed()->switchOff();
      break;
  }
}

void LEDTask::allOff() {
  hw->getGreenLed()->switchOff();
  hw->getRedLed()->switchOff();
}

void LEDTask::allOn() {
  hw->getGreenLed()->switchOn();
  hw->getRedLed()->switchOn();
}
