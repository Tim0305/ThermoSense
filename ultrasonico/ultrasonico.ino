#include "SensorUltrasonico.h"

const int ULTRASONIC_TRIGGER = D6;
const int ULTRASONIC_ECHO = D7;
SensorUltrasonico ultrasonicSensor(ULTRASONIC_TRIGGER, ULTRASONIC_ECHO);

void setup() {
  Serial.begin(9600);
  Serial.println("Hello world");
}

void loop() {
  long cm = ultrasonicSensor.getDistanciaCM();
  Serial.print("Distancia: ");
  Serial.println(cm);
}
