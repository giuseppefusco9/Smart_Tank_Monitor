#include "Arduino.h"
#include "MonitoringTask.h"

MonitoringTask::MonitoringTask(Sonar* sonar, MQTTClient* mqttClient, StateManager* stateManager) 
  : sonar(sonar), mqttClient(mqttClient), stateManager(stateManager) {
  lastReading = WaterLevelData::invalid();
}

void MonitoringTask::init(int period) {
  Task::init(period);
  DEBUG_PRINTLN("MonitoringTask initialized");
}

void MonitoringTask::tick() {
  if (stateManager->getState() != MONITORING) {
    return;
  }

  float distance = sonar->getDistance();
  
  WaterLevelData data;
  data.distance = distance;
  data.calculateLevel(TANK_HEIGHT);
  data.timestamp = millis() / 1000;
  data.state = stateManager->getState();

  lastReading = data;

  if (data.isValid()) {
    DEBUG_PRINT("Water Level: ");
    DEBUG_PRINT(data.level);
    DEBUG_PRINT(" cm (Distance: ");
    DEBUG_PRINT(data.distance);
    DEBUG_PRINTLN(" cm)");
  } else {
    // Helpful debug to see if pulseIn is actually timing out (0) or something else
    DEBUG_PRINT("Sonar Read Failure. Distance: ");
    DEBUG_PRINTLN(distance);
  }

  if (mqttClient->isConnected()) {
    String jsonData = data.toJson();
    
    // DEBUG: Print MQTT publish to CUS
    Serial.println("\n===========================");
    Serial.println("DEBUG [TMS-MQTT]: Publishing to CUS");
    Serial.print("  Level: ");
    Serial.print(data.level);
    Serial.println(" cm");
    Serial.print("  JSON: ");
    Serial.println(jsonData);
    Serial.println("===========================\n");
    
    bool published = mqttClient->publish(MQTT_TOPIC, jsonData);
    
    if (!published) {
      DEBUG_PRINTLN("Failed to publish water level data");
    }
  } else {
    DEBUG_PRINTLN("Cannot publish: MQTT not connected");
  }
}

WaterLevelData MonitoringTask::getLastReading() const {
  return lastReading;
}
