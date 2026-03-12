#include "Buzzer.h"
#include "SensorTCRT5000.h"
#include "SensorHCSR04.h"
#include "images.h"
#include "SSD1306Wire.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include "I2Cdev.h"
#include "MPU6050.h"
#include <ESP32Servo.h>
#include <FastLED.h>
#include <TinyGPSPlus.h>

#define I2CDEV_IMPLEMENTATION I2CDEV_ARDUINO_WIRE

const int MAX_TEMPERATURE = 30; // C
const int MIN_TEMPERATURE = 0; // C
const int MAX_LEVEL = 100; // %
const int MIN_LEVEL = 0; // %
const int ALERT_MAX_TEMPERATURE = 28; // C
const int ALERT_MAX_LEVEL = 90; // %

// Tracked values
float temperature;
int level;

// Flags
bool FULL = false;
bool HOT = false;

// OLED delay
const long OLED_ON_TIME_DURATION = 3000;
unsigned long currentTime = 0;
unsigned long prevTime = 0;
bool displayOn = false;

// OLED Display
const int OLED_SCL_PIN = 22;
const int OLED_SDA_PIN = 21;
const int OLED_ADDRESS = 0x3C;
const int OLED_WIDTH = 128;
const int OLED_HEIGHT = 64;
SSD1306Wire display(OLED_ADDRESS, OLED_SDA_PIN, OLED_SCL_PIN, GEOMETRY_128_64);

// Buzzer
const int BUZZER_PIN = 32;
const int BUZZER_HZ = 1000;
Buzzer buzzer(BUZZER_PIN, BUZZER_HZ);

// TCRT5000
const int TCRT5000_PIN = 4;
SensorTCRT5000 sensorTCRT5000(TCRT5000_PIN);

// Temperature
const int TEMPERATURE_WIRE_BUS = 23;
// Setup a onewire instance to communicate with any OneWire devices
OneWire temperatureWire(TEMPERATURE_WIRE_BUS);
// Pass our OneWire instance to Dallas Temperature Sensor
DallasTemperature temperatureSensor(&temperatureWire);

// HC-SR04
const int HCSR04_TRIGGER_PIN = 19;
const int HCSR04_ECHO_PIN = 33;
SensorHCSR04 sensorHCSR04(HCSR04_TRIGGER_PIN, HCSR04_ECHO_PIN);

// MPU6050
MPU6050 mpu;

// ServoMotor
const int SERVO_PIN = 18;
Servo servo;

// Leds
const int LED_PIN = 5;
const int NUM_LEDS = 5; // number of leds
CRGB leds[NUM_LEDS];

// GPS
TinyGPSPlus gps;
#define GPS_SERIAL Serial2

// 74HC08
const int A1_PIN = 13;
const int A2_PIN = 14;
const int OUT_PIN = 27;

// Functions
void showInfo();
void drawBottle(int percentage);
void turnLedsOff();

void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.println();

  // Init sensors and actuators
  // 74HC08
  pinMode(A1_PIN, OUTPUT);
  pinMode(A2_PIN, OUTPUT);
  pinMode(OUT_PIN, INPUT);
  Serial.println("74HC08 ready");
  
  // Display
  display.init();
  display.flipScreenVertically();
  display.clear();
  Serial.println("Display ready");

  // Temperature
  temperatureSensor.begin();
  Serial.println("Temperature Sensor ready");

  // MPU
  mpu.initialize();
  Serial.println("Accelerometer ready");

  // Servo Motor
  servo.attach(SERVO_PIN);
  Serial.println("Servo motor ready");

  // Leds
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  Serial.println("Leds ready");

  // GPS
  GPS_SERIAL.begin(9600);
  Serial.println("Waiting for GPS fix and satellites...");
}

void loop() {
  // get temperature
  temperatureSensor.requestTemperatures();
  temperature = temperatureSensor.getTempCByIndex(0);

  if (temperature >= ALERT_MAX_TEMPERATURE) {
    HOT = true;
    digitalWrite(A1_PIN, HIGH);
  } else {
    HOT = false;
    digitalWrite(A1_PIN, LOW);
  }

  // get level
  long cm = sensorHCSR04.getDistanciaCM();
  level = map(cm, 10, 2, 0, 100);

  if (level >= ALERT_MAX_LEVEL) {
    FULL = true;
    digitalWrite(A2_PIN, HIGH);
  } else {
    FULL = false;
    digitalWrite(A2_PIN, LOW);
  }

  // Accelerometer
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);

  float ax_ms2 = (ax / 16384.0) * 9.80;
  float ay_ms2 = (ay / 16384.0) * 9.80;
  float az_ms2 = (az / 16384.0) * 9.80;
  
  if (sensorTCRT5000.isActive()) {
    displayOn = true;
    prevTime = millis();
  }

  if (displayOn) {
    servo.write(0);
    currentTime = millis();
    if (currentTime - prevTime >= OLED_ON_TIME_DURATION) {
      displayOn = false;          
      display.displayOff(); // Turn off the display
      turnLedsOff();
    }
    else
      showInfo();
  }

  if (ax_ms2 > abs(3) || ay_ms2 > abs(3) || az_ms2 < 8 || az_ms2 > 12)
    servo.write(90);

  if (digitalRead(OUT_PIN))
    buzzer.on();
  else
    buzzer.off();
}

void showInfo() {
  // Turn on the display
  display.displayOn();
  display.clear();

  // Title
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_10);
  display.drawString(OLED_WIDTH / 2, 0, "INFORMACIÓN");

  // Divison
  display.drawHorizontalLine(0, 12, 128);

  // Data
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);

  // Temperature
  display.drawString(0, 17, String(temperature) + " °C");

  // Level
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 35, "Nivel:");
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 48, String(level) + "%");

  // Bottle
  drawBottle(level);
  display.display();

  // Leds
  if (level >= 70) {
    leds[0] = CRGB::Green;
    leds[1] = CRGB::Green;
    leds[2] = CRGB::Yellow;
    leds[3] = CRGB::Red;
    leds[4] = CRGB::Red;
  } else if (level >= 40) {
    leds[0] = CRGB::Black;
    leds[1] = CRGB::Black;
    leds[2] = CRGB::Yellow;
    leds[3] = CRGB::Red;
    leds[4] = CRGB::Red;
  }
  else if (level >= 5) {
    leds[0] = CRGB::Black;
    leds[1] = CRGB::Black;
    leds[2] = CRGB::Black;
    leds[3] = CRGB::Red;
    leds[4] = CRGB::Red;
  }
  else {
    turnLedsOff();
  }

  FastLED.show();
}

void drawBottle(int percentage) {
  int x = 92;
  int y = 16;
  display.drawXbm(90, 16, 24, 48, bottle_bitmap);

  // Draw rectangle
  int maxHeight = 33;
  int height = map(percentage, 0, 100, 0, maxHeight);

  int width = 20;
  int fillY = y + 13 + (maxHeight - height);
  display.fillRect(x, fillY, width, height);
}

void turnLedsOff() {
  leds[0] = CRGB::Black;
  leds[1] = CRGB::Black;
  leds[2] = CRGB::Black;
  leds[3] = CRGB::Black;
  leds[4] = CRGB::Black;
  FastLED.show();
}