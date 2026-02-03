#ifndef __HW_PLATFORM__
#define __HW_PLATFORM__
#include "config.h"
#include "devices/button.h"
#include "devices/servoMotorImpl.h"
#include "devices/lcd.h"
#include "devices/pot.h"


class HWPlatform
{

public:
  HWPlatform();
  void test();

  button *getButton();
  ServoMotorImpl *getMotor();
  Lcd *getLCD();
  Potentiometer *getPot();

private:
  button *resetButton;
  ServoMotorImpl *servo;
  Lcd *lcd;
  Potentiometer *pot;
};

#endif