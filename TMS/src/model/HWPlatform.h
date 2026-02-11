#ifndef __HW_PLATFORM__
#define __HW_PLATFORM__

#include "devices/Sonar.h"
#include "devices/Led.h"
#include "config.h"

class HWPlatform {

public:
    HWPlatform();

    Led* getGreenLed();
    Led* getRedLed();
    Sonar* getSonar();

private:
    Sonar* sonar;
    Led* greenLed;
    Led* redLed;

};

#endif