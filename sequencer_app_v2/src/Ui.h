#pragma once
#include <stdint.h>
#include "Calibrate.h"
#include "Dac.h"

namespace supersixteen{
class Ui{
    public:
        void init(Calibration& calibration, Dac& dac, Sequencer& sequencer);
        void poll();
        void multiplex();
        bool isSequencing();
        void onStepIncremented();

    private:
        void onShiftButton(bool state);
        void onPlayButton(bool state);
        void onLoadButton(bool state);
        void onSaveButton(bool state);
        void finishSaving();

        void onRecButton(bool state);
        void onRepeatButton(bool state);
        void onGlideButton(bool state);       

        void invertEncoder(); 
        void displaySequenceParam();
        
        void onButtonToggle(int button, bool button_state);
        void onEncoderIncrement(int increment_amount);
        void selectStep(int step);
        void selectBar(byte bar);
        void glideButton();
        void initializeSequenceMode();
        void initializeCalibrationMode();
        void updateCalibration(int step);
        bool cancelSaveOrLoad();
        void shiftFunction(int button);
        void clearSequence();
        void loadNextSequence();

};
}