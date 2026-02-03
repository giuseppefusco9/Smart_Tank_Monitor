#ifndef __SERVO_MOTOR_IMPL__
#define __SERVO_MOTOR_IMPL__

#include "servoMotor.h"
#include <Arduino.h>
#include "servoTimer2.h"

class ServoMotorImpl: public ServoMotor {

public:
  ServoMotorImpl(int pin);

  void on();
  bool isOn();
  void setPosition(int angle);
  virtual int getAngle();
  void off();

  const int NOT_INITIALIZE = -1;
    
private:
  int pin; 
  int angle;
  bool _on;
  ServoTimer2 motor; 
};

#endif