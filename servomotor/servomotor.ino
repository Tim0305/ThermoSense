#include <ESP32Servo.h>

static const int servoPin = 18;

Servo servo1;

void setup() {
  Serial.begin(115200);
  servo1.attach(servoPin);
}

void loop() {
  servo1.write(90);
  delay(1000);
  servo1.write(0);
  delay(1000);
}