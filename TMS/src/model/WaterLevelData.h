#ifndef __WATER_LEVEL_DATA__
#define __WATER_LEVEL_DATA__

#include <ArduinoJson.h>
#include "config.h"
#include "TMSState.h"

/**
 * Water Level Measurement Data Structure
 */
struct WaterLevelData {
  float distance;
  float level;
  unsigned long timestamp;
  TMSState state;

  /**
   * Calculate water level from distance measurement
   * Level = Tank Height - Distance
   */
  void calculateLevel(float tankHeight);

  /**
   * Convert to JSON string for MQTT publishing
   * Returns: JSON string or empty string on error
   */
  String toJson();

  /**
   * Check if measurement is valid
   */
  bool isValid() const;

  /**
   * Create a default/invalid measurement
   */
  static WaterLevelData invalid();
};

#endif
