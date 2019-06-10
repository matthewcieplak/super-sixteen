#pragma once
// LEDMatrix.h
#include "Display.h"
#include "Sequencer.h"

namespace supersixteen {

class LedMatrix{
    public:
        void init(Display &display, Sequencer &sequencer);

        void updateMatrix(int row);

        void multiplex_leds();

        void blink_step();

        void blink_led();

        void reset();

        void setMatrixFromSequencer();

        void toggleLed(int led);
    
};

}