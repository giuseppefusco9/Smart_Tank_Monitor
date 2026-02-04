"""
Main Controller for Control Unit Subsystem (CUS)
Smart Tank Monitoring System

This is the main entry point that coordinates all subsystems:
- MQTT communication with Tank Monitoring Subsystem (TMS)
- Serial communication with Water Channel Subsystem (WCS)
- HTTP server for Dashboard Subsystem (DBS)
"""

import logging
import signal
import sys
import time
from typing import Optional

from src import config
from src.state_manager import StateManager
from src.business_logic import BusinessLogic
from src.mqtt_handler import MQTTHandler
from src.serial_handler import SerialHandler
from src.http_server import HTTPServer


# Configure logging
logging.basicConfig(
    level=getattr(logging, config.LOG_LEVEL),
    format=config.LOG_FORMAT
)
logger = logging.getLogger(__name__)


class ControlUnitSystem:
    """Main controller for the CUS"""
    
    def __init__(self):
        logger.info("Initializing Control Unit Subsystem")
        
        # Initialize state manager
        self.state_manager = StateManager()
        
        # Initialize business logic
        self.business_logic = BusinessLogic(
            self.state_manager,
            on_valve_change_callback=self._on_business_logic_valve_change
        )
        
        # Initialize MQTT handler
        self.mqtt_handler = MQTTHandler(
            on_rainwater_level_callback=self._on_rainwater_level_received
        )
        
        # Initialize serial handler
        self.serial_handler = SerialHandler(
            on_mode_change_callback=self._on_wcs_mode_change,
            on_manual_valve_callback=self._on_wcs_manual_valve
        )
        
        # Initialize HTTP server
        self.http_server = HTTPServer(
            get_state_callback=self._get_system_state,
            set_mode_callback=self._set_system_mode,
            set_valve_callback=self._set_manual_valve
        )
        
        # Track running state
        self._running = False
        
        # Set up signal handlers for graceful shutdown
        signal.signal(signal.SIGINT, self._signal_handler)
        signal.signal(signal.SIGTERM, self._signal_handler)
    
    def start(self):
        """Start all subsystems"""
        try:
            logger.info("=" * 60)
            logger.info("Starting Control Unit Subsystem")
            logger.info("=" * 60)
            
            # Start business logic
            self.business_logic.start()
            logger.info("✓ Business logic started")
            
            # Connect to subsystems in background threads to avoid blocking HTTP server
            import threading
            
            def connect_mqtt():
                try:
                    self.mqtt_handler.connect()
                    logger.info("✓ MQTT connected")
                except Exception as e:
                    logger.error(f"✗ Failed to connect to MQTT broker: {e}")
            
            def connect_serial():
                try:
                    self.serial_handler.connect()
                    logger.info("✓ Serial port connected")
                except Exception as e:
                    logger.error(f"✗ Failed to connect to serial port: {e}")

            threading.Thread(target=connect_mqtt, daemon=True).start()
            threading.Thread(target=connect_serial, daemon=True).start()
            
            # Start HTTP server (runs in main thread)
            logger.info("✓ Starting HTTP server...")
            self._running = True
            
            # Start periodic sync with WCS display
            import threading
            self._sync_thread = threading.Thread(target=self._sync_wcs_display, daemon=True)
            self._sync_thread.start()
            
            logger.info("=" * 60)
            logger.info("Control Unit Subsystem is running")
            logger.info(f"HTTP API available at http://localhost:{config.HTTP_PORT}")
            logger.info("Press Ctrl+C to stop")
            logger.info("=" * 60)
            
            # Run HTTP server (blocking)
            self.http_server.run()
            
        except Exception as e:
            logger.error(f"Error starting CUS: {e}", exc_info=True)
            self.stop()
            sys.exit(1)
    
    def stop(self):
        """Stop all subsystems"""
        logger.info("Stopping Control Unit Subsystem")
        self._running = False
        
        # Stop business logic
        try:
            self.business_logic.stop()
            logger.info("✓ Business logic stopped")
        except Exception as e:
            logger.error(f"Error stopping business logic: {e}")
        
        # Disconnect MQTT
        try:
            self.mqtt_handler.disconnect()
            logger.info("✓ MQTT disconnected")
        except Exception as e:
            logger.error(f"Error disconnecting MQTT: {e}")
        
        # Disconnect serial
        try:
            self.serial_handler.disconnect()
            logger.info("✓ Serial port closed")
        except Exception as e:
            logger.error(f"Error closing serial port: {e}")
        
        logger.info("Control Unit Subsystem stopped")
    
    def _signal_handler(self, sig, frame):
        """Handle shutdown signals"""
        logger.info(f"\nReceived signal {sig}, shutting down...")
        self.stop()
        sys.exit(0)
    
    # ====================
    # Callback Handlers
    # ====================
    
    def _on_rainwater_level_received(self, level: float, timestamp: float):
        """Callback when rainwater level data received from TMS via MQTT"""
        logger.debug(f"Rainwater level callback: {level} cm at {timestamp}")
        
        # Pass to business logic for processing
        self.business_logic.process_rainwater_data(level, timestamp)
        
        # Update WCS display
        self._update_wcs_display()
    
    def _on_wcs_mode_change(self, mode: str):
        """Callback when WCS reports mode change (from button press)"""
        logger.info(f"WCS reported mode change: {mode}")
        
        # Update business logic
        if self.business_logic.switch_mode(mode):
            logger.info(f"Mode switched to {mode} successful")
            # Force a display update back to WCS to confirm the mode change
            self._update_wcs_display()
        else:
            logger.warning(f"Failed to switch to mode {mode} (perhaps already in that mode or unconnected)")
    
    def _on_wcs_manual_valve(self, opening: int):
        """Callback when WCS reports manual valve position change (from potentiometer)"""
        logger.info(f"WCS manual valve position: {opening}%")
        
        # Only accept if in MANUAL mode
        if self.state_manager.is_manual_mode():
            self.state_manager.set_valve_opening(opening)
    
    def _get_system_state(self) -> dict:
        """Get current system state for HTTP API"""
        return self.state_manager.get_full_state()
    
    def _set_system_mode(self, mode: str) -> bool:
        """Set system mode from HTTP API"""
        logger.info(f"DBS REQUEST: Switch to mode {mode}")
        success = self.business_logic.switch_mode(mode)
        
        if success:
            logger.info(f"DBS SUCCESS: Mode switched to {mode}")
            # Update WCS display
            self._update_wcs_display()
        else:
            logger.warning(f"DBS FAILURE: Could not switch to mode {mode}")
        
        return success
    
    def _set_manual_valve(self, opening: int) -> bool:
        """Set valve opening from HTTP API (MANUAL mode only)"""
        logger.info(f"DBS REQUEST: Set manual valve to {opening}%")
        success = self.business_logic.set_manual_valve_opening(opening)
        
        if success:
            logger.info(f"DBS SUCCESS: Manual valve set to {opening}%")
            # The serial command is already sent via the on_valve_change callback
            # registered in BusinessLogic, so we don't need to call it explicitly here
            
            # Update WCS display
            self._update_wcs_display()
        else:
            logger.warning(f"DBS FAILURE: Could not set manual valve to {opening}% (check if in MANUAL mode)")
        
        return success
    
    def _on_business_logic_valve_change(self, opening: int):
        """Callback when business logic determines valve opening should change"""
        logger.debug(f"BusinessLogic triggered valve change to {opening}%")
        # Send command to WCS immediately
        if self.serial_handler.is_connected():
            logger.info(f"Routing valve command to SerialHandler: {opening}%")
            self.serial_handler.send_valve_command(opening)
        else:
            logger.warning("Cannot route valve command: SerialHandler is NOT connected")
    
    # ====================
    # WCS Communication
    # ====================
    
    def _update_wcs_display(self):
        """Update WCS LCD display with current state"""
        mode = self.state_manager.get_mode()
        valve_opening = self.state_manager.get_valve_opening()
        
        # Send display update to WCS
        self.serial_handler.send_display_update(mode, valve_opening)
    
    def _sync_wcs_display(self):
        """Periodically sync WCS display (background thread)"""
        while self._running:
            try:
                # Also send valve command to ensure WCS is in sync
                valve_opening = self.state_manager.get_valve_opening()
                self.serial_handler.send_valve_command(valve_opening)
                
                # Update display
                self._update_wcs_display()
                
                # Sleep for 2 seconds
                time.sleep(2)
                
            except Exception as e:
                logger.error(f"Error in WCS sync: {e}")
                time.sleep(5)


def main():
    """Main entry point"""
    print("""
╔═══════════════════════════════════════════════════════════╗
║   Smart Tank Monitoring System - Control Unit Subsystem   ║
║                        (CUS)                              ║
╚═══════════════════════════════════════════════════════════╝
    """)
    
    # Create and start the system
    cus = ControlUnitSystem()
    cus.start()


if __name__ == '__main__':
    main()
