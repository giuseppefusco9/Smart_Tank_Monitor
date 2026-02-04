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
                        # DEBUG: Print raw serial message from WCS
                        print(f"\n{'='*60}")
                        print(f"DEBUG [CUS-SERIAL]: Message received from WCS")
                        print(f"  Raw Data: {line}")
                        print(f"{'='*60}\n")
                        
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
        
        Expected message formats:
        - Mode change: {"type": "mode", "value": "MANUAL"|"AUTOMATIC"}
        - Manual valve position: {"type": "valve", "value": 0-100}
        - Status: {"type": "status", "message": "..."}
        """
        try:
            data = json.loads(message)
            msg_type = data.get('type', 'unknown')
            
            if msg_type == 'mode':
                mode = data.get('value', '')
                # DEBUG: Print mode change from WCS
                print(f"DEBUG [CUS-SERIAL]: WCS mode changed to: {mode}")
                logger.info(f"WCS mode change: {mode}")
                if self.on_mode_change:
                    self.on_mode_change(mode)
            
            elif msg_type == 'valve':
                opening = int(data.get('value', 0))
                # DEBUG: Print manual valve change from WCS
                print(f"DEBUG [CUS-SERIAL]: WCS manual valve set to: {opening}%")
                logger.info(f"WCS manual valve position: {opening}%")
                if self.on_manual_valve:
                    self.on_manual_valve(opening)
            
            elif msg_type == 'status':
                status_msg = data.get('message', '')
                logger.info(f"WCS status: {status_msg}")
            
            else:
                logger.warning(f"Unknown message type from WCS: {msg_type}")
                
        except json.JSONDecodeError:
            # Not JSON format - might be plain text status message
            logger.info(f"WCS message (plain text): {message}")
        except Exception as e:
            logger.error(f"Error processing WCS message: {e}", exc_info=True)
    
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
            
            # DEBUG: Print valve command sent to WCS
            print(f"\n--- DEBUG [CUS-SERIAL]: Sending valve command to WCS: {opening}% ---\n")
            
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
