# Control Unit Subsystem (CUS)

**Smart Tank Monitoring System - Control Unit Subsystem**

The CUS is the central coordinator for the Tank Monitoring System, managing communication between all subsystems and implementing the automatic valve control logic.

## Features

- **MQTT Communication** with Tank Monitoring Subsystem (TMS)
- **Serial Communication** with Water Channel Subsystem (WCS)
- **HTTP REST API** for Dashboard Subsystem (DBS)
- **Automatic Valve Control** based on L1/L2 water level thresholds
- **Manual Mode** for user-controlled valve operation
- **UNCONNECTED State** for TMS timeout detection

## Project Structure

```
CUS/
├── src/                      # Source code directory
│   ├── config.py            # Configuration parameters
│   ├── state_manager.py     # Thread-safe state management
│   ├── business_logic.py    # Core valve control policy
│   ├── mqtt_handler.py      # MQTT communication with TMS
│   ├── serial_handler.py    # Serial communication with WCS
│   └── http_server.py       # HTTP REST API server
├── cus_main.py              # Main entry point
├── pyproject.toml           # Project dependencies (uv)
├── .gitignore              # Git ignore rules
└── README.md               # This file
```

## Installation

### 1. Install uv

`uv` is a fast Python package installer and resolver written in Rust.

**Windows (PowerShell):**
```powershell
powershell -c "irm https://astral.sh/uv/install.ps1 | iex"
```

**Linux/macOS:**
```bash
curl -LsSf https://astral.sh/uv/install.sh | sh
```

Or with pip:
```bash
pip install uv
```

### 2. Create Virtual Environment

```bash
cd CUS
uv venv
```

This creates a `.venv` directory with a clean Python environment.

### 3. Activate Virtual Environment

**Windows (PowerShell):**
```powershell
.venv\Scripts\Activate.ps1
```

**Windows (CMD):**
```cmd
.venv\Scripts\activate.bat
```

**Linux/macOS:**
```bash
source .venv/bin/activate
```

### 4. Install Dependencies

```bash
uv sync
```

This installs all dependencies from `pyproject.toml` (paho-mqtt, pyserial, flask, flask-cors).

### 5. Install MQTT Broker

You need an MQTT broker for TMS communication. 
In this case, we'll use **Mosquitto** (Windows: https://mosquitto.org/download/)

## Configuration

Edit `src/config.py` to configure the system:

### Key Parameters

```python
# Water level thresholds (in cm)
L1_THRESHOLD = 30  # Triggers 50% valve opening after T1 seconds
L2_THRESHOLD = 50  # Triggers 100% valve opening immediately

# Time thresholds (in seconds)
T1_TIME = 10       # Delay before opening valve at L1
T2_TIMEOUT = 30    # TMS connection timeout

# MQTT Configuration
MQTT_BROKER = "localhost"
MQTT_PORT = 1883

# Serial Configuration
SERIAL_PORT = "COM3"  # Change to /dev/ttyUSB0 on Linux
SERIAL_BAUDRATE = 9600

# HTTP Server
HTTP_HOST = "0.0.0.0"
HTTP_PORT = 5000
```

**Important:** Update `SERIAL_PORT` to match your Arduino's port.

## How to run the CUS

### 1. Start the MQTT Broker 

```bash
mosquitto
```

### 2. Run the CUS

**With uv (recommended):**
```bash
uv run cus_main.py
```

**Or with activated virtual environment:**
```bash
# After activating .venv
python cus_main.py
```

You should see:
```
╔═══════════════════════════════════════════════════════════╗
║   Smart Tank Monitoring System - Control Unit Subsystem   ║
║                        (CUS)                              ║
╚═══════════════════════════════════════════════════════════╝

============================================================
Starting Control Unit Subsystem
============================================================
✓ Business logic started
✓ MQTT connected
✓ Serial port connected
✓ Starting HTTP server...
============================================================
Control Unit Subsystem is running
HTTP API available at http://localhost:5000
Press Ctrl+C to stop
============================================================
```

## HTTP API Reference

The CUS provides a REST API for the Dashboard Subsystem:

### Endpoints

#### Get System Status
```bash
GET /api/status
```
Returns current mode, valve opening, and latest water level.

**Example:**
```bash
curl http://localhost:5000/api/status
```

#### Get Water Level History
```bash
GET /api/rainwater
```
Returns last 100 water level measurements.

#### Switch System Mode
```bash
POST /api/mode
Content-Type: application/json

{"mode": "MANUAL"}  # or "AUTOMATIC"
```

**Example:**
```bash
curl -X POST http://localhost:5000/api/mode \
  -H "Content-Type: application/json" \
  -d '{"mode":"MANUAL"}'
```

#### Set Valve Opening (Manual Mode Only)
```bash
POST /api/valve
Content-Type: application/json

{"opening": 75}  # 0-100%
```

**Example:**
```bash
curl -X POST http://localhost:5000/api/valve \
  -H "Content-Type: application/json" \
  -d '{"opening":75}'
```

#### Get Configuration
```bash
GET /api/config
```
Returns system configuration (L1, L2, T1, T2 values).

#### Health Check
```bash
GET /health
```

## Communication Protocols

### MQTT (with TMS)

**Subscribed Topics:**
- `tms/rainwater/level` - Receives water level data
  ```json
  {"level": 35.5, "timestamp": 1234567890}
  ```

**Published Topics:**
- `cus/ack` - Sends acknowledgments to TMS

### Serial (with WCS)

**Messages sent to WCS:**
- Valve command: `{"type": "valve", "value": 50}\n`
- Display update: `{"type": "display", "mode": "AUTOMATIC", "valve": 50}\n`

**Messages received from WCS:**
- Mode change: `{"type": "mode", "value": "MANUAL"}\n`
- Manual valve: `{"type": "valve", "value": 75}\n`

## System Modes

### AUTOMATIC Mode (Default)
The system automatically controls the valve based on water level:

- **Level ≥ L2 (50cm):** Valve opens to 100% immediately
- **L1 (30cm) ≤ Level < L2 for T1 (10s):** Valve opens to 50%
- **Level < L1:** Valve closes to 0%

### MANUAL Mode
User has full control of the valve via:
- Dashboard (HTTP API)
- WCS potentiometer

Automatic control is disabled.

### UNCONNECTED State
Triggered when no data from TMS for T2 (30s):
- Valve automatically closes (safety)
- Mode cannot be changed
- Automatically restores when TMS reconnects
