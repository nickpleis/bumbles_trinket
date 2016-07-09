#include <avr/power.h>
#include <Adafruit_NeoPixel.h>
#include "Adafruit_NeoPixel.h"

#include "light_strip.h"

#define SAMPLES   30  // Length of buffer for dynamic level adjustment
#define N_PIXELS   24  // Number of pixels in strand
#define STRIP_PIN  2  // NeoPixel LED strand is connected to this pin
#define DOT_RUN_MILLIS 40
#define LAST_PIXEL_OFFSET N_PIXELS-1

class LightStripNeoPixel : public LightStrip {
    public:
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

        enum {
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

        virtual void init();
        virtual void draw();
        virtual bool isWorking();
        virtual void reset();
        virtual void showDebugLight(uint8_t r, uint8_t g, uint8_t b, unsigned long duration);
        virtual void setMode(unsigned int mode);
        virtual unsigned int getMode();
      private:
          void drawDot();
          void off();
          void runningDotUp();
          void runningDotDown();
          void rainbow();
          uint32_t Wheel(byte WheelPos);
          void colorWipe(uint8_t r, uint8_t g, uint8_t b);
};

