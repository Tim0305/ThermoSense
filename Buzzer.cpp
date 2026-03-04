#include "Buzzer.h"

Buzzer::Buzzer(int pin, int hz) {
  this->pin = pin;
  this->hz = hz;

  pinMode(pin, OUTPUT);
}

void Buzzer::on() {
  tone(pin, hz);
}

void Buzzer::off() {
  noTone(pin);
}

void Buzzer::play(int ms) {
  on();
  delay(ms);
  off();
}
