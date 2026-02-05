#ifndef __CONFIG__
#define __CONFIG__

// ===== WiFi Configuration =====
#define WIFI_SSID "iPhone di Giuseppe"
#define WIFI_PASSWORD "giuse2004"
#define WIFI_TIMEOUT 20000                   // WiFi connection timeout (ms)

// ===== MQTT Configuration =====
#define MQTT_BROKER "broker.mqtt-dashboard.com"  // MQTT broker host
#define MQTT_PORT 1883                       // MQTT broker port
#define MQTT_CLIENT_ID "TMS_ESP32"           // MQTT client ID
#define MQTT_TOPIC "tms/rainwater/level"    // MQTT topic for water level data
#define MQTT_USERNAME ""                     // MQTT username (empty if not required)
#define MQTT_PASSWORD ""                     // MQTT password (empty if not required)
#define MQTT_RECONNECT_DELAY 5000            // MQTT reconnection delay (ms)
#define MQTT_MAX_RECONNECT_DELAY 60000       // Maximum reconnection delay (ms)

// ===== Pin Configuration =====
#define SONAR_TRIG_PIN 13                     // Sonar trigger pin
#define SONAR_ECHO_PIN 14                     // Sonar echo pin
#define GREEN_LED_PIN 18                      // Green LED pin (status OK)
#define RED_LED_PIN 19                        // Red LED pin (network error)

// ===== System Parameters =====
#define SAMPLING_FREQUENCY 1000              // F = 1 Hz (1000ms period)
#define TANK_HEIGHT 200.0                    // Tank height in cm
#define SONAR_TIMEOUT 30000                  // Sonar timeout in microseconds
#define DISCONNECT_TIMEOUT 10000             // Time to consider disconnected (ms)
#define LED_BLINK_PERIOD 500                 // LED blink period for init state (ms)

// ===== Task Periods =====
#define MONITORING_TASK_PERIOD SAMPLING_FREQUENCY
#define MQTT_TASK_PERIOD 100                 // MQTT task period (ms)
#define LED_TASK_PERIOD 200                  // LED task period (ms)

// ===== Debug Configuration =====
#define DEBUG_ENABLED true                   // Enable/disable serial debug output
#define SERIAL_BAUD_RATE 115200              // Serial communication baud rate

// ===== Debug Macros =====
#if DEBUG_ENABLED
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINTF(x, ...) Serial.printf(x, __VA_ARGS__)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(x, ...)
#endif

#endif
