#include <OneWire.h>
#include <DallasTemperature.h>

const int TEMPERATURE_WIRE_BUS = 23;

// Setup a onewire instance to communicate with any OneWire devices
OneWire temperatureWire(TEMPERATURE_WIRE_BUS);

// Pass our OneWire instance to Dallas Temperature Sensor
DallasTemperature temperatureSensor(&temperatureWire);

void setup() {
  Serial.begin(9600);
  // start temperature sensor
  temperatureSensor.begin();
}

void loop() {
  temperatureSensor.requestTemperatures();
  float temperatureC = temperatureSensor.getTempCByIndex(0);
  Serial.print("Temperature: ");
  Serial.print(temperatureC);
  Serial.println(" C");
}
