#pragma once
// Sequencer.h
#include "Calibrate.h"
#include "Dac.h"

namespace supersixteen {

class Sequencer{
    public:
        void init(Calibration& calibration, Dac& dac);

        void updateClock();

        void incrementStep();

        void selectStep(int step);

        void incrementTempo(int amount);

        void onPlayButton();

        void toggleGlide();

    private:
        void updateGlide();
        void updateGate();

};

}