#include "WaterLevelData.h"
#include <ArduinoJson.h>
#include "config.h"
#include "TMSState.h"

void WaterLevelData::calculateLevel(float tankHeight) {
  if (distance >= 0) {
    level = tankHeight - distance;
    if (level < 0) level = 0;
    if (level > tankHeight) level = tankHeight;
  } else {
    level = -1;
  }
}

String WaterLevelData::toJson() {
  JsonDocument doc;
  
  doc["distance"] = distance;
  doc["level"] = level;
  doc["timestamp"] = timestamp;
  doc["state"] = stateToString(state);

  String output;
  serializeJson(doc, output);
  return output;
}

bool WaterLevelData::isValid() const {
  return distance >= 0 && level >= 0;
}

WaterLevelData WaterLevelData::invalid() {
  WaterLevelData data;
  data.distance = -1;
  data.level = -1;
  data.timestamp = 0;
  data.state = DISCONNECTED;
  return data;
}
