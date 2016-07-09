#ifndef LIGHT_STRIP_H
#define LIGHT_STRIP_H

#include <avr/power.h>

class LightStrip {
    public:
        virtual void init() = 0;
        virtual void draw() = 0;
        virtual bool isWorking() = 0;
        virtual void reset() = 0;
        virtual void showDebugLight(uint8_t r, uint8_t g, uint8_t b, unsigned long duration) = 0;

        virtual void setMode(unsigned int mode) = 0;
        virtual unsigned int getMode() = 0;
};

#endif
