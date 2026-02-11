#include "HWPlatform.h"

HWPlatform::HWPlatform() {
    sonar = new Sonar(SONAR_ECHO_PIN, SONAR_TRIG_PIN, SONAR_TIMEOUT);
    greenLed = new Led(GREEN_LED_PIN);
    redLed = new Led(RED_LED_PIN);
}

Led* HWPlatform::getGreenLed() {
    return this->greenLed;
}

Led* HWPlatform::getRedLed() {
    return this->redLed;
}

Sonar* HWPlatform::getSonar() {
    return this->sonar;
}