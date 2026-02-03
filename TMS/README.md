# Tank Monitoring Subsystem (TMS)

## Overview

The Tank Monitoring Subsystem (TMS) is an ESP32-based embedded system that monitors rainwater levels in a tank using a sonar sensor. It communicates with the Control Unit Subsystem (CUS) via MQTT protocol.

## Architecture

### Finite State Machine

The TMS operates using a 5-state FSM:

```
INIT → CONNECTING → CONNECTED → MONITORING ⟷ DISCONNECTED
                ↑                    ↓
                └────────────────────┘
```

**States:**
- **INIT**: System initialization, hardware setup
- **CONNECTING**: Attempting WiFi and MQTT connection
- **CONNECTED**: Successfully connected (transition state)
- **MONITORING**: Normal operation - reading and transmitting data
- **DISCONNECTED**: Network/MQTT connection lost, attempting reconnection

### Task Architecture

The system uses three concurrent tasks managed by a scheduler:

1. **LEDTask** (200ms period)
   - Controls green and red status LEDs
   - Provides visual feedback of system state

2. **MQTTTask** (100ms period)
   - Manages WiFi and MQTT connections
   - Handles FSM state transitions
   - Implements reconnection with exponential backoff

3. **MonitoringTask** (configurable, default 1000ms)
   - Reads sonar sensor
   - Calculates water level
   - Publishes data to MQTT broker

## Hardware Configuration

### Components
- ESP32 Development Board
- HC-SR04 Sonar Sensor (or similar)
- 1x Green LED (status OK)
- 1x Red LED (network error)
- 2x 220Ω Resistors (for LEDs)

### Pin Assignments (Default)

| Component | Pin | GPIO |
|-----------|-----|------|
| Sonar Trigger | 5 | GPIO 5 |
| Sonar Echo | 18 | GPIO 18 |
| Green LED | 2 | GPIO 2 |
| Red LED | 4 | GPIO 4 |

*Can be modified in `src/config.h`*

### Circuit Connections

**Sonar:**
- VCC → 5V
- GND → GND
- Trig → GPIO 5
- Echo → GPIO 18

**LEDs:**
- Green LED: GPIO 2 → 220Ω → LED → GND
- Red LED: GPIO 4 → 220Ω → LED → GND

## Software Structure

```
src/
├── config.h                    # Configuration (WiFi, MQTT, pins, parameters)
├── main.cpp                    # Main application
├── devices/
│   ├── Led.h/cpp              # LED control
│   ├── Light.h                # LED interface
│   ├── Sonar.h/cpp            # Sonar sensor
│   └── ProximitySensor.h      # Sensor interface
├── kernel/
│   ├── Task.h                 # Task base class
│   ├── Scheduler.h            # Task scheduler
│   └── MQTTClient.h           # MQTT client wrapper
├── model/
│   ├── TMSState.h             # FSM states and state manager
│   └── WaterLevelData.h       # Data structure for measurements
└── task/
    ├── MonitoringTask.h       # Water level monitoring
    ├── MQTTTask.h             # MQTT connection management
    └── LEDTask.h              # LED status indicators
```

## Configuration

Edit `src/config.h` before building:

### WiFi Settings
```cpp
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
```

### MQTT Settings
```cpp
#define MQTT_BROKER "localhost"        // MQTT broker IP/hostname
#define MQTT_PORT 1883                 // MQTT broker port
#define MQTT_TOPIC "tank/waterlevel"   // Topic for publishing
```

### System Parameters
```cpp
#define SAMPLING_FREQUENCY 1000        // Sampling period (ms)
#define TANK_HEIGHT 200.0              // Tank height (cm)
```

## MQTT Protocol

### Published Messages

**Topic:** `tank/waterlevel` (configurable)

**Format:** JSON
```json
{
  "distance": 45.5,
  "level": 154.5,
  "timestamp": 1675340567,
  "state": "MONITORING"
}
```

**Fields:**
- `distance`: Raw distance from sonar (cm)
- `level`: Calculated water level (cm) = Tank Height - Distance
- `timestamp`: Unix timestamp in seconds
- `state`: Current FSM state

**QoS:** 1 (At least once delivery)

## LED Status Indicators

| State | Green LED | Red LED | Meaning |
|-------|-----------|---------|---------|
| INIT | Blinking | Blinking | System starting |
| CONNECTING | OFF | ON | Attempting connection |
| CONNECTED | ON | OFF | Connected (brief transition) |
| MONITORING | ON | OFF | Normal operation |
| DISCONNECTED | OFF | ON | Network problem |

## Building and Uploading

### Prerequisites
- PlatformIO installed (VS Code extension or CLI)
- ESP32 board drivers installed

### Build
```bash
cd TMS
pio run
```

### Upload
```bash
pio run --target upload
```

### Monitor Serial Output
```bash
pio device monitor
```

## Testing

### 1. Hardware Test
- Connect ESP32 to computer
- Upload code
- Observe LED sequence:
  - Both LEDs blink (INIT)
  - Red LED on (CONNECTING)
  - Green LED on (MONITORING)

### 2. Sensor Test
- Monitor serial output
- Place object at various distances from sonar
- Verify distance and level calculations

### 3. MQTT Test
On a computer with MQTT broker access:
```bash
mosquitto_sub -h <broker_host> -t tank/waterlevel -v
```
You should see JSON messages at the configured sampling frequency.

### 4. Network Failure Test
- Disconnect WiFi or stop MQTT broker
- Verify red LED turns on
- Reconnect network
- Verify green LED returns

## Troubleshooting

### ESP32 Not Connecting to WiFi
- Verify SSID and password in `config.h`
- Check WiFi signal strength
- Ensure 2.4GHz network (ESP32 doesn't support 5GHz)

### MQTT Connection Fails
- Verify broker IP and port
- Check if broker requires authentication (update credentials in config.h)
- Test broker with: `mosquitto_sub -h <broker_host> -t test`

### Sonar Returns Invalid Readings
- Check wiring connections
- Ensure 5V power supply
- Verify no obstacles in sonar path during startup
- Check timeout value (`SONAR_TIMEOUT` in config.h)

### Serial Monitor Shows Nothing
- Check baud rate (should be 115200)
- Verify USB cable supports data transfer
- Press reset button on ESP32

## Debug Mode

Debug output is enabled by default. To disable:

In `src/config.h`:
```cpp
#define DEBUG_ENABLED false
```

## System Parameters

| Parameter | Default | Description |
|-----------|---------|-------------|
| SAMPLING_FREQUENCY | 1000 ms | Water level reading interval |
| TANK_HEIGHT | 200 cm | Tank height for level calculation |
| SONAR_TIMEOUT | 30000 µs | Sonar measurement timeout |
| WIFI_TIMEOUT | 20000 ms | WiFi connection timeout |
| MQTT_RECONNECT_DELAY | 5000 ms | Initial reconnection delay |
| MQTT_MAX_RECONNECT_DELAY | 60000 ms | Maximum reconnection delay |

## License

Part of the Smart Tank Monitoring System for ISI LT - a.y. 2025/2026
