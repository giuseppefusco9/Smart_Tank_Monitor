#include "TMSState.h"
#include <Arduino.h>

const char* stateToString(TMSState state) {
  switch (state) {
    case INIT:         return "INIT";
    case CONNECTING:   return "CONNECTING";
    case CONNECTED:    return "CONNECTED";
    case MONITORING:   return "MONITORING";
    case DISCONNECTED: return "DISCONNECTED";
    default:           return "UNKNOWN";
  }
}


StateManager::StateManager() : currentState(INIT), lastTransitionTime(0) {}

TMSState StateManager::getState() const {
  return currentState;
}

void StateManager::setState(TMSState newState) {
  if (currentState != newState) {
    currentState = newState;
    lastTransitionTime = millis();
  }
}

unsigned long StateManager::getTimeInState() const {
  return millis() - lastTransitionTime;
}

bool StateManager::isOperational() const {
  return currentState == MONITORING;
}

bool StateManager::isConnected() const {
  return currentState == CONNECTED || currentState == MONITORING;
}

bool StateManager::isError() const {
  return currentState == DISCONNECTED;
}
