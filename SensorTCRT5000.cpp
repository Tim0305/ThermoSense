#include "SensorTCRT5000.h"

SensorTCRT5000::SensorTCRT5000(int pinData) {
  this->pinData = pinData;
  pinMode(pinData, INPUT);
}

bool SensorTCRT5000::isActive() {
  return !digitalRead(pinData);
}