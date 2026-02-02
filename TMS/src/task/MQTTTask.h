#ifndef __MQTT_TASK__
#define __MQTT_TASK__

#include "kernel/Task.h"
#include "kernel/MQTTClient.h"
#include "model/TMSState.h"
#include "config.h"

/**
 * MQTT Task
 * Manages MQTT connection, reconnection, and connection health monitoring
 */
class MQTTTask : public Task {
private:
  MQTTClient* mqttClient;
  StateManager* stateManager;
  unsigned long lastConnectionCheck;
  bool wasConnected;

public:
  MQTTTask(MQTTClient* mqttClient, StateManager* stateManager);
  
  void init(int period);
  void tick();

  /**
   * Check if currently connected
   */
  bool isConnected();

  /**
   * Get time since last connection check
   */
  unsigned long getTimeSinceCheck() const;
};

#endif
