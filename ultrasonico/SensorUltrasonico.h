#ifndef SENSORULTRASONICO_H
#define SENSORULTRASONICO_H

#include "Arduino.h"

class SensorUltrasonico {
  public:
    int pinTrigger;
    int pinEcho;
    SensorUltrasonico(int pinTrigger, int pinEcho);
    long getDistanciaCM();
};

#endif
