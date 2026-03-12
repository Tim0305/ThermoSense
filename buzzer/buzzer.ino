const int BUZZER_PIN = 32;
const int hz = 1000;

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
}

void loop() {
  tone(BUZZER_PIN, hz);
  delay(1000);
  noTone(BUZZER_PIN);
  delay(1000);
}