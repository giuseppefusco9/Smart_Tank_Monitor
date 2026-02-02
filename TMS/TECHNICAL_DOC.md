# TMS Implementation - Technical Documentation

## System Design

### Finite State Machine (FSM)

The TMS uses a 5-state FSM to manage its operational lifecycle:

![TMS FSM Diagram](assignment-03-tms-fsm.png)

**State Descriptions:**

| State | Description | LED Status | Duration |
|-------|-------------|------------|----------|
| **INIT** | System initialization, hardware setup | Both blinking | ~1-2 seconds |
| **CONNECTING** | Attempting WiFi and MQTT connection | Red ON | Until connected or timeout |
| **CONNECTED** | Connection established, transitioning to monitoring | Green ON | Brief (~100-200ms) |
| **MONITORING** | Normal operation - reading and sending data | Green ON | Continuous |
| **DISCONNECTED** | Network error, attempting reconnection | Red ON | Until reconnected |

**State Transition Logic:**

```cpp
// Implemented in MQTTTask.h
void tick() {
    bool isConnected = mqttClient->isFullyConnected();
    
    switch (currentState) {
        case INIT:
            // Wait for main to transition to CONNECTING
            break;
            
        case CONNECTING:
            if (mqttClient->reconnect()) {
                stateManager->setState(CONNECTED);
            }
            break;
            
        case CONNECTED:
            if (isConnected) {
                stateManager->setState(MONITORING);
            } else {
                stateManager->setState(CONNECTING);
            }
            break;
            
        case MONITORING:
            if (!isConnected) {
                stateManager->setState(DISCONNECTED);
            }
            break;
            
        case DISCONNECTED:
            if (mqttClient->reconnect()) {
                stateManager->setState(MONITORING);
            }
            break;
    }
}
```

### Task Architecture

The system uses a cooperative multitasking approach with three tasks:

```
┌─────────────────────────────────────────────┐
│            Task Scheduler (10ms)            │
│                                             │
│  ┌──────────────────────────────────────┐  │
│  │  LEDTask (200ms)                     │  │
│  │  - Update LED status based on FSM    │  │
│  │  - Handle blinking patterns          │  │
│  └──────────────────────────────────────┘  │
│                                             │
│  ┌──────────────────────────────────────┐  │
│  │  MQTTTask (100ms)                    │  │
│  │  - Maintain MQTT connection          │  │
│  │  - Process MQTT loop                 │  │
│  │  - Handle FSM transitions            │  │
│  │  - Reconnection with backoff         │  │
│  └──────────────────────────────────────┘  │
│                                             │
│  ┌──────────────────────────────────────┐  │
│  │  MonitoringTask (1000ms)             │  │
│  │  - Read sonar sensor                 │  │
│  │  - Calculate water level             │  │
│  │  - Publish to MQTT (JSON)            │  │
│  └──────────────────────────────────────┘  │
└─────────────────────────────────────────────┘
```

**Task Execution Flow:**

1. Scheduler wakes up every 10ms
2. For each task:
   - Check if task period has elapsed
   - If yes, execute `task->tick()`
   - Update time tracking
3. Sleep until next cycle

### Component Interaction Diagram

```
┌──────────┐     WiFi/MQTT      ┌──────────────┐
│   ESP32  │ ←─────────────────→ │ MQTT Broker  │
│          │                     │    (CUS)     │
└──────────┘                     └──────────────┘
     │
     │ GPIO 5 (Trig)
     │ GPIO 18 (Echo)
     ├────────────→ ┌──────────────┐
     │              │ Sonar Sensor │
     │              │  (HC-SR04)   │
     │              └──────────────┘
     │
     │ GPIO 2
     ├────────────→ ┌──────────────┐
     │              │  Green LED   │
     │              └──────────────┘
     │
     │ GPIO 4
     └────────────→ ┌──────────────┐
                    │   Red LED    │
                    └──────────────┘
```

## Data Flow

### Water Level Measurement Flow

```
1. Timer expires (SAMPLING_FREQUENCY)
         ↓
2. MonitoringTask.tick() called
         ↓
3. sonar->getDistance()
         ↓
4. Calculate level = TANK_HEIGHT - distance
         ↓
5. Create WaterLevelData object
         ↓
6. Serialize to JSON
         ↓
7. mqttClient->publish(MQTT_TOPIC, json)
         ↓
8. Data sent to MQTT Broker (CUS)
```

### Example Data Packet

```json
{
  "distance": 45.5,     // cm from top of tank
  "level": 154.5,       // cm of water in tank
  "timestamp": 1707345678,
  "state": "MONITORING"
}
```

**Calculation:**
- Tank Height: 200 cm
- Sonar reads: 45.5 cm (distance to water surface)
- Water Level: 200 - 45.5 = 154.5 cm

## Network Communication

### MQTT Protocol Details

**Connection:**
- Protocol: MQTT v3.1.1
- QoS: 1 (At least once delivery)
- Clean Session: true
- Keep Alive: 60 seconds

**Reconnection Strategy:**
- Initial delay: 5 seconds
- Exponential backoff: delay × 2 after each failure
- Maximum delay: 60 seconds
- Continuous retry until connected

### WiFi Connection

```cpp
// Connection process
WiFi.begin(SSID, PASSWORD);
// Wait up to WIFI_TIMEOUT (20s)
// If successful → proceed to MQTT
// If failed → retry from beginning
```

## Memory and Performance

### Memory Usage Estimate

- **Flash (Program)**: ~300 KB
  - ESP32 framework: ~200 KB
  - Application code: ~50 KB
  - Libraries (MQTT, JSON): ~50 KB

- **RAM (Heap + Stack)**: ~50 KB
  - Task objects: ~5 KB
  - MQTT buffers: ~10 KB
  - WiFi stack: ~30 KB
  - Application variables: ~5 KB

### CPU Usage

- Idle: ~95% (waiting for task periods)
- Active: ~5%
  - WiFi/MQTT: 2-3%
  - Task execution: 1-2%
  - Sonar reading: <1%

## Error Handling

### Network Failures

**WiFi Disconnection:**
1. MQTTTask detects `WiFi.status() != WL_CONNECTED`
2. Transitions to DISCONNECTED state
3. Attempts reconnection with backoff
4. Red LED indicates error

**MQTT Disconnection:**
1. MQTTTask detects `mqttClient.connected() == false`
2. Transitions to DISCONNECTED state
3. Attempts MQTT reconnection
4. If WiFi also down, reconnects WiFi first

### Sensor Failures

**Invalid Sonar Reading:**
- Returns `NO_OBJ_DETECTED` (-1)
- WaterLevelData marked as invalid
- No MQTT publish for that reading
- Next reading attempted on schedule

## Configuration Reference

### Critical Parameters

| Parameter | Location | Default | Impact |
|-----------|----------|---------|--------|
| WIFI_SSID | config.h | "YOUR_WIFI_SSID" | Must match network |
| MQTT_BROKER | config.h | "localhost" | Must be CUS IP |
| SAMPLING_FREQUENCY | config.h | 1000 ms | Data rate |
| TANK_HEIGHT | config.h | 200 cm | Level calculation |

### Pin Configuration

All pins configurable in `config.h`:

```cpp
#define SONAR_TRIG_PIN 5
#define SONAR_ECHO_PIN 18
#define GREEN_LED_PIN 2
#define RED_LED_PIN 4
```

**Important:** GPIO 2 is often the built-in LED on ESP32. If not visible, use external LED or change to another pin.

## Development Notes

### Adding New Features

**To add a new task:**
1. Create task class inheriting from `Task`
2. Implement `init()` and `tick()` methods
3. Add task to scheduler in `main.cpp`
4. Ensure task period doesn't overload scheduler

**To add MQTT subscriptions:**
1. Set callback in `MQTTClient.h`: `mqttClient.setCallback(callback)`
2. Subscribe in `connectMQTT()`: `mqttClient.subscribe("topic")`
3. Handle incoming messages in callback

### Testing Without Hardware

For testing without physical hardware:
1. Comment out sonar readings
2. Use mock data: `float distance = 50.0;`
3. Test MQTT connectivity and FSM transitions
4. Verify LED states (if using built-in LED)

## Build Instructions

### Using PlatformIO

```bash
# Install PlatformIO (if not installed)
pip install platformio

# Navigate to project
cd TMS

# Build
platformio run

# Upload to ESP32
platformio run --target upload

# Monitor serial output
platformio device monitor
```

### Using Arduino IDE

1. Install ESP32 board support
2. Install libraries:
   - PubSubClient (v2.8+)
   - ArduinoJson (v7.2+)
3. Open `src/main.cpp`
4. Select board: ESP32 Dev Module
5. Upload

## Troubleshooting Guide

### Common Issues

**Issue: Cannot connect to WiFi**
- Check SSID/password in config.h
- Ensure 2.4GHz network (ESP32 limitation)
- Check signal strength
- Look for "WiFi connected" in serial output

**Issue: MQTT connection fails**
- Verify broker IP is reachable
- Check broker is running: `mosquitto -v`
- Test with: `mosquitto_pub -h <ip> -t test -m hello`
- Check firewall settings

**Issue: Erratic sonar readings**
- Ensure stable power supply (5V)
- Check wiring (especially Echo pin)
- Avoid obstacles in front of sensor
- Increase SONAR_TIMEOUT if needed

**Issue: LEDs not responding**
- Verify pin numbers in config.h
- Check resistor values (220Ω recommended)
- Test with simple blink sketch
- Ensure proper GND connection

## Performance Optimization

### Reduce Power Consumption

```cpp
// In config.h, increase sampling frequency
#define SAMPLING_FREQUENCY 5000  // 5 seconds instead of 1

// Enable light sleep between tasks
// (requires additional ESP32-specific code)
```

### Improve Response Time

```cpp
// Decrease task periods
#define MQTT_TASK_PERIOD 50      // From 100ms to 50ms
#define LED_TASK_PERIOD 100      // From 200ms to 100ms
```

### Reduce Network Traffic

```cpp
// Only send data when level changes significantly
if (abs(newLevel - lastLevel) > 1.0) {  // 1cm threshold
    mqttClient->publish(...);
}
```

## Future Enhancements

- [ ] OTA (Over-The-Air) firmware updates
- [ ] Local data buffering when MQTT unavailable
- [ ] Multiple sensor support
- [ ] Watchdog timer for crash recovery
- [ ] Sleep mode for battery operation
- [ ] Web-based configuration interface
