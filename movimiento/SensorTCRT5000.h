#ifndef SENSOR_TCRT5000_H
#define SENSOR_TCRT5000_H

#include "Arduino.h"

class SensorTCRT5000 {
  public:
    int pinData;
    SensorTCRT5000(int pinData);
    bool isActive();
};

#endif
