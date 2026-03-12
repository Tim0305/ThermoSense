#include <Wire.h>
#include "SSD1306Wire.h"
#include "images.h"

// Integrated OLED Display
const int OLED_SCL = 22;  // D5
const int OLED_SDA = 21;  // D6
const int OLED_ADDRESS = 0x3C;
const int OLED_WIDTH = 128;
const int OLED_HEIGHT = 64;

void showInfo();
void drawBottle(int percentage);

SSD1306Wire display(OLED_ADDRESS, OLED_SDA, OLED_SCL, GEOMETRY_128_64);
int p = 100;

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
  showInfo();
  delay(1000);
  p -= 10;
}

void showInfo() {
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