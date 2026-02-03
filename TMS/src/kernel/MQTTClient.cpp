#include "MQTTClient.h"

MQTTClient::MQTTClient() 
  : mqttClient(wifiClient), 
    lastReconnectAttempt(0), 
    reconnectDelay(MQTT_RECONNECT_DELAY),
    wifiConnected(false) {
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
}

bool MQTTClient::connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    return true;
  }

  DEBUG_PRINT("Connecting to WiFi: ");
  DEBUG_PRINTLN(WIFI_SSID);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < WIFI_TIMEOUT) {
    delay(500);
    DEBUG_PRINT(".");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    DEBUG_PRINTLN("\nWiFi connected!");
    DEBUG_PRINT("IP Address: ");
    DEBUG_PRINTLN(WiFi.localIP());
    wifiConnected = true;
    return true;
  } else {
    DEBUG_PRINTLN("\nWiFi connection failed!");
    wifiConnected = false;
    return false;
  }
}

bool MQTTClient::connectMQTT() {
  if (mqttClient.connected()) {
    return true;
  }

  if (!wifiConnected) {
    return false;
  }

  DEBUG_PRINT("Connecting to MQTT broker: ");
  DEBUG_PRINTLN(MQTT_BROKER);

  bool connected;
  if (strlen(MQTT_USERNAME) > 0) {
    connected = mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD);
  } else {
    connected = mqttClient.connect(MQTT_CLIENT_ID);
  }

  if (connected) {
    DEBUG_PRINTLN("MQTT connected!");
    reconnectDelay = MQTT_RECONNECT_DELAY;
    return true;
  } else {
    DEBUG_PRINT("MQTT connection failed, rc=");
    DEBUG_PRINTLN(mqttClient.state());
    return false;
  }
}

bool MQTTClient::reconnect() {
  unsigned long now = millis();
  
  if (now - lastReconnectAttempt < reconnectDelay) {
    return false;
  }

  lastReconnectAttempt = now;

  if (!connectWiFi()) {
    reconnectDelay = (reconnectDelay * 2 < MQTT_MAX_RECONNECT_DELAY) ? reconnectDelay * 2 : MQTT_MAX_RECONNECT_DELAY;
    return false;
  }

  if (!connectMQTT()) {
    reconnectDelay = (reconnectDelay * 2 < MQTT_MAX_RECONNECT_DELAY) ? reconnectDelay * 2 : MQTT_MAX_RECONNECT_DELAY;
    return false;
  }

  return true;
}

bool MQTTClient::publish(const char* topic, const char* payload, bool retain) {
  if (!mqttClient.connected()) {
    DEBUG_PRINTLN("Cannot publish: MQTT not connected");
    return false;
  }

  bool result = mqttClient.publish(topic, payload, retain);
  
  if (result) {
    DEBUG_PRINT("Published to ");
    DEBUG_PRINT(topic);
    DEBUG_PRINT(": ");
    DEBUG_PRINTLN(payload);
  } else {
    DEBUG_PRINTLN("Publish failed!");
  }
  
  return result;
}

bool MQTTClient::publish(const char* topic, const String& payload, bool retain) {
  return publish(topic, payload.c_str(), retain);
}

void MQTTClient::loop() {
  if (mqttClient.connected()) {
    mqttClient.loop();
  }
}

bool MQTTClient::isWiFiConnected() {
  return wifiConnected && (WiFi.status() == WL_CONNECTED);
}

bool MQTTClient::isConnected() {
  return mqttClient.connected();
}

bool MQTTClient::isFullyConnected() {
  return isWiFiConnected() && isConnected();
}

void MQTTClient::disconnect() {
  mqttClient.disconnect();
  WiFi.disconnect();
  wifiConnected = false;
}
