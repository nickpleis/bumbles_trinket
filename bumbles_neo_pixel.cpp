#include "include/bumbles_neo_pixel.h"

///////////////////////////////////////////////////////////////////////////////
// PUBLIC MEMBERS
///////////////////////////////////////////////////////////////////////////////

void LightStripNeoPixel::reset() {
    this->peak = 0;
    this->reverse = 0;
    this->dotCount = 0;
    this->volCount = 0;
    this->lvl = 10;
    this->minLvlAvg = 0;
    this->maxLvlAvg = 512;
    this->lastTime = 0;

    memset(this->vol, 0, sizeof(this->vol));

    for(int i = 0; i < N_PIXELS; i++) {
        this->neoPixel.setPixelColor(0, 0, 0, 0);
    }
    this->neoPixel.show();
  }

void LightStripNeoPixel::init() {
    this->reset();
    this->mode = MODE_OFF;

    // Parameter 1 = number of pixels in strip
    // Parameter 2 = pin number (most are valid)
    // Parameter 3 = pixel type flags, add toPlease choose the Ardunio Application Folder.gether as needed:
    //   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
    //   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
    //   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
    //   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
    this->neoPixel = Adafruit_NeoPixel(N_PIXELS, STRIP_PIN, NEO_GRB + NEO_KHZ800);

    // This is only needed on 5V Arduinos (Uno, Leonardo, etc.).
    // Connect 3.3V to mic AND TO AREF ON ARDUINO and enable this
    // line.  Audio samples are 'cleaner' at 3.3V.
    // COMMENT OUT THIS LINE FOR 3.3V ARDUINOS (FLORA, ETC.):
    // analogReference(EXTERNAL);

    this->neoPixel.begin();
    this->neoPixel.setBrightness(30);
    this->neoPixel.show(); // Initialize all pixels to 'off'
}

void LightStripNeoPixel::showDebugLight(uint8_t r, uint8_t g, uint8_t b, unsigned long duration) {
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

void LightStripNeoPixel::setMode(unsigned int mode) {
    this->mode = mode;
}

unsigned int LightStripNeoPixel::getMode() {
    return this->mode;
}

bool LightStripNeoPixel::isWorking() {
    return !this->neoPixel.getPixels();
}

void LightStripNeoPixel::draw() {
    switch(this->mode) {
        case MODE_OFF:
            this->off();
            break;

        case MODE_DOT_UP:
            this->runningDotUp();
            break;

        case MODE_DOT_DOWN:
            this->runningDotDown();
            break;

        case MODE_RAINBOW:
            this->rainbow();
            break;

        case MODE_WIPE_RED:
            this->colorWipe(255, 0, 0);
            break;

        case MODE_WIPE_GREEN:
            this->colorWipe(0, 255, 0);
            break;

        case MODE_WIPE_BLUE:
            this->colorWipe(0, 0, 255);
            break;

        case MODE_WIPE_YELLOW:
            this->colorWipe(255, 255, 0);
            break;

        case MODE_WIPE_CYAN:
            this->colorWipe(0, 255, 255);
            break;

        case MODE_WIPE_MAGENTA:
            this->colorWipe(255, 0, 255);
            break;
    }
}


///////////////////////////////////////////////////////////////////////////////
// PRIVATE MEMBERS
///////////////////////////////////////////////////////////////////////////////

void LightStripNeoPixel::drawDot() {
    for (int i=0; i<N_PIXELS;i++)
    {
      if (i != this->peak)
      {
        this->neoPixel.setPixelColor(i, 0,0,0);
      }
      else
      {
        this->neoPixel.setPixelColor(i, 255,255,255);
      }
    }
    this->neoPixel.show();
  }

void LightStripNeoPixel::off() {
    if(this->peak != N_PIXELS) {
      this->peak = N_PIXELS; // move outside
      this->drawDot();
    }
}

void LightStripNeoPixel::runningDotUp() {
    if(millis() - this->lastTime >= DOT_RUN_MILLIS) {
      this->lastTime = millis();
      this->drawDot();

      if(this->peak >= LAST_PIXEL_OFFSET) {
        this->peak = 0;
      } else {
        this->peak++;
      }
    }
}

void LightStripNeoPixel::runningDotDown() {
    if(millis() - this->lastTime >= DOT_RUN_MILLIS) {
      this->lastTime = millis();
      this->drawDot();

      if(this->peak <= 0) {
        this->peak = LAST_PIXEL_OFFSET;
      } else {
        this->peak--;
      }
    }
}

uint32_t LightStripNeoPixel::Wheel(byte WheelPos) {
    if(WheelPos < 85) {
        return this->neoPixel.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
    } else if(WheelPos < 170) {
        WheelPos -= 85;
        return this->neoPixel.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    } else {
        WheelPos -= 170;
        return this->neoPixel.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
}

void LightStripNeoPixel::rainbow() {
    uint16_t i;

    if(millis() - this->lastTime >= DOT_RUN_MILLIS) {
        this->lastTime = millis();

        if(this->lvl >= 256) {
            this->lvl = 0;
        } else {
        this->lvl++;
        }

        for(i=0; i < this->neoPixel.numPixels(); i++) {
            uint32_t wheelVal = this->Wheel((i+ this->lvl) & 255);
            this->neoPixel.setPixelColor(i, wheelVal);
        }
        this->neoPixel.show();
    }
}

void LightStripNeoPixel::colorWipe(uint8_t r, uint8_t g, uint8_t b)
{
    if (millis() - this->lastTime >= DOT_RUN_MILLIS) {
        this->lastTime = millis();

        this->neoPixel.setPixelColor(this->peak, r, g, b);
        this->neoPixel.show();

        if(this->peak >= LAST_PIXEL_OFFSET) {
            this->peak = 0;
        } else {
            this->peak++;
        }
    }
}
