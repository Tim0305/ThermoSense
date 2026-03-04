#ifndef SENSOR_HCSR04_H
#define SENSOR_HCSR04_H

#include "Arduino.h"

class SensorHCSR04 {
  public:
    int pinTrigger;
    int pinEcho;
    SensorHCSR04(int pinTrigger, int pinEcho);
    long getDistanciaCM();
};

#endif
