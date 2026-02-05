# Control Unit Subsystem (CUS)

**Smart Tank Monitoring System - Control Unit Subsystem**

The CUS acts as the central hub of the Smart Tank Monitoring System. It coordinates data flow between the Tank Monitoring Subsystem (TMS), the Water Channel Subsystem (WCS), and the Dashboard Subsystem (DBS).

## Architecture Overview

The CUS is a Python-based application that manages three primary communication interfaces:

1.  **MQTT (with TMS)**: Receives real-time water level data from the ESP32-based TMS.
2.  **Serial (with WCS)**: Sends valve opening commands and display updates to the Arduino-based WCS; receives manual mode overrides and valve adjustments.
3.  **HTTP/REST (with DBS)**: Serves as a backend for the web dashboard, providing system status and accepting remote control commands.

## Key Features

-   **Autonomous Control**: Automatically adjusts the water valve based on predefined water level thresholds (L1, L2).
-   **State Management**: maintains the global system state (AUTOMATIC, MANUAL, UNCONNECTED).
-   **Graceful Recovery**: Detects WCS disconnection and manages fallback logic.
-   **Remote Monitoring**: Exposes a REST API for the frontend dashboard.

## Getting Started

### Prerequisites

-   Python 3.8+
-   [uv](https://github.com/astral-sh/uv) (recommended for dependency management)

### Installation

```bash
# Clone the repository and navigate to the CUS directory
cd CUS

# Install dependencies and create a virtual environment
uv sync
```

### Running the Subsystem

```bash
# Start the CUS main controller
uv run cus_main.py
```

## Project Structure

```
CUS/
├── cus_main.py         # Main entry point and coordinator
├── src/
│   ├── business_logic.py  # Automation logic and threshold management
│   ├── config.py         # System-wide configuration (MQTT/Serial/HTTP)
│   ├── http_server.py    # Flask-based REST API for the DBS
│   ├── mqtt_handler.py   # MQTT client for communication with TMS
│   ├── serial_handler.py # Serial (JSON) communication with WCS
│   ├── state_manager.py  # Centralized system state store
│   └── __init__.py
├── pyproject.toml      # Dependency management (uv/hatchling)
└── uv.lock             # Locked dependencies
```