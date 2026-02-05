# Tank Monitoring Subsystem (TMS)

**Smart Tank Monitoring System - Tank Monitoring Subsystem**

The TMS monitors the water level in the rainwater tank using a sonar sensor and communicates the data to the Control Unit Subsystem (CUS) via MQTT over WiFi.

## System States

### INIT
- System is initializing hardware and software components.
- **Visual Feedback**: Green and Red LEDs blink together.

### CONNECTING
- Attempting to establish WiFi and MQTT connection.
- **Visual Feedback**: Red LED is ON, Green LED is OFF.

### CONNECTED
- Successfully connected to the network and MQTT broker.
- Initial state before entering active monitoring.
- **Visual Feedback**: Green LED is ON, Red LED is OFF.

### MONITORING
- Normal operation: reading water level from the sonar sensor at a specific sampling frequency (F).
- Data is published to the MQTT topic `tms/rainwater/level` in JSON format.
- **Visual Feedback**: Green LED is ON, Red LED is OFF.

### DISCONNECTED
- Network or MQTT connection lost.
- System attempts to reconnect automatically.
- **Visual Feedback**: Red LED is ON, Green LED is OFF.

## State Machine Diagram

```
            ┌────────────┐
            │    INIT    │
            └─────┬──────┘
                  │
                  ▼
            ┌────────────┐     Retry
            │ CONNECTING │───┐
            └─────┬──────┘   │
                  │          │
          Success │          │
                  ▼          │
            ┌────────────┐   │
            │ CONNECTED  │   │
            └─────┬──────┘   │
                  │          │
                  ▼          │
    ┌─────────────┴──────────▼─┐
    │        MONITORING        │◄──────┐
    └─────────────┬────────────┘       │
                  │                    │
  Connection Lost │          Reconnected
                  ▼                    │
            ┌────────────┐             │
            │DISCONNECTED│─────────────┘
            └────────────┘
```

## MQTT Data Format

The TMS publishes water level data in the following JSON format:

```json
{
  "level": 75.5,
  "distance": 124.5,
  "timestamp": 1706800000,
  "state": "MONITORING"
}
```

- `level`: Current water level in cm.
- `distance`: Distance from the sensor to the water surface in cm.
- `timestamp`: System uptime in seconds.
- `state`: Current FSM state.

## Project Structure

```
TMS/
├── platformio.ini          # PlatformIO configuration
└── src/
    ├── config.h           # WiFi, MQTT, and pin configuration
    ├── main.cpp           # Main entry point and task setup
    ├── devices/           # Hardware abstractions
    │   ├── Sonar.h/cpp    # HC-SR04 sonar interface
    │   └── Led.h/cpp      # LED control interface
    ├── kernel/            # Core utilities
    │   ├── Scheduler.h/cpp # Task scheduler
    │   ├── Task.h         # Task base class
    │   └── MQTTClient.h/cpp # MQTT and WiFi management
    ├── model/             # Data models and state management
    │   ├── TMSState.h     # FSM states and StateManager
    │   └── WaterLevelData.h/cpp # Water level data structure
    └── task/              # Scheduled tasks
        ├── MonitoringTask.h/cpp # Sensor reading and publishing
        ├── MQTTTask.h/cpp       # Connection management
        └── LEDTask.h/cpp        # Visual feedback management
```
