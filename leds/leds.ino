#include <FastLED.h>

#define LED_PIN 5
#define NUM_LEDS 5

CRGB leds[NUM_LEDS];

int nivel = 5; // ejemplo

void setup() {
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
}

void loop() {

  for(int i = 0; i < NUM_LEDS; i++) {
    
    if(i < nivel) {
      leds[i] = CRGB::Blue;   // LED encendido
    } 
    else {
      leds[i] = CRGB::Black;  // LED apagado
    }

  }

  FastLED.show();
}