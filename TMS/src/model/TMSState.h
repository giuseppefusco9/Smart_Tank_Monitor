#ifndef __TMS_STATE__
#define __TMS_STATE__

/**
 * Finite State Machine States for Tank Monitoring Subsystem
 */
enum TMSState {
  INIT,           // System initialization
  CONNECTING,     // Attempting WiFi and MQTT connection
  CONNECTED,      // Successfully connected to network and MQTT
  MONITORING,     // Normal operation - reading sensor and sending data
  DISCONNECTED    // Network or MQTT connection lost
};

/**
 * Convert state to string for debugging and logging
 */
inline const char* stateToString(TMSState state) {
  switch (state) {
    case INIT:         return "INIT";
    case CONNECTING:   return "CONNECTING";
    case CONNECTED:    return "CONNECTED";
    case MONITORING:   return "MONITORING";
    case DISCONNECTED: return "DISCONNECTED";
    default:           return "UNKNOWN";
  }
}

/**
 * State Manager Class
 * Manages FSM state transitions and provides state query methods
 */
class StateManager {
private:
  TMSState currentState;
  unsigned long lastTransitionTime;

public:
  StateManager() : currentState(INIT), lastTransitionTime(0) {}

  /**
   * Get current state
   */
  TMSState getState() const {
    return currentState;
  }

  /**
   * Set new state and record transition time
   */
  void setState(TMSState newState) {
    if (currentState != newState) {
      currentState = newState;
      lastTransitionTime = millis();
    }
  }

  /**
   * Get time elapsed in current state (ms)
   */
  unsigned long getTimeInState() const {
    return millis() - lastTransitionTime;
  }

  /**
   * Check if in normal operating state
   */
  bool isOperational() const {
    return currentState == MONITORING;
  }

  /**
   * Check if network is available
   */
  bool isConnected() const {
    return currentState == CONNECTED || currentState == MONITORING;
  }

  /**
   * Check if in error state
   */
  bool isError() const {
    return currentState == DISCONNECTED;
  }
};

#endif
