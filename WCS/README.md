# Water Channel Subsystem (WCS)

**Smart Tank Monitoring System - Water Channel Subsystem**

The WCS controls the valve between the rainwater tank and the water channel, displays system status on an LCD, and communicates with the Control Unit Subsystem (CUS) via Serial.

## Features

- **Servo Valve Control** - Controls water flow with 0-100% precision
- **I2C LCD Display** - Shows current mode and valve opening
- **Mode Switching** - Button to toggle between AUTOMATIC and MANUAL modes
- **Manual Control** - Potentiometer for manual valve adjustment
- **Serial Communication** - JSON-based messaging with CUS

## Hardware Components

### Arduino UNO
- **Servo Motor** (Pin 3) - Water valve actuator (0-180°)
- **16x2 I2C LCD** - System status display
- **Button** (Pin 9) - Mode switch (AUTOMATIC ↔ MANUAL)
- **Potentiometer** (A0) - Manual valve control
- **USB Serial** - Communication with CUS

## System Modes

### AUTOMATIC Mode
- CUS controls valve based on water level
- LCD shows "AUTOMATIC" and current valve percentage
- Button press switches to MANUAL

### MANUAL Mode
- User controls valve via potentiometer
- LCD shows "MANUAL" and current valve percentage
- Valve updates sent to CUS
- Button press switches to AUTOMATIC

### UNCONNECTED Mode
- Triggered when CUS disconnects
- Valve closes for safety (0%)
- LCD shows "UNCONNECTED"
- Automatically returns to AUTOMATIC when CUS reconnects

## Communication Protocol

### Messages FROM CUS → WCS

#### Valve Command
```json
{"type": "valve", "value": 50}
```
Sets valve to specified percentage (0-100%)

#### Display Update
```json
{"type": "display", "value": "AUTOMATIC"}
```
Updates mode and display

### Messages FROM WCS → CUS

#### Mode Change
```json
{"type": "mode", "value": "MANUAL"}
```
Sent when button is pressed

#### Manual Valve Update
```json
{"type": "valve", "value": 75}
```
Sent when potentiometer changes in MANUAL mode

## Installation

### 1. Hardware Setup
1. Connect servo motor to Pin 3
2. Connect button to Pin 9 (with pull-down resistor)
3. Connect potentiometer to A0
4. Connect I2C LCD (SDA/SCL)
5. Connect Arduino to PC via USB

### 2. Configure PlatformIO
```bash
cd WCS
```

### 3. Update LCD I2C Address
If LCD doesn't work, scan for I2C address and update in [`config.h`](file:///c:/Users/Giuseppe%20Fusco/Desktop/LabIOT/Smart_Tank_Monitor/WCS/src/config.h):
```cpp
#define LCD_I2C_ADDRESS 0x27  // Or 0x3F
```

### 4. Build and Upload
```bash
pio run --target upload
```

### 5. Monitor Serial Output
```bash
pio device monitor
```

## Configuration

Edit `src/config.h`:

```cpp
// Pin Configuration
#define SERVO_PIN 3          // Servo motor pin
#define BUTTON_PIN 9         // Mode button pin
#define POT_PIN A0           // Potentiometer pin

// Serial Configuration
#define SERIAL_BAUD 9600     // Must match CUS config

// LCD Configuration
#define LCD_I2C_ADDRESS 0x27 // I2C address (scan if needed)
```

## State Machine Diagram

```
        ┌──────────────┐
        │  AUTOMATIC   │◄──────┐
        └──────┬───────┘       │
               │               │
    Button     │      Button   │
      Press    │        Press  │
               ▼               │
        ┌──────────────┐       │
        │    MANUAL    │───────┘
        └──────────────┘
               ▲
               │
        CUS    │    CUS
     Timeout   │  Reconnect
               │
        ┌──────┴───────┐
        │ UNCONNECTED  │
        └──────────────┘
```

## LCD Display Format

```
┌────────────────┐
│ Mode: AUTO     │  ← Line 1: Current mode
│ Valve: 75%     │  ← Line 2: Valve opening
└────────────────┘
```

## Testing

### Test 1: Button Mode Switching
1. Power on WCS
2. LCD should show "AUTOMATIC"
3. Press button → LCD shows "MANUAL"
4. Press button → LCD shows "AUTOMATIC"

### Test 2: Manual Control
1. Switch to MANUAL mode
2. Turn potentiometer
3. Watch servo move and LCD update
4. Check serial monitor for valve messages

### Test 3: CUS Communication
1. Connect WCS to PC running CUS
2. CUS should detect WCS on COM port
3. CUS sends valve commands → Servo responds
4. Press WCS button → CUS logs mode change

## Troubleshooting

### LCD Not Working
- Scan I2C address: `pio device monitor` and check I2C scanner sketch
- Update `LCD_I2C_ADDRESS` in `config.h`
- Check wiring (SDA/SCL)

### Servo Not Moving
- Check servo power supply (5V, sufficient current)
- Verify `SERVO_PIN` matches hardware
- Check serial monitor for valve percentages

### Serial Communication Fails
- Verify baud rate matches CUS (9600)
- Check USB connection
- Ensure CUS is using correct COM port

## Project Structure

```
WCS/
├── platformio.ini          # PlatformIO configuration
├── README.md              # This file
└── src/
    ├── config.h           # Pin and system configuration
    ├── main.cpp           # Main entry point
    ├── devices/           # Hardware abstractions
    │   ├── servoMotorImpl.h/cpp
    │   ├── lcd.h/cpp
    │   ├── buttonimpl.h/cpp
    │   └── pot.h/cpp
    ├── kernel/            # Core utilities
    │   ├── Scheduler.h/cpp
    │   ├── Task.h
    │   ├── Logger.h/cpp
    │   └── SerialComm.h/cpp  # JSON serial handling
    └── tasks/
        └── WCSTask.h/cpp  # Main WCS logic
```

## Dependencies

- `Wire` - I2C communication
- `LiquidCrystal_I2C` - LCD display
- `Servo` - Servo control
- `ArduinoJson` - JSON parsing/serialization
- `EnableInterrupt` - Button debouncing

## Serial Commands for Testing

You can test WCS manually via serial monitor:

```json
{"type":"valve","value":50}
{"type":"display","value":"AUTOMATIC"}
{"type":"display","value":"MANUAL"}
{"type":"display","value":"UNCONNECTED"}
```

## License

Part of the Smart Tank Monitoring System project.
