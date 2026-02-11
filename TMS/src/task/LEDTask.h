#ifndef __LED_TASK__
#define __LED_TASK__

#include "kernel/Task.h"
#include "model/HWPlatform.h"
#include "model/TMSState.h"
#include "config.h"

/**
 * LED Task
 * Controls LED indicators based on system state
 */
class LEDTask : public Task {
private:
  HWPlatform* hw;
  StateManager* stateManager;
  bool blinkState;
  unsigned long lastBlinkTime;

public:
  LEDTask(HWPlatform* hw, StateManager* stateManager);
  
  void init(int period);
  void tick();

  /**
   * Turn off all LEDs
   */
  void allOff();

  /**
   * Turn on all LEDs (for testing)
   */
  void allOn();
};

#endif
