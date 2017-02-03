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
  LSCycling = 0,
  LSStaticLow = 1,
  LSStaticMedium = 2,
  LSStaticHigh = 3,
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
  EEPROM.write(LIGHT_STATE_ADDRESS, lights_state + 1);

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
      staticLights(3);
      break;
    case LSStaticMedium:
        staticLights(2);
        break;
    case LSStaticHigh:
        staticLights(1);
        break;
  }

  Serial.print("State:");
  Serial.println(lights_state);
}


void staticLights(unsigned char brightness) {
  for(int i = 0; i< strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(255 / brightness, 255 / brightness, 255 / brightness));
  }  
  strip.show();

  //sleep_mode();
  //sleep_disable(); // Never gets here until interrupt

  delay(1000);
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
