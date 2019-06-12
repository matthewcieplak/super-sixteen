#pragma once
// LEDMatrix.h
#include "Display.h"
#include "Sequencer.h"

namespace supersixteen {

class LedMatrix{
    public:
        void init(Display &display, Sequencer &sequencer);

        void updateMatrix(int row);

        void multiplexLeds();

        void blinkStep();

        void blinkLed();

        void blinkCurrentStep();

        void reset();

        void setMatrixFromSequencer();

        void toggleLed(int led);
    
};

}