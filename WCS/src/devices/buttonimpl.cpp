#include "buttonimpl.h"
#include <Arduino.h>

ButtonImpl::ButtonImpl(int pin) : pin(pin) {
    pinMode(pin, INPUT);
}

bool ButtonImpl::isPressed() {
    return digitalRead(pin) == HIGH;
}