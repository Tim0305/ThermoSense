#ifndef BUZZER_H
#define BUZZER_H

#include "Arduino.h"

class Buzzer {
private:
  int pin;
  int hz;
public:
  Buzzer(int pin, int hz);
  void setFrequency(int hz);
  void on();
  void play(int ms);
  void off();
};

#endif