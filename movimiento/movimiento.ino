#include "SensorTCRT5000.h"

const int TCRT5000_PIN = D1;
SensorTCRT5000 sensor(TCRT5000_PIN);

void setup() {
  Serial.begin(9600);
}

void loop() {
  int val = sensor.isActive();
  if (val == HIGH)
    Serial.println("Movimiento detectado");

  delay(10);
}
