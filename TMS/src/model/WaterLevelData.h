#ifndef __WATER_LEVEL_DATA__
#define __WATER_LEVEL_DATA__

#include <ArduinoJson.h>
#include "config.h"
#include "TMSState.h"

/**
 * Water Level Measurement Data Structure
 */
struct WaterLevelData {
  float distance;        // Distance measured by sonar (cm)
  float level;          // Calculated water level (cm)
  unsigned long timestamp; // Unix timestamp
  TMSState state;       // Current system state

  /**
   * Calculate water level from distance measurement
   * Level = Tank Height - Distance
   */
  void calculateLevel(float tankHeight) {
    if (distance >= 0) {
      level = tankHeight - distance;
      if (level < 0) level = 0;
      if (level > tankHeight) level = tankHeight;
    } else {
      level = -1; // Invalid reading
    }
  }

  /**
   * Convert to JSON string for MQTT publishing
   * Returns: JSON string or empty string on error
   */
  String toJson() {
    JsonDocument doc;
    
    doc["distance"] = distance;
    doc["level"] = level;
    doc["timestamp"] = timestamp;
    doc["state"] = stateToString(state);

    String output;
    serializeJson(doc, output);
    return output;
  }

  /**
   * Check if measurement is valid
   */
  bool isValid() const {
    return distance >= 0 && level >= 0;
  }

  /**
   * Create a default/invalid measurement
   */
  static WaterLevelData invalid() {
    WaterLevelData data;
    data.distance = -1;
    data.level = -1;
    data.timestamp = 0;
    data.state = DISCONNECTED;
    return data;
  }
};

#endif
