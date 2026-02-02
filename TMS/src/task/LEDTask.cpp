#include "Arduino.h"
#include "LEDTask.h"

LEDTask::LEDTask(Led* greenLed, Led* redLed, StateManager* stateManager) 
  : greenLed(greenLed), redLed(redLed), stateManager(stateManager),
    blinkState(false), lastBlinkTime(0) {
}

void LEDTask::init(int period) {
  Task::init(period);
  DEBUG_PRINTLN("LEDTask initialized");
  greenLed->switchOff();
  redLed->switchOff();
}

void LEDTask::tick() {
  TMSState currentState = stateManager->getState();
  unsigned long now = millis();

  switch (currentState) {
    case INIT:
      if (now - lastBlinkTime >= LED_BLINK_PERIOD) {
        blinkState = !blinkState;
        if (blinkState) {
          greenLed->switchOn();
          redLed->switchOn();
        } else {
          greenLed->switchOff();
          redLed->switchOff();
        }
        lastBlinkTime = now;
      }
      break;

    case CONNECTING:
      greenLed->switchOff();
      redLed->switchOn();
      break;

    case CONNECTED:
      greenLed->switchOn();
      redLed->switchOff();
      break;

    case MONITORING:
      greenLed->switchOn();
      redLed->switchOff();
      break;

    case DISCONNECTED:
      greenLed->switchOff();
      redLed->switchOn();
      break;

    default:
      greenLed->switchOff();
      redLed->switchOff();
      break;
  }
}

void LEDTask::allOff() {
  greenLed->switchOff();
  redLed->switchOff();
}

void LEDTask::allOn() {
  greenLed->switchOn();
  redLed->switchOn();
}
