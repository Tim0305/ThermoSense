#include "Buzzer.h"
#include "SensorTCRT5000.h"
#include "images.h"
#include "SSD1306Wire.h"

// Integrated OLED Display
const int OLED_SCL = D6;  // D6
const int OLED_SDA = D5;  // D5
const int OLED_ADDRESS = 0x3C;
const int OLED_WIDTH = 128;
const int OLED_HEIGHT = 64;
SSD1306Wire display(OLED_ADDRESS, OLED_SDA, OLED_SCL, GEOMETRY_128_64);
int p = 100;

// Buzzer
const int BUZZER_PIN = D7;
const int HZ = 1000;
Buzzer buzzer(BUZZER_PIN, HZ);

// TCRT5000
const int TCRT5000_PIN = D1;
SensorTCRT5000 sensorTCRT5000(TCRT5000_PIN);

void showInfo();
void drawBottle(int percentage);

void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.println();
  Serial.println("Ready!!!");

  // Display
  display.init();
  display.flipScreenVertically();
  display.clear();
}

void loop() {
  if (sensorTCRT5000.isActive())
    showInfo();

  delay(10);
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
  display.drawString(0, 17, "20 °C");

  // Level
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 35, "Nivel:");
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 48, "100%");

  // Bottle
  drawBottle(p);
  display.display();

  delay(3000);

  // Turn off the display
  display.displayOff();
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
