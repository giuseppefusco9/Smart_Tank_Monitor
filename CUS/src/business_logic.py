"""
Business Logic for Control Unit Subsystem (CUS)
Implements the core policy for rainwater level monitoring and valve control
"""

import logging
import threading
import time
from typing import Optional, Callable
from . import config
from .state_manager import StateManager


logger = logging.getLogger(__name__)


class BusinessLogic:
    """Implements business logic for tank monitoring system"""
    
    def __init__(self, state_manager: StateManager, on_valve_change_callback: Optional[Callable[[int], None]] = None):
        self.state = state_manager
        self.on_valve_change = on_valve_change_callback
        self._running = False
        self._monitor_thread = None
        
    def start(self):
        """Start the business logic monitoring thread"""
        if self._running:
            logger.warning("Business logic already running")
            return
        
        self._running = True
        self._monitor_thread = threading.Thread(target=self._monitor_loop, daemon=True)
        self._monitor_thread.start()
        logger.info("Business logic started")
    
    def stop(self):
        """Stop the business logic monitoring thread"""
        self._running = False
        if self._monitor_thread:
            self._monitor_thread.join(timeout=2)
        logger.info("Business logic stopped")
    
    def _monitor_loop(self):
        """Main monitoring loop - runs continuously"""
        while self._running:
            try:
                # Check TMS connection timeout
                self._check_tms_connection()
                
                # Process rainwater level (only in AUTOMATIC mode)
                if self.state.is_automatic_mode():
                    self._process_rainwater_level()
                
                # Sleep briefly to avoid excessive CPU usage
                time.sleep(0.5)
                
            except Exception as e:
                logger.error(f"Error in business logic monitor loop: {e}", exc_info=True)
    
    def _check_tms_connection(self):
        """Check if TMS has timed out and update state accordingly"""
        if self.state.check_tms_timeout():
            logger.warning(f"TMS timeout detected (no message for {config.T2_TIMEOUT}s) - entering UNCONNECTED state")
            # In UNCONNECTED state, we might want to close the valve for safety
            # This is a design decision - adjust as needed
            self.set_valve_opening(config.VALVE_CLOSED)
    
    def _process_rainwater_level(self):
        """
        Process the current rainwater level and apply valve control policy
        Policy:
        - If level > L2: Open valve 100% immediately
        - If level > L1 (but < L2) for T1 seconds: Open valve 50%
        - If level < L1: Close valve (0%)
        """
        current_level = self.state.get_latest_rainwater_level()
        
        if current_level is None:
            # No data available yet
            return
        
        # Check L2 threshold (highest priority - immediate action)
        if current_level >= config.L2_THRESHOLD:
            logger.info(f"Level {current_level}cm >= L2 ({config.L2_THRESHOLD}cm) - opening valve to 100%")
            self.set_valve_opening(config.VALVE_L2_OPENING)
            self.state.reset_l1_timer()
            return
        
        # Check L1 threshold (medium priority - requires T1 time)
        if current_level >= config.L1_THRESHOLD:
            # Level is above L1 but below L2
            
            # Start or continue L1 timer
            self.state.start_l1_timer()
            
            # Check if we've been above L1 for T1 seconds
            if self.state.has_l1_timer_exceeded():
                logger.info(f"Level {current_level}cm >= L1 ({config.L1_THRESHOLD}cm) for {config.T1_TIME}s - opening valve to 50%")
                self.set_valve_opening(config.VALVE_L1_OPENING)
            else:
                timer_duration = self.state.get_l1_timer_duration()
                logger.debug(f"Level above L1, waiting for T1 timer ({timer_duration:.1f}/{config.T1_TIME}s)")
            
            return
        
        # Level is below L1 - close valve
        if current_level < config.L1_THRESHOLD:
            # Reset L1 timer since we're below threshold
            if self.state.get_l1_timer_duration() is not None:
                logger.info(f"Level {current_level}cm < L1 ({config.L1_THRESHOLD}cm) - closing valve")
                self.state.reset_l1_timer()
            
            self.set_valve_opening(config.VALVE_CLOSED)
            return
    
    def set_valve_opening(self, opening: int) -> bool:
        """
        Set valve opening and return success status
        This will trigger communication with WCS
        """
        if self.state.set_valve_opening(opening):
            logger.info(f"Valve opening set to {opening}%")
            
            # Notify external components (like SerialHandler) immediately
            if self.on_valve_change:
                self.on_valve_change(opening)
                
            return True
        else:
            logger.error(f"Invalid valve opening value: {opening}")
            return False
    
    def process_rainwater_data(self, level: float, timestamp: Optional[float] = None):
        """
        Process incoming rainwater level data from TMS
        """
        if timestamp is None:
            timestamp = time.time()
        
        logger.debug(f"Received rainwater level: {level}cm at {timestamp}")
        
        # Store the data
        self.state.add_rainwater_level(level, timestamp)
        
        # If we're in AUTOMATIC mode, trigger immediate processing
        # (in addition to the monitoring loop)
        if self.state.is_automatic_mode():
            self._process_rainwater_level()
    
    def switch_mode(self, new_mode: str) -> bool:
        """
        Switch system mode (AUTOMATIC or MANUAL)
        Returns True if successful
        """
        if new_mode not in [config.MODE_AUTOMATIC, config.MODE_MANUAL]:
            logger.error(f"Invalid mode: {new_mode}")
            return False
        
        old_mode = self.state.get_mode()
        
        # Don't switch if in UNCONNECTED state (must wait for TMS to reconnect)
        if self.state.is_unconnected():
            logger.warning("Cannot switch mode while in UNCONNECTED state")
            return False
        
        if self.state.set_mode(new_mode):
            logger.info(f"Mode switched from {old_mode} to {new_mode}")
            
            # When switching to AUTOMATIC, reset L1 timer
            if new_mode == config.MODE_AUTOMATIC:
                self.state.reset_l1_timer()
            
            return True
        
        return False
    
    def set_manual_valve_opening(self, opening: int) -> bool:
        """
        Set valve opening in MANUAL mode
        Returns True if successful, False if not in MANUAL mode or invalid value
        """
        if not self.state.is_manual_mode():
            logger.warning("Cannot set manual valve opening - not in MANUAL mode")
            return False
        
        return self.set_valve_opening(opening)
