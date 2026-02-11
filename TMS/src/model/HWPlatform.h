#ifndef __HW_PLATFORM__
#define __HW_PLATFORM__

#include "devices/Sonar.h"
#include "devices/Led.h"
#include "config.h"

/**
 * Hardware Platform
 * Encapsulates all hardware components (sensors, actuators)
 */
class HWPlatform {

public:
    HWPlatform();

    /**
     * Getter for green led component
     */
    Led* getGreenLed();

    /**
     * Getter for red led component
     */
    Led* getRedLed();

    /**
     * Getter for sonar sensor component
     */
    Sonar* getSonar();

private:
    Sonar* sonar;
    Led* greenLed;
    Led* redLed;

};

#endif