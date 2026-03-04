#include "Arduino.h"
#include "SensorHCSR04.h"

SensorHCSR04::SensorHCSR04(int pinTrigger, int pinEcho) {
  this->pinTrigger = pinTrigger;
  this->pinEcho = pinEcho;

  pinMode(pinTrigger, OUTPUT);
  pinMode(pinEcho, INPUT);
  digitalWrite(pinTrigger, LOW);
}

long SensorHCSR04::getDistanciaCM() {
  digitalWrite(this->pinTrigger, LOW);
  delayMicroseconds(2);
  digitalWrite(this->pinTrigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(this->pinTrigger, LOW);

  long duration = pulseIn(this->pinEcho, HIGH);
  int distance = duration * 0.034 / 2;
  return distance;
}
