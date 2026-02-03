#ifndef __MQTT_CLIENT__
#define __MQTT_CLIENT__

#include <WiFi.h>
#include <PubSubClient.h>
#include "config.h"

/**
 * MQTT Client Wrapper
 * Manages MQTT connection, publishing, and reconnection logic
 */
class MQTTClient {
private:
  WiFiClient wifiClient;
  PubSubClient mqttClient;
  unsigned long lastReconnectAttempt;
  unsigned long reconnectDelay;
  bool wifiConnected;

public:
  MQTTClient();
  
  bool connectWiFi();
  bool connectMQTT();
  bool reconnect();
  bool publish(const char* topic, const char* payload, bool retain = false);
  bool publish(const char* topic, const String& payload, bool retain = false);
  void loop();
  bool isWiFiConnected();
  bool isConnected();
  bool isFullyConnected();
  void disconnect();
};

#endif
