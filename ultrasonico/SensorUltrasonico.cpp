#include "Arduino.h"
#include "SensorUltrasonico.h"

SensorUltrasonico::SensorUltrasonico(int pinTrigger, int pinEcho) {
  this->pinTrigger = pinTrigger;
  this->pinEcho = pinEcho;

  pinMode(pinTrigger, OUTPUT);
  pinMode(pinEcho, INPUT);
  digitalWrite(pinTrigger, LOW);
}

long SensorUltrasonico::getDistanciaCM() {
  digitalWrite(this->pinTrigger, LOW);
  delayMicroseconds(2);
  digitalWrite(this->pinTrigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(this->pinTrigger, LOW);

  long duration = pulseIn(this->pinEcho, HIGH);
  int distance = duration * 0.034 / 2;
  return distance;
}
