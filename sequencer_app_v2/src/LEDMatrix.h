#pragma once
// LEDMatrix.h
#include "Display.h"

namespace supersixteen {

class LedMatrix{
    public:
        void init(Display &display);

        void updateMatrix(int row);

        void multiplex_leds();

        void blink_step();

        void blink_led();

        void reset();

        void toggleLed(int led);
    
};

}