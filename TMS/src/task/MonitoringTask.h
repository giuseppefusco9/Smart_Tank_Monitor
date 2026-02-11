#ifndef __MONITORING_TASK__
#define __MONITORING_TASK__

#include "kernel/Task.h"
#include "model/HWPlatform.h"
#include "model/WaterLevelData.h"
#include "model/TMSState.h"
#include "kernel/MQTTClient.h"
#include "config.h"

/**
 * Monitoring Task
 * Periodically reads water level from sonar and publishes to MQTT
 */
class MonitoringTask : public Task {
private:
  HWPlatform* hw;
  MQTTClient* mqttClient;
  StateManager* stateManager;
  WaterLevelData lastReading;

public:
  MonitoringTask(HWPlatform* hw, MQTTClient* mqttClient, StateManager* stateManager);
  
  void init(int period);

  void tick();

  /**
   * Get last water level reading
   */
  WaterLevelData getLastReading() const;
};

#endif
