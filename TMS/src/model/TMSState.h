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
const char* stateToString(TMSState state);

/**
 * State Manager Class
 * Manages FSM state transitions and provides state query methods
 */
class StateManager {
private:
  TMSState currentState;
  unsigned long lastTransitionTime;

public:
  StateManager();

  /**
   * Get current state
   */
  TMSState getState() const;

  /**
   * Set new state and record transition time
   */
  void setState(TMSState newState);

  /**
   * Get time elapsed in current state (ms)
   */
  unsigned long getTimeInState() const;

  /**
   * Check if in normal operating state
   */
  bool isOperational() const;

  /**
   * Check if network is available
   */
  bool isConnected() const;

  /**
   * Check if in error state
   */
  bool isError() const;
};

#endif
