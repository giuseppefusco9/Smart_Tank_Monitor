# Smart Tank Monitoring System

An IoT-based solution for real-time monitoring and control of rainwater levels in a tank. The system is composed of four integrated subsystems that communicate via MQTT, Serial, and HTTP protocols.

## System Overview

The Smart Tank Monitoring System provides automated water management by measuring tank levels and controlling a distribution valve. It features manual overrides, real-time visual feedback, and a web-based dashboard for remote operations.

### Subsystems

-   **[TMS (Tank Monitoring Subsystem)](TMS/README.md)**: ESP32-based unit that uses a sonar sensor to measure water levels and publishes data via **MQTT**.
-   **[CUS (Control Unit Subsystem)](CUS/README.md)**: Python-based central controller that coordinates all data flow, manages automation logic, and exposes a **REST API**.
-   **[WCS (Water Channel Subsystem)](WCS/README.md)**: Arduino-based unit that controls the water valve, provides local LCD feedback, and communicates with the CUS via **Serial**.
-   **[DBS (Dashboard Subsystem)](DBS/README.md)**: Web interface for real-time visualization of system status, historical data charts, and remote control.

## Video Demonstration
[Link to video](https://liveunibo-my.sharepoint.com/:f:/g/personal/giuseppe_fusco9_studio_unibo_it/IgAHWy9MQva5Rozwzr-5TzQwAQlLZod_qbH8uCK1wmRtpeo?e=gD2fVq)