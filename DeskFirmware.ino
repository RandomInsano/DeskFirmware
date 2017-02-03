#include <EEPROM.h>
#include <avr/sleep.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN 6
#define LIGHT_STATE_ADDRESS 0

Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, PIN, NEO_GRB + NEO_KHZ800);

enum LightStates {
  LSCycling,
  LSStaticLow,
  LSStaticMedium,
  LSStaticHigh,
  LSSentinal
};

int lights_state;  // What mode the lights should be in

void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  Serial.begin(115200);

  // Read in, then bump eeprom value. Lets us cycle states by resetting
  // Should survive ~100,000 restarts according to datasheet
  lights_state = EEPROM.read(LIGHT_STATE_ADDRESS) % LSSentinal;
  EEPROM.write(LIGHT_STATE_ADDRESS, lights_state);

  // For now, most states don't require a full loop, so put AVR to sleep
  //set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  //sleep_enable();  
}

void loop() {
  switch (lights_state) {
    case LSCycling:
      rainbowCycle(20);
      break;
    case LSStaticLow:
      staticLights(strip.Color(10, 10, 10));
      break;
    case LSStaticMedium:
      staticLights(strip.Color(127, 127, 127));
      break;
    case LSStaticHigh:
      staticLights(strip.Color(255, 255, 255));
      break;
  }

  if (Serial.available() > 0) {
    int value = (Serial.read() - '1') % LSSentinal;

    // Skip right out if we don't want the value
    if (value < 0) return;

    lights_state = value;
    EEPROM.write(LIGHT_STATE_ADDRESS, lights_state);
    Serial.print("Wrote new brightness: ");
    Serial.println(lights_state);
    delay(500);
  }

  delay(500);

  Serial.print("State:");
  Serial.println(lights_state);
}


void staticLights(uint32_t color) {
  for(int i = 0; i< strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
  }  
  strip.show();

  //sleep_mode();
  //sleep_disable(); // Never gets here until interrupt
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
