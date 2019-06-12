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
        bool stepWasIncremented();

        void selectStep(int step);
        bool getStepOnOff(int step);
        int getCurrentStep();
        int getPrevStep();

        int incrementTempo(int amount);

        void onPlayButton();

        bool toggleGlide();
        bool setPitch(int newVal);
        bool setOctave(int newVal);
        bool setDuration(int newVal);
        bool setCv(int newVal);

        bool getGlide();
        int getPitch();
        int getOctave();
        int getDuration();
        int getCv();

        bool  *getStepMatrix();

        int getSelectedStep();
        
    private:
        void updateGlide();
        void updateGate();

};

}