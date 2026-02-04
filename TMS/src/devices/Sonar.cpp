#include "Sonar.h"

#include "Arduino.h"


Sonar::Sonar(int echoP, int trigP, long maxTime) : echoPin(echoP), trigPin(trigP), timeOut(maxTime){
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);  
  temperature = 20; // default value
}

void Sonar::setTemperature(float temp){
  temperature = temp;
}
float Sonar::getSoundSpeed(){
  return 331.5 + 0.6*temperature;   
}

float Sonar::getDistance(){
    digitalWrite(trigPin,LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin,HIGH);
    delayMicroseconds(10); // Standard trigger pulse is 10us
    digitalWrite(trigPin,LOW);
    
    // pulseIn returns duration in microseconds
    float tUS = pulseIn(echoPin, HIGH, timeOut);
    
    if (tUS == 0) {
        return NO_OBJ_DETECTED;
    } else {
        // Correct unit conversion:
        // Sound speed is in m/s (e.g., 343 m/s)
        // Divide by 1,000,000 to get m/us
        // Multiply by 100 to get cm/us -> total factor is divide by 10,000
        // Distance = (time / 2) * speed_cm_us
        float d = (tUS / 2.0) * (getSoundSpeed() / 10000.0);
        return d;  
    }
}

