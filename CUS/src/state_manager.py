"""
State Manager for Control Unit Subsystem (CUS)
Manages shared state across all components with thread-safe access
"""

import threading
import time
from collections import deque
from typing import Optional, List, Dict, Any
from . import config


class StateManager:
    """Thread-safe state manager for the CUS"""
    
    def __init__(self):
        self._lock = threading.Lock()
        
        # System mode
        self._mode = config.DEFAULT_MODE
        
        # Valve state
        self._valve_opening = config.VALVE_CLOSED
        
        # Rainwater level data
        self._rainwater_levels = deque(maxlen=config.N_MEASUREMENTS)
        
        # TMS connection tracking
        self._last_tms_message_time = None
        self._tms_connected = False
        
        # Global system update timestamp (for dashboard)
        self._last_system_update_time = time.time()
        
        # L1 threshold tracking (for T1 timer)
        self._l1_exceeded_start_time = None
        
    # ====================
    # Mode Management
    # ====================
    
    def get_mode(self) -> str:
        """Get current system mode"""
        with self._lock:
            return self._mode
    
    def set_mode(self, mode: str) -> bool:
        """Set system mode (AUTOMATIC or MANUAL)"""
        if mode not in [config.MODE_AUTOMATIC, config.MODE_MANUAL]:
            return False
        
        with self._lock:
            self._mode = mode
            self._last_system_update_time = time.time()
            return True
    
    def is_automatic_mode(self) -> bool:
        """Check if system is in AUTOMATIC mode"""
        with self._lock:
            return self._mode == config.MODE_AUTOMATIC
    
    def is_manual_mode(self) -> bool:
        """Check if system is in MANUAL mode"""
        with self._lock:
            return self._mode == config.MODE_MANUAL
    
    def is_unconnected(self) -> bool:
        """Check if system is in UNCONNECTED state"""
        with self._lock:
            return self._mode == config.MODE_UNCONNECTED
    
    # ====================
    # Valve Management
    # ====================
    
    def get_valve_opening(self) -> int:
        """Get current valve opening percentage (0-100)"""
        with self._lock:
            return self._valve_opening
    
    def set_valve_opening(self, opening: int) -> bool:
        """Set valve opening percentage (0-100)"""
        if not (config.VALVE_MIN <= opening <= config.VALVE_MAX):
            return False
        
        with self._lock:
            self._valve_opening = opening
            self._last_system_update_time = time.time()
            return True
    
    # ====================
    # Rainwater Level Management
    # ====================
    
    def add_rainwater_level(self, level: float, timestamp: Optional[float] = None):
        """Add a rainwater level measurement"""
        if timestamp is None:
            timestamp = time.time()
        
        with self._lock:
            self._rainwater_levels.append({
                'level': level,
                'timestamp': timestamp
            })
            self._last_tms_message_time = timestamp
            self._last_system_update_time = timestamp
            
            # If we were in UNCONNECTED state and receive data, restore previous mode
            if self._mode == config.MODE_UNCONNECTED:
                self._mode = config.DEFAULT_MODE
    
    def get_rainwater_levels(self) -> List[Dict[str, Any]]:
        """Get all stored rainwater level measurements"""
        with self._lock:
            return list(self._rainwater_levels)
    
    def get_latest_rainwater_level(self) -> Optional[float]:
        """Get the most recent rainwater level"""
        with self._lock:
            if len(self._rainwater_levels) > 0:
                return self._rainwater_levels[-1]['level']
            return None
    
    # ====================
    # TMS Connection Tracking
    # ====================
    
    def get_last_tms_message_time(self) -> Optional[float]:
        """Get timestamp of last message from TMS"""
        with self._lock:
            return self._last_tms_message_time
    
    def check_tms_timeout(self) -> bool:
        """
        Check if TMS has timed out
        Returns True if timeout detected and state changed to UNCONNECTED
        """
        with self._lock:
            if self._last_tms_message_time is None:
                return False
            
            time_since_last_message = time.time() - self._last_tms_message_time
            
            if time_since_last_message > config.T2_TIMEOUT:
                if self._mode != config.MODE_UNCONNECTED:
                    self._mode = config.MODE_UNCONNECTED
                    return True
            
            return False
    
    # ====================
    # L1 Threshold Timer
    # ====================
    
    def start_l1_timer(self):
        """Start the L1 threshold timer"""
        with self._lock:
            if self._l1_exceeded_start_time is None:
                self._l1_exceeded_start_time = time.time()
    
    def reset_l1_timer(self):
        """Reset the L1 threshold timer"""
        with self._lock:
            self._l1_exceeded_start_time = None
    
    def get_l1_timer_duration(self) -> Optional[float]:
        """Get how long the level has been above L1 (in seconds)"""
        with self._lock:
            if self._l1_exceeded_start_time is None:
                return None
            return time.time() - self._l1_exceeded_start_time
    
    def has_l1_timer_exceeded(self) -> bool:
        """Check if L1 timer has exceeded T1 threshold"""
        duration = self.get_l1_timer_duration()
        if duration is None:
            return False
        return duration >= config.T1_TIME
    
    # ====================
    # Full State Export
    # ====================
    
    def get_full_state(self) -> Dict[str, Any]:
        """Get complete system state as dictionary"""
        with self._lock:
            return {
                'mode': self._mode,
                'valve_opening': self._valve_opening,
                'rainwater_levels': list(self._rainwater_levels),
                'latest_level': self._rainwater_levels[-1]['level'] if len(self._rainwater_levels) > 0 else None,
                'last_update': self._last_system_update_time,
                'last_tms_message_time': self._last_tms_message_time,
                'l1_timer_active': self._l1_exceeded_start_time is not None,
                'l1_timer_duration': self.get_l1_timer_duration()
            }
