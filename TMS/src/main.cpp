#include <Arduino.h>
#include "config.h"
#include "devices/Sonar.h"
#include "devices/Led.h"
#include "model/TMSState.h"
#include "model/WaterLevelData.h"
#include "kernel/MQTTClient.h"
#include "kernel/Scheduler.h"
#include "task/MonitoringTask.h"
#include "task/MQTTTask.h"
#include "task/LEDTask.h"

// ===== Global Objects =====
Sonar* sonar;
Led* greenLed;
Led* redLed;
StateManager* stateManager;
MQTTClient* mqttClient;
Scheduler* scheduler;

// ===== Tasks =====
MonitoringTask* monitoringTask;
MQTTTask* mqttTask;
LEDTask* ledTask;

/**
 * Initialize hardware components
 */
void initHardware() {
  DEBUG_PRINTLN("=== Initializing Hardware ===");
  
  // Initialize Serial for debugging
  Serial.begin(SERIAL_BAUD_RATE);
  delay(1000);
  DEBUG_PRINTLN("Serial initialized");

  // Initialize Sonar
  sonar = new Sonar(SONAR_ECHO_PIN, SONAR_TRIG_PIN, SONAR_TIMEOUT);
  DEBUG_PRINTLN("Sonar initialized");

  // Initialize LEDs
  greenLed = new Led(GREEN_LED_PIN);
  redLed = new Led(RED_LED_PIN);
  DEBUG_PRINTLN("LEDs initialized");

  // Test LEDs
  DEBUG_PRINTLN("Testing LEDs...");
  greenLed->switchOn();
  delay(300);
  greenLed->switchOff();
  redLed->switchOn();
  delay(300);
  redLed->switchOff();

  DEBUG_PRINTLN("Hardware initialization complete");
}

/**
 * Initialize software components
 */
void initSoftware() {
  DEBUG_PRINTLN("=== Initializing Software ===");

  // Initialize State Manager
  stateManager = new StateManager();
  stateManager->setState(INIT);
  DEBUG_PRINT("Initial state: ");
  DEBUG_PRINTLN(stateToString(stateManager->getState()));

  // Initialize MQTT Client
  mqttClient = new MQTTClient();
  DEBUG_PRINTLN("MQTT Client initialized");

  // Initialize Scheduler
  scheduler = new Scheduler(10); // 10ms base period
  scheduler->init(10);
  DEBUG_PRINTLN("Scheduler initialized");

  DEBUG_PRINTLN("Software initialization complete");
}

/**
 * Initialize and register tasks
 */
void initTasks() {
  DEBUG_PRINTLN("=== Initializing Tasks ===");

  // Create tasks
  monitoringTask = new MonitoringTask(sonar, mqttClient, stateManager);
  mqttTask = new MQTTTask(mqttClient, stateManager);
  ledTask = new LEDTask(greenLed, redLed, stateManager);

  // Initialize tasks with their periods
  monitoringTask->init(MONITORING_TASK_PERIOD);
  mqttTask->init(MQTT_TASK_PERIOD);
  ledTask->init(LED_TASK_PERIOD);

  // Register tasks with scheduler
  scheduler->addTask(ledTask);         // Priority: visual feedback
  scheduler->addTask(mqttTask);        // Priority: network management
  scheduler->addTask(monitoringTask);  // Priority: data collection

  DEBUG_PRINT("Registered ");
  DEBUG_PRINT(scheduler->getNumTasks());
  DEBUG_PRINTLN(" tasks");
  DEBUG_PRINTLN("Tasks initialization complete");
}

/**
 * Arduino setup function
 */
void setup() {
  // Initialize components
  initHardware();
  initSoftware();
  initTasks();

  DEBUG_PRINTLN("\n=== TMS Starting ===");
  DEBUG_PRINTLN("Tank Monitoring Subsystem v1.0");
  DEBUG_PRINT("Sampling Frequency: ");
  DEBUG_PRINT(1000.0 / SAMPLING_FREQUENCY);
  DEBUG_PRINTLN(" Hz");
  DEBUG_PRINT("Tank Height: ");
  DEBUG_PRINT(TANK_HEIGHT);
  DEBUG_PRINTLN(" cm");
  DEBUG_PRINT("MQTT Broker: ");
  DEBUG_PRINT(MQTT_BROKER);
  DEBUG_PRINT(":");
  DEBUG_PRINTLN(MQTT_PORT);
  DEBUG_PRINT("MQTT Topic: ");
  DEBUG_PRINTLN(MQTT_TOPIC);
  DEBUG_PRINTLN("=========================\n");

  // Transition to CONNECTING state
  DEBUG_PRINTLN("Transitioning to CONNECTING state");
  stateManager->setState(CONNECTING);
}

/**
 * Arduino loop function
 */
void loop() {
  // Execute scheduler
  scheduler->schedule();
  
  // Small delay to prevent tight loop
  delay(scheduler->getBasePeriod());

  // Periodic status update (every 30 seconds)
  static unsigned long lastStatusPrint = 0;
  unsigned long now = millis();
  if (DEBUG_ENABLED && (now - lastStatusPrint) >= 30000) {
    DEBUG_PRINTLN("\n--- Status Update ---");
    DEBUG_PRINT("State: ");
    DEBUG_PRINTLN(stateToString(stateManager->getState()));
    DEBUG_PRINT("WiFi: ");
    DEBUG_PRINTLN(mqttClient->isWiFiConnected() ? "Connected" : "Disconnected");
    DEBUG_PRINT("MQTT: ");
    DEBUG_PRINTLN(mqttClient->isConnected() ? "Connected" : "Disconnected");
    DEBUG_PRINT("Uptime: ");
    DEBUG_PRINT(now / 1000);
    DEBUG_PRINTLN(" seconds");
    
    // Read current water level
    float distance = sonar->getDistance();
    if (distance >= 0) {
      float level = TANK_HEIGHT - distance;
      DEBUG_PRINT("Current Water Level: ");
      DEBUG_PRINT(level);
      DEBUG_PRINTLN(" cm");
    }
    DEBUG_PRINTLN("--------------------\n");
    
    lastStatusPrint = now;
  }
}
