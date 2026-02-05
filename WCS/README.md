# Water Channel Subsystem (WCS)

**Smart Tank Monitoring System - Water Channel Subsystem**

The WCS controls the valve between the rainwater tank and the water channel, displays system status on an LCD, and communicates with the Control Unit Subsystem (CUS) via Serial.

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
    │   ├── pot.h/cpp
    │   └── ...
    ├── kernel/            # Core utilities
    │   ├── Scheduler.h/cpp
    │   ├── Task.h
    │   ├── Logger.h/cpp
    │   └── SerialComm.h/cpp  # JSON serial handling
    └── tasks/
        └── WCSTask.h/cpp  # Main WCS logic
```
