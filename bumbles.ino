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

#include "include/light_strip.h"
#include "include/bumbles_neo_pixel.h"

#define DEBOUNCE_DELAY 50  // the debounce time; increase if the output flickers

#ifndef __AVR_ATtiny85__ //DON'T COMPILE THIS FOR TRINKET
  #define LOGGING_ENABLED 1
#endif

namespace bumblesButtons {
  class Button {
    public:
     int pin;
     long lastOffTime; // the previous reading from the input pin

     void init(int pin, LightStrip* strip);
  };

  void Button::init(int pin, LightStrip* strip) {
    this->pin = pin;
    this->lastOffTime = millis();

    pinMode(pin, INPUT);
  }

  bool isPressed(Button* button) {
    byte reading = digitalRead(button->pin);

    #ifdef LOGGING_ENABLED
      Serial.println("Pin: " + String(button->pin) + ", Reading: " + String(reading));
    #endif

    // The idea here is to make sure that this is a "real" button press
    // not some accidental voltage across the pin. To do that we always
    // set a `lastOffTime` everytime we come through and the voltage is
    // LOW on that pin. Then when the voltage moves to HIGH, we simply
    // check to see if enough time has passed before we return true
    // signifying that the button has been pressed.

    if (reading == HIGH) {
      #ifdef LOGGING_ENABLED
        Serial.println("Pin: " + String(button->pin) + " reading == HIGH");
      #endif

      if((millis() - button->lastOffTime) > DEBOUNCE_DELAY) {
        #ifdef LOGGING_ENABLED
          Serial.println("Button pressed!");
        #endif

        return true;
      }
    } else {
      button->lastOffTime = millis();
    }

    return false;
  }
}

bumblesButtons::Button g_button1;
bumblesButtons::Button g_button2;
bumblesButtons::Button g_button3;
bumblesButtons::Button g_button4;

LightStrip* g_strip;

void setup() {
  #ifdef LOGGING_ENABLED
    Serial.begin(9600); // USB is always 12 Mbit/sec
    Serial.println("Initializing everything...");
  #endif

  g_strip = new LightStripNeoPixel();
  g_strip->init();
  g_strip->setMode(LightStripNeoPixel::MODE_DOT_UP);

  g_button1.init(0, g_strip);
  g_button2.init(1, g_strip);
  g_button3.init(3, g_strip);
  //g_button4.init(, &g_strip);
 
  if(g_strip->isWorking()) {
    g_strip->showDebugLight(255, 0, 0, 0xFFFFFFFF);
  }
}

void loop() {
  byte oldMode = g_strip->getMode();

  if(bumblesButtons::isPressed(&g_button1)) {
    g_strip->setMode(LightStripNeoPixel::MODE_DOT_DOWN);
  } else if(bumblesButtons::isPressed(&g_button2)) {
     g_strip->setMode(LightStripNeoPixel::MODE_DOT_UP);
  } else if(bumblesButtons::isPressed(&g_button3)) {
    g_strip->setMode(LightStripNeoPixel::MODE_RAINBOW);
  } //else if(bumblesButtons::isPressed(&g_button4)) {
  //   g_strip.mode = bumblesLights::MODE_OFF;
  // }

  #ifdef LOGGING_ENABLED
    Serial.println("MODE: " + String(g_strip->getMode()) + " " + String(oldMode));
  #endif

  if(g_strip->getMode() != oldMode) {
    #ifdef LOGGING_ENABLED
      Serial.println("Resetting...");
    #endif

    g_strip->reset();
  }

  g_strip->draw();
  delay(600);
}
