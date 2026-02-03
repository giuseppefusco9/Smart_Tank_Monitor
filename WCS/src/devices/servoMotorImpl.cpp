#include "servoMotorImpl.h"
#include "Arduino.h"


ServoMotorImpl::ServoMotorImpl(int pin){
  this->pin = pin;  
  _on = false;
  this->angle = this->NOT_INITIALIZE;
}

void ServoMotorImpl::on(){
  // updated values: min is 544, max 2400 (see ServoTimer2 doc)
  motor.attach(pin); //, 544, 2400);    
  _on = true;
}

bool ServoMotorImpl::isOn(){
  return _on;
}

void ServoMotorImpl::setPosition(int angle){
  this->angle = angle;
	if (this->angle > 180){
		this->angle = 180;
	} else if (this->angle < 0){
		this->angle = 0;
	}
  // 750 -> 0, 2250 -> 180 
  // 750 + angle*(2250-750)/180
  // updated values: min is 544, max 2400 (see ServoTimer2 doc)
  float coeff = (2400.0-544.0)/180;
  motor.write(544 + angle*coeff);              
}

int ServoMotorImpl::getAngle(){
  return this->angle;
}

void ServoMotorImpl::off(){
  _on = false;
  motor.detach();    
}