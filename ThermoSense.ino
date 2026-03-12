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
#include <WiFi.h>
#include <WebServer.h>

#define I2CDEV_IMPLEMENTATION I2CDEV_ARDUINO_WIRE

const int MAX_TEMPERATURE = 30; // C
const int MIN_TEMPERATURE = 0; // C
const int MAX_LEVEL = 100; // %
const int MIN_LEVEL = 0; // %
const int MAX_LEVEL_CM = 15;
const int MIN_LEVEL_CM = 2;
const int ALERT_MAX_TEMPERATURE = 28; // C
const int ALERT_MAX_LEVEL = 90; // %

// Tracked values
float temperature;
int level;

// Flags
bool FULL = false;
bool HOT = false;
bool OPENNED = false;

// Web Server
const char* SSID = "Megacable_2.4G_31F0";
const char* PASSWORD = "UbPCMCKb";
WebServer server(80);

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
void displayLocationInfo();
void handleRoot();
void handleData();
String getHtml();

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
  //display.flipScreenVertically();
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
  GPS_SERIAL.begin(9600, SERIAL_8N1, 16, 17);
  Serial.println("Waiting for GPS fix and satellites...");

  // Leds
  turnLedsOff();
  Serial.println("Leds ready");

  // Web Server
  WiFi.begin(SSID, PASSWORD);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();

  Serial.println("Web server started");
}

void loop() {
  // Web Server
  server.handleClient();

  // GPS
  while (GPS_SERIAL.available()) {
    gps.encode(GPS_SERIAL.read());
  }

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
  level = constrain(map(cm, MAX_LEVEL_CM, MIN_LEVEL_CM, 0, 100), 0, 100); // 0 -> 100 %

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
    OPENNED = true;
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

  if (abs(ax_ms2) > 3 || abs(ay_ms2) > 3 || abs(az_ms2) < 8 || abs(az_ms2) > 12) {
    // close
     servo.write(90);
     OPENNED = false;
  }

  if (digitalRead(OUT_PIN) && OPENNED)
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

void displayLocationInfo() {
  Serial.println(F("-------------------------------------"));
  Serial.println("\n Location Info:");

  Serial.print("Latitude:  ");
  Serial.print(gps.location.lat(), 6);
  Serial.print(" ");
  Serial.println(gps.location.rawLat().negative ? "S" : "N");

  Serial.print("Longitude: ");
  Serial.print(gps.location.lng(), 6);
  Serial.print(" ");
  Serial.println(gps.location.rawLng().negative ? "W" : "E");

  Serial.print("Fix Quality: ");
  Serial.println(gps.location.isValid() ? "Valid" : "Invalid");

  Serial.print("Satellites: ");
  Serial.println(gps.satellites.value());

  Serial.print("Altitude:   ");
  Serial.print(gps.altitude.meters());
  Serial.println(" m");

  Serial.print("Speed:      ");
  Serial.print(gps.speed.kmph());
  Serial.println(" km/h");

  Serial.print("Course:     ");
  Serial.print(gps.course.deg());
  Serial.println("°");

  Serial.print("Date:       ");
  if (gps.date.isValid()) {
    Serial.printf("%02d/%02d/%04d\n", gps.date.day(), gps.date.month(), gps.date.year());
  } else {
    Serial.println("Invalid");
  }

  Serial.print("Time (UTC): ");
  if (gps.time.isValid()) {
    Serial.printf("%02d:%02d:%02d\n", gps.time.hour(), gps.time.minute(), gps.time.second());
  } else {
    Serial.println("Invalid");
  }

  Serial.println(F("-------------------------------------"));
}

// Web server
void handleRoot() {
  server.send(200, "text/html", getHTML());
}

void handleData() {
  String json = "{";
  json += "\"temperature\":" + String(temperature) + ",";
  json += "\"level\":" + String(level) + ",";

  if (gps.location.isValid()) {
    json += "\"lat\":" + String(gps.location.lat(),6) + ",";
    json += "\"lng\":" + String(gps.location.lng(),6);
  } else {
    json += "\"lat\":0,\"lng\":0";
  }

  json += "}";

  server.send(200, "application/json", json);
}

String getHTML() {
  String html = R"rawliteral(
  <!DOCTYPE html>
  <html lang="es">

  <head>

  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">

  <title>Smart Bottle</title>

  <style>

  body{
  margin:0;
  font-family: 'Segoe UI', Arial;
  background:linear-gradient(135deg,#0f172a,#1e293b);
  color:white;
  text-align:center;
  }

  .header{
  padding:30px;
  font-size:28px;
  font-weight:bold;
  }

  .container{
  display:flex;
  flex-wrap:wrap;
  justify-content:center;
  gap:25px;
  padding:20px;
  }

  .card{
  background:#1e293b;
  padding:25px;
  border-radius:16px;
  width:260px;
  box-shadow:0 10px 25px rgba(0,0,0,0.4);
  transition:transform 0.2s;
  }

  .card:hover{
  transform:translateY(-5px);
  }

  .title{
  font-size:18px;
  opacity:0.8;
  }

  .value{
  font-size:42px;
  font-weight:bold;
  margin-top:10px;
  }

  .bar{
  width:100%;
  height:18px;
  background:#334155;
  border-radius:10px;
  overflow:hidden;
  margin-top:15px;
  }

  .fill{
  height:100%;
  background:linear-gradient(90deg,#22c55e,#4ade80);
  width:0%;
  transition:width 0.5s;
  }

  .gps{
  font-size:15px;
  margin-top:10px;
  }

  button{
  margin-top:15px;
  padding:10px 18px;
  border:none;
  border-radius:10px;
  background:#3b82f6;
  color:white;
  font-size:15px;
  cursor:pointer;
  transition:background 0.2s;
  }

  button:hover{
  background:#2563eb;
  }

  .footer{
  margin-top:30px;
  opacity:0.5;
  font-size:14px;
  }

  </style>

  </head>

  <body>

  <div class="header">
  Smart Bottle Dashboard
  </div>

  <div class="container">

  <div class="card">
  <div class="title">🌡 Temperatura</div>
  <div class="value" id="temp">-- °C</div>
  </div>

  <div class="card">
  <div class="title">💧 Nivel de Agua</div>
  <div class="value" id="level">-- %</div>

  <div class="bar">
  <div class="fill" id="levelbar"></div>
  </div>

  </div>

  <div class="card">
  <div class="title">📍 Ubicación GPS</div>

  <div class="gps" id="gps">
  Buscando señal...
  </div>

  <a id="map" target="_blank">
  <button>Ver en Google Maps</button>
  </a>

  </div>

  </div>

  <div class="footer">
  ESP32 Smart Bottle
  </div>

  <script>

  function updateData(){

  fetch("/data")
  .then(response => response.json())
  .then(data => {

  document.getElementById("temp").innerHTML =
  data.temperature.toFixed(1) + " °C";

  document.getElementById("level").innerHTML =
  data.level + " %";

  document.getElementById("levelbar").style.width =
  data.level + "%";

  if(data.lat != 0){

  document.getElementById("gps").innerHTML =
  "Lat: " + data.lat + "<br>Lng: " + data.lng;

  document.getElementById("map").href =
  "https://maps.google.com/?q="+data.lat+","+data.lng;

  }else{

  document.getElementById("gps").innerHTML =
  "Sin señal GPS";

  }

  });

  }

  setInterval(updateData,1000);

  updateData();

  </script>

  </body>
  </html>
  )rawliteral";

  return html;
}


