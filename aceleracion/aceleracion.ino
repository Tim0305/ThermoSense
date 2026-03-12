#include <Wire.h>

#define I2CDEV_IMPLEMENTATION I2CDEV_ARDUINO_WIRE
#include "I2Cdev.h"
#include "MPU6050.h"

MPU6050 mpu;

void setup() {
  Serial.begin(9600);

  // ESP32 (comenta si usas Arduino UNO/Mega)
  Wire.begin(21, 22);

  mpu.initialize();

  Serial.println("MPU6050 listo");
  delay(100);
}

void loop() {

  int16_t ax, ay, az;
  int16_t gx, gy, gz;
  int16_t tempRaw;

  mpu.getAcceleration(&ax, &ay, &az);
  mpu.getRotation(&gx, &gy, &gz);
  tempRaw = mpu.getTemperature();

  // ===== CONVERSIONES (igual que Adafruit internamente) =====

  // Acelerómetro ±2G
  float ax_ms2 = (ax / 16384.0) * 9.80665;
  float ay_ms2 = (ay / 16384.0) * 9.80665;
  float az_ms2 = (az / 16384.0) * 9.80665;

  // Giroscopio ±250°/s
  float gx_dps = gx / 131.0;
  float gy_dps = gy / 131.0;
  float gz_dps = gz / 131.0;

  // Temperatura
  float temperature = (tempRaw / 340.0) + 36.53;

  // ===== OUTPUT TIPO ADAFRUIT =====

  Serial.println("------");

  Serial.print("Acceleration (m/s^2): ");
  Serial.print(ax_ms2, 3);
  Serial.print(", ");
  Serial.print(ay_ms2, 3);
  Serial.print(", ");
  Serial.println(az_ms2, 3);

  if (ax_ms2 > abs(3) || ay_ms2 > abs(3) || az_ms2 < 8 || az_ms2 > 12)
    Serial.println("Cerrar");

  // Serial.print("Rotation (deg/s): ");
  // Serial.print(gx_dps, 3);
  // Serial.print(", ");
  // Serial.print(gy_dps, 3);
  // Serial.print(", ");
  // Serial.println(gz_dps, 3);

  // Serial.print("Temperature (C): ");
  // Serial.println(temperature, 2);

  delay(500);
}