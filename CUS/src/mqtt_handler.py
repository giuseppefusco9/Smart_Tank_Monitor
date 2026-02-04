"""
MQTT Handler for Control Unit Subsystem (CUS)
Handles communication with Tank Monitoring Subsystem (TMS)
"""

import json
import logging
import paho.mqtt.client as mqtt
from typing import Callable, Optional
from . import config


logger = logging.getLogger(__name__)


class MQTTHandler:
    """Handles MQTT communication with TMS"""
    
    def __init__(self, on_rainwater_level_callback: Callable[[float, float], None]):
        """
        Initialize MQTT handler
        
        Args:
            on_rainwater_level_callback: Callback function(level, timestamp) called when rainwater data received
        """
        self.client = mqtt.Client(client_id=config.MQTT_CLIENT_ID)
        self.on_rainwater_level = on_rainwater_level_callback
        
        # Set up MQTT callbacks
        self.client.on_connect = self._on_connect
        self.client.on_disconnect = self._on_disconnect
        self.client.on_message = self._on_message
        
        self._connected = False
    
    def connect(self):
        """Connect to MQTT broker"""
        try:
            logger.info(f"Connecting to MQTT broker at {config.MQTT_BROKER}:{config.MQTT_PORT}")
            self.client.connect(config.MQTT_BROKER, config.MQTT_PORT, config.MQTT_KEEPALIVE)
            self.client.loop_start()
        except Exception as e:
            logger.error(f"Failed to connect to MQTT broker: {e}")
            raise
    
    def disconnect(self):
        """Disconnect from MQTT broker"""
        logger.info("Disconnecting from MQTT broker")
        self.client.loop_stop()
        self.client.disconnect()
        self._connected = False
    
    def _on_connect(self, client, userdata, flags, rc):
        """Callback when connected to MQTT broker"""
        if rc == 0:
            logger.info("Connected to MQTT broker successfully")
            self._connected = True
            
            # Subscribe to topics
            self.client.subscribe(config.MQTT_TOPIC_RAINWATER_LEVEL)
            logger.info(f"Subscribed to topic: {config.MQTT_TOPIC_RAINWATER_LEVEL}")
            
            self.client.subscribe(config.MQTT_TOPIC_TMS_STATUS)
            logger.info(f"Subscribed to topic: {config.MQTT_TOPIC_TMS_STATUS}")
        else:
            logger.error(f"Failed to connect to MQTT broker, return code: {rc}")
            self._connected = False
    
    def _on_disconnect(self, client, userdata, rc):
        """Callback when disconnected from MQTT broker"""
        self._connected = False
        if rc != 0:
            logger.warning(f"Unexpected disconnect from MQTT broker, return code: {rc}")
        else:
            logger.info("Disconnected from MQTT broker")
    
    def _on_message(self, client, userdata, msg):
        """Callback when message received from MQTT broker"""
        try:
            topic = msg.topic
            payload = msg.payload.decode('utf-8')
            
            # DEBUG: Print raw MQTT message reception
            print(f"\n{'='*60}")
            print(f"DEBUG [CUS-MQTT]: Message received from TMS")
            print(f"  Topic: {topic}")
            print(f"  Payload: {payload}")
            print(f"{'='*60}\n")
            
            logger.debug(f"Received MQTT message on topic '{topic}': {payload}")
            
            if topic == config.MQTT_TOPIC_RAINWATER_LEVEL:
                self._handle_rainwater_level(payload)
            elif topic == config.MQTT_TOPIC_TMS_STATUS:
                self._handle_tms_status(payload)
            else:
                logger.warning(f"Received message on unknown topic: {topic}")
                
        except Exception as e:
            logger.error(f"Error processing MQTT message: {e}", exc_info=True)
    
    def _handle_rainwater_level(self, payload: str):
        """
        Handle rainwater level data from TMS
        Expected format: {"level": 35.5, "timestamp": 1234567890.123}
        """
        try:
            data = json.loads(payload)
            level = float(data['level'])
            timestamp = float(data.get('timestamp', 0))
            
            # DEBUG: Print parsed water level data
            print(f"DEBUG [CUS-MQTT]: Water level parsed - {level} cm (timestamp: {timestamp})")
            
            logger.info(f"Rainwater level received: {level} cm")
            
            # Call the callback to process the data
            if self.on_rainwater_level:
                self.on_rainwater_level(level, timestamp)
            
            # Send acknowledgment (optional)
            self._send_acknowledgment(level, timestamp)
            
        except (json.JSONDecodeError, KeyError, ValueError) as e:
            logger.error(f"Invalid rainwater level data format: {payload} - {e}")
    
    def _handle_tms_status(self, payload: str):
        """
        Handle status updates from TMS
        Expected format: {"status": "connected"|"disconnected"|"error", "message": "..."}
        """
        try:
            data = json.loads(payload)
            status = data.get('status', 'unknown')
            message = data.get('message', '')
            
            logger.info(f"TMS status update: {status} - {message}")
            
        except json.JSONDecodeError as e:
            logger.error(f"Invalid TMS status data format: {payload} - {e}")
    
    def _send_acknowledgment(self, level: float, timestamp: float):
        """Send acknowledgment to TMS (optional feature)"""
        try:
            ack_data = {
                'received_level': level,
                'received_timestamp': timestamp,
                'status': 'ok'
            }
            payload = json.dumps(ack_data)
            self.client.publish(config.MQTT_TOPIC_CUS_ACK, payload)
            logger.debug(f"Sent acknowledgment: {payload}")
        except Exception as e:
            logger.error(f"Failed to send acknowledgment: {e}")
    
    def is_connected(self) -> bool:
        """Check if connected to MQTT broker"""
        return self._connected
    
    def publish(self, topic: str, message: dict):
        """Publish a message to MQTT broker"""
        try:
            payload = json.dumps(message)
            self.client.publish(topic, payload)
            logger.debug(f"Published to {topic}: {payload}")
        except Exception as e:
            logger.error(f"Failed to publish message: {e}")
