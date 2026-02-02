#include "Arduino.h"
#include "MQTTTask.h"

MQTTTask::MQTTTask(MQTTClient* mqttClient, StateManager* stateManager) 
  : mqttClient(mqttClient), stateManager(stateManager), 
    lastConnectionCheck(0), wasConnected(false) {
}

void MQTTTask::init(int period) {
  Task::init(period);
  DEBUG_PRINTLN("MQTTTask initialized");
}

void MQTTTask::tick() {
  TMSState currentState = stateManager->getState();
  
  mqttClient->loop();

  bool isConnected = mqttClient->isFullyConnected();

  if (currentState == INIT) {
    return;
  }

  if (currentState == CONNECTING) {
    if (mqttClient->reconnect()) {
      DEBUG_PRINTLN("Connection established!");
      stateManager->setState(CONNECTED);
    }
  } 
  else if (currentState == CONNECTED) {
    if (isConnected) {
      DEBUG_PRINTLN("Transitioning to MONITORING state");
      stateManager->setState(MONITORING);
    } else {
      DEBUG_PRINTLN("Lost connection, returning to CONNECTING");
      stateManager->setState(CONNECTING);
    }
  }
  else if (currentState == MONITORING) {
    if (!isConnected) {
      DEBUG_PRINTLN("Connection lost! Transitioning to DISCONNECTED");
      stateManager->setState(DISCONNECTED);
    }
  }
  else if (currentState == DISCONNECTED) {
    if (mqttClient->reconnect()) {
      DEBUG_PRINTLN("Reconnected! Transitioning to MONITORING");
      stateManager->setState(MONITORING);
    }
  }

  wasConnected = isConnected;
  lastConnectionCheck = millis();
}

bool MQTTTask::isConnected() {
  return mqttClient->isFullyConnected();
}

unsigned long MQTTTask::getTimeSinceCheck() const {
  return millis() - lastConnectionCheck;
}
