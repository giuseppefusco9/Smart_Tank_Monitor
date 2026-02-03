#ifndef BUTTONIMPL
#define BUTTONIMPL

#include "button.h"

class ButtonImpl: public button {
 
public: 
  ButtonImpl(int pin);
  bool isPressed();

private:
  int pin;

};
#endif