#include "HWPlatform.h"
#include "devices/buttonimpl.h"
#include "kernel/MsgService.h"
#include "devices/servoMotorImpl.h"
#include "config.h"
#include "kernel/logger.h"
#include <Arduino.h>

#define TEST_ANGLE 56

void wakeUp(){}

HWPlatform::HWPlatform(){
  this->resetButton = new ButtonImpl(BUTTON_PIN);
  this->servo = new ServoMotorImpl(SERVO_PIN);
  this->lcd = new Lcd();
  this->pot = new Potentiometer(POT_PIN);
  
  this->servo->on();
}

button* HWPlatform::getButton(){
  return this->resetButton;
}


ServoMotorImpl* HWPlatform::getMotor(){
  return this->servo;
}

Lcd* HWPlatform::getLCD(){
  return this->lcd;
}

Potentiometer* HWPlatform::getPot(){
  return this->pot;
}

void HWPlatform::test(){

  if(this->servo->getAngle()  == -1 || this->servo->getAngle() == 1){
    this->servo->setPosition(179);
  } 
   else if (this->servo->getAngle() == 179){
    this->servo->setPosition(1);
  }

  if(this->getButton()->isPressed()){
    Logger.log(F("TEST: button"));
  }

  this->lcd->writeModeMessage("TEST");
  this->lcd->writePercMessage("TEST");
  
}