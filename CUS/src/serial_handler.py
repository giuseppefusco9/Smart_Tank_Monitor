"""
Serial Handler for Control Unit Subsystem (CUS)
Handles communication with Water Channel Subsystem (WCS) via serial port
"""

import json
import logging
import threading
import time
import serial
from typing import Callable, Optional
from . import config


logger = logging.getLogger(__name__)


class SerialHandler:
    """Handles serial communication with WCS"""
    
    def __init__(self, on_mode_change_callback: Optional[Callable[[str], None]] = None,
                 on_manual_valve_callback: Optional[Callable[[int], None]] = None):
        """
        Initialize serial handler
        
        Args:
            on_mode_change_callback: Callback function(mode) called when WCS reports mode change
            on_manual_valve_callback: Callback function(opening) called when WCS reports manual valve change
        """
        self.on_mode_change = on_mode_change_callback
        self.on_manual_valve = on_manual_valve_callback
        
        self.serial_port: Optional[serial.Serial] = None
        self._running = False
        self._read_thread = None
        self._write_lock = threading.Lock()
    
    def connect(self):
        """Connect to serial port"""
        try:
            logger.info(f"Opening serial port {config.SERIAL_PORT} at {config.SERIAL_BAUDRATE} baud")
            self.serial_port = serial.Serial(
                port=config.SERIAL_PORT,
                baudrate=config.SERIAL_BAUDRATE,
                timeout=config.SERIAL_TIMEOUT
            )
            
            # Give Arduino time to reset after serial connection
            time.sleep(2)
            
            logger.info("Serial port opened successfully")
            
            # Start read thread
            self._running = True
            self._read_thread = threading.Thread(target=self._read_loop, daemon=True)
            self._read_thread.start()
            
        except serial.SerialException as e:
            logger.error(f"Failed to open serial port: {e}")
            raise
    
    def disconnect(self):
        """Disconnect from serial port"""
        logger.info("Closing serial port")
        self._running = False
        
        if self._read_thread:
            self._read_thread.join(timeout=2)
        
        if self.serial_port and self.serial_port.is_open:
            self.serial_port.close()
            logger.info("Serial port closed")
    
    def _read_loop(self):
        """Background thread to continuously read from serial port"""
        logger.info("Serial read thread started")
        
        while self._running:
            try:
                if self.serial_port and self.serial_port.is_open and self.serial_port.in_waiting > 0:
                    # Read line from serial port
                    line = self.serial_port.readline().decode('utf-8').strip()
                    
                    if line:
                        logger.debug(f"Received from WCS: {line}")
                        self._process_message(line)
                else:
                    # Brief sleep to avoid excessive CPU usage
                    time.sleep(0.1)
                    
            except Exception as e:
                logger.error(f"Error reading from serial port: {e}", exc_info=True)
                time.sleep(1)  # Wait before retrying
        
        logger.info("Serial read thread stopped")
    
    def _process_message(self, message: str):
        """
        Process incoming message from WCS
        """
        try:
            data = json.loads(message)
            if not isinstance(data, dict):
                logger.debug(f"Received non-object JSON from WCS: {message}")
                return
                
            # Get type safely, ensuring it's a string
            msg_type = data.get('type')
            if not isinstance(msg_type, str):
                logger.debug(f"Message missing 'type' or type is not string: {message}")
                return
            
            if msg_type == 'mode':
                mode = int(data.get('value', 0))
                if mode == 0:
                    mode = "AUTOMATIC"
                elif mode == 1:
                    mode = "MANUAL"
                else:
                    mode = "Unknown"
                logger.info(f"WCS mode change: {mode}")
                if self.on_mode_change:
                    self.on_mode_change(mode)
            
            elif msg_type == 'valve':
                try:
                    opening = int(data.get('value', 0))
                    logger.info(f"WCS manual valve position: {opening}%")
                    if self.on_manual_valve:
                        self.on_manual_valve(opening)
                except (ValueError, TypeError):
                    logger.warning(f"Invalid valve value from WCS: {data.get('value')}")
            
            elif msg_type == 'status':
                status_msg = str(data.get('message', ''))
                logger.info(f"WCS status: {status_msg}")
            
            else:
                logger.debug(f"Unhandled message type from WCS: {msg_type}")
                
        except json.JSONDecodeError:
            # Not JSON format - ignore
            logger.debug(f"Non-JSON message from WCS: {message}")
        except Exception as e:
            logger.error(f"Error processing WCS message: {e}")
    
    def send_valve_command(self, opening: int) -> bool:
        """
        Send valve opening command to WCS
        
        Args:
            opening: Valve opening percentage (0-100)
            
        Returns:
            True if sent successfully, False otherwise
        """
        if not (config.VALVE_MIN <= opening <= config.VALVE_MAX):
            logger.error(f"Invalid valve opening value: {opening}")
            return False
        
        if not self.serial_port or not self.serial_port.is_open:
            logger.error("Serial port not open, cannot send valve command")
            return False
        
        try:
            # Create command message
            command = {
                'type': 'valve',
                'value': opening
            }
            message = json.dumps(command) + '\n'
            
            # Thread-safe write
            with self._write_lock:
                self.serial_port.write(message.encode('utf-8'))
                self.serial_port.flush()
            
            logger.info(f"Sent valve command to WCS: {opening}%")
            return True
            
        except Exception as e:
            logger.error(f"Failed to send valve command: {e}", exc_info=True)
            return False
    
    def send_display_update(self, mode: str, valve_opening: int) -> bool:
        """
        Send display update to WCS (for LCD display)
        
        Args:
            mode: Current system mode
            valve_opening: Current valve opening
            
        Returns:
            True if sent successfully, False otherwise
        """
        if not self.serial_port or not self.serial_port.is_open:
            logger.error("Serial port not open, cannot send display update")
            return False
        
        try:
            # Create display update message
            command = {
                'type': 'display',
                'mode': mode,
                'valve': valve_opening
            }
            message = json.dumps(command) + '\n'
            
            # Thread-safe write
            with self._write_lock:
                self.serial_port.write(message.encode('utf-8'))
                self.serial_port.flush()
            
            logger.debug(f"Sent display update to WCS: {mode}, {valve_opening}%")
            return True
            
        except Exception as e:
            logger.error(f"Failed to send display update: {e}", exc_info=True)
            return False
    
    def is_connected(self) -> bool:
        """Check if serial port is connected"""
        return self.serial_port is not None and self.serial_port.is_open
