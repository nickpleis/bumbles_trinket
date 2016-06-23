/*
 trinketstrip
 
 Have some fun with a Adafruit Trinket and NeoPixel LED strip.
 Push the button and cycle through all modes of the program.
 
 Hardware requiremetns:
 - Adafruit Trinket (ATTiny85)
 - Adafruit Electret Microphone Amplifier (ID: 1063)
   connected to Trinket pin #2 (analog A1)
 - Adafruit NeoPixel Digitial LED strip or anything like that
   connectet to Trinket pin #0
 - a button
   connected to Trinket pin #1 and 5V
 Written by Stefan Scherer under the BSD license.
 This parapgraph must be included in any redistribution.
 */
 
/*
 VU meter
 Derived from http://learn.adafruit.com/trinket-sound-reactive-led-color-organ/code
 
 LED "Color Organ" for Adafruit Trinket and NeoPixel LEDs.
 
Hardware requirements:
- Adafruit Trinket or Gemma mini microcontroller (ATTiny85).
- Adafruit Electret Microphone Amplifier (ID: 1063)
- Several Neopixels, you can mix and match
o Adafruit Flora RGB Smart Pixels (ID: 1260)
o Adafruit NeoPixel Digital LED strip (ID: 1138)
o Adafruit Neopixel Ring (ID: 1463)
 
Software requirements:
- Adafruit NeoPixel library
 
Connections:
- 5 V to mic amp +
- GND to mic amp -
- Analog pinto microphone output (configurable below)
- Digital pin to LED data input (configurable below)
 
Written by Adafruit Industries. Distributed under the BSD license.
This paragraph must be included in any redistribution.
*/

#include <avr/power.h>
#include <Adafruit_NeoPixel.h>
#include "Adafruit_NeoPixel.h"

#define N_PIXELS   50  // Number of pixels in strand
#define STRIP_PIN  0  // NeoPixel LED strand is connected to this pin
#define SAMPLES   30  // Length of buffer for dynamic level adjustment
#define DOT_RUN_MILLIS 40
#define LAST_PIXEL_OFFSET N_PIXELS-1
#define DEBOUNCE_DELAY 50  // the debounce time; increase if the output flickers

namespace bumblesLights {
  enum 
  {
    MODE_OFF,
    MODE_VUMETER,
    MODE_DOT_UP,
    MODE_DOT_DOWN,
    MODE_DOT_ZIGZAG,
    MODE_RAINBOW,
    MODE_RAINBOW_CYCLE,
    MODE_WIPE_RED,
    MODE_WIPE_GREEN,
    MODE_WIPE_BLUE,
    MODE_WIPE_YELLOW,
    MODE_WIPE_CYAN,
    MODE_WIPE_MAGENTA,
    MODE_MAX
  } MODE;

  typedef struct {
    Adafruit_NeoPixel neoPixel;

    byte peak;
    byte mode;
    byte reverse;
    byte dotCount;
    byte volCount;
    int vol[SAMPLES];
    int lvl;
    int minLvlAvg;
    int maxLvlAvg;
    long lastTime;
  } LightStrip;

  void resetStrip(LightStrip* strip) {
    strip->mode = MODE_OFF;
    strip->peak = 0;
    strip->reverse = 0;
    strip->dotCount = 0;
    strip->volCount = 0;
    strip->lvl = 10;
    strip->minLvlAvg = 0;
    strip->maxLvlAvg = 512;
    strip->lastTime = 0;

    memset(strip->vol, 0, sizeof(strip->vol));
  }

  LightStrip* initStrip() {
    LightStrip* strip = new LightStrip;
    resetStrip(strip);

    // Parameter 1 = number of pixels in strip
    // Parameter 2 = pin number (most are valid)
    // Parameter 3 = pixel type flags, add toPlease choose the Ardunio Application Folder.gether as needed:
    //   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
    //   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
    //   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
    //   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
    strip->neoPixel = Adafruit_NeoPixel(N_PIXELS, STRIP_PIN, NEO_GRB + NEO_KHZ800);
  
    // This is only needed on 5V Arduinos (Uno, Leonardo, etc.).
    // Connect 3.3V to mic AND TO AREF ON ARDUINO and enable this
    // line.  Audio samples are 'cleaner' at 3.3V.
    // COMMENT OUT THIS LINE FOR 3.3V ARDUINOS (FLORA, ETC.):
    // analogReference(EXTERNAL);

    strip->neoPixel.begin();
    strip->neoPixel.setBrightness(30);
    strip->neoPixel.show(); // Initialize all pixels to 'off'

    return strip;
  }

  void showDebugLight(uint8_t r, uint8_t g, uint8_t b, unsigned long duration) {
    Adafruit_NeoPixel ministrip = Adafruit_NeoPixel(1, STRIP_PIN, NEO_GRB + NEO_KHZ800);
    ministrip.begin();

    bool on = false;
    unsigned long started = millis();

    while((millis() - started) < duration) {
      if(on) {
        ministrip.setPixelColor(0, r, g, b);
      } else {
        ministrip.setPixelColor(0, 0, 0, 0);
      }

      ministrip.show();
      on = !on;
      delay(100);
    }
  }

  void drawDot(LightStrip* strip) {
    for (int i=0; i<N_PIXELS;i++)
    {
      if (i != strip->peak)
      {
        strip->neoPixel.setPixelColor(i, 0,0,0);
      }
      else
      {
        strip->neoPixel.setPixelColor(i, 255,255,255);
      }
    }
    strip->neoPixel.show();
  }

  void off(bumblesLights::LightStrip* strip) {
    if(strip->peak != N_PIXELS) {
      strip->peak = N_PIXELS; // move outside
      drawDot(strip);
    }
  }

  void runningDotUp(LightStrip* strip) {
    if(millis() - strip->lastTime >= DOT_RUN_MILLIS) {
      strip->lastTime = millis();
      drawDot(strip);
   
      if(strip->peak >= LAST_PIXEL_OFFSET) {
        strip->peak = 0;
      } else {
        strip->peak++;
      }
    }
  }

  void runningDotDown(LightStrip* strip) {
    if(millis() - strip->lastTime >= DOT_RUN_MILLIS) {
      strip->lastTime = millis();
      drawDot(strip);
   
      if(strip->peak <= 0) {
        strip->peak = LAST_PIXEL_OFFSET;
      } else {
        strip->peak--;
      }
    }
  }

  uint32_t Wheel(LightStrip* strip, byte WheelPos) {
    if(WheelPos < 85) {
     return strip->neoPixel.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
    } else if(WheelPos < 170) {
     WheelPos -= 85;
     return strip->neoPixel.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    } else {
     WheelPos -= 170;
     return strip->neoPixel.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
}

  void rainbow(LightStrip* strip) {
    uint16_t i;

    if(millis() - strip->lastTime >= DOT_RUN_MILLIS) {
      strip->lastTime = millis();

      if(strip->lvl >= 256) {
        strip->lvl = 0;
      } else {
        strip->lvl++;
      }

      for(i=0; i < strip->neoPixel.numPixels(); i++) {
        uint32_t wheelVal = Wheel(strip, (i+ strip->lvl) & 255);
        strip->neoPixel.setPixelColor(i, wheelVal);
      }
      strip->neoPixel.show();
    }
  }

  void drawPattern(LightStrip* strip) {
    switch(strip->mode) {
      case MODE_OFF:
        off(strip);
        break;

      case MODE_DOT_UP:
        runningDotUp(strip);
        break;

      case MODE_DOT_DOWN:
        runningDotDown(strip);
        break;

      case MODE_RAINBOW:
        rainbow(strip);
        break;
    }
  }
}

namespace bumblesButtons {
  typedef struct {
    int pin;
    byte lastOffTime; // the previous reading from the input pin
  } Button;

  Button* initButton(int pin) {
    Button* button = new Button;
    button->pin = pin;
    button->lastOffTime = 0;

    pinMode(pin, INPUT);

    return button;
  }

  void setupButton(Button button) {
    pinMode(button.pin, INPUT);
  }

  bool isPressed(Button* button) {
    byte reading = digitalRead(button->pin);

    if(button->pin == 1 && reading == HIGH) {
      bumblesLights::showDebugLight(0, 255, 0, 5000);
    } else if(button->pin == 2 && reading == HIGH) {
      bumblesLights::showDebugLight(0, 0, 255, 5000);
    }

    // check to see if you just pressed the button
    // (i.e. the input went from LOW to HIGH),  and you've waited
    // long enough since the last press to ignore any noise:  
    if (reading == HIGH) {
      if ((millis() - button->lastOffTime) > DEBOUNCE_DELAY) {
        return true;
      }
    } else {
      button->lastOffTime = millis();
    }

    return false;
  }
}

bumblesButtons::Button* g_button1;
bumblesButtons::Button* g_button2;
bumblesButtons::Button* g_button3;
bumblesButtons::Button* g_button4;

bumblesLights::LightStrip* g_strip;

void setup() {
  // initialize trinket to run at 16MHz
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
 
  g_strip = bumblesLights::initStrip();
  g_strip->mode = bumblesLights::MODE_DOT_UP;

  g_button1 = bumblesButtons::initButton(1);
  g_button2 = bumblesButtons::initButton(2);
  g_button3 = bumblesButtons::initButton(3);
  g_button4 = bumblesButtons::initButton(4);
 
  if(!g_strip->neoPixel.getPixels()) {
    bumblesLights::showDebugLight(255, 0, 0, 0xFFFFFFFF);
  }
}

void loop() {
  byte oldMode = g_strip->mode;

  if(bumblesButtons::isPressed(g_button1)) {
    g_strip->mode = bumblesLights::MODE_RAINBOW;
  } else if(bumblesButtons::isPressed(g_button2)) {
    g_strip->mode = bumblesLights::MODE_DOT_DOWN;
  } else if(bumblesButtons::isPressed(g_button3)) {
    g_strip->mode = bumblesLights::MODE_RAINBOW;
  } else if(bumblesButtons::isPressed(g_button4)) {
    g_strip->mode = bumblesLights::MODE_OFF;
  }

  if(g_strip->mode != oldMode) {
    bumblesLights::resetStrip(g_strip);
  }

  bumblesLights::drawPattern(g_strip);
}
