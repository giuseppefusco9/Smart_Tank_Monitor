"""
Configuration file for the Control Unit Subsystem (CUS)
Smart Tank Monitoring System
"""

# ====================
# Assignment Parameters
# ====================
# These parameters correspond to the assignment specification

# L₁ - First threshold level (cm)
# When rainwater level exceeds L₁ for T₁ time → valve opens to 50%
L1_THRESHOLD = 30

# L₂ - Second threshold level (cm)
# When rainwater level exceeds L₂ → valve immediately opens to 100%
L2_THRESHOLD = 50

# T₁ - Time delay at L₁ threshold (seconds)
# How long level must stay above L₁ before opening valve to 50%
T1_TIME = 10

# T₂ - TMS connection timeout (seconds)
# Time without TMS data before entering UNCONNECTED state
T2_TIMEOUT = 30

# N - Number of measurements to keep for dashboard graph
N_MEASUREMENTS = 100

# ====================
# MQTT Configuration
# ====================

MQTT_BROKER = "broker.mqtt-dashboard.com"  # MQTT broker host
MQTT_PORT = 1883
MQTT_KEEPALIVE = 60

# MQTT Topics
MQTT_TOPIC_RAINWATER_LEVEL = "tms/rainwater/level"  # Subscribe: receive level data from TMS

# MQTT Client ID
MQTT_CLIENT_ID = "CUS_Controller"

# ====================
# Serial Communication
# ====================

SERIAL_PORT = "COM11"
SERIAL_BAUDRATE = 9600
SERIAL_TIMEOUT = 1  # seconds

# ====================
# HTTP Server
# ====================

HTTP_HOST = "0.0.0.0"  # Listen on all interfaces
HTTP_PORT = 5000
HTTP_DEBUG = True 

# ====================
# System Modes
# ====================

MODE_AUTOMATIC = "AUTOMATIC"
MODE_MANUAL = "MANUAL"
MODE_UNCONNECTED = "UNCONNECTED"
DEFAULT_MODE = MODE_AUTOMATIC

# ====================
# Valve Control
# ====================

VALVE_MIN = 0  # Minimum valve opening (%)
VALVE_MAX = 100  # Maximum valve opening (%)
VALVE_L1_OPENING = 50  # Valve opening when level > L1 for T1 time
VALVE_L2_OPENING = 100  # Valve opening when level > L2
VALVE_CLOSED = 0  # Valve opening when level < L1

# ====================
# Logging
# ====================
LOG_LEVEL = "INFO"
LOG_FORMAT = '%(asctime)s - %(name)s - %(levelname)s - %(message)s'
