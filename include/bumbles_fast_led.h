#include <HID.h>

#include <FastLED.h> //Include FastLED Library
 
FASTLED_USING_NAMESPACE
 
//Warn me if FASTLED != 3.1 or later
#if FASTLED_VERSION < 3001000
#error "Requires FastLED 3.1 or later; check github for latest code."
#endif
 
//LED SETUP
#define LED_PIN 4 //Data pin
#define LED_TYPE    WS2811 //LED Type (Note: WS2811 driver used for WS2812B)
#define COLOR_ORDER RGB //LED Color Order
#define NUM_LEDS 50 //Number of LEDs
#define FRAMES_PER_SECOND  120 //Number of frames per second
#define NUM_MODES 3 //Max number of effects

class LightStripFastLED : public LightStrip {
    public:
        CRGB leds[NUM_LEDS]; //Name of LED Array

        //LED Color Palette & Blending
        CRGBPalette16 currentPalette; //Color Palette
        TBlendType    currentBlending; //Color Blending

        //MODE VARIABLES
        int ledMode = 0; //FIRST ACTIVE MODE
        int BRIGHTNESS = 192; //0-255.  Lower number saves battery life, higher number is screamingly bright
        int SATURATION = 255; //0 is white (no color) and 255 is fully saturated with color
        int HUE = 0; //0-255, around the color wheel
        int STEPS = 4; //Wider or narrower bands of color
        int SPEEDO = 10; //The speed of the animation
         
        bool colorORblack=true; // Container for blink function
        unsigned long lastUpdate=0; //container for removing delay();

        uint8_t gHue = 0; // rotating "base color" used by many of the patterns

        void init();
        void draw();

}

