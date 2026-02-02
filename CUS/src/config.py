"""
Configuration file for the Control Unit Subsystem (CUS)
Smart Tank Monitoring System
"""

# ====================
# System Parameters
# ====================

# Rainwater level thresholds (in cm)
L1_THRESHOLD = 30  # Level 1: triggers 50% valve opening after T1 time
L2_THRESHOLD = 50  # Level 2: triggers immediate 100% valve opening

# Time thresholds (in seconds)
T1_TIME = 10  # Time level must stay above L1 before opening valve to 50%
T2_TIMEOUT = 30  # Timeout for TMS connection before entering UNCONNECTED state

# Number of measurements to keep for dashboard graph
N_MEASUREMENTS = 100

# ====================
# MQTT Configuration
# ====================

MQTT_BROKER = "localhost"
MQTT_PORT = 1883
MQTT_KEEPALIVE = 60

# MQTT Topics
MQTT_TOPIC_RAINWATER_LEVEL = "tms/rainwater/level"  # Subscribe: receive level data from TMS
MQTT_TOPIC_TMS_STATUS = "tms/status"  # Subscribe: TMS status updates
MQTT_TOPIC_CUS_ACK = "cus/ack"  # Publish: acknowledgments to TMS

# MQTT Client ID
MQTT_CLIENT_ID = "CUS_Controller"

# ====================
# Serial Communication
# ====================

SERIAL_PORT = "COM3"  # Change to /dev/ttyUSB0 or /dev/ttyACM0 on Linux
SERIAL_BAUDRATE = 9600
SERIAL_TIMEOUT = 1  # seconds

# ====================
# HTTP Server
# ====================

HTTP_HOST = "0.0.0.0"  # Listen on all interfaces
HTTP_PORT = 5000
HTTP_DEBUG = True  # Set to False in production

# ====================
# System Modes
# ====================

MODE_AUTOMATIC = "AUTOMATIC"
MODE_MANUAL = "MANUAL"
MODE_UNCONNECTED = "UNCONNECTED"

# Default mode on startup
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

LOG_LEVEL = "INFO"  # DEBUG, INFO, WARNING, ERROR, CRITICAL
LOG_FORMAT = '%(asctime)s - %(name)s - %(levelname)s - %(message)s'
